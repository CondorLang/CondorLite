cmake_minimum_required(VERSION 2.8)

enable_testing()

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_LIBRARIES} ${CMAKE_SOURCE_DIR}/libCondorLib.a)
include_directories(. ../src)

set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set(BINARY_DIR ${CMAKE_SOURCE_DIR}/../build)

set(SOURCE_LIST
  ${TEST_DIR}/main.c
  ${TEST_DIR}/condor/ast/test_ast.c
)

add_executable(test_condor ${SOURCE_LIST})

target_link_libraries(test_condor ${LIBS})
add_test(test_condor ${BINARY_DIR}/test_condor)