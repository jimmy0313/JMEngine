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
			map<string, JME_RpcClient::JME_RpcClientPtr> _serverRPC;	//rpc服务端, 用于调用远程服务
			JME_RpcServer::JME_RpcServerPtr _clientRPC;	//rpc客户端

// 			// 以下两个net handler通常用不上， 基本只有在编写网关的时候才有用的时候, 网关与逻辑服务器的交互不使用rpc
// 			// 内部服务器之间通过rpc进行交互
			
//			JME_NetHandler::JME_NetHandlerPtr _clientHandler;	//主动连接我的“客户端”的请求消息处理, 用于处理网关过来的客户端消息
// 			JME_NetHandler::JME_NetHandlerPtr _serverHandler;	//我主动连接的“服务端”的响应响应处理
		};

	}
}
#endif // JME_ServerInterface_h__
