M=1024

simd_x=1
subdim_y=1

simd_y=8
for comp_u in 1 2 4
do
	for simd_wi in 1 2 4 8
	do
		#for simd_y in 1 2 4 8
		#do
            	for subdim_x in 1 2 4 8
		do
                	for blockdim in 2 4 8 16 32
			do
                    		for unroll_f in 2 4 8 16 32
				do
					flag0=$(($M%(($blockdim*$subdim_x)*$simd_x)))
					flag1=$(($M%(($blockdim*$subdim_y)*$simd_y)))
					flag2=$(($blockdim%$simd_wi))
					flag3=$(($blockdim%$unroll_f))
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


					tmp_par2=$(($unroll_f*$simd_x*$simd_y*$simd_wi*$comp_u))
					if [ $tmp_par2 -gt 128 ]
					then
						flag6=1
								
					else
						flag6=0
					fi

					if [ $flag0 -eq 0 ]&&[ $flag1 -eq 0 ]&&[ $flag2 -eq 0 ]&&[ $flag3 -eq 0 ]&&[ $flag4 -eq 0 ]&&[ $flag5 -eq 0 ]&&[ $flag6 -eq 0 ]  
					then
						design_name="mm_b""$blockdim""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u""_unrollb""$unroll_f"
						source_file_name="$design_name"".cl"

						folder_rpt_name="$design_name""_report"

						if [ ! -f ./$folder_rpt_name/quartus_sh_compile.log ]
						then
							time aoc --board de5net_a7 $source_file_name

							
							mkdir $folder_rpt_name
							cp $design_name/*.txt $folder_rpt_name
							cp $design_name/*.log $folder_rpt_name

							rm -r $design_name
						fi
					fi
				done
			done
		done
		#done
	done
done
