struct LTSHA1Digest {
    unsigned char digest[20];

    void print_c();
    char *tostr();
};

LTSHA1Digest ltSHA1(const char *data, size_t len);
LTSHA1Digest ltSHA1_lua_modules(const char **modules, int num_modules);
