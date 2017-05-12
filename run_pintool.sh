#!/bin/bash

# make tests
cd programs/bfs/
# make
cd ../lock_add
make
cd ../wild_add
make
cd ../lock_fill_bucket
make
cd ../wild_fill_bucket
make
cd ../reader_writer
make
cd ../../

# enter pintool dir
cd pin-3.2-81205-gcc-linux/source/tools/MyMemTracer/
make

#run pintool
../../../pin -t obj-intel64/TraceFuncsList.so -o lock_add.trace -- ../../../../programs/lock_add/lock_add 16 30
../../../pin -t obj-intel64/TraceFuncsList.so -o wild_add.trace -- ../../../../programs/wild_add/wild_add 16 30
../../../pin -t obj-intel64/TraceFuncsList.so -o wild_fill_bucket.trace -- ../../../../programs/wild_fill_bucket/wild_fill_bucket 16 100 1000
../../../pin -t obj-intel64/TraceFuncsList.so -o lock_fill_bucket.trace -- ../../../../programs/lock_fill_bucket/lock_fill_bucket 16 100 1000
../../../pin -t obj-intel64/TraceFuncsList.so -o reader_writer.trace -- ../../../../programs/reader_writer/reader_writer 16 10000

# copy traces
cp -r *.trace ../../../../traces/

# return to main dir
cd ../../../../

# clean tests
cd programs/bfs/
#make clean
cd ../lock_add
make clean
cd ../wild_add
make clean
cd ../lock_fill_bucket
make clean
cd ../wild_fill_bucket
make clean
cd ../reader_writer
make clean
cd ../../

