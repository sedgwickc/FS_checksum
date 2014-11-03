#! /bin/bash

E_SSL="./checksum"
E_SHA="./checksum_popen"
E_MEM="./checksum_memap"

R_SSL="res_ssl"
R_SHA1SUM="res_sha"
R_MEMMAP="res_memmap"

T_TEN="dat_files/ten_mb.dat"
T_TWENTY="dat_files/twenty_mb.dat"
T_EIGHTY="dat_files/eighty_mb.dat"
T_120="dat_files/120_mb.dat"

SUM_SSL=0

# find average run time over 1000 runs
for i in {1..10}
do
	RES=$(/usr/bin/time -f %U $E_SSL $T_TEN)
	SECONDS=0
	$E_SSL $T_TEN
	SUM_SSL=$(echo "$SUM_SSL + $SECONDS" | bc );
	echo $SECONDS ", " $SUM_SSL

done

echo $SUM_SSL
