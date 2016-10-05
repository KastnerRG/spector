
# Merge sort

## Description

This program sorts an array of values by using the merge sort algorithm. We chose the values to be integers in our implementation, but this can be easily changed.

### Structure

The code takes an array of integers to be sorted, and another array of the same size for intermediate storage. Both arrays are modified during the computation, and which one will contain the output depends on the number of values to sort.

## Usage

In the `src` directory, you can find the standalone source code to run one specific design.

To generate, compile and run multiple designs, refer to the instructions in the `scripts` directory.


## Algorithm

The algorithm is implemented using loops instead of recursion. It loops over the input multiple times to merge all the blocks of size 2 first, then double the size of the blocks until merging the last two blocks. The basic implementation of the merge algorithm pulls values from global memory, compares them and store the smallest back to global memory. This is done by alternating the input and output buffers. Optionally, part of the merge sort can be performed in local memory. In this case, a complete merge sort is done on all the blocks of some predefined size. Additionally, if multiple work-items are used, each work-item takes care of merging one block. And if multiple work-groups are used, each one applies the algorithm on one portion of the input array, and the code is called one more time to merge the final output.

### Global memory only
```
for block_size from 2 to input_size
	for each block of the input
		do global merge on block
	swap input/output buffers


global merge on block:
	separate block into 2 halfs
	take 2 values
	compare
	store smallest value
```


### Using local memory
```
for each block of size local_size
	do local merge sort on block
for block_size from local_size to input_size
	for each block of the input
		do global merge on block
	swap input/output buffers

local merge sort on block:
	copy the entire block into local memory
	for local_block_size from 2 to local_size
		for each local block of the block
			do local merge on the local block

local merge on block:
	same as global merge, but works in local memory
	
```


## OpenCL kernels

There is only one kernel for computation, but some features have their separate function to improve readability. The choice between the two implementation described above is enabled by the use of macros controlled by the knobs. One can also activate a specialized piece of code to merge blocks of size two. By default, the kernel always starts merging blocks of size 2, but when using multiple work-groups, the kernel is launched a second time on an array that already has sorted blocks. In this case, we compile a slightly more generalized kernel that can start merging at a specified block size. Note that this disables the sorting in local memory as local sorting always starts with blocks of size 2.


### Knobs
There are 7 knobs with varying values in these kernels.

- `KNOB_WORK_ITEMS`: Number of work-items. Each work-item merges a different block.
- `KNOB_LOCAL_SORT_LOGSIZE`: Log2 of the block size to merge in local memory.
- `KNOB_LOCAL_USE_PTR`: When using local merge sort, either swap pointers for input/output buffers, or copy data from one buffer to the other. Enabling pointers can force the use of block RAM in hardware instead of registers.
- `KNOB_SPECIAL_CASE_1`: Enable a specialized piece of code to merge blocks of size 2 in global memory.
- `KNOB_WORK_GROUPS`: Number of work-groups. This divides the input array so that each work-group applies merge sort on different portions. Then we run a final merge sort to get the final output.
- `KNOB_COMPUTE_UNITS`: Number of compute units when using multiple work-groups.
- `KNOB_UNROLL_LOCAL_COPY`: Unroll factor for the loop copying the data from global to local memory and the copy back. 
