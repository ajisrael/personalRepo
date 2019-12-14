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
        self.numVertices = int(line1[1].strip())

        # Loop through next <numVertices> # of lines to build ipAddrs
        self.ipAddrs = []
        for i in range(self.numVertices):
            self.ipAddrs.append((lines[0].split('='))[1].strip())
            lines.remove(lines[0])

        # Remove empty line
        if lines[0] == '':
            lines.remove(lines[0])
        
        self.matrix = [[0 for col in range(self.numVertices)] for row in range(self.numVertices)]
        # Loop through remaining lines
        for i in range(len(lines)-1):
            row = (lines[i].split(','))
            for j in range(len(row)):
                self.matrix[i][j] = int(row[j].strip())

    def getRow(self, vertexID):
        return self.matrix[vertexID]

    def updatePacket(self):
        self.packet = str(self.sourceVertex) + ', ' + str(self.numVertices) + '\n'
        for i in range(len(self.ipAddrs)):
            self.packet += str(i) + ' = ' + self.ipAddrs[i] + '\n'
        self.packet += '\n'
        for i in range(self.numVertices):
            row = self.getRow(i)
            for j in range(len(row)-1):
                self.packet += str(row[j]) + ', '
            self.packet += str(row[len(row)-1]) + '\n'


class Vertex:
    def __init__(self, id, dest, cost, nextHop):
        self.id = id
        self.dest = dest
        self.cost = cost
        self.nextHop = nextHop

class FlowTable:
    def __init__(self, table):
        self.table = table

        self.packet = ""
        for key in table.keys():
            self.packet += key + ', ' + str(table[key]) + '\n'