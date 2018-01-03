#!/bin/bash

# get pure llvm source codes
./scripts/get_llvm_src_tree.sh
# install HexType source codes
./scripts/install-hextype-files.sh

# build HexType
case "$(uname -s)" in
  Darwin)
  corecount="$(sysctl -n hw.ncpu)"
  ;;
  *)
  corecount="$(`grep '^processor' /proc/cpuinfo|wc -l`)"
  ;;
esac

make -j"$corecount"
