# ==========================================================================BOF
# Orig: 2019.10.05 - Alex Israels
# Revs: 2019.10.07 - Alex Israels
# Func: A POP3 server communicating over TCP connection. It supports the
#       following commands:
#           - STAT: Number and total size of all messages
#           - LIST: Message#'s and size of messages
#           - RETR: Retrieve selected message
#           - DELE: Delete selected message
#           - TOP : Retrieve part of selected message
#           - QUIT: Ends session with client
#           - HELP: Provides a list of server commands to the client
# Args: serverPort       = Port to listen for connection from POP3 client.
# Vars: serverSocket     = Socket for listening for connection request.
#       connectionSocket = Socket for process if connection has been accepted.
#       helloMsg         = Welcome message to be sent to client. 
#       error            = Error message to be sent to client. 
#       listening        = Boolean for when server is listening for connection.
#       connected        = Boolean for when server is connected to client.
#       clientData       = Raw data from client.
#       clientArgs       = Formatted data from client (split on ' ').
# Retn: None
# -----------------------------------------------------------------------------

### Import Required Libraries ### ---------------------------------------------

from socket import * # Socket lib
import sys           # Argument lib
import os            # File lib

# -----------------------------------------------------------------------------

### Define Global Variables ### -----------------------------------------------

store = {         # Hashtable for email store data
   'size'  : 0 ,  # Size in bytes of all emails in store
   'count' : 0    # Total number of emails in store
}

remove = []       # array of emails to be deleted at end of session

# -----------------------------------------------------------------------------

### Define each command as a function ### -------------------------------------

# loadStore: Load informaiton about email store
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
   store['size'] = 0      # Reinitialize size of store to 0
   store['count'] = 0     # Reinitialize count of store to 0
   i = 1                  # Reinitialize email number to 1

   for file in os.listdir(curDir):                    # Loop through cwd
      path = os.path.join(curDir, file)               # Save the entire path
      if os.path.isfile(path) and ".eml" in file:     # If it is an email file
         if not(file in remove):                      # & isn't in remove list
            store[file] = (i, os.path.getsize(path))  # Save new files & sizes
            store['count'] += 1                       # Increase count by 1
            store['size'] += os.path.getsize(path)    # Increase size
            i += 1                                    # Increase email number
   
   #---------------------------------------------------------------------------

# getEmail: Find email in store from email number
def getEmail(emailNum):
   #---------------------------------------------------------------------------
   # Func: Finds an email in the email store
   # Args: emailNum = Number of email in store.
   # Retn: file     = The name of the file in the store.
   #---------------------------------------------------------------------------
   
   for file in store.keys():                       # Loop through emails
      if not(file in 'count' or file in 'size'):   # Filter count & size
         if store[file][0] == emailNum:            # Find requested email
            return file                            # Return the email

   #---------------------------------------------------------------------------

# STAT: Number and total size of all messages
def STAT():
   #---------------------------------------------------------------------------
   # Func: Gets the total number and size in bytes of all emails 
   #       and returns that data in the form:
   #           "+OK <num> <size>"
   # Args: None
   # Vars: store   = Global hash table holding all email data.
   # Retn: message = Message to be returned to client.
   #---------------------------------------------------------------------------

   message = '+OK '+str(store['count'])+' '+str(store['size']) # Client message
   return message                                              # Return message

   #---------------------------------------------------------------------------

# LIST: Lists email numbers and their sizes
def LIST():
   #---------------------------------------------------------------------------
   # Func: Lists the message number and size in bytes of all emails 
   #       individually and returns that data in the form:
   #           "+OK Mailbox scan listing follows\n"
   #           "1 <sizeof 1>\n"
   #           "2 <sizeof 2>\n"
   #           "...\n"
   #           "n <sizeof n>\n"
   #           "."
   # Args: None
   # Vars: store   = Global hash table holding all email data.
   #       file    = Name of email to be listed
   # Retn: message = Message to be returned to client.
   #---------------------------------------------------------------------------

   message = '+OK Mailbox scan listing follows\n'  # Start of listing to client

   for file in store.keys():                       # Loop through emails
      if not(file in 'count' or file in 'size'):   # Filter count and size vars
         message += str(store[file][0]) + ' '      # Add email# to message
         message += str(store[file][1]) + '\n'     # Add email size to message
   message += '.'                                  # Peroid noting end of list
   
   return message                                  # return message

   #---------------------------------------------------------------------------

# RETR: Retrieve selected email
def RETR(emailNum):
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
   
   message = '+OK '                             # Start message

   file = getEmail(emailNum)                    # Find Email
   message += str(store[file][1]) + ' octets\n' # Add size to message
   with open(file, 'r') as email:               # Open the email
      message += email.read()                   # Add email to message
   email.close()                                # Close the email
   message += '.'                               # Peroid marking end
   message += 'FILENAME:' + file                # Save file name at end of msg

   return message                               # Return the message

   #---------------------------------------------------------------------------

# DELE: Delete selected email
def DELE(emailNum):
   #---------------------------------------------------------------------------
   # Func: Deletes a selected email via its number and returns a message in the 
   #       form:
   #           "+OK Message deleted"
   # Args: emailNum = The number of the email to be deleted from the store.
   # Vars: file     = Name of email to be deleted
   #       store    = Global hash table holding all email data.
   # Retn: message  = Message to be returned to client.
   #---------------------------------------------------------------------------
   
   message = '+OK Message deleted'           # Start message
   file = getEmail(emailNum)                 # Find email
   
   if file != None:                          # If email exists
      remove.append(file)                    # Add email to remove list
      store['count'] -= 1                    # Decrease store count
      store['size'] -= store[file][1]        # Decrease store size
      del store[file]                        # Delete the email from store
   else:                                     # Else
      message = "-ERR Message doesn't exist" # Notify client email DNE
   loadStore()                               # Reload the store
   
   return message                            # Return the message

   #---------------------------------------------------------------------------

# TOP : Retrieve part of selected message
def TOP(emailNum, numOfLines):
   #---------------------------------------------------------------------------
   # Func: Retrieves a selected email via its number and returns all headers
   #       and n number of lines in the form:
   #           "+OK Top of message follows\n"
   #           "--- all message headers ---\n"
   #           "--- first <n> lines of body ---\n"
   #           "."
   # Args: emailNum   = The number of the email to be returned to client.
   #       numOfLines = The number of lines of the body to be returned.
   # Vars: file       = Email requested by client.
   #       i          = Index counter for body lines
   # Retn: message    = Message to be returned to client.
   #---------------------------------------------------------------------------
   
   message = '+OK Top of message follows\n' # Start Message
   i = 0                                    # Set body incrementer to 0
   file = getEmail(emailNum)                # Find email
   
   with open(file, 'r') as email:           # Open the email
      lines = email.read()                  # Load email into lines var
   email.close()                            # Close the email
   lines = lines.split('\n')                # Separate email into lines
   bodyIndex = lines.index('')              # Find index where body starts
   header = lines[0:bodyIndex]              # Load header lines into header var
   body = lines[bodyIndex:len(lines)]       # Load body lines into body var

   for line in header:                      # Loop through each line of header
      message += line + '\n'                # Add header line to message

   if numOfLines > 0:                       # If body lines requested
      for line in body:                     # Loop through each body line
         if i <= numOfLines:                # Up to number of lines requested
            message += line + '\n'          # Add body line to message
            i += 1                          # Increment body line counter

   message += '.'                           # Period marking end
   
   return message                           # Return message

   #---------------------------------------------------------------------------

# RSET: Reset the session 
def RSET():
   #---------------------------------------------------------------------------
   # Func: Resets the session and repsonds to the client in the following form:
   #           "+OK Reset state\n"
   # Args: None
   # Vars: remove  = List of emails to be deleted at end of session.
   # Retn: message = Message to be returned to client.
   #---------------------------------------------------------------------------
   
   message = '+OK Reset state' # Create message
   del remove[:]               # Clear list of emails to be removed 
   loadStore()                 # Reload the email store

   return message              # Return message 

   #---------------------------------------------------------------------------

# QUIT: Ends session with client
def QUIT():
   #---------------------------------------------------------------------------
   # Func: Ends the session and repsonds to the client in the following form:
   #           "+OK Goodbye\n"
   # Args: None
   # Vars: remove  = List of emails to be deleted at end of session.
   # Retn: message = Goodbye phrase to client.
   #---------------------------------------------------------------------------

   message = "+OK Goodbyes are not forever, are not the end;\n"  # Make message
   message +="it simply means I'll miss you until we meet again."

   for file in remove:  # Delete files
      os.remove(file)

   return message       # Return message

   #---------------------------------------------------------------------------

# HELP: Lists server commands
def HELP():
   #---------------------------------------------------------------------------
   # Func: Lists all server commands in the following format:
   #           "<Command> : <Explaination"
   # Args: None
   # Vars: None
   # Retn: message = Message sent to client.
   #---------------------------------------------------------------------------

   message =  "STAT : Prints the status of the email store.\n"
   message += "LIST : Prints a list of all emails.\n"
   message += "DELE <email#> : Deletes an email from the store.\n"
   message += "RETR <email#> : Returns and saves an email.\n"
   message += "TOP  <email#> <#OfLines> : Prints email headers and "
   message += "#OfLines of email body.\n"
   message += "RSET : Resets POP3 session.\n"
   message += "QUIT : Quits POP3 session."
   return message

   #---------------------------------------------------------------------------
#------------------------------------------------------------------------------

### Start Main Process ###-----------------------------------------------------
# Load the server port (default = 25783)
if len(sys.argv) > 1:
   serverPort = int(sys.argv[1])
else:
   serverPort = 25783

# Make sure it is not a well known port
if serverPort <= 1024:
   print('Do not mess with "well known ports" please.')
   exit()

# Open socket
print('Server Port: ', serverPort)
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)
print('Server ready to recieve...')

# Start listening
listening = True
while listening:
   connectionSocket, addr = serverSocket.accept() # Accept incomming connection
   print('Connection data recieved...')
   connected = True
   loadStore()                                    # Load the email store
   while connected:                               # Start the conversation
      # Read in commands
      clientData = connectionSocket.recv(2048).decode().upper()
      clientArgs = clientData.split(' ')
      print('Command ' + clientArgs[0] + ' recieved...')

      # Map to correct command
      try:
         if 'STAT' in clientArgs[0]:
            connectionSocket.send(STAT().encode())
         elif 'LIST' in clientArgs[0]:
            connectionSocket.send(LIST().encode())
         elif 'RETR' in clientArgs[0]:
            connectionSocket.send(RETR(int(clientArgs[1])).encode())
         elif 'DELE' in clientArgs[0]:
            connectionSocket.send(DELE(int(clientArgs[1])).encode())
         elif 'TOP'  in clientArgs[0]:
            connectionSocket.send(TOP(int(clientArgs[1]),int(clientArgs[2])).encode())
         elif 'QUIT' in clientArgs[0]:
            connectionSocket.send(QUIT().encode())
            connected = False
         elif 'RSET' in clientArgs[0]:
            connectionSocket.send(RSET().encode())
         elif 'HELP' in clientArgs[0]:
            connectionSocket.send(HELP().encode())
         elif 'HI'   in clientArgs[0]:
            helloMsg = '+OK Mailbox open, ' + str(store['count']) + ' messages'
            connectionSocket.send(helloMsg.encode())
         else:
            error =  'Command '+ clientArgs[0] +' does not exist.\n'
            error += 'Type "Help" for a list of commands.'
            connectionSocket.send(error.encode())
      
      # Capture errors and notify client
      except:
         error = 'Something went wrong. \n' + str(sys.exc_info()[0])
         connectionSocket.send(error.encode())
      
      # Note status of server
      print('Return message sent...')
   
   # When connection is closed ...
   print('Connection closed...')    # Stop conversation after 'QUIT'
   connectionSocket.close()         # Close connection socket
   listening = True                 # Continue listening for new connections

# Should never get here
print('Error: Server not able to recieve ...')
serverSocket.close()
exit()
# ==========================================================================EOF