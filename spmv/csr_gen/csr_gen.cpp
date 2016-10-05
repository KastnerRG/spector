// ----------------------------------------------------------------------
// Original work Copyright 2011-2015 by Virginia Polytechnic Institute and State University
// All rights reserved.
// From the OpenDwarfs benchmark suite (https://github.com/vtsynergy/OpenDwarfs),
// released under the LGPL v2.1 license provided in the LICENSE file accompanying this software.
//
// Modified work Copyright (c) 2016, The Regents of the University of California All rights reserved.
// ----------------------------------------------------------------------
/*
 * Filename: csr_gen.cpp
 * Version: 1.0
 * Description: Sparse matrix-vector multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<getopt.h>
#include<math.h>
#include "ziggurat.h"


#define MINIMUM(i,j) ((i)<(j) ? (i) : (j))

#define CSR_NAME_MAX_LENGTH 256

void check(int b,const char* msg)
{
	if(!b)
	{
		fprintf(stderr,"error: %s\n\n",msg);
		exit(-1);
	}
}


int* int_new_array(const size_t N,const char* error_msg)
{
	int* ptr;
	int err;
	
	ptr = (int*)malloc(N * sizeof(int));
	check(ptr != NULL,error_msg);

	return ptr;
}

long* long_new_array(const size_t N,const char* error_msg)
{
	long* ptr;
	int err;
	ptr = (long*)malloc(N * sizeof(long));
	check(ptr != NULL,error_msg);

	return ptr;
}



float* float_new_array(const size_t N,const char* error_msg)
{
	float* ptr;
	int err;
	
	ptr = (float*)malloc(N * sizeof(float));
	check(ptr != NULL,error_msg);

	return ptr;
}


float* float_array_realloc(float* ptr,const size_t N,const char* error_msg)
{
	int err;

	ptr = (float*)realloc(ptr,N * sizeof(float));
	check(ptr != NULL,error_msg);

	return ptr;
}


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


unsigned long gen_rand(const long LB, const long HB)
{
	int range = HB - LB + 1;
	check((HB >= 0 && LB >= 0 && range > 0),"sparse_formats.gen_rand() - Invalid Bound(s). Exiting...");
	return (rand() % range) + LB;
}

int unsigned_int_comparator(const void* v1, const void* v2)
{
	const unsigned int int1 = *((unsigned int*) v1);
	const unsigned int int2 = *((unsigned int*) v2);

	if(int1 < int2)
		return -1;
	else if(int1 > int2)
		return +1;
	else
		return 0;
}


csr_matrix rand_csr(const unsigned int N,const unsigned int density, const double normal_stddev,unsigned long* seed,FILE* log)
{
	unsigned int i,j,nnz_ith_row,nnz,update_interval,rand_col;
	double nnz_ith_row_double,nz_error,nz_per_row_doubled,high_bound;
	int kn[128];
	float fn[128],wn[128];
	char* used_cols;
	csr_matrix csr;

	csr.num_rows = N;
	csr.num_cols = N;
	csr.density_perc = (((double)(density))/10000.0);
	csr.nz_per_row = (((double)N)*((double)density))/1000000.0;
	csr.num_nonzeros = round(csr.nz_per_row*N);
	csr.stddev = normal_stddev * csr.nz_per_row; //scale normalized standard deviation by average NZ/row

	fprintf(log,"Average NZ/Row: %-8.3f\n",csr.nz_per_row);
	fprintf(log,"Standard Deviation: %-8.3f\n",csr.stddev);
	fprintf(log,"Target Density: %u ppm = %g%%\n",density,csr.density_perc);
	fprintf(log,"Approximate NUM_nonzeros: %d\n",csr.num_nonzeros);

	csr.Ap = int_new_array(csr.num_rows+1,"rand_csr() - Heap Overflow! Cannot Allocate Space for csr.Ap");
	csr.Aj = int_new_array(csr.num_nonzeros,"rand_csr() - Heap Overflow! Cannot Allocate Space for csr.Aj");

	csr.Ap[0] = 0;
	nnz = 0;
	nz_per_row_doubled = 2*csr.nz_per_row; //limit nnz_ith_row to double the average because negative values are rounded up to 0. This
	high_bound = MINIMUM(csr.num_cols,nz_per_row_doubled); //limitation ensures the distribution will be symmetric about the mean, albeit not truly normal.
	used_cols = (char*)malloc(csr.num_cols*sizeof(char));
	check(used_cols != NULL,"rand_csr() - Heap Overflow! Cannot allocate space for used_cols");

	r4_nor_setup(kn,fn,wn);
	srand(*seed);

	update_interval = round(csr.num_rows / 10.0);
	if(!update_interval) update_interval = csr.num_rows;

	for(i=0; i<csr.num_rows; i++)
	{
		if(i % update_interval == 0) fprintf(log,"\t%d of %d (%5.1f%%) Rows Generated. Continuing...\n",i,csr.num_rows,((double)(i))/csr.num_rows*100);

		nnz_ith_row_double = r4_nor(seed,kn,fn,wn); //random, normally-distributed value for # of nz elements in ith row, NORMALIZED
		nnz_ith_row_double *= csr.stddev; //scale by standard deviation
		nnz_ith_row_double += csr.nz_per_row; //add average nz/row
		if(nnz_ith_row_double < 0)
			nnz_ith_row = 0;
		else if(nnz_ith_row_double > high_bound)
			nnz_ith_row = high_bound;
		else
			nnz_ith_row = (int) round(nnz_ith_row_double);

		csr.Ap[i+1] = csr.Ap[i] + nnz_ith_row;
		if(csr.Ap[i+1] > csr.num_nonzeros)
			csr.Aj = (int*)realloc(csr.Aj,sizeof(int)*csr.Ap[i+1]);

		for(j=0; j<csr.num_cols; j++)
			used_cols[j] = 0;

		for(j=0; j<nnz_ith_row; j++)
		{
			rand_col = gen_rand(0,csr.num_cols - 1);
			if(used_cols[rand_col])
			{
				j--;
			}
			else
			{
				csr.Aj[csr.Ap[i]+j] = rand_col;
				used_cols[rand_col] = 1;
			}
		}
		qsort((&(csr.Aj[csr.Ap[i]])),nnz_ith_row,sizeof(unsigned int),unsigned_int_comparator);
	}

	nz_error = ((double)abs((signed int)(csr.num_nonzeros - csr.Ap[csr.num_rows]))) / ((double)csr.num_nonzeros);
	if(nz_error >= .05)
		fprintf(stderr,"WARNING: Actual NNZ differs from Theoretical NNZ by %5.2f%%!\n",nz_error*100);
	csr.num_nonzeros = csr.Ap[csr.num_rows];
	fprintf(log,"Actual NUM_nonzeros: %d\n",csr.num_nonzeros);
	csr.density_perc = (((double)csr.num_nonzeros)*100.0)/((double)csr.num_cols)/((double)csr.num_rows);
	csr.density_ppm = (unsigned int)round(csr.density_perc * 10000.0);
	fprintf(log,"Actual Density: %u ppm = %g%%\n",csr.density_ppm,csr.density_perc);

	free(used_cols);
	csr.Ax = float_new_array(csr.num_nonzeros,"rand_csr() - Heap Overflow! Cannot Allocate Space for csr.Ax");
	for(i=0; i<csr.num_nonzeros; i++)
	{
		csr.Ax[i] = 1.0 - 2.0 * (rand() / (2147483647 + 1.0));
		while(csr.Ax[i] == 0.0)
			csr.Ax[i] = 1.0 - 2.0 * (rand() / (2147483647 + 1.0));
	}

	return csr;
}


void write_csr(const csr_matrix* csr,const unsigned int num_csr,const char* file_path)
{
	FILE* fp;
	int i,j;
	fp = fopen(file_path,"w");
	check(fp != NULL,"sparse_formats.write_csr() - Cannot Open File");
	fprintf(fp,"%u\n\n",num_csr);

	for(j=0; j<num_csr; j++)
	{
		fprintf(fp,"%u\n%u\n%u\n%u\n%lf\n%lf\n%lf\n",csr[j].num_rows,csr[j].num_cols,csr[j].num_nonzeros,csr[j].density_ppm,csr[j].density_perc,csr[j].nz_per_row,csr[j].stddev);

		for(i=0; i<=csr[j].num_rows; i++)
			fprintf(fp,"%u ",csr[j].Ap[i]);
		fprintf(fp,"\n");

		for(i=0; i<csr[j].num_nonzeros; i++)
			fprintf(fp,"%u ",csr[j].Aj[i]);
		fprintf(fp,"\n");

		for(i=0; i<csr[j].num_nonzeros; i++)
			fprintf(fp,"%f ",csr[j].Ax[i]);
		fprintf(fp,"\n\n");
	}

	fclose(fp);
}

void free_csr(csr_matrix* csr,const unsigned int num_csr)
{
	int k;
	for(k=0; k<num_csr; k++)
	{
		free(csr[k].Ap);
		free(csr[k].Aj);
		free(csr[k].Ax);
	}
	free(csr);
}



int main(int argc, char** argv)
{
	unsigned int N = 512,num_matrices=1,i;
	unsigned int density = 5000;
	unsigned long seed=10000;
	double normal_stddev = .01;
	char *file_path=NULL,do_print=0,do_rand=1,do_save=1,free_file=0;

	csr_matrix* csr = (csr_matrix*)malloc(sizeof(struct csr_matrix)*num_matrices);

	for(i=0; i<num_matrices; i++)
		csr[i] = rand_csr(N,density,normal_stddev,&seed,stdout);

	if(do_save)
	{
		if(!file_path)
		{
			file_path = (char*)malloc(sizeof(char)*CSR_NAME_MAX_LENGTH);
			check(file_path != NULL,"createcsr.main() - Heap Overflow! Cannot allocate space for 'file_path'");

			int normal_stddev_rounded = (int) round(normal_stddev * 100);
			snprintf(file_path,(sizeof(char)*CSR_NAME_MAX_LENGTH),"./csrmatrix_R%u_N%lu_D%lu_S%02d",num_matrices,N,density,normal_stddev_rounded);
			free_file=1;
		}
		printf("Saving Matrix to File '%s'...\n\n",file_path);
		write_csr(csr,num_matrices,file_path);
	}
	if(free_file) free(file_path);
	free_csr(csr,num_matrices);
	return 0;
}
