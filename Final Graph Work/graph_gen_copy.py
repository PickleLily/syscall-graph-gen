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
totalGraphs = -1
currentGraph = -1

# ------------------------------------------------------------------------------------------------------------------------------

class GraphManager:
    def __init__(self):
        self.graphList = {}         # key: (pid, networkTuple), value: Graph
        self.current_Graph = None   # Graph
        self.totalGraphs = 0

    def addSubgraph(self, fd:int, pid:int, networkTuple:str):
        isDuplicate = self.duplicateNetwork(networkTuple)
        if isDuplicate is not None:
            isDuplicate.isValid = -1
            isDuplicate.fdList = None
        newGraph = Subgraph(fd, pid, networkTuple)
        key = (self.totalGraphs, newGraph.masterPID, newGraph.originalTuple)
        self.graphList[key] = newGraph
        self.current_Graph = newGraph  
        self.totalGraphs += 1 
            # print(f"Made key {key}")

    # Return the graph object or lack thereof 
    def duplicateNetwork(self, networkTuple:str):
        return next((graph for graph in self.graphList.values() 
                    if graph.isValid == 0 and 
                    networkTuple == graph.originalTuple), None)
    
    # Return the graph object that we are "HOPEFULLY" looking at
    def swapSubgraph(self, pid:int, fd:int):
        return next((graph for graph in self.graphList.values() if graph.isValid == 0 and fd in graph.fdList), None)

globalGraphManager = GraphManager()   

# ------------------------------------------------------------------------------------------------------------------------------

class Subgraph:

    def __init__(self, fd:int, masterPid:int, originalTuple:str): #Make lsit of potential fd
        # totalGraphs = totalGraphs + 1
        # currentGraph = totalGraphs
        # graphs[currentGraph] = self
        
        self.fdList = set()                            # Initialize the list of valid fds to nothing  
        self.fdList.add(fd)
        self.masterPID = masterPid                  # initialize the masterPID (PID or original connection) through input
        self.originalTuple = originalTuple          # Initialize the original connection tuple of the graph
        self.isValid = OPEN                         # Initialize the validity of a graph to OPEN automatically
        self.nodes = {} #NodeID -> node             # (fd, pid, args)
        self.edges = {} #from->to & call -> edge    # (isFrom, isTo, syscall) -> May want to convert to the node's keys
    
        # Unsure if we need any of what is below
        self.graphNum = totalGraphs
        self.node_count = 0
        self.edge_count = 0

        # print(f"Hit syscall accept4 for FD:{fd} and Tuple:{originalTuple} and PID:{masterPid}")
        # Create the initial graph
        remote, local = originalTuple.split("->")
        remoteNode = self.addNode(fd, remote, masterPid, False, "diamond")
        localNode = self.addNode(fd, local, masterPid, False, "diamond")
        self.addEdge(remoteNode, localNode, "accept4", False, "solid")
        PIDNode = self.addNode(fd, masterPid, masterPid, True, "rectangle")
        self.addEdge(localNode, PIDNode, "", False, "dashed")
            
        # return self

        #TODO --> how to implement the currentfd?
        
    def __eq__(self, other):
        return (
            self.masterPID == other.masterPID and
            self.originalTuple == other.originalTuple
        )
    
    def addFD(self, fd:int):
        if fd not in self.fdList:
            self.fdList.add(fd)

    # Method to add Node to subgraph
    def addNode(self, fd:int, args:str, pid:int, isProcess:bool, shape:str):
        exists = self.findNode(fd, pid, args)
        if exists is not None:
            # print("Node already exists with ID: {exists}")
            return exists
        else:
            newNode = Node(fd, pid, args, isProcess, shape)
            key = (fd, pid, args)
            self.nodes[key] = newNode
            self.node_count += 1
            self.addFD(fd)                  # Adds FD to list of potential FDs
            return newNode

    def addEdge(self, isFrom:'Node', isTo:'Node', syscall:str, isBidirectional:bool, edgeType:str):
        exists = self.findEdge(isFrom, isTo, syscall)
        if exists is not None:
            # print("Node already exists with ID: {exists}")
            return exists
        else:
            newEdge = Edge(isFrom, isTo, syscall, isBidirectional, edgeType)
            key = (isFrom, isTo, syscall)
            self.edges[key] = newEdge
            self.edge_count += 1

    # Method to find a Node if it already exists in the subgraph
    # If nothing matches, returns None
    def findNode(self, fd:int, args:str, pid:int):
        key = (fd, pid, args)
        return self.nodes.get(key)
    
    def findEdge(self, fromNode:'Node', toNode:'Node', syscall:str):
        key = (fromNode, toNode, syscall)
        # altkey = (toNode, fromNode, syscall)
        # if (altkey):
        #     self.edges[altkey].isBidirectional = True
        return self.edges.get(key)

    def updateEdge():
        return 0
    
    def updateNode():
        return 0 

    def __repr__(self):
        return f"Subgraph({self.fdList, self.masterPID, self.originalTuple, self.isValid, self.nodes.values(), self.edges.values()})"
    
# ------------------------------------------------------------------------------------------------------------------------------
class Node:
    def __init__(self, fd:int, nodePID:int, args:str, isProcess:bool, shape:str):
        # self.nodeID = nodeID
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
    def __init__(self, isFrom:Node, isTo:Node, syscall:str, isBidirectional:bool, edgeType:str):
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
        output[3] = self.parseRet(output[3])
        output[4] = self.parsePID(output[4])
        
        # Go to next parsing step
        self.parseSyscall(output[0], output[1],output[2],output[3],output[4])
            

    def parseSyscall(self, fd:int, syscall:str, args:str, ret:int, pid:int):
        ''' A long function designed to do heavy lifting with the handling of special cases '''
        # print(syscall)
        if syscall in ('accept4', 'accept') and fd != -1:
            args = self.parseArgs(args, "networkTuple")
            # Find if matches connect call? TODO
            globalGraphManager.addSubgraph(fd, pid, args)
        return

    def parseArgs(self, args:str, options:str):
        if options == 'networkTuple':
            networkTuple = re.search(r"tuple=([^\s]+)", args)
            return networkTuple.group(1) if networkTuple else None
    
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
    parentDir = os.path.dirname(f"./Dot_Files/")
    dirName = f"./Dot_Files/Timestamp_{timestamp_str}/"

    if setting == "individual":
        # make subdi
        # char makeCommand[256];
        # makeCommand = f('mkdir "./Dot_Files/Timestamp_{time}"')
        try:
            os.mkdir(dirName)
            print(f"Directory '{dirName}' created successfully!")
        except FileExistsError:
            print(f"The directory '{dirName}' already exists.")
        except PermissionError:
            print(f"PermissionError: You don't have permission to create the directory.")
        except Exception as e:
            print(f"An error occurred: {e}")
        # sprintf(makeCommand, "mkdir \".\\Dot_Files\\Timestamp_%s\"", timeBuffer);
        # if (system(makeCommand) == -1){ // Try the command
        #     perror("Could not make subdirectory for graphs");
        # }
        i = 0
        for graphkey, graph in globalGraphManager.graphList.items():
            # filename = f".\\Dot_Files\\Timestamp_{timestamp}\\graph{i}"
            with open(f"./Dot_Files/Timestamp_{timestamp_str}/graph{i}.dot", "w") as dot:
                print(f"Created graph {dot}")
                dot.write("digraph nginx_syscalls {\n")
                dot.write("rankdir=LR;\n")

                for nodekey, node in graph.nodes.items():
                    node_identifier = formatKeyForPrinting(None, None, None, nodeKey=nodekey)
                    dot.write(f"    {node_identifier} [label=\"{node.args}\", shape={node.shape}];\n")

                for egdekey, edge in graph.edges.items():
                    fromKey = formatKeyForPrinting(edge.isFrom.fd, edge.isFrom.nodePID, edge.isFrom.args, None)
                    toKey = formatKeyForPrinting(edge.isTo.fd, edge.isTo.nodePID, edge.isTo.args, None)
                    dot.write(
                        f"    {fromKey} -> {toKey} "
                        f"[style=\"{edge.edgeType}\", label=\"{edge.syscall}\", minlen=2, weight=2];\n")


                if (graph.isValid == 1) :
                    dot.write(f"  -1 [label=\"Graph Did Not Receive 'Close' Syscall\", shape=box, penwidth=4, color=red, pos=\"5,5!\"];\n")
                
                dot.write("}\n")#close subgraph
                
        
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

    with open("./Falco Trace Files/TestEvents.txt", 'r') as file:
        # Read the content of the file
        for line in file:
            content = p.parseLine(line)
            if (content):
                pass
                # print(content)
        
        for graph in globalGraphManager.graphList.values():
            print(f"{graph}")

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