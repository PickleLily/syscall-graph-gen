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
            return

        #TODO --> how to implement the currentfd?

    def 

    def addNode(self, node):
        if self.findNode(node):
            print("Node already exists")
        else:
            self.nodes[node] = node
            print(f"Node {node} added")

    def findNode(self, fd:int, args:str, PID:int):
        return any(node.fd == fd and node.args == args and node.nodePID == PID for node in self.nodes)
    
    def addEdgeToSubgraph(self, edge):
        self.edges.add(edge)
        pass

    def updateEdge():
        return 0
    
    def updateNode():
        return 0 
    

    def __repr__(self):
        return f"Subgraph({self.graphNum})"
        
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

    def addNode(self, fd:int, args:str, PID:str, isProcess:bool, shape:str):
        print()
        '''
        Check to make sure node doesn't exist (findNode)
        if the Node doesn't exist, add node
        '''
        print(f"There are now {self.subgraph.node_count} nodes in graph {currentGraph}")
        
    def findNode(self, fd:int, args:str, PID:str, isProcess:bool, shape:str):
        '''
        Go through list of nodes to identify any duplicates
        If duplicate is found return that node
        Otherwise return nothing
        '''
        
        print()
        
        


class Edge:
    def __init__(self, isFrom:int, isTo:int, syscall:str, isBidirectional:bool, edgeType:str):
        self.isFrom = isFrom # Do we want to swap this with the actual node structs or nah
        self.isTo = isTo
        self.syscall = syscall
        
        self.isBidirectional = isBidirectional
        self.edgeType = edgeType


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
        if edge.from_node == from_node and edge.to_node == to_node and edge.syscall == syscall:
            return
        
        if edge.from_node == to_node and edge.to_node == from_node && edge.syscall == syscall:
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
                    break;
        
def main():
    '''
    OpenFile functionality
    
    '''
    return 0