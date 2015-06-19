#include "JME_ServerInterface.h"
#include "json/json.h"
#include "JME_JsonFile.h"
#include "JME_GLog.h"
namespace JMEngine
{
	namespace server
	{


		JME_Server::JME_Server()
		{

		}

		JME_Server::~JME_Server()
		{

		}

		void JME_Server::init()
		{
			_config = JMEngine::file::load_jsonfile_val("./config/conf.json");
			
			auto& _baseConf = _config["base"];

			_serverId = _baseConf["server_id"].asInt();

			auto& rpc = _config["rpc"];
			auto& serverConf = rpc["server"];
			for (auto& c : serverConf)
			{
				auto r = JME_RpcClient::create(c["ip"].asString(), c["port"].asString(), c["reconnect"].asUInt(), c["buffer_size"].asUInt());
				_serverRPC[c["name"].asString()] = r;
			}
			auto& clientConf = rpc["client"];
			_clientRPC = JME_RpcServer::create(JME_RpcHandler::create(), clientConf["port"].asUInt(), clientConf["buffer_size"].asUInt());

			onInit();

			auto name = _baseConf["server"].asString();
			LogT << name << " start succeed" << LogEnd;
		}

		void JME_Server::onInit()
		{

		}

	}
}