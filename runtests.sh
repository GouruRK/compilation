#!/bin/bash

RES=0
NBFILES=0
ACC=0

echo "Starting tests on test/good"

for file in test/good/* ; do
    echo "Test sur $file"
    ./bin/tpcc $file 2> /dev/null > /dev/null
    ACC=$?
    if [ $ACC -ne 0 ]; then
        echo "Test failed on file $file"
    else
        RES=$(($RES + 1))
    fi
    NBFILES=$(($NBFILES + 1))
done

echo
echo "Starting tests on test/syn-err"

for file in test/syn-err/* ; do
    echo "Test sur $file"
    ./bin/tpcc $file 2> /dev/null > /dev/null
    ACC=$?
    if [ $ACC -ne 1 ]; then
        echo "Test failed on file $file"
    else
        RES=$(($RES + 1))
    fi
    NBFILES=$(($NBFILES + 1))
done

echo
echo "Starting tests on test/sem-err"

for file in test/sem-err/* ; do
    echo "Test sur $file"
    ./bin/tpcc $file 2> /dev/null > /dev/null
    ACC=$?
    if [ $ACC -ne 2 ]; then
        echo "Test failed on file $file"
    else
        RES=$(($RES + 1))
    fi
    NBFILES=$(($NBFILES + 1))
done

echo
echo "Starting tests on test/warn"

for file in test/warn/* ; do
    echo "Test sur $file"
    ./bin/tpcc $file 2> /dev/null > /dev/null
    ACC=$?
    if [ $ACC -ne 0 ]; then
        echo "Test failed on file $file"
    else
        RES=$(($RES + 1))
    fi
    NBFILES=$(($NBFILES + 1))
done

echo
echo "Successfuls tests : $RES/$NBFILES"
