// ----------------------------------------------------------------------
// Original work Copyright 2011-2015 by Virginia Polytechnic Institute and State University
// All rights reserved.
// From the OpenDwarfs benchmark suite (https://github.com/vtsynergy/OpenDwarfs),
// released under the LGPL v2.1 license provided in the LICENSE file accompanying this software.
//
// Modified work Copyright (c) 2016, The Regents of the University of California All rights reserved.
// ----------------------------------------------------------------------
/*
 * Filename: spmv_base.cl
 * Version: 1.0
 * Description: Sparse matrix-vector multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */



#define NUM_PAR UNROLL_F*VECT_WIDTH


#if VECT_WIDTH==1
#define FLOAT_VECT float
float sub_sum(FLOAT_VECT * tmp_Ax, FLOAT_VECT * tmp_x)
{
	__private FLOAT_VECT tmp_sum=0;
//parallel mul
	#pragma unroll UNROLL_F
	for (int ii=0; ii<UNROLL_F; ii++)
		tmp_sum+= tmp_Ax[ii]*tmp_x[ii];

//reduction
	return tmp_sum;
	
}
#endif

#if VECT_WIDTH==2
#define FLOAT_VECT float2
float sub_sum(FLOAT_VECT * tmp_Ax, FLOAT_VECT * tmp_x)
{
	__private FLOAT_VECT tmp_sum=0;
//parallel mul
	#pragma unroll UNROLL_F
	for (int ii=0; ii<UNROLL_F; ii++)
		tmp_sum+= tmp_Ax[ii]*tmp_x[ii];


//reduction
	return tmp_sum.s0+tmp_sum.s1;
	
}
#endif

#if VECT_WIDTH==4
#define FLOAT_VECT float4
float sub_sum(FLOAT_VECT * tmp_Ax, FLOAT_VECT * tmp_x)
{
	__private FLOAT_VECT tmp_sum=0;
//parallel mul
	#pragma unroll UNROLL_F
	for (int ii=0; ii<UNROLL_F; ii++)
		tmp_sum+= tmp_Ax[ii]*tmp_x[ii];


//reduction
	__private float tmp_red0, tmp_red1;
	tmp_red0=tmp_sum.s0+tmp_sum.s2;
	tmp_red1=tmp_sum.s1+tmp_sum.s3;

	return tmp_red0+tmp_red1;
}
#endif


#if VECT_WIDTH==8
#define FLOAT_VECT float8
float sub_sum(FLOAT_VECT * tmp_Ax, FLOAT_VECT * tmp_x)
{
	__private FLOAT_VECT tmp_sum=0;
//parallel mul
	#pragma unroll UNROLL_F
	for (int ii=0; ii<UNROLL_F; ii++)
		tmp_sum+= tmp_Ax[ii]*tmp_x[ii];


//reduction
	__private float tmp_red0, tmp_red1,tmp_red2,tmp_red3,tmp_red4,tmp_red5;
	tmp_red0=tmp_sum.s0+tmp_sum.s4;
	tmp_red1=tmp_sum.s1+tmp_sum.s5;
	tmp_red2=tmp_sum.s2+tmp_sum.s6;
	tmp_red3=tmp_sum.s3+tmp_sum.s7;

	tmp_red4=tmp_red0+tmp_red2;
	tmp_red5=tmp_red1+tmp_red3;

	return tmp_red4+tmp_red5;
}
#endif


#if VECT_WIDTH==16
#define FLOAT_VECT float16
float sub_sum(FLOAT_VECT * tmp_Ax, FLOAT_VECT * tmp_x)
{
	__private FLOAT_VECT tmp_sum=0;
//parallel mul
	#pragma unroll UNROLL_F
	for (int ii=0; ii<UNROLL_F; ii++)
		tmp_sum+= tmp_Ax[ii]*tmp_x[ii];


//reduction
	__private float tmp_red0,tmp_red1,tmp_red2,tmp_red3,tmp_red4,tmp_red5,tmp_red6,tmp_red7,tmp_red8,tmp_red9,tmp_red10,tmp_red11,tmp_red12,tmp_red13;
	tmp_red0=tmp_sum.s0+tmp_sum.s8;
	tmp_red1=tmp_sum.s1+tmp_sum.s9;
	tmp_red2=tmp_sum.s2+tmp_sum.sa;
	tmp_red3=tmp_sum.s3+tmp_sum.sb;
	tmp_red4=tmp_sum.s4+tmp_sum.sc;
	tmp_red5=tmp_sum.s5+tmp_sum.sd;
	tmp_red6=tmp_sum.s6+tmp_sum.se;
	tmp_red7=tmp_sum.s7+tmp_sum.sf;

	tmp_red8=tmp_red0+tmp_red4;
	tmp_red9=tmp_red1+tmp_red5;
	tmp_red10=tmp_red2+tmp_red6;
	tmp_red11=tmp_red3+tmp_red7;

	tmp_red12=tmp_red8+tmp_red10;
	tmp_red13=tmp_red9+tmp_red11;

	return tmp_red12+tmp_red13;
}
#endif



//__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__attribute__((reqd_work_group_size(BLOCKDIM,1,1)))
void __kernel csr(const unsigned int num_rows,
		__global const unsigned int* restrict Ap, 
		__global const unsigned int* restrict Aj, 
		__global const float* restrict Ax, 
		__constant float* restrict x, 
		__global float* restrict y)
{
	__private size_t row = get_global_id(0);

	if(row < num_rows)
	{     
		__private float sum = y[row];

		__private const unsigned int row_start = Ap[row];
		__private const unsigned int row_end   = Ap[row+1];

		__private size_t jj,ii,kk;
		__private FLOAT_VECT p_Ax[UNROLL_F];
		//__private unsigned int p_Aj[UNROLL_F];
		__private FLOAT_VECT	p_x[UNROLL_F];
		for (jj = row_start; jj < row_end; jj=jj+NUM_PAR)
		{
			if(jj+NUM_PAR-1 < row_end)
			{
#pragma unroll UNROLL_F
				for(ii=0; ii<UNROLL_F; ii++)
				{
					#if VECT_WIDTH==1
					p_x[ii] = x[Aj[jj+ii*VECT_WIDTH]];
					#endif

					#if VECT_WIDTH>=2
					p_x[ii].s0 = x[Aj[jj+ii*VECT_WIDTH]];
					p_x[ii].s1 = x[Aj[jj+ii*VECT_WIDTH+1]];
					#endif


					#if VECT_WIDTH>=4
					p_x[ii].s2 = x[Aj[jj+ii*VECT_WIDTH+2]];
					p_x[ii].s3 = x[Aj[jj+ii*VECT_WIDTH+3]];
					#endif


					#if VECT_WIDTH>=8
					p_x[ii].s4 = x[Aj[jj+ii*VECT_WIDTH+4]];
					p_x[ii].s5 = x[Aj[jj+ii*VECT_WIDTH+5]];
					p_x[ii].s6 = x[Aj[jj+ii*VECT_WIDTH+6]];
					p_x[ii].s7 = x[Aj[jj+ii*VECT_WIDTH+7]];
					#endif

					#if VECT_WIDTH>=16
					p_x[ii].s8 = x[Aj[jj+ii*VECT_WIDTH+8]];
					p_x[ii].s9 = x[Aj[jj+ii*VECT_WIDTH+9]];
					p_x[ii].sa = x[Aj[jj+ii*VECT_WIDTH+10]];
					p_x[ii].sb = x[Aj[jj+ii*VECT_WIDTH+11]];
					p_x[ii].sc = x[Aj[jj+ii*VECT_WIDTH+12]];
					p_x[ii].sd = x[Aj[jj+ii*VECT_WIDTH+13]];
					p_x[ii].se = x[Aj[jj+ii*VECT_WIDTH+14]];
					p_x[ii].sf = x[Aj[jj+ii*VECT_WIDTH+15]];
					#endif
				}

#pragma unroll UNROLL_F
				for(ii=0; ii<UNROLL_F; ii++)
				{
					#if VECT_WIDTH==1
					p_Ax[ii] = Ax[jj+ii*VECT_WIDTH];
					#endif

					#if VECT_WIDTH>=2
					p_Ax[ii].s0 = Ax[jj+ii*VECT_WIDTH];
					p_Ax[ii].s1 = Ax[jj+ii*VECT_WIDTH+1];
					#endif


					#if VECT_WIDTH>=4
					p_Ax[ii].s2 = Ax[jj+ii*VECT_WIDTH+2];
					p_Ax[ii].s3 = Ax[jj+ii*VECT_WIDTH+3];
					#endif


					#if VECT_WIDTH>=8
					p_Ax[ii].s4 = Ax[jj+ii*VECT_WIDTH+4];
					p_Ax[ii].s5 = Ax[jj+ii*VECT_WIDTH+5];
					p_Ax[ii].s6 = Ax[jj+ii*VECT_WIDTH+6];
					p_Ax[ii].s7 = Ax[jj+ii*VECT_WIDTH+7];
					#endif

					#if VECT_WIDTH>=16
					p_Ax[ii].s8 = Ax[jj+ii*VECT_WIDTH+8];
					p_Ax[ii].s9 = Ax[jj+ii*VECT_WIDTH+9];
					p_Ax[ii].sa = Ax[jj+ii*VECT_WIDTH+10];
					p_Ax[ii].sb = Ax[jj+ii*VECT_WIDTH+11];
					p_Ax[ii].sc = Ax[jj+ii*VECT_WIDTH+12];
					p_Ax[ii].sd = Ax[jj+ii*VECT_WIDTH+13];
					p_Ax[ii].se = Ax[jj+ii*VECT_WIDTH+14];
					p_Ax[ii].sf = Ax[jj+ii*VECT_WIDTH+15];
					#endif
				}
				
				sum += sub_sum(p_Ax, p_x);
			}
			else
			{
				for(kk=jj; kk<row_end; kk++)
					sum += Ax[kk] * x[Aj[kk]];
			}
		}
		y[row] = sum;
	}
}


