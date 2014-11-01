# Charles Sedgwick
# This script tests the output of FSchecksum_debug for output with more lines
# than expected. The number of lines expected is 2 * BUFF_SIZE since for every
# element in the buffer it will output its creation and its removal. 
# The unexpected output is printed to file unexp_out.log.

#! /bin/bash

i=$(($1*2))

while [ $i -eq $(($1*2)) ]
do 
	i=$(./FSchecksum_debug -n $1 test/chksums | tee unexp_out.log | wc -l)
done

echo $i

