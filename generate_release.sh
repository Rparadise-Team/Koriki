#!/bin/bash

cd base
echo ${1} > Koriki/version.txt
zip -9 -x '*.gitignore' -x '*.git*' -r ../update_koriki_${1}.zip .
rm Koriki/version.txt
