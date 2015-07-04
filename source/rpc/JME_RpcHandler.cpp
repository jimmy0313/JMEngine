#include "JME_RpcHandler.h"

namespace JMEngine
{
	namespace rpc
	{

		map<string, JME_RpcHandler::RpcHandler> JMEngine::rpc::JME_RpcHandler::_handlers;

		void JME_RpcHandler::regRpcHandler( const char* method, RpcHandler handler )
		{
			auto res = _handlers.insert(make_pair(method,handler)); 
			if(!res.second)
			{
				LogE << "Method [ " << method << " ] had been registered" << LogEnd;
				abort();
			}
			else
			{
				LogI << "Register method [ " << method << " ] succeed" << LogEnd;
			}
		}

		void JME_RpcHandler::execRpcHandler( JME_RpcServerPtr conn, JMEngine::net::JME_TcpSessionPtr session, const JME_Rpc& params )
		{
			auto it = _handlers.find(params._method);
			if (it == _handlers.end())
			{
				LogE << "Can't find handler for method " << params._method << LogEnd;
			}
			else
			{
				it->second(conn, session, params);
			}
		}

		JME_RpcHandler::JME_RpcHandlerPtr JME_RpcHandler::create()
		{
			return JME_RpcHandlerPtr(new JME_RpcHandler);
		}

	}
}