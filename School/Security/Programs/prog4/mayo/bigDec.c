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

#define MAXKEYLEN 56

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
        int             i;

	EVP_DecryptUpdate(dctx, res, &outlen, ct, ctlen);
        nbytes+=outlen;
	EVP_DecryptFinal_ex(dctx, &res[outlen], &outlen);
        nbytes+=outlen;


	fprintf(f, "Decrypted result: <");	
        fprint_string_as_hex(stdout, res, nbytes);
        fprintf(f,"> \n");

        /* Assumes we're decrypting ASCII plaintext */
        printf("Decrypted result in ASCII: \n<");
        for (i=0;i<nbytes;i++) printf("%c",res[i]);
        printf(">\n");
	free(res);
}


int 
main(int argc, char **argv)
{
        EVP_CIPHER_CTX  *ctx;              /* Context for decryption */
	EVP_CIPHER     *cipher;            /* Cipher for decryption */
        char            *key;
	char            ivec[EVP_MAX_IV_LENGTH] = {0};

	unsigned char  *ciphertext;        /* Ciphertext buffer */
	int             ctlen;             /* Length of ciphertext */

        struct stat     fstats;            /* Stats on ciphertext file */
        int fd;                            /* Descriptor for ciphertext file */

        /* Get key */
	fd=open(argv[2],O_RDWR);
	fstat(fd,&fstats);
        printf("Key is %d bytes\n",fstats.st_size);
        key=malloc(MAXKEYLEN);
        read(fd,key,fstats.st_size);
        fprintf(stdout,"Key: <");
        fprint_string_as_hex(stdout,key,fstats.st_size);
	fprintf(stdout,">\n");
        close(fd);

        /* Read ciphertext */
        fd=open(argv[1],O_RDONLY);
	fstat(fd,&fstats);
        printf("Ciphertext is %d bytes\n",fstats.st_size);
        ciphertext=malloc(fstats.st_size);
        read(fd,ciphertext,fstats.st_size);
        fprintf(stdout,"Ciphertext: <");
        fprint_string_as_hex(stdout,ciphertext,fstats.st_size);
	fprintf(stdout,">\n");
        ctlen=fstats.st_size;

        /* Decrypt */
        cipher=(EVP_CIPHER *)EVP_bf_cbc();
        ctx=(EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
        EVP_DecryptInit_ex(ctx,cipher,NULL,NULL,NULL);
        EVP_CIPHER_CTX_set_key_length(ctx,32);
        EVP_DecryptInit_ex(ctx,NULL,NULL,key,ivec);
       	decrypt_and_print(ctx, ciphertext, ctlen, stdout);

	return 0;
}
