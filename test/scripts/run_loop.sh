#! /bin/bash
# Charles Sedgwick
# 1684807

NUM=$2
T=$1

echo "Generating test config"
echo "----------------------"
echo
./gen_config.sh

echo "Running FSchecksum $NUM times with $T threads"
echo "----------------------"
echo
for num in $( seq $NUM ) 
do
	../../FSchecksum -n $T ../usr_bin_sums
done
