#ifndef conn_state_h__
#define conn_state_h__

#include "boost/shared_ptr.hpp"
#include <boost/thread/thread.hpp>
#include <map>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>
#include <string>
#include "net_handler.h"

namespace Json
{
	class Value;
}
namespace hoyosvr
{
	namespace net
	{
		enum SocketState
		{
			State_Idle				=		1,		//空闲
			State_Disconnect		=		2,		//断开
			State_Check				=		3,		//检测
			State_Connecting		=		4,		//连接中
			State_Waiting			=		5,		//等待回复
		};

		class State
		{
		public:
			State();
			virtual ~State();
		public:
			int						_netId;
			int						_serverId;
			std::string				_serverType;
			std::string				_serverName;
			std::string				_serverIp;
			std::string				_serverPort;
			SocketState				_serverState;
			int						_tryTimes;
			boost::system_time		_stateTime;
			int						_failedTimes;
			int						_checkTimes;
		};
		typedef std::map<int,State>			ServersState;

		class ServerInterface;
		class ServerManager
		{
		public:
			ServerManager(ServerInterface* server);
			~ServerManager();

			void initServerState(Json::Value& cfg);
			void connectSucceed(int netId);
			void connectFailed(int netId);
			void connectorLost(int netId);
			void connectorHeartBeat(int netId);
			int getNetIdByServerName(const char* name);
			void getNetsByServerType(const char* name, std::vector<int>& nlist);

			void run();

		private:
			ServersState _states;
			boost::recursive_mutex _metux;
			ServerInterface* _server;
		};
	}
}
#endif // conn_state_h__
