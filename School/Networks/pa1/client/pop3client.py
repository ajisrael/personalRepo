# ==========================================================================BOF
# Orig: 2019.10.05 - Alex Israels
# Revs: 2019.10.07 - Alex Israels
# Func: Acts as the client to a POP3 server communicating over TCP connection.
#       It supports the following commands:
#           - PRINT: Prints local email to console.
#           - CONN:  Allows client to reconnect to server w/o restarting.
#           - EXIT:  Terminates client process.
#           - HELP:  Provides a list of client commands.
# Args: serverName   = The name of the POP3 server (local machine = localhost)
#       serverPort   = Port for establishing connection to POP3 server
# Vars: clientSocket = Socket connection to server AF_NET = IPv4 & STREAM = TCP
#       command      = Message to be sent to server.
#       response     = Message recieved from server. 
#       email        = Formatted response split on 'FILENAME:' if it exists.
#       content      = Content for client console = email[0].
#       args         = Arguments for client processes.
#       connected    = Boolean for connected status with POP3 server.
#       running      = Boolean for when client is running.
# Retn: None
# -----------------------------------------------------------------------------

### Import Required Libraries ### ---------------------------------------------

from socket import * # Socket lib
import sys           # Argument lib
import os            # File lib

# -----------------------------------------------------------------------------

### Define Global Variables ### -----------------------------------------------

localStore = {    # Hashtable for email store data
   'size'  : 0 ,  # Size in bytes of all emails in store
   'count' : 0    # Total number of emails in store
}

# -----------------------------------------------------------------------------

### Define each command as a function ### -------------------------------------

# loadStore: Load information about email store
def loadStore():
   #---------------------------------------------------------------------------
   # Func: Loads information about emails in the current directory into a
   #       hash table to help speed up other functions.
   # Args: None
   # Vars: curDir       = Current working directory of server.
   #       loacalStore  = Global hash table to hold all email data.
   #       count        = The number of emails in the email store.
   #       size         = The total size of all emails in bytes.
   #       path         = The combined path of the file and the cwd.
   #       file         = The name of the file in the store.
   #       i            = Email Number
   # Retn: None
   #---------------------------------------------------------------------------
   
   curDir = os.getcwd()     # Current working directory
   localStore['size'] = 0   # Reinitialize size of store to 0
   localStore['count'] = 0  # Reinitialize count of store to 0
   i = 1                    # Reinitialize email number to 1

   for file in os.listdir(curDir):                     # Loop through cwd
      path = os.path.join(curDir, file)                # Save the entire path
      if os.path.isfile(path) and ".eml" in file:      # If it is an email file
         localStore[file] = (i, os.path.getsize(path)) # Save new files & sizes
         localStore['count'] += 1                      # Increase count by 1
         localStore['size'] += os.path.getsize(path)   # Increase size
         i += 1                                        # Increase email number
   
   #---------------------------------------------------------------------------

# getEmail: Find email in store from email number
def getEmail(emailNum):
   #---------------------------------------------------------------------------
   # Func: Finds an email in the email store
   # Args: emailNum = Number of email in store.
   # Retn: file     = The name of the file in the store.
   #---------------------------------------------------------------------------

   for file in localStore.keys():                  # Loop through emails
      if not(file in 'count' or file in 'size'):   # Filter count & size
         if localStore[file][0] == emailNum:       # Find requested email
            return file                            # Return the email

   #---------------------------------------------------------------------------

# printEmail: Prints email to console after connection with server
def printEmail(emailNum):
   #---------------------------------------------------------------------------
   # Func: Finds a selected email via its number and returns it in the form:
   #           "--- all message headers and message ---\n"
   # Args: emailNum = The number of the email to be returned to the client.
   # Vars: file     = Name of email to be returned.
   # Retn: message  = Message, headers, and email to be printed to the console.
   #---------------------------------------------------------------------------

   file = getEmail(emailNum)                    # Find Email
   with open(file, 'r') as email:               # Open the email
      message = email.read()                    # Add email to message
   email.close()                                # Close the email
   return message                               # Return the message

   #---------------------------------------------------------------------------

def connectToServer(args):
   #---------------------------------------------------------------------------
   # Func: Establishes TCP connection to POP3 Server.
   # Args: args         = Tuple of (serverName, serverPort)
   #       serverName   = Name of server (default = localhost)
   #       serverPort   = Listening port of server (default = 25783)
   # Vars: command      = Command to be sent to server upon accepted conn.
   #       response     = Reply from server in response to command.
   # Retn: clientSocket = Socket for TCP connection.
   #---------------------------------------------------------------------------

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

   # Load and print welcome response
   response = clientSocket.recv(2048)
   print(response.decode())

   return clientSocket

   #---------------------------------------------------------------------------
#------------------------------------------------------------------------------

### Start Main Process ### ----------------------------------------------------

clientSocket = connectToServer(sys.argv) # Socket for TCP connection to server
connected = True                         # Bool for connected status w/ server
running = True                           # Bool for client status
loadStore()                              # Load local email store

# Start and maintain conversation until 'QUIT' command
while running:
   while connected:
      command = input()                                  # Ask for next command
      clientSocket.send(command.encode())                # Send command
      response = clientSocket.recv(2048)                 # Open response
      email = response.decode().split('FILENAME:')       # Split on 'FILENAME:' 
      content = email[0]                                 # Get content 4 client
      print(content)                                     # Print client content

      if 'RETR' in command.upper():                      # If RETR command
         fileNum = command.split(' ')[1]                 # Capture the file #
         fileName = email[1]                             # Get fileName
         localStore[fileName] = fileNum                  # Save to localStore
         file = open(fileName, 'w')                      # Open file to write
         content = content.split('octets\n')             # Remove server info
         file.writelines(content[1][:len(content[1])-1]) # Write email to file
         file.close()                                    # Close the file

      if command.upper() == 'QUIT':                      # If QUIT command
         connected = False                               # Break conn. loop

   clientSocket.close()                                  # Close socket

   while not connected:                                  # When not conn.
      command = input()
      if 'PRINT' in command.upper():                     # If PRINT command
         loadStore()                                     # Load email store
         args = command.split(' ')                       # Get args
         if len(args) > 1:
            print(printEmail(int(args[1])))              # Print email
         else:                                           
            print("Incorrect command: PRINT <messageNumber>")
      elif 'CONN' in command.upper():                    # If CONN command
         args = command.split(' ')                       # Get args
         clientSocket = connectToServer(args)            # Connect to server
         connected = True                                # Update status
      elif 'EXIT' in command.upper():                    # If EXIT command
         print("Have a nice day.")
         running = False                                 # End client session
         exit()
      elif 'HELP' in command.upper():                    # If HELP command
         print("PRINT <emailNumber> : Print email to console.")
         print("CONN <serverName> <serverPort> : Connect to server.")
         print("EXIT : Exit client.")                    # List commands
      else:                                              # Else catch
         print("Command " + command.upper() + " does not exist:")
         print("Type HELP for list of commands.")        # Commands that DNE

# Should never get here
print("Error: Client malfunction")
exit()
# ==========================================================================EOF