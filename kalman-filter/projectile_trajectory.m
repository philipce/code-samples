function [actual, ideal] = projectile_trajectory(dt, t_max, g)
% Provides the trajectory of the projectile at all time steps from 0 to 
% the given max time. 
%
% This function provides two trajectories: actual and ideal. The actual 
% accounts for friction, whereas ideal neglects any air resistance and just
% gives just a simple trajectory. The constants defined herein make the 
% trajectories somewhat realistic for a small ball shot from a canon. 
%
% On input:
%   dt (float): time step length in seconds
%   t_max (float): simulation is run from 0 up to t_max
%   g (float): gravitational acceleration in m/s^2
%
% On output: 
%   actual (4xN): projectile state [x;y;vx;vy] at each time step,
%       accounting for air resistance
%   ideal (4xN): projectile state [x;y;vx;vy] at each time step,
%       according to ideal frictionless model
%
% Example: [a, i] = projectile_trajectory(.1, 25, 9.8); 

% phyisical parameters
rho = 1.225; % air density in kg/m^3
C = .4; % drag coefficient
A = .0004; % cross sectional area in m^2 (radius about 1 cm)
m = .2; % weight in kg
D = rho*C*A/2;
px_init = 0; % initial position at origin
py_init = 0;
vx_init = 200; % vx=200, vy=150 is about 250 m/s at 37 degrees
vy_init = 150; 

% set up empty state vector (each col is [x;y;vx;vy])
actual = zeros(4, length(0:dt:t_max));
ideal = zeros(4, length(0:dt:t_max));

% compute actual pos, vel, accel at each time step (account for friction)
px = px_init;
py = py_init;
vx = vx_init; 
vy = vy_init;
v = (vx^2 + vy^2)^.5;
i = 1; % next time step index into state matrix
for t = 0:dt:t_max    
    % compute acceleration
    ax = -(D/m) * v * vx;
    ay = -g - (D/m) * v * vy;
    
    % save state at current time step
    actual(:,i) = [px;py;vx;vy];
    
    % update velocity
    vx = vx + ax * dt;
    vy = vy + ay * dt;
    
    % update position
    px = px + vx * dt + .5 * ax * dt^2;
    py = py + vy + dt + .5 * ay * dt^2;    
    
    i = i + 1;
end

% compute ideal pos, vel, accel at each time step (ignore friction)
px = px_init;
py = py_init;
vx = vx_init; 
vy = vy_init;
ax = 0; % acceleration is constant in ideal case
ay = -g;
i = 1;
for t = 0:dt:t_max        
    % save state at current time step
    ideal(:,i) = [px;py;vx;vy];
    
    % update velocity
    vx = vx + ax * dt;
    vy = vy + ay * dt;
    
    % update position
    px = px + vx * dt + .5 * ax * dt^2;
    py = py + vy + dt + .5 * ay * dt^2;    
    
    i = i + 1;
end

