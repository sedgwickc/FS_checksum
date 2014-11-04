#! /bin/bash

E_SSL="./checksum"
E_SHA="./checksum_popen"
E_MEM="./checksum_memmap"

R_SSL="res_ssl"
R_SHA1SUM="res_sha"
R_MEMMAP="res_memmap"

FILES=( "dat_files/one_mb.dat" 
"dat_files/five_mb.dat" 
"dat_files/ten_mb.dat" 
"dat_files/twenty_mb.dat" 
"dat_files/forty_mb.dat"
"dat_files/eighty_mb.dat" 
"dat_files/160_mb.dat" )

echo $E_SSL
for file in "${FILES[@]}" 
do
	echo -n "$file: "
	$E_SSL $file
	
done

echo
echo $E_MEM
for file in "${FILES[@]}" 
do
	echo -n "$file: "
	$E_MEM $file
done
