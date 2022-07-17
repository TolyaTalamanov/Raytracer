set(TEST_NAME raytracer-tests)
enable_testing()

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cpp)

add_executable(${TEST_NAME} ${TEST_SRC_FILES})

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_link_libraries(${TEST_NAME} gtest gtest_main)
target_link_libraries(${TEST_NAME} ${PROJECT_NAME})
