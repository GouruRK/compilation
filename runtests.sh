#!/bin/bash

RES=0
NBFILES=0
ACC=0

echo "Starting tests on test/good"

for file in test/good/* ; do
    echo "Test sur $file"
    ./bin/tpcc < $file 2> /dev/null > /dev/null
    ACC=$?
    RES=$(($RES + 1 - $ACC))
    if [ 1 -eq $ACC ]; then
        echo "Test failed on file $file"
    fi
    NBFILES=$(($NBFILES + 1))
done


echo
echo "Starting tests on test/syn-err"

for file in test/syn-err/* ; do
    echo "Test sur $file"
    ./bin/tpcc < $file 2> /dev/null > /dev/null
    ACC=$?
    RES=$(($RES + $ACC))
    if [ 0 -eq $ACC ]; then
        echo "Test failed on file $file"
    fi
    NBFILES=$(($NBFILES + 1))
done

echo
echo "Successfuls tests : $RES/$NBFILES"
