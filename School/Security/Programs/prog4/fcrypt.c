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

#define KEYLEN    16
#define DIGLEN    20
#define BLOCKSIZE  8

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

void encryptAndPrint(EVP_CIPHER_CTX * ectx, char *msg, int mlen,
		  char *res, int *olen, FILE * f)
{
	int extlen;

	EVP_EncryptUpdate(ectx, res, olen, msg, mlen);
	EVP_EncryptFinal_ex(ectx, &res[*olen], &extlen);

	*olen += extlen;

	fprintf(f, "Encrypted result <");
	fprint_string_as_hex(f, res, *olen);
	fprintf(f, ">\n");
}

int main (int argc, char* argv[])
{
    struct termios termInfo;        // Struct for terminal echo

    EVP_CIPHER_CTX * ctx;           // Context of ecryption
    EVP_CIPHER     * cipher;        // Resulting Cipher

    char phrase1[80] = NULL;        // Holds the 1st passphrase from the user
    char phrase2[80] = NULL;        // Holds the 2nd passphrase from the user
    char ** digest   = NULL;        // Digest generated from passphrase

    unsigned char kPass[KEYLEN];    // Kpass generated from SHA1
    unsigned char kEnc[KEYLEN];     // Kenc generated from /dev/urandom

    int messLen = KEYLEN; // Length of message to encrypt
    int promting  = 1;    // Boolean for prompting loop
    int passLen = 0;      // Length of passphrase
    int mode = 0;         // Mode of application 1 = encrypt, -1 = decrypt
    int dataFile = 0;     // File descriptor for dataFile
    int keyFile = 0;      // File descriptor for keyFile
    int sha1Len = 0;      // Length of kPass generated from SHA1
    int ctxLen  = 0;      // Length of ciphertext

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
    tcgetattr(STDIN_FILENO, &termInfo);
    termInfo.c_flag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termInfo);

    // Loop for getting valid passphrase
    while (prompting = 1)
    {
        // Prompt user for passphrase
        printf("Enter a passphrase: ");
        scanf("%s", phrase1);

        // Prompt user for second passphrase
        printf("Verify passphrase: ");
        scanf("%s", phrase2);

        // If equal then check length
        if (strcmp(phrase1, phrase2) == 0)
        {
            // If in range of 10 to 80 characters break loop
            passLen = strlen(phrase1);
            if (passLen >= 10 && passLen <= 80)
            {
                prompting = 0;
            }
        }
    }

    // Turn on terminal echo
    tcgetattr(STDIN_FILENO, &termInfo);
    termInfo.c_flag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termInfo);

    // Once a valid passphrase is accepted run SHA1 over phrase to make Kpass
    mdctx = EVP_MD_CTX_create();
    EVP_MD_CTX_init(mdctx);
    EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(mdctx, phrase1, passLen);
    EVP_DigestFinal_ex(mdctx, kPass, &sha1Len);

    // Check if SHA1 Length is valid
    if (sha1Len != DIGLEN)
    {
        printf("Error generating digest. Unexpected digest length %d.", sha1Len);
        exit(1);
    }

    // Print out Kpass in hexadecimal
    fprintf(stdout, "Kpass: ");
    printHex(stdout, kPass, KEYLEN);
    fprintf(stdout, "\n");

    // Determine invocation and run accordingly
    if (mode == 1) // Encrypt
    {
        // Generate 128 bit random number Kenc from /dev/urandom
        initializeKey(kEnc, KEYLEN);

        // Print out Kenc in hexadecimal
        printHex(stdout, kEnc, KEYLEN);

        // Encrypt datafile using Blowfish in CBC mode
        ctx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
        EVP_CIPHER_CTX_init(ctx);
        cipher = (EVP_CIPHER *) EVP_bf_cbc();
        EVP_EncryptInit_ex(ctx, cipher, NULL, kEnc, ivec);
        mlen = KEYLEN;
        ciphertext = allocateCiphertext(mlen);
        encryptAndPrint(ctx, kPass, messLen)

        // Write encrypted data to <datafile>.enc
        

        // Make sure permission bits are set to 0400 for encrypted file

        // Use Kpass to encrypt Kenc

        // If keyfile doesn't exist it is created

        // If it does exist it is truncated to zero before writing

        // Make sure the file is not a symbolic link & the permission bits are 0400

        // Write encrypted Kenc to keyfile

    }
    else if (mode == -1) // Decrpyt
    {
        // Decrypt datafile

        // Print out decrypt key for keyfile to get Kenc in hexadecimal

        // Print out Kenc in hexadecimal

        // Print out decrypted datafile in hexadecimal

        // Print out decrypted datafile in ascii
    }
    else
    {
        printf("Invalid mode. Mode must be '-e' or '-d'.\n");
        exit(1);
    }

}