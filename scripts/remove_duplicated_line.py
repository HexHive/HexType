import sys
import os
import collections

class AllocationInfo:
    objName = ""

filepath = sys.argv[1]
files = []

allocation = []
alloccount = 0

with open(filepath, 'r') as f:
    data = f.readlines()
    for line in data:
        if line not in allocation:
            allocation.append(line)
            print line
