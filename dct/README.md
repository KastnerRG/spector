
# Discrete Cosine Transform (DCT)

## Description

This algorithm is based on the NVIDIA CUDA implementation of DCT explained here: http://developer.download.nvidia.com/assets/cuda/files/dct8x8.pdf
This is a 2-dimensional DCT that works on 8x8 blocks. It takes a 2-dimensional signal as input and returns the signal transformed into the frequency domain using 8x8 blocks.

### Data structure

The input signal is given as an array of 32-bits floating-point values organized in a row-major format. The stride between rows is given as input and can be different from the image width. The output signal has the same format.

## Usage

* `cl_gen` contains a python script to generate all the OpenCL designs and bash scripts to compile them.
* `fpga_cpp_gen` contains a python script to generate all the source codes to run each design, and a bash script to compile and run all the programs.

The compiled designs should be copied to (or generated in) the `fpga_cpp_gen`, then everything can run inside this directory.

## Algorithm

The algorithm divides the input signal into 8x8 blocks that are loaded into shared memory. Each 8x8 block is then processed by calculating DCT for rows, then for columns. The DCT coefficients are precalculated, and the DCT algorithm can be either a loop or manually unrolled. There are also parameters that enable multiple blocks to be loaded in shared memory, and multiple rows and columns to be processed simultaneously.

```
for each 8x8 block
	load input values into local memory
	for each row in the block
		calculate DCT
	for each column in the block
		calculate DCT
	store the result in global memory
```


## OpenCL kernel

Each work-group processes one 8x8 block, and within the group each work-item processes one row (and subsequently one column). This behavior can change with some parameters. Two parameters can change the number of blocks that a work-group processes, one parameter can change the number of rows/colums from multiple blocks that a work-item processes, and one parameter changes the number of rows/columns within a block that a work-item processes.

### Knobs

- `COMP_U`: Number of compute units
- `SIMD_WI`: Number of SIMD for work-items
- `BLOCK_SIZE_F`: The number of rows to run DCT on before running DCT on the same number of columns. It needs to be smaller or equal to the number of blocks per work-group in X and Y, and of course it should be a multiple of 8 for the 8x8 DCT algorithm to properly work.
- `BLOCKDIM_X`: The total number of columns per work-group. e.g. 16 means that a work-group processes two 8x8 blocks in the X direction.
- `BLOCKDIM_Y`: Same as previous for the Y direction.
- `SIMD_TYPE`: If 0, each work-item processes multiple consecutive rows/columns using OpenCL vector type. If 1, each work-item processes multiple rows/columns from different blocks using vector types.
- `SIMD_LOC`: Number of rows/columns for one work-item to process using SIMD vecor types.
- `BLOCK_UNROLL`: Unroll factor for the loop iterating over the rows and the loop over columns that launch 8-point DCT.
- `DCT_UNROLL`: If 0, uses the loop version of 8-point DCT. If 1, use the manually unrolled 8-point DCT.

## License information

This benchmark contains code from the NVIDIA OpenCL examples from the CUDA 4.2.9 toolkit. The NVIDIA license information is included in the NVIDIA-License.txt file.


