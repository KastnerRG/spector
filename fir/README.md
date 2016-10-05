
# Complex FIR filter

## Description

This benchmark is based on the Altera OpenCL design example of a Time-Domain FIR Filter (https://www.altera.com/support/support-resources/design-examples/design-software/opencl/td-fir.html). The Altera example is itself based on the HPEC Challenge Benchmark suite (http://www.omgwiki.org/hpec/files/hpec-challenge/tdfir.html).

This code implements a complex single-precision floating point filter, where multiple filters are applied to a stream of input data.

### Data format

The input data are stored in a contiguous array of two floating point values (real, imaginary).
The filters are stored similarly in a contiguous array of two float values, and each filter follows the previous one.
There is an output array that matches exactly the input array.


## Usage

In the `src` directory, you can find the standalone source code to run one specific design.

To generate, compile and run multiple designs, refer to the instructions in the `scripts` directory.


## Algorithm

The algorithm is a simple sliding window that applies the filter coefficients to the input data. Every time a block of input data has been processed, it loads the next filter's coefficients and continue the sliding window.
To avoid too much branching complexity in the kernel, the sliding window continues to shift while the algorithm loads a new filter. This method requires the input data to be padded by the number of iterations it takes to load a filter. This process is detailed in the Altera FIR optimization guide.

```
for each input value
	shift input data in sliding window
	if need to load new filter
		for number of coefficients to load in one iteration
			shift filter coefficients
			load new coefficients
	for each filter coefficient
		perform FIR computation with input data
	write result to output
	if number of processed input equals input block length
		set a flag to load new coefficients at next iteration
```

## OpenCL kernel

There is only one kernel that runs for the entire input data. Originally the kernel is a single work-item task, but it has been extended to perform multiple computations simultaneously.
The kernel contains the main for loop described above, and all the inner operations have been parameterized.
It is possible to launch the kernel with multiple work-items where each work-item processes a different filter.

The length of the filter is set at compile-time, such that one design only works for a specific filter size.

### Knobs


- `KNOB_COEF_SHIFT`: The number of filter coefficients to load at each iteration
- `KNOB_NUM_PARALLEL`: The number of FIR computations to perform in a single iteration. The size of the sliding window is extended by this number.
- `KNOB_UNROLL_FILTER_3`: Unroll factor for the FIR computation loop (for each coefficient)
- `KNOB_UNROLL_TOTAL`: Unroll factor for the main loop (for each input value)
- `KNOB_NUM_WORK_ITEMS`: Number of work-items. This divides the input data into multiple blocks, each work-item works on one block.
- `KNOB_NUM_WORK_GROUPS`: Number of work groups, same consequences as work-items.
- `KNOB_SIMD`: Number of work-items in a work-group to group into a SIMD computation.
- `KNOB_COMPUTE_UNITS`: Number of duplicated kernels that can run in parallel (upper bound is number of work-groups).

The following knobs are all unroll factors for various loops in the kernel, and have all been set to full unroll:

- `KNOB_UNROLL_FILTER_1`      
- `KNOB_UNROLL_FILTER_2`      
- `KNOB_UNROLL_COEF_SHIFT_1` 
- `KNOB_UNROLL_COEF_SHIFT_2` 
- `KNOB_UNROLL_PARALLEL_1`    
- `KNOB_UNROLL_PARALLEL_2`    
- `KNOB_UNROLL_PARALLEL_3`   

## License information

This benchmark is based on an Altera OpenCL example. All original Altera code retains its license, the modifications are released under BSD 3.

