// Copyright (C) 2013-2014 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

/******************************************************************************
** File: PcaCArray.h
**
** HPEC Challenge Benchmark Suite
** Common Header File
**
** Contents:
**    Definition and methods of the Generic C Array package.
** Description:
**    A PcaArray struct encapsulates a multi-dimensional array of data (up 
**    to three dimensions). This structure is used solely as a standard 
**    means to interface the I/O between Matlab and the PCA C kernels. It 
**    supports data type including int and float. Complex data are
**    stored in an interleaved manner in memory (i.e. r0,i0,r1,i1,...). For
**    details, please see the comment above each macro.
**
** Author: Hector Chan
**         MIT Lincoln Laboratory
**
******************************************************************************/
#ifndef PCA_CARRAY_H
#define PCA_CARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AOCLUtils/aocl_utils.h"

/* Constants for identifying REAL/COMPLEX data */
#define PCA_REAL    1
#define PCA_COMPLEX 2

/* Boolean true/false */
#define PCA_FALSE 0
#define PCA_TRUE  1

/* 32-bit unsigned integer type */
/* This typedef ensures the kernels/pregenerated data works independent of the 
   32-bit architecture where the kernels/data are generated and tested. If
   unsigned int does not equals to four bytes, please modify the following line
   and supply a data type that is four bytes. */
typedef unsigned int uint32;

/* File header contants for id purposes */
static const uint32 Version = 2;
static const uint32 EndianIndicator    = 0xDEADBEEF;
static const uint32 RevEndianIndicator = 0xEFBEADDE;
static const uint32 ComplexIndicator   = 0x2;

/* For internal use */
union FourByteWord {
  unsigned char s[sizeof(uint32)];
  uint32 l;
};

/*************************************************/
/* Structures for Complex int and float */
typedef struct ComplexInt {
  int r, i;
} ComplexInt;

typedef struct ComplexFloat {
  float r, i;
} ComplexFloat;

/*************************************************/
/* C Array structure for int and float */
typedef struct PcaCArrayInt {
  int               *data;                  /* pointer to the start of the memory block */
  void              *datav;                 /* void pointer to a multi-dimensional array */
  unsigned int       size[3];               /* size of each dimension */
  unsigned int       ndims;                 /* number of dimensions for this array */
  unsigned int       rctype;                /* data is real/complex - PCA_REAL/PCA_COMPLEX */
} PcaCArrayInt;

typedef struct PcaCArrayFloat {
  float               *data;
  void                *datav;
  unsigned int         size[3];
  unsigned int         ndims;
  unsigned int         rctype;
} PcaCArrayFloat;

/*************************************************/
/* ptr = pca_malloc ( type, length )
 *  - this is just a wrapper around malloc
 *
 *  Example: The following creates a 1d integer array of 5 elements.
 *
 *    int *ptr = pca_malloc ( int, 5 );
 */
#define pca_malloc(type,s) (s ? (type*)alignedMalloc(s*sizeof(type)) : NULL)

/*************************************************/
/* pca_malloc_1d ( type, ptr, dim1, dtype )
 * pca_malloc_2d ( type, ptr, dim1, dim2, dtype )
 * pca_malloc_3d ( type, ptr, dim1, dim2, dim3, dtype )
 *  - type can be int or float
 *  - ptr points to the start of the memory block after the macro executes
 *  - dim1, dim2, and dim3 specify the size of each dimension
 *  - dtype is to specify whether the array is real or complex
 *
 *  Example: The following example allocates three differnent float arrays.
 *
 *    float *p1, **p2, ***p3;
 *    pca_malloc_1d ( float, p1, 5, PCA_REAL );
 *    pca_malloc_2d ( float, p2, 3, 4, PCA_REAL );
 *    pca_malloc_3d ( float, p3, 3, 4, 5, PCA_COMPLEX );
 */
#define pca_malloc_1d(type,ptr,s,rc) \
{ int i, actual_d2, d2, d1 = 1; \
   type *blk; \
   d2 = s; \
   actual_d2 = (rc == PCA_REAL) ? d2 : d2 * 2; \
   blk = pca_malloc(type, d1*actual_d2); \
   ptr = pca_malloc(type*, d1); \
   if (ptr) for (i=0; i<d1; i++) ptr[i] = blk + i*actual_d2; }

#define pca_malloc_2d(type,ptr,d1,d2,rc) \
{ int i, actual_d2; \
   type *blk; \
   actual_d2 = (rc == PCA_REAL) ? d2 : d2 * 2; \
   blk = pca_malloc(type, d1*actual_d2); \
   ptr = pca_malloc(type*, d1); \
   for (i=0; i<d1; i++) ptr[i] =  blk + i*actual_d2; }

#define pca_malloc_3d(type,ptr,d1,d2,d3,rc) \
{ int i, j, actual_d3; \
   type *blk; \
   actual_d3 = (rc == PCA_REAL) ? d3 : d3 * 2; \
   blk = pca_malloc(type, d1*d2*actual_d3); \
   ptr = pca_malloc(type**, d1); \
   for (i=0; i<d1; i++) ptr[i] = pca_malloc(type*, d2); \
   for (i=0; i<d1; i++) for (j=0; j<d2; j++) ptr[i][j] = blk + actual_d3 * (i*d2+j); }

/*************************************************/
/* pca_create_carray_1d ( type, carray, d1, dtype )
 *  - type can be int or float
 *  - carray can be any of the above PcaCArrayXXXX types
 *  - d1 specifies the length of the array
 *  - dtype specifies whether the array is real or complex
 *
 *  Example: The following example creates a complex float 5x1 array.
 *
 *    struct PcaCArrayFloat pfloat;
 *    pca_create_carray_1d ( float, pfloat, 5, PCA_COMPLEX );
 *
 *  Note: After the above code segment is executed, the struct 
 *        pfloat fields are populated as follows.
 *
 *    pfloat.data  -- pointing to the start of the memory block
 *    pfloat.datav -- same as pfloat.data
 *    pfloat.size  -- [5 1 0]
 *    pfloat.ndim  -- 2
 *    pfloat.dtype -- PCA_COMPLEX
 */
#define pca_create_carray_1d(type,carray,d1,rc) \
{ type **ptr; \
   pca_malloc_1d(type, ptr, d1, rc); \
   carray.data  = ptr ? *ptr : NULL; \
   carray.datav = (void*) ptr; \
   carray.rctype = rc; \
   carray.size[0] = d1; carray.size[1] = 1; carray.size[2] = 0; \
   carray.ndims = 2; }

/*************************************************/
/* pca_create_carray_2d ( type, carray, d1, d2, dtype )
 *  - type can be int or float
 *  - carray can be any of the above PcaCArrayXXXX types
 *  - d1 and d2 specify the size of each dimension
 *  - dtype specifies whether the array is real or complex
 *
 *  Example: The following example creates a real float 3x4 array.
 *
 *    struct PcaCArrayFloat pfloat;
 *    pca_create_carray_2d ( float, pfloat, 3, 4, PCA_REAL );
 *
 *  Note: After the above code segment is executed, the struct 
 *        pfloat fields are populated as follows.
 *
 *    pfloat.data  -- pointing to the start of the memory block
 *    pfloat.datav -- can be used to access the memory as a 2d array
 *                     ((float**)pfloat.datav)[row_index][col_index]
 *    pfloat.size  -- [3 4 0]
 *    pfloat.ndim  -- 2
 *    pfloat.dtype -- PCA_REAL
 */
#define pca_create_carray_2d(type,carray,d1,d2,rc) \
{ type **ptr; \
   pca_malloc_2d(type, ptr, d1, d2, rc); \
   carray.data  = ptr ? *ptr : NULL; \
   carray.datav = (void*) ptr; \
   carray.rctype = rc; \
   carray.size[0] = d1; carray.size[1] = d2; carray.size[2] = 0; \
   carray.ndims = 2; }

/*************************************************/
/*  pca_create_carray_3d ( type, carray, d1, d2, d3, dtype )
 *  - type can be int or float
 *  - carray can be any of the above PcaCArrayXXXX types
 *  - d1, d2, and d3 specify the size of each dimension
 *  - dtype specifies whether the array is real or complex
 *
 *  Example: The following example creates a real float 3x4x5 array.
 *
 *    struct PcaCArrayFloat pfloat;
 *    pca_create_carray_3d ( float, pfloat, 3, 4, 5, PCA_REAL );
 *
 *  Note: After the above code segment is executed, the struct 
 *        pfloat fields are populated as follows.
 *
 *    pfloat.data  -- pointing to the start of the memory block
 *    pfloat.datav -- can be used to access the memory as a 3d array
 *                     ((float***)pfloat.datav)[d1][d2][d3]
 *    pfloat.size  -- [3 4 5]
 *    pfloat.ndim  -- 3
 *    pfloat.dtype -- PCA_REAL
 */
#define pca_create_carray_3d(type,carray,d1,d2,d3,rc) \
{ type ***ptr; \
   pca_malloc_3d(type, ptr, d1, d2, d3, rc); \
   carray.data  = ptr ? **ptr : NULL; \
   carray.datav = (void*) ptr; \
   carray.rctype = rc; \
   carray.size[0] = d1; carray.size[1] = d2; carray.size[2] = d3; \
   carray.ndims = 3; }

/*************************************************/
/* clean_mem ( type, carray )
 *  - type can be int or float
 *  - carray can be any of the above PcaCArrayXXXX type
 *
 *  Example: The following example creates a real float 3x4 array
 *           then clears the memory.
 *
 *    struct PcaCArrayFloat pfloat;
 *    pca_create_carray_2d ( float, pfloat, 3, 4, PCA_REAL );
 *    ...
 *    clean_mem ( float, pfloat );
 *
 *  Note: After the above code segment is executed, the array will
 *        become unusable. Fields will become:
 *
 *    pfloat.data  -- pointing to some unusable memory
 *    pfloat.datav -- pointing to some unusable memory
 *    pfloat.size  -- [0 0 0]
 *    pfloat.ndim  -- 0
 */
#define clean_mem(type,carray) \
{  unsigned int i; \
    alignedFree(carray.data); \
    switch (carray.ndims) { \
      case 2: alignedFree( ((type**)(carray.datav)) ); break; \
      case 3: for(i=0; i<carray.size[0]; i++) alignedFree( ((type***)(carray.datav))[i] ); \
              alignedFree( ((type***)(carray.datav)) ); break; \
    } \
    carray.size[0] = 0; \
    carray.size[1] = 0; \
    carray.size[2] = 0; \
    carray.ndims   = 0; \
    carray.rctype  = 0; }

/*************************************************/
/* swap_endian(type, relt, elt)
 *  - type can be int or float
 *  - relt is the returned data that has the correct endianness 
 *  - elt is the piece of data that requires swapping its endianness
 *
 *  Note: This macro swaps the endianness of data, 
 *        and it is meant strictly for internal use.
 */
#define swap_endian(type, relt, elt) \
{ union { \
     unsigned char s[sizeof(type)]; \
     type          t; \
   } val, uelt; \
   unsigned int x; \
   \
   uelt.t = elt; \
   for (x=0; x<sizeof(type); x++) \
     val.s[x] = uelt.s[sizeof(type)-1-x]; \
   \
   relt = val.t; }

/*************************************************/
/* get_next_word(retval, file, rev_endian)
 *  - retval is the next word in the file stream
 *  - file is current opened file stream for data
 *  - rev_endian is marked if the endianess of the file matches the machine's
 *
 *  Note: This macro returns the next word from an opened file stream, 
 *        and it is meant strictly for internal use.
 */
#define get_next_word(retval, file, rev_endian) \
{ union FourByteWord lw; \
   \
   fread(&lw, sizeof(uint32), 1, file); \
   if (rev_endian) { swap_endian(uint32, retval, lw.l); } \
   else { retval = lw.l; } }

/*************************************************/
/* get_data_set(type, data, len, file, rev_endian)
 *  - type can be int or float
 *  - data points to the memory where the data being read in will be stored
 *  - len specifies the number of elements to be read in from file
 *  - file is current opened file stream for data
 *  - rev_endian is marked if the endianess of the file matches the machine's
 *
 *  Note: This macro returns the data stored in a PCA input file from an 
 *        opened file stream, and it is meant strictly for internal use.
 */
#define get_data_set(type, data, len, file, rev_endian) \
{ unsigned int i, j; \
  char *buff, *buff_it, *ptr; \
  \
  if (!rev_endian) fread(data, sizeof(type), len, file); \
  else { \
    ptr = (char *)data; \
    buff = (char *)malloc(sizeof(type)*len); \
    buff_it = buff; \
    fread(buff_it, sizeof(type), len, file); \
    \
    for (i=0; i<len; i++) { \
      /* swap endian */ \
      for (j=0; j<sizeof(type); j++) { \
        *ptr = buff_it[(sizeof(type)-1)-j]; \
        ptr++; \
      } \
      buff_it+=sizeof(type); \
    } \
    free(buff); \
  } \
}

/*************************************************/
/* readFromFile ( type, filename, carray )
 *  - type can be int or float
 *  - filename is the file which carray will read data from
 *  - carray can be any of the above PcaCArrayXXXX type
 *
 *  Example:
 *
 *    struct PcaCArrayFloat pfloat;
 *    readFromFile ( float, "array_in.dat", pfloat );
 *    ...
 *    clean_mem( float, pfloat );
 *
 *  Note 1: This macro is able to read files that are generated by the
 *          Matlab writeFile function.
 *  Note 2: This macro will automatically allocate memory for
 *          the C array structure, so no malloc or pca_create_carray_xd 
 *          is necessary.  Also, please remember to free the memory 
 *          afterwards.
 */
#define readFromFile(type, filename, carray) \
{ union FourByteWord  hw; \
   FILE *file; \
   int i, rev_endian, dims[3], tlen; \
   unsigned len; \
   uint32 opt_word; \
   \
   file = fopen(filename, "rb"); \
   if (file == NULL) { printf ("Failed opening: %s for reading\n", filename); exit(0); }\
   if (sizeof(uint32) != 4) { printf ("Please make sure the data type 'uint32' is a 4-byte data type in PcaCArray.h\n"); exit(0); } \
   \
   fread(&hw, sizeof(union FourByteWord), 1, file); \
   \
   if (hw.l == EndianIndicator) rev_endian = PCA_FALSE; \
   else if (hw.l == RevEndianIndicator) rev_endian = PCA_TRUE; \
   else { printf ("FILE: %s, UNKNOWN Format\n", filename); exit(0); } \
   \
   get_next_word(opt_word, file, rev_endian); \
   if (opt_word >> 16 != Version) { printf ("File Format Version string ERROR!\n"); exit(0); }\
   \
   carray.rctype = (opt_word & ComplexIndicator) ? PCA_COMPLEX : PCA_REAL; \
   tlen = carray.rctype; \
   \
   get_next_word(carray.ndims, file, rev_endian); \
   for (i=0; i<carray.ndims; i++) { \
     get_next_word(len, file, rev_endian); \
     dims[i] = len; \
     tlen   *= len; \
   } \
   \
   switch (carray.ndims) { \
     case 1: pca_create_carray_1d ( type, carray, dims[0], carray.rctype ); break; \
     case 2: pca_create_carray_2d ( type, carray, dims[0], dims[1], carray.rctype ); break; \
     case 3: pca_create_carray_3d ( type, carray, dims[0], dims[1], dims[2], carray.rctype ); break; \
   } \
   \
   get_data_set(type, carray.data, tlen, file, rev_endian); \
   fclose(file); }

/*************************************************/
/* void writeToFile ( type, filename, carray )
 *  - type can be int or float
 *  - filename is the file which the values of carray will write to.
 *  - carray can be any of the above PcaCArrayXXXX type
 *
 *  Example:
 *
 *    struct PcaCArrayInt pint;
 *    struct ComplexInt*  ptr;
 *
 *    pca_create_carray_2d ( int, pint, 3, 4, PCA_COMPLEX );
 *    ptr = (struct ComplexInt*) (pint.data);
 *
 *    ptr[0].r = 10; ptr[0].i = 20;
 *    ptr[1].r = 30; ptr[1].i = 40;
 *    ptr[2].r = 50; ptr[2].i = 60;
 *    ptr[3].r = 70; ptr[3].i = 80;
 *
 *    writeToFile ( int, "array_out.dat", pint );
 *    ...
 *    clean_mem( int, pint );
 *
 *  Note: The file that is generated with this macro can be read
 *        by the Matlab readFile function.
 */
#define writeToFile(type, filename, carray) \
{ union FourByteWord lw; \
   int i, num_elts; \
   FILE *file; \
   \
   file = fopen(filename, "wb"); \
   if (file == NULL) { printf ("Failed opening: %s for writing\n", filename); exit(0); } \
   if (sizeof(uint32) != 4) { printf ("Please make sure the data type 'uint32' is a 4-byte data type in PcaCArray.h Line 44\n"); exit(0); } \
   \
   lw.l = EndianIndicator; \
   fwrite(&lw, sizeof(uint32), 1, file); \
   \
   lw.l = (Version << 16) + 1; \
   \
   lw.l += (carray.rctype == PCA_COMPLEX) ? ComplexIndicator : 0;  \
   \
   fwrite(&lw, sizeof(uint32), 1, file); \
   \
   lw.l = carray.ndims; \
   fwrite(&lw, sizeof(uint32), 1, file); \
   \
   if (carray.data != NULL) { \
     num_elts = carray.rctype; \
     \
     for (i=0; i<carray.ndims; i++) { \
       lw.l = carray.size[i]; \
       fwrite(&lw, sizeof(uint32), 1, file); \
       \
       num_elts *= lw.l; \
     } \
     \
     fwrite(carray.data, sizeof(type), num_elts, file); \
   } \
   fclose(file); }

#endif
/* ----------------------------------------------------------------------------
Copyright (c) 2006, Massachusetts Institute of Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are  
met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Massachusetts Institute of Technology nor  
       the names of its contributors may be used to endorse or promote 
       products derived from this software without specific prior written 
       permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF  
THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------- */
