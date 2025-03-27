#ifndef HTTPD_STRINGLIB
#define HTTPD_STRINGLIB

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "safe_string.h"
#include <stdlib.h>

string normalize_uri(string uri) {
    if (uri[0] == 0) {
        sfree(uri);
        return NULL;
    }

    size_t n;
    string* in = ssplit(uri, 1, "/", &n);
    if (!in) {
        sfree(uri);
        return NULL;
    }

    string* out = malloc(n * sizeof(string));
    if (!out) {
        sfree(uri);
        sfreearr(in, n);
        return NULL;
    }

    size_t out_size = 0;
    bool was_empty = false;
    for (size_t i = 0; i < n; i++)
    {
        string cur = in[i];
        if (cur[0] == 0) {
            out[out_size++] = cur;
            was_empty = true;
            continue;
        }
        if (strncmp(cur, "..", 3) == 0) {
            if (out_size == 0 || was_empty) {
                continue;
            }
            out_size--;
        } else if (strncmp(cur, ".", 2) == 0) {
            continue;
        } else {
            out[out_size++] = cur;
            was_empty = false;
        }
    }
    string new_uri = sjoins(out_size, out, 1, "/");
    sfreearr(in, n);
    free(out);
    sfree(uri);
    sltrimchar(new_uri, 1, "/");
    return new_uri;
}

char* getext(const char* str) {
    char* lastSlash = strrchr(str, '/');
    if (lastSlash == NULL)
        return strrchr(str, '.');
    else
        return strrchr(lastSlash, '.');
}

#define ISDIR_INVALID -1
int isdir(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return ISDIR_INVALID;
    return S_ISDIR(statbuf.st_mode);
}

char* getcontype(const char* ext) {
    if (!ext) {
        return "application/octet-stream";
    }
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(ext, ".css") == 0) {
        return "text/css";
    } else if (strcmp(ext, ".js") == 0) {
        return "application/javascript";
    } else if (strcmp(ext, ".json") == 0) {
        return "application/json";
    } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(ext, ".png") == 0) {
        return "image/png";
    } else if (strcmp(ext, ".gif") == 0) {
        return "image/gif";
    } else if (strcmp(ext, ".svg") == 0) {
        return "image/svg+xml";
    } else if (strcmp(ext, ".txt") == 0) {
        return "text/plain";
    } else if (strcmp(ext, ".pdf") == 0) {
        return "application/pdf";
    }
    return "application/octet-stream";
}
#endif
