#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdarg.h>
#include <syslog.h>


/* Copied from babeld by Juliusz Chroboczek */
#define DO_NTOHS(_d, _s) \
    do { unsigned short _dd; \
         memcpy(&(_dd), (_s), 2); \
         _d = ntohs(_dd); } while(0)
#define DO_NTOHL(_d, _s) \
    do { unsigned int _dd; \
         memcpy(&(_dd), (_s), 4); \
         _d = ntohl(_dd); } while(0)
#define DO_HTONS(_d, _s) \
    do { unsigned short _dd; \
         _dd = htons(_s); \
         memcpy((_d), &(_dd), 2); } while(0)
#define DO_HTONL(_d, _s) \
    do { unsigned _dd; \
         _dd = htonl(_s); \
         memcpy((_d), &(_dd), 4); } while(0)

extern FILE* logfile;
extern int use_syslog;
void log_print(FILE *file, int use_syslog, int code, const char *fmt, ...);

#define error(fmt, ARGS...) \
            do {  \
                log_print(logfile, use_syslog, LOG_ERR, fmt, ## ARGS); \
            } while (0)

#define info(fmt, ARGS...) \
            do {  \
                if (verbose >= 1) { log_print(logfile, use_syslog, LOG_INFO, fmt, ## ARGS); } \
            } while (0)

#define debug(fmt, ARGS...) \
            do {  \
                if (verbose >= 2) { log_print(logfile, use_syslog, LOG_DEBUG, fmt, ## ARGS); } \
            } while (0)

/* Returns the integer that is closest to a/b */
static inline int divide_closest(int a, int b)
{
  int ret = a / b;
  /* Works for both positive and negative numbers */
  if (2 * (a % b) >= b)
    ret += 1;
  return ret;
}

static inline void log_date_time(char *buffer)
{
    int millisec;
    struct tm* tm_info;
    struct timeval tv;
    char tbuf[64];

    gettimeofday(&tv, NULL);

    //millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    millisec = tv.tv_usec/1000; // Round to nearest millisec
    if (millisec>=1000) { // Allow for rounding up to nearest second
        millisec -=1000;
        tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec);

    strftime(tbuf, 64, "%Y:%m:%d %H:%M:%S", tm_info);
    sprintf(buffer, "%s.%03d", tbuf, millisec);
}

/*
static inline void log_print(FILE *file, int use_syslog, int code, const char *fmt, ...)
{
    char *msg = "";

    if (file == NULL) {
        file = stdout;
    }

    switch (code) {
    case LOG_ERR:
        msg = "ERR";
        break;
    case LOG_INFO:
        msg = "INFO";
        break;
    case LOG_DEBUG:
        msg = "DEBUG";
        break;
    }

    char buffer[2014];

    if (use_syslog) {
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buffer, fmt, ap);
        va_end(ap);

        syslog(code|LOG_LOCAL0, buffer);

    } else {
        log_date_time(buffer); 
        fprintf(file, "%s [%s] ", buffer, msg);

        va_list ap;
        va_start(ap, fmt);
        vfprintf(file, fmt, ap);
        va_end(ap);

        fputc('\n',file);
        fflush(file);
    }
}
*/

void subtract_timespec(struct timespec *result, const struct timespec *a, const struct timespec *b);

void timeval_add_ms(struct timeval *a, unsigned int ms);

void timeval_add_us(struct timeval *a, unsigned long int us);

/* Given a [rate], generate an interarrival sample according to a Poisson
   process and store it in [tv]. */
void generate_poisson_interarrival(struct timeval* tv, double rate);
