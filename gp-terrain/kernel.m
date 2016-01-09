function c = kernel(u, v, K)
% Implementation of different types of kernel functions. Given a kernel
% data structure and two vectors, compute the covariance.
%
% On input: 
%     u (1xD double): first D-dimensional vector
%     v (1xD double): second D-dimensional vector
%     K: kernel data structure consisting of (as relevant):
%       .type (string): kernel type to use, e.g. 'neural_net' 
%       .constrain (boolean): false allows sigma_n to be optimized
%       .l (1xD double): lengthscales corresponding to vector dimensions
%       .sigma_f (double): sigma_f^2 is signal variance
%       .beta (double): bias factor
%       .alpha (double): 
%       .sigma_n (double): sigma_n^2 is noise variance
%       .nu (double): Matern class parameter
%       .knn (int): number of nearest neighbors to consider
%
% On output: 
%     c (double): covariance of u and v
%
% Authors: Philip Erickson
% Date: May 2015

switch K.type
    case 'squared_exp'
        Sigma = diag(K.l)^2;       
        d = (u-v)/Sigma*(u-v)'; % d is r^2/l^2 in some notation
        c = K.sigma_f^2 * exp(-d/2);                 
    
    case 'rational_quad'
        Sigma = diag(K.l)^2;
        d = (u-v)/Sigma*(u-v)';
        c = K.sigma_f^2 * (1 + d/(2*K.alpha))^(-K.alpha);
    
    case 'matern'
        switch K.nu
            case 3/2
                Sigma = diag(K.l)^2;
                d = (u-v)/Sigma*(u-v)';
                t1 = sqrt(3) * sqrt(d); 
                c = K.sigma_f^2 * (1+t1) * exp(-t1);
            
            case 5/2
                Sigma = diag(K.l)^2;
                d = (u-v)/Sigma*(u-v)';
                t1 = sqrt(5) * sqrt(d); 
                c = K.sigma_f^2 * (1+t1+5*d/3) * exp(-t1);
            
            otherwise
                error(strcat('unsupported parameter nu:', num2str(K.nu)));
        end   
        
    case 'neural_net'
        Sigma = inv(diag(K.l)^2);
        t1 = K.beta + 2*u*Sigma*v';
        t2 = 1 + K.beta + 2*u*Sigma*u';
        t3 = 1 + K.beta + 2*v*Sigma*v';
        t4 = asin(t1/sqrt(t2*t3));
        if ~isreal(t4)
            fprintf('Warning in kernel.m: result of asin is complex\n');
            t4 = real(t4);
        end
        c = K.sigma_f^2 * t4;     
        
    otherwise
        error(strcat('unsupported kernel:', K.type));
end



end