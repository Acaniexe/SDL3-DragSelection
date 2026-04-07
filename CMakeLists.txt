cmake_minimum_required(VERSION 3.10)

project(MySDLApp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB SRC_FILES "src/*.cpp")

add_executable(app ${SRC_FILES})

target_include_directories(app PRIVATE SDL3/include)
target_link_directories(app PRIVATE SDL3/lib)
target_link_libraries(app SDL3)