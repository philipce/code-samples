function [z_star, v_star] = infer(X, z, K, X_star)
% Perform GP based inference. Given a set of training data and kernel 
% function, infer values desired test points.
%
% Note: a k-D tree is used to select only some of the nearest neighbors 
% from among the training data in order to speed up inference, so K.knn
% must be defined.
%
% On input: 
%     X (NxD double): N points of D-dimensional training inputs
%     z (Nx1 double): N training outputs corresponding to X
%     K (kernel.m): kernel data structure to use in inference 
%     X_star (MxD double): M test points at which to perform inference
%
% On output: 
%     z_star (Mx1 double): mean values at test points
%     v_star (Mx1 double): variance at test points
%
% Custom files required: kernel.m
%
% Authors: Phil Erickson
% Date: May 2015

% initialize return values
z_star = zeros(size(X_star,1), 1);
v_star = zeros(size(X_star,1), 1);

% create KD tree of training points
kdX = KDTreeSearcher(X);

% iterate through each test point in X_star
for i = 1:size(X_star,1)
    
    % select current test point
    x_star = X_star(i,:);
    
    % select training points to use for inference       
    nIdx = knnsearch(kdX, x_star, 'K', K.knn);          
    X_cur = X(nIdx,:);
    z_cur = z(nIdx);
    
    % create covariance matrix and vector of selected training data
    K_m = zeros(size(X_cur,1));    
    k_star = zeros(size(X_cur,1), 1);   
    for r = 1:size(K_m,1)
        u = X_cur(r,:);
        k_star(r) = kernel(x_star, u, K);
        for c = 1:size(K_m,2)
            v = X_cur(c,:);
            K_m(r,c) = kernel(u,v,K);
        end        
    end
    
    % add noise to covariance matrix and invert
    [U, err] = chol(K_m + K.sigma_n^2*eye(size(K_m)));
    if err ~= 0
        fprintf('Warning in infer.m: matrix is not pos semi definite');
    end
    U_inv = inv(U);
    K_mn_inv = U_inv * U_inv';
        
    % compute inferred mean
    z_star(i) = k_star' * K_mn_inv * z_cur;
    
    % compute inferred variance
    v_star(i) = kernel(x_star,x_star,K) - k_star'*K_mn_inv*k_star;
    
end



end