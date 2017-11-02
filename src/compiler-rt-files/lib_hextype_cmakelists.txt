set(HEXTYPE_SOURCES
  hextype.cc
  hextype_rbtree.cc
  hextype_report.cc
  )

include_directories(..)

set(HEXTYPE_CFLAGS ${SANITIZER_COMMON_CFLAGS})

add_custom_target(hextype)

add_compiler_rt_runtime(clang_rt.hextype
        STATIC
        ARCHS x86_64
        SOURCES ${HEXTYPE_SOURCES}
        CFLAGS ${HEXTYPE_CFLAGS}
        PARENT_TARGET hextype)

add_sanitizer_rt_symbols(clang_rt.hextype)

add_dependencies(compiler-rt hextype)
