#!/bin/bash

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
fi



M=1024

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
				for blockdim in 2 4 8 16 32
				do
					for unroll_f in 2 4 8 16 32
					do
						for unroll_sel in 1 2
						do
							flag0=$(($M%(($blockdim*$subdim_x)*$simd_x)))
							flag1=$(($M%(($blockdim*$subdim_y)*$simd_y)))
							flag2=$(($blockdim%$simd_wi))

							if [ $unroll_sel -eq 1 ]
							then
								flag3=$(($blockdim%$unroll_f))
								HOST_CODE_FILE_NAME="mm_b""$blockdim""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u""_unrollb""$unroll_f"

							else
								flag3=$(($subdim_y%$unroll_f))
								HOST_CODE_FILE_NAME="mm_b""$blockdim""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u""_unrollp""$unroll_f"
							fi

							tmp_par=$(($unroll_f*$subdim_x*$subdim_y*$simd_x*$simd_y*$simd_wi*$comp_u))
							if [ $tmp_par -gt 256 ]
							then
								flag4=1

							else
								flag4=0
							fi


							tmp_loc=$(($blockdim*$blockdim*$subdim_x*$subdim_y*$simd_x*$simd_y))
							if [ $tmp_loc -gt 1024 ]
							then
								flag5=1

							else
								flag5=0
							fi

							if [ $flag0 -eq 0 ]&&[ $flag1 -eq 0 ]&&[ $flag2 -eq 0 ]&&[ $flag3 -eq 0 ]&&[ $flag4 -eq 0 ]&&[ $flag5 -eq 0 ]
							then

								export MYOPENCL_HOST_CODE_FILE_NAME=$HOST_CODE_FILE_NAME

								aocx_file_name=""
								aocx_file_name+=$HOST_CODE_FILE_NAME
								aocx_file_name+=".aocx"
								if [ -f ./$aocx_file_name ] || [ $run_all -eq 1 ]
								then


									host_program_name=""
									host_program_name+=$HOST_CODE_FILE_NAME
									host_program_name+="_host"
									num_design=$(($num_design+1))
									echo "design number:" >> run_results.txt
									echo $num_design >> run_results.txt
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
done




simd_x=1
subdim_y=1
for comp_u in 1 2 4
do
	for simd_wi in 1 2 4 8
	do
		for simd_y in 1 2 4 8
		do
			for subdim_x in 1 2 4 8
			do
				for blockdim in 2 4 8 16 32
				do
					for unroll_f in 2 4 8 16 32
					do
						for unroll_sel in 1 2
						do
							flag0=$(($M%(($blockdim*$subdim_x)*$simd_x)))
							flag1=$(($M%(($blockdim*$subdim_y)*$simd_y)))
							flag2=$(($blockdim%$simd_wi))

							if [ $unroll_sel -eq 1 ]
							then
								flag3=$(($blockdim%$unroll_f))
								HOST_CODE_FILE_NAME="mm_b""$blockdim""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u""_unrollb""$unroll_f"

							else
								flag3=$(($subdim_x%$unroll_f))
								HOST_CODE_FILE_NAME="mm_b""$blockdim""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u""_unrollp""$unroll_f"
							fi

							tmp_par=$(($unroll_f*$subdim_x*$subdim_y*$simd_x*$simd_y*$simd_wi*$comp_u))
							if [ $tmp_par -gt 256 ]
							then
								flag4=1

							else
								flag4=0
							fi

							tmp_loc=$(($blockdim*$blockdim*$subdim_x*$subdim_y*$simd_x*$simd_y))
							if [ $tmp_loc -gt 1024 ]
							then
								flag5=1

							else
								flag5=0
							fi

							if [ $flag0 -eq 0 ]&&[ $flag1 -eq 0 ]&&[ $flag2 -eq 0 ]&&[ $flag3 -eq 0 ]&&[ $flag4 -eq 0 ]&&[ $flag5 -eq 0 ]
							then

								export MYOPENCL_HOST_CODE_FILE_NAME=$HOST_CODE_FILE_NAME

								aocx_file_name=""
								aocx_file_name+=$HOST_CODE_FILE_NAME
								aocx_file_name+=".aocx"
								if [ -f ./$aocx_file_name ] || [ $run_all -eq 1 ]
								then


									host_program_name=""
									host_program_name+=$HOST_CODE_FILE_NAME
									host_program_name+="_host"
									num_design=$(($num_design+1))
									echo "design number:" >> run_results.txt
									echo $num_design >> run_results.txt
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
done


