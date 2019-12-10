#include <openssl/evp.h>
void initialize_key(unsigned char *key, int length);
void fprint_string_as_hex(FILE * f, unsigned char *s, int len);
unsigned char  *allocate_ciphertext(int mlen);
void encrypt_and_print(EVP_CIPHER_CTX * ectx, char *msg, int mlen,
                  char *res, int *olen, FILE * f);
int encryptWithPhrase(char *plaintext, char *file, int size);
