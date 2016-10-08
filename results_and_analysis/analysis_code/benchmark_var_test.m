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

% Note: This should produce tables found in the paper using MATLAB R2013a
rng(0);

path = '../';

files = {...
    'bfs_dense',...
    'bfs_sparse',...
    'dct',...
    'fir',...
    'hist',...
    'mm',...
    'normals',...
    'sobel',...
    'spmv_5000',...
    'spmv_500000',...
    'mergesort'};

soft_pareto_threshs = [1, .1];

table_top_row = ['Soft Pareto '];
for idx=1:length(soft_pareto_threshs)
    table_top_row = [table_top_row,...
        '& \multicolumn{4}{c|}{',num2str(soft_pareto_threshs(idx)),'} '];
end

table_second_row = ['Benchmark '];
for idx=1:length(soft_pareto_threshs)
    table_second_row = [table_second_row,...
        '& $r^2_\ell$ & $G_\ell(\beta)$ & $r^2_t$ & $G_t(\beta)$ '];
end

disp([table_top_row, ' \\']);
disp('\hline');
disp([table_second_row, ' \\']);
disp('\hline');

% For use by charting functions
betas_idx = 1;
betas_l = cell(0);
betas_t = cell(0);

format shortg

for i=1:length(files)
    fn = strcat(path, files{i}, '.mat');
    data = load(fn);
    
    % 999 is for invalid values (did not run, or failed in some other way)
    data.logic_util = data.logic_util(data.run_results_timing~=999);
    data.knob_settings = data.knob_settings(data.run_results_timing~=999,:);
    data.run_results_timing = data.run_results_timing(data.run_results_timing~=999,:);
    
    % Unit-square-ize responses 
    data.logic_util = (1./data.logic_util)./max(1./data.logic_util);
    data.run_results_timing = (1./data.run_results_timing)./max(1./data.run_results_timing);
    
    
    latex_table = cell(length(soft_pareto_threshs),1);
    
    % Test stats for differing "soft" pareto thresholds
    for thresh_idx=1:length(soft_pareto_threshs)
        
        [y, sp_idx] = prtp([data.run_results_timing, data.logic_util], ...
                            true,...
                            soft_pareto_threshs(thresh_idx),...
                            true,...
                            1);
                        
        yt = y(:,1);
        yl = y(:,2);
        X = data.knob_settings(sp_idx,:);

        D = x2fx(X, 'interaction');
        D(:,1) = []; % Not interested in constant term
        D(:, sum(D)==0 | sum(D)==size(D,1)) = []; % Not interested in trivial interactions
        
        % Eliminate redundant variables
        d_idx = 1;
        while d_idx < size(D,2)-1
            if any(D(:,d_idx) ~= D(:,d_idx+1))
                d_idx = d_idx+1;
            else
                D(:,d_idx+1) = [];
            end
        end
        
        Dp = D;

        [Bl,Sl] = lasso(Dp,yl,'CV',10,'Standardize',true);
        [Bt,St] = lasso(Dp,yt,'CV',10,'Standardize',true);

        Blp = [Bl(:, Sl.IndexMinMSE), Bl(:, Sl.Index1SE)];
        Btp = [Bt(:, St.IndexMinMSE), Bt(:, St.Index1SE)];
        
        betas_l{betas_idx} = Blp;
        betas_t{betas_idx} = Btp;
        betas_idx = betas_idx+1;
        
        [r2l, ~, ginil] = compute_r2_from_lasso(Dp, yl, Bl, Sl);
        [r2t, ~, ginit] = compute_r2_from_lasso(Dp, yt, Bt, St);
        
        round_to = @(x, n)(round(x*10^n)/10^n);
        
        latex_table{thresh_idx} = [
            sprintf('%0.2f',round_to(r2l, 2)),' & ',...
            sprintf('%0.2f',round_to(ginil, 2)),' & ',...
            sprintf('%0.2f',round_to(r2t, 2)),' & ',...
            sprintf('%0.2f',round_to(ginit, 2)), ' & '];
        
    end 
    
    table_row = cell2mat(latex_table');
    disp([files{i}, ' & ', table_row(1:end-3), ' \\'])
end


