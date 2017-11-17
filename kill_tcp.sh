#!/bin/bash

for i in `pidof tcp-poisson-send2.out`
do
echo $i
kill -9 $i
done
cat c0lient* | awk '{print $2}' >> "final_nictonic.txt"
cat c0lient* | awk '{print $1}' >> "final_normal.txt"
rm c0lient*
