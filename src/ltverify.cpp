#include "lt.h"

#ifndef LTVERIFYMODULES
#define LTVERIFYMODULES NULL
#endif

#ifndef LTVERIFYDIGEST
#define LTVERIFYDIGEST {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#endif

#ifndef LTVERIFYSECRET
#define LTVERIFYSECRET {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif

const char    *lt_verify_modules[] = LTVERIFYMODULES;
LTSHA1Digest   lt_verify_digest  = LTVERIFYDIGEST;
int            lt_verify_secret[] = LTVERIFYSECRET;

void ltDoVerify() {
    if (lt_verify_modules == NULL) return;

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
#else
        for (int i = 0; i < 20; i++) {
            lt_verify_secret[i] = i;
        }
#endif
    } else {
#ifdef LTDEVMODE
        printf("SHA1 validation succeeded\n");
#endif
    }
}
