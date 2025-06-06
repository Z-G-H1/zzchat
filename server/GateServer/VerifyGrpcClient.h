#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConPool {
public:
	//构造函数，初始化连接池。
	RPConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {

		// 循环创建指定数量的 gRPC 连接
		for (size_t i = 0; i < poolSize_; ++i) {
			// 创建一个 gRPC channel，指向目标服务器。这里使用不安全的凭证，用于测试或内网环境。
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());

			// 通过 channel 创建一个客户端存根 (Stub)，并将其放入连接队列中。
			// Stub 是与服务端特定服务交互的客户端接口。
			connections_.push(VerifyService::NewStub(channel));
		}
	}

	~RPConPool() {
		// 使用 lock_guard 确保在析构期间对共享资源的操作是线程安全的
		std::lock_guard<std::mutex> lock(mutex_);
		// 调用 Close 方法，停止连接池并唤醒所有等待的线程
		Close();
		// 清空队列中剩余的所有连接
		while (!connections_.empty()) {
			connections_.pop();
		}
	}


	std::unique_ptr<VerifyService::Stub> getConnection() {
		// 使用 unique_lock，因为它需要与 condition_variable 配合使用
		std::unique_lock<std::mutex> lock(mutex_);

		// cond_.wait 会原子地解锁 mutex_ 并使当前线程进入等待状态。
		// 当被唤醒时，它会再次锁住 mutex_ 并检查 lambda 条件。
		// 等待条件：连接池未被停止 并且 连接队列不为空。
		cond_.wait(lock, [this]() {
			// 如果连接池已停止，则直接返回 true，不再等待
			if (b_stop_) return true;
			// 否则，当连接队列不为空时，返回 true，表示可以获取连接
			return !connections_.empty();
			});

		// 如果线程被唤醒是因为连接池被停止了，则直接返回空指针
		if (b_stop_) {
			return nullptr;
		}

		// 从队列头部取出一个连接
		// std::move 用于转移 unique_ptr 的所有权
		auto connection = std::move(connections_.front());
		// 从队列中移除该连接
		connections_.pop();

		return connection;
	}

	void returnConnection(std::unique_ptr<VerifyService::Stub> connection) {
		// 使用 lock_guard 保证线程安全
		std::lock_guard<std::mutex> lock(mutex_);

		// 如果连接池已经停止，则直接销毁归还的连接，不再将其放回池中
		if (b_stop_) {
			return;
		}

		// 将连接重新放回队列尾部
		connections_.push(std::move(connection));

		// 通知一个（cond_.wait 中）正在等待的线程，告诉它现在有可用连接了
		cond_.notify_one();
	}


	void Close() {
		// 设置停止标志为 true
		b_stop_ = true;
		// 唤醒所有在 cond_.wait 处等待的线程
		cond_.notify_all();
	}

private:
	std::atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;

public:
	~VerifyGrpcClient() {

	}
	// 通过email 获取验证码 获取回包
	GetVerifyRsp GetVerifyCode(std::string email) {
		// 构造请求
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;

		request.set_email(email);
		// 通过 stub_ 对象，调用远程服务器上的 GetVarifyCode 方法，发送 request，并阻塞等待，直到服务器返回结果，结果会被填充到 reply 对象中。
		// 服务端需要重写并实现 GetVarifyCode 方法
		auto stub = pool_->getConnection();
		Status status = stub->GetVerifyCode(&context, request, &reply);

		if (status.ok()) {
			pool_->returnConnection(std::move(stub));
			return reply;
		}
		else {
			pool_->returnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient() {

		auto& gCfgMgr = ConfigMgr::Inst();
		std::string host = gCfgMgr["VerifyServer"]["Host"];
		std::string port = gCfgMgr["VerifyServer"]["Port"];
		pool_.reset(new RPConPool(5, host, port));
	}

	std::unique_ptr<RPConPool> pool_;

};
