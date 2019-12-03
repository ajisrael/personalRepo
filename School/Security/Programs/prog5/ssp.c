//------------------------------------------------------------------------------
// Orig: 2019.11.20 - Alex Israels
// Revs: 2019.11.20 - Alex Israels
// Prog: ssp.c
// Func: Pools multiple files into one file.
// Reqm: 1. Does not spawn children
//       2. Only ordinary files are spooled:
//          - If an ordinary file contains a non-printalble character, it should
//            not be spooled.
//          - Printable: ASCII characters with decimal value between 32 and 126
//            inclusive, horizontal tab (9), newline (10), and carriage 
//            return (13) are all considered "printable".
//       3. Only files with a size <= 0.25 x (1000)^3 bytes should be spooled.
//       4. If a file cannot be read by the executing process, an entry is made
//          in slog and the program continues until all arguments have been
//          processed.
//       5. If a file cannot be spooled an entry is made in slog, but execution
//          continues until all arguments have been processed.
//       6. Performance is considered. The entire file should be read into
//          memory before searching for non-printable characters. Do not make a
//          system call to read and write each individual character.
//       7. Memory space to hold contents of a file being spooled must be 
//          dynamically allocated. If a call made to allocate space fails, an
//          entry is made in slog and the program should exit.
//       8. The names of all files spooled are appended to slog. Slog should be
//          created if it does not exist. (Owned by the EUID of executing ssp).
//          The program terminates before spooling any files if log file cannot
//          be written.
//       9. Slog is an ordinary file. Owned by the RUID of executing spooler.
//          Group and world permission bits are clear. If slog does not have the
//          right owner or the group and world permission bits are not clear,
//          the process terminates.
//      10. Openning files is done through the open system call. Reading and 
//          writing to a file is done through the read and write system calls.
// Secr: Secure coding guidelines to follow:
//       1. Process environment is cleared when program begins execution.
//       2. Process is not allowed to execute as root.
//       3. Return codes are checked and handled.
//       4. Umask is set to prevent group and world bits from being set on any
//          file that is created.
//       5. Files are opened with minimum required access.
//       6. Unused descriptors are closed when no longer in use. No open
//          descriptors on program exit.
//       7. No memory errors: 
//              - Accesses beyond bounds of a buffer
//              - Use after a free
//              - Memory leaks
//              - etc.
//          No dynamically allocated memory by process on program exit.
//       8. Log content is not specified, explaination required in README.
//       9. All other incorporated secure coding practices explained in README.
//      10. No integer overflow errors.
// Assm: No file spooled by this application contains sensitive data.
// Defn: MAXFILE = The maximum size of a spoolable file in bytes.
//       FLOPERR = Base length of file open error message to slog.
//       FLMAERR = Base length of file malloc error message to slog.
//       FLSZERR = Base length of file size error message to slog.
//       FLCHERR = Base length of file character error message to slog.
//       FLSTERR = Base length of file stat error message to slog.
//       SPOOLAD = Base length of successfull file add to spool msg to slog.
//       INVALID = Flag for when a process fails or file is invalid.
//       ERRFLAG = Flag for when a function errors.
//       MINPRNT = Minimum value of a printable character (inclusive).
//       MAXPRNT = Maximum value of a printable character (inclusive).
// Vars: gMan    = Global memory manager.
//------------------------------------------------------------------------------

#include <unistd.h>         // System data
#include <sys/types.h>      // System data
#include <sys/stat.h>       // System data
#include <sys/mman.h>       // Memory management
#include <stdlib.h>         // Memory management
#include <stdio.h>          // IO ops
#include <strings.h>        // IO ops
#include <string.h>         // File ops
#include <fcntl.h>          // File ops
#include <termios.h>        // Core management
#include <sys/time.h>       // Core management
#include <sys/resource.h>   // Core management

#define MAXFILE 250000000 // Maximum size of a spoolable file
#define FLOPERR        17 // Base length of file open error message to slog
#define FLMAERR        20 // Base length of file malloc error message to slog
#define FLSZERR        26 // Base length of file size error message to slog
#define FLCHERR        38 // Base length of file char error message to slog
#define FLSTERR        22 // Base length of file stat error message to slog
#define SPOOLAD        22 // Base length of successfull file add to spool
#define INVALID        -1 // Flag for when a process fails or file is invalid
#define ERRFLAG        -2 // Flag for when a function errors
#define MINPRNT        32 // Minimum value of a printable character
#define MAXPRNT       126 // Maximum value of a printable character


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

int checkFile(int fd, struct stat * fStats)
//------------------------------------------------------------------------------
// Name: checkFile
// Func: Checks a file to make sure it is valid to be spooled.
// Meth:
// Args: fd     = File descriptor for file to be checked.
//       fStats = Struct for getting metadata of a file.
// Retn: valid  = Status of the file, if it can be spooled or not.
//        0 = File is valid.
//       -1 = File is invalid.
//------------------------------------------------------------------------------
{
    int valid = 0; // Satus of validity of a file

    // Get file stats
    if (fstat(fd, fStats) == INVALID)
    {
        perror("fstat");
        valid = ERRFLAG;
    }

    // Check if not regular
    if (!(S_ISREG(fStats->st_mode)))
    {
        valid = INVALID;
    }

    return valid;
}

int main(int argc, char** argv)
//------------------------------------------------------------------------------
// Name: main
// Func: The main process of the program.
// Meth:
// Args: argc = The number of arguments passed into the program.
//       argv = Array of arguments passed into the program.
// Vars: fileStat = Struct for getting metadata of a file.
// Retn: 0 = Program ran as expected.
//       1 = An error occured.
//------------------------------------------------------------------------------
{
    struct stat * fileStat = NULL;  // Ptr to stat structure of a file

    uid_t uid  = getuid();   // UID of current process
    uid_t euid = geteuid();  // EUID of current process

    int slogFD  = 0;         // File descriptor for slog
    int spoolFD = 0;         // File descriptor for spool
    int currFD  = 0;         // File descriptor for current file
    int i       = 0;         // Index of file for loop
    int j       = 0;         // Index of character check for loop
    int valid   = 0;         // Determines whether or not a file is valid
    int stat    = 0;         // Return value from custom funciton calls
    int logLen  = 0;         // Length of entry in slog

    char * slog   = "slog";  // Name of slog file
    char * spool  = "spool"; // Name of spool file
    char * logBuf = NULL;    // Ptr to base addr of buffer for writing to slog
    char * fBuf   = NULL;    // Ptr to base addr of buffer for writing to spool

    // Check for correct input
    if (argc <= 1)
    {
        printf("Error: No files entered.\n");
        exit(1);
    }

    // Clear process environment
    if (clearenv() != 0)
    {
        perror("clearenv");
        exit(1);
    }

    // Set the core dump size to zero
    secureCoreDump();

    // Make sure process isn't running as root
    if (euid == 0 || uid == 0)
    {
        printf("root: ssp cannot be run as root.\n");
        exit(1);
    }

    // Set the umask to prevent group and world bits from being set
    umask(077);

    // Slog Setup Begin --------------------------------------------------------
    slogFD = open(slog, O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (slogFD == INVALID)
    {
        perror("slog_open");
        exit(1);
    }

    // Allocate memory for fileStat
    fileStat = malloc(sizeof(struct stat));
    if (fileStat == NULL)
    {
        perror("malloc_fstat");
        close(slogFD);
        exit(1);
    }

    // Verify file is regular and get its stats
    stat = checkFile(slogFD, fileStat);
    if (stat == ERRFLAG)
    {
        close(slogFD);
        free(fileStat);
        exit(1);
    }
    else if (stat == INVALID)
    {
        printf("slog_reg: File slog is not a regular file.\n");
        close(slogFD);
        free(fileStat);
        exit(1);
    }

    // Check IDs of the file
    if (uid != fileStat->st_uid)
    {
        printf("slog_uid: Uid's do not match.\n");
        close(slogFD);
        free(fileStat);
        exit(1);
    }

    // Check group and world bits
    if (fileStat->st_mode & 077)
    {
        printf("slog_gid: Group and world bits set.\n");
        close(slogFD);
        free(fileStat);
        exit(1);
    }
    // Slog Setup End ----------------------------------------------------------

    // Spool Setup Begin -------------------------------------------------------
    spoolFD = open(spool, O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (spoolFD == INVALID)
    {
        perror("spool_open");
        close(slogFD);
        free(fileStat);
        exit(1);
    }

    // Verify file is regular and get its stats
    stat = checkFile(slogFD, fileStat);
    if (stat == ERRFLAG)
    {
        close(slogFD);
        close(spoolFD);
        free(fileStat);
        exit(1);
    }
    else if (stat == INVALID)
    {
        printf("spool_reg: File spool is not a regular file.\n");
        close(slogFD);
        close(spoolFD);
        free(fileStat);
        exit(1);
    }

    // Check IDs of the file
    if (uid != fileStat->st_uid)
    {
        printf("spool_uid: Uid's do not match.\n");
        close(slogFD);
        close(spoolFD);
        free(fileStat);
        exit(1);
    }

    // Check group and world bits
    if (fileStat->st_mode & 077)
    {
        printf("spool_gid: Group and world bits set.\n");
        close(slogFD);
        close(spoolFD);
        free(fileStat);
        exit(1);
    }
    // Spool Setup End ---------------------------------------------------------

    // File Loop Begin ---------------------------------------------------------
    for (i = 1; i < argc; i++)
    {
        // Open file
        currFD = open(argv[i], O_RDONLY);

        // Check open call for errors and log if needed
        if (currFD == INVALID)
        {
            logLen = FLOPERR + strlen(argv[i]);
            logBuf = malloc(logLen * sizeof(char));
            if (logBuf == NULL)
            {
                close(slogFD);
                close(spoolFD);
                close(currFD);
                free(fileStat);
                exit(1);
            }
            if (sprintf(logBuf, "Failed to open %s.\n", argv[i]) < 0)
            {
                perror("slog_sprintf_open");
                close(slogFD);
                close(spoolFD);
                close(currFD);
                free(fileStat);
                free(logBuf);
                exit(1);
            }
            if (write(slogFD, logBuf, logLen) == INVALID)
            {
                perror("slog_write_open");
                close(slogFD);
                close(spoolFD);
                close(currFD);
                free(fileStat);
                free(logBuf);
                exit(1);
            }

            free(logBuf);
        }
        else
        {
            // Check if file is regular and get its stats
            stat = checkFile(currFD, fileStat);
            if (stat == ERRFLAG)
            {
                close(slogFD);
                close(spoolFD);
                close(currFD);
                free(fileStat);
                exit(1);
            }
            else if (stat == INVALID)
            {
                logLen = FLSTERR + strlen(argv[i]);
                logBuf = malloc(logLen * sizeof(char));
                if (logBuf == NULL)
                {
                    close(slogFD);
                    close(spoolFD);
                    close(currFD);
                    free(fileStat);
                    exit(1);
                }
                if (sprintf(logBuf, "File %s is not regular.\n", argv[i]) < 0)
                {
                    perror("slog_sprintf_regular");
                    close(slogFD);
                    close(spoolFD);
                    close(currFD);
                    free(fileStat);
                    free(logBuf);
                    exit(1);
                }
                if (write(slogFD, logBuf, logLen) == INVALID)
                {
                    perror("slog_write_regular");
                    close(slogFD);
                    close(spoolFD);
                    close(currFD);
                    free(fileStat);
                    free(logBuf);
                    exit(1);
                }

                free(logBuf);
            }
            else
            {
                // Check file size is less than MAXFILE
                if (fileStat->st_size > MAXFILE)
                {
                    logLen = FLSZERR + strlen(argv[i]);
                    logBuf = malloc(logLen * sizeof(char));
                    if (logBuf== NULL)
                    {
                        close(slogFD);
                        close(spoolFD);
                        close(currFD);
                        free(fileStat);
                        exit(1);
                    }

                    if (sprintf(logBuf, "File %s is too big to read.\n", argv[i]) < 0)
                    {
                        perror("slog_sprintf_size");
                        close(slogFD);
                        close(spoolFD);
                        close(currFD);
                        free(fileStat);
                        free(logBuf);
                        exit(1);
                    }

                    if (write(slogFD, logBuf, logLen) == INVALID)
                    {
                        perror("slog_write_size");
                        close(slogFD);
                        close(spoolFD);
                        close(currFD);
                        free(fileStat);
                        free(logBuf);
                        exit(1);
                    }

                    free(logBuf);
                }
                else
                {
                    // Allocate space to read file into memory
                    fBuf = malloc(fileStat->st_size);
                    if (fBuf == NULL)
                    {
                        logLen = FLMAERR + strlen(argv[i]);
                        logBuf = malloc(logLen * sizeof(char));
                        if (logBuf == NULL)
                        {
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            exit(1);
                        }
                        if (sprintf(logBuf, "Failed malloc for %s.\n", argv[i]) < 0)
                        {
                            perror("slog_sprintf_malloc");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            exit(1);
                        }
                        if (write(slogFD, logBuf, logLen) == INVALID)
                        {
                            perror("slog_write_malloc");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            exit(1);
                        }
                        close(slogFD);
                        close(spoolFD);
                        close(currFD);
                        free(fileStat);
                        free(logBuf);
                        exit(1);
                    }

                    // Read entire file into memory
                    if (read(currFD, fBuf, fileStat->st_size) == INVALID)
                    {
                        perror("file_read");
                        close(slogFD);
                        close(spoolFD);
                        close(currFD);
                        free(fileStat);
                        free(logBuf);
                        free(fBuf);
                        exit(1);
                    }

                    // Loop to look for non-printable characters
                    for (j = 0; j < fileStat->st_size; j++)
                    {
                        if (fBuf[j] == '\t'||fBuf[j] == '\n'||fBuf[j] == '\r')
                        {
                            continue;
                        }
                        else if (fBuf[j] >= 32 && fBuf[j] <= 126)
                        {
                            continue;
                        }
                        else
                        {
                            valid = INVALID;
                            break;
                        }
                    }

                    // If all are printable
                    if (valid != INVALID)
                    {
                        // Copy to spool
                        if (write(spoolFD, fBuf, fileStat->st_size) == INVALID)
                        {
                            perror("spool_write");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            free(fBuf);
                            exit(1);
                        }

                        // Add filename to slog
                        logLen = SPOOLAD + strlen(argv[i]);
                        logBuf = malloc(logLen * sizeof(char));
                        if (logBuf == NULL)
                        {
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(fBuf);
                            exit(1);
                        }
                        if (sprintf(logBuf, "File %s added to spool.\n", argv[i]) < 0)
                        {
                            perror("slog_sprintf_spool");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            free(fBuf);
                            exit(1);
                        }
                        if (write(slogFD, logBuf, logLen) == INVALID)
                        {
                            perror("slog_write_spool");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            free(fBuf);
                            exit(1);
                        }

                        free(logBuf);
                    }
                    else // If file cannot be spooled add to slog
                    {
                        logLen = FLCHERR + strlen(argv[i]);
                        logBuf = malloc(logLen * sizeof(char));
                        if (logBuf == NULL)
                        {
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(fBuf);
                            exit(1);
                        }
                        if (sprintf(logBuf, "File %s contained an invalid character.\n", argv[i]) < 0)
                        {
                            perror("slog_sprintf_inv_char");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            free(fBuf);
                            exit(1);
                        }
                        if (write(slogFD, logBuf, logLen) == INVALID)
                        {
                            perror("slog_write_inv_char");
                            close(slogFD);
                            close(spoolFD);
                            close(currFD);
                            free(fileStat);
                            free(logBuf);
                            free(fBuf);
                            exit(1);
                        }

                        free(logBuf);
                    }
                    // Free file contents in memory
                    free(fBuf);
                }
            }
            // Close current file descriptor
            close(currFD);
        }
    }
    // File Loop End -----------------------------------------------------------
    close(slogFD);
    close(spoolFD);
    free(fileStat);

    exit(0);
}
