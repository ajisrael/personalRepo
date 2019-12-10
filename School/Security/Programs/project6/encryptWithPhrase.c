
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "encryptWithPhrase.h"

#define SHA1LENGTH  20
#define BFKEYLEN    16
#define BLOCKSIZE    8  /* Blowfish block size */
#define BUFSIZE     10
void 
initialize_key(unsigned char *key, int length)
{
  int more;
  int intsize;
  int done;
  int left;

  srand(10);
  
  intsize=sizeof(int);
  left=length;
  done=0;
  while (done<length)
    { 
      more=rand();
      if (left >= intsize) 
	{	  
	  strncpy(key+done,(unsigned char*)&more,intsize);
	  left-=intsize;done+=intsize;
	}else{
 	  strncpy(key+done,(unsigned char *)&more,left);
	  done+=left;	 
	}
    }
}
unsigned char  *
allocate_ciphertext(int mlen)
{
	/* Alloc enough space for any possible padding. */
	return (unsigned char *)malloc(mlen + BLOCKSIZE);
}
void 
encrypt_and_print(EVP_CIPHER_CTX * ectx, char *msg, int mlen,
                  char *res, int *olen, FILE * f)
{
  
  int             extlen;
  
  EVP_EncryptUpdate(ectx, res, olen, msg, mlen);
  EVP_EncryptFinal(ectx, &res[*olen], &extlen);
  
  *olen += extlen;  
}

int encryptWithPhrase(char *plaintext, char *file, int size)
{
        unsigned char   fkey[BFKEYLEN];       /* File encryption key.  More 
                                                 bytes than needed.  */
        unsigned char   kkey[SHA1LENGTH];     /* Raw key key bytes */

        EVP_MD_CTX      hctx;     	      /* Context for hash */
        int             hlen;	              /* Length of hash */

        EVP_CIPHER_CTX  ctx;                  /* Context for encryption */
	EVP_CIPHER      *cipher;              /* Cipher */
	char            ivec[EVP_MAX_IV_LENGTH] = {0}; 
	int             ret;	              /* Return status */
        int             fdk;                  /* Key to this file */
        int             fdp;                  /* Plaintext from this file */
        int             fde;
        char           *encFile;
        char           *keyFile;
	unsigned char  *ciphertext;           /* Pointer to ciphertext */
	int             ctlen;                /* Length of ciphertext */
        char            phrase[80];
	int             namelen;
	
 	
	/* Generate the key -- may be more bytes than needed*/
        initialize_key(fkey,BFKEYLEN);

        /* Get the passphrase. Will run digest over this to produce key */ 
        printf("Input passphrase: ");
        gets(phrase);
	
        /* Compute the digest. Output is the key key.*/
        EVP_DigestInit(&hctx, EVP_sha1());
        EVP_DigestUpdate(&hctx, phrase, strlen(phrase)); /* Won't include the
							 string terminator*/
        EVP_DigestFinal(&hctx, kkey, &hlen);	
	
       /* Encrypt key */
       cipher = (EVP_CIPHER *)EVP_bf_cbc();       /* Blowfish CBC mode */
       EVP_EncryptInit(&ctx, cipher, kkey, ivec); /* Kkey to encrypt kfile */
       ciphertext = allocate_ciphertext(BFKEYLEN);
       encrypt_and_print(&ctx, fkey, BFKEYLEN, /* Encrypt fkey output is */
                          ciphertext, &ctlen, stdout);     /* ciphertext */
	
         /* Write encrypted key to file */
         namelen=strlen(file)+strlen(".key")+1;	
         keyFile=malloc(namelen);
	 strcpy(keyFile,file);
	 strcat(keyFile,".key");
 	 keyFile[namelen-1]=0;
	 
        fdk=open(keyFile,O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
        write(fdk,ciphertext,ctlen); 
	close(fdk);

        namelen=strlen(file)+strlen(".key")+1;	
        encFile=malloc(namelen);
	strcpy(encFile,file);
	strcat(encFile,".enc");
	encFile[namelen-1]=0;
	
        fde=open(encFile,O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_APPEND, S_IRUSR | S_IWUSR);

        ret=EVP_CIPHER_CTX_cleanup(&ctx);

        cipher=(EVP_CIPHER *)EVP_bf_cbc();
        EVP_EncryptInit(&ctx, cipher, fkey, ivec); /* fkey to encrypt file */
        ciphertext = allocate_ciphertext(size+1);
        encrypt_and_print(&ctx, plaintext, size, 
                          ciphertext, &ctlen, stdout);        /* ciphertext */

        write(fde,ciphertext,ctlen);
}
