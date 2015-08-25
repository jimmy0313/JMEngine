#ifndef JME_RpcServer_h__
#define JME_RpcServer_h__

#include <string>

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"

#include "google/protobuf/message.h"

#include "JME_RpcHandler.h"
#include "JME_Acceptor.h"
#include "JME_NetHandler.h"
#include "JME_TcpSession.h"
#include "rpc.pb.h"

using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class JME_RpcServer
		{
		public:
			typedef boost::shared_ptr<JME_RpcServer> JME_RpcServerPtr;
		public:
			JME_RpcServer(JME_RpcHandler::JME_RpcHandlerPtr handler, unsigned short port, size_t bufferSize);
			~JME_RpcServer();

			static JMEngine::rpc::JME_RpcServer::JME_RpcServerPtr create(JME_RpcHandler::JME_RpcHandlerPtr handler, unsigned short port, size_t bufferSize);
			
			void response(JMEngine::net::JME_TcpSessionPtr session, const jme_rpc& params);
		public:
			void start();
			void stop();

		private:
			JMEngine::net::JME_TcpAcceptor::JME_TcpAcceptorPtr _acceptorPtr;
			JME_RpcHandler::JME_RpcHandlerPtr _handler;
		}; 

		class JME_RpcSessionNetHandler : public JMEngine::net::JME_NetHandler
		{
		public:
			typedef boost::shared_ptr<JME_RpcSessionNetHandler> JME_RpcSessionNetHandlerPtr;

		public:
			JME_RpcSessionNetHandler(JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler);
			static JMEngine::rpc::JME_RpcSessionNetHandler::JME_RpcSessionNetHandlerPtr create(JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler);

			void sessionConnectSucceed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session);
			void sessionConnectFailed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionDisconnect(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionReceiveMessage(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JMEngine::net::JME_MessagePtr msg);
			void sessionReadError(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
		private:
			JME_RpcHandler::JME_RpcHandlerPtr _handler;
			JME_RpcServer* _server;
		};
	}
}
#endif // JME_RpcServer_h__
