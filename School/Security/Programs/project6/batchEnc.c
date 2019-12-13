//
//  Useage: (1) fge file start end  OR
//          (2) fge -f listFile
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
/// CHANGE: Required library for secure core dump
#include <sys/resource.h>
#include <sys/time.h>
/// CHANGE: Added for getuid() and geteuid() function calls
#include <unistd.h>

#define NAMLEN 20

struct fdata
{
  char name[NAMLEN];
  /// CHANGE: Changed from unsigned short to int
  int start;
  int end;
};

/// CHANGE: Function added to check a file
int checkFile(char *name, struct stat *fStats, int * start, int * end)
//------------------------------------------------------------------------------
// Name: checkFile
// Func: Checks a file to make sure it is valid to be encrypted.
// Meth: Calls fstat on the file and sets valid to -2 if the call failed and -1
//       if the file is not regular. Otherwise valid stays at 0. It also checks
//       to make sure the program is not trying to encrypt beyond the size of
//       the file.
// Args: fd     = File descriptor for file to be checked.
//       fStats = Struct for getting metadata of a file.
//       max    = Furthest point in file trying to be encrypted
// Retn: valid  = Status of the file, if it can be spooled or not.
//        0 = File is valid.
//       -1 = File is invalid.
//       -2 = Fstat() errored in some way.
//------------------------------------------------------------------------------
{
  int valid = 0; // Satus of validity of a file
  int fd = 0;    // file descriptor of file name

  // Get the file discriptor
  fd = open(name, O_RDONLY | O_NOFOLLOW);
  if (fd == -1)
  {
    perror("open");
    valid = -2;
  }

  // Get file stats
  if (fstat(fd, fStats) == -1)
  {
    perror("fstat");
    valid = -2;
  }

  // Close file descriptor
  if (close(fd) == -1)
  {
    perror("close");
    valid = -2;
  }

  // Check if not regular
  if (!(S_ISREG(fStats->st_mode)))
  {
    printf("File Type: File %s is not a regular file. File %s will not be encrypted.\n", name, name);
    valid = -1;
  }

  // Make sure to not encrypt beyond the size of a file
  if (*start < 0)
  {
    printf("Starting offset of %s was < 0, value set to 0.\n", name);
    *start = 0;
  }

  if (*end > fStats->st_size)
  {
    printf("Ending offset of %s was < 0, value set to %d.\n", name, fStats->st_size);
    *end = fStats->st_size;
  }

  // Check for when start is greater or equal to end
  if (*start >= *end)
  {
    printf("Invocation: For file %s start >= end. File %s will not be encrypted.\n", name, name);
    valid = -1;
  }

  return valid;
}

/// CHANGE: Added function to set core dump to zero.
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

/// CHANGE: Changed from unsigned int to int
int pastWhite(int fd)
{
  char ch;
  int ret;

  ret = read(fd, &ch, 1);
  //-- Is line empty?
  if (ret == 0)
    return (0);
  if (ret < 0)
    return (2); /// CHANGE: From -1 to 2 b/c return value is type char

  //-- Past whitespace.  Stop if hit EOF or on error.
  for (; ch == 32 || ch == 9; ret = read(fd, &ch, 1))
  {
    if (ret == 0)
      return (0);
    if (ret < 0)
      return (2); /// CHANGE: From -1 to 2 b/c return value is type char
  }

  //--- Read some non-blank character.
  //    Position so that next character read is the non-blank
  lseek(fd, lseek(fd, 0, SEEK_CUR) - 1, SEEK_SET);

  //--- Non-blank was line return.  Sentinel return value of 1.
  if (ch == '\n')
    return (1);
  return (ch); //--- First non-blank character.
}

/// CHANGE: Return type from unsigned to int
int getFileList(struct fdata *fileList[], char *file)
{

  char num[5]; //-- Temporary storage for text value of input offset
  int fctr;    //-- Number of segments to encrypt
  int nctr;    //-- Characters in an offset
  int fd;      //-- Descriptor for list file
  char ch;     //-- Store a character while handling
  int ret;     //-- Return status

  nctr = 0;
  fctr = 0;
  /// CHANGE: O_RDWR to O_RDONLY remove O_CREAT  added O_NOFOLLOW
  fd = open(file, O_RDONLY | O_NOFOLLOW);
  if (fd == -1)
  {
    perror("fileList_open");
  }

START:

  //--- Past initial white space
  ch = pastWhite(fd);
  if (ch == 0)
    return (fctr); //-- Last line blank without '\n'
  /// CHANGE: Compares to 2 b/c ch is of type char not int
  if (ch == 2)
    return (-1); //-- Error
  if (ch == 1)
    return (fctr); //-- Last line blank with '\n'

  //-- Get the file name
  nctr = 0;
  for (read(fd, &ch, 1);
       ch != 32 && ch != 9 && ch != '\n';
       fileList[fctr]->name[nctr++] = ch, read(fd, &ch, 1))
    ;

  lseek(fd, lseek(fd, 0, SEEK_CUR) - 1, SEEK_SET);
  if (ch == '\n')
    return (1);
  fileList[fctr]->name[nctr] = 0;

  //-- Past white space
  ch = pastWhite(fd);
  if (ch == 0)
    return (-2); //-- EOF too soon
  /// CHANGE: Compares to 2 b/c ch is of type char not int
  if (ch == 2)
    return (-3); //-- Error
  if (ch == 1)
    return (-4); //-- Line return too soon

  //-- Get the first offset
  nctr = 0;
  for (read(fd, &ch, 1);
       ch != 32 && ch != 9 && ch != '\n';
       num[nctr++] = ch, read(fd, &ch, 1))
    ;
  lseek(fd, lseek(fd, 0, SEEK_CUR) - 1, SEEK_SET);

  if (ch == '\n')
    return (-5); //--- Missing last offset
  num[nctr] = 0;
  fileList[fctr]->start = atoi(num);

  //-- Past white space
  ch = pastWhite(fd);
  if (ch == 0)
    return (-6); //-- Line ended too soon
  if (ch < 0)
    return (-7); //-- Read error

  //-- Get the second offset
  nctr = 0;
  for (ret = read(fd, &ch, 1);
       (ch != 32 && ch != 9 && ch != '\n' && ret == 1);
       num[nctr++] = ch, ret = read(fd, &ch, 1))
    ;

  num[nctr] = 0;
  fileList[fctr]->end = atoi(num);
  if (ch == '\n')
  {
    fctr++;
    goto START;
  } //--- Last offset followed by ;\n'
  if (ret == 0)
    return (++fctr); //--- Last offset at end of file

  //--- Get trailing whitespace
  lseek(fd, lseek(fd, 0, SEEK_CUR) - 1, SEEK_SET);
  ch = pastWhite(fd);

  /// CHANGE: Compares to 2 b/c ch is of type char not int
  if (ch == 2)
    return (-1);
  if (read(fd, &ch, 1) == 0)
    return (fctr);
  if (ch == 10)
  {
    fctr++;
    goto START;
  }

  /// CHANGE: Close file descriptor and check for error
  if (close(fd) == -1)
  {
    perror("fileList_close");
    exit(1);
  }

  return (-1);
}

int encryptFile(char *file, int start, int end)
{
  char *plain;
  int fd;
  int len;

#ifdef DEBUG
  printf("Encrypting <%s> from %d to %d \n", file, start, end);
#endif
  plain = malloc(sizeof(char) * (end - start));
  fd = open(file, O_RDWR);
  /// CHANGE: Error check open
  if (fd == -1)
  {
    perror("encrypt_file_open");
    return -1;
  }

  /// CHANGE: Error check read
  if (read(fd, plain, end - start) == -1)
  {
    perror("encrypt_file_read");
    close(fd);
    return -1;
  }

  /// CHANGE: Error check close
  if (close(fd) == -1)
  {
    perror("encrypt_file_close");
    return -1;
  }

  /// CHANGE: Error check encryptWithPhrase
  if (encryptWithPhrase(plain, file, end - start) == 1)
  {
    free(plain);
    return -1;
  }
  else
  {
    free(plain);
    return 0;
  }
}

/// CHANGE: Easier way to free file list ptrs when handling errors
void freeFileList(struct fdata **fileList)
{
  int i = 0;
  for (i = 0; i < 10; i++)
  {
    free(fileList[i]);
  }
  free(fileList);
}

int main(int argc, char *argv[])
{
  /// CHANGE: Get stats of files for error checking
  struct stat * fileStat = NULL;  // Ptr to stat structure of a file
  int fileCheck = 0; // Return status of checkFile()

  struct fdata **fileList;
  int entries;
  int i;

  /// CHANGE: get uids of current process for preventing root from running
  uid_t uid = getuid();   // UID of current process
  uid_t euid = geteuid(); // EUID of current process

  /// CHANGE: Check input
  if (!(argc == 3 || argc == 4))
  {
    printf("Invocation: batchEnc file start end or -f file.\n");
    exit(1);
  }

  /// CHANGE: Set core dump size to zero
  secureCoreDump();

  /// CHANGE: Clear process environment
  if (clearenv() != 0)
  {
    perror("clearenv");
    exit(1);
  }

  /// CHANGE: Set the umask to prevent group and world bits from being set
  umask(077);

  /// CHANGE: Prevent process from running as root
  if (euid == 0 || uid == 0)
  {
    printf("root: ssp cannot be run as root.\n");
    exit(1);
  }

  //-- Create space for an array of structure pointers.
  //   Each array element points to information for encryption of one segment
  fileList = malloc(10 * sizeof(struct fdata *));
  for (i = 0; i < 10; i++)
  {
    fileList[i] = malloc(sizeof(struct fdata));
  }

  //-- Branch based on whether user ones one segment encrypted or has a list
  if (strcmp("-f", argv[1]) == 0)
  {
    entries = getFileList(fileList, argv[2]);

    //CHANGE: Added switch statement for looking error messages of getFileList
    switch (entries)
    {
    case -1:
      perror("ch < 1");
      freeFileList(fileList);
      exit(1);
      break;
    case -2:
      perror("EOF too soon");
      freeFileList(fileList);
      exit(1);
      break;
    case -3:
      perror("second ch < 1");
      freeFileList(fileList);
      exit(1);
      break;
    case -4:
      perror("Line return too soon");
      freeFileList(fileList);
      exit(1);
      break;
    case -5:
      perror("Missing last offset");
      freeFileList(fileList);
      exit(1);
      break;
    case -6:
      perror("Line ended too soon");
      freeFileList(fileList);
      exit(1);
      break;
    case -7:
      perror("Read error");
      freeFileList(fileList);
      exit(1);
      break;
    }
  }
  else
  {
    entries = 1;
    /// CHANGE: Removed double malloc and meaningless strlen
    /// fileList[0] = malloc(sizeof(struct fdata));
    /// strlen(argv[1]);

    strcpy(fileList[0]->name, argv[1]);
    fileList[0]->start = atoi(argv[2]);
    fileList[0]->end = atoi(argv[3]);
  }

  //-- The encryption
  /// CHANGE: Freeing file list on error
  fileStat = malloc(sizeof(struct stat));
  for (i = 0; i < entries; i++)
  {
    /// CHANGE: Checking input parameters to encrypt file.
    fileCheck = checkFile(fileList[i]->name, fileStat, &fileList[i]->start, &fileList[i]->end);
    if (fileCheck == -2)  // Check for error of a function call
    {
      freeFileList(fileList);
      free(fileStat);
      exit(1);
    }
    if (fileCheck == 1)  // Check for improper invocation
    {
      continue;
    }
    if (encryptFile(fileList[i]->name, fileList[i]->start, fileList[i]->end) == -1)
    {
      freeFileList(fileList);
      free(fileStat);
      exit(1);
    }
  }
  /// CHANGE: freeing filelist and fileStat
  freeFileList(fileList);
  free(fileStat);

  /// CHANGE: exit with good code
  exit(0);
}
