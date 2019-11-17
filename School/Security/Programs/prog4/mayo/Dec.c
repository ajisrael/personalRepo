/*========================================================================
  Portions of this code taken from:

          Building Secure Software, by John Viega and Gary McGraw

  Code only demonstrates basic functions required for the project.  It is
  not secure.  Return codes are given in the manual pages.
  ========================================================================
*/
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define MD5LENGTH   16

void 
fprint_string_as_hex(FILE * f, unsigned char *s, int len)
{
	int             i;
	for (i = 0; i < len; i++) {
		fprintf(f, "%02x", s[i]);
	}
}

void 
fprint_key(FILE * f, unsigned char *key, int len)
{
	fprintf(f, "Key: ");
	fprint_string_as_hex(f, key, len);
	fprintf(f, "\n");
}

void 
decrypt_and_print(EVP_CIPHER_CTX * dctx, char *ct, int ctlen,
		  FILE * f)
{
	int             outlen;
	unsigned char  *res = (unsigned char *)malloc(ctlen);
        int             nbytes=0;

	EVP_DecryptUpdate(dctx, res, &outlen, ct, ctlen);
        nbytes+=outlen;
	EVP_DecryptFinal_ex(dctx, &res[outlen], &outlen);
        nbytes+=outlen;


	fprintf(f, "Decrypted result: <");	
        fprint_string_as_hex(stdout, res, nbytes);
        fprintf(f,"> \n");
	free(res);
}


int 
main(int argc, char **argv)
{
        EVP_CIPHER_CTX  *ctx;              /* Context for decryption */
	EVP_CIPHER     *cipher;            /* Cipher for decryption */
	unsigned char   key[MD5LENGTH];    /* Decryption key */
	char            ivec[EVP_MAX_IV_LENGTH] = {0};
	unsigned char  *ciphertext;        /* Ciphertext buffer */
	int             ctlen;             /* Length of ciphertext */

        unsigned char   phrase[100];       /* Passphrase */
	EVP_MD_CTX      *mdctx;            /* Context for MD5 sum */
	int             mdlen;             /* Length of MD5 sum */


        struct stat     fstats;            /* Stats on ciphertext file */
        int fd;                            /* Descriptor for ciphertext file */

        /* Get the ciphertext */
	fd=open(argv[1],O_RDWR);

	fstat(fd,&fstats);
        ciphertext=malloc(fstats.st_size);
        read(fd,ciphertext,fstats.st_size);
        fprintf(stdout,"Ciphertext: <");
        fprint_string_as_hex(stdout,ciphertext,fstats.st_size);
	fprintf(stdout,">\n");

        /* Get the passphrase */ 
        printf("Input passphrase: ");
        gets(phrase);

        /* Compute MD5 sum/ key */
        mdctx=(EVP_MD_CTX *)malloc(sizeof(EVP_MD_CTX));
	EVP_MD_CTX_init(mdctx);
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	EVP_DigestUpdate(mdctx, phrase, strlen(phrase));
	EVP_DigestFinal_ex(mdctx, key, &mdlen);

	fprintf(stdout, "Digest of <%s> \nis: <", phrase);
	fprint_string_as_hex(stdout, key, mdlen);
	fprintf(stdout, ">\n");

        /* Decrypt */
        ctx=(EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        cipher=(EVP_CIPHER *)EVP_bf_cbc();
        ctlen=fstats.st_size;       
        EVP_DecryptInit_ex(ctx, cipher, NULL, key, ivec);
	decrypt_and_print(ctx, ciphertext, ctlen, stdout);

	free(ciphertext);

	return 0;
}
