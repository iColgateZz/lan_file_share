#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "helpers.c"

#define CONST 8000

int main(void) {

    // char getExtension_test[][CONST] = {
    //     "home/tag/", "home/tag", "home/tag.pdf", "/", "", "lol.pdf/",
    //     "lol.pdf/."
    // };

    // char* getExtension_result[CONST] = {
    //     NULL, NULL, ".pdf", NULL, NULL, NULL, "."
    // };

    // for (int i = 0; i < sizeof(getExtension_test) / CONST; i++) {
    //     printf("Case %d:\n", i + 1);
    //     printf("Testcase: %s\n", getExtension_test[i]);
    //     printf("Expected: %s\n", getExtension_result[i]);
    //     char* str = getExtension(getExtension_test[i]);
    //     if (str) printf("Received: %s\n", str);
    //     else printf("Received: NULL\n");
    // }
    // printf("\n\n\n");

    // char* str = "";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = ".";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "..";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "/";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "/tmp";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "test";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "stringlib.c";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "not_real";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "static";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "static/";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "/static";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // str = "/static/";
    // printf("'%s' isDir: %d\n", str, isdir(str));

    // string uri1 = snew("/simple");
    // printf("%s\n", uri1);
    // printf("%s\n", normalize_uri(uri1));

    // string uri2 = snew("/simple/../lol");
    // printf("%s\n", uri2);
    // printf("%s\n", normalize_uri(uri2));

    // string uri3 = snew("/simple/./././././but/advanced");
    // printf("%s\n", uri3);
    // printf("%s\n", normalize_uri(uri3));

    // string uri4 = snew("does/something/unexpected/../as/intended");
    // printf("%s\n", uri4);
    // printf("%s\n", normalize_uri(uri4));

    // string uri5 = snew("/../../../../LOL");
    // printf("%s\n", uri5);
    // printf("%s\n", normalize_uri(uri5));

    // string uri6 = snew("LOL");
    // printf("%s\n", uri6);
    // printf("%s\n", normalize_uri(uri6));

    // string uri7 = snew("");
    // printf("%s\n", uri7);
    // printf("is NULL: %d\n", normalize_uri(uri7) == NULL);

    // string uri8 = snew("../../../../LOL");
    // printf("%s\n", uri8);
    // printf("%s\n", normalize_uri(uri8));

    char* path = "";
    listdir(path);

    return 0;
}