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

		string JME_RpcHandler::execRpcHandler( const string& method, const string& params )
		{
			auto it = _handlers.find(method);
			if (it == _handlers.end())
			{
				LogE << "Can't find handler for method " << method << LogEnd;
			}
			else
			{
				return it->second(params);
			}
		}

		JME_RpcHandler::JME_RpcHandlerPtr JME_RpcHandler::create()
		{
			return JME_RpcHandlerPtr(new JME_RpcHandler);
		}

	}
}