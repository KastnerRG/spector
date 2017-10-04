#!/bin/bash

W=1920
H=1080

device="fpga"
run_all=0

# Run with "gpu" or "gpu_all" to run the programs on GPU instead of FPGA

if [ "$1" == "gpu_all" ]
then
	device="gpu"
	run_all=1
elif [ "$1" == "gpu" ]
then
	device="gpu"
elif [ "$1" == "cpu_all" ]
then
	device="cpu"
	run_all=1
elif [ "$1" == "cpu" ]
then
	device="cpu"
fi


> run_results.txt

num_design=0

simd_y=1
subdim_x=1
for comp_u in 1 2 4
do
	for simd_wi in 1 2 4 8
	do
		for simd_x in 1 2 4 8
		do
            		for subdim_y in 1 2 4 8
			do
                		for blockdim_x in 2 4 8 16 32
				do
                    			for blockdim_y in 2 4 8 16 32
					do
						flag0=$(($W%(($blockdim_x*$subdim_x)*$simd_x)))
						flag1=$(($H%(($blockdim_y*$subdim_y)*$simd_y)))
						flag2=$(($blockdim_x%$simd_wi))
						if [ $flag0 -eq 0 ]&&[ $flag1 -eq 0 ]&&[ $flag2 -eq 0 ] 
						then
							HOST_CODE_FILE_NAME="sobel_bx""$blockdim_x""_by""$blockdim_y""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u"
							export MYOPENCL_HOST_CODE_FILE_NAME=$HOST_CODE_FILE_NAME
							aocx_file_name=""
							aocx_file_name+=$HOST_CODE_FILE_NAME
							aocx_file_name+=".aocx"
							if [ -f ./$aocx_file_name ] || [ $run_all -eq 1 ]
							then
								host_program_name=""
								host_program_name+=$HOST_CODE_FILE_NAME
								host_program_name+="_host"
								#num_design+=1
								#echo "design number:" >> run_results.txt
								#echo $num_design >> run_results.txt
								echo $host_program_name >> run_results.txt
								
								make $device

								#run host program
								if [ "$device" == "fpga" ]
								then
									aocl program $aocx_file_name
								fi
							
								./$host_program_name $device >> run_results.txt
							fi
						fi
					done
				done
			done
		done
	done
done


simd_x=1
subdim_y=1
for comp_u in 1 2 4
do
	for simd_wi in 1 2 4 8
	do
		for simd_y in 1 2 4 8
		do
            		for subdim_x in 1 2 4 8 16
			do
                		for blockdim_x in 2 4 8 16 32
				do
                    			for blockdim_y in 2 4 8 16 32
					do
						flag0=$(($W%(($blockdim_x*$subdim_x)*$simd_x)))
						flag1=$(($H%(($blockdim_y*$subdim_y)*$simd_y)))
						flag2=$(($blockdim_x%$simd_wi))
						if [ $flag0 -eq 0 ]&&[ $flag1 -eq 0 ]&&[ $flag2 -eq 0 ]
						then
							HOST_CODE_FILE_NAME="sobel_bx""$blockdim_x""_by""$blockdim_y""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u"
							export MYOPENCL_HOST_CODE_FILE_NAME=$HOST_CODE_FILE_NAME
							aocx_file_name=""
							aocx_file_name+=$HOST_CODE_FILE_NAME
							aocx_file_name+=".aocx"
							if [ -f ./$aocx_file_name ] || [ $run_all -eq 1 ]
							then
								

								host_program_name=""
								host_program_name+=$HOST_CODE_FILE_NAME
								host_program_name+="_host"
								#num_design+=1
								#echo "design number:" >> run_results.txt
								#echo $num_design >> run_results.txt
								echo $host_program_name >> run_results.txt
								
								make $device
								
								#run host program
								if [ "$device" == "fpga" ]
								then
									aocl program $aocx_file_name
								fi
								./$host_program_name $device >> run_results.txt
							fi
						fi
					done
				done
			done
		done
	done
done
