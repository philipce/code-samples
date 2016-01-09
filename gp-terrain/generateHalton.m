function h = generateHalton(k,lb,ub)
% Generate a set of Halton points across an arbitrary dimensional space.
%
% On input: 
%     k (integer): number of points to generate
%     lb (1xD double): vector of lower bounds for each dimension
%     ub (1xD double): vector of upper bounds for each dimension
%
% On output: 
%     S (kxD double): k D-dimensional points that evenly cover the space
%
% Authors: Phil Erickson
% Date: May 2015

% data set dimension
D = size(lb,2);

% create halton set and sample
H = haltonset(D,'Skip',1e3,'Leap',1e2);
H = scramble(H,'RR2');
h = net(H, k);

% scale each dimension to the given bounds
for d = 1:D
    dMin = lb(d);
    dMax = ub(d);
    h(:,d) = (dMax-dMin) .* h(:,d) + dMin;
end