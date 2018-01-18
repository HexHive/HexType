## HexType

### How to build

a. Download HexType source code
```
git clone git@github.com:HexHive/HexType.git
```

b. Build HexType
```
run ./build.sh
```

### How to use

a. Set HexType path
```
$ $BUILD_DIR/bin/export HEXTYPE_LOG_PATH="/tmp/hextype"
```

b. Run HexType
- Use the `create-clang-typeinfo` option or copy the pre-made type info (in the `HexType/etc/clang_type_info`) into the HexType path as `typeinfo.txt` file name
- Use `HexType/scripts/remove_duplicated_line.py` in order to remove duplicated lines when you create new type information file
```
$ $BUILD_DIR/bin/clang++ test.cc -fsanitize=hextype
```
- Note that during various application tests, we found in a few cases that some type information was omitted during the transition from source code to LLVM IR (which can cause false positive issue). In order to solve this issue, HexType also relies on clang level type information using `-mllvm -create-clang-typeinfo` option.

c. Please enable/disable below `#define` in the `lib/hextype/rbtree.h` according to your purpose  
```
HEX_LOG: print numerical statistics
PRINT_BAD_CASTING: print type confusion result
PRINT_BAD_CASTING_FILE: print type confusio result into file
```

d. Please use below additional options as compile option (with `-mllvm` option, e.g., `-mllvm -statck-opt`) according to your purpose

- Coverage

```
handle-reinterpret-cast : handle reinterpret_cast to increase coverage
handle-placement-new : handle placement_new to increase coverage
```

- Optimization
  - If you use the `cast-obj-opt` option, create a type casting related object list using `create-cast-related-type-list` option or copy the pre-made list (in the `HexType/etc/typecasting_releated_type_rule`) into the HexType path as `casting_obj.txt` file name
  - Use `HexType/scripts/merge_typecasting_related_type.py` in order to merge type information when you create new type casting related set
```
stack-opt : apply stack optimization
safestack-opt : apply stack optimization using safestack
cast-obj-opt : apply only typecasting relate objects tracing optimization
inline-opt : apply inline optimization
compile-time-verify-opt : apply compile time verification optimization
enhance-dynamic-cast : replace dynamic_cast`s type casting verification function
```

- Etc

```
make-loginfo : collect and print numerical statistics
make-typeinfo : print type and hash information
create-cast-releated-type-list : create typecasting related object list
create-clang-typeinfo : create clang level type info
```

e. HexType`s major changes

- Clang

```
- Handle typecasting (static_cast and C style) and reinterpret_cast
llvm/tools/clang/lib/CodeGen/CGClass.cpp
llvm/tools/clang/lib/CodeGen/CGExpr.cpp
llvm/tools/clang/lib/CodeGen/CGExprScalar.cpp
- Handle dynamic_cast
llvm/tools/clang/lib/CodeGen/ItaniumCXXABI.cpp
```

- LLVM Pass

```
llvm/lib/Transforms/Instrumentation/HexTypeTreePass.cpp : Heap object tracing and class relationship tree
llvm/lib/Transforms/Instrumentation/HexTypePass.cpp : Stack and global object tracing
llvm/lib/Transforms/Utils/HexTypeUtil.cpp : Common utils
```

- Runtime Library

```
llvm/projects/compiler-rt/lib/hextype/hextype.cc : Type casting verfication and Object tracing
llvm/projects/compiler-rt/lib/hextype/hextype.h
llvm/projects/compiler-rt/lib/hextype/hextype_rbtree.cc : RB tree implemenation
llvm/projects/compiler-rt/lib/hextype/hextype_rbtree.h
llvm/projects/compiler-rt/lib/hextype/hextype_report.cc : create HexType logs
llvm/projects/compiler-rt/lib/hextype/hextype_report.h
```

### How to test

a. HexType testcase
```
make test
```

b. TypeSan testcase
```
run `test/typesancheck/hextype.py`
```
