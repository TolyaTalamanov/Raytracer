cmake_minimum_required(VERSION 3.15)

########################################
# Project setup
########################################
project(raytracer)

########################################
# Set up the compiler flags
########################################
set(CMAKE_CXX_STANDARD 17)

########################################
# Define output paths
########################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

########################################
# Define basic flags
########################################
option(BUILD_TESTS "Build raytracer with tests" ON)

add_subdirectory(thirdparty)

set(SRC_FILES
    # API
    ${CMAKE_CURRENT_LIST_DIR}/src/image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/render.cpp
    # IMPLEMENTATION
    ${CMAKE_CURRENT_LIST_DIR}/src/tokenizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/builder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/datatypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/object.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/parser.cpp
)

set(Raytracer_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/")
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
# Set public API
target_include_directories(${PROJECT_NAME}
                           PUBLIC ${Raytracer_INCLUDE_DIR}
                           PRIVATE "${PROJECT_SOURCE_DIR}/src")
# Add dependencies
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARY} ${JPEG_LIBRARIES})

# Tool
add_executable(raytracer-tool main.cpp)
target_link_libraries(raytracer-tool ${PROJECT_NAME})

# Tests
enable_testing()
add_executable(raytracer-tests raytracer_tests.cpp)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_link_libraries(raytracer-tests gtest gtest_main)
target_link_libraries(raytracer-tests ${PROJECT_NAME})
