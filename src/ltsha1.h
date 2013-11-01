struct LTSHA1Digest {
    char digest[20];

    void print_c();
};

char* ltSHA1(const char *data, size_t len);
LTSHA1Digest ltSHA1_lua_modules(const char **modules, int num_modules);
