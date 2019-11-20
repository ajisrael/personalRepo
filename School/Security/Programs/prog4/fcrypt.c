//------------------------------------------------------------------------------
// Orig: 2019.11.10 - Alex Israels
// Revs: 2019.11.19 - Alex Israels
// Prog: fcrpt.c
// Func: Encrypts and decrptes a file, depending on the input of the user.
// Defn: KEYLEN    = Length of Kenc.
//       MAXKEYLEN = Max length of Kenc.
//       DIGLEN    = Length of digest Kpass.
//       BLOCKSIZE = Size of a block for cipher in bytes.
//       STDIN_FD  = File desctriptor value for stdin.
//       BUFSIZE   = Size of buffer for reading and writing to files.
//       MAXPHLEN  = Maximum length of a phrase.
//       MINPHLEN  = Minimum length of a phrase.
//------------------------------------------------------------------------------

#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <unistd.h>

#define KEYLEN     16 // Length of Kenc
#define MAXKEYLEN  16 // Max length of Kenc
#define DIGLEN     20 // Length of digest Kpass
#define BLOCKSIZE   8 // Size of a block for cipher in bytes
#define STDIN_FD    0 // File descriptor value for stdin
#define BUFSIZE    10 // Size of buffer for reading and writing to files
#define MAXPHLEN   80 // Maximum length of a phrase
#define MINPHLEN   10 // Minimum length of a phrase

void initializeKey(unsigned char *key, int length)
// -----------------------------------------------------------------------------
// Name: initializeKey
// Func: Initializes key <key> for a length <length> from /dev/urandom.
// Args: key    = Key to be initialized.
//       length = Length of key.
// Vars: rng    = File stream for /dev/urandom.
//       num    = Number of bytes read from rng.
// -----------------------------------------------------------------------------
{
	FILE * rng;   // File stream for /dev/urandom
	int  num = 0; // Number of bytes read from rng

	rng = fopen("/dev/urandom", "r"); // Open /dev/urandom

    if (rng == NULL)  // Check that it opened correctly
    {
        perror("init_key");
        exit(1);
    }

    // Read random numbers
	while (num < length) {
		num += fread(&key[num], 1, length - num, rng);
	}

    // Close file stream
	fclose(rng);
}

void printHex(FILE * f, unsigned char *s, int len)
// -----------------------------------------------------------------------------
// Name: printHex
// Func: Prints out a character array s to file stream f for len bytes.
// Args: f   = Files stream for output.
//       s   = Pointer to character array.
//       len = Length of character array.
// Vars: i   = Iterator of for loop.
// -----------------------------------------------------------------------------
{
	int i;
	for (i = 0; i < len; i++) {
		fprintf(f, "%02x", s[i]);
	}
}

unsigned char * allocateCiphertext(int mlen)
// -----------------------------------------------------------------------------
// Name: allocateCiphertext
// Func: Allocates space in memory with a minimum size BLOCKSIZE.
// Args: mlen = Lenth of memory in bytes to allocate.
// Retn: Base addr of allocated memory.
// -----------------------------------------------------------------------------
{
	// Alloc enough space for any possible padding.
	return (unsigned char *) malloc(mlen + BLOCKSIZE);
}

int lockMemory(char * addr, size_t size) {
// -----------------------------------------------------------------------------
// Name: lockMemory
// Func: Locks pages in memory at address addr for size / pageSize pages.
// Meth: Then gets the system page size, caclulates the number of pages 
//       required, and unlocks them.
// Args: addr = Location of variable to lock in memory.
//       size = Size in bytes of variable to lock in memory.
// Vars: page_offset = Offset from size to page.
//       page_size   = Page size on system.
// Retn: Return value from munlock().
// -----------------------------------------------------------------------------
    unsigned long page_offset;  // Offest from size to page
    unsigned long page_size;    // Page size of system

    // Set page size and offset
    page_size = sysconf(_SC_PAGE_SIZE);
    page_offset = (unsigned long) addr % page_size;

    addr -= page_offset; // Adjust addr to pg boundary
    size += page_offset; // Adjust size w/page_offset
    return ( mlock(addr, size) ); // Lock the memory
} 

int unlockMemory(char * addr, size_t size) 
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
    unsigned long page_offset;  // Offest from size to page
    unsigned long page_size;    // Page size of system

    // Clear variable to NULL
    bzero(addr, size);

    // Set page size and offset
    page_size = sysconf(_SC_PAGE_SIZE);
    page_offset = (unsigned long) addr % page_size;

    addr -= page_offset; // Adjust addr to page boundary
    size += page_offset; // Adjust size with page_offset
    return ( munlock(addr, size) ); // Unlock the memory
} 

void secureCoreDump()
// -----------------------------------------------------------------------------
// Name: secureCoreDump
// Func: Sets the size of the core dump to zero.
// Meth: Gets the current core settings, sets the current and max limit
//       attributes to zero and sets that as the settings for the core.
// Vars: plimits = Rlimit structure for getting and setting core attributes.
// -----------------------------------------------------------------------------
{
    struct rlimit plimits;
    if (getrlimit(RLIMIT_CORE, &plimits) < 0)
    {
        perror("core_get");
        exit(1);
    }
    plimits.rlim_cur = 0;
    plimits.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &plimits) < 0)
    {
        perror("core_set");
        exit(1);
    }
}

int main (int argc, char* argv[])
// -----------------------------------------------------------------------------
// Name: Main
// Func: Either encrypts or decrypts a file using a passphrase depending on the 
//       input from the user.
// Meth: First the program checks for the correct input. Confirming there are 3
//       arguments. It then checks that the first argument is '-e' or '-d' and
//       assigns the mode of the program accordingly. The user is then promted 
//       for a passphrase twice to confirm the passphrase entered is what the 
//       user intended. The passphrase must be between 10 and 80 characters to 
//       be valid. Once the passphrase has been confirmed a SHA1 hash is ran 
//       over the passphrase to generate Kpass. After Kpass is generated the 
//       program runs in the mode determined by argv[1].
// Encr: The program initializes Kenc from '\dev\urandom\' and uses this to
//       encrypt the datafile specified by argv[2] using a Blow Fish algortithm.
//       The encrypted data is written to a newly created file <dataFile>.enc.
//       Kenc is then encrypted with a Blow Fish algorithm using Kpass as the
//       key and written to <keyFile> specified by argv[3].
// Decr: The program uses Kpass to decrypt Kenc stored in <keyFile>. Once Kenc
//       is decrypted it is used to decrypt <dataFile>.enc whose decrypted
//       contents are then printed out to the console.
// Args: argc = The number of arguments to the program.
//       argv = Array of character arrays specifying arguments to the program.
// Vars: termInfo    = Struct for manipulating terminal properties/
//       fstats      = Struct for getting metadata of a file.
//       shactx      = Context for SHA1 hash.
//       ctx         = Context for file encryption and decryption.
//       keyctx      = Context for key encryption and decryption.
//       cipher      = Cipher for files.
//       keyCipher   = Cipher for keys.
//       phrase1     = 1st passphrase from user.
//       phrase2     = 2nd passphrase from user.
//       buf         = Buffer of BUFSIZE for reading and writing.
//       encFileName = Name of <datafile>.enc.
//       ivec        = Initialization vector for Blow Fish algorithm.
//       kPass       = Key generated by SHA1.
//       kEnc        = Key generated from /dev/urandom.
//       ciphertext  = Ciphertext generated by Blow Fish algorithm.
//       res         = Result of key decryption.
//       fileResult  = Result of file decryption.
//       messLen     = Length of message to encrypt and recived from decryption.
//       prompting   = Boolean for loop to keep promting user for passphrases.
//       passLen     = Length of the passphrase.
//       mode        = Mode of the application: 1 = encrypt, -1 = decrypt.
//       dataFile    = File descriptor for dataFile.
//       encFile     = File descriptor for dataFile.enc.
//       keyFile     = File descriptor for keyFile.
//       sha1Len     = The length of the outputted digest from SHA1.
//       ctLen       = Length of ciphertext.
//       outLen      = Length of decrypted result.
//       keyMLen     = Max length of decrypted Kenc.
//       resLen      = Actual length of decrypted Kenc.
//       i           = Iterator for loops.
// Retn: 0 = Success.
//       1 = Error.
// -----------------------------------------------------------------------------
{
    struct termios termInfo;        // Struct for terminal echo
    struct stat    fstats;          // Struct for file stats

    EVP_MD_CTX     * shactx;        // Context of SHA1
    EVP_CIPHER_CTX * ctx;           // Context of file d/ecryption
    EVP_CIPHER_CTX * keyCtx;        // Context of key  d/ecryption
    EVP_CIPHER     * cipher;        // Resulting File Cipher
    EVP_CIPHER     * keyCipher;     // Resulting Key Cipher

    char phrase1[MAXPHLEN];  // Holds the 1st passphrase from the user
    char phrase2[MAXPHLEN];  // Holds the 2nd passphrase from the user
    char buf[BUFSIZE];       // Buffer of BUFSIZE for reading and writing
    char *  encFileName;     // Name of <datafile>.enc

    char ivec[EVP_MAX_IV_LENGTH] = {0}; // Initialization vector for BF algo

    unsigned char kPass[DIGLEN];    // Kpass generated from SHA1
    unsigned char kEnc [KEYLEN];    // Kenc generated from /dev/urandom
    unsigned char * ciphertext;     // Ciphertext generated by BF algo
    unsigned char * res;            // Result of key decryption
    unsigned char * fileResult;     // Result of file decryption

    int messLen = KEYLEN; // Length of message to encrypt
    int prompting  = 1;   // Boolean for prompting loop
    int passLen    = 0;   // Length of passphrase
    int mode       = 0;   // Mode of application 1 = encrypt, -1 = decrypt
    int dataFile   = 0;   // File descriptor for dataFile
    int encFile    = 0;   // File descriptor for dataFile.enc
    int keyFile    = 0;   // File descriptor for keyFile
    int sha1Len    = 0;   // Length of kPass generated from SHA1
    int ctLen      = 0;   // Length of ciphertext
    int outLen     = 0;   // Length of decrypted result
    int keyMLen    = 0;   // Length of Kenc
    int resLen     = 0;   // Length of resulting Kenc
    int i          = 0;   // Iterator for loops

    // Error checking for invocation
    if (argc != 4)
    {
        printf("Error: Invalid invocation. Please run with following format:\n");
        printf("./fcrypt <mode> <datafile> <keyfile>\n");
        exit(1);
    }

    // Secure Core
    secureCoreDump();

    // Assign mode
    if (strcmp(argv[1], "-e") == 0)
    {
        mode =  1;  // Set mode to encrypt
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        mode = -1;  // Set mode to decrypt
    }
    else
    {
        printf("Error: Invalid mode. Mode must be '-e' or '-d'.\n");
        exit(1);
    }

    // Turn off terminal echo
    if (tcgetattr(STDIN_FD, &termInfo) == -1)
    {
        perror("get_terminal_info");
        exit(1);
    }
    termInfo.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FD, TCSAFLUSH, &termInfo) == -1)
    {
        perror("set_terminal_info");
        exit(1);
    }

    // Lock phrases
    lockMemory(phrase1, MAXPHLEN);
    lockMemory(phrase2, MAXPHLEN);

    // Loop for getting valid passphrase
    while (prompting == 1)
    {
        // Prompt user for passphrase
        printf("Enter a passphrase: ");
        scanf("%[^\n]%*c", phrase1);
        printf("\n");

        // Prompt user for second passphrase
        printf("Verify  passphrase: ");
        scanf("%[^\n]%*c", phrase2);
        printf("\n");

        // If equal then check length
        if (strcmp(phrase1, "") == 0 || strcmp(phrase2, "") == 0)
        {
            printf("Error: No phrase entered.\n");
            unlockMemory(phrase1, MAXPHLEN);
            unlockMemory(phrase2, MAXPHLEN);
            exit(1);
        } 
        else if (strcmp(phrase1, phrase2) == 0)
        {
            // If in range of 10 to 80 characters break loop
            passLen = strlen(phrase1);
            if (passLen >= MINPHLEN && passLen <= MAXPHLEN)
            {
                prompting = 0;
            }
            else
            {
                printf("Invalid phrase length. Must be between 10 and 80 characters.\n");
            }
        }
        else 
        {
            printf("Phrases are not equal. Please try again.\n");
        }
    }

    // Turn on terminal echo
    if (tcgetattr(STDIN_FD, &termInfo) == -1)
    {
        perror("get_terminal_info");
        unlockMemory(phrase1, MAXPHLEN);
        unlockMemory(phrase2, MAXPHLEN);
        exit(1);
    }
    termInfo.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FD, TCSAFLUSH, &termInfo) == -1)
    {
        perror("set_terminal_info");
        unlockMemory(phrase1, MAXPHLEN);
        unlockMemory(phrase2, MAXPHLEN);
        exit(1);
    }

    // Lock Kpass
    lockMemory(kPass, DIGLEN);

    // Once a valid passphrase is accepted run SHA1 over phrase to make Kpass
    shactx = EVP_MD_CTX_create();
    EVP_MD_CTX_init(shactx);
    EVP_DigestInit_ex(shactx, EVP_sha1(), NULL);
    EVP_DigestUpdate(shactx, phrase1, passLen);
    EVP_DigestFinal_ex(shactx, kPass, &sha1Len);

    // Check if SHA1 Length is valid
    if (sha1Len != DIGLEN)
    {
        printf("Error: Unexpected digest length %d.", sha1Len);
        unlockMemory(phrase1, MAXPHLEN);
        unlockMemory(phrase2, MAXPHLEN);
        unlockMemory(kPass, DIGLEN);
        EVP_MD_CTX_destroy(shactx);
        exit(1);
    }

    // Unlock phrases
    unlockMemory(phrase1, MAXPHLEN);
    unlockMemory(phrase2, MAXPHLEN);

    // Print out Kpass in hexadecimal
    printf("Kpass: <");
    printHex(stdout, kPass, sha1Len);
    printf(">\n");

    // Determine invocation and run accordingly:
// Encrypt ---------------------------------------------------------------------
    if (mode == 1) 
    {
    // Encrypt Datafile Begin --------------------------------------------------
        // Generate 128 bit random number Kenc from /dev/urandom
        lockMemory(kEnc, KEYLEN);
        initializeKey(kEnc, messLen);

        // Encrypt datafile using Blowfish in CBC mode
        // Open datafile
        dataFile = open(argv[2], O_RDONLY);
        if (dataFile == -1)
        {
            perror("dataFile");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            EVP_MD_CTX_destroy(shactx);
            exit(1);
        }

        // Create encrypted dataFile name
        encFileName = malloc(strlen(argv[2]) + 5);
        strcpy(encFileName, argv[2]);
        strcat(encFileName, ".enc");

        // Create encyrpted dataFile
        encFile = open(encFileName, O_CREAT|O_TRUNC|O_WRONLY|O_NOFOLLOW|O_APPEND,0400);
        if (encFile == -1)
        {
            perror("encFile");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            EVP_MD_CTX_destroy(shactx);
            free(encFileName);
            close(dataFile);
            exit(1);
        }
        free(encFileName);

        // Set up Blow Fish algorithm
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
        EVP_CIPHER_CTX_set_key_length(ctx, KEYLEN);
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, kEnc, ivec) == 0)
        {
            perror("datafile_encrypt_init");
            unlockMemory(kEnc, KEYLEN);
            unlockMemory(kPass, DIGLEN);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(ctx);
            close(dataFile);
            close(encFile);
            exit(1);
        }
        ciphertext = allocateCiphertext(BUFSIZE);

        // Write encrypted data to <datafile>.enc
        while ((messLen = read(dataFile, buf, BUFSIZE)) > 0)
        {
            // Encrypt the data
            if (EVP_EncryptUpdate(ctx, ciphertext, &ctLen, buf, messLen) == 0)
            {
                perror("datafile_encrypt_update");
                unlockMemory(kEnc, KEYLEN);
                unlockMemory(kPass, DIGLEN);
                free(ciphertext);
                EVP_CIPHER_CTX_free(ctx);
                EVP_MD_CTX_destroy(shactx);
                close(dataFile);
                close(encFile);
                exit(1);
            }

            // Write the encrypted data to dataFile.enc
            write(encFile, ciphertext, ctLen);
        }

        // Encrypt last block of data
        if (EVP_EncryptFinal_ex(ctx, ciphertext, &ctLen) == 0)
        {
            perror("datafile_encrypt_final");
            unlockMemory(kEnc, KEYLEN);
            unlockMemory(kPass, DIGLEN);
            free(ciphertext);
            EVP_CIPHER_CTX_free(ctx);
            EVP_MD_CTX_destroy(shactx);
            close(dataFile);
            close(encFile);
            exit(1);
        }

        // Write last block of data
        write(encFile, ciphertext, ctLen);
    // Encrypt Datafile End ----------------------------------------------------

        // Clean up memory
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        close(dataFile);
        close(encFile);

    // Encrypt Kenc Begin ------------------------------------------------------
        // If keyfile doesn't exist it is created
        keyFile = open(argv[3], O_CREAT|O_TRUNC|O_WRONLY|O_NOFOLLOW|O_APPEND,0400);
        if (keyFile == -1)
        {
            perror("keyFile");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            exit(1);
        }

        // Set up Blow Fish algorithm
        keyCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(keyCtx);
        keyCipher = (EVP_CIPHER *) EVP_bf_cbc();
        if (EVP_EncryptInit_ex(keyCtx, keyCipher, NULL, kPass, ivec) == 0)
        {
            perror("Kenc_encrypt_init");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            close(keyFile);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }
        ciphertext = allocateCiphertext(MAXKEYLEN);

        // Set message lengths
        keyMLen = MAXKEYLEN;
        ctLen = 0;
        messLen = 0;
            
        // Encrypt Kenc
        if (EVP_EncryptUpdate(keyCtx, ciphertext, &ctLen, kEnc, keyMLen) == 0)
        {
            perror("Kenc_encrypt_update");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            close(keyFile);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }

        if (EVP_EncryptFinal_ex(keyCtx, &ciphertext[*(&ctLen)], &messLen) == 0)
        {
            perror("Kenc_encrypt_final");
            unlockMemory(kPass, DIGLEN);
            unlockMemory(kEnc,  KEYLEN);
            close(keyFile);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }
        ctLen += messLen;

        // Write encrypted Kenc to keyFile
        write(keyFile, ciphertext, ctLen);
    // Encrypt Kenc End --------------------------------------------------------

        // Clean up memory
        unlockMemory(kPass, DIGLEN);
        unlockMemory(kEnc,  KEYLEN);
        close(keyFile);
        free(ciphertext);
        EVP_MD_CTX_destroy(shactx);
        EVP_CIPHER_CTX_free(keyCtx);
    }
// Decrpyt ---------------------------------------------------------------------
    else if (mode == -1) 
    {
    // Decrypt Kenc Begin ------------------------------------------------------
        // Print out Kpass in hexadecimal
        printf("Kpass: <");
        printHex(stdout, kPass, sha1Len);
        printf(">\n");

        // Get encrypted Kenc
        keyFile = open(argv[3], O_RDONLY | O_NOFOLLOW);
        fstat(keyFile, &fstats);
        ciphertext = malloc(fstats.st_size);
        read(keyFile, ciphertext, fstats.st_size);
        close(keyFile);

        // Setup to decrypt Kenc
        keyCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(keyCtx);
        keyCipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctLen = fstats.st_size;
        if (EVP_DecryptInit_ex(keyCtx, keyCipher, NULL, kPass, ivec) == 0)
        {
            perror("Kenc_decrypt_init");
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }

        // Reset message length
        messLen = 0;

        // Setup resulting Kenc
        resLen = ctLen;
        res = (unsigned char *) malloc(resLen);
        lockMemory(res, resLen);

        // Decrypt Kenc
        if (EVP_DecryptUpdate(keyCtx, res, &outLen, ciphertext, ctLen) == 0)
        {
            perror("Kenc_decrypt_update");
            unlockMemory(res, resLen);
            free(ciphertext);
            free(res);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }
        messLen += outLen;
        if (EVP_DecryptFinal_ex(keyCtx, &res[outLen], &outLen) == 0)
        {
            perror("Kenc_decrypt_final");
            unlockMemory(res, resLen);
            free(ciphertext);
            free(res);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(keyCtx);
            exit(1);
        }
        messLen += outLen;

        // Print out Kenc in hexadecimal
        printf("Kenc:  <");
        printHex(stdout, res, messLen);
        printf(">\n");
    // Decrypt Kenc End --------------------------------------------------------

        // Clean up memory
        free(ciphertext);
        EVP_CIPHER_CTX_free(keyCtx);

    // Decrypt Datafile Begin --------------------------------------------------
        // Open dataFile.enc
        encFile = open(argv[2], O_RDONLY | O_NOFOLLOW);
        fstat(encFile, &fstats);
        ciphertext = malloc(fstats.st_size);
        read(encFile, ciphertext, fstats.st_size);

        // Decrypt dataFile.enc with Kenc
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctLen = fstats.st_size;
        if (EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL) == 0)
        {
            perror("datafile_decrypt_init");
            free(res);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(ctx);
            exit(1);
        }

        EVP_CIPHER_CTX_set_key_length(ctx, KEYLEN);

        if (EVP_DecryptInit_ex(ctx, NULL, NULL, res, ivec) == 0)
        {
            perror("datafile_decrypt_init");
            free(res);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(ctx);
            exit(1);
        }
    
        messLen = 0;
        outLen = 0;
        fileResult = (unsigned char *) malloc(ctLen);

        if (EVP_DecryptUpdate(ctx, fileResult, &outLen, ciphertext, ctLen) == 0)
        {
            perror("datafile_decrypt_update");
            free(res);
            free(fileResult);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(ctx);
            exit(1);
        }
        messLen += outLen;

        if (EVP_DecryptFinal_ex(ctx, &fileResult[outLen], &outLen) == 0)
        {
            perror("datafile_decrypt_final");
            free(res);
            free(fileResult);
            free(ciphertext);
            EVP_MD_CTX_destroy(shactx);
            EVP_CIPHER_CTX_free(ctx);
            exit(1);
        }
        messLen += outLen;

        // Unlock resulting kEnc
        unlockMemory(res, resLen);

        // Print out decrypted datafile in hexadecimal
        printf("Datafile   (HEX): <");
        printHex(stdout, fileResult, messLen);
        printf(">\n");

        // Print out decrypted datafile in ascii
        printf("Datafile (ASCII): <");
        for (i = 0; i < messLen; i++)
        {
            printf("%c", fileResult[i]);
        }
        printf(">\n");
    // Decrypt Datafile End ----------------------------------------------------

        // Clean up memory
        free(res);
        free(fileResult);
        free(ciphertext);
        EVP_MD_CTX_destroy(shactx);
        EVP_CIPHER_CTX_free(ctx);
        close(encFile);
    }
    else
    {
        printf("Invalid mode. Mode must be '-e' or '-d'.\n");
        exit(1);
    }
    fclose(stdout);
    exit(0);
}