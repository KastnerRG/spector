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

function plot_knobs(knob_settings, results, name)

knobs_all = [knob_settings results];

[m,n] = size(knobs_all);

results_knob = zeros(n-1,m,2);

for i = 1:n-1
    results_knob(i,:,:) = sortrows(knobs_all(:, [i,end]), 1);
end

for i = 1:n-1
    data = squeeze(results_knob(i,:,:));
    x = data(:,1);
    y = data(:,2);
    
    figure('Name', sprintf('%s - Knob %d', name, i))
    plot(x,y,'*')
end

end

% plot_knobs(knob_settings, logic_util, 'Logic');
% plot_knobs(knob_settings, run_results_timing, 'Timing');

