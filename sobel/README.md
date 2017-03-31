
# Sobel filter

## Description

This code applies a Sobel filter on an input image, which size is defined at compile-time. It applies the 3x3 filter on X and Y, combines the results and applies a threshold to give back a b&w image of the contours.
It is based on the Altera OpenCL example of a Sobel filter (https://www.altera.com/support/support-resources/design-examples/design-software/opencl/sobel-filter.html).

### Data structure

The input image is an array of unsigned integers, each representing a pack of 8-bits (r,g,b,~) values. The output is an array of unsigned integers containing 0 or 1 to denote a contour.

## Usage

* `src` contains the base source code for the host and device, along with a few scripts.
* `scripts` contains the high-level scripts to generate designs, compile and run them.
* `benchmarks` is where the high-level scripts generate the designs.

## Algorithm

Basically, for each pixel of the input image, the algorithm takes the 8 pixels around, converts them all to grayscale, applies a 3x3 kernel for X and another kernel for Y, combines the results and write this to the output. To improve efficiency, all the pixels from a block are loaded into a shared buffer so that the values can be reused by multiple workers. Also, some parameters enable a sliding window within this block, or enable SIMD computation.

```
for each block on the input image
	load the pixel values from this block in local storage
	for each pixel in the local storage
		load the 8 pixels around from local storage to registers
		apply the 3x3 filter in X and Y
		combine the X and Y results and apply a threashold
		save the result in global storage
```

## OpenCL kernel

There is one kernel performing the entire computation. Each work-group takes care of one block of the image. Each work-item takes care of one pixel within the block.

There are parameters that can make a work-item perform multiple computations.

With the SIMD parameter, each work-item applies the filter on multiple pixels at once by using OpenCL vector types, and by loading more pixels in the registers.

The sliding window parameter creates an inner loop where each work-item processes one pixel (or multiple with SIMD), then shifts the registers to load one new row or column of data from the local storage. It changes the algorithm above like this:
```
	for each pixel in the local storage
		load the 8 pixels around from local storage to registers
		for sliding window size
			apply the 3x3 filter in X and Y
			combine the X and Y results and apply a threashold
			save the result in global storage
			shift the registers and load new row/column from local storage
```

### Knobs

- `BLOCKDIM_X`: Size of each block in X
- `BLOCKDIM_Y`: Size of each block in Y
- `SUBDIM_X`: Local sliding window size, moving in X direction
- `SUBDIM_Y`: Local sliding window size, moving in Y direction
- `SIMD_X`: Number of elements to process as SIMD in X direction
- `SIMD_Y`: Number of elements to process as SIMD in Y direction
- `SIMD`: Altera SIMD for work-items
- `COMPUTE_UNITS`: Number of compute units

## License information

This benchmark contains code from the Altera OpenCL examples. The Altera license information is included in the OpenCL file.



