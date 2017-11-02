set(HEXTYPE_LIT_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(HEXTYPE_LIT_TEST_MODE "Standalone")
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/HEXTYPEConfig/lit.site.cfg)
set(HEXTYPE_TESTSUITES ${CMAKE_CURRENT_BINARY_DIR}/HEXTYPEConfig)

set(HEXTYPE_TEST_DEPS ${SANITIZER_COMMON_LIT_TEST_DEPS})
if(NOT COMPILER_RT_STANDALONE_BUILD)
  list(APPEND HEXTYPE_TEST_DEPS hextype)
endif()

add_lit_testsuite(check-runtime-hextype "Running HEXTYPE tests"
  ${HEXTYPE_TESTSUITES}
  DEPENDS ${HEXTYPE_TEST_DEPS})
set_target_properties(check-runtime-hextype PROPERTIES FOLDER "HEXTYPE unittests")
