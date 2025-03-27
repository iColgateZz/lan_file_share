// prime.c
#ifndef HTTPD_PRIME
#define HTTPD_PRIME

#include <math.h>


/* Return whether x is prime or not.
   -1 if undefined ( < 2)
   1 if it is
   0 if not 
*/
int is_prime(const int x) {
    if (x < 2) { return -1; }
    if (x < 4) { return 1; }
    if ((x % 2) == 0) { return 0; }
    for (int i = 3; i <= floor(sqrt((double) x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

/* Find next prime after x or return x if x is prime. */
int next_prime(int x) {
    while (is_prime(x) != 1) {
        x++;
    }
    return x;
}
#endif
