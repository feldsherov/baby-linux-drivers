#!/bin/env bash
set -ex

# setup clean up on exit
clean_up () {
    ARG=$?
    echo "> clean_up"
    rm -rf ./test_node
    exit $ARG
} 
trap clean_up EXIT

major=$(cat /proc/devices | grep rw_cdevice | awk '{ print $1 }' )

if [ -z ${major} ] 
then
    echo Fail to get major device number.
    echo Check that rw_cdevice module is inserted.
    exit -1
fi

mknod ./test_node c ${major} 0

echo test_text > ./test_node
cat ./test_node
