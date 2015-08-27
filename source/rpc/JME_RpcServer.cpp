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
			LOGI("Listen on port [ %d ] for rpc service", port);
		}

		JME_RpcServer::~JME_RpcServer()
		{
			
		}

		void JME_RpcServer::response( JMEngine::net::JME_TcpSessionPtr session, const jme_rpc& params )
		{
			string m;
			params.SerializeToString(&m);

			JMEngine::net::JME_Message msg(RPCMessage, m);	

			session->writeMessage(msg);
		}



		JMEngine::rpc::JME_RpcSessionNetHandler::JME_RpcSessionNetHandlerPtr JME_RpcSessionNetHandler::create(JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler)
		{
			return JMEngine::rpc::JME_RpcSessionNetHandler::JME_RpcSessionNetHandlerPtr(new JME_RpcSessionNetHandler(server, handler));
		}

		void JME_RpcSessionNetHandler::sessionConnectSucceed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session )
		{
			session->start(RPCSession);

			LOGT("New rpc client connected from [ %s:%s ]", session->getIp(), session->getPort());
		}

		void JME_RpcSessionNetHandler::sessionConnectFailed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void JME_RpcSessionNetHandler::sessionDisconnect( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void JME_RpcSessionNetHandler::sessionReceiveMessage( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JMEngine::net::JME_MessagePtr msg )
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

				_server->response(session, rpc);
			}
		}

		void JME_RpcSessionNetHandler::sessionReadError( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
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

		JME_RpcSessionNetHandler::JME_RpcSessionNetHandler( JME_RpcServer* server, JME_RpcHandler::JME_RpcHandlerPtr handler ):
			_server(server),
			_handler(handler)
		{

		}

	}
}