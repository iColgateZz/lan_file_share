/* safe_stringing.h */

/* Include libs */
#include <stdint.h>
#include <stdbool.h>

/* Definitions */
#ifndef SAFE_STRING_H
#define SAFE_STRING_H

typedef char* string;

typedef struct Header8 {
    uint8_t len;
    uint8_t allocated;
    uint8_t type;
    char buf[];
} Header8;

typedef struct Header16 {
    uint16_t len;
    uint16_t allocated;
    uint8_t type;
    char buf[];
} Header16;

typedef struct Header32 {
    uint32_t len;
    uint32_t allocated;
    uint8_t type;
    char buf[];
} Header32;

typedef struct Header64 {
    uint64_t len;
    uint64_t allocated;
    uint8_t type;
    char buf[];
} Header64;

/* Exposed functions */

string snew(const void* input);
string snewlen(const void* input, size_t ilen);
void sfree(const string s);
size_t sgetlen(const string s);
void supdatelen(const string s, size_t len);
string sdup(const string s);
string sjoin(size_t n, const char* s[n], size_t plen, const char* pattern);
string sjoins(size_t n, const string s[n], size_t plen, const char* pattern);
string scatc(const char* s1, const char* s2);
string scats(const string s1, const string s2);
string scat(string s, size_t cstr_len, char* cstr);
bool slower(string s);
bool supper(string s);
bool sstartswith(string s, size_t plen, const char* pattern);
bool sendswith(string s, size_t plen, const char* pattern);
ssize_t sfind(string s, size_t plen, const char* pattern);
ssize_t srfind(string s, size_t plen, const char* pattern);
ssize_t scount(string s, size_t plen, const char* pattern);
bool strim(string s, size_t plen, const char* pattern);
bool sremove(string s, size_t plen, const char* pattern);
string sslice(string s, size_t start, size_t end);
string sbite(string s, size_t plen, const char* pattern);
ssize_t sfind_advanced(string s, size_t plen, const char* pattern);
string* ssplit(const string s, size_t seplen, const char* sep, size_t* n);
void sfreearr(string* arr, size_t n);
bool sltrimchar(string s, size_t c_size, char* c_arr);
string sreplace(const string s, size_t olen, const char* old, size_t nlen, const char* new);

#endif 
