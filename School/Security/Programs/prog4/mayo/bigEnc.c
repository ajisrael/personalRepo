/*========================================================================
  Portions of this code taken from:

          Building Secure Software, by John Viega and Gary McGraw

  Code only demonstrates basic functions required for the project.  It is
  not secure.  Return codes are given in the manual pages.

  Functionality: (1) Generate random bytes from /dev/urandom; 
                     Write in hex to stdout
                 (2) Write selected key size bytes of these to file
		     named as second argument
		     Write in hex to stdout
		 (3) Use key to encrypt contents of file named as first
		     argument; write ciphertext to file with same name
		     appended with ".enc"; write in ciphertext in hex 
		     to stdout
  ========================================================================
*/

#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAXKEYLEN  56
#define ACTKEYLEN  32
#define BLOCKSIZE   8  /* Blowfish block size */
#define BUFSIZE    10
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
	return (unsigned char *)malloc(mlen + BLOCKSIZE);
}

int 
main(int argc, char **argv)
{
        EVP_CIPHER_CTX  *ctx;                 /* Context for encryption */
	EVP_CIPHER      *cipher;              /* Cipher */
	char            ivec[EVP_MAX_IV_LENGTH] = {0}; 
        int             fdk;                  /* Key to this file */
        unsigned char   key[MAXKEYLEN];       /* Encryption key  */
        int             fdp;                  /* Plaintext from this file */
        char            buf[BUFSIZE];
        int             mlen;                 /* Length of plaintext */
        int             fde;
        char           *encFile;
	unsigned char  *ciphertext;           /* Pointer to ciphertext */
	int             ctlen;                /* Length of ciphertext */

	/* Generate the key -- may be more bytes than needed*/
        initialize_key(key,MAXKEYLEN);
        fprintf(stdout,"Raw file key bytes <");
	fprint_string_as_hex(stdout, key, MAXKEYLEN);
        fprintf(stdout,">\n");

        /* Write key to file */
        fdk=open(argv[2],O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
        write(fdk,key,ACTKEYLEN); 
        printf("Key is <");fprint_string_as_hex(stdout,key,ACTKEYLEN);
        printf(">\n");
	close(fdk);


	/* Encrypt file contents */
        fdp=open(argv[1],O_RDONLY);
 
        encFile=malloc(strlen(argv[1])+5); /* filename.enc */
        strcpy(encFile,argv[1]);
        strcat(encFile,".enc");
        printf("Writing encrypted file content to <%s>\n",encFile);
        fde=open(encFile,O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_APPEND, S_IRUSR | S_IWUSR);

        cipher=(EVP_CIPHER *)EVP_bf_cbc();
        ctx = (EVP_CIPHER_CTX *)malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        EVP_EncryptInit_ex(ctx,cipher,NULL,NULL,NULL);
        EVP_CIPHER_CTX_set_key_length(ctx,ACTKEYLEN);
	EVP_EncryptInit_ex(ctx,NULL,NULL,key,ivec);

        ciphertext=allocate_ciphertext(BUFSIZE); 
        ctlen=0;
        while ((mlen=read(fdp,buf,BUFSIZE))>0) {
          printf("Read %d bytes of plaintext <",mlen);
          fprint_string_as_hex(stdout,buf,mlen);printf(">\n");

          EVP_EncryptUpdate(ctx, ciphertext, &ctlen, buf, mlen);

          write(fde,ciphertext,ctlen);
          printf("Wrote %d bytes of ciphertext <",ctlen);
          fprint_string_as_hex(stdout,ciphertext,ctlen);
          printf(">\n");

	}

        ctlen=0;
        EVP_EncryptFinal_ex(ctx,ciphertext, &ctlen);
        write(fde,ciphertext,ctlen);
        printf("Wrote %d bytes of ciphertext <",ctlen);
        fprint_string_as_hex(stdout,ciphertext,ctlen);
        printf(">\n");

	
}
