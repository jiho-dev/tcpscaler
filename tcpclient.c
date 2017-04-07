#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

static void readcb(struct bufferevent *bev, void *ctx)
{
  struct evbuffer *input = bufferevent_get_input(bev);
  size_t len = evbuffer_get_length(input);
  /* Discard input */
  evbuffer_drain(input, len);
}

static void writecb(evutil_socket_t fd, short events, void *ctx)
{
  struct bufferevent *bev = ctx;
  struct evbuffer *output = bufferevent_get_output(bev);
  static char data[64] = {0};
  evbuffer_add(output, data, 64);
}

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
  if (events & BEV_EVENT_ERROR) {
    perror("Connection error");
  }
}

int main(int argc, char** argv)
{
  struct event_base *base;
  struct bufferevent **bufevents;
  struct addrinfo hints;
  struct addrinfo *res_list, *res;
  struct sockaddr_storage *server;
  struct event *writeev;
  struct timeval timeout;
  int server_len;
  int sock;
  int ret;
  unsigned long int nb_conn, conn, rate;
  char host_s[24];
  char port_s[6];

  if (argc < 5) {
    fprintf(stderr, "usage: %s <host> <port> <nb_conn> <rate>\n", argv[0]);
    fprintf(stderr, "Connects to the specified host and port, with the chosen number of TCP connections.\n");
    fprintf(stderr, "[rate] is the total number of writes per second towards the server, accross all TCP connections.\n");
    fprintf(stderr, "Each write is 64 bytes.\n");
    return 1;
  }
  nb_conn = strtoul(argv[3], NULL, 10);
  rate = strtoul(argv[4], NULL, 10);
  timeout.tv_sec = nb_conn / rate;
  timeout.tv_usec = (1000000 * nb_conn / rate) % 1000000;
  timeout.tv_usec = timeout.tv_usec > 0 ? timeout.tv_usec : 1;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  ret = getaddrinfo(argv[1], argv[2], &hints, &res_list);
  if (ret != 0) {
    fprintf(stderr, "Error in getaddrinfo: %s\n", gai_strerror(ret));
    return 1;
  }

  for (res = res_list; res != NULL; res = res->ai_next) {
    sock = socket(res->ai_family, res->ai_socktype,
		  res->ai_protocol);
    if (sock == -1)
      continue;

    getnameinfo(res->ai_addr, res->ai_addrlen, host_s, 24,
		port_s, 6, NI_NUMERICHOST | NI_NUMERICSERV);
    printf("Trying to connect to %s port %s...\n", host_s, port_s);
    if (connect(sock, res->ai_addr, res->ai_addrlen) != -1) {
      printf("Success!\n");
      close(sock);
      break;
    } else {
      perror("Failed to connect");
      close(sock);
    }
  }

  /* No address succeeded */
  if (res == NULL) {
    fprintf(stderr, "Could not connect to host\n");
    return 1;
  }

  /* Copy working server */
  server = malloc(sizeof(struct sockaddr_storage));
  memcpy(server, res->ai_addr, sizeof(struct sockaddr_storage));
  server_len = res->ai_addrlen;
  freeaddrinfo(res_list);

  base = event_base_new();
  if (!base) {
    fprintf(stderr, "Couldn't open event base\n");
    return 1;
  }

  /* Connect again, but using libevent, and multiple times. */
  bufevents = malloc(nb_conn * sizeof(struct bufferevent*));
  for (conn = 0; conn < nb_conn; conn++) {
    bufevents[conn] = bufferevent_socket_new(base, -1, 0);
    if (bufevents[conn] == NULL) {
      fprintf(stderr, "Failed to create socket-based bufferevent.\n");
      break;
    }
    ret = bufferevent_socket_connect(bufevents[conn], (struct sockaddr*)server, server_len);
    if (ret != 0) {
      fprintf(stderr, "Failed to connect to host with bufferevent.\n");
      bufferevent_free(bufevents[conn]);
      bufevents[conn] = NULL;
      break;
    }
    bufferevent_setcb(bufevents[conn], readcb, NULL, eventcb, NULL);
    bufferevent_enable(bufevents[conn], EV_READ|EV_WRITE);
    /* Setup a periodic task to send data */
    writeev = event_new(base, -1, EV_PERSIST, writecb, bufevents[conn]);
    ret = event_add(writeev, &timeout);
    if (ret != 0) {
      fprintf(stderr, "Failed to add periodic sending task for connection %d\n", conn);
    }
    /* Progress output */
    if (conn % 500 == 0)
      printf("Opened %d connections so far...\n", conn);

    /* Wait a bit, 1ms means 1000 new connections each second. */
    usleep(1000);
  }
  printf("Opened %d connections to host %s port %s\n", conn, host_s, port_s);

  printf("Starting event loop\n");
  event_base_dispatch(base);

done:
  for (conn = 0; conn < nb_conn; conn++) {
    if (bufevents[conn] == NULL)
      break;
    bufferevent_free(bufevents[conn]);
  }
  free(bufevents);
  event_base_free(base);
  return 0;
}
