#ifndef JME_RPCManager_h__
#define JME_RPCManager_h__

#include "JME_TcpSession.h"
#include "JME_NetHandler.h"
#include "JME_Dispatcher.h"
#include "JME_Message.h"
#include "JME_RpcClient.h"
#include "JME_RpcServer.h"

#include "JME_Singleton.h"

using namespace JMEngine;
using namespace JMEngine::net;
using namespace JMEngine::rpc;
using namespace JMEngine::game;

namespace Json
{
	class Value;
}

#define GETRPC(server) JMEngine::game::JME_RPCManager::getInstance()->getRpcClient(server)
namespace JMEngine
{
	namespace game
	{
		class JME_RPCManager
		{
		public:
			CreateSingletonClass(JME_RPCManager);

			//************************************
			// Method:    initRpcClient
			// FullName:  JMEngine::game::JME_RPCManager::initRpcClient
			// Access:    public 
			// Returns:   void
			// Qualifier: Ϊ�˷�������� ��ʼ��rpc�ͻ��˷��ڷ�������ʼ����һ���� ��������rpc������
			// Parameter: const Json::Value & conf
			//************************************
			void initRpcClient(const Json::Value& conf);


			//************************************
			// Method:    initRpcServer
			// FullName:  JMEngine::game::JME_RPCManager::initRpcServer
			// Access:    public 
			// Returns:   void
			// Qualifier: rpc���������� ���ڷ�������ʼ���������� ��rpc����ע����Ϻ����
			// Parameter: const Json::Value & conf
			//************************************
			void initRpcServer(const Json::Value& conf);

			JME_RpcClient::JME_RpcClientPtr getRpcClient(const char* server);
		private:
			map<string, JME_RpcClient::JME_RpcClientPtr> _rpcClient;	//rpc�ͻ���, ���ڵ���Զ�̷���

			JME_RpcServer::JME_RpcServerPtr _rpcServer;
		};
	}
}
#endif // JME_RPCManager_h__
