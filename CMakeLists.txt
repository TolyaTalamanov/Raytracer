project(render-sample)
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARY} ${JPEG_LIBRARIES})
