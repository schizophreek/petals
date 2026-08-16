#!/bin/sh
cd "$(dirname "$0")"

chmod +x ./petals_onion ./petals_spruce

if grep -q "sun8i" /proc/cpuinfo; then
    ./petals_spruce > log.txt 2>&1
else
    ./petals_onion > log.txt 2>&1
fi
sync
