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

filename = 'hist_vs_model.pdf'
%filename = 'normals_vs_model.pdf'

D = [sortrows([Dp*Bl(:,Sl.IndexMinMSE)+Sl.Intercept(Sl.IndexMinMSE),yl], 2),...
    sortrows([Dp*Bt(:,St.IndexMinMSE)+St.Intercept(St.IndexMinMSE),yt],2)];

spn = 1;

% Create figure
figure1 = figure('Name','','Color',[1 1 1]);

ylabel_str = {'Normalized 1/Logic utilization', 'Normalized Throughput'};
xlabel_str = {'Design # - Sorted by Efficiency', 'Design # - Sorted by Performance'};

title_str = {'Histogram Design Space vs. LASSO Model', ''};
%title_str = {'Normal Est. Design Space vs. LASSO Model', ''};

for i=1:2:size(D,2)

    % Create axes
    axes1 = subplot(2,1,spn);

    xlim(axes1, [0, size(Dp,1)]);
    ylim(axes1, [0, 1.3]);

    color1 = [114 147 203]/255;
    color2 = [204 37 41]/255;

    % Figure title
    title(title_str{spn}, 'FontSize', 20,'fontweight','bold');

    % Create ylabel
    ylabel(ylabel_str{spn}, 'FontSize',14);

    % Create xlabel
    xlabel(xlabel_str{spn}, 'FontSize',18);
    
    % Create plot
    hold(axes1,'on');
    plot(D(:,i), 'MarkerFaceColor',color1,'LineStyle','-',...
        'Color',color1,'MarkerSize',pointSize)
    
    plot(D(:,i+1), 'MarkerFaceColor',color2,'LineStyle','-',...
        'Color',color2,'MarkerSize',pointSize)
    
    hold(axes1, 'off');
    
    l = legend('model prediction','ground truth','Location','NorthWest');
    set(l,'FontSize',16);
    legend boxoff 
    
    spn = spn+1;
end

% Print to PDF
if ~isempty(filename)
    
    set(figure1,'Units','Inches');

    pos = get(figure1,'Position');
    
    leftmargin  = -0.4;
    rightmargin = -0.6;
    
    set(figure1, 'PaperUnits', 'Inches',...
        'PaperPosition', [leftmargin 0 pos(3) pos(4)],...
        'PaperSize', [pos(3) + leftmargin + rightmargin pos(4)]);
        
    print(figure1, filename, '-dpdf', '-r0')
end


