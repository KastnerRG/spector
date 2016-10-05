W=1920
H=1080

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
							design_name="sobel_bx""$blockdim_x""_by""$blockdim_y""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u"
							source_file_name="$design_name"".cl"
							folder_rpt_name="$design_name""_report"

							aocx_file_name="$design_name"".aocx"
							
							grep -l "Can't fit design in device" ./$folder_rpt_name/*
							if [ $? -eq 0 ]
							then
								
								echo "$source_file_name" >> not_fit_list.txt
								
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
							design_name="sobel_bx""$blockdim_x""_by""$blockdim_y""_subx""$subdim_x""_suby""$subdim_y""_simdx""$simd_x""_simdy""$simd_y""_simdwi""$simd_wi""_compu""$comp_u"
							source_file_name="$design_name"".cl"
							folder_rpt_name="$design_name""_report"

							aocx_file_name="$design_name"".aocx"
							
							
							grep -l "Can't fit design in device" ./$folder_rpt_name/*
							if [ $? -eq 0 ]
							then
								
								echo "$source_file_name" >> not_fit_list.txt
								
							fi
						fi
					done
				done
			done
		done
	done
done
