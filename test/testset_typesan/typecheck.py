#!/usr/bin/python

# Usage typechecker.py COMPILER SANITIZER_ARGS
# Example: ./typechecker.py clang++ -fsanitize=hextype
# DO NOT use optimization or similar options, they are added by the script
# Reproduce individual failed test using: COMPILER -O0 -std=c++11 typecheck.cpp -DALLOC_REPORTEDALLOCTYPE -DALLOC_REPORTEDLAYOUTTYPE -DCAST_REPORTEDCASTTYPE SANITIZER_ARGS
# Add -DDO_PASSING for false positives

import sys
import subprocess

# Compile program with given settings
def compile(compiler, compileArgs, allocOption, baseOption, castOption):
    args  = [compiler, "-O0", "-std=c++11", "firstmodule.cpp", "typecheck.cpp", "allocate.cpp", "secondmodule.cpp", "-DALLOC_" + allocOption, "-DBASE_" + baseOption, "-DCAST_" + castOption]
    args.extend(compileArgs)
    subprocess.call(args);

# Check if run fails or not
def runAndCheck():
    try:
        output = subprocess.check_output(["./a.out"], stderr=subprocess.STDOUT);
        # Some mechanisms only write error to output, but terminate successfully
        if output:
            raise subprocess.CalledProcessError(-1, ["./a.out"], subprocess.STDOUT)
    except subprocess.CalledProcessError:
        return True
    return False
    
def testConfiguration(compiler, compileArgs, allocOption, baseOption, castOption, errorFormat):
    formatArgs = dict()
    if allocOption:
       formatArgs["ALLOC"] = allocOption
    else:
        allocOption = "NEW";
    if baseOption:
       formatArgs["BASE"] = baseOption
    else:
        baseOption = "BASIC";
    if castOption:
       formatArgs["CAST"] = castOption
    else:
        castOption = "BASIC";
    errorString = errorFormat.format(**formatArgs);
    passesTest = True
    compile(compiler, compileArgs, allocOption, baseOption, castOption)
    if runAndCheck() == False:
        print errorString;
        args  = [compiler, "-O0", "-std=c++11", "firstmodule.cpp", "typecheck.cpp", "allocate.cpp", "secondmodule.cpp", "-DALLOC_" + allocOption, "-DBASE_" + baseOption, "-DCAST_" + castOption]
        print args
        print compileArgs
        passesTest = False
    compileArgs.append("-DDO_PASSING")
    compile(compiler, compileArgs, allocOption, baseOption, castOption)
    if runAndCheck() == True:
        print errorString, "(false positive)";
        args  = [compiler, "-O0", "-std=c++11", "firstmodule.cpp", "typecheck.cpp", "allocate.cpp", "secondmodule.cpp", "-DALLOC_" + allocOption, "-DBASE_" + baseOption, "-DCAST_" + castOption]
        print args
        print compileArgs
        passesTest = False
    return passesTest

# Options used in typecheck.cpp
allocOptions = ["STACK", "STACK_ARRAY", "STACK_ARRAY_DEEP", "MALLOC", "MALLOC_ARRAY", "MALLOC_VLA", "CALLOC_ARRAY", "CALLOC_VLA", "REALLOC", "REALLOC_ARRAY", "REALLOC_VLA", "NEW", "NEW_ARRAY", "NEW_VLA", "OVERLOADED_NEW", "OVERLOADED_NEW_ARRAY", "OVERLOADED_NEW_VLA", "GLOBAL", "GLOBAL_ARRAY", "GLOBAL_ARRAY_DEEP", "ARGUMENT"]
baseOptions = ["BASIC", "NESTED0", "NESTED", "NESTED_ARRAY", "NESTED_DEEP", "NESTED_ARRAY_DEEP", "INHERITANCE", "VINHERITANCE", "INHERITANCE_MULTI", "VINHERITANCE_MULTI", "INHERITANCE_MULTI_DEEP", "VINHERITANCE_MULTI_DEEP"]
castOptions = ["BASIC", "INHERITANCE_MULTI", "PHANTOM", "PHANTOM_DEEP"]

# Allocation options that support virtual objects
virtualAllocOptions = ["STACK", "STACK_ARRAY", "STACK_ARRAY_DEEP", "NEW", "NEW_ARRAY", "NEW_VLA", "OVERLOADED_NEW", "OVERLOADED_NEW_ARRAY", "OVERLOADED_NEW_VLA", "GLOBAL", "GLOBAL_ARRAY", "GLOBAL_ARRAY_DEEP", "ARGUMENT"]
# Structure layouts using virtual inheritance
virtualBaseOptions = ["VINHERITANCE", "VINHERITANCE_MULTI", "VINHERITANCE_MULTI_DEEP"]

# Check different allocation types with basic options
unhandledAllocOptions = set()
for allocOption in allocOptions:
    if testConfiguration(sys.argv[1], sys.argv[2:], allocOption, None, None, "Allocations of type {ALLOC} not handled") == False:
        unhandledAllocOptions.add(allocOption)

# Check different structure layouts with basic options
unhandledBaseOptions = set()
for baseOption in baseOptions:
    if testConfiguration(sys.argv[1], sys.argv[2:], None, baseOption, None, "Structures with layout {BASE} not handled") == False:
        unhandledBaseOptions.add(baseOption)

# Check different cast types with basic options
unhandledCastOptions = set()        
for castOption in castOptions:
    if testConfiguration(sys.argv[1], sys.argv[2:], None, None, castOption, "Down-cast of type {CAST} not handled") == False:
        unhandledCastOptions.add(castOption)

# Check all combinations of the variants working with basic options
for allocOption in allocOptions:
    if allocOption in unhandledAllocOptions:
        continue
    for baseOption in baseOptions:
        if baseOption in unhandledBaseOptions:
            continue
        # Restrict virtual inheritance checks to the supported allocation types 
        if baseOption in virtualBaseOptions and not allocOption in virtualAllocOptions:
            continue
        if testConfiguration(sys.argv[1], sys.argv[2:], allocOption, baseOption, None, "Combination of allocation type {ALLOC} and structure layout {BASE} not handled") == True:
            for castOption in castOptions:
                if castOption in unhandledCastOptions:
                    continue
                testConfiguration(sys.argv[1], sys.argv[2:], allocOption, baseOption, castOption, "Combination of allocation type {ALLOC}, structure layout {BASE} and down-cast type {CAST} not handled") == True
