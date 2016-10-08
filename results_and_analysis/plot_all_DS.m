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

clear;
load('bfs_dense.mat');
plot_design_space(run_results_timing, logic_util, 'BFS dense', 'BFS_dense');
clear;
load('bfs_sparse.mat');
plot_design_space(run_results_timing, logic_util, 'BFS sparse', 'BFS_sparse');
clear;
load('dct.mat');
plot_design_space(run_results_timing, logic_util, 'DCT', 'DCT');
clear;
load('fir.mat');
plot_design_space(run_results_timing, logic_util, 'FIR filter', 'FIR');
clear;
load('hist.mat');
plot_design_space(run_results_timing, logic_util, 'Histogram', 'Histogram');
clear;
load('mm.mat');
plot_design_space(run_results_timing, logic_util, 'Matrix Multiply', 'MM');
clear;
load('mergesort.mat');
plot_design_space(run_results_timing, logic_util, 'Merge Sort', 'Mergesort');
clear;
%load('nw.mat');
%plot_design_space(run_results_timing, logic_util, 'NW', 'NW');
%clear;
load('normals.mat');
plot_design_space(run_results_timing, logic_util, 'Normal estimation', 'Normals');
clear;
load('sobel.mat');
plot_design_space(run_results_timing, logic_util, 'Sobel filter', 'Sobel');
clear;
load('spmv_5000.mat');
plot_design_space(run_results_timing, logic_util, 'SPMV 0.5%', 'SPMV_5000');
clear;
load('spmv_500000.mat');
plot_design_space(run_results_timing, logic_util, 'SPMV 50%', 'SPMV_500000');