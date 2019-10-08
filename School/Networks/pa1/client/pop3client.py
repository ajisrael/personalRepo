# ==========================================================================BOF
# Orig: 2019.10.05 - Alex Israels
# Func: Acts as the client to a POP3 server communicating over TCP connection.
# Args: serverName   = The name of the POP3 server (local machine = localhost)
#       serverPort   = Port for establishing connection to POP3 server
#       clientSocket = Socket connection to server AF_NET = IPv4 & STREAM = TCP
#       command      = Message to be sent to server.
#       response     = Message recieved from server. 
#       email        = Formatted response split on 'FILENAME:' if it exists.
#       content      = Content for client console = email[0].
#       fileName     = Name of email file to be saved = email[1].
#       file         = File object to write email content to.
# -----------------------------------------------------------------------------

### Import Required Libraries ###
from socket import *
import sys
import os

### Declare Global Variables ###
localStore = {    # Hashtable for email store data
   'size'  : 0 ,  # Size in bytes of all emails in store
   'count' : 0    # Total number of emails in store
}

### Define each command as a function ###
def loadStore():
   #---------------------------------------------------------------------------
   # Func: Loads information about emails in the current directory into a
   #       hash table to help speed up other functions.
   # Args: None
   # Vars: curDir  = Current working directory of server.
   #       store   = Global hash table to hold all email data.
   #       count   = The number of emails in the email store.
   #       size    = The total size of all emails in bytes.
   #       path    = The combined path of the file and the cwd.
   #       file    = The name of the file in the store.
   #       i       = Email Number
   # Retn: None
   #---------------------------------------------------------------------------
   
   curDir = os.getcwd()   # Current working directory
   localStore['size'] = 0      # Reinitialize size of store to 0
   localStore['count'] = 0     # Reinitialize count of store to 0
   i = 1                  # Reinitialize email number to 1

   for file in os.listdir(curDir):                    # Loop through cwd
      path = os.path.join(curDir, file)               # Save the entire path
      if os.path.isfile(path) and ".eml" in file:     # If it is an email file
         localStore[file] = (i, os.path.getsize(path))   # Save new files & sizes
         localStore['count'] += 1                        # Increase count by 1
         localStore['size'] += os.path.getsize(path)     # Increase size
         i += 1                                          # Increase email number

def getEmail(emailNum):
   #---------------------------------------------------------------------------
   # Func: Finds an email in the email store
   # Args: emailNum = number of email in store
   # Retn: file     = The name of the file in the store.
   #---------------------------------------------------------------------------
   global localStore

   for file in localStore.keys():                  # Loop through emails
      if not(file in 'count' or file in 'size'):   # Filter count & size
         if localStore[file][0] == emailNum:       # Find requested email
            return file                            # Return the email


def printEmail(emailNum):
   #---------------------------------------------------------------------------
   # Func: Retrieves a selected email via its number and returns it in the 
   #       form:
   #           "+OK <size> octets\n"
   #           "--- all message headers and message ---\n"
   #           "."
   # Args: emailNum = The number of the email to be returned to the client.
   # Vars: store    = Global hash table holding all email data.
   #       file     = Name of email to be returned
   # Retn: message  = Message, headers, and email to be returned to client.
   #---------------------------------------------------------------------------

   file = getEmail(emailNum)                    # Find Email
   with open(file, 'r') as email:               # Open the email
      message = email.read()                    # Add email to message
   email.close()                                # Close the email
   return message                               # Return the message

def connectToServer(args):
   # Load the server name and server port
   # Default serverName = 'localhost'
   # Default serverPort = 25783
   if len(args) > 2:
      serverName = args[1]
      serverPort = int(args[2])
   elif len(args) == 2:
      serverName = args[1]
      serverPort = 25783
   else:
      serverName = 'localhost'
      serverPort = 25783
   # Make sure it is not a well known port
   if serverPort <= 1024:
      print('Do not mess with "well known ports" please.')
      exit()

   # Open Socket
   clientSocket = socket(AF_INET, SOCK_STREAM)
   clientSocket.connect((serverName,serverPort))

   # Send welcome command
   command = 'HI'
   clientSocket.send(command.encode())

   return clientSocket

### Start Main Process ###
clientSocket = connectToServer(sys.argv)

# Load and print welcome response
response = clientSocket.recv(2048)
print(response.decode())
connected = True
running = True
loadStore()
# Start and maintain conversation until 'QUIT' command
while running:
   while connected:
      command = input()                               # Ask for next command
      clientSocket.send(command.encode())             # Send command
      response = clientSocket.recv(2048)              # Open response
      email = response.decode().split('FILENAME:')    # Split on 'FILENAME:' 
      content = email[0]                              # Get content for client
      print(content)                                  # Print client content

      if 'RETR' in command.upper():                      # If RETR command
         fileNum = command.split(' ')[1]                 # Capture the file number
         fileName = email[1]                             # Get fileName
         localStore[fileName] = fileNum                  # Save to the local store
         file = open(fileName, 'w')                      # Open file to write
         content = content.split('octets\n')             # Remove server info
         file.writelines(content[1][:len(content[1])-1]) # Write email to file
         file.close()                                    # Close the file

      if command.upper() == 'QUIT':
         connected = False

   clientSocket.close()

   while not connected:
      command = input()
      if 'PRINT' in command.upper():
         loadStore()
         args = command.split(' ')
         if len(args) > 1:
            print(printEmail(int(args[1])))
         else:
            print("Incorrect command: PRINT <messageNumber>")
      elif 'CONN' in command.upper():
         args = command.split(' ')
         clientSocket = connectToServer(args)
         connected = True
      elif 'EXIT' in command.upper():
         print("Have a nice day.")
         running = False
         exit()
      elif 'HELP' in command.upper():
         print("PRINT <emailNumber> : Print email to console.")
         print("CONN <serverName> <serverPort> : Connect to server.")
         print("EXIT : Exit client.")
      else:
         print("Command " + command.upper() + " does not exist:")
         print("Type HELP for list of commands.")

