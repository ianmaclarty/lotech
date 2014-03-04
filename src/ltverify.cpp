#include "lt.h"

#ifndef LTVERIFYMODULES
#define LTVERIFYMODULES {NULL}
#endif

#ifndef LTVERIFYDIGEST
#define LTVERIFYDIGEST {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#endif

#ifndef LTVERIFYSECRET
#define LTVERIFYSECRET {1, 0, 0, 0, 0, 0, 0, 0}
#endif

const char    *lt_verify_modules[] = LTVERIFYMODULES;
LTSHA1Digest   lt_verify_digest  = LTVERIFYDIGEST;
int            lt_verify_secret[] = LTVERIFYSECRET;

void ltDoVerify() {
    if (lt_verify_modules == NULL || lt_verify_modules[0] == NULL) return;

    int n = 0;
    while (lt_verify_modules[n] != NULL) {
        n++;
    }

    LTSHA1Digest digest = ltSHA1_lua_modules(lt_verify_modules, n);

#ifdef LTDEVMODE
    printf("SHA1 digest = ");
    digest.print_c();
#endif

    bool eq = true;
    for (int i = 0; i < 20; i++) {
        if (digest.digest[i] != lt_verify_digest.digest[i]) {
            eq = false;
            break;
        }
    }
    if (!eq) {
#ifdef LTDEVMODE
        printf("SHA1 validation failed!\n");
        lt_verify_secret[0]++;
#else
        for (int i = 0; i < 8; i++) {
            lt_verify_secret[i] = 0;
        }
#endif
    } else {
#ifdef LTDEVMODE
        printf("SHA1 validation succeeded\n");
#endif
        lt_verify_secret[0]++;
    }
}

char *ltSecret(const char *txt) {
    int l = strlen(txt) + 200;
    char *tmp = (char*)malloc(l);
    snprintf(tmp, l, "%s{%d,%d,%d,%d,%d,%d,%d,%d}",
        txt,
        lt_verify_secret[0],
        lt_verify_secret[1],
        lt_verify_secret[2],
        lt_verify_secret[3],
        lt_verify_secret[4],
        lt_verify_secret[5],
        lt_verify_secret[6],
        lt_verify_secret[7]);
    LTSHA1Digest digest = ltSHA1(tmp, strlen(tmp));
    free(tmp);
    return digest.tostr();
}
