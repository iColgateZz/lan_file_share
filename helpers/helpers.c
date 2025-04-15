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
#include <dirent.h>

#define MAX_PATH_LEN            8000
#define MAX_DIR_SIZE            1024
#define CHUNK_SIZE              65536
#define ISDIR_INVALID           -1

string normalize_uri(string uri)
{
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
    sfreearr(in, n); free(out); sfree(uri);
    strim(new_uri, 1, "/");
    return new_uri;
}

char* getext(const char* str)
{
    char* lastSlash = strrchr(str, '/');
    if (lastSlash == NULL)
        return strrchr(str, '.');
    else
        return strrchr(lastSlash, '.');
}

int isdir(const char *path)
{
    char buf[MAX_PATH_LEN];
    buf[MAX_PATH_LEN - 1] = 0;
    snprintf(buf, MAX_PATH_LEN - 1, "./%s", path);

    struct stat statbuf;
    if (stat(buf, &statbuf) != 0)
        return ISDIR_INVALID;
    return S_ISDIR(statbuf.st_mode);
}

char* getconttype(const char* ext)
{
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
    } else if (strcmp(ext, ".c") == 0 || strcmp(ext, ".py") == 0 || 
               strcmp(ext, ".sh") == 0 || strcmp(ext, ".h") == 0) {
        return "text/plain";
    }
    return "application/octet-stream";
}

string* listdir(const char* path, size_t* n)
{
    struct dirent *de;
    char buf[MAX_PATH_LEN];

    buf[MAX_PATH_LEN - 1] = 0;
    snprintf(buf, MAX_PATH_LEN - 1, "./%s", path);

    DIR *dr = opendir(buf);
    if (!dr) return NULL; 

    string* arr = malloc(MAX_DIR_SIZE * sizeof(string));
    if (!arr) return NULL;

    size_t idx = 0;
    while ((de = readdir(dr)) && idx < MAX_DIR_SIZE)
    {
        if (de->d_type == DT_DIR || de->d_type == DT_REG) {
            arr[idx] = snew(de->d_name);
            if (!arr[idx]) goto cleanup;
            idx++;
        }
    }
    closedir(dr);
    *n = idx;
    return arr;

cleanup:
    for (size_t i = 0; i < idx; i++)
        sfree(arr[i]);
    free(arr);
    return NULL;
}

string read_file(const char* file_name)
{
    size_t bytes_read;
    string buf = snewlen(NULL, CHUNK_SIZE);
    if (!buf) return NULL;

    FILE* file = fopen(file_name, "rb");
    if (!file) return NULL;

    string ret = snewlen("", 0);
    while ((bytes_read = fread(buf, 1, CHUNK_SIZE, file)) > 0)
        ret = scat(ret, bytes_read, buf);

    sfree(buf);
    fclose(file);
    return ret;
}

bool send_chunked_file(int c, string buf)
{
    size_t bytes_read;
    char chunk_header[20];
    size_t len = sgetlen(buf);
    size_t n = 0;
    while (n < len)
    {
        bytes_read = len - n >= CHUNK_SIZE ? CHUNK_SIZE : len - n;
        snprintf(chunk_header, sizeof(chunk_header), "%zx\r\n", bytes_read);
        if (write(c, chunk_header, strlen(chunk_header)) < 0)
            return false;
        if (write(c, buf + n, bytes_read) < 0)
            return false;
        if (write(c, "\r\n", 2) < 0)
            return false;
        n += bytes_read;
    }
    if (write(c, "0\r\n\r\n", 5) < 0)
        return false;
    return true;
}

#endif
