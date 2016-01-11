#include "rpc/JME_RpcHandler.h"

namespace JMEngine
{
	namespace rpc
	{

		RpcHandlerInterface::RpcHandlerInterfacePtr RpcHandlerInterface::create()
		{
			return RpcHandlerInterfacePtr(new RpcHandlerInterface);
		}

		void RpcHandlerInterface::regRpcHandler( const char* method, RpcHandler handler )
		{
			auto& handlers = getRpcHandler();
			auto res = handlers.insert(make_pair(method,handler)); 
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
			auto& handlers = getRpcHandler();

			auto it = handlers.find(method);
			if (it == handlers.end())
			{
				LOGE("Can't find handler for method [ %s ]", method);
			}
			else
			{
				return it->second(params);
			}
			return nullptr;
		}

		map<string, RpcHandlerInterface::RpcHandler>& RpcHandlerInterface::getRpcHandler()
		{
			 static map<string, RpcHandlerInterface::RpcHandler> handlers;
			 return handlers;
		}

	}
}