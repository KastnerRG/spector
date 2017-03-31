
# Matrix Multiplication

## Description

This code implements a simple matrix multiply A * B = C with squared floating-point matrices. It is based on the Altera OpenCL example of Matrix Multiplication (https://www.altera.com/support/support-resources/design-examples/design-software/opencl/matrix-multiplication.html).
Due to the blocking algorithm used, the size of the matrices need to be divisible by some of the parameters described below. Padding is not implemented right now, but is trivial to add.

### Data structure

The matrices are simply stored in contiguous arrays of float values.

## Usage

* `src` contains the base source code for the host and device, along with a few scripts.
* `scripts` contains the high-level scripts to generate designs, compile and run them.
* `benchmarks` is where the high-level scripts generate the designs.

## Algorithm

This is a typical blocking matrix multiplication algorithm. The resulting matrix C is divided into blocks. Each block is computed by performing multiple small matrix multiplications between blocks of matrix A and blocks of matrix B. The implementation includes several parameters to change the size of the blocks and to process multiple blocks at once.

Note that all the blocks are squared and the block size always divides the matrix size.


```
for each block of matrix C
	for matrix size divided by block size (ie. number of blocks in one row of A or one column of B)
		load one block of A
		load one block of B
		matrix multiplication between block of A and block of B
		add the result to local block of C
	copy the local block of C to global memory
```

## OpenCL kernels

There is only one kernel. Each work-group takes care of one block of C. Each work-item takes care of one element in a block, this includes loading one element from A and from B to local storage, multiplying one row of block A by one column of block B, and copying one element back to global storage.

There are knobs that enable multiple block processing for work-groups and work-items, either by adding another inner loop, or by using OpenCL vector types for SIMD computation. See the knob description below.

### Knobs

- `BLOCKDIM`: Width of blocks.
- `SUBDIM_X`: How many blocks of C in X direction to process in one work-group. This adds a for loop so that each work-item process this number of blocks.
- `SUBDIM_Y`: How many blocks of C in Y direction to process in one work-group.
- `SIMD_X`: How many blocks of C in X direction to process in one work-group. This performs the inner matrix multiply with OpenCL vector types.
- `SIMD_Y`: How many blocks of C in Y direction to process in one work-group.
- `SIMD`: This is the Altera SIMD option for work-items.
- `COMPUTE_UNITS`: Number of compute units.
- `UNROLL_POSITION`: This enables or disable unrolling loop on load and store operations
- `UNROLL_F`: This is the unrolling factor for all the loops with pragmas.

## License information

This benchmark contains code from the Altera OpenCL examples. The Altera license information is included in the OpenCL file.


