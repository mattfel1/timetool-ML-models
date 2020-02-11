#!/bin/bash 
# this runs on pskevin

files=()
ctr=0
for i in {1..99999}
do
   files+=("/u1/coffee/2dtimetool_simulation_data/raw/chirp-2000_interferedelay1650_photonen6.0_carriertagdiamond_nfibers109_netalon1_1.00_1.00_fibermap.out.$i")
   files+=("/u1/coffee/2dtimetool_simulation_data/raw/chirp-2000_interferedelay1650_photonen6.0_carriertagdiamond_nfibers109_netalon1_1.00_1.00_interference.out.$i")
   ctr=$((ctr+1))
   if [[ $ctr = 5000 ]]; then
     echo "chunk $i"
     tar -cjf /u1/mattfel/zips/chunk_$i.tar.bz "${files[@]}"
     #bzip2 -c "${files[@]}" > /u1/mattfel/zips/chunk_$i.bz
     files=()
     ctr=0
   fi

done

