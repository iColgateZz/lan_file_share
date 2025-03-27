#ifndef HTTPD_LOGGER
#define HTTPD_LOGGER

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_RESET_ALL          "\x1b[0m"

void log_info(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
}

void log_success(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    printf(ANSI_COLOR_GREEN);
    vprintf(msg, ap);
    printf(ANSI_RESET_ALL);
    va_end(ap);
}

void log_warn(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    printf(ANSI_COLOR_YELLOW);
    vprintf(msg, ap);
    printf(ANSI_RESET_ALL);
    va_end(ap);
}

void log_err(FILE* f, char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    fprintf(f, ANSI_COLOR_RED);
    vfprintf(f, msg, ap);
    fprintf(f, ANSI_RESET_ALL);
    va_end(ap);
}
#endif
