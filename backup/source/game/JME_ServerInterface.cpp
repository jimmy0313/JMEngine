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

			const auto& rpc = _config["rpc"];
			const auto& clientConf = rpc["client"];
			for (const auto& c : clientConf)
			{
				auto rpc = JME_RpcClient::create(c["ip"].asString(), c["port"].asString(), c["reconnect"].asUInt(), c["buffer_size"].asUInt());
				_rpcClient[c["name"].asString()] = rpc;
			}
			init();

			init_();
		}

		void JME_ServerInterface::init_()
		{
			const auto& _baseConf = _config["base"];

			if (_baseConf.isMember("remote"))
			{
				const auto& remoteConf = _baseConf["remote"];

				for (auto& it : remoteConf)
				{
					unsigned short port = it["port"].asUInt();
					string ip = it["ip"].asString();
					size_t bufferSize = it["buffer_size"].asUInt();
					size_t reconnect = it["reconnect"].asUInt();

					string name = it["server"].asString();

					auto c = JME_TcpSession::create(_serverHandler, bufferSize, reconnect);
					c->connect(ip, boost::lexical_cast<string>(port));

					_servers[name] = c;
				}
			}

			if (_baseConf.isMember("acceptor"))
			{
				const auto& acceptConf = _baseConf["acceptor"];

				unsigned short port = acceptConf["port"].asUInt();
				_acceptorPtr = JMEngine::net::JME_TcpAcceptor::create(_clientHandler, port);

				size_t bufferSize = acceptConf["buffer_size"].asUInt();
				_acceptorPtr->accept(0, bufferSize);

				LogI << "Start accept connector on port{" << port << "}" << LogEnd;
			}
		}

		void JME_ServerInterface::init()
		{

		}

		JME_RpcClient::JME_RpcClientPtr JME_ServerInterface::getRpcByName( const char* name )
		{
			auto it = _rpcClient.find(name);

			return it != _rpcClient.end() ? it->second : JME_RpcClientPtr();
		}

	}
}