// -----------------------------------------------------------------------------
// Orig: 2019.10.12 - Alex Israels & Collin Palmer
// Revs: 2019.10.17 - Alex Israels & Collin Palmer
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
//       usrInLn      = Number of users per line in the ACL file.
//       spc          = Marks last char as a space.
//       fl           = Boolean for first line.
//       nl           = Boolean for new line.
// -----------------------------------------------------------------------------
{
    // Initialize local variables ----------------------------------------------
    char * aclFilePtr = NULL;                  // ptr to name of ACL file
    struct stat fileStat;                      // ptr to stat struct of fileName
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
    int usrInLn = 0;                           // # of users/line in ACL file
    int spc = 0;                               // Marks last char of ACL as ' '
    int fl = 1;                                // Boolean for first line
    int nl = 0;                                // Boolean for new line

    bzero(userBuf, 256);                       // Clear userBuf
    // -------------------------------------------------------------------------

    // Begin - Main Process ====================================================
    
    /*=== BEGIN PRIVILEGE ===*/
    // Process begins with privilege, not required for lstating files and error
    // checking:
    seteuid(getuid());
    /*=== END PRIVILEGE ===*/

    // Begin - Error Checking --------------------------------------------------
    // Check for correct number of arguments:
    if (argc > 2 || argc < 2)
    {   // If anything but 1 argument is passed, then notify user and exit
        printf("Incorrect number of arguments:\nFormat: ./showme <fileName>\n");
        exit(1);
    }

    // Check if file exists: 
    testFile(argv[1], fileStat);

    // Build aclFilePtr:
    aclFilePtr = malloc(strlen(argv[1]) + strlen(".acl") + 1); // +4 is for .acl
    strcat(aclFilePtr, argv[1]);
    strcat(aclFilePtr, ".acl");

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

    // Compare ACL file owner:
    if (euid != aclFileStat.st_uid)
    {
        printf("Access to ACL: Permission Denied\n");
        free(aclFilePtr);
        exit(1);
    }

    // Compare owners between files:
    if (fileStat.st_uid != aclFileStat.st_uid)
    {
        printf("ACL File not created by %s owner: Permission Denied\n",argv[1]);
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

    /*=== BEGIN PRIVILEGE ===*/
    seteuid(euid);

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
        if (buf[0] == '\n')       // If new line
        {
            
            nl = 1;               // Mark new line
            usrInLn = 0;          // Clear flags
            spc = 0;
            fl = 0;
            userBuf[i] = buf[0];  // Save '\n' to parse later
            i++;
        }
        else if (buf[0] != ' ')   // Ignore all spaces
        {
            if ((spc > 0) || (fl > 0) || (nl > 0)) // If previous char was ' '
            {
                usrInLn ++;       // Increase # of users per ln
                spc = 0;          // Clear flags
                nl = 0;
                fl = 0;
            }

            if (usrInLn > 1)      // If more than one user/line
            {                     // Exit process
                printf("Error ACL file format: %s\n", aclFilePtr);
                close(fd);
                free(aclFilePtr);
                exit(1);
            }
            else                  // Else save char
            {
                userBuf[i] = buf[0];
                i++;
            }
        }
        else 
        {
            spc++;                // Mark last char as space
        }
    }
    close(fd);

    if (userBuf[i-2] != '\n')
    {
        printf("Error ACL file format: %s\n", aclFilePtr);
        free(aclFilePtr);
        exit(1);
    }

    /*=== END PRIVILEGE ===*/
    seteuid(getuid());

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
    for (i = 0; i < (usrCount - 1); i++)
    {
        if (strcmp((ruid->pw_name),userNames[i]) == 0)
        {
            permission = 1;
        }
    }

    // If the user has permision echo out file to console:
    if (permission == 1)
    {
        /*=== BEGIN PRIVILEGE ===*/
        seteuid(euid);
        fd = open(argv[1], O_RDONLY);
        while ((bytes = read(fd, &buf, 1)) > 0)
        {
            printf("%c",buf[0]);
        }
        printf("\n");
        close(fd);
        /*=== END PRIVILEGE ===*/
        seteuid(getuid());
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
