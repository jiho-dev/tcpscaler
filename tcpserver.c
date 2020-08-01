#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>

int conn_cnt=0;
int event_read_cnt=0;
int per_msg = 1000;

#define MAX_OPENFILES_DEFAULT 1024 * 1024
#define MAX_OPENFILES_TARGET  1024 * 1024 * 256

static void readcb(struct bufferevent *bev, void *ctx)
{
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);

    event_read_cnt ++;

    if (event_read_cnt % per_msg == 0) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[128];
        strftime(s, sizeof(s), "%c", tm);

        printf("%s: Revc Msg cnt=%d \n", s, event_read_cnt);
    }

    /* Copy all the data from the input buffer to the output buffer. */
    evbuffer_add_buffer(output, input);
}

static void eventcb(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");

    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        conn_cnt --;
        bufferevent_free(bev);

        printf("Close socket: conn_cnt=%d \n", conn_cnt);
    }
}

static void accept_conn_cb(struct evconnlistener *listener,
                           evutil_socket_t fd, struct sockaddr *address,
                           int socklen, void *ctx)
{
    char host[NI_MAXHOST];
    char port[NI_MAXSERV];

    if (address->sa_family==AF_INET6) {
        struct sockaddr_in6* addr6=(struct sockaddr_in6*)address;
        if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr)) {
            struct sockaddr_in addr4;

            memset(&addr4,0,sizeof(addr4));
            addr4.sin_family=AF_INET;
            addr4.sin_port=addr6->sin6_port;

            memcpy(&addr4.sin_addr.s_addr,addr6->sin6_addr.s6_addr+12,sizeof(addr4.sin_addr.s_addr));
            memcpy(address,&addr4,sizeof(addr4));
        }
    }

    getnameinfo(address, socklen, host, NI_MAXHOST, port, NI_MAXSERV,
                NI_NUMERICHOST | NI_NUMERICSERV);

    conn_cnt ++;

    struct sockaddr_in myaddr;
    socklen_t client_len = sizeof(myaddr);
    getsockname(fd, (struct sockaddr *)&myaddr, &client_len);

    printf("New connection: conn_cnt=%d %s:%s -> %s:%d\n", 
           conn_cnt, 
           host, port,
           inet_ntoa(myaddr.sin_addr),
           ntohs(myaddr.sin_port));

    /* Setup a bufferevent */
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, eventcb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

    static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

void usage(char* progname) {
    fprintf(stderr, "usage: %s -p <port>  [-P last_port] [-v]\n", progname);
    fprintf(stderr, "-p: Listen port, default 4242 \n");
    fprintf(stderr, "-P: The last listen port in range, default 4242.  \n");
    fprintf(stderr, "-m: Print out per received messages \n");
    fprintf(stderr, "-v: Verbose \n");

}

int main(int argc, char** argv)
{
    struct event_base *base;
    struct evconnlistener *listener;
    //struct sockaddr_in6 sin;
    struct sockaddr_in sin;
    struct rlimit limit_openfiles;
    FILE *nr_open;
    int ret, i;
    int port = 4242;
    int max_port = 4242;
    int verbose = 0;

    /*
    if (argc > 1) {
    port = atoi(argv[1]);
    }
    if (port <= 0 || port > 65535) {
    fprintf(stderr, "Invalid port\n");
    return 1;
    }
    */

    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, "p:P:vm:")) != -1) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            max_port = port;
            break;
        case 'P':
            max_port = atoi(optarg);
            break;
        case 'm':
            per_msg = atoi(optarg);
            break;
        case 'v':
            verbose ++;
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    /* Setup limit on number of open files. */
    /* First, set soft limit to hard limit */
    ret = getrlimit(RLIMIT_NOFILE, &limit_openfiles);
    if (ret != 0) {
        perror("Failed to get limit on number of open files");
        return 1;
    }
    limit_openfiles.rlim_cur = limit_openfiles.rlim_max;
    ret = setrlimit(RLIMIT_NOFILE, &limit_openfiles);

    /* Then try to increase the limit to the maximum (needs to be root) */
    limit_openfiles.rlim_cur = MAX_OPENFILES_DEFAULT;
    limit_openfiles.rlim_max = MAX_OPENFILES_DEFAULT;
    ret = setrlimit(RLIMIT_NOFILE, &limit_openfiles);
    if (ret != 0) {
        perror("Failed to increase limit on number of open files to MAX_OPENFILES_DEFAULT");
        fprintf(stderr, "Try to run this program as root.\n");
    } else {
        /* Try to increase the limit even further. */
        nr_open = fopen("/proc/sys/fs/nr_open", "w");
        if (nr_open == NULL) {
            perror("Failed to open /proc/sys/fs/nr_open for writing");
        } else {
            fprintf(nr_open, "%d\n", MAX_OPENFILES_TARGET);
            fclose(nr_open);
        }
        limit_openfiles.rlim_cur = MAX_OPENFILES_TARGET;
        limit_openfiles.rlim_max = MAX_OPENFILES_TARGET;
        ret = setrlimit(RLIMIT_NOFILE, &limit_openfiles);
        if (ret != 0) {
            perror("Failed to increase limit on number of open files to MAX_OPENFILES_TARGET");
        }
    }

    /* Display final limit */
    ret = getrlimit(RLIMIT_NOFILE, &limit_openfiles);
    if (ret != 0) {
        perror("Failed to get limit on number of open files");
        return 1;
    }
    printf("Maximum number of TCP clients: %ld\n", limit_openfiles.rlim_cur);

    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Couldn't open event base\n");
        return 1;
    }

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));

    //sin.sin6_family = AF_INET6;
    /* Listen on the given port, on :: */
    //sin.sin6_port = htons(port);

    for (i = port; i < max_port+1; i++) {
        sin.sin_family = AF_INET;
        //sin.sin_port = htons(port);
        sin.sin_port = htons(i);

        listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                           LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, 8192,
                                           (struct sockaddr*)&sin, sizeof(sin));
        if (!listener) {
            perror("Couldn't create listener");
            return 1;
        }

        char l_host[NI_MAXHOST];
        char l_port[NI_MAXSERV];
        getnameinfo((struct sockaddr*)&sin, sizeof(sin), l_host, NI_MAXHOST,
                    l_port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        printf("Listening on %s port %s\n", l_host, l_port);
        evconnlistener_set_error_cb(listener, accept_error_cb);
    }

    return event_base_dispatch(base);
}
