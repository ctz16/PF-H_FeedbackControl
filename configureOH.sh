#!/bin/bash
cat configOH.txt | nc 192.168.1.31 10001
echo 'OH coefficient set'
