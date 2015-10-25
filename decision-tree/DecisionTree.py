"""
This module defines and uses a decision tree to perform binary classification.

Input data may be of arbitrary dimension. Output labels are either 'positive' or 'negative'.
Tree structure is learned using the ID3 algorithm. Module defines a main method that can be invoked
to perform learning on the included training data and report results on the test data.

    Example:
    
        $ python DecisionTree.py
        
Testing for this module was performed by running the main method and comparing results against
results obtained by hand computation.
"""

from __future__ import division # force floating point division
from numpy import array, log2
import sys


class DecisionTree:
    """
    Implementation of a decision tree.
     
    Member Data:
    - root (DecisionNode): root node for the decision tree        
    
    Member Functions:
    - __init__(data, fVals): Construct a decision tree
    - __repr__(): Return a string representation of tree
    - queryTree(query): Predict label of query using decision tree
    - buildTree(data, untested, fVals): Recursively build decision tree
    """
    
    def __init__(self, data, fVals):
        """
        Construct a decision tree.             
        
        Parameters:
        - data (M+1-tuple list): N M-Dimensional data points. M+1th element is the label, 'positive' 
          or 'negative' 
        - fVals (tuple list): each tuple enumerates possible values for corresponding feature       
        """       
                
        # make set of testable features
        numFeature = len(data[0])-1;
        s = set(range(numFeature));
        
        # build tree
        self.root = self.buildTree(data, s, fVals);        
        
        
    def __repr__(self):
        """
        Return string representation of tree.
        """
        
        return repr(self.root);


    def queryTree(self, query):
        """
        Use decision tree to predict label for query.
        
        Parameters:
        - query (M-tuple): M-dimensional data point
        
        Returns:
        - label (string): predicted label, 'positive' or 'negative'
        """
        
        node = self.root;
        done = False;
        while not done:        
            if node.type == 'label': # predict value if node is a label
                label = node.value;
                done = True;
            else: # otherwise, move onto child node corresponding to feature value in query
                feature = node.value;
                fVal = query[feature];
                node = node.children[fVal]; 
                
        return label;
        

    def buildTree(self, data, untested, fVals): 
        """
        Recursively build decision tree using ID3 algorithm.
        
        Parameters:
        - data (M+1-tuple list): N M-Dimensional data points. M+1th element is the label, 'positive' 
          or 'negative' 
        - untested (set of ints): features not yet used to split on
        - fVals (tuple list): each tuple enumerates possible values for corresponding feature
        
        Returns
        -------
        node (DecisionNode): root node of decision tree
        """
        
        # BASE CASE 0: all examples have the same label (entropy is 0 iff labels are the same)
        setEntropy = calcSetEntropy(data);
        if setEntropy == 0:
            node = DecisionNode('label', data[0][-1], {});
            return node;
                
        # BASE CASE 1: already split on all features
        # find most common output label of set
        outcomes = [];
        for d in data:
            outcomes.append(d[-1]);  
        common = max(outcomes, key=outcomes.count);  
        
        # if no features left to split on, return leaf node with most common label
        if len(untested) == 0:
            node = DecisionNode('label', common, {});
            return node;   
    
        # RECURSIVE CASE: create children nodes by splitting on best feature    
        # select feature with highest info gain to split on
        maxGain = -1; # info gain has range [0,1]
        bestF = None;
        for f in untested: # iterate across all untested features
            expEntropyF = calcExpEntropy(data, f);
            gain = setEntropy - expEntropyF; # definition of info gain
            if gain > maxGain:
                maxGain = gain;
                bestF = f; 
                
        # remove feature from list of untested features
        untested.remove(bestF);
        
        # make child node for every possible feature value of selected feature  
        children = {};    
        for fVal in fVals[bestF]:
            # select all data points that match value for splitting feature
            subset = [];
            for point in data:
                if point[bestF] == fVal:
                    subset.append(point);
                    
            # handle empty subset by selecting most common label
            if not subset:
                child = DecisionNode('label', common, {});
            
            # otherwise recursively build child node
            else:
                child = self.buildTree(subset, set(untested), fVals); # copy untested feature set
            
            # add child node into dictionary of children
            children[fVal] = child;
            
        # create decision node
        node = DecisionNode('feature', bestF, children);
        
        return node;    
            
            
class DecisionNode:
    """
    Implementation of a decision node for use in decision tree.   
    
    For nodes of type 'label', the value is the name of the label. Otherwise, for nodes of type
    'feature', the value is the numeric feature (index into feature vector) of the feature being
    split on.     
     
    Member Data:
    - nodeType (string): 'feature' or 'label'
    - nodValue (string): the name of the feature/label
    - nodeChildren (DecisionNode dict): contains all children nodes, indexed by feature value
    
    Member Functions:
    - __init__(data, fVals): Construct a decision tree
    - __repr__(): Return a string representation of node and children
    - __str__(): Return string representation of node type and value
    """
    
    def __init__(self, nodeType, nodeValue, nodeChildren):
        """
        Construct decision node.           
        """
        
        self.type = nodeType;
        self.value = nodeValue;
        self.children = nodeChildren;
        
        
    def __repr__(self):
        """
        Return string representation of node and children.
        """
        
        nodeStr = '';
        parentStr = str(self);        
        if self.type == 'feature':         
            for child in self.children:                
                nodeStr += parentStr + '=' + child + '-->' + \
                           '[' + str(self.children[child]) + ']\n';
            for child in self.children:
                nodeStr += repr(self.children[child]);
              
        return nodeStr;
            
            
    def __str__(self):
        """
        Return string representation of node type and value.
        """
        
        nodeStr = 'Feature:' if self.type == 'feature' else 'Label:';
        nodeStr += str(self.value);
        
        return nodeStr;        


#################################################
# F U N C T I O N S
#################################################
def calcEntropy(p, n):
    """
    Calculate the entropy of probabilities.
    
    H(p) = -sum_i[ p_i*log_2(p_i) ]. Define 0 log 0 = 0.
        
    Parameters:
    - p (float): probability of a positive outcome
    - n (float): probability of a negative outcome
    
    Returns:
    - entropy (float): calculated entropy
    """

    h_p = 0 if p == 0 else -p * log2(p);
    h_n = 0 if n == 0 else -n * log2(n);       
    entropy = h_p + h_n;

    return entropy; 
    
    
def calcSetEntropy(data):
    """
    Calculate entropy of a set of outcomes.
    
    Parameters:
    - data (M+1-tuple list): N M-Dimensional data points. M+1th element is the label, 'positive' 
      or 'negative' 
        
    Returns:
    - entropy (float): calculated set entropy
    """
    
    p = 0;
    n = 0;
    for d in data:
        if d[-1] == 'positive':
            p += 1;
        elif d[-1] == 'negative':
            n += 1;
        else:
            raise ValueError('invalid outcome: %s' % str(d[-1]));
                        
    entropy = calcEntropy(p/len(data), n/len(data));
    
    return entropy;
            
    
def calcExpEntropy(data, feature):
    """
    Calculate the expected entropy from splitting on the specified feature. 
    
    Note: it's not guaranteed that we check all possible features--we only iterate over those 
    feature values encountered in the data. That's okay since a feature with 0 frequency would 
    contribute nothing anyway.        
        
    Parameters:
    - data (M+1-tuple list): N M-Dimensional data points. M+1th element is the label, 'positive' 
      or 'negative' 
    - feature (int): feature to split on
    
    Returns:
    - expEntropy (float): expected entropy from splitting on given feature        
    """
    
    # determine input data point dimension
    dim = len(data[0])-1;
    
    # validate feature index
    if feature < 0 or feature >= dim:
        raise ValueError('feature out of range: %s' % feature);
    
    # count occurences of features and outcomes
    count = {}; # map feature value to number of occurences
    pos = {}; # map feature value to number of positive outcomes 
    neg = {}; # map feature value to number of negative outcomes 
    for d in data:
        # enforce data point dimension consistency
        if len(d)-1 != dim:
            raise ValueError('inconsistent point dimension: %s' % str(d));
            
        # check feature value  
        fVal = d[feature];         
        if fVal in count: 
            count[fVal] += 1; # increment previously seen feature value
        else:
            count[fVal] = 1; # initialize newly seen feature value
            pos[fVal] = 0;
            neg[fVal] = 0;
    
        # increment appropriate outcome count
        if d[-1] == 'positive':
            pos[fVal] += 1;
        elif d[-1] == 'negative':
            neg[fVal] += 1;                
        else:    
            raise ValueError('invalid outcome: %s' % str(d[-1]));
                
    # compute expected value of entropy
    expEntropy = 0;
    for fVal in count.keys():        
        p = 0 if count[fVal] == 0 else pos[fVal]/count[fVal];
        n = 0 if count[fVal] == 0 else neg[fVal]/count[fVal];
        expEntropy += count[fVal]/len(data) * calcEntropy(p, n); 
        
    return expEntropy;
    
    
def calcInfoGain(data, feature):
    """
    Calculate the information gain from splitting on a given feature.
    
    Gain(S,F) = Entropy(S) - ExpectedEntropy(S,F).

    Parameters:
    - data (M+1-tuple list): N M-Dimensional data points. M+1th element is the label, 'positive' 
      or 'negative' 
    - feature(int): feature to split on
    
    Returns:
    gain (float): information gain
    """        
    
    gain = calcSetEntropy(data) - calcExpEntropy(data, feature);
    
    return gain;


def main():       
    """
    Read included training data, build decision tree, and report results on given test data.
    """
    
    # read in training and test data
    train = [];
    for i in range(1,7):
        name = 'data/tic-tac-toe-train-' + str(i) + '.txt';
        f = open(name, 'r');
        l = f.readline().strip();
        while l != '':        
            t = tuple(item for item in l.split(',') if item.strip());
            train.append(t);
            l = f.readline().strip();    
            
    test = [];
    name = 'data/tic-tac-toe-test.txt';
    f = open(name, 'r');
    l = f.readline().strip();
    while l != '':        
        t = tuple(item for item in l.split(',') if item.strip());
        test.append(t);
        l = f.readline().strip();
    print '%d train points read' % len(train);  
    print '%d test points read' % len(test);
        
    # create decision tree
    fVals = [['x','o','b']]*9; # given data allows features x, o, b
    tree = DecisionTree(train, fVals);     
    
    # make predictions on test data 
    print 'Predicting test set...'
    correct = 0;
    total = 0;
    for t in test:
        query = t[0:-1];
        actual = t[-1];
        predict = tree.queryTree(query);        
        total += 1;        
        if predict == actual:
            correct += 1;
    print 'Accuracy on test set: %d / %d = %f' % (correct,total,correct/total);
            

if __name__ == '__main__':
    main();
