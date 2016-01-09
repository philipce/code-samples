function [A, Az, X, Xz] = sample(S, z, k, method)
% Select locations from a given point cloud to sample according to the
% specified sampling method.
%
% Note: Uniform sampling is actually quasi-uniform. For this sample option, 
%     data must be in rectangular interval, e.g. 2D data with all points in 
%     the region: x in [2,10] and y in [5,15]. 
%
% On input: 
%     S (NxD double): D-dimensional point cloud, containing possible
%       sample locations
%     z (Nx1 double): output values corresponding to input S 
%     k (integer): number of points to select
%     method (string): selection method
%
% On output: 
%     A (kxD double): sampled points from S
%     Az (kx1 double): outputs corresponding to sampled points from S
%     X (N-kxD double): unsampled points from S
%     Xz (N-kx1 double): outputs corresponding to unsampled points from S
%
% Authors: Phil Erickson
% Date: May 2015

% handle more desired samples than available points
if k >= size(S,1)    
    
    if k > size(S,1)
        fprintf('Warning in sample.m: sampling %d points from %d\n', ...
                k,size(S,1));
    end
    
    A = S;
    Az = z;
    X = [];
    Xz = [];
    return
end

switch method
    case 'uniform'
        % Not truly uniform; uses halton sequence to create set H of 
        % quasi-random samples. A KD tree is then used to select samples 
        % in S closest to the points in H.        
        h = generateHalton(k, min(S), max(S));

        % create KD tree and get unique nearest neighbors
        kdS = KDTreeSearcher(S);
        idx = knnsearch(kdS, h); 
        [uidx,ia,~] = unique(idx);
        dupes = setdiff(1:size(idx,1),ia); % duplicate elements of h
        notidx = setdiff(1:size(S,1),idx);
        
        % partition input set
        A = S(uidx,:);
        Az = z(uidx,:);
        X = S(notidx,:);
        Xz = z(notidx,:);
        
        % reassign points the matched an already used nearest neighbor
        if ~isempty(dupes)                
            for i=1:numel(dupes)
                kdX = KDTreeSearcher(X);
                idx = knnsearch(kdX, h(dupes(i),:)); 
                A = [A;X(idx,:)];
                Az = [Az;Xz(idx,:)];
                X = [X(1:idx-1,:) ; X(idx+1:end,:)]; % remove point from x            
                Xz = [Xz(1:idx-1,:) ; Xz(idx+1:end,:)];
            end     
        end
        
    case 'random'
        all = 1:size(S,1);
        selected = randperm(size(S,1), k);
        unselected = setdiff(all,selected);
        A = S(selected,:);
        Az = z(selected,:);
        X = S(unselected,:);        
        Xz = z(unselected,:);        

    otherwise
        error(strcat('invalid sampling method: ', method));
        
end



end