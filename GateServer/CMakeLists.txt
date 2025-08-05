# CMakeLists.txt for GateServer (Flat Directory Structure)

cmake_minimum_required(VERSION 3.10)
project(GateServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find source files in the current directory
file(GLOB SOURCES "*.cpp")

# Find required libraries using pkg-config (recommended)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Add executable
add_executable(GateServer ${SOURCES})

# Link libraries
target_link_libraries(GateServer
    PRIVATE
    ${JSONCPP_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    pthread
)

# Set output directory for the executable
set_target_properties(GateServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)