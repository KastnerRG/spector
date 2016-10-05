
# Normal estimation

## Description

This code is inspired by some pre-processing algorithms in Kinfu, the open-source implementation of KinectFusion from the PCL library (http://pointclouds.org/).

It estimates the 3D normals for an organized point cloud. A device such as Microsoft Kinect or Intel RealSense provides a 2D map of distances (depth map) that we can convert into a point cloud. This point cloud is organized since we can estimate neighbors of a 3D point by using neighbors on the 2D map. By using this neighbor information, we can quickly estimate the 3D normals for all the points on the depth map.

### File format

The input is a simple binary file containing a vertex map of size 640x480 stored using 32-bits floats in row-major format. A vertex map is a depth map for which each value has been converted to a 3D coordinate in real distances. The coordinates are interleaved, ie. (x1,y1,z1) (x2,y2,z2) ...

The file containing the normals for verification follows exactly the same format.

### Data structure

The vertex map is stored in a flat array of floats in row-major format with interleaved coordinates. The normal map follows the same format.

## Usage

In the `src` directory, you can find the standalone source code to run one specific design.

To generate, compile and run multiple designs, refer to the instructions in the `scripts` directory.



## Algorithm

For each point on the vertex map, we take this point, the point to the right and the point below (with respect to the 2D map). Then we calculate the cross-product of the difference between each neighbor and the current point. The result is normalized and stored in the normal map. If any of the vertices is null (NaN), the normal is null. The last column and the last row of the normal map are set to null.

```
for each vertex point on the map
	if vertex is on last row or last column
		set normal to null
	else
		get the neighbor on the right and below
		if any of the vertices are null
			set the normal to null
		else
			calculate the difference between each neighbor and the current point
			calculate the cross-product between these differences
			normalize the result
			store the normal into the normal map
```

## OpenCL kernels

One kernel does the entire computation and implements the whole algorithm above. If multiple work-items are used, the input data are cut along rows so that each work-item processes a block of whole rows. The input data are fetched using a small sliding window where the right neighbor is shifted at each iteration to be reused as the current vertex in the next iteration. A parameter can vary the window size so that multiple inputs are processed in one iteration.

A second design uses the work-groups to divide the input data, and the work-items are used to process each element of the sliding window simultaneously. In the current version, this design does not seem to work on FPGA.

### Knobs

- `KNOB_WORK_ITEMS`: Number of work-items. Will divide the input data (design 0) or process consecutive elements simultaneously (design 1).
- `KNOB_WORK_GROUPS`: Number of work-groups. Will divide the input data.
- `KNOB_COMPUTE_UNITS`: Number of compute units.
- `KNOB_UNROLL`: Unroll factor for the outer loop that iterates over all the input data.
- `KNOB_UNROLL_INNER`: Design 0 only. Unroll factor for the loop that iterates over the elements within a sliding window.
- `KNOB_WINDOW_SIZE_X`: Size of the sliding window. ie. Number of consecutive elements to potentially process simultaneously.
- `KNOB_DESIGN_TYPE`: [1 not working on FPGA] Design 0 - work-items and work-groups simply process different blocks of input data. Design 1 - work-items process consecutive elements within the sliding window.

