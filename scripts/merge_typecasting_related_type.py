import sys
import os

class AllocationInfo:
    objName = ""

filepath = sys.argv[1]
files = []

allocation = []
alloccount = 0

for i in os.listdir(filepath):
    if os.path.isfile(os.path.join(filepath,i)) and 'casting_obj' in i:
        with open(i, 'r') as f:
            data = f.readlines()
            for line in data:
                words = line.split()
        	if (len(words) > 0):
        	    tmpObj = words[0]
        	    str2 = ".";
	            if (tmpObj.find(str2) != -1):
        		tests = tmpObj.split(".");
        	    	tmpObj = tests[0]
        	    dupilcated = 0
        	    for x in xrange(0, alloccount):
	                if(allocation[x] == tmpObj):
        	 	    dupilcated = 1
	        	    break
        	    if(dupilcated == 0):
        		allocation.append(tmpObj)
	                alloccount += 1
for x in sorted(allocation):
    print x
