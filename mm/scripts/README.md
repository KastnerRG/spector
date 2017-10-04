
This folder contains the scripts used to generate the designs, compile them, and run them.

* Run `1_generate.py` to generate the OpenCL designs.
* Run `2_compile.py` to compile them. WARNING: Uses only one process.
* Run `3_run.py` to generate the host program and run them. You can use `fpga`, `gpu`, `gpu_all`, `cpu`, or `cpu_all` as argument. `gpu_all` will run everything on GPU, whether a AOCX file was generated or not.


