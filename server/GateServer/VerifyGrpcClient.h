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
	GetVerifyRsp GetVerifyCode(std::string email) {
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
		std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());

		// ============= �����������ϴ��� =============
		std::cout << "[C++-DEBUG] Attempting to connect to target: " << std::endl;

		// ������5���ڵȴ����ӳɹ�
		bool connected = channel->WaitForConnected(gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(5, GPR_TIMESPAN)));

		if (connected) {
			std::cout << "[C++-DEBUG] Channel is successfully connected." << std::endl;
		}
		else {
			// �������ʧ�ܣ���ӡ����ǰ��״̬
			grpc_connectivity_state state = channel->GetState(true);
			std::cout << "[C++-DEBUG] FAILED TO CONNECT within 5 seconds. Current channel state code is: " << state << std::endl;
			// ״̬�뺬��: 0=IDLE, 1=CONNECTING, 2=READY(ͬconnected), 3=TRANSIENT_FAILURE, 4=SHUTDOWN
		}


		//ʹ�����洴����ͨ��������һ����� (Stub) 
		//����ǿͻ������������ͨ�ŵĽӿ� v������������Զ�̷���ı��ش���
		// ��� stub_ �����з������ã����ᱻ gRPC ���ת��Ϊ���������͵���������
		stub_ = VerifyService::NewStub(channel);
	}
	std::unique_ptr<VerifyService::Stub> stub_;

}; 

