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
				LOGE("Method [ %s ] had been registered", method);
				abort();
			}
			else
			{
				LOGI("Register method [ %s ] succeed", method);
			}
		}

		google::protobuf::Message* JME_RpcHandler::execRpcHandler( const string& method, const string& params )
		{
			auto it = _handlers.find(method);
			if (it == _handlers.end())
			{
				LOGE("Can't find handler for method [ %s ]", method);
			}
			else
			{
				return it->second(params);
			}
			return nullptr;
		}

		JME_RpcHandler::JME_RpcHandlerPtr JME_RpcHandler::create()
		{
			return JME_RpcHandlerPtr(new JME_RpcHandler);
		}

	}
}