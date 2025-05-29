#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;

public:
	// 通过email 获取验证码 获取回包
	GetVerifyRsp GetVerifyCode(std::string email) {
		// 构造请求
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;

		request.set_email(email);
		// 通过 stub_ 对象，调用远程服务器上的 GetVarifyCode 方法，发送 request，并阻塞等待，直到服务器返回结果，结果会被填充到 reply 对象中。
		// 服务端需要重写并实现 GetVarifyCode 方法
		Status status = stub_->GetVerifyCode(&context,request,&reply);

		if (status.ok()) {
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient() {
		// 创建一个到 gRPC 服务器的通信通道
		std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());

		// ============= 在这里加入诊断代码 =============
		std::cout << "[C++-DEBUG] Attempting to connect to target: " << std::endl;

		// 尝试在5秒内等待连接成功
		bool connected = channel->WaitForConnected(gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(5, GPR_TIMESPAN)));

		if (connected) {
			std::cout << "[C++-DEBUG] Channel is successfully connected." << std::endl;
		}
		else {
			// 如果连接失败，打印出当前的状态
			grpc_connectivity_state state = channel->GetState(true);
			std::cout << "[C++-DEBUG] FAILED TO CONNECT within 5 seconds. Current channel state code is: " << state << std::endl;
			// 状态码含义: 0=IDLE, 1=CONNECTING, 2=READY(同connected), 3=TRANSIENT_FAILURE, 4=SHUTDOWN
		}


		//使用上面创建的通道，创建一个存根 (Stub) 
		//存根是客户端与服务器端通信的接口 v这个存根对象是远程服务的本地代理
		// 你对 stub_ 的所有方法调用，都会被 gRPC 框架转化为网络请求发送到服务器。
		stub_ = VerifyService::NewStub(channel);
	}
	std::unique_ptr<VerifyService::Stub> stub_;

}; 

