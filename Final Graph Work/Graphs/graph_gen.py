# import ... as ...

# Global variables
MAX_SUBNODES = 100
MAX_SUBEDGES = 1000
MAX_SUBGRAPHS = 100
DOT_TYPE =  "individual"
DEBUG_LEVEL = 0

# graph isValid types
OPEN = 0 
CLOSED = -1
INVALID = 1

# Global graph Reference
graphs = [MAX_SUBGRAPHS]
totalGraphs = -1
currentGraph = -1
------------------------------------------------------------------------------------------------------------------------------
class Node:
    def __init__(self, nodeID:int, fd:int, nodePID:int, args:str, isProcess:bool, shape:str, subgraph:Subgraph):
        self.nodeID = nodeID
        self.fd = fd
        self.args = args
        
        self.isProcess = isProcess
        self.shape = shape
        self.nodePID = nodePID
        self.subgraph = subgraph


    def __repr__(self):
        return f"Node({self.fd, self.nodePID, self.args})"
    
    def __eq__(self, comparison):
        return (
            self.fd == comparison.fd and
            self.nodePID == comparison.nodePID and 
            self.args == comparison.args
        )

    def __hash__(self):
        return hash((self.fd, self.nodePID, self.args))
        
# ------------------------------------------------------------------------------------------------------------------------------ 
class Edge:
    def __init__(self, isFrom:int, isTo:int, syscall:str, isBidirectional:bool, edgeType:str):
        self.isFrom = isFrom # Do we want to swap this with the actual node structs or nah
        self.isTo = isTo
        self.syscall = syscall
        
        self.isBidirectional = isBidirectional
        self.edgeType = edgeType

    def __repr__(self):
        return f"Node({self.isFrom, self.isTo, self.syscall})"
        
    def __eq__(self, comparison):
        return (
            self.isFrom == comparison.isFrom and
            self.isTo == comparison.isTo and 
            self.syscall == comparison.syscall
        )
    
    def __hash__(self):
        return hash((self.isFrom, self.isTO, self.syscall))

------------------------------------------------------------------------------------------------------------------------------
class Subgraph:

    def __init__(self, fd:int, pid:int, remoteVal = None, localVal = None): #Make lsit of potential fd
        totalGraphs = totalGraphs + 1
        currentGraph = totalGraphs
        graphs[currentGraph] = self
        
        self.fd = fd        
        self.masterPID = pid
        self.masterRemote = pid

        self.nodes = {} #NodeID -> node
        self.edges = {} #from->to & call -> edge
    
        self.isValid = OPEN
        self.graphNum = totalGraphs
        self.node_count = 0
        self.edge_count = 0

        # create the initial graph
        if(remoteVal != None and localVal != None):
            remote = Node(remoteVal, fd, "diamond", self.node_count, pid, False, self)
            local = Node(localVal, fd, "diamond", self.node_count, pid, False, self)
            network_edge = Edge(local.nodeID, remote.nodeID, "accept4", "solid", self)
            pid = Node(pid, fd, "rectangle", self.node_count, pid, True, self)
            pid_edge = Edge(pid.Node)
            
        return self

        #TODO --> how to implement the currentfd?

    def addEdge(from_node: int, to_node: int, syscall: str):
        graph = graphs[currentGraph]
        
        # Check max edges -> Default termination end case
        if(graph.edge_count >= MAX_SUBEDGES):
            print("Maximum number of edges exceeded")
            exit(1)
            
        # Check to see if both directions are the same. If so, default to the one above the PID
        if (from_node == to_node):
            from_node = from_node - 1; # TODO Default this to 1??

        # Check if edge already exists
        # TODO -- not skipping 0 & 1...
        for edge in graph.edges:
            # Duplicate edge found, do not add
            if edge.isFrom == from_node and edge.isTO == to_node and edge.syscall == syscall:
                return
            
            if edge.isFrom == to_node and edge.isTO == from_node and edge.syscall == syscall:
                # TODO -- is this true???
                edge.isBiderectional = True
                return
            
        # if edge DOESNT exist...create it!!!
        newEdge = Edge(to_node, from_node, syscall, "solid", graph)

        # if the edge is close, return fd to original PID FD, PID will always be node 3 (2)
        if syscall == "close":
            # if the current nodes pointing to the same FD as the accept
            if graph.nodes[to_node].fd == graph.nodes[0].fd and graph.nodes[0].nodePID == graph.masterPID:
                # invalidate the entire graph
                graph.isValid = INVALID
            else:
                #subprocess
                for i, descriptor in enumerate(graph.currentfd):
                    if graph.node[to_node].fd == descriptor:
                        # invalidate the current fd
                        graph.currentfd[i] = -1
                        break

    def addNode(self, fd:int, args:str, pid:int, isProcess:bool, shape:str):
        temp = self.findNode(fd, pid, args)
        if temp is not None:
            print("Node already exists with ID: {temp}")
            return temp
        else:
            
            self.nodes[node] = node
            print(f"Node {node} added")

    # Method to find a Node if it already exists in the subgraph
    # If nothing matches, returns None
    def findNode(self, fd:int, args:str, pid:int, default=None):
        key = (fd, pid, args)
        return self.nodes.get(key, default)
    
    def addEdgeToSubgraph(self, edge):
        self.edges.add(edge)
        pass

    def updateEdge():
        return 0
    
    def updateNode():
        return 0 
    

    def __repr__(self):
        return f"Subgraph({self.graphNum})"
    



class Parser:
    def parseLine(line: str, fd: int, syscall: str, args: str, ret: str, pid: int):
        return

    def parseSyscall(syscall: str, ret: str, args:str, fd:int):
        return

    def parseArgs(args: str, output: str):
        return


# other functions:
def handleConnectionSystemCall():
    return
def printSubgraphMetadata():
    return
def createDOT():
    return
def getNodeFD():
    return
def getProcessNode():
    return
def getSubgraphFD():
    return
def moveFDToArgs():
    return

def main():
    '''
    OpenFile functionality
    
    '''
    return 0