#!/usr/bin/python

"""
This module defines a maze class and provides search methods for navigating the maze. 

A maze is specified by a text file containing rows of characters, where each character defines a 
cell's (row,column) location in a rectangular grid and specifies the cell value as either the maze 
start, finish, an open cell, or a blocked cell. 

A maze node class is defined to represent a search node in the maze. It contains all the relevant 
information about a particular search path through the maze, including all states, actions taken, 
and associated costs.

Various search methods are implemented to find a path and action sequence from a start to finish.

The module provides a main method that can be invoked with a specified maze file. The file is 
read in, the various searches performed on the maze (with default options set), and results are
reported.
    
    Example:    
        
        $ python MazeSearch.py maps/map0.txt
        
Alternatively, the user may create his own Maze object and use the defined search() function to
perform a search over it.
"""

import sys  
import re   
from heapq import heappush, heappop, heapify
from time import time

##################################################
# G L O B A L   D A T A 
##################################################
MAZE_DEFS = {'start':'i', 'finish':'g', 'empty':'0', 'wall':'x'} # define maze character encoding 
MAZE_ACTS = {'N', 'S', 'W', 'E', 'NW', 'NE', 'SW', 'SE'}; # all possible actions
ACT_OFFSETS = {'N':(-1,0), 'S':(1,0), 'W':(0,-1), 'E':(0,1), 'NW':(-1,-1), 'NE':(-1,1), 
               'SW':(1,-1), 'SE':(1,1)}; # define (row,col) updates that result from a given action


##################################################
# C L A S S   D E F I N I T I O N S
##################################################
class Maze:
    """
    Maze class provides access methods to underlying 2D layout.
            
    Member Data:
    - layout (2D list): inner lists contain chars and represent rows, elements (chars) in the 
      list represent the columns in the given row
    - width (int): number of columns in maze
    - height (int): number of rows in maze        
    - start (tuple): (row,col) location in maze
    - finish (tuple): (row,col) location in maze    
    - dispStr (string): display string representing the maze  
    - heuristicFunct (function): h(x,y) that gives heuristic distance between two cells (where a 
      cell is represented as a 2-tuple) in the maze. The default heuristic is the zero function.     
    
    Member Functions:
    - __init__(fileName, actions (optional), h (optional)): construct a new maze object
    - __repr__(): return string representation of the maze
    - parseLayout(fileName): parse the given file for constructing a maze
    - isFinish(cell): determine whether cell is a goal state
    - inBounds(cell): determine whether a cell is inside the maze boundaries
    - isOpen(cell): determine whether a cell is unobstructed
    - getSuccessors(cell): return all successor states reachable from the cell in a single step
    - transitionCost(cell, act, nextCell): determine the cost of moving from one cell to another
    """   

    def __init__(self, fileName, actions=MAZE_ACTS, h=lambda x,y: 0):
        """
        Create new maze based on supplied layout.
        
        Parameters:
        - fileName (string): path and name to file containing maze layout  
        - actions (string set): actions allowed in this maze; by default all are allowed                   
        """

        # validate action list
        for a in actions:
            if a not in MAZE_ACTS:
                raise ValueError('Invalid action: ' + a);
        self.allowedActs = set(actions);
        
        # parse file containing maze layout
        mazeVals = self.parseLayout(fileName);
        self.layout = mazeVals['layout'];        
        self.width = mazeVals['width'];
        self.height = mazeVals['height'];        
        self.start = mazeVals['start'];
        self.finish = mazeVals['finish'];
        self.dispStr = mazeVals['dispStr'];
        
        # set heuristic function for this maze (default is the zero function)
        self.heuristicFunc = h;


    def __repr__(self):
        """
        Give string representation of the maze.
        """
        
        return self.dispStr;   
        
        
    def parseLayout(self, fileName):
        """
        Create new maze layout from the specified file.
        
        File should contain rows of characters, with each row separated by a newline. Characters
        within a row should not be separated. Valid mazes should be rectangular (jagged edges not
        supported). A valid layout consists of only the characters defined in MAZE_DEFS.
        
        For example, a file containing the following text will produce a valid maze:
                0000g
                0xx00
                00xx0
                00x00
                i0000
                
        Note: the final row should not terminate with a newline!
        
        Raises a ValueError excpetion if the layout given in the file is not valid.

        Parameters:
        - layout (2D list): inner lists represent rows, elements therein columns in the maze
        
        Returns:
        - layout (2D list): inner lists contain chars and represent rows, elements (chars) in the 
          list represent the columns in the given row
        - width (int): number of columns in maze
        - height (int): number of rows in maze        
        - start (tuple): (row,col) location in maze
        - finish (tuple): (row,col) location in maze    
        - dispStr (string): display string representing the maze                 
        """

        # open file and read lines from it
        f = open(fileName, 'r');
        layout = [list(line.strip()) for line in f]
        f.close()
        
        # set initial values
        width = len(layout[0]); # uniform width required!
        height = len(layout);  
        start = None; 
        finish = None;
        dispStr = ''; # build display string as we validate layout                 
        errs = [False, False, False, False]; # multi-start, multi-finish, bad char, short row

        # validate layout
        for idx, row in enumerate(layout):      
            # look for starting point   
            c = row.count(MAZE_DEFS['start']); 
            if (c == 1 and start is not None) or c > 1:
                errs[0] = True;                
            elif c == 1:            
                start = (idx,row.index(MAZE_DEFS['start']));
                
            # look for finish point
            c = row.count(MAZE_DEFS['finish']);
            if (c == 1 and finish is not None) or c > 1:
                errs[1] = True;                
            elif c == 1:            
                finish = (idx,row.index(MAZE_DEFS['finish']));
            
            # validate all chars in this row
            validChars = ''.join(value for key, value in MAZE_DEFS.items())
            regEx = '^[' + validChars + ']*$';
            if not re.match(regEx,''.join(row)): 
                errs[2] = True;
            
            # validate row length
            if len(row) != width: 
                errs[3] = True;
                
            # add to display string    
            dispStr += ''.join(cell for cell in row) + '\n';
            
        # handle start/finish point errors
        errMsg = ''
        if start is None: 
            errMsg += 'No start found. '
        if finish is None: 
            errMsg += 'No finish found. '
        if errs[0]: 
            errMsg += 'Multiple start points. '
        if errs[1]: 
            errMsg += 'Multiple end points. '
        if errs[2]: 
            errMsg += 'Invalid character found. '
        if errs[3]: 
            errMsg += 'Inconsistent row length. '
        if errMsg:  
            raise ValueError('Invalid maze layout: ' + errMsg);
            
        # put values in dict and return
        ret = {'layout':layout, 'width':width, 'height':height, 'start':start, 'finish':finish, 
               'dispStr':dispStr};
               
        return ret;        


    def isFinish(self, cell):
        """
        Determine whether a given cell in the maze is the exit.

        Parameters:
        - cell (tuple): (row,col) location in maze

        Returns:
        - (boolean): true iff the given location is the exit
        """
        
        return self.inBounds(cell) and self.layout[cell[0]][cell[1]] == MAZE_DEFS['finish'];    
        
        
    def inBounds(self, cell):
        """
        Determine whether a given cell is in bounds of the maze.

        Parameters:
        - cell (tuple): (row,col) location in maze

        Returns:
        - (boolean): false if the indicated cell is out of bounds otherwise true
        """
        
        if cell[0] < 0 or cell[1] < 0 or cell[0] > self.height-1 or cell[1] > self.width-1:
            return False;
        else:    
            return True;
        
        
    def isOpen(self, cell):
        """
        Determine whether a given cell in the maze is open.

        Parameters:
        - cell (tuple): (row,col) location in maze

        Returns:
        - (boolean): false if the indicated cell is out of bounds or a wall, otherwise true            
        """
        
        return self.inBounds(cell) and self.layout[cell[0]][cell[1]] != MAZE_DEFS['wall'];
        
     
    def getSuccessors(self, cell):
        """
        Get single-step successor state for each possible action in the given cell and the cost 
        associated with the transition.
        
        A valid successor cell is horizontally or vertically adjacent (or diagonal if set), in 
        bounds, with no wall blocking the cell.       
        
        Parameters:
        - cell (tuple): location (row,col) of cell to get successors for
        
        Returns: 
        - successors (dictionary): maps actions to the successor cells and costs, for example:
          if moving NE from cell (2,2) costs 1.5, then successors['NE'] = [(1,1),1.5]
        """
        
        successors = {};
        
        # try all allowed actions
        for act in self.allowedActs:
            offset = ACT_OFFSETS[act];
            nextCell = (cell[0]+offset[0], cell[1]+offset[1]);            
            if self.isOpen(nextCell): 
                transCost = self.transitionCost(cell, act, nextCell);
                successors[act] = [nextCell,transCost];

        return successors;
        
          
    def transitionCost(self, cell, act, nextCell):
        """
        Get the cost associated with a transition from one cell to the next via a given action. 
        
        Simple cost function that assigns diagonal moves a cost of X, all other allowed actions a
        cost of Y, and disallowed/unknown actions a cost of infinity. Note that while this 
        function ensures a given action is allowable in the maze, it does not verify that the move 
        is valid (e.g. next cell may be blocked or next cell may be farther than 1 move away).     
        
        Parameters:
        - cell (tuple): location (row,col) of initial cell
        - act (string): action taken to transition to next cell
        - nextCell (tuple): location resulting from the given action and cell
        
        Returns: 
        - cost (float): cost of the specified transition        
        """
        
        if act not in self.allowedActs:
            cost = float('inf');
        elif act in {'NW', 'NE', 'SW', 'SE'}:
            cost = 1.5; # with this cost function, manhattan is inadmissible! (i.e. 2 < 1.5)
        else:
            cost = 1;
            
        return cost;        


class MazeNode:
    """
    Search node to be expanded in maze traversal.
    
    Node tracks the path traversed so far through the maze, along with the corresponding actions and
    the associated cost.
    
    Member Data:
    - path (2-tuple list): list of cells in this node's search path
    - acts (string list): list of actions taken along search path
    
    Member Functions:
    - __init__(path, acts (optional), cost (optional)): construct a new maze node object
    - __repr__(): return string representation of the maze node
    """

    def __init__(self, path, acts=[], cost=0):
        """
        Create new maze search node, specifying the path and actions taken so far.
        
        Parameters:
        - path (2-tuple list): the node's current search path
        - acts (string list): list of actions taken along path (may be empty for start cell)
        - cost (float): cost of all actions taken along path
        """
                
        self.path = path;
        self.acts = acts; # empty acts indicate a new search
        self.cost = cost;
        
        
    def __repr__(self):
        """
        Give string representation of the maze node.
        """    
        
        nodeStr = '<' + str(self.path[0]); # first cell has no predecessor action
        for i in range(1,len(self.path)):
            nodeStr += '; ' + str(self.acts[i-1]) + '->' + str(self.path[i]);
        nodeStr += '|' + str(len(self.path)) + '>';
        return nodeStr;
        
                        
##################################################
# F U N C T I O N   D E F I N I T I O N S
##################################################
def manhattanDist(cell1, cell2):
    """
    Manhattan distance calculation suitable for use as a heuristic function.
    
    Note: for some cost functions, this may not be admissible! Admissibility requires that the 
    heuristic never overestimate the true cost, i.e. h(n) <= h*(n) for all n.
    """
    
    return abs(cell1[0]-cell2[0]) + abs(cell1[1]-cell2[1]);
    
def euclideanDist(cell1, cell2):
    """
    Euclidean distance calculation suitable for use as a heuristic function.
    """
    
    return ((cell1[0]-cell2[0])**2 + (cell1[1]-cell2[1])**2)**.5;

    
def pathKey(node):
    """
    Give sort key that prioritizes node with longer path, where min key is higher priority. 
    
    This is useful for tie breaking; we guess that all else being equal, the path that has traveled
    farther is more likely to be closer to the goal.
    """
    
    return -len(node.path);
    
        
def nodeKey(search, node, goal, heuristicFunc):
    """
    Give sort key that prioritizes node based on search, where min key is higher priority. 
    
    This prioritization is essentially the difference between the various search algorithms.
    """
    
    if search == 'A*':
        g = node.cost;
        h = heuristicFunc(node.path[-1], goal);
        key = g + h;    
    elif search == 'BFS':
        key = len(node.acts);
    elif search == 'DFS' or search == 'IDDFS':
        key = -len(node.acts);
    elif search == 'UCS':
        key = node.cost;
    else:
        raise ValueError('Invalid search type: ' + search);
        
    return key;  
    
    
def search(maze, searchType):
    """
    Perform a search for a path from start to finish of the given maze.
    
    Supported searches are A*, breadth-first search, depth-first search, and uniform cost search.
    There is no explicit priority for chosing which action to be taken first when expanding a node.
    
    Parameters:
    - maze (Maze): maze on which to perform search  
    - searchType (string): type of search to perform; valid types are 'A*', 'BFS', 'DFS', or 'UCS'
    
    Returns: 
    - goalNode (MazeNode): search node with solution, or None if maze is unsolvable  
    - expandedNodes (int): number of nodes expanded during search
    """
    
    # initialization
    visited = set(); # set of cells that have already been expanded to
    frontier = []; # priority min heap for selecting node to expand
    goalNode = None;
    expandedNodes = 0; # track number of nodes expanded
    
    # create new node at maze start and add to frontier
    startNode = MazeNode([maze.start]);
    
    # create a priority tuple that encodes how nodes ought to be sorted
    nKey = nodeKey(searchType, startNode, maze.finish, maze.heuristicFunc); 
    pKey = pathKey(startNode);
    priorityTuple = (nKey, pKey);   
    prioritizedStartNode = priorityTuple + (startNode,);
    heappush(frontier, prioritizedStartNode);  

    # while there are nodes on the frontier or being held, expand the highest priority
    while frontier:                  
        # select top node for expansion and extract last cell in path
        prioritizedNode = heappop(frontier); # heap contains tuples to prioritize node 
        expNode = prioritizedNode[-1]; # last element is actual maze node
        curCell = expNode.path[-1]; # new cells always pushed onto the path end        
        expandedNodes += 1;     
        
        # ensure that cells aren't revisited
        if curCell in visited:
            continue;
        visited.add(curCell);
        
        # check if node is goal        
        if maze.isFinish(curCell):
            goalNode = expNode;
            return (goalNode, expandedNodes);                        
        
        # expand node to get successor states resulting from all possible actions
        successors = maze.getSuccessors(curCell);      
        
        # add those successor cells that aren't yet visited to the frontier
        for act in successors:
            cell, transCost = successors[act];                                          
            if cell not in visited: 
                # create new maze node                
                newPath = list(expNode.path); # copy old lists and append successor
                newPath.append(cell);
                newActs = list(expNode.acts);
                newActs.append(act);
                newCost = expNode.cost + transCost;
                newNode = MazeNode(newPath, newActs, newCost);
                
                # if doing bfs/dfs, can check for goal as children are generated
                if searchType == 'BFS' or searchType == 'DFS':
                    if maze.isFinish(cell):
                        goalNode = newNode;
                        return (goalNode, expandedNodes);
                
                # create tuple representing a prioritized node for sorting              
                nKey = nodeKey(searchType, newNode, maze.finish, maze.heuristicFunc); 
                pKey = pathKey(newNode);
                priorityTuple = (nKey, pKey);   
                prioritizedNode = priorityTuple + (newNode,);  

                # add new node to frontier
                heappush(frontier, prioritizedNode);  
                
    # return solution; empty frontier with no goal found implies no solution
    return (goalNode, expandedNodes);            
    
    
def main():
    """
    Solve a given maze by various search algorithms and output the results.
    """

    # create maze object from command line specified file
    if len(sys.argv) != 2:
        print 'Usage: MazeSearch.py file_name';
        return;
    else:
        mazeFile = str(sys.argv[1]);
        actions = {'N', 'S', 'W', 'E','NE', 'NW', 'SW', 'SE'};
        heuristic = euclideanDist;
        maze = Maze(mazeFile, actions, heuristic);  
        
    # display maze info
    print '\nMAZE SEARCH\n';
    print maze;
    print 'Allowed actions within the maze: ' + str(maze.allowedActs);
    print 'Maze heuristic: ' + str(maze.heuristicFunc.__name__);      
    
    # try each search on the maze
    searches = ['A*', 'BFS', 'DFS', 'UCS'];
    for s in searches:          
        print '\nPerforming ' + s;
        start = time();
        solution, nodes = search(maze, s);
        duration = time()-start;
        print 'Search duration: %0.5f seconds' % duration;
        print 'Expanded nodes: ' + str(nodes); 
        if solution == None:
            print 'No solution exists!';
        else:
            if s == 'BFS' or s == 'DFS':
                print 'Number of actions: ' + str(len(solution.acts));
            else:
                print 'Cost of solution: ' + str(solution.cost);
            print 'Cells (%d): %s' % (len(solution.path), str(solution.path));
            print 'Actions (%d): %s' % (len(solution.acts), str(solution.acts));  
    
    
if __name__ == '__main__':
    main();









