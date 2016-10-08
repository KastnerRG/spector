% Let Fi(X), i=1...n, are objective functions
% for minimization. 
% A point X* is said to be Pareto optimal one
% if there is no X such that Fi(X)<=Fi(X*) for 
% all i=1...n, with at least one strict inequality.
% A=prtp(B),
% B - m x n input matrix: B=
% [F1(X1) F2(X1) ... Fn(X1);
%  F1(X2) F2(X2) ... Fn(X2);
%  .......................
%  F1(Xm) F2(Xm) ... Fn(Xm)]
% A - an output matrix with rows which are Pareto
% points (rows) of input matrix B.
% [A,b]=prtp(B). b is a vector which contains serial
% numbers of matrix B Pareto points (rows).
% Example.
% B=[0 1 2; 1 2 3; 3 2 1; 4 0 2; 2 2 1;...
%    1 1 2; 2 1 1; 0 2 2];
% [A b]=prtp(B)
% A =
%      0     1     2
%      4     0     2
%      2     2     1
% b =
%      1     4     7
% 
% Attribution: 
% https://www.mathworks.com/matlabcentral/fileexchange/22507-calculation-of-pareto-points
%
% Copyright (c) 2006, Eduard Polityko
% All rights reserved.
% Lightly modified (plotting flags, some computation) by Alric Althoff 
% 
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are
% met:
% 
%     * Redistributions of source code must retain the above copyright
%       notice, this list of conditions and the following disclaimer.
%     * Redistributions in binary form must reproduce the above copyright
%       notice, this list of conditions and the following disclaimer in
%       the documentation and/or other materials provided with the distribution
% 
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
% POSSIBILITY OF SUCH DAMAGE.
%


function [A varargout] = prtp(B, max_flag, soft_thresh, plot_flag, fig_num)

A=[]; 
varargout{1}=[];
sz1 = size(B,1);
jj=0; kk(sz1) = 0;
c(sz1,size(B,2)) = 0;
bb = c;

if max_flag
    cmp = @(x)x > -soft_thresh;
else
    cmp = @(x)x < soft_thresh;
end

for k=1:sz1
    j = 0;
    ak = B(k,:);
    for i=1:sz1
        if i ~= k
            j = j+1;
            bb(j,:) = ak - B(i,:);
        end
    end
    
    if any(cmp(bb(1:j,:)'))
        jj = jj+1;
        c(jj,:) = ak;
        kk(jj) = k;
    end
end

if jj
    A = c(1:jj,:);
    varargout{1} = kk(1:jj);
else
    warning('Points:Pareto',...
        'There are no Pareto points. The result is an empty matrix.')
end
if plot_flag
    figure(fig_num);plot(B(:,1),B(:,2),'.',A(:,1),A(:,2),'.')
    drawnow;
end
