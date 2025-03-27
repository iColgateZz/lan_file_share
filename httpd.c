/* httpd.c */

/* 
    Specs used for the server:
    https://datatracker.ietf.org/doc/html/rfc9110
    https://datatracker.ietf.org/doc/html/rfc9112
    https://developer.mozilla.org/en-US/docs/Web/HTTP
*/

/* C libraries */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/errno.h>
#include <stdbool.h>
#include <ctype.h>

/* Custom libraries */
#include "logger/logger.c"
#include "helpers/helpers.c"
#include "helpers/safe_string.h"
#include "htable/htable.c"

/* Definitions */
#define LISTENADDR              "0.0.0.0"
#define METHOD_SIZE             8
#define URI_SIZE                8000
#define VERSION_SIZE            9
#define MAX_REQUEST_SIZE        16384
#define CHUNK_SIZE              65536
#define SECONDS_TO_WAIT         10
#define SIMPLE_RESPONSE_SIZE    256

#define OK                      200
#define BAD_REQUEST             400
#define NOT_FOUND               404
#define URI_TOO_LONG            414
#define INTERNAL_SERVER_ERROR   500
#define NOT_IMPLEMENTED         501
#define VERSION_NOT_SUPPORTED   505

#define NOTHING_TO_READ         600

#define SET_STATUS(request, code, error) \
    do { \
        request->status_code = code; \
        request->valid = false; \
        error_desc = error; \
    } while (0)

/* Structures */
struct Request 
{
    string method;
    string uri;
    string version;
    bool valid;
    size_t status_code;
};

/* Global state */
char* error_desc;

int init_server(const int port)
{
    int s;
    struct sockaddr_in srv;

    log_info("Initializing the server\n");
    log_info("Opening a socket\n");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        error_desc = "socket() error";
        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);

    log_info("Binding to port %d\n", port);
    if (bind(s, (struct sockaddr*) &srv, sizeof(srv))) {
        close(s);
        error_desc = "bind() error";
        return 0;
    }

    if (listen(s, 1024)) {
        close(s);
        error_desc = "listen() error";
        return 0;
    }
    log_info("Listening on %s:%d\n\n-----------------------------\n\n", LISTENADDR, port);

    return s;
}

int accept_client(const int s, char client_ip[INET_ADDRSTRLEN])
{
    int c;
    struct sockaddr_in cli;
    socklen_t addrlen;

    memset(&cli, 0, sizeof(cli));
    memset(client_ip, 0, INET_ADDRSTRLEN);
    addrlen = sizeof(cli);

    c = accept(s, (struct sockaddr*) &cli, &addrlen);
    if (c < 0) {
        error_desc = "accept() error";
        return 0;
    }

    if (getpeername(c, (struct sockaddr*)&cli, &addrlen) < 0) {
        error_desc = "getpeername() error";
        return 0;
    }
    
    if (inet_ntop(AF_INET, &cli.sin_addr, client_ip, INET_ADDRSTRLEN) == NULL) {
        error_desc = "inet_ntop() error";
        return 0;
    }

    return c;
}

void read_request(const int c, struct Request* request, string buffer)
{
    fd_set rfds;
    ssize_t len = 0;
    struct timeval tv = {.tv_sec = SECONDS_TO_WAIT, .tv_usec = 0};

    FD_ZERO(&rfds);
    FD_SET(c, &rfds);
    int ret = select(c + 1, &rfds, 0, 0, &tv);

    if (ret > 0 && FD_ISSET(c, &rfds)) {
        memset(buffer, 0, MAX_REQUEST_SIZE);
        len = read(c, buffer, MAX_REQUEST_SIZE);
        supdatelen(buffer, (size_t) len);
    }

    if (len <= 0)
        SET_STATUS(request, NOTHING_TO_READ, "Nothing to read\n");
    return;
}

void parse_request_line(struct Request* request, string buffer)
{
    if (!request->valid)
        return;
    slower(request->method = sbite(buffer, 1, " "));
    request->uri = sbite(buffer, 1, " ");
    slower(request->version = sbite(buffer, 2, "\r\n"));
}

void check_request_line(struct Request* request)
{
    if (!request->valid)
        return;
    if (!request->method || !request->uri || !request->version) {
        SET_STATUS(request, BAD_REQUEST, "Method, uri or version is NULL\n");
        return;
    }
    if (strncmp(request->method, "get", 4) != 0) {
        SET_STATUS(request, NOT_IMPLEMENTED, "Unknown method\n");
        return;
    }
    if (strncmp(request->version, "http/1.1", 9) != 0) {
        SET_STATUS(request, VERSION_NOT_SUPPORTED, "Unknown version\n");
        return;
    }
}

void parse_headers(struct Request* request, ht_htable* headers, string buffer)
{
    if (!request->valid)
        return;
    /* No whitespace is allowed between request line and headers. */
    if (isspace(*buffer)) {
        SET_STATUS(request, BAD_REQUEST, "Whitespace between request line and headers\n");
        return;
    }
    /* TODO: Write linear allocator for the project. */
    string key;
    string value;
    while (*buffer != '\r') {
        slower(key = sbite(buffer, 1, ":"));
        if (!key || sendswith(key, 1, " ")) {
            SET_STATUS(request, BAD_REQUEST, "Whitespace after field name\n");
            return;
        }
        strim((value = sbite(buffer, 2, "\r\n")), 2, " \t");
        if (!value) {
            sfree(key);
            SET_STATUS(request, BAD_REQUEST, "Field value is NULL\n");
            return;
        }
        if (ht_search(headers, key)) {
            SET_STATUS(request, BAD_REQUEST, "Duplicate headers\n");
            return;
        }
        ht_insert(headers, key, value);
        sfree(key);
        sfree(value);
    }
}

void check_headers(struct Request* request, ht_htable* headers)
{
    if (!request->valid)
        return;
    if (!ht_search(headers, "host")) {
        SET_STATUS(request, BAD_REQUEST, "No Host field\n");
        return;
    }
    /* The presence of a message body in a request is signaled by a 
       Content-Length or Transfer-Encoding header field. */
    if (ht_search(headers, "content-length") || ht_search(headers, "transfer-encoding")) {
        SET_STATUS(request, BAD_REQUEST, "Body is present\n");
        return;
    }
}

void parse_request(struct Request* request, ht_htable* headers, string buffer)
{
    if (!request->valid)
        return;
    /* Skip initial empty lines if there are any */
    sltrimchar(buffer, 2, "\r\n");

    parse_request_line(request, buffer);
    check_request_line(request);

    parse_headers(request, headers, buffer);
    check_headers(request, headers);

    /* After headers, there must only be '\r\n', nothing else. */
    if (sgetlen(buffer) > 2)
        SET_STATUS(request, BAD_REQUEST, "Malicious payload\n");
}

void send_simple_response(int c, size_t code, char* msg,
                          char* content_type, char* connection, char* body)
{
    char buf[SIMPLE_RESPONSE_SIZE];
    snprintf(buf, SIMPLE_RESPONSE_SIZE,
    "HTTP/1.1 %zu %s\r\n"
    "Server: httpd\r\n"
    "Content-type: %s\r\n"
    "Content-Length: %zu\r\n"
    "Connection: %s\r\n"
    "\r\n"
    "%s",
    code, msg, content_type, strlen(body), connection, body);
    write(c, buf, strlen(buf));
}

void send_file(int c, struct Request* request)
{
    char buf[CHUNK_SIZE];
    char chunk_header[8];
    size_t bytes_read;
    string file_name = request->uri;

    snprintf(buf, CHUNK_SIZE,
    "HTTP/1.1 200 OK\r\n"
    "Server: httpd\r\n"
    "Content-type: %s\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Connection: keep-alive\r\n"
    "\r\n", getconttype(getext(file_name)));

    if (write(c, buf, strlen(buf)) < 0) {
        SET_STATUS(request, INTERNAL_SERVER_ERROR, "Error sending headers\n");
        return;
    }

    FILE *file = fopen(file_name, "rb");
    if (!file) {
        SET_STATUS(request, INTERNAL_SERVER_ERROR, "Can't open file\n");
        return;
    }

    while ((bytes_read = fread(buf, 1, CHUNK_SIZE, file)) > 0)
    {
        snprintf(chunk_header, sizeof(chunk_header), "%zx\r\n", bytes_read);
        if (write(c, chunk_header, strlen(chunk_header)) < 0)
            goto cleanup;

        if (write(c, buf, bytes_read) < 0)
            goto cleanup;

        if (write(c, "\r\n", 2) < 0)
            goto cleanup;
    }
    fclose(file);
    if (write(c, "0\r\n\r\n", 5) < 0)
        SET_STATUS(request, INTERNAL_SERVER_ERROR, "Error writing the final chunk\n");
    return;

cleanup:
    SET_STATUS(request, INTERNAL_SERVER_ERROR, "Error writing chunk data\n");
    fclose(file);
}

void send_template(int c, struct Request* request)
{
    FILE* file = fopen("static/template.html", "rb");
    if (!file) {
        SET_STATUS(request, INTERNAL_SERVER_ERROR, "Can't open template file\n");
        return;
    }
    
    size_t bytes_read;
    char buffer[CHUNK_SIZE];
    char chunk_header[8];
    string template = snew("");
    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, file)) > 0)
    {
        string buf = snewlen(buffer, bytes_read);
        string tmp = template;
        template = scats(template, buf);
        sfree(buf); sfree(tmp);
    }
    
    size_t n;
    string* arr = ssplit(template, 6, "#TITLE", &n);
    string listing = snew("Listing of /");
    string title = scats(listing, request->uri);
    string with_title = sjoins(n, arr, sgetlen(title), title);
    sfreearr(arr, n); sfree(listing); sfree(title);

    arr = ssplit(with_title, 8, "#LISTING", &n);
    sfree(with_title);

    string li = snew("<li><a href=\"/PATH\">PATH</a></li>\n");
    string* li_arr = ssplit(li, 4, "PATH", &n);
    string* dir_file = listdir(request->uri, &n);

    string links = snew("");
    for (size_t i = 0; i < n; i++) {
        string link = sjoins(3, li_arr, sgetlen(dir_file[i]), dir_file[i]);
        string tmp = links;
        links = scats(links, link);
        sfree(link); sfree(tmp);
    }
    string to_return = sjoins(2, arr, sgetlen(links), links);
    sfree(li); sfreearr(li_arr, 3); sfreearr(dir_file, n); sfree(links);
    
    snprintf(buffer, CHUNK_SIZE,
    "HTTP/1.1 200 OK\r\n"
    "Server: httpd\r\n"
    "Content-type: text/html\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Connection: keep-alive\r\n"
    "\r\n");
    write(c, buffer, strlen(buffer));
    size_t len = sgetlen(to_return);
    n = 0;
    while (n < len)
    {
        bytes_read = len - n >= CHUNK_SIZE ? CHUNK_SIZE : len;
        snprintf(chunk_header, sizeof(chunk_header), "%zx\r\n", bytes_read);
        write(c, chunk_header, strlen(chunk_header));
        write(c, to_return + n, bytes_read);
        write(c, "\r\n", 2);
        n += len;
    }
    write(c, "0\r\n\r\n", 5);
}

void check_uri(struct Request* request)
{
    if (!request->valid)
        return;
    else if (!request->uri)
        SET_STATUS(request, INTERNAL_SERVER_ERROR, "uri is NULL\n");
    else if (isdir(request->uri) == ISDIR_INVALID)
        SET_STATUS(request, NOT_FOUND, "Resource not found\n");
}

bool respond(int c, struct Request* request)
{
    if (request->status_code == NOTHING_TO_READ)
        return false;
    
    request->uri = normalize_uri(request->uri);
    check_uri(request);

    if (!request->valid)
        send_simple_response(c, request->status_code, "", "text/plain", "close", "");
    else {
        if (isdir(request->uri))
            send_template(c, request);
        else
            send_file(c, request);
    }
    return request->valid;
}

void handle_client(const int c) 
{
    struct Request request;
    ht_htable* headers = ht_new();
    string buffer = snewlen(NULL, MAX_REQUEST_SIZE);
    // Maybe add a check that allocation was successful.

    bool keep_alive = true;
    while (keep_alive) 
    {
        request.valid = true;
        request.status_code = OK;

        read_request(c, &request, buffer);
        parse_request(&request, headers, buffer);
        printf("Request vals:\nMethod: %s\nUri: %s\nVersion: %s\nCode: %zu\n\n", 
                request.method, request.uri, request.version, request.status_code);
        keep_alive = respond(c, &request);
        log_err(stderr, error_desc);
        if (request.status_code != NOTHING_TO_READ) {
            ht_clear(headers);
            sfree(request.method); sfree(request.uri); sfree(request.version);
        }
    }

    sfree(buffer);
    ht_del_htable(headers);
    return;
}

/* Start the main server process, spawn child processes for clients. */
int main(int argc, char* argv[]) 
{
    int s, c, f;
    char* port;
    char client_ip[INET_ADDRSTRLEN];

    if (argc < 2) {
        log_err(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    port = argv[1];
    s = init_server(atoi(port));
    if (!s) {
        log_err(stderr, "%s: %d\n", error_desc, errno);
        return -1;
    }

    while (1) 
    {
        c = accept_client(s, client_ip);
        if (!c) {
            log_warn("%s: %d\n", error_desc, errno);
            continue;
        }

        /* If > 2000 calls then fork EAGAIN error (35). */
        f = fork();
        if (f == 0) {
            log_success("Client connected: %s\n", client_ip);
            handle_client(c);
            log_success("Client disconnected: %s\n", client_ip);

            close(c);
            close(s);
            exit(0);
        } 
        else if (f == -1) 
            log_warn("Fork() error: %d\n", errno);
        close(c);
    }

    close(s);
    return -1;
}
