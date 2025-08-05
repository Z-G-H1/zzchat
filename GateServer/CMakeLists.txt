cmake_minimum_required(VERSION 3.10)

# 项目名称
project(Server)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 gRPC 和 Protobuf 包
# 假设 gRPC 和 Protobuf 已经安装到系统，或者你设置了相应的环境变量
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

# gRPC 和 Protobuf 的头文件路径会自动添加到 include_directories
# 它们通常由 find_package 命令自动处理
# include_directories(${gRPC_INCLUDE_DIRS} ${Protobuf_INCLUDE_DIRS})

# 定义 proto 文件
set(PROTO_FILE message.proto)

# 从 .proto 文件生成 C++ 代码
# 这会自动生成 message.pb.cc, message.pb.h, message.grpc.pb.cc, message.grpc.pb.h
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILE})

# 定义源文件
set(MY_SRCS
    CServer.cpp
    HttpConnection.cpp
    LogicSystem.cpp
    main.cpp
)

# 定义所有源文件
set(ALL_SRCS
    ${MY_SRCS}
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

# 添加可执行文件
# 可执行文件名为 server，与你图片中的文件名一致
add_executable(server ${ALL_SRCS})

# 链接 gRPC 和 Protobuf 库
# gRPC::grpc++ 是 C++ 库，gRPC::grpc 是 C 库
target_link_libraries(server
    gRPC::grpc++
    gRPC::grpc
    Protobuf::protobuf
    jsoncpp_lib
)