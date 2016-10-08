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

function plot_design_space(run_results_timing, logic_util, figure_title, filename)

addpath ./analysis_code/

%%%%%%%
% A value of exactly 999 used to indicate invalid results in FIR and others
% All the invalid values were removed, but keeping this piece of code
% around just in case.

% good_idx=find(run_results_timing ~= 999);
% 
% num_designs = size(good_idx,1);
% 
% good_time = run_results_timing(good_idx);
% good_area = logic_util(good_idx);

% Normalize
%norm_time = (1./good_time)/(max(1./good_time));
%norm_area = ((1./good_area)/(max(1./good_area)));
%%%%%%%

num_designs = size(run_results_timing,1);
 
% Normalize
norm_time = (1./run_results_timing)/(max(1./run_results_timing));
norm_area = ((1./logic_util)/(max(1./logic_util)));

% Get Pareto optimal designs
[~, pareto] = prtp([norm_time, norm_area], true, 0, false, 1);
nonpareto = setdiff(1:num_designs, pareto);



X1 = norm_time(nonpareto);
Y1 = norm_area(nonpareto);
X2 = norm_time(pareto);
Y2 = norm_area(pareto);

pointSize = 5;

% Create figure
figure1 = figure('Name','Design Space','Color',[1 1 1]);

% Create axes
axes1 = axes('Parent',figure1);

xlim(axes1,[0 1]);
ylim(axes1,[0.2 1.1]);

ax = gca;
ax.XTick = 0:0.2:1;
ax.YTick = 0.2:0.2:1;

color1 = [114 147 203]/255;
color2 = [204 37 41]/255;

hold(axes1,'on');

% Figure title
title(figure_title, 'FontSize', 18);

% Create ylabel
ylabel('Normalized 1/Logic utilization','FontSize',16);

% Create xlabel
xlabel('Normalized Throughput','FontSize',16);

% Create plot
plot(X1,Y1,'MarkerFaceColor',color1,'Marker','o','LineStyle','none',...
    'Color',color1,'MarkerSize', pointSize);

% Create plot
plot(X2,Y2,'MarkerFaceColor',color2,'Marker','^','LineStyle','none',...
    'Color',color2,'MarkerSize', pointSize);



% Print to PDF
if ~isempty(filename)
    
    set(figure1,'Units','Inches');

    pos = get(figure1,'Position');
    
    leftmargin  = -0.2;
    rightmargin = -0.4;
    
    set(figure1, 'PaperUnits', 'Inches',...
        'PaperPosition', [leftmargin 0 pos(3) pos(4)],...
        'PaperSize', [pos(3)+leftmargin+rightmargin pos(4)]);
        
    print(figure1, filename, '-dpdf', '-r0')
end





