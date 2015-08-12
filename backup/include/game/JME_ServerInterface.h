#ifndef JME_ServerInterface_h__
#define JME_ServerInterface_h__

#include "JME_TcpSession.h"
#include "JME_NetHandler.h"
#include "JME_Dispatcher.h"
#include "JME_Message.h"
#include "JME_RpcClient.h"
#include "JME_RpcServer.h"

using namespace JMEngine;
using namespace JMEngine::net;
using namespace JMEngine::rpc;
using namespace JMEngine::game;

namespace JMEngine
{
	namespace game
	{
		class JME_ServerInterface
		{
		public:
			void onInit();

			//************************************
			// Method:    init
			// FullName:  JMEngine::game::JME_ServerInterface::init
			// Access:    virtual public 
			// Returns:   void
			// Qualifier: �߼�����������һЩ��ʼ���� �������ݿ��
			//************************************
			virtual void init();

			//************************************
			// Method:    init_
			// FullName:  JMEngine::game::JME_ServerInterface::init_
			// Access:    public 
			// Returns:   void
			// Qualifier: ��ʼ���������ӣ� ������ ����handler��ʼ����Ϻ����
			//************************************
			void init_();
		public:

			JME_RpcClient::JME_RpcClientPtr getRpcByName(const char* name);
		protected:
			Json::Value _config;
			size_t _serverId;

			JME_TcpAcceptor::JME_TcpAcceptorPtr _acceptorPtr;
			JME_NetHandler::JME_NetHandlerPtr _clientHandler;	//���������ҵġ��ͻ��ˡ���������Ϣ����

			map<string, JME_TcpSession::JME_TcpSessionPtr> _servers;
			JME_NetHandler::JME_NetHandlerPtr _serverHandler;	//���������ӵġ����������Ļظ���Ϣ����

			map<string, JME_RpcClient::JME_RpcClientPtr> _rpcClient;	//rpc�ͻ���, ���ڵ���Զ�̷���
		};
	}
}
#endif // JME_ServerInterface_h__
