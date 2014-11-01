#! /bin/bash

E_SSL="./checksum"
E_SHA="./checksum_popen"

R_SSL="res_ssl"
R_SHA1SUM="res_sha"
R_MEMMAP="res_memmap"

T_TEN="ten_mb.dat"
T_TWENTY="twenty_mb.dat"
T_EIGHTY="eighty_mb.dat"
T_120="120_mb.dat"

SUM_SSL=0

# find average run time over 1000 runs
for i in {1..10}
do
	RES=$(/usr/bin/time -f %U $E_SSL $T_TEN)
	SUM_SSL=$((echo 'scale=10;$SUM_SSL+$RES' | bc))
	echo $RES

done

echo $SUM_SSL
