const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

// 定义 proto 文件的路径
const PROTO_PATH = path.join(__dirname, 'message.proto')

// 加载 proto 文件
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
    keepCase: true, // 保留字段原始命名格式
    longs: String,  // 长整型用字符串表示，避免精度丢失
    enums: String,  // 枚举用字符串表示
    defaults: true, // 为字段设置默认值
    oneofs: true    // 支持 oneof 特性
})

// 从包定义中加载 gRPC 对象
const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)
// 导出 message 包
const message_proto = protoDescriptor.message
module.exports = message_proto