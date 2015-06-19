#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <map>
#include <deque>
#include "msg_base.h"
#include <json/json.h>
#include "net_handler.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/detail/singleton.hpp>
#include "conn_state.h"

using boost::asio::ip::tcp;


namespace hoyosvr
{
	struct bindMsg
	{
		int netId;	// net of socket that the msg come from
		msg::hoyo_msg::ptr msg;
	};

	class BindKey
	{
	public:
		int server_id;
		int server_index;

		bool operator < (const BindKey& key) const
		{
			if(this->server_id < key.server_id)
				return true;
			if(this->server_id > key.server_id)
				return false;
			if(this->server_index < key.server_index)
				return true;
			if(this->server_index > key.server_index)
				return false;
			return false;
		}
	};

	namespace net
	{
		class tcp_session;
		class ServerInterface : 
			public boost::enable_shared_from_this<ServerInterface>,
			public net_handler
		{
		public:
			typedef boost::shared_ptr<hoyosvr::net::tcp_session>		tcp_session_ptr;
			typedef std::map<int,tcp_session_ptr>					client_map;					
			typedef	std::deque<bindMsg>								msg_queue;
			typedef boost::shared_ptr< net_handler>					net_handler_ptr;
			typedef boost::shared_ptr<tcp::acceptor>				acceptor_pointer;

			typedef boost::shared_ptr<boost::asio::io_service>			io_service_ptr;
			typedef boost::shared_ptr<boost::thread>					thread_ptr;
			typedef boost::shared_ptr<boost::asio::io_service::work>	work_ptr;

			ServerInterface(void);
			virtual ~ServerInterface(void);

			void init();
			void run();
			void stop();

			inline bool is_stop() { return _bStop;}

			virtual	void on_disconnect(int client_id,tcp_session_ptr conn_ptr);
			virtual	void sync_process_recv_msg(int net_id,const msg::hoyo_msg::ptr recv_msg_ptr);
			virtual	bool async_process_recv_msg(tcp_session_ptr conn,const msg::hoyo_msg::ptr recv_msg_ptr);

			virtual void onInit();
			virtual	void onUpdate();
			virtual void onStop(){}


			void recv_client_handler(tcp_session_ptr session, const char* data_ptr,int len);
			void recv_client_handler(tcp_session_ptr session, msg::hoyo_msg& m);
			void server_connect_handler(tcp_session_ptr session, int error_value);
			void client_connect_handler(tcp_session_ptr session, int error_value);

			void sendHeartbeat(tcp_session_ptr session);
			void sendHeartbeat(int netId);
			void recvHeartbeat(int netId);
			void connectorHeartDead(int netId);

			void sendToServer(const char* server, msg::hoyo_msg& msg);
			void sendToServer(int net_id, msg::hoyo_msg& msg);

			void connectorLost(int net_id);

			void connectServer(int net_id, int server_id, std::string& server_name, std::string& ip_str, std::string& port_str);

			int getNetByServerIdx(int serverId, int serverIdx);	//通过serverId 与 serverIndex获取网关netId
			void remNetByServerIdx(int serverId, int serverIdx);	//移除绑定

			int getServerId();

			boost::asio::io_service& getLogicIoService();
		private:
			void startAccept();
			void handleAccept(tcp_session_ptr new_connection,acceptor_pointer ap,const boost::system::error_code& error);

			void initThread();
			void update();		
			
			void handleConnectServer(tcp_session_ptr connector,const boost::system::error_code& error);

			void bindGateServer(msg::hoyo_msg::ptr p, tcp_session_ptr session);
		protected:
			client_map _clientMap;		// all 'clients' connect to self
			int _clientCount;	// auto increase client index
			net_handler_ptr _clientHandler;	// handler for client
			boost::recursive_mutex	_clientMutex;

			Json::Value _configs;		// server config data

			msg_queue _msgQueue;		// msg deque
			boost::mutex _queueMutex;

			acceptor_pointer _acceptor;	// socket acceptor

			client_map				_serverInside; // all inside servers
			boost::mutex			_serverMutex;
			net_handler_ptr			_serverHandler;	// handler for server

			bool					_bStop;	// server stop?

			std::map<BindKey,int>		_gateBindServer;	// bind data for gate-id and net-id

			hoyosvr::net::ServerManager* _serverManager;	// manager for inside servers

		private:
			//net
			io_service_ptr _netIoService;	
			thread_ptr _netThread;
			work_ptr _netWork;
		public:
			//logic
			io_service_ptr _logicIoService;
		private:
			thread_ptr _logicThread;
			work_ptr _logicWork;
		};
	}
}




