
This folder contains the scripts used to generate the designs, compile them, and run them on different devices.

* `2_generate_params.py` generates the designs into the "../benchmarks/" folder.
* `3_estimateResults.py` runs the synthesis process (`aoc -c`) on all the designs. Uses multiple threads (number of threads can be given as a parameter).
* `4_parseEstimations.py` parses the synthesis results and determines which design are likely to fit.
* `5_compileFull.py` runs the entire compilation process on all the designs likely to fit. The number of threads can be given as a parameter.
* `6_runPrograms.py` runs all the designs on the FPGA, parses the timing and area data, and output the results in a file.
* `7_runGpu.py` runs all the designs on GPU.


## Note

There might be some issues with including a header file at the top of the OpenCL file. We need to update `2_generate_params.py` to do a similar process as in Merge Sort.


