#include "math.h"

double nan(const char *tagp) {
    return (double)(NAN);
}

float nanf(const char *tagp) {
    return (float)(NAN);
}

long double nanl(const char *tagp) {
    return (long double)(NAN);
}
