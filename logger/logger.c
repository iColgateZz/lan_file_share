#ifndef HTTPD_LOGGER
#define HTTPD_LOGGER

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static inline
void get_cur_time(char* time_buffer, size_t size)
{
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);
    strftime(time_buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void log_info(char* msg, ...) {
    va_list ap;

    char time_buffer[26];
    get_cur_time(time_buffer, sizeof(time_buffer));

    printf("[%s] ", time_buffer);
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    va_end(ap);
}

void log_err(FILE* f, char* msg, ...) {
    va_list ap;
    
    char time_buffer[26];
    get_cur_time(time_buffer, sizeof(time_buffer));

    fprintf(f, "[%s] ", time_buffer);
    va_start(ap, msg);
    vfprintf(f, msg, ap);
    va_end(ap);
}
#endif
