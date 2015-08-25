#include "JME_ServerInterface.h"
#include "JME_JsonFile.h"

namespace JMEngine
{
	namespace game
	{


		void JME_ServerInterface::onInit()
		{
			_config = JMEngine::file::load_jsonfile_val("./config/conf.json");

			const auto& _baseConf = _config["base"];
			_serverId = _baseConf["server_id"].asInt();
			_serverName = _baseConf["server_name"].asString();

			const auto& rpc = _config["rpc"];

			JME_RPCManager::getInstance()->initRpcClient(rpc);

			init();

			init_();
		}

		void JME_ServerInterface::init_()
		{
			const auto& rpc = _config["rpc"];
			JME_RPCManager::getInstance()->initRpcServer(rpc);

			const auto& _baseConf = _config["base"];

			if (_baseConf.isMember("acceptor"))
			{
				const auto& acceptConf = _baseConf["acceptor"];

				unsigned short port = acceptConf["port"].asUInt();
				_acceptorPtr = JMEngine::net::JME_TcpAcceptor::create(_clientHandler, port);

				size_t bufferSize = acceptConf["buffer_size"].asUInt();
				_acceptorPtr->accept(0, bufferSize);

				LogI << "Start accept connector on port{" << port << "}" << LogEnd;
			}

			LogT << "Server [ " << _serverName << " ] start complete" << LogEnd;
		}

		void JME_ServerInterface::init()
		{

		}
	}
}