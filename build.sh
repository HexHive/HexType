#!/bin/bash

# get pure llvm source codes
./scripts/get_llvm_src_tree.sh
# install HexType source codes
./scripts/install-hextype-files.sh

# build HexType
corecount="`grep '^processor' /proc/cpuinfo|wc -l`"
${JOBS="$corecount"}
make -j"$JOBS"
