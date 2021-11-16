#!/bin/bash -e

#get LLVM
git clone git@github.com:llvm-mirror/llvm.git
pushd llvm
git reset --hard 8f4f26c9c7fec12eb039be6b20313a51417c97bb
popd

#get clang
git clone git@github.com:llvm-mirror/clang.git
pushd clang
git reset --hard d59a142ef50bf041797143db71d2d4777fd32d27
popd

#get compiler-rt
git clone git@github.com:llvm-mirror/compiler-rt.git
pushd compiler-rt
git reset --hard 961e78720a32929d7e4fc13a72d7266d59672c42
popd

#Set up clang, compiler-rt
pushd llvm/tools
ln -s ../../clang .
popd

pushd llvm/projects
ln -s ../../compiler-rt .
popd
