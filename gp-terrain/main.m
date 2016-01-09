% This script demonstrates the use of Gaussian process methods for 
% regression and shows how the various functions can be used. This example 
% is somewhat simplified for the purpose of demonstration.
%
% Authors: Philip Erickson
% Date: May 2015

clear all;
close all;
clc;

% user defined parameters
sampleMethod = 'random'; % method for sampling complete dataset
optMethod = 'gradient'; % optimization method for learning
testNum = 100; % number of points in test set
trainNum = 250; % number of points in training set
hpLearnNum = 15; % number of samples used to learn hyperparameters
startPts = 3; % number of points to begin search to learn hp at 
pointsPerEdge = 20; % number of inference points in model surface plot
neighbors = 5; % number of nearest neighbors to use in inference

% read in terrain point cloud and display dataset
fprintf('Reading in dataset...');
terrain = csvread('NGAT.PointCloud.csv');
fprintf(' done.\n');

figure()
plot3(terrain(:,1),terrain(:,2),terrain(:,3),'.k');
title('Dataset: Actual Values');

% select test, training, and hyperparameter learning sets
fprintf('\nSampling dataset for test and train data...');
[testIn, testOut, X, Xz] = sample(terrain(:,1:2), terrain(:,3), ...
                                  testNum, sampleMethod);                                
[trainIn, trainOut, ~, ~] = sample(X, Xz, trainNum, sampleMethod);
[learnIn, learnOut, ~, ~] = sample(trainIn, trainOut, hpLearnNum, ...
                                   sampleMethod);
fprintf(' done.\n');

% create kernel structure with initial values 
K.type = 'matern';
K.constrain = false;
K.knn = neighbors;
K.nu = 3/2;
K.l = [1, 1];
K.sigma_f = 1;
K.sigma_n = 1;

% generate search initial points, quasi uniform across a priori region
start = generateHalton(startPts, [-50, -50, 0, 0],[50, 50, 500, 100]);

% learn hyperparameters
fprintf('\nLearning hyperparameters off %d points...\n', hpLearnNum);
tic;
K = hyperparams(learnIn, learnOut, K, optMethod, start);
fprintf('Finished learning hyperparameters in %.2f seconds.\n',toc);

% used learned hyperparameters and training data to perform inference
fprintf('\nInferring %d test points using %d training points...\n', ...
        size(testIn,1), size(trainIn,1));
tic;
[inferValues, ~] = infer(trainIn, trainOut, K, testIn);
fprintf('Finished inference in %.2f seconds\n',toc);
testError = mse(testOut, inferValues);
fprintf('MSE predicting test points: %.5f (RMS %.5f)\n', testError, ...
        sqrt(testError));
    
% plot surface representing resulting GP model
fprintf('\nComputing model surfaces...\n');
xLims = [min(terrain(:,1)), max(terrain(:,1))];
yLims = [min(terrain(:,2)), max(terrain(:,2))];
[plotXG, plotYG] = meshgrid(linspace(xLims(1),xLims(2),pointsPerEdge), ...
                            linspace(yLims(1),yLims(2),pointsPerEdge));
plotPts = [plotXG(:), plotYG(:)];
tic;
[inferValues, uncertainty] = infer(trainIn, trainOut, K, plotPts);
fprintf('Computed model in %.2f seconds\n',toc);

figure('position',[20,500,900,400]);
subplot(1,2,1);
tri = delaunay(plotPts(:,1),plotPts(:,2));
trisurf(tri,plotPts(:,1), plotPts(:,2), inferValues,'edgecolor','none');
camlight left;
lighting gouraud;
title('GP Model: Inferred Values');

subplot(1,2,2);
tri = delaunay(plotPts(:,1),plotPts(:,2));
trisurf(tri,plotPts(:,1), plotPts(:,2), uncertainty,'edgecolor','none');
camlight left;
lighting gouraud;
title('GP Model: Uncertainty');



