#ifndef JME_ServerInterface_h__
#define JME_ServerInterface_h__

#include "JME_TcpSession.h"
#include "JME_NetHandler.h"
#include "JME_Dispatcher.h"
#include "JME_Message.h"
#include "JME_RpcClient.h"
#include "JME_RpcServer.h"

#include "JME_RPCManager.h"

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

		private:
			//************************************
			// Method:    init
			// FullName:  JMEngine::game::JME_ServerInterface::init
			// Access:    virtual public 
			// Returns:   void
			// Qualifier: 逻辑服务器进行一些初始化， 例如数据库等
			//************************************
			virtual void init();

			//************************************
			// Method:    init_
			// FullName:  JMEngine::game::JME_ServerInterface::init_
			// Access:    public 
			// Returns:   void
			// Qualifier: 初始化各种连接， 必须在 各个handler初始化完毕后调用
			//************************************
			void init_();
		public:

		protected:
			Json::Value _config;
			size_t _serverId;

			// 主要用于网关服务器， 接受客户端连接
			JME_TcpAcceptor::JME_TcpAcceptorPtr _acceptorPtr;
			JME_NetHandler::JME_NetHandlerPtr _clientHandler;	//主动连接我的“客户端”的请求消息处理
		};
	}
}
#endif // JME_ServerInterface_h__
