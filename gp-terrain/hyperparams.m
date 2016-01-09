function K_star = hyperparams(X, z, K, opt, startPts)
% Learns the hyper parameters for the given kernel. Given training data 
% and the type of kernel to use, function returns the set of 
% hyperparameters that maximizes the log marginal likelihood.
%
% On input: 
%     X (NxD double): N points of D-dimensional training inputs
%     z (Nx1 double): N training outputs corresponding to X
%     K (kernel.m): kernel data structure to use in inference 
%       Note: kernel fields must be initialized to indicate type of kernel
%       and initial guess at hyperparameters.
%
%     opt (string): optional - optimization method to be used
%     startPts (MxP double): optional - M P-dimensional starting points 
%       Note that the P values must be in the same order as the hyperparams 
%       are removed from the kernel data structure (see below)
%
% On output: 
%     K_star (kernel.m): kernel data structure containing learned values
%
% Custom files required: likelihood.m
% Nested functions: cost
%
% Authors: Phil Erickson
% Date: May 2015

% setup
if nargin < 4
    opt = 'direct';
end
D = size(X,2);

% extract the initial guesses for relevant hyperparameters
switch K.type
    case 'squared_exp'
        if K.constrain
            p0 = [K.l, K.sigma_f];
        else
            p0 = [K.l, K.sigma_f, K.sigma_n];
        end
        
    case 'rational_quad'
        if K.constrain
            p0 = [K.l, K.sigma_f, K.alpha];
        else
            p0 = [K.l, K.sigma_f, K.alpha, K.sigma_n];
        end
        
    case 'matern'
        if K.constrain
            p0 = [K.l, K.sigma_f];
        else
            p0 = [K.l, K.sigma_f, K.sigma_n];
        end
        
    case 'neural_net'
        if K.constrain
            p0 = [K.l, K.sigma_f, K.beta];
        else
            p0 = [K.l, K.sigma_f, K.beta, K.sigma_n];
        end
        
    otherwise
        error(strcat('unsupported kernel:', K.type));
end

% check the dimensionality of the start points
if ~isequal(size(p0,2), size(startPts,2))
    error('dimension mismatch, aborting optimize');
end

% perform specified type of optimization
switch opt
    case 'direct'
        options = optimset('MaxFunEvals', Inf, 'Display','none');
        [params, fval, exitflag, output] = fminsearch(@cost, p0, options);                
        
    case 'gradient'        
        options = optimoptions(@fminunc,'Algorithm','quasi-newton', ...
                               'Display','none');
        bestParams = p0;
        bestFval = Inf;
        for i = 1:size(startPts,1)
            fprintf('Searching from point %d of %d...\n',i, ...
                    size(startPts,1));
            p0 = startPts(i,:);
            [params, fval, exitflag, output] = fminunc(@cost, p0, options);   
            if fval < bestFval
                bestFval = fval;
                bestParams = params;
            end
        end
        params = bestParams;
        
    otherwise
        error(strcat('unknown optimizer option:', opt));
end

% put learned parameters into kernel structure
switch K.type
    case 'squared_exp'
        K.l = params(1:D);                
        K.sigma_f = params(D+1);
        
    case 'rational_quad'
        K.l = params(1:D);                
        K.sigma_f = params(D+1);
        K.alpha = params(D+2);
        
    case 'matern'
        K.l = params(1:D);                
        K.sigma_f = params(D+1);
        
    case 'neural_net'
        K.l = params(1:D);                
        K.sigma_f = params(D+1);
        K.beta = params(D+2);
        
    otherwise
        error(strcat('unsupported kernel:', K.type));
end

K_star = K;


    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % likelihood function wrapper
    function c = cost(p)
    % insert parameter vector into appropriate kernel structure
    switch K.type
        case 'squared_exp'
            K.l = p(1:D);                
            K.sigma_f = p(D+1);
            if ~K.constrain
               K.sigma_n = p(D+2); 
            end
            
        case 'rational_quad'
            K.l = p(1:D);                
            K.sigma_f = p(D+1);
            K.alpha = p(D+2);
            if ~K.constrain
               K.sigma_n = p(D+3); 
            end
            
        case 'matern'
            K.l = p(1:D);                
            K.sigma_f = p(D+1);
            if ~K.constrain
               K.sigma_n = p(D+2); 
            end
            
        case 'neural_net'
            K.l = p(1:D);                
            K.sigma_f = p(D+1);
            K.beta = p(D+2);
            if ~K.constrain
               K.sigma_n = p(D+3); 
            end
            
        otherwise
            error(strcat('unsupported kernel:', K.type));
    end

    c = -1 * likelihood(X,z,K);
    
    end
    %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


end
