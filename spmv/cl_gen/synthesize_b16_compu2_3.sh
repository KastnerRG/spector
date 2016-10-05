blockdim=16

for comp_u in 2 3
do
	for unroll_f in 1 2 4 8 16 32
	do
            	for vect_w in 1 2 4 8 16
		do
			design_name="spmv_b""$blockdim""_compu""$comp_u""_unrollf""$unroll_f""_vectw""$vect_w"
			source_file_name="$design_name"".cl"
			if [ -f ./$source_file_name ]  
			then
				folder_rpt_name="$design_name""_report"

				if [ ! -f ./$folder_rpt_name/quartus_sh_compile.log ]
				then
					time aoc --board de5net_a7 $source_file_name
							
					mkdir $folder_rpt_name
					cp $design_name/*.txt $folder_rpt_name
					cp $design_name/*.log $folder_rpt_name
					cp $design_name/*.area $folder_rpt_name
					cp $design_name/*.attrib $folder_rpt_name

					rm -r $design_name
				fi
			fi

		done
	done
done
                	
