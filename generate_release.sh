#!/bin/bash

version=$1

if [ ! ${1} ]; then
echo "enter number version, example v1.0"
else
cd base
echo ${1} > Koriki/version.txt
zip -9 -x '*.gitignore' -x '*.git*' -r ../koriki_${1}_full.zip .
fi
