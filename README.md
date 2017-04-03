*Note: The previous version of the histogram data was wrong. It has been replaced with the right data. The other benchmarks are good.*

# Spector

Spector is an OpenCL benchmark suite for FPGA. The particularity of these benchmarks is that each of them is outfitted with a range of optimization parameters (or _knobs_). These _knobs_ can be tuned to create hundreds of unique designs per benchmark.

Each unique design can be compiled and run on FPGA to create design spaces that can be analyzed. We provide the performance and utilization characteristics of thousands of designs to encourage research and improve design space exploration.

## Source codes

We provide the following algorithms as OpenCL benchmarks for FPGA:

* [Breadth-First Search (graph traversal)](bfs)
* [Discrete Cosine Transform](dct)
* [Time-Domain FIR filter](fir)
* [Histogram calculation](histogram)
* [Merge sort](mergesort)
* [Matrix multiplication](mm)
* [Normal estimation](normals)
* [Sobel filter](sobel)
* [Sparse matrix-vector multiplication](spmv)

## Compatibility

* The source codes have been tested under Linux, Ubuntu 14.04.
* The OpenCL codes have been tested on a Terasic DE5 board, compiled with the Altera OpenCL SDK v14.1.
* The scripts to generate the architectures have been tested on Ubuntu 14.04. Note that some of these scripts rely on soft links, which may not be available in some OS.

## Design space data

We compiled thousands of unique designs with the Altera OpenCL SDK, executed them on a Terasic DE5 board, and recorded the performance and area results. These data are available in the [results_and_analysis](results_and_analysis) directory in MATLAB format.

## Paper

```
Spector: An OpenCL FPGA Benchmark Suite
Quentin Gautier, Alric Althoff, Pingfan Meng and Ryan Kastner
```

## License information

This work is released under the BSD 3 license provided in the LICENSE file in this folder.

Exceptions are:

* The OpenCL code of the BFS benchmark is released under the LGPL v2.1 license.
* The OpenCL code and most of the C/C++ code of the SPMV benchmark is released under the LGPL v2.1 license.
* Any unmodified code from an external source retains its original license.

## Acknowledgments

This work was supported in part by an Amazon Web Services Research Education grant.

