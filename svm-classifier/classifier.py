#!/usr/bin/python

"""
This modules defines and uses a support vector machine to perform binary classification. Input data 
may be of arbitrary dimension. Output labels are either +1 or -1. This module also contains code to 
perform k-fold cross validation across multiple cores.

An example of the classifier can be run, by providing data dimension and paths to files containing 
test and training sets. A fifth argument can be provided to add a constant bias to the input data.
See the function readFile for details on determining the data dimension and adding a bias.

    Example:
    
        $ ./classifier.py 11 ./data0/train ./data0/test
        
            OR
            
        $ ./classifier.py 5 ./astro/scaled/train ./astro/scaled/test 1
"""

from __future__ import division # force floating point division
import enum
import itertools
import multiprocessing as mp
import numpy as np
import operator
import random
import sys


def main():
    """
    Provides an example of how to use the classifier functions to perform inference on data.
    """

    if len(sys.argv) < 4 or len(sys.argv) > 5:
        print 'Usage: classifier.py data_dimension train_set_path test_set_path [option: add_bias]'; 
        return;       

    # create sets of possible hyperparameter values
    setC = {0.001, 0.01, 0.1, 1, 10, 25, 100}; # trade off regularizer and error minimization
    setRho = {0.001, 0.01, 0.1, 1}; # learning rate for gradient descent   
    hyperparams = [setC, setRho];
    
    # create svm classifier for selected data
    dataDim = int(sys.argv[1]);
    trainPath = str(sys.argv[2]);
    testPath = str(sys.argv[3]);
    if len(sys.argv) == 5:
        c = Classifier('svm', hyperparams, dataDim, testPath, trainPath, addBias=True);
    else:
        c = Classifier('svm', hyperparams, dataDim, testPath, trainPath);
    
    print 'Classifier type: ', c.type, \
          '\nTraining set: ', trainPath, \
          '\nTest set: ', testPath;
           
    print 'Determining hyperparameters to use...';
    c.learnHyperparams(report=1);
    
    print 'Training classifier...';
    c.train();
    
    print 'Performing inference on test set...';
    c.test(); 
    
    print '\nREPORT:', \
          '\nUsing hyperparameters: ', c.theta, \
          '\nLearned weight vector: ', c.w, \
          '\nPrediction accuracy on test set: ', c.accuracy * 100, ' percent';


class Classifier:
    """
    Implementation of a classifier. Currently supports only SVM binary classifier, but room is left 
    for easy extension to other methods. 
    
    Member Data:
    - ClassifierType (enum): enum definition for identifying classifier method
    - type (ClassifierType): learning method for classifier instance
    - hpRanges (list of sets): list contains a set for each hyperparameter, where each set contains 
      the possible values that a hyperparameter may take
    - k (int): number of folds for cross validation
    - cvEpochs (int): number of epochs to run for in cross validation
    - epochs (int): number of epochs to run for in inference
    - D (int): data dimension
    - trainSet (list of tuples): list of tuples representing a data point and corresponding label; 
      each tuple contains an ndarray at index 0 and an integer label (+/-1) at index 1
    - testSet (list of tuples): list of data to test, structured like train set
    
    Member Functions:
    - __init__
    - learnHyperparams
    - train
    - test
    - predict
    - readFile 
    """

    # enum allowing for different classifier types -- currently only svm is supported
    ClassifierType = enum.Enum('ClassifierType', 'SVM PERCEPTRON KNN DECISIONTREE');
    
    
    def __init__(self, classifierType, hyperparams, dimension, trainPath, testPath, addBias=False):
        """
        Construct new classifier object.
        
        Parameters:
        - classifierType (ClassifierType): type of classifier to create
        - hyperparams (list of sets): list contains a set of possible values for each hyperparameter
        - dimension (int): data dimension
        - trainPath (string): file path to train set
        - testPath (string): file path to test set
        """
    
        # set classifier type
        if classifierType.upper() == 'SVM':
            self.type = self.ClassifierType.SVM;
        else:
            raise ValueError('Unknown classifier type: ' + classifierType); 
            
        # store value ranges for classifier hyperparameters
        self.hpRanges = hyperparams;
        
        # set default learning parameters
        self.k = 5; # k-fold cross validation
        self.cvEpochs = 10; # training epochs for cross validation
        self.epochs = 20; # training epochs for inference
        
        # read in train and test data
        self.D = dimension;
        self.trainSet = self.readFile(trainPath, dimension, addBias);
        self.testSet = self.readFile(testPath, dimension, addBias);             
         
            
    def learnHyperparams(self, report=None):
        """
        Learn hyperparameters on the given data set, printing results at the indicated level.
        
        Parameters:
        - report (int): number of tabs to indent printed report, or don't print if none
        """
    
        self.theta = crossValidate(self.k, self.cvEpochs, self.hpRanges, self.trainSet, \
                                   self.train, self.test, report);
    
    
    def train(self, trainSet=None, epochs=None, theta=None):
        """ 
        Call the appropriate training function for classifier type on given data. If given data is
        none, training will be done on the entire training set and the results stored, otherwise 
        results will be returned but not saved (useful for cross validation).
        
        Parameters:
        - trainSet (list of tuples): data for training
        - epochs (int): number of epochs to train for
        - theta (float list): values for hyperparameters
        
        Returns:
        - w (ndarray): weight vector
        """
        
        setW = False;
        if trainSet is None:
            trainSet = self.trainSet;
            epochs = self.epochs;
            theta = self.theta;
            setW = True;
        
        if self.type == self.ClassifierType.SVM:
            C = theta[0];
            rho = theta[1];
            w = trainSVM(trainSet, epochs, C, rho);            
        else:
            raise NotImplementedError("Only SVM is currently supported");
            
        if setW:
            self.w = w;
            
        return w;


    def test(self, w=None, testSet=None):
        """ 
        Call the appropriate testing function for classifier type on given data. If given data is
        none, testing will be done on the entire test set and the results stored, otherwise 
        results will be returned but not saved (useful for cross validation).
        
        Parameters:
        - w (ndarray): weight vector
        - testSet (list of tuples): data for testing
        
        Returns:
        Tuple consisting of... 
        - correct (int): number of correct predictions
        - attempts (int): number of predictions made
        """
        
        setAccuracy = False;
        if w is None:
            w = self.w;
            testSet = self.testSet;
            setAccuracy = True;
            
        if self.type == self.ClassifierType.SVM:            
            (correct, attempts) = testSVM(w, testSet);             
        else:
            raise NotImplementedError("Only SVM is currently supported");
            
        if setAccuracy:
            self.accuracy = correct/attempts;
            
        return (correct, attempts);
    
    
    def predict(self, w, x):
        """
        Predict lable of input data point.
        
        Parameters:
        - w (ndarray): weight vector
        - x (ndarray): input data point

        Returns:
        - label (int): predicted label for input data point, either +/- 1
        """    

        if self.type == self.ClassifierType.SVM:                        
            return predictSVM(w, x);    
        else:
            raise NotImplementedError("Only SVM is currently supported");      
        
        
    def readFile(self, fileName, d, addBias=False):
        """
        Reads values from a specified file in libSVM format into a input vector and output value.
        
        For example, a 3D feature vector with a leading bias term and a positive label could be
                +1 0:1 1:0.575 2:0.705 3:0.995                        
                                
        The bias term need not be given explicitly, in which case the addBias parameter ought to be 
        set to true, to add a 1 as the final element of the data point.
        
        Note also that elements not explicitly given a value (e.g. above element 3 is .995) are set
        to zero, hence the parameter d must be used to specify the data dimension (d should indicate
        the dimensionality of the data points in the file; if a bias is explicit in the file then it 
        should be included in d).
        
        NB: It is assumed that data vectors are zero indexed! Hence the point:
                +1 1:0.575 2:0.705 3:0.995
        would be considered 4-dimensional, with an implied value of 0 at index 0.
        
        Labels must be either +1 or -1. Files containing 1 or 0 will have labels converted.

        Parameters:
        - fileName (string): path to file to read
        - d (int): data point dimension, i.e. number of features (including explicit bias)
        - addBias (boolean): if true, add a final element to the data point of constant 1 bias (in
          this case, dimensionality d will be increased by 1 to accomodate a new bias term)
          
        Returns:
        - dataSet (list of tuples): list of tuples representing a data point and corresponding 
          label; each tuple contains an ndarray at index 0 and an integer label (+/-1) at index 1
        """

        dataSet = [];

        # open file and read lines from it, where each line contains a data point and label
        f = open(fileName, 'r');
        for line in f:
            # split line into list of strings, each string representing an element of the data point
            dataPt = (line.strip()).split(); 
            
            # extract label for current data point
            label = int(dataPt[0]); 
            if label == 0:
                label = -1;   
            
            # create ndarray for data point with bias
            if addBias:
                fVector = np.zeros(d+1);
                fVector[-1] = 1;
            else:
                fVector = np.zeros(d);
            for i in range(1,len(dataPt)):                               
                fIndex, fVal =  dataPt[i].split(':');
                fVector[int(fIndex)] = float(fVal);
                
            # add data point and label to data set
            dataSet.append((fVector,label));
                
        return dataSet;


###########################################################################
# C R O S S   V A L I D A T O R
###########################################################################
def crossValidate(k, epochs, hyperparams, data, trainFunc, testFunc, report=None):
    """
    Perform k-fold cross validation to determine hyperparameters on a given data set.
    
    This function kicks off worker threads to concurrently compute prediction accuracy for each 
    combination of hyper parameters. A Queue is created for each worker to post results to, where
    each worker posts a tuple containing (theta, rate). Worker thread function is defined in 
    cvWorker(). After starting all workers, function waits for all to finish before pulling results
    out of the queue.

    Parameters:
    - k (int): number of folds
    - epochs (int): number of epochs to run for      
    - hyperparams (list of float sets): each entry in the list is a set of possible values for the
      corresponding hyperparameter, e.g. hyperparams[0] = {.5, 1, 5, 10}
    - data (list of tuples): list of tuples representing a data point and corresponding 
      label; each tuple contains an ndarray at index 0 and an integer label (+/-1) at index 1
    - trainFunc (function): pointer to a training function that takes a data set and parameters and
      returns a method for classification (e.g. epochs and hyperparameters and returns weights)
    - testFunc (function): pointer to a test function that takes dataset and parameters (e.g. data 
      and weights)and returns number of correct predictions and total number of predictions  
    - report (int): if not None, print status update reports at given indent level
      
    Returns:
    - bestTheta (tuple): best hyperparameter values found in testing
    """
    
    if not (report == None):
        tabs = '\t' * report;
        print tabs, 'Performing %d-fold cross validation...' % k;
    
    # create vars to save the best hyperparameters and their performance
    bestTheta = None;
    bestRate = float("-inf");
    
    # create queue for worker threads to post results to
    queue = mp.Queue();
    
    # create train/test folds
    numPerFold = len(data) // k;
    numLeftOver = len(data) % k;
    folds = [data[i*numPerFold:i*numPerFold+numPerFold] for i in range(0, k)];
    if numLeftOver > 0:
        folds[-1].extend(data[-numLeftOver:]);   
        
    # create a list of tuples; each tuple defining a unique assignment of hyperparameters   
    thetas = list(itertools.product(*hyperparams));
    
    # create worker threads try all combinations of hyperparameters    
    workers = [];    
    for theta in thetas:             
        p = mp.Process(target=cvWorker, args=(epochs, theta, folds, trainFunc, \
                                              testFunc, report, queue));
        workers.append(p)
            
    # start worker threads and wait for them to finish
    for p in workers:
        p.start();
    for p in workers:
        p.join()
        
    if not (report == None):
        print tabs, 'All worker threads have terminated.';
        
    # read results out of queue  
    while not queue.empty():
        [theta, rate] = queue.get();
        if rate > bestRate:
            bestTheta = theta
            bestRate = rate;
                
    return bestTheta;


def cvWorker(epochs, theta, folds, trainFunc, testFunc, report, queue):
    """
    Perform cross validation on the given folds using the specified hyper parameters.
    
    This function is a worker thread function, used by crossValidate() to perform the computations
    concurrently. 
    
    NB: Worker function should not modify folds as it is shared with all workers.

    Parameters:
    - epochs (int): number of epochs to run for
    - theta (tuple): values for each hyperparameter
    - folds (list of lists): each inner list contains tuples and represents particular fold; each 
      tuple in the fold contains an ndarray at index 0 and an integer label (+/-1) at index 1
    - trainFunc (function): pointer to a training function that takes a data set and parameters and
      returns a method for classification (e.g. epochs and hyperparameters and returns weights)
    - testFunc (function): pointer to a test function that takes dataset and parameters (e.g. data 
      and weights)and returns number of correct predictions and total number of predictions  
    - report (int): if not None, print status update reports at given indent level
    - queue (Queue): multiprocessing Queue to post results to
      
    Returns:
    - no value is returned by worker, but (theta, rate) tuple is posted to the given queue for 
      processing by the crossValidate() function.
    """

    # track how many correct predictions are made over all folds with current hyperparams
    totalCorrect = 0;
    totalAttempts = 0;
    for (i,f) in enumerate(folds):                     
        
        testFold = f;
        trainFold = reduce(operator.add, folds[:i] + folds[i+1:]); # flatten training fold             
        
        # learn weights           
        w = trainFunc(trainFold, epochs, theta);
  
        # accumulate test accuracy
        [correct, attempts] = testFunc(w, testFold);
        totalCorrect += correct;
        totalAttempts += attempts;                   
        
    # update based on results and post to queue
    rate = totalCorrect / totalAttempts;
    if not (report == None):
        tabs = '\t' * report;
        print tabs, 'Cross validation accuracy=', rate, 'for theta=', theta;
    results = (theta, rate);          
    queue.put(results)
    
    return;        
        
        
###########################################################################
# S V M
###########################################################################      
def trainSVM(dataSet, epochs, C, rho):
    """
    Perform stochastic gradient descent to learn SVM for given data.

    Parameters:
    - dataSet (list of tuples): list of tuples representing a data point and corresponding 
      label; each tuple contains an ndarray at index 0 and an integer label (+/-1) at index 1
    - epochs (int): number of epochs to run  
    - C (float): hyperparameter to tradeoff regularizer and empirical error
    - rho (float): hyperparameter for initial learning rate
      
    Returns:
    - w (ndarray): weight vector
    """
    
    D = len(dataSet[0][0]);
    w = np.zeros(D);
    t = 0;
    
    # run for some epochs, over every training point in random order on each epoch   
    for epoch in range(epochs):    

        random.shuffle(dataSet);                
        for [x, y] in dataSet:
        
            # compute learning rate for this itr
            r = rho/(1 + rho*t/C); 
                        
            # compute gradient on single example
            if y*np.dot(w, x) <= 1:
                grad = w - C*y*x;
            else:
                grad = w;
                
            # update weight vector
            w = w - r*grad;
            
            t = t + 1;
            
    return w;            


def predictSVM(w, x):
    """
    Use SVM to predict lable of input data point.
    
    Parameters:
    - w (ndarray): weight vector
    - x (ndarray): input data point

    Returns:
    - label (int): predicted label for input data point, either +/- 1
    """
    
    # compute activation for test example and threshold the result
    a = np.dot(w, x);
    label = 1 if a > 0 else -1;
    
    return label;
    
    
def testSVM(w, dataSet):
    """
    Use SVM to predict labels on given dataSet.
    
    Parameters:
    - w (ndarray): weight vector
    - dataSet (list of tuples): list of tuples representing a data point and corresponding 
      label; each tuple contains an ndarray at index 0 and an integer label (+/-1) at index 1
      
    Returns:
    Tuple consisting of
    - correct (int): number of correct predictions out of test set  
    - attempts (int): total number of predictions made
    """
    
    correct = 0;
    attempts = 0;
    for [x, y] in dataSet:
        attempts += 1;
        if y == predictSVM(w, x):
            correct += 1;

    return (correct, attempts);   
    
    



if __name__ == '__main__':
    main();   

