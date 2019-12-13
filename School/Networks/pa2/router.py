################################################################################
# Orig: 2019.12.11 - Alex Israels
# Revs: 2019.12.13 - Alex Israels
# Prog: router.py
# Func: Implements a forward search algorithm
# Meth:
# Args:
# Vars:
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
# Forwarding Table:
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
# Adjacency Matrix Packet:
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
# Flow Table:
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

global debug
debug = True

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

# Open connection to controller:
serverPort = 25784
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)
print('Router ready to recieve...')
listening = True

# Listen on TCP port for adjacentcy matrix from the controller:
# ------------------------------------------------------------------------------
# Adjacentcy Matrix Packet:
# SourceVertexID, NumVertices
# 0 = ADDRESS
# 1 = ADDRESS
# ...
# NumVertices-1 = ADDRESS
# x, x, x...
# x, x, x...
# .
# .
# .
# x, x, x...
# ------------------------------------------------------------------------------
# Prop: - Lists IP Addrs of all devices
#       - # of rows and columns = NumVertices
#       - Routing algo starts at SourceVertexID
# Note: - Need to verify SourceVertexID is a valid node
# ------------------------------------------------------------------------------
while listening:
    connectionSocket, addr = serverSocket.accept()
    print('Connection data recieved from controller...')
    connected = True

    # Run forward search algorithm
    while connected:
        # Get the packet
        packet = connectionSocket.recv(2048).decode()

        # Build the matrix object
        matrix = AdjMatrix(packet)

        # Start confirmed list of Forward Search Algorithm (FSA) @ source vertex
        # Confirmed list = [path length, port]
        confirmed = [[sys.maxsize, 0] for col in range(matrix.numVertices)]
        confirmed[matrix.sourceVertex] = [0, 0]

        # Start working list of FSA
        working  = [[sys.maxsize, 0] for col in range(matrix.numVertices)]
        working[matrix.sourceVertex] = [0, 0]

        # Get ports for current vertex
        curPorts = matrix.matrix[matrix.sourceVertex]
        curVertex = matrix.sourceVertex

        # Loop through ports and update working list
        for vertex in range(matrix.numVertices):
            if curPorts[vertex] != 0:
                working[vertex] = [working[vertex][0]+1, curPorts[vertex]]

        # Find shortest path in working list and update confirmed list
        minPath = sys.maxsize
        newVertex = matrix.numVertices
        newPort = 0
        for vertex in range(matrix.numVertices):
            if vertex != curVertex:
                if working[vertex][0] < minPath and working[vertex][1] != 0:
                    minPath = working[vertex][0]
                    newVertex = vertex
                    newPort = working[vertex][1]
        
        if debug:
            print("Next Vertex: " + str(newVertex))
            print("Next Port:   " + str(newPort))
            print("Min Path:    " + str(minPath))
        confirmed[newVertex] = [minPath, newPort]
        curVertex = newVertex
        

        

# Generate flow table for packet switch:
# ------------------------------------------------------------------------------
# Flow Table Packet:
# 141.219.10.20, 4
# 10.0.1.19, 2
# . . .
# ------------------------------------------------------------------------------
# Simple file of addrs and egress port #'s
# ------------------------------------------------------------------------------

# Send flow table to controller
