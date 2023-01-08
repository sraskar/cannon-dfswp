#!/bin/bash

# ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile

# 100 * 8 : 32KB per matrix 
#./cannonTp_test  1 15 10 5 2  >> mmTimeStatic.txt

echo "4"
./cannonTP_test 1 1 8 2 4 
./cannonTP_test 1 2 12 3 4 
./cannonTP_test 1 3 16 4 4 
./cannonTP_test 1 4 20 5 4 
./cannonTP_test 1 5 24 6 4 
./cannonTP_test 1 6 28 7 4 
./cannonTP_test 1 7 32 8 4 
./cannonTP_test 1 8 36 9 4 
./cannonTP_test 1 9 40 10 4 
./cannonTP_test 1 10 44 11 4 
./cannonTP_test 1 11 48 12 4 
./cannonTP_test 1 12 52 13 4 
./cannonTP_test 1 13 56 14 4 
./cannonTP_test 1 14 60 15 4 
./cannonTP_test 1 15 64 16 4 
# ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile
echo "8 "
./cannonTP_test 1 1 16 2 8
./cannonTP_test 1 2 24 3 8
./cannonTP_test 1 3 32 4 8
./cannonTP_test 1 4 40 5 8
./cannonTP_test 1 5 48 6 8
./cannonTP_test 1 6 56 7 8
./cannonTP_test 1 7 64 8 8
./cannonTP_test 1 8 72 9 8
./cannonTP_test 1 9 80 10 8
./cannonTP_test 1 10 88 11 8
./cannonTP_test 1 11 96 12 8
./cannonTP_test 1 12 104 13 8
./cannonTP_test 1 13 112 14 8
./cannonTP_test 1 14 120 15 8
./cannonTP_test 1 15 128 16 8
# ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile
echo "16"
./cannonTP_test 1 1 32 2 16
./cannonTP_test 1 2 48 3 16
./cannonTP_test 1 3 64 4 16
./cannonTP_test 1 4 80 5 16
./cannonTP_test 1 5 96 6 16
./cannonTP_test 1 6 112 7 16
./cannonTP_test 1 7 128 8 16
./cannonTP_test 1 8 144 9 16
./cannonTP_test 1 9 160 10 16
./cannonTP_test 1 10 176 11 16
./cannonTP_test 1 11 192 12 16
./cannonTP_test 1 12 208 13 16
./cannonTP_test 1 13 224 14 16
./cannonTP_test 1 14 240 15 16
./cannonTP_test 1 15 256 16 16
# ./CannonTP_test numSU numCU Matsize sqrtNumTiles SqrtNumElementPerTile
echo "32"
./cannonTP_test 1 1 64 2 32
./cannonTP_test 1 2 96 3 32
./cannonTP_test 1 3 128 4 32
./cannonTP_test 1 4 160 5 32
./cannonTP_test 1 5 192 6 32
./cannonTP_test 1 6 224 7 32
./cannonTP_test 1 7 256 8 32
./cannonTP_test 1 8 288 9 32
./cannonTP_test 1 9 320 10 32
./cannonTP_test 1 10 352 11 32
./cannonTP_test 1 11 384 12 32
./cannonTP_test 1 12 416 13 32
./cannonTP_test 1 13 448 14 32
./cannonTP_test 1 14 480 15 32
./cannonTP_test 1 15 512 16 32
