
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "encryptWithPhrase.h"
/// CHANGE: Required library for secure core dump
#include <sys/resource.h>
#include <sys/time.h>
/// CHANGE: Required library for locking and unlocking memory
#include <unistd.h>

#define SHA1LENGTH 20
#define BFKEYLEN 16
#define BLOCKSIZE 8 /* Blowfish block size */
#define BUFSIZE 10

/// CHANGE: Function added to lock memory
int lockMemory(char *addr, size_t size)
{
        // -----------------------------------------------------------------------------
        // Name: lockMemory
        // Func: Locks pages in memory at address addr for size / pageSize pages.
        // Meth: Gets the system page size, caclulates the number of pages
        //       required, and locks them.
        // Args: addr = Location of variable to lock in memory.
        //       size = Size in bytes of variable to lock in memory.
        // Vars: page_offset = Offset from size to page.
        //       page_size   = Page size on system.
        // Retn: Return value from mlock().
        // -----------------------------------------------------------------------------
        unsigned long page_offset; // Offest from size to page
        unsigned long page_size;   // Page size of system

        // Set page size and offset
        page_size = sysconf(_SC_PAGE_SIZE);
        page_offset = (unsigned long)addr % page_size;

        addr -= page_offset;        // Adjust addr to pg boundary
        size += page_offset;        // Adjust size w/page_offset
        return (mlock(addr, size)); // Lock the memory
}

/// CHANGE: Funciton added to unlock memory
int unlockMemory(char *addr, size_t size)
// -----------------------------------------------------------------------------
// Name: unlockMemory
// Func: Unlocks pages in memory at address addr for size / pageSize pages.
// Meth: Sets the value of the contents of the char array at addr to NULL. Then
//       gets the system page size, caclulates the number of pages required,
//       and unlocks them.
// Args: addr = Location of variable to unlock in memory.
//       size = Size in bytes of variable to unlock in memory.
// Vars: page_offset = Offset from size to page.
//       page_size   = Page size on system.
// Retn: Return value from munlock().
// -----------------------------------------------------------------------------
{
        unsigned long page_offset; // Offest from size to page
        unsigned long page_size;   // Page size of system

        // Clear variable to NULL
        memset(addr,0,size);

        // Set page size and offset
        page_size = sysconf(_SC_PAGE_SIZE);
        page_offset = (unsigned long)addr % page_size;

        addr -= page_offset;          // Adjust addr to page boundary
        size += page_offset;          // Adjust size with page_offset
        return (munlock(addr, size)); // Unlock the memory
}

/// CHANGE: Added function to set core dump to zero.
int secureCoreDumpRet()
// -----------------------------------------------------------------------------
// Name: secureCoreDumpRet
// Func: Sets the size of the core dump to zero.
// Meth: Gets the current core settings, sets the current and max limit
//       attributes to zero and sets that as the settings for the core.
// Vars: plimits = Rlimit structure for getting and setting core attributes.
// Retn: status  = The status of the function. 0 = good, -1 = error occured.
// -----------------------------------------------------------------------------
{
        int status = 0; // status of function
        struct rlimit plimits;
        if (getrlimit(RLIMIT_CORE, &plimits) < 0)
        {
                perror("core_get");
                status = -1;
        }
        plimits.rlim_cur = 0;
        plimits.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &plimits) < 0)
        {
                perror("core_set");
                status = -1;
        }

        return status;
}

void initialize_key(unsigned char *key, int length)
{
        int more;
        int intsize;
        int done;
        int left;

        /// CHANGE: Random seed for srand
        srand(time(NULL));

        intsize = sizeof(int);
        left = length;
        done = 0;
        while (done < length)
        {
                more = rand();
                if (left >= intsize)
                {
                        strncpy(key + done, (unsigned char *)&more, intsize);
                        left -= intsize;
                        done += intsize;
                }
                else
                {
                        strncpy(key + done, (unsigned char *)&more, left);
                        done += left;
                }
        }
}

unsigned char *allocate_ciphertext(int mlen)
{
        /* Alloc enough space for any possible padding. */
        return (unsigned char *)malloc(mlen + BLOCKSIZE);
}

/// CHANGE: Returns int for error checking
int encrypt_and_print(EVP_CIPHER_CTX *ectx, unsigned char *msg, int mlen,
                      unsigned char *res, int *olen, FILE *f)
{

        int extlen;

        /// CHANGE: Added status variable for detecting errors
        int status = 0;

        /// CHANGE: Error checking on EVP functions
        if (EVP_EncryptUpdate(ectx, res, olen, msg, mlen) == 0)
        {
                perror("encrypt_update");
                status = -1;
        }

        if (status == 0 && EVP_EncryptFinal(ectx, &res[*olen], &extlen) == 0)
        {
                perror("encrypt_update");
                status = -1;
        }

        *olen += extlen;
        return status;
}

int encryptWithPhrase(char *plaintext, char *file, int size)
{
        //default size of passphrase
        int psize = 80;
        unsigned char fkey[BFKEYLEN];   /* File encryption key.  More 
                                                 bytes than needed.  */
        unsigned char kkey[SHA1LENGTH]; /* Raw key key bytes */

        EVP_MD_CTX hctx; /* Context for hash */
        int hlen;        /* Length of hash */

        EVP_CIPHER_CTX ctx; /* Context for encryption */
        EVP_CIPHER *cipher; /* Cipher */
        char ivec[EVP_MAX_IV_LENGTH] = {0};
        int ret; /* Return status */
        int fdk; /* Key to this file */
        int fdp; /* Plaintext from this file */
        int fde;
        char *encFile;
        char *keyFile;
        unsigned char *ciphertext; /* Pointer to ciphertext */
        int ctlen;                 /* Length of ciphertext */

        /// CHANGE: malloc instead of static size (starts at 80)
        char *phrase = malloc(sizeof(char) * psize);
        int namelen;

        /// CHANGE: get uids of current process for preventing root from running
        uid_t uid = getuid();   // UID of current process
        uid_t euid = geteuid(); // EUID of current process

        // CHANGE: Clear process environment
        if (clearenv() != 0)
        {
                perror("clearenv");
                free(phrase);
		return 1;
        }

        /// CHANGE: Secure the core dump size to zero
        // Set core dump size to zero
        if (secureCoreDumpRet() == -1)
        {
                perror("secure_core_dump");
                free(phrase);
                return 1;
        }

        /// CHANGE: Set the umask to prevent group and world bits from being set
        umask(077);

        /// CHANGE: Prevent process from running as root
        if (euid == 0 || uid == 0)
        {
                printf("root: ssp cannot be run as root.\n");
                free(phrase);
		return 1;
        }

        /* Generate the key -- may be more bytes than needed*/
        initialize_key(fkey, BFKEYLEN);

        if (lockMemory(fkey, sizeof(fkey)) == -1)
        {
                perror("fkey_lockMemory");
                free(phrase);
                return 1;
        }

        /* Get the passphrase. Will run digest over this to produce key */
        printf("Input passphrase: ");

        /// CHANGE: reads forever for an really long password
        int read = 1;
        while (read == 1 || phrase[read - 2] != '\n')
        {
                if (read > psize)
                        phrase = realloc(phrase, sizeof(char) * ++psize);
                scanf("%c", &phrase[read - 1]);
                read++;
        }
        phrase[read - 2] = '\0';

        /// CHANGE: Make sure passphrase isn't less than 3 characters
        if (strlen(phrase) < 3)
        {
                printf("Invocation: Passphrase needs to be at least 3 characters long.\n");
                free(phrase);
                return 1;
        }

        if (lockMemory(phrase, sizeof(char) * strlen(phrase)) == -1)
        {
                perror("phrase_lockMemory");
                free(phrase);
                return 1;
        }
	

        /* Compute the digest. Output is the key key.*/

        /// CHANGE: Check all return codes of EVP calls
        if (EVP_DigestInit(&hctx, EVP_sha1()) == 0)
        {
                perror("digest_init");
                free(phrase);
                return 1;
        }

        /// CHANGE: Lock kkey into memory
        if (lockMemory(kkey, sizeof(kkey)) == -1)
        {
                perror("kkey_lockMemory");
                free(phrase);
                return 1;
        }

        /// CHANGE: Check all return codes of EVP calls
        if (EVP_DigestUpdate(&hctx, phrase, strlen(phrase)) == 0)
        {
                perror("digest_update");
                free(phrase);
                
                return 1;
        }
        /* Won't include the string terminator*/

        if (EVP_DigestFinal(&hctx, kkey, &hlen) == 0)
        {
                perror("digest_final");
                free(phrase);
                
                return 1;
        }

        /// CHANGE: Does not clean up or free hctx
        if (EVP_MD_CTX_cleanup(&hctx) == 0)
        {
                perror("EVP_MD_CTX_cleanup");
                free(phrase);
                
                return 1;
        }
        

        /// CHANGE: Setting phrase to zero in memory and unlock the memory
        if (unlockMemory(phrase, sizeof(char) * strlen(phrase)) == -1)
        {
                perror("phrase_unlockMemory");
                free(phrase);
                return 1;
        }


        /// CHANGE: Freeing phrase
        free(phrase);


        /* Encrypt key */
        cipher = (EVP_CIPHER *)EVP_bf_cbc(); /* Blowfish CBC mode */

        /// CHANGE: Error checking of EVP funtion
        if (EVP_EncryptInit(&ctx, cipher, kkey, ivec) == 0) /* Kkey to encrypt kfile */
        {
                perror("kFile_encrypt_init");
                return 1;
        }

        /// CHANGE: Error checking for unlock memory, and unlocking memory
        if (unlockMemory(kkey, sizeof(kkey)) == -1)
                {
                        perror("kkey_unlockMemory");
                        return 1;
                }

        ciphertext = allocate_ciphertext(BFKEYLEN);
        /// CHANGE: check ciphertext gets malloc'd
        if (ciphertext == NULL)
        {
                perror("allocate_ciphertext");
                EVP_CIPHER_CTX_cleanup(&ctx);
                
                return 1;
        }

        /// CHANGE: check encrypt_and_print return codes
        if (encrypt_and_print(&ctx, fkey, BFKEYLEN,              /* Encrypt fkey output is */
                              ciphertext, &ctlen, stdout) == -1) /* ciphertext */
        {
                EVP_CIPHER_CTX_cleanup(&ctx);
		free(ciphertext);                
                return 1;
        }

        //CHANGE: Moved ctx cleanup higher up
      	
	  if (EVP_CIPHER_CTX_cleanup(&ctx) == 0)
        {
                perror("cleanup_ctx");
         	free(ciphertext);       
                return 1;
        }
	

        /* Write encrypted key to file */
        namelen = strlen(file) + strlen(".key") + 1;
        keyFile = malloc(namelen);

        /// CHANGE: Error check for malloc call
        if (keyFile == NULL)
        {
                perror("keyFile_malloc");
                free(ciphertext);
		return 1;
        }

        strcpy(keyFile, file);
        strcat(keyFile, ".key");
        

        fdk = open(keyFile, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        /// CHANGE: Error checking of open
        if (fdk == -1)
        {
                perror("keyFile_open");
                free(keyFile);
		free(ciphertext);
                return 1;
        }

        /// CHANGE: Free keyFile
        free(keyFile);

        /// CHANGE: Error checking of write
        if (write(fdk, ciphertext, ctlen) == -1)
        {
                perror("keyFile_write");
                close(fdk);
                free(ciphertext);
		return 1;
        }

        /// CHANGE: Error checking of close
        if (close(fdk) == -1)
        {
                perror("keyFile_close");
                return 1;
        }

	free(ciphertext);
        namelen = strlen(file) + strlen(".key") + 1;
        encFile = malloc(namelen);

        /// CHANGE: Error checking of malloc
        if (encFile == NULL)
        {
                perror("encFile_malloc");
                return 1;
        }

        strcpy(encFile, file);
        strcat(encFile, ".enc");
        
        fde = open(encFile, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_APPEND, S_IRUSR | S_IWUSR);

        /// CHANGE: Error checking of open
        if (fde == -1)
        {
                perror("encFile_open");
                free(encFile);
                return 1;
        }

        /// CHANGE: Free encFile
        free(encFile);

        cipher = (EVP_CIPHER *)EVP_bf_cbc();

        /// CHANGE: Error checking of EVP funtion
        if (EVP_EncryptInit(&ctx, cipher, fkey, ivec) == 0) /* fkey to encrypt file */
        {
                perror("encFile_encrypt_init");
                close(fde);
                return 1;
        }

        ciphertext = allocate_ciphertext(size + 1);

        /// CHANGE: Check for proper allocation of ciphertext
        if (ciphertext == NULL)
        {
                perror("encFile_allocate_ciphertext");
                close(fde);
                return 1;
        }

        /// CHANGE: check encrypt_and_print return codes
        if (encrypt_and_print(&ctx, plaintext, size,              /* Encrypt fkey output is */
                              ciphertext, &ctlen, stdout) == -1) /* ciphertext */
        {
                EVP_CIPHER_CTX_cleanup(&ctx);
		free(ciphertext);                
                return 1;
        }

        //CHANGE: unlocking fkey in memory
        if (unlockMemory(fkey, sizeof(fkey)) == -1)
        {
                perror("unlock Memory Error fkey");
                close(fde);
                free(ciphertext);
		return 1;
        }

     
	   //CHANGE cleanup ctx again   
      if (EVP_CIPHER_CTX_cleanup(&ctx) == 0)
        {
                perror("cleanup_ctx");
         	free(ciphertext);       
                close(fde);
                return 1;
        }
        
        /// CHANGE: Error checking write
        if (write(fde, ciphertext, ctlen) == -1)
        {
                perror("writing ciphertext");
                free(ciphertext);
		close(fde);
                return 1;
        }

        /// CHANGE: Error chekcing Close
        if (close(fde) == -1)
        {
                perror("closing fde");
                free(ciphertext);
		return 1;
        }
	free(ciphertext);
        /// CHANGE: Exit status okay
        return 0;
}
