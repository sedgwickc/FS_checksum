#! /bin/bash
# Charles Sedgwick
# 1684807

NUM=$2
T=$1

echo
echo "Generating test config"
echo "----------------------"
./gen_config.sh

echo
echo "Running FSchecksum $NUM times with $T threads"
echo "---------------------------------------------"
for num in $( seq $NUM ) 
do
	../../FSchecksum -n $T ./usr_bin_sums
done
