function model = projectile_model(dt, g, r, q)
% System dynamics for a 2D projectile. State is x and y components of 
% position and velocity. Assuming no friction and constant gravitational
% acceleration in negative y direction.
%
% Linear Gaussian: x_t = A_t * x_t0 + B_t * u_t + noise
%
% On input:
%   dt (float): time step in seconds
%   g (float): gravitational acceleration in m/s^2
%   r (float): transition noise (diagonal on covariance matrix)
%   q (float): observation noise (diagonal on covariance matrix)
%
% On output: 
%   sys (struct): structure representing the system, consisting of:
%       .A (4x4 matrix): state transition model
%       .B (4x4 matrix): control-input model
%       .C (2x4 matrix): observation model
%       .R (4x4 matrix): covariance matrix for transition uncertainty 
%       .Q (2x2 matrix): covariance matrix for observation noise
%       .u (4x1 vector): control vector
%
% Example: s = projectile_model(.1, 9.8, .1, 100);

% state transition: p = p + dt * v; v = v + dt * a
model.A = [1, 0, dt, 0;
           0, 1, 0, dt;
           0, 0, 1, 0;
           0, 0, 0, 1];

% select only y acceleration 
model.B = [0, 0, 0, 0;
           0, 0, 0, 0;
           0, 0, 0, 0;
           0, 0, 0, 1];
     
% only acceleration is from gravity in negative y direction
model.u = [0; 0; 0; -dt*g];     

% observe only x and y position
model.C = [1, 0, 0, 0;
           0, 1, 0, 0];     
     
% set uncertainty on transition (R) and measurement (Q)
model.R = eye(4) * r;
model.Q = eye(2) * q;    
     
