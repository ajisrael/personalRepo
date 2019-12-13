################################################################################
# Orig: 2019.12.11 - Alex Israels
# Revs: 2019.12.13 - Alex Israels
# Prog: controller.py
# Func: 
# Meth:
# Args:
# Vars: adjMatrix = Adjacency matrix packet sent to routing program.
#       flowTable = Flow table returned from routing program to send to switch.
# ==============================================================================
# Example Network:
# ------------------------------------------------------------------------------
# Host 1                                                               Host 2
#     \                                                                  /
#      \ 1 Switch A 3            2 Switch B  1             1 Switch C 3 /
#       ==============----------===============-----------==============
#      / 2                            | 3                             2 \
#     /                               |                                  \
# Host 4                            Host 8                             Host 16
# ------------------------------------------------------------------------------
# Example Forwarding Table:
# Name:             | Vertex #:            | Address:
# Host 1            | 0                    | 10.0.0.1
# Host 2            | 1                    | 10.0.0.2
# Host 4            | 2                    | 10.0.0.3
# Host 8            | 3                    | 10.0.0.4
# Host 16           | 4                    | 10.0.0.5
# Switch A          | 5                    | 10.0.0.6
# Switch B          | 6                    | 10.0.0.7
# Switch C          | 7                    | 10.0.0.8
# ------------------------------------------------------------------------------
# Example Adjacency Matrix Packet:
# 6, 8
# 0 = 10.0.0.1
# 1 = 10.0.0.2
# 2 = 10.0.0.3
# 3 = 10.0.0.4
# 4 = 10.0.0.5
# 5 = 10.0.0.6
# 6 = 10.0.0.7
# 7 = 10.0.0.8
# 
# 0, 0, 0, 0, 0, 1, 0, 0
# 0, 0, 0, 0, 0, 0, 0, 1
# 0, 0, 0, 0, 0, 1, 0, 0
# 0, 0, 0, 0, 0, 0, 1, 0
# 0, 0, 0, 0, 0, 0, 0, 1
# 1, 0, 2, 0, 0, 0, 3, 0
# 0, 0, 0, 3, 0, 2, 0, 1
# 0, 3, 0, 0, 2, 0, 1, 0
# ------------------------------------------------------------------------------
# Example Flow Table:
# 10.0.0.1, 2
# 10.0.0.2, 1
# 10.0.0.3, 2
# 10.0.0.4, 3
# 10.0.0.5, 1
# 10.0.0.6, 2
# 10.0.0.8, 1
# ==============================================================================
################################################################################

### Import Required Libraries ### ----------------------------------------------

from socket import *        # Socket lib
import sys                  # Argument lib
import os                   # File lib

# ------------------------------------------------------------------------------

### Define Global Variables ### ------------------------------------------------

# Adjacency matrix to send to rounting program
flowTable = "" # Flow table returned from router to send to switch program

# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Func: Defines an adjMatrix object to easily assign values from an Adjacency 
#       Matrix Packet. Additionally defines functions for manipulating the 
#       adjacency matrix itself. Specifically adding and removing elements.
# Meth: Initializes the adjMatrix from a packet by spliting the lines of the 
#       packet and assinging the correct values to variables tied to the object.
# ------------------------------------------------------------------------------
class AdjMatrix:
    def __init__(self, packet):
        self.packet = packet
        # Separate lines in Adjacency Matrix Packet:
        lines = packet.split('\n')

        # Separate first line to get source vertex and # of verticies
        line1 = lines[0].split(',')
        lines.remove(lines[0])
        self.sourceVertex = int(line1[0].strip())
        self.numVertices = int(line1[1].strip())

        # Loop through next <numVertices> # of lines to build ipAddrs
        self.ipAddrs = []
        for i in range(self.numVertices):
            self.ipAddrs.append((lines[0].split('='))[1].strip())
            lines.remove(lines[0])

        # Remove empty line
        lines.remove(lines[0])
        
        self.matrix = [[0 for col in range(self.numVertices)] for row in range(self.numVertices)]
        # Loop through remaining lines
        for i in range(len(lines)-1):
            row = (lines[i].split(','))
            for j in range(len(row)):
                self.matrix[i][j] = int(row[j].strip())

def initMatrix():
    # --------------------------------------------------------------------------
    # Name: initMatrix
    # Func: Initializes the adjacency matrix on startup.
    # Meth: Reads a file in the current directory containing the default
    #       adjacency matrix.
    # Vars: curDir   = Current working directory.
    #       fileName = Name of default matrix file.
    #       path     = Full path to default matrix file.
    # ==========================================================================
    # Default matrix on boot: 
    # 8, 9
    # 0 = 10.1.0.1
    # 1 = 10.1.0.2
    # 2 = 10.2.0.3
    # 3 = 10.2.0.4
    # 4 = 10.3.0.5
    # 5 = 10.3.0.6
    # 6 = 10.0.0.1
    # 7 = 10.0.0.2
    # 8 = 10.0.0.3
    # 
    # 0, 0, 0, 0, 0, 0, 1, 0, 0
    # 0, 0, 0, 0, 0, 0, 1, 0, 0
    # 0, 0, 0, 0, 0, 0, 0, 1, 0
    # 0, 0, 0, 0, 0, 0, 0, 1, 0
    # 0, 0, 0, 0, 0, 0, 0, 0, 1
    # 0, 0, 0, 0, 0, 0, 0, 0, 1
    # 2, 4, 0, 0, 0, 0, 0, 3, 1
    # 0, 0, 3, 4, 0, 0, 2, 0, 1
    # 0, 0, 0, 0, 2, 1, 3, 4, 0
    # ==========================================================================
    # --------------------------------------------------------------------------
    curDir   = os.getcwd()                    # Current working directory
    fileName = "adjMatrix.txt"                # Name of default matrix file
    path     = os.path.join(curDir, fileName) # Full path to file
    if os.path.isfile(path):
        with open(fileName, 'r') as fileMatrix:
            matrix = AdjMatrix(str(fileMatrix.read()))
            if debug:
                print("Packet :" + str(matrix.packet))
                print("SourceV:" + str(matrix.sourceVertex))
                print("NumVert:" + str(matrix.numVertices))
                print("IP Addr:" + str(matrix.ipAddrs))
                print("Matrix :" + str(matrix.matrix))
    else:
        erMsg = "adjMatrix: No such file."
        print(erMsg)
        matrix = erMsg

    return matrix
    # --------------------------------------------------------------------------

def connectRouter(matrix):
    #---------------------------------------------------------------------------
    # Func: Establishes TCP connection to router program
    # Vars: serverName   = Name of server (default = localhost)
    #       serverPort   = Listening port of server (default = 25784)
    # Retn: clientSocket = Socket for TCP connection.
    #---------------------------------------------------------------------------
    
    serverName = 'localhost'
    serverPort = 25784

    # Open Socket
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    print("Connected to " + serverName + ":" + str(serverPort))

    # Send request for startup flow table (ADD 0)
    command = matrix.packet
    clientSocket.send(command.encode())

    # Load and print welcome response
    response = clientSocket.recv(2048)
    print(response.decode())

    return clientSocket

    #---------------------------------------------------------------------------

# Initialize adjacency matrix:
global debug 
debug = True
matrix = initMatrix()

# Connect to router
routerSocket = connectRouter(matrix)

# Periodically open TCP connection to routing program and send adjacency matrix:


# Listen for incoming flow table:

# Open TCP connection to switch program:

# Send flowtable to swtich program:

# Listen for update events from switch program (port online/offline):
# ------------------------------------------------------------------------------
# Update Packet:
# VertexID, ADD/DELETE, PortNum, IPAdrs
# ------------------------------------------------------------------------------
# Switch with ID# VertexID is reporting that port# PortNum was ADDed or DELETEd
# ------------------------------------------------------------------------------