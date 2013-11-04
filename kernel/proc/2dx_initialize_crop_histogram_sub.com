#
#
# this is not an independent script. It should only be called from other scripts.
#
# This sub-script will crop the histogram, if needed.
#
if ( ${new_mrc_created} == "y" ) then
  #############################################################################
  ${proc_2dx}/linblock "labelh - to calculate image statistics"
  #############################################################################
  #
  set inimage = ${nonmaskimagename}.mrc
  # 
  ${bin_2dx}/labelh.exe << eot
${inimage}
17               ! Get statistics
${crop_histogram_percent}
${crop_histogram_stdev}
eot
  #
  echo " "
  cat < labelh.tmp
  echo " "
  #
  set goodmin = `head -6 labelh.tmp | tail -1`
  set goodmax = `head -7 labelh.tmp | tail -1`
  #
  if ( ${crop_histogram} == 'y' ) then
    #############################################################################
    ${proc_2dx}/linblock "labelh - to cutoff over and underflows to ${goodmin} to ${goodmax}"
    #############################################################################
    #
    #
    \mv -f ${nonmaskimagename}.mrc SCRATCH/${nonmaskimagename}.tmp.mrc
    # 
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.mrc
99               ! Cut off over and underflows
3
${nonmaskimagename}.mrc
${goodmin},${goodmax}
eot
    #
    echo "# IMAGE: "SCRATCH/${nonmaskimagename}.tmp.mrc "<raw image before histogram correction>" >> LOGS/${scriptname}.results
    #
  else
    ${proc_2dx}/lin "Not cropping histogram (Advanced parameter)."
  endif
  #
  if ( ${imageorigin} == '4' ) then
    #############################################################################
    ${proc_2dx}/linblock "LABEL - to reduce pixel amplitude by a factor of 4."
    ${proc_2dx}/linblock "LABEL - to reduce pixel amplitude by a factor of 4." >> History.dat
    #############################################################################  
    #
    \mv -f ${nonmaskimagename}.mrc SCRATCH/${nonmaskimagename}.tmp.mrc
    #
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.mrc
2               ! switch to REAL (floating point)
${nonmaskimagename}.mrc
0.25,0
1
eot
    #
  endif
  #
  if ( ${imageorigin} == '5' ) then
    #############################################################################
    ${proc_2dx}/linblock "LABEL - to produce MODE=1 INTEGER*2 image with autoscaling 0...16k."
    ${proc_2dx}/linblock "LABEL - to produce MODE=1 INTEGER*2 image with autoscaling 0...16k." >> History.dat
    #############################################################################  
    #
    \mv -f ${nonmaskimagename}.mrc SCRATCH/${nonmaskimagename}.tmp.mrc
    #
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.mrc
16               ! switch to INTEGER*2 (MODE 1) with autoscaling 0…16000
${nonmaskimagename}.mrc
eot
    #
  endif
  #
  if ( ${imageorigin} == '6' ) then
    #############################################################################
    ${proc_2dx}/linblock "LABEL - to produce MODE=1 with unsigned/signed swap and autoscaling 0...16k."
    ${proc_2dx}/linblock "LABEL - to produce MODE=1 with unsigned/signed swap and autoscaling 0...16k." >> History.dat
    #############################################################################  
    #
    \mv -f ${nonmaskimagename}.mrc SCRATCH/${nonmaskimagename}.tmp.mrc
    #
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.mrc
18               ! switch to INTEGER*2 (MODE 1) with unsigned/signed swap and autoscaling 0…16000
${nonmaskimagename}.mrc
eot
    #
  endif
  #
    if ( ${imageorigin} == '7' ) then
    #############################################################################
    ${proc_2dx}/linblock "LABEL - to produce MODE=2 and autoscaling 0...16k, and rotating 90deg."
    ${proc_2dx}/linblock "LABEL - to produce MODE=2 and autoscaling 0...16k, and rotating 90deg." >> History.dat
    #############################################################################  
    #
    \mv -f ${nonmaskimagename}.mrc SCRATCH/${nonmaskimagename}.tmp.mrc
    #
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.mrc
19               ! switch to REAL (MODE 2) with autoscaling 0…16000
SCRATCH/${nonmaskimagename}.tmp.2.mrc
eot
    #
    ${bin_2dx}/labelh.exe << eot
SCRATCH/${nonmaskimagename}.tmp.2.mrc
99               ! further modes
1		 ! various roations
${nonmaskimagename}.mrc
1		 ! Z90 rotation
eot
    #
  endif
  #
endif
#