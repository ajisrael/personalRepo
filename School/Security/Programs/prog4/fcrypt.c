//------------------------------------------------------------------------------
// Orig: 2019.11.10 - Alex Israels
// Revs: 2019.11.16 - Alex Israels
// Prog: fcrpt.c
// Func: Encrypts and decrptes a file, depending on the input of the user
// Vars: TBD
//------------------------------------------------------------------------------

#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>

#define KEYLEN     16
#define DIGLEN     20
#define BLOCKSIZE   8
#define STDIN_FD    0
#define BUFSIZE    10

void initializeKey(unsigned char *key, int length)
{
	FILE *rng;
	int  num = 0;
	rng = fopen("/dev/urandom", "r");

	while (num < length) {
		num += fread(&key[num], 1, length - num, rng);
	}

	fclose(rng);
}

void printHex(FILE * f, unsigned char *s, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		fprintf(f, "%02x", s[i]);
	}
}

unsigned char * allocateCiphertext(int mlen)
{
	/* Alloc enough space for any possible padding. */
	return (unsigned char *) malloc(mlen+BLOCKSIZE);
}

int main (int argc, char* argv[])
{
    struct termios termInfo;        // Struct for terminal echo
    struct stat    fstats;          // Struct for file stats

    EVP_MD_CTX     * shactx;        // Context of SHA1
    EVP_CIPHER_CTX * ctx;           // Context of file d/ecryption
    EVP_CIPHER_CTX * keyCtx;        // Context of key  d/ecryption
    EVP_CIPHER     * cipher;        // Resulting Cipher

    char phrase1[80];        // Holds the 1st passphrase from the user
    char phrase2[80];        // Holds the 2nd passphrase from the user
    char buf[BUFSIZE];       // Buffer of BUFSIZE for reading and writing
    char ** digest;          // Digest generated from passphrase
    char *  encFileName;     // Name of datafile.enc

    char ivec[EVP_MAX_IV_LENGTH] = {0}; // Initialization vector for BF algo

    unsigned char kPass[DIGLEN];    // Kpass generated from SHA1
    unsigned char kEnc[KEYLEN];     // Kenc generated from /dev/urandom
    
    unsigned char * ciphertext;     // Ciphertext generated by BF algo
    unsigned char * res;            // Result of decryption by BF algo

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

    // Error checking for invocation
    if (argc != 4)
    {
        printf("Invalid invocation. Please run with following format:\n");
        printf("fcrypt <mode> <datafile> <keyfile>\n");
        exit(1);
    }

    // Assign mode
    if (strcmp(argv[1], "-e") == 0)
    {
        mode  = 1;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        mode = -1;
    }
    else
    {
        printf("Invalid mode. Mode must be '-e' or '-d'.\n");
        exit(1);
    }

    // Turn off terminal echo
    tcgetattr(STDIN_FD, &termInfo);
    termInfo.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FD, TCSAFLUSH, &termInfo);

    // Loop for getting valid passphrase
    while (prompting == 1)
    {
        // Prompt user for passphrase
        printf("Enter a passphrase: ");
        scanf("%[^\n]%*c", phrase1);
        printf("\n");

        // Prompt user for second passphrase
        printf("Verify passphrase: ");
        scanf("%[^\n]%*c", phrase2);
        printf("\n");

        // If equal then check length
        if (strcmp(phrase1, phrase2) == 0)
        {
            // If in range of 10 to 80 characters break loop
            passLen = strlen(phrase1);
            if (passLen >= 10 && passLen <= 80)
            {
                prompting = 0;
            }
            else
            {
                printf("Phrase <%s> is not valid.\n", phrase1);
                printf("The phrase must be between 10 and 80 characters.\n");
            }
        }
        else 
        {
            printf("Phrases <%s>, and <%s> are not Equal.\n", phrase1, phrase2);
            printf("Please try again.\n");
        }
    }

    // Turn on terminal echo
    tcgetattr(STDIN_FD, &termInfo);
    termInfo.c_lflag |= ECHO;
    tcsetattr(STDIN_FD, TCSAFLUSH, &termInfo);

    // Once a valid passphrase is accepted run SHA1 over phrase to make Kpass
    shactx = EVP_MD_CTX_create();
    EVP_MD_CTX_init(shactx);
    EVP_DigestInit_ex(shactx, EVP_sha1(), NULL);
    EVP_DigestUpdate(shactx, phrase1, passLen);
    EVP_DigestFinal_ex(shactx, kPass, &sha1Len);

    // Check if SHA1 Length is valid
    if (sha1Len != DIGLEN)
    {
        printf("Error generating digest. Unexpected digest length %d.", sha1Len);
        exit(1);
    }

    // Print out Kpass in hexadecimal
    fprintf(stdout, "Kpass: ");
    printHex(stdout, kPass, sha1Len);
    fprintf(stdout, "\n");

    // Determine invocation and run accordingly
    if (mode == 1) // Encrypt
    {
        /// Testing
        printf("Beginning Encryption.\n");
        // Generate 128 bit random number Kenc from /dev/urandom
        initializeKey(kEnc, messLen);

        // Print out Kenc in hexadecimal
        fprintf(stdout, "Kenc:  ");
        printHex(stdout, kEnc, messLen);
        fprintf(stdout, "\n");

        // Encrypt datafile using Blowfish in CBC mode
        // Open datafile
        dataFile = open(argv[2], O_RDONLY);

        // Create encrypted dataFile name
        encFileName = malloc(strlen(argv[2]) + 5);
        strcpy(encFileName, argv[2]);
        strcat(encFileName, ".enc");
        printf("Writing encrypted file content to <%s>\n", encFileName);

        // Open encyrpted dataFile
        encFile = open(encFileName, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_APPEND, S_IRUSR | S_IWUSR);
        free(encFileName);

        // Set up Blow Fish algorithm
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
        EVP_CIPHER_CTX_set_key_length(ctx, KEYLEN);
        EVP_EncryptInit_ex(ctx, NULL, NULL, kEnc, ivec);
        ciphertext = allocateCiphertext(BUFSIZE);

        // Write encrypted data to <datafile>.enc
        while ((messLen = read(dataFile, buf, BUFSIZE)) > 0)
        {
            // Read from datafile
            printf("Read %d bytes of plaintext <", messLen);
            printHex(stdout, buf, messLen);
            printf(">\n");

            // Encrypt the data
            EVP_EncryptUpdate(ctx, ciphertext, &ctLen, buf, messLen);

            // Write the encrypted data to dataFile.enc
            write(encFile, ciphertext, ctLen);

            printf("Wrote %d bytes of ciphertext <", ctLen);
            printHex(stdout, ciphertext, ctLen);
            printf(">\n");
        }

        // Encrypt and write last block of data
        ctLen = 0;
        EVP_EncryptFinal_ex(ctx, ciphertext, &ctLen);
        write(encFile, ciphertext, ctLen);
        printf("Wrote %d bytes of ciphertext <", ctLen);
        printHex(stdout, ciphertext, ctLen);
        printf(">\n");

        // Clean up memory
        EVP_CIPHER_CTX_cleanup(ctx);
        free(ctx);
        close(dataFile);
        close(encFile);

        /// Testing output
        printf("Encrypting Kenc:\n");

        // If keyfile doesn't exist it is created
        keyFile = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_APPEND, S_IRUSR | S_IWUSR);

        // Use Kpass to encrypt Kenc
        // Set up Blow Fish algorithm
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
        EVP_CIPHER_CTX_set_key_length(ctx, DIGLEN);
        EVP_EncryptInit_ex(ctx, NULL, NULL, kPass, ivec);
        ciphertext = allocateCiphertext(KEYLEN);
        ctLen = 0;
        messLen = 0;
            
        // Encrypt Kenc
        EVP_EncryptUpdate(ctx, ciphertext, &ctLen, kEnc, BLOCKSIZE);

        // Write encrypted Kenc to keyfile
        write(keyFile, ciphertext, ctLen);
        printf("Wrote %d bytes of ciphertext <", ctLen);
        printHex(stdout, ciphertext, ctLen);
        printf(">\n");
        messLen += ctLen;

        ctLen = 0;
        EVP_EncryptFinal_ex(ctx, ciphertext, &ctLen);
        write(keyFile, ciphertext, ctLen);
        printf("Wrote %d bytes of ciphertext <", ctLen);
        printHex(stdout, ciphertext, ctLen);
        printf(">\n");
        messLen += ctLen;

        /// Testing
        printf("Kenc Length: %d\n", messLen);

        // Clean up memory
        close(keyFile);
        EVP_CIPHER_CTX_cleanup(ctx);
        free(ctx);

        /// Testing
        printf("Encryption complete.\n");

    }
    else if (mode == -1) // Decrpyt
    {
        // Get encrypted Kenc
        keyFile = open(argv[3], O_RDONLY | O_NOFOLLOW);
        
        fstat(keyFile, &fstats);
        ciphertext = (unsigned char *) malloc(fstats.st_size);
        read(keyFile, ciphertext, fstats.st_size);

        /// Testing
        printf("Decrypting Kenc: %d bytes\n");
        fprintf(stdout, "Encrypted Kenc: <");
        printHex(stdout, ciphertext, fstats.st_size);
        fprintf(stdout, ">\n");

        // Decrypt Kenc
        keyCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(keyCtx);
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctLen = fstats.st_size;
        if (EVP_DecryptInit_ex(keyCtx, cipher, NULL, kPass, ivec) == 0)
        {
            printf("Initial Decryption of Kenc Failed.\n");
        }
        
        // if (EVP_CIPHER_CTX_set_key_length(keyCtx, DIGLEN) == 0)
        // {
        //     printf("Setting Key Length Failed.\n");
        //     exit(1);
        // }

        /// Testing set key length
        //printf("Set Key Length: %d bytes\n", EVP_CIPHER_CTX_key_length(keyCtx));

        // if (EVP_DecryptInit_ex(keyCtx, NULL, NULL, kPass, ivec) == 0)
        // {
        //     printf("Initial Decryption of Kenc Failed.\n");
        //     exit(1);
        // }
        messLen = 0;
        res = (unsigned char *) malloc(ctLen);

        if (EVP_DecryptUpdate(keyCtx, res, &outLen, ciphertext, ctLen) == 0)
        {
            printf("Update Decryption of Kenc Failed.\n");
            exit(1);
        }
        messLen += outLen;

        printf("Kenc @ Outlen %d: <", outLen);
        printHex(stdout, &res[outLen], KEYLEN);
        printf(">\n");

        if (EVP_DecryptFinal_ex(keyCtx, &res[outLen], &outLen) == 0)
        {
            printf("Final Decryption of Kenc Failed.\n");
            exit(1);
        }
        messLen += outLen;

        // Print out Kenc in hexadecimal
        fprintf(stdout, "Decrypted Kenc: %d Bytes<", messLen);
        printHex(stdout, res, messLen);
        fprintf(stdout, ">\n");

        // Clean up memory
        close(keyFile);
        free(ciphertext);
        free(res);
        EVP_CIPHER_CTX_cleanup(keyCtx);
        free(keyCtx);

        // Open dataFile.enc
        encFile = open(argv[2], O_RDONLY | O_NOFOLLOW);
        fstat(encFile, &fstats);
        ciphertext = (unsigned char *) malloc(fstats.st_size);
        read(encFile, ciphertext, fstats.st_size);

        /// Testing
        fprintf(stdout, "Encrypted Datafile (HEX): <\n");
        printHex(stdout, ciphertext, fstats.st_size);
        fprintf(stdout, "\n>\n");

        // Decrypt dataFile.enc with Kenc
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        ctLen = fstats.st_size;
        EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL);
        printf("RET SETKEYLEN: %d\n", (EVP_CIPHER_CTX_set_key_length(ctx, KEYLEN)));
        EVP_DecryptInit_ex(ctx, NULL, NULL, kEnc, ivec);
        messLen = 0;
        res = (unsigned char *) malloc(ctLen);
        EVP_DecryptUpdate(ctx, res, &outLen, ciphertext, ctLen);
        messLen += outLen;
        EVP_DecryptFinal_ex(ctx, &res[outLen], &outLen);
        messLen += outLen;

        // Print out decrypted datafile in hexadecimal
        fprintf(stdout, "Decrypted Datafile (HEX): <\n");
        printHex(stdout, res, messLen);
        fprintf(stdout, "\n>\n");

        // Print out decrypted datafile in ascii
        fprintf(stdout, "Decrypted Datafile (ASCII): <\n%s\n>\n", res);

        // Clean up memory
        free(res);
        close(encFile);
        free(ciphertext);
        EVP_CIPHER_CTX_cleanup(ctx);
        free(ctx);
    }
    else
    {
        printf("Invalid mode. Mode must be '-e' or '-d'.\n");
        exit(1);
    }

}