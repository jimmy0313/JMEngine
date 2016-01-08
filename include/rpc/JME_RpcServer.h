#ifndef JME_RpcServer_h__
#define JME_RpcServer_h__

#include <string>

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"

#include "google/protobuf/message.h"

#include "rpc/JME_RpcHandler.h"
#include "net/JME_Acceptor.h"
#include "net/JME_NetHandler.h"
#include "net/JME_TcpSession.h"
#include "rpc/JME_RpcPb.h"

using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class RpcServer
		{
		public:
			typedef boost::shared_ptr<RpcServer> RpcServerPtr;
		public:
			RpcServer(RpcHandlerInterface::RpcHandlerInterfacePtr handler, unsigned short port, size_t bufferSize);
			~RpcServer();

			static JMEngine::rpc::RpcServer::RpcServerPtr create(RpcHandlerInterface::RpcHandlerInterfacePtr handler, unsigned short port, size_t bufferSize);
			
			void response(JMEngine::net::TcpSessionPtr session, const jme_rpc& params);
		public:
			void start();
			void stop();

		private:
			JMEngine::net::TcpAcceptor::TcpAcceptorPtr _acceptorPtr;
			RpcHandlerInterface::RpcHandlerInterfacePtr _handler;
		}; 

		class RpcSessionNetHandler : public JMEngine::net::NetHandler
		{
		public:
			typedef boost::shared_ptr<RpcSessionNetHandler> RpcSessionNetHandlerPtr;

		public:
			RpcSessionNetHandler(RpcServer* server, RpcHandlerInterface::RpcHandlerInterfacePtr handler);
			static JMEngine::rpc::RpcSessionNetHandler::RpcSessionNetHandlerPtr create(RpcServer* server, RpcHandlerInterface::RpcHandlerInterfacePtr handler);

			void sessionConnectSucceed(JMEngine::net::TcpSession::TcpSessionPtr session);
			void sessionConnectFailed(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);
			void sessionDisconnect(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);
			void sessionReceiveMessage(JMEngine::net::TcpSession::TcpSessionPtr session, JMEngine::net::MessagePtr msg);
			void sessionReadError(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);
		private:
			RpcHandlerInterface::RpcHandlerInterfacePtr _handler;
			RpcServer* _server;
		};
	}
}
#endif // JME_RpcServer_h__
