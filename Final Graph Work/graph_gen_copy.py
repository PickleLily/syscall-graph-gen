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

# Graph graphState types
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
            isDuplicate.graphState = INVALID
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
                    if graph.graphState == 0 and 
                    networkTuple == graph.originalTuple), None)
    
    # Return the graph object that we are "HOPEFULLY" looking at
    def swapSubgraph(self, pid:int, fd:int):
        tempGraph = next((graph for graph in self.graphList.values() if graph.graphState == 0 and fd in graph.fdList), self.current_Graph)
        if tempGraph is not None and tempGraph.graphState != 0:
            self.current_Graph = None
        else:
            self.current_Graph = tempGraph

globalGraphManager = GraphManager()   

# ------------------------------------------------------------------------------------------------------------------------------

class Subgraph:

    def __init__(self, fd:int, masterPid:int, originalTuple:str):
        self.fdList = set()                         # Initialize a set of valid fds
        self.fdList.add(fd)                         # Add the current fd to that set
        self.masterPID = masterPid                  # initialize the masterPID (PID or original connection) through input
        self.originalTuple = originalTuple          # Initialize the original connection tuple of the graph
        self.graphState = OPEN                         # Initialize the validity of a graph to OPEN automatically
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
            # node.nodePID
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
        tempNode = next((node.nodeId for node in self.nodes if 
                         node.fd == fd and node.args == args
                         and node.nodePID == pid), None)
        if tempNode and fd not in self.fdList:
            self.fdList.add(fd)
        return tempNode
    
    def findEdge(self, fromNode:int, toNode:int, syscall:str):
        return next((edge.edgeId for edge in self.edges if
                     edge is not None and 
                     edge.isFrom == fromNode and
                     edge.isTo == toNode and edge.syscall == syscall), None)

    def updateEdge(self, edgeBeingUpdatedId:int, updatedSyscall:str):
        # See if the updated edge already exists
        existingEdgeId = self.findEdge(self.edges[edgeBeingUpdatedId].isFrom, self.edges[edgeBeingUpdatedId].isTo, updatedSyscall)
        if existingEdgeId is not None:          # If the updated edge exists just "delete" this one
            self.edges[edgeBeingUpdatedId] = None
            return None
        existingOtherDirectionId = self.findEdge(self.edges[edgeBeingUpdatedId].isTo, self.edges[edgeBeingUpdatedId].isFrom, updatedSyscall)
        if existingOtherDirectionId is not None:
            self.addEdge(self.edges[existingOtherDirectionId].isTo, self.edges[existingOtherDirectionId].isFrom, self.edges[existingOtherDirectionId].syscall, True, self.edges[edgeBeingUpdatedId].edgeType)
            return existingOtherDirectionId
        
        self.edges[edgeBeingUpdatedId].syscall = updatedSyscall
        return edgeBeingUpdatedId
    
    def updateNode(self, nodeBeingUpdatedId:int, updatedArgs:str, updatedProcess:bool, updatedShape:str, updateFD=None):
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
            if updateFD is not None:
                self.nodes[nodeBeingUpdatedId].fd = updateFD
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
                
    def isBidirectional(self, from_node_ID, to_node_ID):
        for e in self.edges:

            if e.isTo == from_node_ID and e.isFrom == to_node_ID:
                return True
        return False 

    def __repr__(self):
        return f"Subgraph({self.fdList, self.masterPID, self.originalTuple, self.graphState, self.nodes, self.edges})"
    
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
    def __init__(self, edgeId:int, isFrom:int, isTo:int, syscall:str, isBidirectional:bool, edgeType:str):
        self.edgeId = edgeId
        self.isFrom = isFrom
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
        if(output[0] == 44 and output[1]=='fcntl' and output[2] == 'fd=44(<4t>127.0.0.1:34262->127.0.0.1:3306) cmd=5(F_SETFL)' and output[4]==1003):
            print('hi')
        self.parseSyscall(output[0],output[1],output[2],output[3],output[4])
            

    def parseSyscall(self, fd:int, syscall:str, args:str, ret:str, pid:int):
        ''' A long function designed to do heavy lifting with the handling of special cases '''
        # If there is no active graph, don't progress
        if globalGraphManager.current_Graph is None and syscall not in {'accept4', 'accept'}:
            return
        
        # Swap to current graph -> if we don't encounter a known FD assume we're in the same graph
        globalGraphManager.swapSubgraph(pid, fd)

        # Find the process node for the incoming call, or create a dummy 'floating' process node
        if globalGraphManager.current_Graph is not None:
            processNodeId = globalGraphManager.current_Graph.getProcessNode(pid)
            if processNodeId is None:
                processNodeId = globalGraphManager.current_Graph.addNode(fd, pid, pid, True, "rectangle")
        
        # print(syscall)
        dontUpdateLastSystemCall = False

        # Handling of specific system calls
        if syscall in {"accept4", "accept"} and fd != -1:
            args = self.parseArgs(args, "networkTuple")
            # If current graph is not None check for instance of tuple existing before
            if globalGraphManager.current_Graph is not None:
                # Check to see if accept is related to a connect call
                for node in globalGraphManager.current_Graph.nodes:
                    # If we find a node where the args match (a socket node)
                    if node.args == args:    
                        for nodeTwo in globalGraphManager.current_Graph.nodes:
                            # If we find a node that has the PID & is a process node
                            # Update that node and add the edge there
                            if nodeTwo.nodePID == pid and nodeTwo.isProcess == True:          
                                updateNode = globalGraphManager.current_Graph.updateNode(nodeTwo.nodeId, pid, True, "rectangle", updateFD=fd)
                                globalGraphManager.current_Graph.addEdge(node.nodeId, updateNode, syscall, False, "solid")
                        # Else if no PID node
                        newPidNode = globalGraphManager.current_Graph.addNode(fd, pid, pid, True, "rectangle")
                        globalGraphManager.current_Graph.addEdge(node.nodeId, newPidNode, syscall, False, "solid")
            
            # If no graphs exist make graph
            # Fall through of the accept4 not relating to a connect call also leads here
            globalGraphManager.addSubgraph(fd, pid, args)
            return
        
        # For connect we only want to pull the result system call, not the initial
        # Take the tuple and save ALL of it
        elif syscall in {"connect"} and ret != "<NA>":
            args = self.parseArgs(args, "networkTuple")
            socketNode = globalGraphManager.current_Graph.findNode(fd, str(fd), pid)
            if socketNode is not None:
                print(f"Node we're looking to replace: {globalGraphManager.current_Graph.nodes[socketNode]}")
                updatedNode = globalGraphManager.current_Graph.updateNode(socketNode, args, False, "diamond")
                globalGraphManager.current_Graph.addEdge(processNodeId, updatedNode, syscall, False, "solid")
            # If socket node never existed make hanging
            else:
                newNode = globalGraphManager.current_Graph.addNode(fd, args, pid, False, "diamond")
                globalGraphManager.current_Graph.addEdge(processNodeId, newNode, syscall, False, "solid")
            return

        # Handle close calls by removing active FDs from the currentGraph
        # It seems that ret has to be <NA> for valid close calls
        elif syscall in {"close"} and ret == '<NA>':
            # Ensure this is a valid fd that we are already checking
            if fd in globalGraphManager.current_Graph.fdList:
                args = self.parseArgs(args, 'fileDescriptor')
                # If we are closing a network connection
                if '->' in args:
                    # Check tuple to close entire graph
                    if globalGraphManager.current_Graph.originalTuple == args:
                        # Add close edge towards network tuple, close graph -> This will always flow from PID node (2) to local tuple node (1)
                        globalGraphManager.current_Graph.addEdge(2, 1, "close", False, "solid")
                        # Then handle the shutting of the entire graph -> Set graphState to CLOSED and then set the fdList to None (empty)
                        globalGraphManager.current_Graph.graphState = CLOSED
                        globalGraphManager.current_Graph.fdList = None
                    # This is a different node altogether
                    # TODO this may need to be handled differently than looping other network tuples into  regular case
                else:
                    # Get target Node we are "closing"
                    targetNode = globalGraphManager.current_Graph.findNode(fd, args, pid)
                    if targetNode is not None:
                        globalGraphManager.current_Graph.addEdge(processNodeId, targetNode, syscall, False, "solid")
                        globalGraphManager.current_Graph.fdList.remove(fd)
                    # Remove fd from valid list of fd
            return
                    
        # Handle other non-connective system calls
        # Filter out specific ignorable syscalls we weren't before here
        # Add new node that follows some specifics
        # If the FD is not -1 ( a Null/failed operation )
        elif fd != -1:

            if "res" not in args:
                # Parse the args as a FD type
                args = self.parseArgs(args, "fileDescriptor")

                # Overwrite an empty form as FD itself
                if args == "":
                    args = str(fd)

                # Check to see if we're interacting with the original socket
                if globalGraphManager.current_Graph.originalTuple in args:
                    # If we are, we send all edges from local side
                    processNodeId = 1
                    nodeId = 2
                else:
                    nodeId = globalGraphManager.current_Graph.findNode(fd, args, pid)
                
                # This node exists
                if nodeId is not None:
                    # See if we have to update blank edge showing this is now a valid graph (Very hard-coded)
                    if globalGraphManager.current_Graph.edge_count == 2 and processNodeId == globalGraphManager.current_Graph.nodes[1].nodeId:
                        globalGraphManager.current_Graph.edges[1].edgeType = "solid"
                        globalGraphManager.current_Graph.edges[1].syscall = syscall
                    else:
                        globalGraphManager.current_Graph.addEdge(processNodeId, nodeId, syscall, False, "solid")
                # Ths node does not exist
                else:                              
                    newNode = globalGraphManager.current_Graph.addNode(fd, args, pid, False, "ellipse")
                    globalGraphManager.current_Graph.addEdge(processNodeId, newNode, syscall, False, "solid")
                
            # If our desired information is in the args and it is a legal system call to update
            elif "res" in args and syscall not in {'flock', 'close'} and globalGraphManager.current_Graph.lastSystemCall[1] == syscall:
                targetNode = globalGraphManager.current_Graph.findNode(fd, globalGraphManager.current_Graph.lastSystemCall[2], pid)
                if targetNode is None or fd not in globalGraphManager.current_Graph.fdList:
                    print(f"Old call: {globalGraphManager.current_Graph.lastSystemCall[0], globalGraphManager.current_Graph.lastSystemCall[1], globalGraphManager.current_Graph.lastSystemCall[2], globalGraphManager.current_Graph.lastSystemCall[3], globalGraphManager.current_Graph.lastSystemCall[4]}")
                    print(f"New call: {fd}, {syscall}, {args}, {ret}, {pid}")
                    print(" Trying to modify edge for a syscall we haven't seen before ")
                else:
                    args = self.parseArgs(args, "data")
                    targetEdge = globalGraphManager.current_Graph.findEdge(processNodeId, targetNode, syscall)
                    updated_args = f"{syscall} - {args}"
                    if "None" not in updated_args:
                        globalGraphManager.current_Graph.updateEdge(targetEdge, updated_args)
                dontUpdateLastSystemCall = True

            # Update last system call
            if globalGraphManager.current_Graph is not None and (dontUpdateLastSystemCall is not None or dontUpdateLastSystemCall == False):
                globalGraphManager.current_Graph.updateLastSystemCall(fd, syscall, args, ret, pid)
            return

    def parseArgs(self, args:str, options:str):
        if options == 'networkTuple':
            networkTuple = re.search(r"tuple=([^\s]+)", args)
            return networkTuple.group(1) if networkTuple else None

        if options == 'fileDescriptor':
            fd = re.search(r"<[^>]+>(\s*[^)]*)", args)        # This should also pull out <f> and 
            if fd is not None: 
                output = fd.group(1)
                if '(' in output:
                    output = output + ')'
                return output
            return None
        
        if options == 'data':
            data = re.search(r"data=(.*?)(?=(fd|tuple|$))", args)
            if data is not None:
                output = data.group(1).replace("...", ".").replace("\"", "'")
                output = output.replace("{", "").replace("}","")
                output = output.replace("(", "").replace(")", "").strip()
                if len(set(output)) != 1 and output != "" and "NULL" not in output:
                   return output
    
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
                print(f"Created graph {dot.name}")
                dot.write("digraph nginx_syscalls {\n")
                dot.write("rankdir=LR;\n")

                for node in graph.nodes:
                    if node is not None:
                    # node_identifier = formatKeyForPrinting(None, None, None, nodeKey=nodekey)
                        if node.args == "" or node.args == node.nodePID:
                            dot.write(f"    {node.nodeId} [label=\"{node.args}\", shape={node.shape}];\n")
                        else:
                            dot.write(f"    {node.nodeId} [label=\"{node.args}  - {node.nodePID}\", shape={node.shape}];\n")

                for edge in graph.edges:
                    if edge is not None:
                    # fromKey = formatKeyForPrinting(edge.isFrom.fd, edge.isFrom.nodePID, edge.isFrom.args, None)
                    # toKey = formatKeyForPrinting(edge.isTo.fd, edge.isTo.nodePID, edge.isTo.args, None)
                        dot.write(
                            f"    {graph.nodes[edge.isFrom].nodeId} -> {graph.nodes[edge.isTo].nodeId} "
                            f"[style=\"{edge.edgeType}\", label=\"{edge.syscall}\", minlen=2, weight=2];\n")

                if (graph.graphState == 1) :
                    dot.write(f"  -1 [label=\"Graph Did Not Receive 'Close' Syscall\", shape=box, penwidth=4, color=red, pos=\"5,5!\"];\n")
                
                dot.write("}\n")#close subgraph
    else:
        createDOT("individual")

    print(f"printed graphs(s): {timestamp_str}")


def getNodeFD():
    return


def getSubgraphFD():
    return

def moveFDToArgs():
    return

def main():
    # Open target trace file
    p = Parser()

    with open(".\Falco Trace Files\Command_in\high_exploit.txt", 'r') as file:
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
    # createDOT()
    # return i


if __name__ == "__main__":
    main()