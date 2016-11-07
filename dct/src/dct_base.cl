// ----------------------------------------------------------------------
// Modified work
// Copyright (c) 2016, The Regents of the University of California All
// rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of The Regents of the University of California
//       nor the names of its contributors may be used to endorse or
//       promote products derived from this software without specific
//       prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REGENTS OF THE
// UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
// ----------------------------------------------------------------------
/*
 * Original work Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
/*
 * Filename: dct_base.cl
 * Version: 1.0
 * Description: DCT8x8 OpenCL benchmark.
 * Author: Pingfan Meng
 */


#if BLOCK_SIZE_F == 1
#define BLOCK_SIZE 8

#elif BLOCK_SIZE_F== 2
#define BLOCK_SIZE 16

#elif BLOCK_SIZE_F== 4
#define BLOCK_SIZE 32

#endif


#if SIMD_LOC == 1
typedef float SIMD_DTYPE;

#elif SIMD_LOC == 2
typedef float2 SIMD_DTYPE;

#elif SIMD_LOC == 4
typedef float4 SIMD_DTYPE;

#elif SIMD_LOC == 8
typedef float8 SIMD_DTYPE;

#endif

////////////////////////////////////////////////////////////////////////////////
// Hardcoded unrolled fast 8-point (i)DCT routines
////////////////////////////////////////////////////////////////////////////////
#define    C_a 1.3870398453221474618216191915664f  //a = sqrt(2) * cos(1 * pi / 16)
#define    C_b 1.3065629648763765278566431734272f  //b = sqrt(2) * cos(2 * pi / 16)
#define    C_c 1.1758756024193587169744671046113f  //c = sqrt(2) * cos(3 * pi / 16)
#define    C_d 0.78569495838710218127789736765722f //d = sqrt(2) * cos(5 * pi / 16)
#define    C_e 0.54119610014619698439972320536639f //e = sqrt(2) * cos(6 * pi / 16)
#define    C_f 0.27589937928294301233595756366937f //f = sqrt(2) * cos(7 * pi / 16)
#define C_norm 0.35355339059327376220042218105242f //1 / sqrt(8)


#if DCT_UNROLL==1
inline void DCT8(SIMD_DTYPE *D_I, SIMD_DTYPE *D_O){
    SIMD_DTYPE X07P = D_I[0] + D_I[7];
    SIMD_DTYPE X16P = D_I[1] + D_I[6];
    SIMD_DTYPE X25P = D_I[2] + D_I[5];
    SIMD_DTYPE X34P = D_I[3] + D_I[4];

    SIMD_DTYPE X07M = D_I[0] - D_I[7];
    SIMD_DTYPE X61M = D_I[6] - D_I[1];
    SIMD_DTYPE X25M = D_I[2] - D_I[5];
    SIMD_DTYPE X43M = D_I[4] - D_I[3];

    SIMD_DTYPE X07P34PP = X07P + X34P;
    SIMD_DTYPE X07P34PM = X07P - X34P;
    SIMD_DTYPE X16P25PP = X16P + X25P;
    SIMD_DTYPE X16P25PM = X16P - X25P;

    D_O[0] = C_norm * (X07P34PP + X16P25PP);
    D_O[2] = C_norm * (C_b * X07P34PM + C_e * X16P25PM);
    D_O[4] = C_norm * (X07P34PP - X16P25PP);
    D_O[6] = C_norm * (C_e * X07P34PM - C_b * X16P25PM);

    D_O[1] = C_norm * (C_a * X07M - C_c * X61M + C_d * X25M - C_f * X43M);
    D_O[3] = C_norm * (C_c * X07M + C_f * X61M - C_a * X25M + C_d * X43M);
    D_O[5] = C_norm * (C_d * X07M + C_a * X61M + C_f * X25M - C_c * X43M);
    D_O[7] = C_norm * (C_f * X07M + C_d * X61M + C_c * X25M + C_a * X43M);
}

#elif DCT_UNROLL==0
__constant float  C[16]= { 1.0f, 0.98078528040323043057924223830923f, 0.92387953251128673848313610506011f, 0.8314696123025452356714026791451f, 0.70710678118654752440084436210485f, 0.55557023301960228867102387084742f, 0.38268343236508983729038391174981f, 0.19509032201612833135051516819658f, 0, -0.19509032201612819257263709005201f, -0.38268343236508972626808144923416f, -0.55557023301960195560411648330046f, -0.70710678118654752440084436210485f, -0.83146961230254534669370514166076f, -0.92387953251128673848313610506011f, -0.98078528040323043057924223830923f};

inline void DCT8(SIMD_DTYPE *D_I, SIMD_DTYPE *D_O){
    SIMD_DTYPE buf[8];

    for(uint k = 0; k < 8; k++){
        buf[k] = 0;
        for(uint n = 0; n < 8; n++)
            buf[k] += D_I[n] * C[(2*n*k+k)%16]*(((2*n*k+k)/16)%2?-1:1);
    }

    D_O[0] = buf[0]* C_norm;
    for(uint i = 1; i < 8; i++)
        D_O[i] = buf[i] * 0.5f;
}
#endif 



#if SIMD_LOC == 1
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X/SIMD_LOC, BLOCKDIM_Y /BLOCK_SIZE, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0)*SIMD_LOC;
    const uint    localY = BLOCK_SIZE * get_local_id(1);
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i] = l_H[i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i] = l_V[i * (BLOCKDIM_X + 1)];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i];
    }
}
#endif

#if SIMD_LOC == 2
#if SIMD_TYPE == 0
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X/SIMD_LOC, BLOCKDIM_Y /BLOCK_SIZE, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0)*SIMD_LOC;
    const uint    localY = BLOCK_SIZE * get_local_id(1);
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[i * (BLOCKDIM_X + 1)+1]= d_Src[i * stride+1];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[(BLOCKDIM_X + 1)+i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[(BLOCKDIM_X + 1)+i] = D_1[i].s1;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[i * (BLOCKDIM_X + 1)+1];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[i * stride+1] =D_3[i].s1;
    }
}


#elif SIMD_TYPE == 1
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X, BLOCKDIM_Y /BLOCK_SIZE /SIMD_LOC, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0);
    const uint    localY = BLOCK_SIZE * get_local_id(1)*SIMD_LOC;
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + BLOCK_SIZE) * stride];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s1;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[(i + BLOCK_SIZE) * stride] =D_3[i].s1;
    }
}

#endif  //#if SIMD_TYPE == 0,1


#endif //#if SIMD_LOC == 2



#if SIMD_LOC == 4
#if SIMD_TYPE == 0
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X/SIMD_LOC, BLOCKDIM_Y /BLOCK_SIZE, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0)*SIMD_LOC;
    const uint    localY = BLOCK_SIZE * get_local_id(1);
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[i * (BLOCKDIM_X + 1)+1]= d_Src[i * stride+1];
	l_V[i * (BLOCKDIM_X + 1)+2]= d_Src[i * stride+2];
	l_V[i * (BLOCKDIM_X + 1)+3]= d_Src[i * stride+3];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[(BLOCKDIM_X + 1)+i];
        D_0[i].s2 = l_H[(BLOCKDIM_X + 1)*2+i];
	D_0[i].s3 = l_H[(BLOCKDIM_X + 1)*3+i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[(BLOCKDIM_X + 1)+i] = D_1[i].s1;
	l_H[(BLOCKDIM_X + 1)*2+i] = D_1[i].s2;
	l_H[(BLOCKDIM_X + 1)*3+i] = D_1[i].s3;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[i * (BLOCKDIM_X + 1)+1];
	D_2[i].s2 = l_V[i * (BLOCKDIM_X + 1)+2];
	D_2[i].s3 = l_V[i * (BLOCKDIM_X + 1)+3];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[i * stride+1] =D_3[i].s1;
	d_Dst[i * stride+2] =D_3[i].s2;
	d_Dst[i * stride+3] =D_3[i].s3;
    }
}


#elif SIMD_TYPE == 1
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X, BLOCKDIM_Y /BLOCK_SIZE /SIMD_LOC, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0);
    const uint    localY = BLOCK_SIZE * get_local_id(1)*SIMD_LOC;
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + BLOCK_SIZE) * stride];
	l_V[(i + 2*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 2*BLOCK_SIZE) * stride];
	l_V[(i + 3*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 3*BLOCK_SIZE) * stride];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s2 = l_H[2*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s3 = l_H[3*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s1;
	l_H[2*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s2;
	l_H[3*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s3;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s2 = l_V[(i + 2*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s3 = l_V[(i + 3*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[(i + BLOCK_SIZE) * stride] =D_3[i].s1;
	d_Dst[(i + 2*BLOCK_SIZE) * stride] =D_3[i].s2;
	d_Dst[(i + 3*BLOCK_SIZE) * stride] =D_3[i].s3;
    }
}
#endif  //#if SIMD_TYPE == 0,1


#endif //#if SIMD_LOC == 4



#if SIMD_LOC == 8
#if SIMD_TYPE == 0
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X/SIMD_LOC, BLOCKDIM_Y /BLOCK_SIZE, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0)*SIMD_LOC;
    const uint    localY = BLOCK_SIZE * get_local_id(1);
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[i * (BLOCKDIM_X + 1)+1]= d_Src[i * stride+1];
	l_V[i * (BLOCKDIM_X + 1)+2]= d_Src[i * stride+2];
	l_V[i * (BLOCKDIM_X + 1)+3]= d_Src[i * stride+3];
	l_V[i * (BLOCKDIM_X + 1)+4]= d_Src[i * stride+4];
	l_V[i * (BLOCKDIM_X + 1)+5]= d_Src[i * stride+5];
	l_V[i * (BLOCKDIM_X + 1)+6]= d_Src[i * stride+6];
	l_V[i * (BLOCKDIM_X + 1)+7]= d_Src[i * stride+7];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[(BLOCKDIM_X + 1)+i];
        D_0[i].s2 = l_H[(BLOCKDIM_X + 1)*2+i];
	D_0[i].s3 = l_H[(BLOCKDIM_X + 1)*3+i];
	D_0[i].s4 = l_H[(BLOCKDIM_X + 1)*4+i];
	D_0[i].s5 = l_H[(BLOCKDIM_X + 1)*5+i];
	D_0[i].s6 = l_H[(BLOCKDIM_X + 1)*6+i];
	D_0[i].s7 = l_H[(BLOCKDIM_X + 1)*7+i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[(BLOCKDIM_X + 1)+i] = D_1[i].s1;
	l_H[(BLOCKDIM_X + 1)*2+i] = D_1[i].s2;
	l_H[(BLOCKDIM_X + 1)*3+i] = D_1[i].s3;
	l_H[(BLOCKDIM_X + 1)*4+i] = D_1[i].s4;
	l_H[(BLOCKDIM_X + 1)*5+i] = D_1[i].s5;
	l_H[(BLOCKDIM_X + 1)*6+i] = D_1[i].s6;
	l_H[(BLOCKDIM_X + 1)*7+i] = D_1[i].s7;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[i * (BLOCKDIM_X + 1)+1];
	D_2[i].s2 = l_V[i * (BLOCKDIM_X + 1)+2];
	D_2[i].s3 = l_V[i * (BLOCKDIM_X + 1)+3];
	D_2[i].s4 = l_V[i * (BLOCKDIM_X + 1)+4];
	D_2[i].s5 = l_V[i * (BLOCKDIM_X + 1)+5];
	D_2[i].s6 = l_V[i * (BLOCKDIM_X + 1)+6];
	D_2[i].s7 = l_V[i * (BLOCKDIM_X + 1)+7];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[i * stride+1] =D_3[i].s1;
	d_Dst[i * stride+2] =D_3[i].s2;
	d_Dst[i * stride+3] =D_3[i].s3;
	d_Dst[i * stride+4] =D_3[i].s4;
	d_Dst[i * stride+5] =D_3[i].s5;
	d_Dst[i * stride+6] =D_3[i].s6;
	d_Dst[i * stride+7] =D_3[i].s7;
    }
}


#elif SIMD_TYPE == 1
__attribute__((num_simd_work_items(SIMD_WI)))
__attribute__((num_compute_units(COMP_U)))
__kernel __attribute__((reqd_work_group_size(BLOCKDIM_X, BLOCKDIM_Y /BLOCK_SIZE /SIMD_LOC, 1)))
void DCT8x8(
    __global float * restrict d_Dst,
    __global float * restrict d_Src, 
    uint stride,
    uint imageH,
    uint imageW
){
    __local float l_Transpose[BLOCKDIM_Y][BLOCKDIM_X + 1];
    const uint    localX = get_local_id(0);
    const uint    localY = BLOCK_SIZE * get_local_id(1)*SIMD_LOC;
    const uint modLocalX = localX & (BLOCK_SIZE - 1);
    const uint   globalX = get_group_id(0) * BLOCKDIM_X + localX;
    const uint   globalY = get_group_id(1) * BLOCKDIM_Y + localY;

    //Process only full blocks
    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
        return;

    __local float *l_V = &l_Transpose[localY +         0][localX +         0];
    __local float *l_H = &l_Transpose[localY + modLocalX][localX  - modLocalX];

    d_Src += globalY * stride + globalX;
    d_Dst += globalY * stride + globalX;

    SIMD_DTYPE D_0[BLOCK_SIZE];
    SIMD_DTYPE D_1[BLOCK_SIZE];
    SIMD_DTYPE D_2[BLOCK_SIZE];
    SIMD_DTYPE D_3[BLOCK_SIZE];
   
    #pragma unroll BLOCK_SIZE   
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_V[i * (BLOCKDIM_X + 1)] = d_Src[i * stride];
	l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + BLOCK_SIZE) * stride];
	l_V[(i + 2*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 2*BLOCK_SIZE) * stride];
	l_V[(i + 3*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 3*BLOCK_SIZE) * stride];
	l_V[(i + 4*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 4*BLOCK_SIZE) * stride];
	l_V[(i + 5*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 5*BLOCK_SIZE) * stride];
	l_V[(i + 6*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 6*BLOCK_SIZE) * stride];
	l_V[(i + 7*BLOCK_SIZE) * (BLOCKDIM_X + 1)]= d_Src[(i + 7*BLOCK_SIZE) * stride];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_0[i].s0 = l_H[i];
	D_0[i].s1 = l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s2 = l_H[2*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s3 = l_H[3*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s4 = l_H[4*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s5 = l_H[5*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s6 = l_H[6*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
	D_0[i].s7 = l_H[7*BLOCK_SIZE * (BLOCKDIM_X + 1) + i];
    }

    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_0[i*8],&D_1[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        l_H[i] = D_1[i].s0;
        l_H[BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s1;
	l_H[2*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s2;
	l_H[3*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s3;
	l_H[4*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s4;
	l_H[5*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s5;
	l_H[6*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s6;
	l_H[7*BLOCK_SIZE * (BLOCKDIM_X + 1) + i] = D_1[i].s7;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        D_2[i].s0 = l_V[i * (BLOCKDIM_X + 1)];
        D_2[i].s1 = l_V[(i + BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s2 = l_V[(i + 2*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s3 = l_V[(i + 3*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s4 = l_V[(i + 4*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s5 = l_V[(i + 5*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s6 = l_V[(i + 6*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
	D_2[i].s7 = l_V[(i + 7*BLOCK_SIZE) * (BLOCKDIM_X + 1)];
    }
    
    
    #if BLOCK_UNROLL
    #pragma unroll BLOCK_SIZE_F
    #endif
    for(uint i = 0; i < BLOCK_SIZE_F; i++)
    {
    	DCT8(&D_2[i*8],&D_3[i*8]);
    }

    #pragma unroll BLOCK_SIZE
    for(uint i = 0; i < BLOCK_SIZE; i++)
    {
        d_Dst[i * stride] =D_3[i].s0;
        d_Dst[(i + BLOCK_SIZE) * stride] =D_3[i].s1;
	d_Dst[(i + 2*BLOCK_SIZE) * stride] =D_3[i].s2;
	d_Dst[(i + 3*BLOCK_SIZE) * stride] =D_3[i].s3;
	d_Dst[(i + 4*BLOCK_SIZE) * stride] =D_3[i].s4;
	d_Dst[(i + 5*BLOCK_SIZE) * stride] =D_3[i].s5;
	d_Dst[(i + 6*BLOCK_SIZE) * stride] =D_3[i].s6;
	d_Dst[(i + 7*BLOCK_SIZE) * stride] =D_3[i].s7;
    }
}

#endif  //#if SIMD_TYPE == 0,1

#endif //#if SIMD_LOC == 8
