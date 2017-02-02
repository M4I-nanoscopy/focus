#
#
#
# Attention:
# This is not an independent csh file.
# This file has to be sourced from another csh.
#
set local_inputstack = $1
set local_samplepixel = $2
set local_SERIALEM_FACTOR = $3
#
echo ":Input stack for dose calculation is ${local_inputstack}"
echo ":SERIALEM_FACTOR is ${local_SERIALEM_FACTOR}"
echo "::Pixel size is ${local_samplepixel} Angstroms"
#
set frame_info_count = ` clip info ${local_inputstack} | grep mean | cut -d\= -f2 `
echo "::Mean frame info number is: ${frame_info_count} counts/pixel" 
#
set FRAMENUM = ` clip info ${local_inputstack} | grep size | cut -d\, -f3 | cut -d\) -f1`
echo "::Frame number is ${FRAMENUM}"
#
set frame_image_dose = ` echo "scale=3; ${frame_info_count} * ${frame_image_dose_factor} / ${local_SERIALEM_FACTOR}" | bc `
echo "::Calculated frame electron count is: ${frame_image_dose} electrons/pixel" 
#
set frame_image_counts = ${frame_image_dose}
#
if ( ${frame_image_dose_source} == "0" ) then
  set frame_image_dose = ${frame_image_dose_manually}
endif
#
set frame_dose = ` echo "scale=3; ${frame_image_dose} / ${local_samplepixel} / ${local_samplepixel}" | bc `
echo "::Calculated frame electron dose  is: ${frame_dose} electrons/A2/frame" 
#
set total_dose = ` echo "scale=3; ${frame_dose} * ${movie_imagenumber_total} " | bc `
echo "::Calculated total electron dose  is: ${total_dose} electrons/A2" 
#

