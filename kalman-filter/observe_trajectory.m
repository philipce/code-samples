function obs = observe_trajectory(trajectory, model, n)
% Given an actual trajectory and a model for the system, this function
% takes n observations of the trajectory at random (uniform) time steps,
% according to the observation model.
%
% The returned matrix of observations contains the same number of columns
% as the given trajectory, each column in observations corresponding to a
% time step in the trajectory. Exactly n of these columns will contain
% state observations. The remaining columns contain NaN, indicating that 
% the corresponding time step was unobserved. This allows determining when
% (what time step) a particular observation was made.
%
% Linear Gaussian measurement: z_t = C_t * x_t + noise
%
% On input:
%   trajectory (kxm matrix): trajectory consisting of k state variables at
%       m time steps
%   model (struct): structure representing the system, consisting of:
%       .A (nxn matrix): state transition model
%       .B (nxm matrix): control-input model
%       .C (kxn matrix): observation model
%       .R (nxn matrix): covariance matrix for transition uncertainty 
%       .Q (kxk matrix): covariance matrix for observation noise
%       .u (mx1 vector): control vector
%   n (int): number of time steps to observe
%
% On output: 
%   obs (kxm matrix): matrix wherein columns represent observed state, and 
%       NaN indicates unobserved time steps

% select time steps to observe
indices = sort(randperm(length(trajectory), n));

% select observable (non hidden) states in trajectory 
obs_traj = model.C * trajectory; 

% take observations on observable trajectory at selected time steps 
obs = NaN(size(obs_traj));
obs(:, indices) = obs_traj(:, indices);

% create vector of noise to add to observations
mu_zeros = zeros(1,size(model.C,1));
obs_noise = mvnrnd(mu_zeros, model.Q, n)'; 

% add noise to observations
obs(:, indices) =  obs(:, indices) + obs_noise;
