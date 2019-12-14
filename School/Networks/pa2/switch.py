################################################################################
# Orig: 2019.12.11 - Alex Israels
# Revs: 2019.12.13 - Alex Israels
# Prog: switch.py
# Func: 
# Meth: 
# Asum: Started after router and controller programs have been started.
# Reqs: FORWARD: (F)
#           - Should be completed with a properly formatted IP address. The 
#             program should indicate the port to forward the datagram out.
#       ADD:     (A)
#           - Should be completed with a port number and the IP address of the
#             connected device.
#           - The switch will then send an update packet to the controller with
#             this new information.
#           - It should print out a confirmation that the new flow table has
#             been received.
#       DELETE:  (D)
#           - Works the same as ADD except that it removes a port from the
#             switch. No IP address is needed here.
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

### Import Required Libraries ### ---------------------------------------------

from socket import *        # Socket lib
from dataStructs import *   # Data structure objects
import sys                  # Argument lib
import os                   # File lib
import pdb

# -----------------------------------------------------------------------------

global verbose
verbose = False

def buildTable(packet):
    lines = packet.split('\n')
    table = {}
    for line in lines:
        if line != '':
            args = line.split(', ')
            table[args[0].strip()] = int(args[1].strip())
    
    return FlowTable(table)

def getFlowTable(vert, comm, port, IP):
    #---------------------------------------------------------------------------
    # Func: Establishes TCP connection to controller program
    # Vars: serverName   = Name of server (default = localhost)
    #       serverPort   = Listening port of server (default = 25783)
    # Retn: clientSocket = Socket for TCP connection.
    #---------------------------------------------------------------------------
    
    serverName = 'localhost'
    serverPort = 25783

    # Open Socket
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    print("Sending update packet...")

    # Send request for startup flow table (ADD 0)
    command = vert + ', ' + comm + ', ' + str(port) + ', ' + IP
    clientSocket.send(command.encode())

    # Load and print welcome response
    response = clientSocket.recv(2048)
    print("Flow table recieved...")
    if verbose:
        print("VERBOSE: Flow Table ---------------------------------------START")
        print(response.decode())
        print("VERBOSE: Flow Table -----------------------------------------END")

    clientSocket.close()
    return buildTable(response.decode())

    #---------------------------------------------------------------------------

# Prompt user for first starting vertex.
vertex = input("Enter Source Vertex ID: ")
# Establish TCP connection to controller program
flowTable = getFlowTable(vertex, 'ADD', 0, '0.0.0.0')
# Prompt user for input: FORWARD, ADD, or DELETE a port
running = True

while running:
    command = input()
    args = command.split(' ')
    if args[0].upper() == 'FORWARD' or args[0].upper() == 'F':
        found = False
        for ip in flowTable.table.keys():
            if ip == args[1].strip():
                found = True
                print("Forwarding packet out port " + str(flowTable.table[ip]) + ".")
                break
        if found == False:
            print("No rule to match for packet.")
    elif args[0].upper() == 'ADD' or args[0].upper() == 'A':
        flowTable = getFlowTable(args[1], 'ADD', args[2], args[3])
    elif args[0].upper() == 'DELETE' or args[0].upper() == 'D':
        flowTable = getFlowTable(args[1], 'DELETE', args[2], '0.0.0.0')
    elif args[0].upper() == 'EXIT':
        flowTable = getFlowTable('-1', 'EXIT', '-1', '0.0.0.0')
        running = False
    else:
        print("Command " + command + " does not exist.")
        print("Try one of the following commands:")
        print("F <IPAddress>")
        print("A <sourceVertex> <Port#> <IPAddress>")
        print("D <sourceVertex> <Port#>")
