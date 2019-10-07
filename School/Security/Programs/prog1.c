/* Headers */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

/* User Error Message */
void usrMsg()
{
    printf("canrun: invalid input\n");
    printf("Example: ./canrun bin1 bin2 ... binN\n");
    exit(0);
}

int main (int argc, char** argv){
    /* Begin Variables -----------------------------------------*/
    /* Loop counters */
    int i = 1;
    int j = 0;

    /* lstat vars */
    int status = -1;
    struct stat buf;
    
    /* arraylist of copied ARGS */
    char** bins = malloc(sizeof(char));
    int binSize = 0;
    int binMax = 1;
    int binPt = 0;
    
    /* vars for reading files */
    int fd = 0;
    char* readBuf = (char*) malloc(sizeof(int)*7);

    /* execve() vars */
    char *a[2], *e[3];
    /* End Variables -------------------------------------------*/

    /* Begin Process -------------------------------------------*/
    /* Verify ARGS exist */
    if (argv[1] == NULL)
    {
        /* If not output USR MSG */
        usrMsg();
    }

    /* Iterate over ARGV */
    while (argv[i] != NULL)
    {
        lstat(argv[i], &buf);
        /* Copy ARGS that have been verified regular and executable */
        if (S_ISREG(buf.st_mode) && 
        (((buf.st_mode & S_IRWXU) == S_IRWXU) || ((buf.st_mode & S_IXUSR) == S_IXUSR)) &&
        (((buf.st_mode & S_IRWXG) == S_IRWXG) || ((buf.st_mode & S_IXGRP) == S_IXGRP)) &&
        (((buf.st_mode & S_IRWXO) == S_IRWXO) || ((buf.st_mode & S_IXOTH) == S_IXOTH)))
        {
            /* Resize bins as nessesary */
            binSize++;
            if (binSize == binMax)
            {
                binMax *= 2;
                bins = realloc(bins, sizeof(char)*binMax);
            }

            /* Copy verified ARGS */
            bins[binPt] = malloc(sizeof(argv[i]));
            bins[binPt] = argv[i];
            binPt++;
        }
        i++;
    }
    
    /* If all are not executable output USR MSG */
    if (binSize == 0)
    {
        usrMsg();
    }

    /* Save environment variables for execve() */
    e[0] = "ENV0=val0";
    e[1] = "ENV0=val1";
    e[2] = NULL;

   /* Iterate array of ARGS and 
    * print 1st 7 bytes in DEC
    * print 1st 7 bytes in HEX
    * then execute binary   
    */ 
    for (i=0; i < binSize; i++)
    {
        fd = open(bins[i], O_RDONLY);
        status = read(fd, readBuf, sizeof(int)*7);
        if (status == -1)
        {
            perror("Read Failed");
        }

        /* Print 7 bytes in DEC */
        for (j = 0; j < 7 ; j++)
        {
            printf("%d ", *(readBuf + j));
        }
        printf("\n");

        /* Print 7 bytes in HEX */
        for (j = 0; j < 7 ; j++)
        {
            printf("%x ", *(readBuf + j));
        }
        printf("\n");

        /* Build argument list for execve() */
        a[0] = bins[i];
        a[1] = NULL;

        if (fork() == 0)
        {
            /* Execute binary */
            execve(bins[i], a, e);
            printf("%s failed to execute.\n", bins[i]);
            exit(0);
        }
        else
        {
            /* Wait for child to finish */
            wait(NULL);
        }
    }

    close(fd);

    /* Free memory for reading files */
    free(readBuf);

    /* Print ARGS */
    for (i=0; i < binSize; i++)
    {
        printf("%s\n", bins[i]);
    }

    /* Deallocate all Dynamic Mem */
    free(bins);
    
    /* End program */
    exit(0);
}
