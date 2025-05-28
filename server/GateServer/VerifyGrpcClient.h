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
	// ͨ��email ��ȡ��֤�� ��ȡ�ذ�
	GetVerifyRsp GetVarifyCode(std::string email) {
		// ��������
		ClientContext context;
		GetVerifyRsp reply;
		GetVerifyReq request;

		request.set_email(email);
		// ͨ�� stub_ ���󣬵���Զ�̷������ϵ� GetVarifyCode ���������� request���������ȴ���ֱ�����������ؽ��������ᱻ��䵽 reply �����С�
		// �������Ҫ��д��ʵ�� GetVarifyCode ����
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
		// ����һ���� gRPC ��������ͨ��ͨ��
		std::shared_ptr<Channel> channel = grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials());
		//ʹ�����洴����ͨ��������һ����� (Stub) 
		//����ǿͻ������������ͨ�ŵĽӿ� v������������Զ�̷���ı��ش���
		// ��� stub_ �����з������ã����ᱻ gRPC ���ת��Ϊ���������͵���������
		stub_ = VerifyService::NewStub(channel);
	}
	std::unique_ptr<VerifyService::Stub> stub_;

}; 

