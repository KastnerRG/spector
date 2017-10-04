M=1024

num_design=0

echo "" > run_results.txt

simd_y=1
subdim_x=1
for comp_u in 1 2
do
	for simd_wi in 1 2 4 8
	do
		for dct_unroll in 0 1
		do
			for block_unroll in 0 1
			do
				for block_size_f in 1 2 4
				do
					for simd_loc in 1 2 4 8
					do
						for simd_type in 0 1
						do
							for blockdim_x in 8 16 32 64
							do

								for blockdim_y in 8 16 32 64
								do
									HOST_CODE_FILE_NAME="dct_bx""$blockdim_x""_by""$blockdim_y""_simdType""$simd_type""_simdLoc""$simd_loc""_blockSizeF""$block_size_f""_blockU""$block_unroll""_DCTU""$dct_unroll""_simdwi""$simd_wi""_compu""$comp_u"


									export MYOPENCL_HOST_CODE_FILE_NAME=$HOST_CODE_FILE_NAME

									aocx_file_name=""
									aocx_file_name+=$HOST_CODE_FILE_NAME
									aocx_file_name+=".aocx"
									if [ -f ./$aocx_file_name ]
									then


										host_program_name=""
										host_program_name+=$HOST_CODE_FILE_NAME
										host_program_name+="_host"
										num_design=$(($num_design+1))
										echo "design number:" >> run_results.txt
										echo $num_design >> run_results.txt
										echo $host_program_name >> run_results.txt
										make fpga
										#run host program
										aocl program $aocx_file_name
										./$host_program_name fpga 5 >> run_results.txt
									fi
								done
							done			
						done
					done
				done
			done
		done
	done
done






