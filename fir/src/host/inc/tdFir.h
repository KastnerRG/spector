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
** File: tdFir.h
**
** HPEC Challenge Benchmark Suite
** TDFIR Kernel Benchmark
**
** Contents:
** Desc    : This include file provides declarations for various functions 
**           in support of the generic C TDFIR implementation.
**
** Author: Matthew A. Alexander 
**         MIT Lincoln Laboratory
**
******************************************************************************/

#ifndef TDFIR_H_
#define TDFIR_H_

#include "PcaCArray.h"

struct tdFirVariables{
  PcaCArrayFloat input;
  PcaCArrayFloat filter;
  PcaCArrayFloat result;
  PcaCArrayFloat time;
  int   numFilters;
  int   inputLength;
  int   filterLength;
  int   resultLength;
  int   arguments;
  int   dataSet;
};

void tdFirSetup(struct tdFirVariables *tdFirVars);
void tdFirCPU(struct tdFirVariables *tdFirVars);
void tdFirComplete(struct tdFirVariables *tdFirVars);
void elCplxMul(float *dataPtr, float *filterPtr, 
	       float *resultPtr, int inputLength);
void printVector(float * dataPtr, int inputLength);
void zeroData(float *dataPtr, int length, int filters);

void tdFirVerify(struct tdFirVariables *tdFirVars);
void tdFirVerifyComplete(struct tdFirVariables *tdFirVars);

// FPGA specific routines
void tdFirFPGA(struct tdFirVariables *tdFirVars); 

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
