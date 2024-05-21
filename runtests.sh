#!/bin/bash

RES=0
NBFILES=0
ACC=0

sources=(test)
folders=(good sem-err syn-err warn)
rvalues=(0 2 1 0)

run() {
    echo "Starting tests on $1"

    for file in $1/$2/* ; do
        echo "Test sur $file"
        ./bin/tpcc $file 2> /dev/null > /dev/null
        ACC=$?
        if [ $ACC -ne $3 ]; then
            echo "Test failed on file $file"
        else
            RES=$(($RES + 1))
        fi
        NBFILES=$(($NBFILES + 1))
    done
    echo 
}


for src in ${sources[@]}; do
    for i in "${!folders[@]}"; do
        run $src ${folders[$i]} ${rvalues[$i]}
    done
done

echo "Successfuls tests : $RES/$NBFILES"
