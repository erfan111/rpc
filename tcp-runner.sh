#!/bin/bash
for i in `seq 1 $1`;
do
#./tcp-poisson-send.out -d $3 -p 8888 -r $2 &
./tcp-poisson-send.out -d $3 -p 9001 -r $2 &

#./poisson-send.out -d 10.254.254.1 -p 3000 -r $2 > "c1lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 3000 -r $2 > "c2lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 3000 -r $2 > "c3lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 3000 -r $2 > "c4lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9005 -r $2 > "c5lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9006 -r $2 > "c6lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9007 -r $2 > "c7lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9008 -r $2 > "c8lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9009 -r $2 > "c9lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9010 -r $2 > "c10lient$i.txt" &
#./poisson-send.out -d 10.254.254.1 -p 9011 -r $2 > "c11lient$i.txt" &
done
