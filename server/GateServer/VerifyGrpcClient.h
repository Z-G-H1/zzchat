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
	//���캯������ʼ�����ӳء�
	RPConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {

		// ѭ������ָ�������� gRPC ����
		for (size_t i = 0; i < poolSize_; ++i) {
			// ����һ�� gRPC channel��ָ��Ŀ�������������ʹ�ò���ȫ��ƾ֤�����ڲ��Ի�����������
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());

			// ͨ�� channel ����һ���ͻ��˴�� (Stub)��������������Ӷ����С�
			// Stub ���������ض����񽻻��Ŀͻ��˽ӿڡ�
			connections_.push(VerifyService::NewStub(channel));
		}
	}

	~RPConPool() {
		// ʹ�� lock_guard ȷ���������ڼ�Թ�����Դ�Ĳ������̰߳�ȫ��
		std::lock_guard<std::mutex> lock(mutex_);
		// ���� Close ������ֹͣ���ӳز��������еȴ����߳�
		Close();
		// ��ն�����ʣ�����������
		while (!connections_.empty()) {
			connections_.pop();
		}
	}


	std::unique_ptr<VerifyService::Stub> getConnection() {
		// ʹ�� unique_lock����Ϊ����Ҫ�� condition_variable ���ʹ��
		std::unique_lock<std::mutex> lock(mutex_);

		// cond_.wait ��ԭ�ӵؽ��� mutex_ ��ʹ��ǰ�߳̽���ȴ�״̬��
		// ��������ʱ�������ٴ���ס mutex_ ����� lambda ������
		// �ȴ����������ӳ�δ��ֹͣ ���� ���Ӷ��в�Ϊ�ա�
		cond_.wait(lock, [this]() {
			// ������ӳ���ֹͣ����ֱ�ӷ��� true�����ٵȴ�
			if (b_stop_) return true;
			// ���򣬵����Ӷ��в�Ϊ��ʱ������ true����ʾ���Ի�ȡ����
			return !connections_.empty();
			});

		// ����̱߳���������Ϊ���ӳر�ֹͣ�ˣ���ֱ�ӷ��ؿ�ָ��
		if (b_stop_) {
			return nullptr;
		}

		// �Ӷ���ͷ��ȡ��һ������
		// std::move ����ת�� unique_ptr ������Ȩ
		auto connection = std::move(connections_.front());
		// �Ӷ������Ƴ�������
		connections_.pop();

		return connection;
	}

	void returnConnection(std::unique_ptr<VerifyService::Stub> connection) {
		// ʹ�� lock_guard ��֤�̰߳�ȫ
		std::lock_guard<std::mutex> lock(mutex_);

		// ������ӳ��Ѿ�ֹͣ����ֱ�����ٹ黹�����ӣ����ٽ���Żس���
		if (b_stop_) {
			return;
		}

		// ���������·Żض���β��
		connections_.push(std::move(connection));

		// ֪ͨһ����cond_.wait �У����ڵȴ����̣߳������������п���������
		cond_.notify_one();
	}


	void Close() {
		// ����ֹͣ��־Ϊ true
		b_stop_ = true;
		// ���������� cond_.wait ���ȴ����߳�
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
	// ͨ��email ��ȡ��֤�� ��ȡ�ذ�
	GetVerifyRsp GetVerifyCode(std::string email) {
		// ��������
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;

		request.set_email(email);
		// ͨ�� stub_ ���󣬵���Զ�̷������ϵ� GetVarifyCode ���������� request���������ȴ���ֱ�����������ؽ��������ᱻ��䵽 reply �����С�
		// �������Ҫ��д��ʵ�� GetVarifyCode ����
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
