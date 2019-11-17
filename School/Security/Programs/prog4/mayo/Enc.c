/*========================================================================
  Portions of this code taken from:

          Building Secure Software, by John Viega and Gary McGraw

  Code only demonstrates basic functions required for the project.  It is
  not secure.  Return codes are given in the manual pages.

  Functionality: 
  (1) Get passphrase from user
  (2) Run MD5 sum over the passphrase (represents key encryption key) 
  (3) Generate random bytes to encrypt (represents file encryption key)
  (4) Encrypt the file encryption key (random bytes) with the
      key encryption key (hashed passphrase) and write to file named
      as command line argument
  
  ========================================================================
*/

#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define MD5LENGTH  16
#define FILEKEYLEN 16
#define BLOCKSIZE   8

void 
initialize_key(unsigned char *key, int length)
{
	FILE           *rng;
	int             num = 0;
	rng = fopen("/dev/urandom", "r");

	while (num < length) {
		num += fread(&key[num], 1, length - num, rng);
	}

	fclose(rng);
}

void 
fprint_string_as_hex(FILE * f, unsigned char *s, int len)
{
	int             i;
	for (i = 0; i < len; i++) {
		fprintf(f, "%02x", s[i]);
	}
}

unsigned char  *
allocate_ciphertext(int mlen)
{
	/* Alloc enough space for any possible padding. */
	return (unsigned char *)malloc(mlen+BLOCKSIZE);
}

void 
encrypt_and_print(EVP_CIPHER_CTX * ectx, char *msg, int mlen,
		  char *res, int *olen, FILE * f)
{
	int             extlen;

	EVP_EncryptUpdate(ectx, res, olen, msg, mlen);
	EVP_EncryptFinal_ex(ectx, &res[*olen], &extlen);

	*olen += extlen;

	fprintf(f, "Encrypted result <");
	fprint_string_as_hex(f, res, *olen);
	fprintf(f, ">\n");
}

int 
main(int argc, char **argv)
{
        EVP_CIPHER_CTX  *ctx;                 /* Context for encryption */
	EVP_CIPHER      *cipher;              /* Cipher */
	unsigned char   ekey[MD5LENGTH];      /* Key to encipher file encryption key */
	char            ivec[EVP_MAX_IV_LENGTH] = {0}; 

        unsigned char   fkey[FILEKEYLEN];     /* File encryption key (currently unused) */
        int             mlen;                 /* Length of message to encrypt */

	unsigned char  *ciphertext;
	int             ctlen;                /* Length of ciphertext */

	unsigned char   phrase[100];          /* Passphrase */
	EVP_MD_CTX      *mdctx;               /* Context for MD5 sum */
	int             mdlen;                /* Length of MD5 sum */

        int fd;                               /* Will write ciphertext to this file */

        /* Get the passphrase */ 
        printf("Input passphrase: ");
        gets(phrase);

        /* Compute the digest */
        mdctx=(EVP_MD_CTX *)malloc(sizeof(EVP_MD_CTX));
        EVP_MD_CTX_init(mdctx);
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	EVP_DigestUpdate(mdctx, phrase, strlen(phrase));
	EVP_DigestFinal_ex(mdctx, ekey, &mdlen);
        if (mdlen != MD5LENGTH) exit(1);

	fprintf(stdout, "Digest of: <%s> \nis: <", phrase);
	fprint_string_as_hex(stdout, ekey, mdlen);
	fprintf(stdout, ">\n");

	/* The data to encrypt */
        initialize_key(fkey,FILEKEYLEN);
        fprintf(stdout,"File encryption key <");
	fprint_string_as_hex(stdout, fkey, FILEKEYLEN);
        fprintf(stdout,">\n");

        /* Encrypt it */
        ctx = (EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
	EVP_CIPHER_CTX_init(ctx);
	cipher = (EVP_CIPHER *)EVP_bf_cbc();
	EVP_EncryptInit_ex(ctx, cipher, NULL, ekey, ivec);
        mlen=FILEKEYLEN;
	ciphertext = allocate_ciphertext(mlen);
	encrypt_and_print(ctx, fkey, mlen, 
                          ciphertext, &ctlen, stdout);

        /* Write to file */
        fd=open(argv[1],O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
        write(fd,ciphertext,ctlen);
	close(fd);
}
