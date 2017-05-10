#!/usr/bin/python

f = open("MOESIvDragon.trace", "w")

for i in range(1,1001):
    f.write("0 W 0x100\n")
    for j in range(1, 16):
        f.write(str(j) + " R 0x100\n")

f.close()
