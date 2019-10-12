// -----------------------------------------------------------------------------
// Orig: 2019.10.12 - Alex Israels & Collin Palmer
// Prog: showme.c
// Func: Takes a filename as an argument and checks for an Access Control List
//       (ACL) with the filename and a .acl extension. The ACL specifies what
//       users are granted permission to read the file. If they do have the
//       permissions to access the file then showme prints the contents of
//       filename to stdout.
// Meth: Check if filename exists and is ordinary file. Then check if a file
//       named filename.acl exists and is owned by the showme binary owner.
//       (SUID will be set) Then verify group and world permissions are cleared
//       as they are not needed. Then check the ACL file for an entry that
//       specifies a username with the RUID of the showme process has access
//       to filename. If all are met then the contents of filename are echoed
//       to stdout.
// Args: filename = The name of the file attempting to be accessed.
// Vars: bufPtr  = Arbitrarily large buffer for holding string.
//       dataPtr = Pointer to data register.
//       ctrlPtr = Pointer to control register. 
//       dataVal = Value of the data register. Ensures data reg. is read once.
// Defn: BASE_ADDR   0xDEADBEEF = Base address of JTAG UART
//       RVALID_MASK 0x00008000 = Mask for the RVALID bit of the data register
//       WSPACE_MASK 0xFFFF0000 = Mask for the WSPACE bits of the cont. reg.
//       DATA_MASK   0x000000FF = Mask for the data bits of the data register
// -----------------------------------------------------------------------------



void main(int argc, char* argv)
{

}