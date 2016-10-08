% ----------------------------------------------------------------------
% Copyright (c) 2016, The Regents of the University of California All
% rights reserved.
% 
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are
% met:
% 
%     * Redistributions of source code must retain the above copyright
%       notice, this list of conditions and the following disclaimer.
% 
%     * Redistributions in binary form must reproduce the above
%       copyright notice, this list of conditions and the following
%       disclaimer in the documentation and/or other materials provided
%       with the distribution.
% 
%     * Neither the name of The Regents of the University of California
%       nor the names of its contributors may be used to endorse or
%       promote products derived from this software without specific
%       prior written permission.
% 
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
% "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
% LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
% A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REGENTS OF THE
% UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT, INDIRECT,
% INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
% BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
% OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
% ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
% TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
% USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
% DAMAGE.
% ----------------------------------------------------------------------

function [R2,fvu,gini] = compute_r2_from_lasso(X,y,B,S)

    weights = B(:,S.IndexMinMSE);
    
    norm_weights = abs(weights)./sum(abs(weights));
    gini = ginicoeff(norm_weights);
    
    y_hat = X*weights + S.Intercept(S.IndexMinMSE);
    
    centered_y = y - mean(y);
    
    SStot = sum(centered_y.^2);
    SSres = sum((y - y_hat).^2);
    
    n = length(y);
    p = size(X,2);
    
    fvu = ((n-1)/(n-p)) * (SSres/SStot);  % fraction of variance unexplained
    R2 = 1 - fvu;

end