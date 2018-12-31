#!/bin/sh

for i in *
	do
	grep "$@" $i
	if [ $? -eq 0 ]; then
	    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
		echo $i
		echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	fi
done
	
