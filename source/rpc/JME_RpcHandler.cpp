#include "rpc/JME_RpcHandler.h"

namespace JMEngine
{
	namespace rpc
	{

		map<string, RpcHandlerInterface::RpcHandler> JMEngine::rpc::RpcHandlerInterface::_handlers;

		void RpcHandlerInterface::regRpcHandler( const char* method, RpcHandler handler )
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

		google::protobuf::Message* RpcHandlerInterface::execRpcHandler( const string& method, const string& params )
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

		RpcHandlerInterface::RpcHandlerInterfacePtr RpcHandlerInterface::create()
		{
			return RpcHandlerInterfacePtr(new RpcHandlerInterface);
		}

	}
}