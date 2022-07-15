#!/bin/bash

DISK="./virtual-disk"
MPOINT="./mount-point"
TEMP="./temp"  

mkdir $TEMP

cp src/fuseLib.c $MPOINT/f1.txt
cp src/fuseLib.c $TEMP/f1.txt
cp src/myFS.h $MPOINT/f2.txt
cp src/myFS.h $TEMP/f2.txt

./my-fsck $DISK
diff $TEMP/f1.txt $MPOINT/f1.txt
diff $TEMP/f2.txt $MPOINT/f2.txt
truncate -o -s -1 $TEMP/f1.txt
truncate -o -s -1 $MPOINT/f1.txt
./my-fsck $DISK
diff $TEMP/f1.txt $MPOINT/f1.txt

cp src/MyFileSystem.c $MPOINT/f3.txt
cp src/MyFileSystem.c $TEMP/f3.txt
./my-fsck $DISK
diff $TEMP/f3.txt $MPOINT/f3.txt

truncate -o -s +1 $TEMP/f2.txt
truncate -o -s +1 $MPOINT/f2.txt
./my-fsck $DISK
diff $TEMP/f2.txt $MPOINT/f2.txt

rm -r $TEMP
