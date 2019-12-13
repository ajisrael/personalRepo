################################################################################
# Orig: 2019.12.12 - Alex Israels
# Revs: 2019.12.13 - Alex Israels
# Prog: adjMatrix.py
# Func: Defines an adjMatrix object to easily assign values from an Adjacency 
#       Matrix Packet. Additionally defines functions for manipulating the 
#       adjacency matrix itself. Specifically adding and removing elements.
# Meth: Initializes the adjMatrix from a packet by spliting the lines of the 
#       packet and assinging the correct values to variables tied to the object.
################################################################################
class AdjMatrix:
    def __init__(self, packet):
        self.packet = packet
        # Separate lines in Adjacency Matrix Packet:
        lines = packet.split('\n')

        # Separate first line to get source vertex and # of verticies
        line1 = lines[0].split(',')
        lines.remove(lines[0])
        self.sourceVertex = int(line1[0].strip())
        self.numVerticies = int(line1[1].strip())

        # Loop through next <numVerticies> # of lines to build ipAddrs
        self.ipAddrs = []
        for i in range(self.numVerticies):
            self.ipAddrs.append((lines[0].split('='))[1].strip())
            lines.remove(lines[0])

        # Remove empty line
        lines.remove(lines[0])
        
        self.matrix = [[0 for col in range(self.numVerticies)] for row in range(self.numVerticies)]
        # Loop through remaining lines
        for i in range(len(lines)-1):
            row = (lines[i].split(','))
            for j in range(len(row)):
                self.matrix[i][j] = int(row[j].strip())