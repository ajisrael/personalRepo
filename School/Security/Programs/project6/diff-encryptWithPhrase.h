5,6c5,7
< void encrypt_and_print(EVP_CIPHER_CTX * ectx, char *msg, int mlen,
<                   char *res, int *olen, FILE * f);
---
> /// CHANGE: params take in unsigned char * and returns int for error checking
> int encrypt_and_print(EVP_CIPHER_CTX * ectx, unsigned char *msg, int mlen,
>                   unsigned char *res, int *olen, FILE * f);
