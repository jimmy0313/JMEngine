#include "conn_state.h"
#include "msg_server.h"
#include <GLog.h>
#include "json/json.h"

namespace hoyosvr
{
	namespace net
	{

		State::State():
			_netId(0),
			_serverType(""),
			_serverName(""),
			_serverIp(""),
			_serverPort(""),
			_serverState(State_Idle),
			_tryTimes(0),
			_failedTimes(0),
			_checkTimes(10)
		{

		}

		State::~State()
		{

		}

		void ServerManager::connectSucceed( int netId )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);

			auto it = _states.find(netId);
			if ( it != _states.end() )
			{
				it->second._serverState = State_Idle;
				it->second._tryTimes = 0;
				it->second._stateTime = boost::get_system_time();

				LogI << "Connect to server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ] succeed" << LogEnd;
			}
		}

		void ServerManager::connectFailed( int netId )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);

			auto it = _states.find(netId);
			if ( it != _states.end() )
			{
				it->second._serverState = State_Disconnect;
				it->second._tryTimes = 0;
				it->second._stateTime = boost::get_system_time();

				LogE << "Connect to server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ] failed" << LogEnd;
			}
		}

		void ServerManager::run()
		{
			boost::recursive_mutex::scoped_lock lock(_metux);
			for ( auto it = _states.begin(); it != _states.end(); ++it )
			{
				State* s = (State*) (&it->second);

				boost::system_time	check_time = boost::get_system_time();
				if ( State_Disconnect == s->_serverState )		//检测是否需要重连
				{
					int lapseTime = (int)((check_time - s->_stateTime).total_milliseconds());
					if ( lapseTime >= 5 * 1000 )
					{
						LogI << "Reconnect to server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ]" << LogEnd;

						_server->connectServer(s->_netId,s->_serverId,s->_serverName,s->_serverIp,s->_serverPort);

						s->_serverState = State_Connecting;
						s->_stateTime = boost::get_system_time();
					}
					continue;
				}
				else if ( State_Idle == s->_serverState )	// 空闲
				{
					int lapseTime = (int)((check_time - s->_stateTime).total_milliseconds());
					if ( lapseTime >= 10 * 1000 )
					{
						s->_serverState = State_Check;
					}
					continue;
				}
				else if ( State_Check == s->_serverState )		// 检测socket是否正常
				{
					if ( s->_failedTimes == s->_checkTimes/2 )
					{
						LogW << "Server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ] heartbeat failed for " << s->_failedTimes << " times, check it!!!" << LogEnd;
					}
					if ( s->_failedTimes >= s->_checkTimes )
					{
						LogW << "Server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ] heartbeat failed for " << s->_failedTimes << " times, try to reconnect it!!!" << LogEnd;

						_server->connectorHeartDead(s->_netId);
						s->_serverState = State_Disconnect;
						s->_failedTimes = 0;
					}
					else
					{
						_server->sendHeartbeat(s->_netId);
						s->_serverState = State_Waiting;
						s->_stateTime = boost::get_system_time();
					}
					continue;
				}
				else if ( State_Waiting == s->_serverState )	// 等待回复检测消息
				{
					int lapseTime = (int)((check_time - s->_stateTime).total_milliseconds());
					if ( lapseTime >= 5 * 1000 )
					{
						s->_serverState = State_Check;
						s->_failedTimes++;
					}
					continue;
				}
				else if ( State_Connecting == s->_serverState )	// 等待连接结果
				{
					int lapseTime = (int)((check_time - s->_stateTime).total_milliseconds());
					if ( lapseTime >= 30 * 1000 )
					{
						s->_serverState = State_Disconnect;
					}
					continue;
				}
			}
		}

		void ServerManager::connectorLost( int net_id )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);
			auto it = _states.find(net_id);
			if ( it != _states.end() )
			{
				LogW << " Lost connector for server [ " << it->second._serverName << " -- " << it->second._serverIp << ": " << it->second._serverPort << " ]" << LogEnd;
				it->second._serverState = State_Disconnect;
				it->second._stateTime = boost::get_system_time();
			}
		}

		void ServerManager::connectorHeartBeat( int netId )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);
			auto it = _states.find(netId);
			if ( it != _states.end() )
			{
				it->second._serverState = State_Idle;
				it->second._failedTimes = 0;
				it->second._stateTime = boost::get_system_time();
			}
		}

		void ServerManager::initServerState( Json::Value& cfg )
		{
			for (auto it = cfg.begin(); it != cfg.end(); ++it)
			{
				std::string serverType = it.key().asString();
				
				for(auto itp = (*it).begin(); itp != (*it).end(); ++itp)
				{
					Json::Value mVal = *itp;

					int net_id = atoi(itp.key().asCString());
					State state;
					state._netId = net_id;
					state._serverId = mVal["server_id"].asInt();
					state._serverType = serverType;
					state._serverName = mVal["server"].asString();
					state._serverIp = mVal["ip"].asString();
					state._serverPort = mVal["port"].asString();
					state._serverState = State_Disconnect;
					state._tryTimes = 0;
					state._stateTime = boost::get_system_time();
					state._checkTimes = max(10,mVal["check_times"].asInt());
					_states[net_id] = state;
				}
			}
		}

		int ServerManager::getNetIdByServerName( const char* name )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);
			for (auto it = _states.begin(); it != _states.end(); ++it)
			{
				State* s = (State*) (&it->second);
				if ( !s->_serverName.compare(name) )
					return s->_netId;
			}
			return -1;
		}

		void ServerManager::getNetsByServerType( const char* name, std::vector<int>& nlist )
		{
			boost::recursive_mutex::scoped_lock lock(_metux);
			for (auto it = _states.begin(); it != _states.end(); ++it)
			{
				State* s = (State*) (&it->second);
				if (!s->_serverType.compare(name))
				{
					nlist.push_back(s->_netId);
				}
			}
		}

		ServerManager::ServerManager( ServerInterface* server ):
			_server(server)
		{

		}

		ServerManager::~ServerManager()
		{

		}

	}
}