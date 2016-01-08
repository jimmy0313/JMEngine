#include "rpc/JME_RpcServer.h"
#include "log/JME_GLog.h"
#include "net/JME_Core.h"

namespace JMEngine
{
	namespace rpc
	{

		JMEngine::rpc::RpcServer::RpcServerPtr RpcServer::create( RpcHandlerInterface::RpcHandlerInterfacePtr handler, unsigned short port, size_t bufferSize )
		{
			return JMEngine::rpc::RpcServer::RpcServerPtr(new RpcServer(handler, port, bufferSize));
		}


		RpcServer::RpcServer( RpcHandlerInterface::RpcHandlerInterfacePtr handler, unsigned short port, size_t bufferSize ):
			_handler(handler)
		{
			_acceptorPtr = JMEngine::net::TcpAcceptor::create(RpcSessionNetHandler::create(this, handler), port);
 			_acceptorPtr->accept(0, bufferSize);
			LOGI("Listen on port [ %d ] for rpc service", port);
		}

		RpcServer::~RpcServer()
		{
			
		}

		void RpcServer::response( JMEngine::net::TcpSessionPtr session, const jme_rpc& params )
		{
			string m;
			params.SerializeToString(&m);

			JMEngine::net::Message msg(RPCMessage, m);	

			session->writeMessage(msg);
		}



		JMEngine::rpc::RpcSessionNetHandler::RpcSessionNetHandlerPtr RpcSessionNetHandler::create(RpcServer* server, RpcHandlerInterface::RpcHandlerInterfacePtr handler)
		{
			return JMEngine::rpc::RpcSessionNetHandler::RpcSessionNetHandlerPtr(new RpcSessionNetHandler(server, handler));
		}

		void RpcSessionNetHandler::sessionConnectSucceed( JMEngine::net::TcpSession::TcpSessionPtr session )
		{
			session->start(RPCSession);

			LOGT("New rpc client connected from [ %s:%s ]", session->getIp(), session->getPort());
		}

		void RpcSessionNetHandler::sessionConnectFailed( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void RpcSessionNetHandler::sessionDisconnect( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void RpcSessionNetHandler::sessionReceiveMessage( JMEngine::net::TcpSession::TcpSessionPtr session, JMEngine::net::MessagePtr msg )
		{
			string param = msg->getMessageStr();
			jme_rpc rpc;
			rpc.ParseFromString(param);

			auto result = _handler->execRpcHandler(rpc.method(), rpc.params());
			if (nullptr != result)
			{
				jme_rpc response;
				response.set_rpc_id(rpc.rpc_id());
				response.set_method(rpc.method());
				response.set_params(boost::move(result->SerializeAsString()));

				delete result;
				result = nullptr;

				_server->response(session, response);
			}
		}

		void RpcSessionNetHandler::sessionReadError( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{
			try
			{
				session->stop();

				LOGW("Rpc client from [ %s:%s ] disconnected", session->getIp(), session->getPort());
			}
			catch(boost::system::system_error e)
			{
				LOGE("error ==> [ %d:%s ]", e.code().value(), e.what());
			}
		}

		RpcSessionNetHandler::RpcSessionNetHandler( RpcServer* server, RpcHandlerInterface::RpcHandlerInterfacePtr handler ):
			_server(server),
			_handler(handler)
		{

		}

	}
}