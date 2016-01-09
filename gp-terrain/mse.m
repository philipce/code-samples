function [mse, errVals] = mse(trueVals, predictVals)
% Calculate mean squared error between two vectors.
%
% On input: 
%     trueVals (Nx1 double): vector of actual values
%     predictVals (Nx1 double): vector of observed values
%
% On output: 
%     mse (double): mean squared error
%     errVals (Nx1 double): element wise difference of input vectors, in
%       same order as original input vectors
%
% Authors: Phil Erickson
% Date: May 2015

% verify input
assert(size(trueVals,2) == 1, 'vector must be Nx1');
assert(isequal(size(trueVals), size(predictVals)), .... 
       'input vectors must be same size');

% calculate mean square error
errVals = predictVals-trueVals;
mse = sum(errVals.^2)/size(trueVals,1);