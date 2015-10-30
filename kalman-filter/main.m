% This script demonstrates the use of the kalman filter in projectile
% tracking. 
%
% A simple model for a 2D projectile is obtained and used for the kalman 
% filter transition model. The actual trajectory of the projectile is
% simulated using a more complex and accurate model. Observations of the 
% actual projectile trajectory are obtained. The kalman filter is used to 
% generate beliefs on the actual projectile state. All three trajectories 
% (the simple model, the complex model, and the kalman belief) are plotted,
% along with the observations made.

% user defined parameters
dt = .1; % simulation time step in seconds
t_max = 35; % max simulation time in seconds
obs_freq = .35; % frequency with which time steps are observed, range [0,1]
g = 9.8; % gravitational acceleration in m/s^2
r = 8; % transition uncertainty; increase for fast response to obs
q = 2000; % observation noise; increase for greater observation spread 

% get system dynamics model for idealized 2D projectile
sys = projectile_model(dt, g, r, q);

% get projectile trajectory (sample off actual, use ideal for comparison)
[actual_states, ideal_states] = projectile_trajectory(dt, t_max, g); 

% find where projectile hits ground, if it does at all
plot_end = find(actual_states(2,:) < 0, 1);
if isempty(plot_end)
    plot_end = length(actual_states);
end

% take noisy, observations of actual (x,y) location at some time steps
num_obs = floor(length(actual_states) * obs_freq);
obs_steps = observe_trajectory(actual_states, sys, num_obs);
non_nan_indices = find(~isnan(obs_steps(1,:))); % indices of non NaN obs
obs_values = obs_steps(:, non_nan_indices); 

% use kalman filter algorithm to filter noisy observations
belief_states = zeros(size(actual_states));
prev_mu = zeros(size(actual_states,1), 1); % initial guess
prev_sigma = eye(size(actual_states,1));
for i = 1:length(belief_states)      
    % get current observation
    z = obs_steps(:, i);
    unobserved = ~isempty(find(isnan(z),1)); % NaN means step not observed
    if unobserved
        z = []; % z should be empty for KF function if no observation
    end
    
    % update belief state using kalman filter
    [mu, sigma] = kalman_filter(prev_mu, prev_sigma, sys, z);
    belief_states(:, i) = mu;

    % move on to next time step
    prev_mu = mu;
    prev_sigma = sigma;    
end

% plot actual/ideal trajectory, noisy observations, and belief
plot(actual_states(1, 1:plot_end), actual_states(2, 1:plot_end), '-g', ...
     ideal_states(1, 1:plot_end), ideal_states(2, 1:plot_end), '-.k', ...
     belief_states(1, 1:plot_end), belief_states(2, 1:plot_end), '-b', ...
     obs_values(1,:), obs_values(2,:), '.r');
legend('Actual trajectory', 'Ideal model trajectory', 'Belief', ...
       'Observations', 'Location', 'northwest');
xlabel('X Position (m)');
ylabel('Y Position (m)');
x_axis_lim = max(actual_states(1, 1:plot_end))*2;
y_axis_lim = max(actual_states(2, 1:plot_end))*2;
axis([0, x_axis_lim, 0, y_axis_lim]);




