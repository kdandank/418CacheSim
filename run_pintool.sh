#!/bin/bash

# enter pintool dir
cd pin-3.2-81205-gcc-linux/source/tools/MyMemTracer/

#run pintool
#../../../pin -t obj-intel64/MyMemTracer.so -- ~/private/Documents/15618/collab/assignment3/bfs/bfs /afs/cs/academic/class/15418-s17/public/asst3_graphs/tiny.graph
../../../pin -t obj-intel64/MyMemTracer.so -o lock_add.trace -- ~/private/Documents/15618/collab/final/418CacheSim/programs/lock_add/lock_add 10 30
../../../pin -t obj-intel64/MyMemTracer.so -o wild_add.trace -- ~/private/Documents/15618/collab/final/418CacheSim/programs/wild_add/wild_add 10 30

# copy traces
cp -r *.trace ../../../../traces/

# return to main dir
cd ../../../../

