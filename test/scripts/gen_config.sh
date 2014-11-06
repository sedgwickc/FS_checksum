#! /bin/bash

for file in /usr/bin/*
do
	sha1sum $file >> ./usr_bin_sums
done
