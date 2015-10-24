function [mu, sigma] = kalman_filter(mu_prev, sigma_prev, sys, z)
% Implementation of the Kalman filter algorithm for linear dynamic systems.
%
% On input:
%   mu_prev (nx1 vector): previous belief on means for all n states
%   sigma_prev (nxn matrix): previous belief on state covariance
%   sys (struct): structure representing the system, consisting of:
%       .A (nxn matrix): state transition model
%       .B (nxm matrix): control-input model
%       .C (kxn matrix): observation model
%       .R (nxn matrix): covariance matrix for transition uncertainty 
%       .Q (kxk matrix): covariance matrix for observation noise
%       .u (mx1 vector): control vector
%   z (kx1 vector): measurement vector
%
% On output: 
%   mu (nx1 vector): updated belief on state means
%   sigma (nxn matrix): updated belief on state covariance

% update mean and covariance belief without incorporating measurement
mu_bar = sys.A * mu_prev + sys.B * sys.u;
sigma_bar = sys.A * sigma_prev * sys.A' + sys.R;

% if measurement is provided, take measurement into account 
if isempty(z)
    mu = mu_bar;
    sigma = sigma_bar;
else
    % compute kalman gain--how much the measurement affects new belief state
    K = sigma_bar * sys.C' / (sys.C * sigma_bar * sys.C' + sys.Q);

    % incorporate measurement to give desired belief
    mu = mu_bar + K * (z - sys.C * mu_bar);
    KC = K * sys.C;
    sigma = (eye(size(KC)) - KC) * sigma_bar;
end
