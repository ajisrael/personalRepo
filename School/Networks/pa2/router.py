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
from dataStructs import *   # Data structure objects
import sys                  # Argument lib
import os                   # File lib
import pdb

# ------------------------------------------------------------------------------

global verbose
verbose = False
global debug
debug = False

# ------------------------------------------------------------------------------
# Func: Defines an adjMatrix object to easily assign values from an Adjacency 
#       Matrix Packet. Additionally defines functions for manipulating the 
#       adjacency matrix itself. Specifically adding and removing elements.
# Meth: Initializes the adjMatrix from a packet by spliting the lines of the 
#       packet and assinging the correct values to variables tied to the object.
# ------------------------------------------------------------------------------


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
        listening = False
        # Get the packet
        packet = connectionSocket.recv(2048).decode()
        print("Adjacency matrix recieved...")

        # Build the matrix object
        matrix = AdjMatrix(packet)
        if matrix.sourceVertex == -1:
            print("Exit signal found...")
            print("Terminating process...")
            connectionSocket.close()
            listening = False
            break

        # Build confirmed list
        destination = Vertex(matrix.sourceVertex, matrix.sourceVertex, 0, 0)
        confirmed = [destination]
        tentative = []
        curVert = matrix.sourceVertex
        costs = [0 for i in range(matrix.numVertices)]
        
        building = True
        while building:
            # Get the array of paths connected to current vertex
            paths = matrix.getRow(curVert)
            # Print out costs
            if verbose:
                for vertex in range(len(paths)):
                    print("VERBOSE: Cost to get to " + str(vertex) + " is " + str(costs[vertex]) + ".")
            # Loop through paths
            for vertex in range(len(paths)):
                # Reset confirmed and tentative checks
                conf = False
                tent = False
                # Assign cost
                curCost = costs[curVert] + 1
                # Check for valid path (port != 0)
                if paths[vertex] != 0:
                    if verbose:
                        print("VERBOSE: Path from " + str(vertex) + 
                        " to " + str(curVert) + " found.")
                    # Check if path is in confirmed list
                    for i in range(len(confirmed)):
                        if vertex == confirmed[i].id:
                            conf = True
                            if verbose:
                                print("VERBOSE: Path from " + str(vertex) + 
                                " to " + str(curVert) + " in confirmed list.")
                            break # Go to next vertex if in confirmed list
                    # If not in confirmed list
                    if conf == False:
                        # Check if path is in tentative list
                        for i in range(len(tentative)):
                            if vertex == tentative[i].id:
                                if verbose:
                                    print("VERBOSE: Path from " + str(vertex) + 
                                    " to " + str(curVert) + " in tentative list.")
                                tent = True
                                # Compare costs
                                if curCost < tentative[i].cost:
                                    if verbose:
                                        print("VERBOSE: Cost of vertex " + str(vertex) + "updated " + 
                                        "from " + str(tentative[i].cost) + " to " + str(curCost) +
                                        "through port " + str(paths[vertex]) + ".")
                                    tentative[i].cost = curCost
                                    tentative[i].nextHop = paths[vertex]
                                break # Go to next vertex if in tentative list
                    # When not in any list, add to tentative
                    if (conf == False) and (tent == False):
                        costs[vertex] = curCost
                        newVertex = Vertex(vertex, curVert, costs[vertex], paths[vertex])
                        tentative.append(newVertex)
                        if verbose:
                            print("VERBOSE: Vertex " + str(newVertex.id) + " added to tentative list:\n"
                            "VERBOSE: Cost = " + str(newVertex.cost) + ".\n"
                            "VERBOSE: NextHop = " + str(newVertex.nextHop) + ".")
            # Find next vertex to add to confirmed list and remove from tentative list
            minCost = sys.maxsize
            nextVert = -1
            minIndex = -1
            for i in range(len(tentative)):
                if tentative[i].cost < minCost:
                    minCost = tentative[i].cost
                    nextVert = tentative[i].id
                    minIndex = i
            curVert = nextVert
            confirmed.append(tentative[minIndex])
            if verbose:
                print("VERBOSE: Vertex " + str(tentative[minIndex].id) + " added to confirmed list.")
                print("VERBOSE: Vertex " + str(tentative[minIndex].id) + " removed from tentative list.")
            tentative.remove(tentative[minIndex])
            if verbose:
                print("VERBOSE: New current vertex: " + str(curVert) + ".")
            if len(tentative) == 0:
                building = False
            # if debug:    
            #     pdb.set_trace()
        if verbose:
            print("VERBOSE: FSA Complete")

# Generate flow table for packet switch:
# ------------------------------------------------------------------------------
# Flow Table Packet:
# 141.219.10.20, 4
# 10.0.1.19, 2
# . . .
# ------------------------------------------------------------------------------
# Simple file of addrs and egress port #'s
# ------------------------------------------------------------------------------
        table = {}
        for i in range(len(matrix.ipAddrs)):
            if i == matrix.sourceVertex:
                continue
            for j in range(len(confirmed)):
                # Find the vetex in the confirmed list
                if i == confirmed[j].id:
                    # Find the path to get to that vertex from the source vertex
                    dest = confirmed[j].dest
                    if dest != matrix.sourceVertex:
                        nextIndex = -1
                        while dest != matrix.sourceVertex:
                            for k in range(len(confirmed)):
                                if dest == confirmed[k].id:
                                    dest = confirmed[k].dest
                                    nextIndex = k
                                    break
                        table[matrix.ipAddrs[i]] = confirmed[nextIndex].nextHop
                    else:
                        table[matrix.ipAddrs[i]] = confirmed[j].nextHop
        flowTable = FlowTable(table)
        if verbose:
            print("VERBOSE: Flow Table ---------------------------------------START")
            print(flowTable.packet)
            print("VERBOSE: Flow Table -----------------------------------------END")
        if debug:    
            pdb.set_trace()
        # Send flow table to controller
        print("Sending flow table...")
        connectionSocket.send(flowTable.packet.encode())
        connected = False
        listening = True
    connectionSocket.close()
