import datetime as dt

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

# ------------------------------------------------------------------------------------------------------------------------------
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
            nodeID = self.node_count
            self.node_count +- 1

             

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
    
# ------------------------------------------------------------------------------------------------------------------------------
class Node:
    def __init__(self, nodeID:int, fd:int, nodePID:int, args:str, isProcess:bool, shape:str, graph:Subgraph):
        # self.nodeID = nodeID
        self.fd = fd
        self.nodePID = nodePID
        self.args = args
        
        self.isProcess = isProcess
        self.shape = shape
        # self.graph = graph


    def __repr__(self):
        return f"Node({self.fd, self.nodePID, self.args, self.isProcess, self.shape})"
    
    def __eq__(self, comparison):
        return (
            self.fd == comparison.fd and
            self.nodePID == comparison.nodePID and 
            self.args == comparison.args
        )

    def __hash__(self):
        return hash((self.fd, self.nodePID, self.args))

    def getData(self):
        # Method to return the extra data if needed
        return {
            "isProcess": self.isProcess,
            "shape": self.shape
    }
        
# ------------------------------------------------------------------------------------------------------------------------------ 
class Edge:
    def __init__(self, isFrom:Node, isTo:Node, syscall:str, isBidirectional:bool, edgeType:str):
        self.isFrom = isFrom # Do we want to swap this with the actual node structs or nah
        self.isTo = isTo
        self.syscall = syscall
        
        self.isBidirectional = isBidirectional
        self.edgeType = edgeType

    def __repr__(self):
        return f"Node({self.isFrom, self.isTo, self.syscall, self.isBidirectional, self.edgeType})"
        
    def __eq__(self, comparison):
        return (
            self.isFrom == comparison.isFrom and
            self.isTo == comparison.isTo and 
            self.syscall == comparison.syscall
        )
    
    def __hash__(self):
        return hash((self.isFrom, self.isTo, self.syscall))
    
    def getData(self):
        # Method to return the extra data if needed
        return {
            "isBidirectional": self.isBidirectional,
            "edgeType": self.edgeType
    }

 # ------------------------------------------------------------------------------------------------------------------------------

class Parser:
    def parseLine(line: str, fd: int, syscall: str, args: str, ret: str, pid: int):
        return

    def parseSyscall(syscall: str, ret: str, args:str, fd:int):
        return

    def parseArgs(args: str, output: str):
        return

# ------------------------------------------------------------------------------------------------------------------------------
# other functions:
def handleConnectionSystemCall():
    return

def printSubgraphMetadata():
    return

def createDOT(setting: str):
    # get current timestamp
    time = dt.now()

    if setting == "individual":
        # make subdir
        # char makeCommand[256];
        makeCommand = f'mkdir "./Dot_Files/Timestamp_{time}"'

        # sprintf(makeCommand, "mkdir \".\\Dot_Files\\Timestamp_%s\"", timeBuffer);
        # if (system(makeCommand) == -1){ // Try the command
        #     perror("Could not make subdirectory for graphs");
        # }
        # open Dot file
        path = f"./Dot_Files/Timestamp_{time}.dot"
        print(f"Created graph {path}")

        with open(path, "w") as dot_file:
            dot_file.write("digraph nginx_syscalls {") #header
            dot_file.write("rankdir=LR;\n")

        # add all nodes
        # add all edges
        # add error state (i.e. no close)
            if (graphs[i].isValid == 1) {
                fprintf(dot_file, "  -1 [label=\"Graph Did Not Receive 'Close' Syscall\", shape=box, penwidth=4, color=red, pos=\"5,5!\"];\n");
            }
        # close
            dot_file.write("}\n")#close subgraph
            dot_file.write("}")# Close 

    elif setting == "together":
        # open Dot file
        path = f"./Dot_Files/Timestamp_{time}.dot"
        print(f"Created graph {path}")
        with open(path, "w") as dot_file:
            dot_file.write("digraph nginx_syscalls {") #header

            # go through list of subgraphs:
            for i, subgraph in enumerate(graphs):
                dot_file.write("subgraph cluster_{i} {")
                #add all nodes
                for j, node in enumerate(subgraph.nodes):
                    dot_file.write(f"  {j}{i} [label=\"{node.args}\" style={node.shape}]")
                #add all edges
                for j, edge in enumerate(subgraph.edges):
                    dot_file.write(f"  {edge.isFrom}{i}->{edge.isTo}{i} [label=\"{edge.syscall}\" style={edge.type}]")



            dot_file.write("}\n")#close subgraph
            dot_file.write("}")# Close  
    else:
        createDOT("individual")

    print(f"printed graphs(s): {time}")


def getNodeFD():
    return

def getProcessNode():
    return

def getSubgraphFD():
    return

def moveFDToArgs():
    return

def main():
    # OpenFile functionality
    input_log = open("./Falco Trace Files/TestEvents.txt", "r")

    # go through file line by line
    i = 0
    for line in input_log:
        i = i+1

    # Include additional debugging information if desired
    if (DEBUG_LEVEL == 1) {
        printSubgraphMetadata();
    }
    createDOT()
    return i