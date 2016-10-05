
# Sparse matrix-vector multiplication (spmv)

## Description

This code performs a simple matrix-vector multiplication and addition y = Ax + y where the matrix A is sparse and the vectors x and y are dense. It is based on the Sparse Matrix-Vector Multiplication FPGA benchmark from the OpenDwarfs project (https://github.com/vtsynergy/OpenDwarfs).

### Data structure

The matrix is stored in the CSR format, with one array for the non-zero elements (floating-point) _Ax_, one array for the cumulated number of elements per row _Ap_, and one array for the column indices for each element _Aj_.

The vectors _x_ and _y_ are simple contiguous arrays of floating-point values.

## Usage

* `cl_gen` contains a python script to generate all the OpenCL designs, and bash scripts to compile them with AOCL.
* `host_gen_FPGA` contains a python script to generate the source code that runs each design.
* `csr_gen` contains a program to generate matrices in CSR format.
* `FPGA_run` contains files necessary to run the designs, including a bash script that compiles and runs all the programs.

All the generated designs and source code, and all the data, Makefile and scripts should be in the same directory.

## Algorithm

The algorithm loops over each row, gets the indices of the elements of this row, the gets the column of each element and multiplies it by the corresponding vector's row. There are some parameters to enable the processing of multiple elements simultaneously.

```
for each row
	initialize sum with y[row]
	get the indices of elements in the row from Ap
	for each element
		get the column index j of this element from Aj
		get x[j] and multiply by this element
		add result to sum
	store the sum back into y
```

## OpenCL kernels

One kernel takes care of the computation. Each work-item processes one row (so this replaces the outer for loop). Two parameters control the number of elements processed simultaneously by one work-item. One parameter simply unrolls a loop, and the other enable the use of OpenCL vector types to store data and perform the multiplication as SIMD.

### Knobs

- `BLOCKDIM`: The number of work-items per work-group
- `COMP_U`: The number of compute units
- `UNROLL_F`: This creates an inner loop over some number of elements and unrolls it so that elements can be processed simultaneously
- `VECT_WIDTH`: This is the size of the OpenCL vector type to use when processing elements. Elements are loaded and multiplied in parallel using this type.


## License information

Most of this benchmark is based on the OpenDwarfs benchmark released under the LGPLv2.1 license. As such, the OpenCL code, the CSR matrix generation code, and all the C/C++ helper codes are release under the LGPLv2.1. The LGPL license is provided in the LICENSE file in this folder.

The rest of the code is release under the BSD 3 license.

