// -----------------------------------------------------------------------------
// Orig: 2019.10.12 - Alex Israels & Collin Palmer
// Revs: 2019.10.16 - Alex Israels & Collin Palmer
// Prog: showme.c
// Func: Takes a fileName as an argument and checks for an Access Control List
//       (ACL) with the fileName and a .acl extension. The ACL specifies what
//       users are granted permission to read the file. If they do have the
//       permissions to access the file then showme prints the contents of
//       fileName to stdout.
// Meth: Check if fileName exists and is ordinary file. Then check if a file
//       named fileName.acl exists and is owned by the showme binary owner.
//       (SUID will be set) Then verify group and world permissions are cleared
//       as they are not needed. Then check the ACL file for an entry that
//       specifies a username with the RUID of the showme process has access
//       to fileName. If all are met then the contents of fileName are echoed
//       to stdout.
// Args: fileName = The name of the file attempting to be accessed.
// -----------------------------------------------------------------------------

// Import Libraries: -----------------------------------------------------------
//  Names:         Purpose:
#include <stdlib.h>    // standard
#include <stdio.h>     // standard
#include <unistd.h>    // files
#include <fcntl.h>     // files
#include <string.h>    // string arithmetic
#include <sys/wait.h>  // credential management
#include <sys/types.h> // credential management
#include <sys/stat.h>  // credential management
#include <pwd.h>       // credential checking
// -----------------------------------------------------------------------------

void testFile(char * fileName, struct stat fileStat)
// -----------------------------------------------------------------------------
// Func: Tests to see if the file named fileName exists in the same directory.
// Args: fileName = Name of file attempting to be opened.
//       fileStat = Pointer to stat struct for fileName.
// -----------------------------------------------------------------------------
{
    // Test access of file:
    // Get stats:
    if (lstat(fileName, &fileStat) == -1)
    {   
        printf("Error: Getting stats for %s", fileName);
    }
    // Test file:
    if (S_ISREG(fileStat.st_mode) != 1)
    {   // If file DNE, then notify user and exit
        printf("%s does not exist.\n", fileName);
        free(fileName);
        exit(1);
    }
}
// -----------------------------------------------------------------------------

void printStats(char * fileName, struct stat fileStat)
// -----------------------------------------------------------------------------
// Func: Prints the stats of a file to stdout.
// Args: fileName = Name of the file in question.
//       fileStat = Pointer to stat structure for a file.  
// -----------------------------------------------------------------------------
{
    // Print general informaiton about file:
    printf("Information for %s\n", fileName);
    printf("---------------------------\n");
    printf("File Size: \t\t%d bytes\n", (int) fileStat.st_size);
    printf("Number of Links: \t%d\n", (int) fileStat.st_nlink);
    printf("File inode: \t\t%d\n", (int) fileStat.st_ino);
    printf("File Owner: \t\t%d\n", (int) fileStat.st_uid);

    // Print file permissions:
    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n\n");
}
// -----------------------------------------------------------------------------

int main(int argc, char ** argv)
// -----------------------------------------------------------------------------
// Func: The main process of the program. Checks for correct number of arguments
//       and proceeds to complete main functionality of the program as defined
//       in the openning comment block.
// Args: argc         = Number of arguments passed into program.
//       argv         = Array of arguments passed into program.
// Vars: aclFilePtr   = Name of ACL file attempting to be accessed.
//       aclFileStat  = Stat structure of ACL file.
//       euid         = EUID of current running process.
//       ruid         = Passwd struct containing the ruid and matching username.
//       permission   = Boolean for granting permission to read a file.
//       fd           = File descriptor for open().
//       bytes        = Checker for EOF when reading a file.
//       buf[1]       = Byte size buffer for moving characters btw files.
//       userBuf[256] = Buffer for ACL file contents.
//       userName     = Ptr to a username string.
//       userNames    = Array of ptrs to usernames.
//       i            = Index variable for loops.
//       userCount    = Number of usernames in ACL file.
//       ptrNewLn     = Ptr to \n in ACL to count # of users.
// -----------------------------------------------------------------------------
{
    // Initialize local variables ----------------------------------------------
    char * aclFilePtr = NULL;                  // ptr to name of ACL file
    struct stat aclFileStat;                   // ptr to stat struct of ACL file
    uid_t euid = geteuid();                    // EUID of current process
    struct passwd * ruid = getpwuid(getuid()); // RUID of user running proc.
    int permission = 0;                        // BOOL for granting permission
    int fd = 0;                                // File descriptor for open()
    int bytes = 0;                             // Checker for read from fd
    char buf[1];                               // Byte buffer for moving chars
    char userBuf[256];                         // Buffer for reading usernames
    char * userName = NULL;                    // Ptr to username str
    char ** userNames = NULL;                  // Arr for username ptrs
    int i = 0;                                 // Index var for loops
    int usrCount = 0;                          // Keep track of # of usrn in ACL
    char * ptrNewLn = NULL;                    // Ptr to \n in ACL for #of users

    bzero(userBuf, 256);                       // Clear userBuf
    // -------------------------------------------------------------------------

    // Begin - Main Process ====================================================

    // Begin - Error Checking --------------------------------------------------
    // Check for correct number of arguments:
    if (argc > 2 || argc < 2)
    {   // If anything but 1 argument is passed, then notify user and exit
        printf("Incorrect number of arguments:\nFormat: ./showme <fileName>\n");
        exit(1);
    }

    // Check if file exists: 
    testFile(argv[1], aclFileStat);

    // Build aclFilePtr:
    aclFilePtr = malloc(strlen(argv[1]) + strlen(".acl") + 1); // +4 is for .acl
    strcat(aclFilePtr, argv[1]);
    strcat(aclFilePtr, ".acl");
    printf("ACL: %s\n", aclFilePtr);

    // Check if ACL file exists:
    testFile(aclFilePtr, aclFileStat);

    // Check if ACL file has correct permissions:
    if (stat(aclFilePtr, &aclFileStat) < 0) // Get stats for ACL file
    {   // If something went wrong with getting stats, then notify user and exit
        printf("Something went wrong when getting stats for %s.\n", aclFilePtr);
        free(aclFilePtr);
        exit(1);
    }
    // End - Error Checking ----------------------------------------------------

    // Begin - Access ACL  -----------------------------------------------------
    // Print stats to console for TESTING:
    printStats(aclFilePtr, aclFileStat);

    // Compare file owners:
    if (euid != aclFileStat.st_uid)
    {
        printf("Access to ACL: Permission Denied\n");
        free(aclFilePtr);
        exit(1);
    }

    // Verify ACL group and world bits not set:
    if (aclFileStat.st_mode & 077)
    {
        printf("Group and world bits are set.\n");
        printf("Access Denied: %s\n", aclFilePtr);
        free(aclFilePtr);
        exit(1);
    }

    // Read ACL file:
    if ((fd = open(aclFilePtr, O_RDONLY)) == -1)
    {
        printf("Failed to open file %s.\n", aclFilePtr);
        free(aclFilePtr);
        exit(1);
    }
    i = 0;
    while ((bytes = read(fd, &buf, 1)) > 0)
    {
        if (buf[0] != ' ')        // Ignore all spaces
        {
            userBuf[i] = buf[0];  // Save everything including '\n' to parse ltr
            i++;
        }
    }
    close(fd);
    // End - Access ACL --------------------------------------------------------

    // Begin - Build Usernames -------------------------------------------------
    // Get number of usernames:
    ptrNewLn=strchr(userBuf,'\n');
    while (ptrNewLn!=NULL) {
        usrCount++;
        ptrNewLn=strchr(ptrNewLn+1,'\n');
    }

    // Allocate memory for usernames:
    userNames = malloc(sizeof(char*) * usrCount);

    // Fill usernames array:
    i = 0;
    userName = strtok(userBuf, "\n");
    while (userName != NULL)
    {
        userNames[i] = userName;
        userName = strtok(NULL, "\n");
        i++;
    }
    // End - Build Usernames ---------------------------------------------------

    // Begin - Compare Permissions ---------------------------------------------
    // Compare ruid with acl uids:
    printf("RUID: \t\t%s\n", ruid->pw_name);
    for (i=0; i < (usrCount - 1); i++)
    {
        printf("USRNAME: \t%s\n", userNames[i]);
        if (strcmp((ruid->pw_name),userNames[i]) == 0)
        {
            printf("Permision Granted\n");
            permission = 1;
        }
    }

    // If the user has permision echo out file to console:
    if (permission == 1)
    {
        fd = open(argv[1], O_RDONLY);
        printf("Opened File %s\n", argv[1]);
        while ((bytes = read(fd, &buf, 1)) > 0)
        {
            printf("%c",buf[0]);
        }
        printf("\n");
        close(fd);
    }
    else // Else deny user access to file
    {
        printf("Permission denied by ACL file.\n");
        free(aclFilePtr);
        free(userNames);
        exit(1);
    } 
    // End - Compare Permissions -----------------------------------------------

    // End - Main Process ======================================================

    // Begin - Cleanup =========================================================

    // Free dynamic memory:
    free(aclFilePtr);
    free(userNames);

    // Exit program:
    exit(0);

    // End - Cleanup ===========================================================    
}
// -----------------------------------------------------------------------------
