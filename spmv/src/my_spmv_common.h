// ----------------------------------------------------------------------
// Original work Copyright 2011-2015 by Virginia Polytechnic Institute and State University
// All rights reserved.
// From the OpenDwarfs benchmark suite (https://github.com/vtsynergy/OpenDwarfs),
// released under the LGPL v2.1 license provided in the LICENSE file accompanying this software.
//
// Modified work Copyright (c) 2016, The Regents of the University of California All rights reserved.
// ----------------------------------------------------------------------
/*
 * Filename: my_spmv_common.h
 * Version: 1.0
 * Description: Sparse matrix-vector multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */



#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct csr_matrix
{//if ith row is empty Ap[i] = Ap[i+1]
    unsigned int index_type;
    float value_type;
    unsigned int num_rows, num_cols, num_nonzeros,density_ppm;
    double density_perc,nz_per_row,stddev;

    int * Ap;  //row pointer
    int * Aj;  //column indices
    float * Ax;  //nonzeros
}
csr_matrix;






int float_array_comp(const float* control, const float* experimental, const unsigned int N, const unsigned int exec_num);
void spmv_csr_cpu(const csr_matrix* csr,const float* x,const float* y,float* out);
csr_matrix* read_csr(unsigned int* num_csr,const char* file_path);
void free_csr(csr_matrix* csr,const unsigned int num_csr);

