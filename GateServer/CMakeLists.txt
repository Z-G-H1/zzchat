# CMakeLists.txt for GateServer (Final Corrected Version)

cmake_minimum_required(VERSION 3.15)
project(GateServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制 CMake 优先搜索 /usr/local 路径
list(APPEND CMAKE_PREFIX_PATH "/usr/local")

# 查找所有必需的库
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(jsoncpp REQUIRED)
find_package(Threads REQUIRED)
find_package(hiredis REQUIRED)


# 自动包含所有需要的头文件目录
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${jsoncpp_INCLUDE_DIRS}
)

# 自动查找当前目录下所有的 .cpp 和 .cc 文件
file(GLOB SOURCES "*.cpp" "*.cc")

# 创建可执行文件
add_executable(GateServer ${SOURCES})

# 链接所有库到您的程序
target_link_libraries(GateServer
    PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
    Boost::system
    Boost::filesystem
    jsoncpp_lib
    Threads::Threads
)