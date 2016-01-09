function l = likelihood(X, z, K)
% Compute the log marginal likelihood of given training outputs conditioned
% on the training inputs and covariance function and hyperparameters, according to:
%   log(z|X,theta) = -1/2*z'*K_z^-1*z - 1/2*log|K_z| - n/2*log(2*pi)
%
% On input: 
%     X (NxD double): N points of D-dimensional training inputs
%     z (Nx1 double): N training outputs corresponding to X
%     K (kernel.m): kernel data structure to use in likelihood evaluation
%
% On output: 
%     l (double): likelihood based on given information
%
% Authors: Phil Erickson
% Date: May 2015

% create covariance matrix of selected training data
K_z = zeros(size(X,1));     
for r = 1:size(K_z,1)
    u = X(r,:);
    for c = 1:size(K_z,2)
        v = X(c,:);
        K_z(r,c) = kernel(u,v,K);
    end        
end

% add noise to matrix
K_z = K_z + K.sigma_n^2*eye(size(K_z));

% invert matrix
[U, err] = chol(K_z);
if err ~= 0 % valid kernel should never have error
    fprintf('Warning in likelihood.m: matrix not PSD');
    disp(K);
    l = -inf; % make this set of observed data impossible
    return;   
end
U_inv = inv(U);
K_inv = U_inv * U_inv';

% calculate determinant
K_det = prod(diag(U))^2;

% calculate likelihood
l = -.5*z'*K_inv*z - .5*log(K_det) - .5*size(X,1)*log(2*pi); 



end
