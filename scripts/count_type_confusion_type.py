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
        words = line.split()
        if (len(words) > 0):
            allocation.append(words[1])

counter = collections.Counter(allocation)
print(counter.most_common(10))
