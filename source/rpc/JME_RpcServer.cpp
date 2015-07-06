#include "JME_RpcServer.h"
#include "JME_GLog.h"
#include "JME_Core.h"

namespace JMEngine
{
	namespace rpc
	{

		JMEngine::rpc::JME_RpcServer::JME_RpcServerPtr JME_RpcServer::create( JME_RpcHandler::JME_RpcHandlerPtr handler, unsigned short port, size_t bufferSize )
		{
			return JMEngine::rpc::JME_RpcServer::JME_RpcServerPtr(new JME_RpcServer(handler, port, bufferSize));
		}


		JME_RpcServer::JME_RpcServer( JME_RpcHandler::JME_RpcHandlerPtr handler, unsigned short port, size_t bufferSize ):
			_handler(handler)
		{
			_acceptorPtr = JMEngine::net::JME_TcpAcceptor::create(JME_RpcSessionNetHandler::create(this, handler), port);
 			_acceptorPtr->accept(0, bufferSize);
			LogT << "Listen on port {" << port << "} for rpc service" << LogEnd;
		}

		JME_RpcServer::~JME_RpcServer()
		{
			
		}

		void JME_RpcServer::response( JMEngine::net::JME_TcpSessionPtr session, const JME_Rpc& params )
		{
			string m = params.serializeAsString();
			JMEngine::net::JME_Message msg(1, m);	

			session->writeMessage(msg);
		}



		JMEngine::rpc::JME_RpcSessionNetHandler::JME_RpcSessionNetHandlerPtr JME_RpcSessionNetHandler::create(JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler)
		{
			return JMEngine::rpc::JME_RpcSessionNetHandler::JME_RpcSessionNetHandlerPtr(new JME_RpcSessionNetHandler(server, handler));
		}

		void JME_RpcSessionNetHandler::sessionConnectSucceed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session )
		{
			try
			{
				LogT << "New rpc client connected from " << session->socket().remote_endpoint().address().to_string() << LogEnd;

				session->start(1);
			}
			catch(boost::system::system_error e)
			{
				LogE <<  e.what() << "{" << e.code() << "}" << LogEnd;
			}
		}

		void JME_RpcSessionNetHandler::sessionConnectFailed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void JME_RpcSessionNetHandler::sessionDisconnect( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void JME_RpcSessionNetHandler::sessionReceiveMessage( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JMEngine::net::JME_MessagePtr msg )
		{
			string m = msg->getMessageStr();
			JME_Rpc r(m);
			_handler->execRpcHandler(_server, session, r);
		}

		void JME_RpcSessionNetHandler::sessionReadError( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			try
			{
				session->stop();

				LogT << "Rpc client connected from " << session->getIp() << " disconnect" << LogEnd;
			}
			catch(boost::system::system_error e)
			{
				LogE <<  e.what() << "{" << e.code() << "}" << LogEnd;
			}
		}

		JME_RpcSessionNetHandler::JME_RpcSessionNetHandler( JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler ):
			_server(server),
			_handler(handler)
		{

		}

	}
}