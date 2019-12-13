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
class adjMatrix:
    def __init__(self, packet):
        self.packet = packet
        # Separate lines in Adjacency Matrix Packet:
        lines = packet.splitLines()

        # Separate first line to get source vertex and # of verticies
        line1 = lines[0].split(',')
        lines.remove(lines[0])
        self.sourceVertex = int(line1[0].strip())
        self.numVerticies = int(line1[1].strip())

        # Loop through next <numVerticies> # of lines to build ipAddrs
        self.ipAddrs = []
        for i in range(self.numVerticies):
            self.ipAddrs.append((lines[i].split('=').strip())[1])

        # Remove lines with ipAddrs and empty line
        lines.remove(range(self.numVerticies + 1))
        
        # Loop through remaining lines
        for i in range(len(lines)):
            row = lines[i].split(',').strip()
            for j in range(len(row)):
                self.matrix[i][j].append(int(row[j]))