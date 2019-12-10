//
//  Useage: (1) fge file start end  OR  
//                (2) fge -f listFile
//
//  When invoked as fge file start end, it encrypts the contents of the file 
//  named on the command line starting at the offset start and ending and the
//  offset end.  The encrypted content is written to a file named file.enc
//
//  When invoked as fge -f listFile, it read entries with the format 
//  "file start end" from listFile and performs encryption as described 
//  above. 
//
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
struct fdata{
  char name[20];
  unsigned short start;
  unsigned short end;
};

 

unsigned int pastWhite(int fd){
     char ch;
     int ret;

    
     ret=read(fd,&ch,1);
     //-- Is line empty?
     if (ret==0)   return(0);
     if (ret<0)    return(-1);

     //-- Past whitespace.  Stop if hit EOF or on error.     
     for(;ch==32||ch==9;ret=read(fd,&ch,1)){
         if (ret==0)   return(0);
         if (ret<0)    return(-1);
     }

     //--- Read some non-blank character.  
     //    Position so that next character read is the non-blank
     lseek(fd, lseek(fd,0,SEEK_CUR)-1, SEEK_SET);

     //--- Non-blank was line return.  Sentinel return value of 1.
     if (ch=='\n') return(1);       
     return(ch);  //--- First non-blank character.

}
unsigned getFileList(struct fdata *fileList[], char *file){

     char num[5];   //-- Temporary storage for text value of input offset
     int fctr;      //-- Number of segments to encrypt
     int nctr;      //-- Characters in an offset
     int fd;        //-- Descriptor for list file
     char ch;       //-- Store a character while handling 
     int ret;       //-- Return status 
     
     nctr=0;fctr=0;
     fd=open(file,O_RDWR|O_CREAT,644);

START:

    //--- Past initial white space
    ch=pastWhite(fd);
    if (ch==0) return(fctr); //-- Last line blank without '\n'
    if (ch<0) return(-1);    //-- Error
    if (ch==1) return(fctr); //-- Last line blank with '\n'
    
    //-- Get the file name
    nctr=0;
    for (read(fd,&ch,1);
         ch!=32&&ch!=9&&ch!='\n';
         fileList[fctr]->name[nctr++]=ch,read(fd,&ch,1));
    
    lseek(fd,lseek(fd,0,SEEK_CUR)-1,SEEK_SET);
    if (ch=='\n') return(1);
    fileList[fctr]->name[nctr]=0;

    
    //-- Past white space
    ch=pastWhite(fd);
    if (ch==0)return(-2);  //-- EOF too soon
    if (ch<0) return(-3);  //-- Error
    if (ch==1) return(-4); //-- Line return too soon

    //-- Get the first offset
    nctr=0;
    for (read(fd,&ch,1);
         ch!=32&&ch!=9&&ch!='\n';
         num[nctr++]=ch,read(fd,&ch,1));
    lseek(fd,lseek(fd,0,SEEK_CUR)-1,SEEK_SET);
    
    if (ch=='\n') return(-5);   //--- Missing last offset
    num[nctr]=0;
    fileList[fctr]->start=atoi(num);


    //-- Past white space
    ch=pastWhite(fd);
    if (ch==0)return(-6);  //-- Line ended too soon
    if (ch<0) return(-7);  //-- Read error

    //-- Get the second offset
    nctr=0;
    for (ret=read(fd,&ch,1);(ch!=32&&ch!=9&&ch!='\n'&&ret==1);num[nctr++]=ch,ret=read(fd,&ch,1));
    num[nctr]=0; 
    fileList[fctr]->end=atoi(num);
    if (ch=='\n') {fctr++;goto START;} //--- Last offset followed by ;\n'
    if (ret==0) return(++fctr);        //--- Last offset at end of file
        
    //--- Get trailing whitespace
    lseek(fd,lseek(fd,0,SEEK_CUR)-1,SEEK_SET);
    ch=pastWhite(fd);
    if (ch<0) return(-1);
    if (read(fd,&ch,1)==0) return(fctr);
    if (ch==10) {fctr++;goto START;}
    return(-1);
}
int encryptFile(char *file, int start, int end)
{
  char *plain;
  int fd;
  int len;
 
#ifdef DEBUG 
  printf("Encrypting <%s> from %d to %d \n",file,start,end);  
#endif
  plain=malloc(sizeof(char)*(end-start));
  fd=open(file,O_RDWR);
  read(fd,plain,end-start);
  encryptWithPhrase(plain,file,end-start);
}

int main(int argc, char *argv[]){

  struct fdata **fileList;
  int entries; 
  int i;


  //-- Create space for an array of structure pointers.
  //   Each array element points to information for encryption of one segment
  fileList=malloc(10*sizeof(struct fdata *));
  for (i=0;i<10;i++) fileList[i]=malloc(sizeof(struct fdata));
  
  //-- Branch based on whether user ones one segment encrypted or has a list
  if (strcmp("-f",argv[1])==0){
    entries=getFileList(fileList,argv[2]);
  } else {
     entries=1;
     fileList[0]=malloc(sizeof(struct fdata));
     strcpy(fileList[0]->name,argv[1]);     
     fileList[0]->start=atoi(argv[2]);
     fileList[0]->end=atoi(argv[3]);
  }
 
  //-- The encryption
   for (i=0;i<entries;i++)
       encryptFile(fileList[i]->name,fileList[i]->start,fileList[i]->end);
}

