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
// Assm: No file spooled by this application contains sensitive data.
// Defn: MAXFILE = The maximum size of a spoolable file in bytes.
//------------------------------------------------------------------------------

#include <sys/types.h> // System data
#include <sys/stat.h>  // System data
#include <unistd.h>    // System data
#include <stdlib.h>    // Memory management
#include <stdio.h>     // IO ops

#define MAXFILE 250000000 // Maximum size of a spoolable file

//------------------------------------------------------------------------------
struct memPair
{
    char * ptr;
    int status;
};

struct memManager
{
    struct memPair ptrs[256]; // Matrix of pointers to allocated memory & thier status
    int size;                 // # of allocated pointers
} gMan;
//------------------------------------------------------------------------------

int allocMem(char * ptr, int size)
//------------------------------------------------------------------------------
// Name: allocMem
// Func: Wrapper for global memory manager when mallocing memory.
// Meth: Mallocs size bytes of memory, sets base addr of ptr, and updates the 
//       global memory manager.
// Args: ptr  = Base address of newly allocated memory.
//       size = Size of newly allocated memory in bytes.
// Retn:  0 = Everything is good.
//       -1 = An error occured.
//------------------------------------------------------------------------------
{
    ptr = malloc(size);
    
    if (ptr == NULL)
    {
        perror("alloc");
        return -1;
    }

    gMan.ptrs[gMan.size].ptr = ptr;  // Add ptr to memory manager
    gMan.ptrs[gMan.size].status = 1; // Set status of ptr to alloced
    gMan.size++;

    return 0;
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

int checkFile(int fd, struct stat fStats)
//------------------------------------------------------------------------------
// Name: checkFile
// Func: Checks a file to make sure it is valid to be spooled.
// Meth:
// Args: fd     = File descriptor for file to be checked.
//       fStats = Struct for getting metadata of a file.
// Retn: valid  = Status of the file, if it can be spooled or not.
//        0 = File is valid.
//        1 = File is invalid.
//       -1 = An error occured.
//------------------------------------------------------------------------------
{
    int valid = 1; // Satus of validity of a file

    if (fstat(fd, &fStats) == -1)
    {
        perror("fstat");
        return valid = -1;
    }
    // Check if not regular
    if (!(S_ISREG(fStats.st_mode)))
    {
       return valid;
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
// Retn:
//------------------------------------------------------------------------------
{
    //struct stat fileStat;   // ptr to stat structure of a file
    char * ptr1 = NULL;
    char * ptr2 = NULL;
    char * ptr3 = NULL;

    gMan.size = 0;          // Size initalized to zero
    
    allocMem(ptr1, 10);
    printf("mem alocated\n");

    allocMem(ptr2, 4);
    printf("mem alocated\n");

    allocMem(ptr3, 40);
    printf("mem alocated\n");

    freeMem(ptr1);

    allocMem(ptr1, 100);
    printf("mem alocated\n");
    
    freeMem(NULL);
    printf("mem freed\n");

    exit(0);
}
