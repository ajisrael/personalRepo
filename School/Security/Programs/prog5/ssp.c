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
//       MEMSIZE = The maximum number of memory pairs in memory manager.
// Vars: gMan    = Global memory manager.
//------------------------------------------------------------------------------

#include <unistd.h>         // System data
#include <sys/types.h>      // System data
#include <sys/stat.h>       // System data
#include <sys/mman.h>       // Memory management
#include <stdlib.h>         // Memory management
#include <stdio.h>          // IO ops
#include <strings.h>        // IO ops
#include <fcntl.h>          // File ops
#include <termios.h>        // Core management
#include <sys/time.h>       // Core management
#include <sys/resource.h>   // Core management

#define MAXFILE 250000000 // Maximum size of a spoolable file
#define MEMSIZE        64 // Maximum number of memory pairs in memory manager

//------------------------------------------------------------------------------
struct memPair     // Touple of a ptr to memory and its status
{
    char * ptr;    // Ptr to base addr of allocated memory
    char   status; // Status of base addr: 1 = alloced, 0 = freed
};

struct memManager             // Structure to better manage memory
{
    struct memPair ptrs[MEMSIZE]; // Array of memory pairs
    int size;                     // # of allocated pointers
} gMan; // Global memory manager
//------------------------------------------------------------------------------

void initMemManager()
//------------------------------------------------------------------------------
// Name: initMemManager
// Func: Initializes the base size of the memory manager.
// Meth: Mallocs ptrNum memPairs & sets base addr of the memory manager.
// Args: ptrNum = Number of memPairs to allocate.
//------------------------------------------------------------------------------
{
    //gMan.ptrs = malloc(ptrNum * sizeof(struct memPair));
    // if (gMan.ptrs == NULL)
    // {
    //     perror("mem_manager_init");
    //     exit(1);
    // }
    gMan.size = 0;            // Size of memory manager initalized to zero
}

void freeMemManager()
//------------------------------------------------------------------------------
// Name: freeMemManager
// Func: Frees memory manager ptr.
//------------------------------------------------------------------------------
{
    //free(gMan.ptrs);    // Free base addr of memory manager
    gMan.size = 0;      // Reset size to 0
}

int allocMem(char * ptr, int size)
//------------------------------------------------------------------------------
// Name: allocMem
// Func: Wrapper for global memory manager when mallocing memory.
// Meth: Mallocs size bytes of memory, sets base addr of ptr, and updates the 
//       global memory manager.
// Args: ptr  = Base address of newly allocated memory.
//       size = Size of newly allocated memory in bytes.
// Retn: stat = Status of the function call.
//          0 = Everything is good.
//         -1 = An error occured.
//------------------------------------------------------------------------------
{
    int stat = 0;           // Status of the funciton call

    ptr = malloc(size);     // Allocate memory
    
    if (ptr == NULL)        // Check for error
    {
        perror("alloc");    // Set perror
        stat = -1;          // Set status
    }
    else
    {
        gMan.ptrs[gMan.size].ptr = ptr;  // Add ptr to memory manager
        gMan.ptrs[gMan.size].status = 1; // Set status of ptr to alloced
        gMan.size++;  // Increment # of alloced pointers
        stat = 0;     // Set status
    }

    return stat;      // return status            
}

int reallocMem(char * ptr, int size)
//------------------------------------------------------------------------------
// Name: reallocMem
// Func: Wrapper for global memory manager when reallocing memory.
// Meth: Reallocs size bytes of memory, sets new base addr of ptr, and updates 
//       the global memory manager.
// Args: ptr   = Base address of newly allocated memory.
//       size  = Size of newly allocated memory in bytes.
// Vars: found = Determines if a pointer was found in the memory manager.
//       i     = Index of for loop.
//       rePtr = Ptr to newley allocated memory.
// Retn: stat  = Status of the function call.
//          0  = Everything is good.
//         -1  = An error occured.
//------------------------------------------------------------------------------
{
    int stat  =  0;      // Status of the function call
    int found = -1;      // Determines if a ptr was found in memory manager
    int i     =  0;      // Index of for loop
    char * rePtr = NULL; // Ptr to newly allocated memory
    
    for (i = 0; i < gMan.size; i++)         // Loop through ptrs in gMan
    {   
        if (ptr == gMan.ptrs[i].ptr)        // Find ptr in memory manager
        {  
            found = 0;                      // Mark found 
            rePtr = realloc(ptr, size);     // Reallocate memory
            if (rePtr == NULL)              // Check for error
            {
                perror("realloc");          // Set perror
                stat = -1;                  // Set status
            }
            else                            // If no error
            {
                gMan.ptrs[i].ptr = rePtr;   // Update ptr in memory manager
                gMan.ptrs[i].status = 1;    // Update status in memory manager
                i = gMan.size;              // Break loop
            }
        }
    }

    if (found == -1)  // If ptr was not found in memory manager
    {
        stat = allocMem(ptr, size);         // Alloc ptr and set status
    }

    return stat;      // return status            
}

void freeMem(char * ptr)
//------------------------------------------------------------------------------
// Name: freeMem
// Func: Wrapper for global memory manager when freeing memory.
// Meth: Frees ptr from memory and updates global memory manager. If ptr = NULL
//       then it frees all dynamically allocated memory.
// Args: ptr = Base address of memory to free.
// Vars: i   = Index of for loop.
//------------------------------------------------------------------------------
{
    int i = 0; // Index of for loop

    if (ptr != NULL) // Check for specific ptr
    {
        for (i = 0; i < gMan.size; i++)  // Find ptr in memory manager
        {
            if (ptr == gMan.ptrs[i].ptr)
            {
                free(ptr);               // Free ptr from memory
                gMan.ptrs[i].status = 0; // Set status of ptr to freed
                i = gMan.size;           // Break loop
            }
        }
    }
    else // Free all malloced ptrs
    {
        for (i = 0; i < gMan.size; i++) // Loop through all ptrs
        {
            if (gMan.ptrs[i].status == 1)   // If ptr is allocated
            {
                free(gMan.ptrs[i].ptr);     // Free ptr
                gMan.ptrs[i].status = 0;    // Set status of ptr to freed
            }
        }
        gMan.size = 0;                  // Reset size of allocated mem to 0
    }
}

int lockMemory(char * addr, size_t size) {
// -----------------------------------------------------------------------------
// Name: lockMemory
// Func: Locks pages in memory at address addr for size / pageSize pages.
// Meth: Gets the system page size, caclulates the number of pages required, and
//       locks them.
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

int checkFile(int fd, struct stat fStats)
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
    if (fstat(fd, &fStats) == -1)
    {
        perror("fstat");
        freeMem(NULL);
        close(fd);
        exit(1);
    }

    // Check if not regular
    if (!(S_ISREG(fStats.st_mode)))
    {
        valid = -1;
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
    int test = 1;

    struct stat fileStat;   // Ptr to stat structure of a file

    uid_t uid = getuid();   // UID of current process

    int slogFD  = 0;        // File descriptor for slog
    int spoolFD = 0;        // File descriptor for spool
    //int currFD  = 0;        // File descriptor for current file
    int i       = 0;        // Index of for loop

    char * slog  = "slog";  // Name of slog file
    char * spool = "spool"; // Name of spool file

    // Check for correct input
    if (argc <= 1)
    {
        printf("Error: No files entered.\n");
        exit(1);
    }


    /// Test print
    if (test == 1) {printf("UID: %d\n", uid);}

    // Clear process environment

    // Make sure process isn't running as root

    // Set the umask to prevent group and world bits from being set
    umask(077);

    // Slog Setup Begin --------------------------------------------------------
    slogFD = open(slog, O_WRONLY | O_APPEND | O_CREAT);
    if (slogFD == -1)
    {
        perror("slog");
        exit(1);
    }

    // Verify file is regular and get it's stats
    if (checkFile(slogFD, fileStat) == -1)
    {
        printf("slog_reg: File slog is not a regular file.\n");
        close(slogFD);
        exit(1);
    }

    /// Test print
    if (test == 1) {printf("UID: %d\n", fileStat.st_uid);

    // Check IDs of the file
    if (uid != fileStat.st_uid)
    {
        printf("slog_uid: Uid's do not match.\n");
        close(slogFD);
        exit(1);
    }

    // Check group and world bits
    if (fileStat.st_mode & 077)
    {
        printf("slog_gid: Group and world bits set.\n");
        close(slogFD);
        exit(1);
    }
    // Slog Setup End ----------------------------------------------------------

    // Spool Setup Begin -------------------------------------------------------
    spoolFD = open(spool, O_WRONLY | O_APPEND | O_CREAT);
    if (spoolFD == -1)
    {
        perror("spool");
        close(slogFD);
        exit(1);
    }

    // Verify file is regular and get it's stats
    if (checkFile(slogFD, fileStat) == -1)
    {
        printf("spool_reg: File spool is not a regular file.\n");
        close(slogFD);
        close(spoolFD);
        exit(1);
    }

    // Check IDs of the file
    if (uid != fileStat.st_uid)
    {
        printf("spool_uid: Uid's do not match.\n");
        close(slogFD);
        close(spoolFD);
        exit(1);
    }

    // Check group and world bits
    if (fileStat.st_mode & 077)
    {
        printf("spool_gid: Group and world bits set.\n");
        close(slogFD);
        close(spoolFD);
        exit(1);
    }
    // Spool Setup End ---------------------------------------------------------

    initMemManager();       // Initialize memory manager

    // Start looping through files
    for (i = 1; i < argc; i++)
    {
        // Open file

        // Check file is ordinary

            // Check file size is less than MAXFILE

                // If valid read entire file into memory
                
                    // If malloc fails entry made to slog and exit program

                // Loop to look for non-printable characters

                // If all are printable copy to spool

                // Add filename to slog

        // If file cannot be spooled add to slog

    
    }
    
    exit(1);
}
