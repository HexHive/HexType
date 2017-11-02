LLVM_DIR = ${CURDIR}/llvm
BUILD_DIR = ${CURDIR}/build

hextype:
	mkdir -p ${BUILD_DIR}
	(cd ${BUILD_DIR} && \
	  CC=clang CXX=clang++ \
	  cmake \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DCMAKE_C_COMPILER=clang \
	  -DCMAKE_CXX_COMPILER=clang++ \
	  -DLLVM_ENABLE_ASSERTIONS=ON \
	  -DLLVM_BUILD_TESTS=ON \
	  -DLLVM_BUILD_EXAMPLES=ON \
	  -DLLVM_INCLUDE_TESTS=ON \
	  -DLLVM_INCLUDE_EXAMPLES=ON \
	  -DBUILD_SHARED_LIBS=on \
	  -DLLVM_TARGETS_TO_BUILD="X86" \
	  -DCMAKE_C_FLAGS="-fstandalone-debug" \
	  -DCMAKE_CXX_FLAGS="-fstandalone-debug" \
	  ${LLVM_DIR})
	(cd ${BUILD_DIR} && make -j`nproc`)

test:
	(cd ${BUILD_DIR} && make check-clang-hextype -j`nproc`)
	(cd ${BUILD_DIR} && make check-runtime-hextype -j`nproc`)

clean:
	rm -rf ${BUILD_DIR}

.PHONY: hextype test clean

