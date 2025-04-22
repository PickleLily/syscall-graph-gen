import re
import datetime as dt
from datetime import datetime
import subprocess
import os

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

# ------------------------------------------------------------------------------------------------------------------------------

class GraphManager:
    def __init__(self):
        self.graphList = {}         # key: (pid, networkTuple), value: Graph
        self.current_Graph = None   # Graph
        self.totalGraphs = 0

    def addSubgraph(self, fd:int, pid:int, networkTuple:str):
        isDuplicate = self.duplicateNetwork(networkTuple)
        if isDuplicate is not None:
            isDuplicate.isValid = 1
            isDuplicate.fdList = None
        newGraph = Subgraph(fd, pid, networkTuple)
        key = (self.totalGraphs, newGraph.masterPID, newGraph.originalTuple)
        self.graphList[key] = newGraph
        self.totalGraphs += 1 
        self.current_Graph = newGraph  
            # print(f"Made key {key}")

    # Return the graph object or lack thereof 
    def duplicateNetwork(self, networkTuple:str):
        return next((graph for graph in self.graphList.values() 
                    if graph.isValid == 0 and 
                    networkTuple == graph.originalTuple), None)
    
    # Return the graph object that we are "HOPEFULLY" looking at
    def swapSubgraph(self, pid:int, fd:int):
        self.current_Graph = next((graph for graph in self.graphList.values() if graph.isValid == 0 and fd in graph.fdList), self.current_Graph)

globalGraphManager = GraphManager()   

# ------------------------------------------------------------------------------------------------------------------------------

class Subgraph:

    def __init__(self, fd:int, masterPid:int, originalTuple:str):
        self.fdList = set()                         # Initialize a set of valid fds
        self.fdList.add(fd)                         # Add the current fd to that set
        self.masterPID = masterPid                  # initialize the masterPID (PID or original connection) through input
        self.originalTuple = originalTuple          # Initialize the original connection tuple of the graph
        self.isValid = OPEN                         # Initialize the validity of a graph to OPEN automatically
        self.nodes = [] #NodeID -> node             # Nodes are stored at the index of their nodeId
        self.edges = [] #from->to & call -> edge    # Stored at the index of their edgeId
        self.node_count = 0
        self.edge_count = 0

        # Give ability to reference last system call
        self.lastSystemCall = (fd, "accept4", originalTuple, fd, masterPid)

        # Create the initial graph
        remote, local = originalTuple.split("->")
        remoteNode = self.addNode(fd, remote, masterPid, False, "diamond")
        localNode = self.addNode(fd, local, masterPid, False, "diamond")
        self.addEdge(remoteNode, localNode, "accept4", False, "solid")
        PIDNode = self.addNode(fd, masterPid, masterPid, True, "rectangle")
        self.addEdge(localNode, PIDNode, "", False, "dashed")
            
        

        #TODO --> how to implement the currentfd?
        
    def __eq__(self, other):
        return (
            self.masterPID == other.masterPID and
            self.originalTuple == other.originalTuple
        )
    
    # Updates last encountered system call
    def updateLastSystemCall(self, fd:int, syscall:str, args:str, ret:str, pid:int):
        self.lastSystemCall = (fd, syscall, args, ret, pid)
    
    # Returns the process node that shares a PID with the specified PID
    def getProcessNode(self, currentpid:int):
        return next((node.nodeId for node in self.nodes if node.nodePID == currentpid
                      and node.isProcess == True), None)   # Fallthrough will be original process node?
    # TODO ->Replace/create a temp process node to see if things are happening that were never started?
    
    def addFD(self, fd:int):
        if fd not in self.fdList:
            self.fdList.add(fd)

    # Method to add Node to subgraph
    def addNode(self, fd:int, args:str, pid:int, isProcess:bool, shape:str):
        exists = self.findNode(fd, pid, args)
        if exists is not None:
            # print("Node already exists with ID: {exists}")
            return exists                   # Is the nodeId
        else:
            newNode = Node(self.node_count, fd, pid, args, isProcess, shape)
            self.nodes.insert(self.node_count, newNode) # Inset at the current node_count index
            self.node_count += 1            # Increase this index
            self.addFD(fd)                  # Adds FD to list of potential FDs
            return newNode.nodeId           # Returns id (index) of the newNode

    def addEdge(self, isFrom:'Node', isTo:'Node', syscall:str, isBidirectional:bool, edgeType:str):
        exists = self.findEdge(isFrom, isTo, syscall)
        if exists is not None:
            # print("Node already exists with ID: {exists}")
            return exists
        inverse = self.findEdge(isTo, isFrom, syscall)      # If this edge will make another bidirection, do that instead of adding instance
        if inverse is not None:
            self.edges[inverse].isBidirectional = True
        else:
            newEdge = Edge(self.edge_count, isFrom, isTo, syscall, isBidirectional, edgeType)
            self.edges.insert(self.edge_count, newEdge)
            self.edge_count += 1
            return newEdge.edgeId           # Returns the id (index of the newEdge)

    # Method to find a Node if it already exists in the subgraph
    # If nothing matches, returns None
    def findNode(self, fd:int, args:str, pid:int):
         return next((node.nodeId for node in self.nodes if 
                         node.fd == fd and node.args == args
                         and node.nodePID == pid), None)
    
    def findEdge(self, fromNode:'Node', toNode:'Node', syscall:str):
        return next((edge.edgeId for edge in self.edges if
                     edge.isFrom == fromNode and
                     edge.isTo == toNode and edge.syscall == syscall), None)

    def updateEdge():
        return 0
    
    def updateNode(self, nodeBeingUpdatedId:int, updatedArgs:str, updatedProcess:bool, updatedShape:str):
        # See if our updated node already exists
        existingUpdatedNodeId = self.findNode(self.nodes[nodeBeingUpdatedId].nodeId, updatedArgs, self.nodes[nodeBeingUpdatedId].nodePID)
        if existingUpdatedNodeId is not None:
            self.moveEdges(nodeBeingUpdatedId, existingUpdatedNodeId)   #TODO
            self.nodes[nodeBeingUpdatedId] = None                       # Set the old node (no longer being used) to None. We don't want to mess with indicies
            return existingUpdatedNodeId                                # Return the id of the node we shifted to
        else :
            self.nodes[nodeBeingUpdatedId].args = updatedArgs
            self.nodes[nodeBeingUpdatedId].isProcess= updatedProcess
            self.nodes[nodeBeingUpdatedId].shape = updatedShape
            return nodeBeingUpdatedId           # Id of the node we updated
        
    def moveEdges(self, fromId:int, toId:int):
        nodeTo = self.nodes[toId]
        nodeFrom = self.nodes[fromId]
        if nodeTo is None:
            raise Exception("Invalid node being shifted to")
        if nodeFrom is None:
            raise Exception("Invalid node being shifted from")
        
        # Shift all edges
        for edge in self.edges:
            if edge.isTo == fromId:
                self.addEdge(edge.isFrom, toId, edge.syscall, edge.isBidirectional, edge.edgeType)
            if edge.isFrom == fromId:
                self.addEdge(toId, toId, edge.syscall, edge.isBidirectional, edge.edgeType)

    def __repr__(self):
        return f"Subgraph({self.fdList, self.masterPID, self.originalTuple, self.isValid, self.nodes, self.edges})"
    
# ------------------------------------------------------------------------------------------------------------------------------
class Node:
    def __init__(self, nodeId:int, fd:int, nodePID:int, args:str, isProcess:bool, shape:str):
        self.nodeId = nodeId
        self.fd = fd
        self.nodePID = nodePID
        self.args = args
        
        self.isProcess = isProcess
        self.shape = shape
        # self.graph = graph


    def __repr__(self):
        return f"{self.fd, self.nodePID, self.args}"
    
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

    def formatNodeKey(self, key):
        return "_".join(map(str, key))

# ------------------------------------------------------------------------------------------------------------------------------ 
class Edge:
    def __init__(self, edgeId:int, isFrom:Node, isTo:Node, syscall:str, isBidirectional:bool, edgeType:str):
        self.edgeId = edgeId
        self.isFrom = isFrom # Do we want to swap this with the actual node structs or nah
        self.isTo = isTo
        self.syscall = syscall
        
        self.isBidirectional = isBidirectional
        self.edgeType = edgeType

    def __repr__(self):
        return f"Edge({self.isFrom, self.isTo, self.syscall, self.isBidirectional, self.edgeType})"
        
    def __eq__(self, comparison):
        return (
            (self.isFrom == comparison.isFrom and
            self.isTo == comparison.isTo and 
            self.syscall == comparison.syscall)
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
    ''' Collections of system calls and their constraints '''

    def parseLine(self, line:str):
        # Define pattern to match all valid inputs to
        pattern = r"(FD|Syscall|Args|Return|PID):([^,\n]+)"
        parsedInputs = re.findall(pattern, line)

        # Handle broad invalid case
        if parsedInputs is None or len(parsedInputs) != 5:
            # Do nothing
            return None

        # Handle non invalid case(s)
        output = [None, None, None, None, None]
        # Load values
        for index, value in enumerate(parsedInputs):
            output[index] = value[1]               

        # Run other argument parsing efforts
        # Return
        # print(output) #TODO make look pretty
        output[0] = self.parseFD(output[0])
        # output[3] = self.parseRet(output[3])
        output[4] = self.parsePID(output[4])
        
        # Go to next parsing step
        self.parseSyscall(output[0],output[1],output[2],output[3],output[4])
            

    def parseSyscall(self, fd:int, syscall:str, args:str, ret:str, pid:int):
        ''' A long function designed to do heavy lifting with the handling of special cases '''
        # Swap to current graph -> if we don't encounter a known FD assume we're in the same graph
        globalGraphManager.swapSubgraph(pid, fd)

        # Find the process node for the incoming call, or create a dummy 'floating' process node
        if(globalGraphManager.current_Graph is not None):
            processNodeId = globalGraphManager.current_Graph.getProcessNode(pid)
            if processNodeId is None:
                processNodeId = globalGraphManager.current_Graph.addNode(fd, pid, pid, True, "rectangle")
        
        
        # print(syscall)
        if syscall in {"accept4", "accept"} and fd != -1:
            args = self.parseArgs(args, "networkTuple")
            # Find if matches connect call? TODO
            globalGraphManager.addSubgraph(fd, pid, args)
            globalGraphManager.current_Graph.updateLastSystemCall(fd, syscall, args, ret, pid)

        
        # For connect we only want to pull the result system call, not the initial
        # Take the tuple and save ALL of it
        if syscall in {"connect"} and ret != "<NA>":
            args = self.parseArgs(args, "networkTuple")
            socketNode = globalGraphManager.current_Graph.findNode(fd, args, pid)
            if socketNode is not None:
                print(f"Node we're looking to replace: {globalGraphManager.current_Graph.nodes[socketNode]}")
                updatedNode = globalGraphManager.current_Graph.updateNode(socketNode, args, False, "diamond")
                globalGraphManager.current_Graph.addEdge(processNodeId, updatedNode, syscall, False, "solid")

            # If socket node never existed make hanging
            else:
                newNode = globalGraphManager.current_Graph.addNode(fd, args, pid, False, "diamond")
                globalGraphManager.current_Graph.addEdge(processNodeId, newNode, syscall, False, "solid") 

        # Handle other non-connective system calls
        if globalGraphManager.current_Graph is not None:
            
            # Add new node that follows some specifics
            # If the FD is not -1 ( a Null/failed operation )
            if fd != -1:

                # If our desired information is in the args
                if "res" in args and globalGraphManager.current_Graph.lastSystemCall[1] == syscall:
                    pass


        # Update last system call
        if globalGraphManager.current_Graph is not None:
            globalGraphManager.current_Graph.updateLastSystemCall(fd, syscall, args, ret, pid)
        return

    def parseArgs(self, args:str, options:str):
        if options == 'networkTuple':
            networkTuple = re.search(r"tuple=([^\s]+)", args)
            return networkTuple.group(1) if networkTuple else None

        if options == 'fileDescriptor':
            fd = re.search(r"")
    
    def parseFD(self, fd:str):
        if fd == '<NA>':
            return -1
        else:
            return int(fd)
        
    def parseRet(self, ret:str):
        if ret == '<NA>':
            return None
        else:
            return int(ret)
        
    def parsePID(self, pid:str):
        if pid == '<NA>':
            return None
        else:
            return int(pid)

# ------------------------------------------------------------------------------------------------------------------------------
# other functions:
def formatKeyForPrinting(fd:int, pid:int, args:str, nodeKey:tuple):
    if nodeKey is None:
        output = ("".join(map(str, (fd, pid, args)))).replace(".", "").replace(":", "")
    else:
        output = "".join(map(str, nodeKey)).replace(".", "").replace(":", "")
    return output

def handleConnectionSystemCall():
    return

def printSubgraphMetadata():
    return

def createDOT(setting: str):
    # get current timestamp
    timestamp = datetime.now().timestamp()
    timestamp_str = str(int(timestamp))
    dirName = f"./Dot_Files/Timestamp_{timestamp_str}/"

    if setting == "individual":
        try:
            os.mkdir(dirName)
            print(f"Directory '{dirName}' created successfully!")
        except FileExistsError:
            raise Exception(f"The directory '{dirName}' already exists.")
        except PermissionError:
            raise Exception(f"PermissionError: You don't have permission to create the directory.")
        except Exception as e:
            raise Exception(f"An error occurred: {e}")

        # i = 0
        for index, (key, graph) in enumerate(globalGraphManager.graphList.items()):
            with open(f"./Dot_Files/Timestamp_{timestamp_str}/graph{index}.dot", "w") as dot:
                print(f"Created graph {dot}")
                dot.write("digraph nginx_syscalls {\n")
                dot.write("rankdir=LR;\n")

                for node in graph.nodes:
                    # node_identifier = formatKeyForPrinting(None, None, None, nodeKey=nodekey)
                    dot.write(f"    {node.nodeId} [label=\"{node.args}\", shape={node.shape}];\n")

                for edge in graph.edges:
                    # fromKey = formatKeyForPrinting(edge.isFrom.fd, edge.isFrom.nodePID, edge.isFrom.args, None)
                    # toKey = formatKeyForPrinting(edge.isTo.fd, edge.isTo.nodePID, edge.isTo.args, None)
                    dot.write(
                        f"    {graph.nodes[edge.isFrom].nodeId} -> {graph.nodes[edge.isTo].nodeId} "
                        f"[style=\"{edge.edgeType}\", label=\"{edge.syscall}\", minlen=2, weight=2];\n")


                if (graph.isValid == 1) :
                    dot.write(f"  -1 [label=\"Graph Did Not Receive 'Close' Syscall\", shape=box, penwidth=4, color=red, pos=\"5,5!\"];\n")
                
                dot.write("}\n")#close subgraph
                # i++
                
        
    else:
        createDOT("individual")

    print(f"printed graphs(s): {timestamp_str}")


def getNodeFD():
    return

def getProcessNode():
    return

def getSubgraphFD():
    return

def moveFDToArgs():
    return

def main():
    # Open target trace file
    p = Parser()

    with open(".\\Falco Trace Files\\TestEvents.txt", 'r') as file:
        # Read the content of the file
        for line in file:
            content = p.parseLine(line)
            if (content):
                pass
                # print(content)
        
        # for graph in globalGraphManager.graphList.values():
            # print(f"{graph}")

    createDOT("individual")
    # # go through file line by line
    # i = 0
    # for line in input_log:
    #     i = i+1

    # Include additional debugging information if desired
    # if (DEBUG_LEVEL == 1) :
    #     printSubgraphMetadata()
    # createDOT()
    # return i


if __name__ == "__main__":
    main()