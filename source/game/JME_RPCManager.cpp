#include "JME_RPCManager.h"
#include "json/json.h"

namespace JMEngine
{
	namespace game
	{

		void JME_RPCManager::initRpcClient(const Json::Value& conf)
		{
			const auto& clientConf = conf["client"];
			for (const auto& c : clientConf)
			{
				auto rpc = JME_RpcClient::create(c["ip"].asString(), c["port"].asString(), c["reconnect"].asUInt(), c["buffer_size"].asUInt());
				_rpcClient[c["name"].asString()] = rpc;
			}
		}

		JME_RpcClient::JME_RpcClientPtr JME_RPCManager::getRpcClient(const char* server)
		{
			auto rpc = _rpcClient.find(server);
			if (rpc == _rpcClient.end())
				throw "can't find this rpc client";

			return rpc->second;
		}

		void JME_RPCManager::initRpcServer(const Json::Value& conf)
		{
			if (conf.isMember("server"))
			{
				auto& serverConf = conf["server"];
				_rpcServer = JME_RpcServer::create(JME_RpcHandler::create(), serverConf["port"].asUInt(), serverConf["buffer_size"].asUInt());
			}
		}

	}
}