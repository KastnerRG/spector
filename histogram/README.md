
# Histogram

## Description

This code calculates the distribution histogram of unsigned 8-bits values. It takes an arbitrary number of input values in [0..255], and gets back an array of 256 bins.

### Data structure

The code works on one unsigned char array with the input data, and one integer array of size 256 for the output. Additionally when using multiple parallel computation, the implementation can use an intermediate array containing several intermediate histograms.


## Usage

In the `src` directory, you can find the standalone source code to run one specific design.

To generate, compile and run multiple designs, refer to the instructions in the `scripts` directory.


## OpenCL kernels

There is one main kernel, and optionally a second one to process intermediate results.

The first kernel loops over all the input values, and count them into a local histogram. At the end the local histogram is copied into global memory.

If multiple work-items are used, the input data are divided into multiple blocks and each work-item creates a histogram for one of the blocks. Then these histograms can be combined either in shared memory if only one work-group was used, or all the intermediate histograms are copied into global memory, and the second kernel accumulates them into a single output. The second kernel uses a single work-item to loop over all the intermediate histograms and sum them into a local output, then copies the local output to global memory.

The first kernel can also process multiple values at the same time (within one work-item) by keeping several local histograms and accumulating them at the end.

### Knobs

- `KNOB_NUM_HIST`: This is the number of local histograms in the first kernel to compute simultaneously.
- `KNOB_HIST_SIZE`: This changes the size of the local histogram so that it will be stored either in registers or in block RAM
- `KNOB_NUM_WORK_ITEMS`: Number of work-items. This will create intermediate results that need to be accumulated.
- `KNOB_NUM_WORK_GROUPS`: Number of work-groups. If there are multiple work-groups, the intermediate results have to be accumulated in a second kernel (cannot use shared memory).
- `KNOB_COMPUTE_UNITS`: Number of compute units (duplicated kernels)
- `KNOB_ACCUM_SMEM`: Choose to accumulate the intermediate results in shared memory if possible (only one work-group), or in a second kernel.
- `KNOB_UNROLL_FACTOR`: Unrolls the main loop over the input values.

- `KNOB_SIMD`: This knob is not used due to branching


