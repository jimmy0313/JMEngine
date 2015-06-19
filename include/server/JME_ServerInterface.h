#ifndef JME_ServerInterface_h__
#define JME_ServerInterface_h__

#include "boost/shared_ptr.hpp"

#include "JME_NetHandler.h"
#include "JME_RpcClient.h"
#include "JME_RpcServer.h"
#include "JME_RpcHandler.h"
#include <map>

using namespace std;
using namespace JMEngine;
using namespace JMEngine::net;
using namespace JMEngine::rpc;


namespace Json
{
	class Value;
}
namespace JMEngine
{
	namespace server
	{
		class JME_Server
		{
		public:
			typedef boost::shared_ptr<JME_Server> ServerPtr;
		public:
			JME_Server();
			~JME_Server();

			void init();
			
			virtual void onInit();
		protected:
			Json::Value _config;
		private:		
			size_t _serverId;
			map<string, JME_RpcClient::JME_RpcClientPtr> _serverRPC;	//rpc�����, ���ڵ���Զ�̷���
			JME_RpcServer::JME_RpcServerPtr _clientRPC;	//rpc�ͻ���

// 			// ��������net handlerͨ���ò��ϣ� ����ֻ���ڱ�д���ص�ʱ������õ�ʱ��, �������߼��������Ľ�����ʹ��rpc
// 			// �ڲ�������֮��ͨ��rpc���н���
			
//			JME_NetHandler::JME_NetHandlerPtr _clientHandler;	//���������ҵġ��ͻ��ˡ���������Ϣ����, ���ڴ������ع����Ŀͻ�����Ϣ
// 			JME_NetHandler::JME_NetHandlerPtr _serverHandler;	//���������ӵġ�����ˡ�����Ӧ��Ӧ����
		};

	}
}
#endif // JME_ServerInterface_h__
