#!/bin/sh
/bin/gzip -1 > /config/coredump.process_$1.gz
sync