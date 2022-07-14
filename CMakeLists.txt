project(render-sample)
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

add_executable(raytracer-tests raytracer-tests.cpp)

add_executable(${PROJECT_NAME} main.cpp)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARY} ${JPEG_LIBRARIES})


enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_link_libraries(raytracer-tests gtest gtest_main)
target_link_libraries(raytracer-tests ${PNG_LIBRARY} ${JPEG_LIBRARIES})
