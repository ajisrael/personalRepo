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

### Start Main Process ###

# Load the server name and server port
# Default serverName = 'localhost'
# Default serverPort = 25783
if len(sys.argv) > 2:
   serverName = sys.argv[1]
   serverPort = int(sys.argv[2])
elif len(sys.argv) == 2:
   serverName = sys.argv[1]
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

# Start and maintain conversation until 'QUIT' command
while command.upper() != 'QUIT':
   command = input()                               # Ask for next command
   clientSocket.send(command.encode())             # Send command
   response = clientSocket.recv(2048)              # Open response
   email = response.decode().split('FILENAME:')    # Split on 'FILENAME:' 
   content = email[0]                              # Get content for client
   print(content)                                  # Print client content

   if 'RETR' in command.upper():                      # If RETR command
      fileName = email[1]                             # Get fileName
      file = open(fileName, 'w')                      # Open file to write
      content = content.split('octets\n')             # Remove server info
      file.writelines(content[1][:len(content[1])-1]) # Write email to file
      file.close()                                    # Close the file
clientSocket.close()