extern const char    *lt_verify_modules[];
extern LTSHA1Digest   lt_verify_digest;
extern int            lt_verify_secret[];

void ltDoVerify();
char *ltSecret(const char *txt);
