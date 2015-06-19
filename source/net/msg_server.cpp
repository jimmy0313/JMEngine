#include "msg_server.h"
#include "tcp_session.h"
#include <file_system.h>
#include "GLog.h"
#include <time_helper.h>
#include <boost/lexical_cast.hpp>
#include <tools.h>
#include "msg_base.h"

const int MB_UNIT = 1024 * 1024;
const int KB_UNIT = 1024;

namespace hoyosvr
{
	namespace net
	{

		ServerInterface::ServerInterface(void):
			_bStop(false),
			_clientCount(1000)
		{
			initThread();
		}


		ServerInterface::~ServerInterface(void)
		{
		}

		void ServerInterface::init()
		{
			_configs = hoyosvr::file_system::load_jsonfile_val("./config/server_cfg.json");
			Json::Value serversCfg = hoyosvr::file_system::load_jsonfile_val("./config/servers.json");

 			_serverManager = new hoyosvr::net::ServerManager(this);
 			_serverManager->initServerState(serversCfg);
			
			onInit();

			int port = _configs["server_port"].asInt();
			_acceptor = acceptor_pointer(new tcp::acceptor(*_netIoService, tcp::endpoint(tcp::v4(), port)));

			LogT << "start accept client on port --- " << color_green(port) << LogEnd;
			startAccept();
			
			
			_logicIoService->post(boost::bind(&ServerInterface::update,this));
		}

		void ServerInterface::run()
		{
			_netThread = thread_ptr(new boost::thread(boost::bind(&boost::asio::io_service::run, _netIoService)));
			_logicThread = thread_ptr(new boost::thread(boost::bind(&boost::asio::io_service::run, _logicIoService)));
		}

		void ServerInterface::stop()
		{			
			_bStop = true;

			onStop();

			_netIoService->stop();
			_logicIoService->stop();
			_logicThread->join();
			_netThread->join();
		}

		void ServerInterface::on_disconnect( int client_id,tcp_session_ptr conn_ptr )
		{
			boost::recursive_mutex::scoped_lock lock(_clientMutex);

			auto it = _clientMap.find(client_id);
			if(it != _clientMap.end())
			{
				_clientMap.erase(it);
				LogI <<  "Client [ " << client_id << " ] disconnected ..." << LogEnd;
			}
		}

		void ServerInterface::sync_process_recv_msg( int net_id, const msg::hoyo_msg::ptr recv_msg_ptr )
		{

		}

		bool ServerInterface::async_process_recv_msg(tcp_session_ptr conn, const msg::hoyo_msg::ptr recv_msg_ptr)
		{
			return false;
		}

		void ServerInterface::onUpdate()
		{
		}

		void ServerInterface::handleAccept(tcp_session_ptr session,acceptor_pointer ap,const boost::system::error_code& error)
		{
			if (!error)
			{
				{
					boost::recursive_mutex::scoped_lock lock(_clientMutex);
					_clientMap[++_clientCount] = session;
				}
				try
				{
					std::string  ip_add = session->socket().remote_endpoint().address().to_string();
					LogI <<  "New client [ " << _clientCount << " ] connected ...ip: " << ip_add << LogEnd;
				}
				catch (std::exception& e)
				{
					LogE << __FUNCTION__ << " handler accept error: " << e.what() << LogEnd;
				}
				session->start(_clientCount,shared_from_this(), *_logicIoService);

				startAccept();
			}
			else
			{
				try
				{
					session->stop();
					std::string  ip_add = session->socket().remote_endpoint().address().to_string();
					LogE <<  __FUNCTION__ << " " << error.message() <<  "\tip:" << ip_add.c_str() << LogEnd;
				}
				catch (std::exception& e)
				{
					LogE << __FUNCTION__ << " error: " << e.what() << LogEnd;
				}
				startAccept();
			}
		}
		void ServerInterface::startAccept()
		{			
			size_t n = _configs["client_buffer_size"].asUInt(); //byte 
			n *= KB_UNIT;//K
			tcp_session_ptr new_connection = tcp_session::create( *_netIoService, n ); // n(M)

			_acceptor->async_accept(new_connection->socket(),
				boost::bind(&ServerInterface::handleAccept,this, new_connection,_acceptor,
				boost::asio::placeholders::error));
		}


		void ServerInterface::recv_client_handler( tcp_session_ptr session,const char* data_ptr,int len )
		{
			if ( len < msg::msg_offset ) 
				return;
			msg::hoyo_msg::ptr p = msg::hoyo_msg::create(data_ptr,len);

			if ( p->_type == SERVERBIND )
			{
				return this->bindGateServer(p, session);
			}
			// lock lock lock !!!!
			if ( HEARTBEAT == p->_type )
			{
				this->sendHeartbeat(session);
			}
			else if ( !async_process_recv_msg(session,p) )				// process received message by async mode firstly
			{
				// in case need to process message by sync mode
				// we put it in the message queue firtly
				// another logic io_service update will process them one by one

				bindMsg msg;
				msg.msg = p;
				msg.netId = session->get_net_id();

				boost::mutex::scoped_lock lock(_queueMutex);
				_msgQueue.push_back(msg);		
			}
		}

		void ServerInterface::recv_client_handler( tcp_session_ptr session, msg::hoyo_msg& m )
		{
			_clientHandler->recv_client_handler(session,m);
		}

		// should disconnect from client in case
		void ServerInterface::client_connect_handler( tcp_session_ptr session,int error_value )
		{
			on_disconnect(session->get_net_id(),session);
		}

		void ServerInterface::onInit()
		{

		}

		void ServerInterface::update()
		{
			if(_bStop) 
				return;

			static boost::system_time start_time;
			start_time = boost::get_system_time();
			
			// main loop
			while (1)
			{
				// lock lock lock
				boost::mutex::scoped_lock lock(_queueMutex);
				if (!_msgQueue.empty())
				{				
					int net_id = _msgQueue.front().netId;
					msg::hoyo_msg::ptr p = _msgQueue.front().msg;
					_msgQueue.pop_front();

					// unlock right now
					lock.unlock();

					// process the message now
					boost::system_time startTime = boost::get_system_time();
					sync_process_recv_msg(net_id,p);
					boost::system_time endTime = boost::get_system_time();
					int lapseTime = (int)((endTime - startTime).total_milliseconds());
					if ( lapseTime > 1 )
					{
						string t = boost::lexical_cast<string>(lapseTime);
						LogD << "client: " << p->_player_id << " cmd [" << p->_type << "] consume time : " << color_red(t) << LogEnd;
					}
				}
				else
					break;
				
				// don't spend too much time
				boost::system_time end_time = boost::get_system_time();
				if((end_time - start_time).total_milliseconds() >= 100)
				{
					break;
				}
			}			

			_serverManager->run();

			onUpdate();

			// sleep 1 micro second
			hoyosvr::time_helper::sleepMicroSec(1);

			_logicIoService->post(boost::bind(&ServerInterface::update,this));
		}

		void ServerInterface::sendHeartbeat( tcp_session_ptr session )
		{
			msg::hoyo_msg m(HEARTBEAT);
			session->write_hoyo_msg(m);
		}

		void ServerInterface::sendHeartbeat( int netId )
		{
			msg::hoyo_msg m(HEARTBEAT);
			sendToServer(netId, m);
		}

		void ServerInterface::connectServer(int net_id, int server_id, std::string& server_name, std::string& ip_str, std::string& port_str)
		{
			int size = _configs["buffer_size"].asUInt();
			tcp_session_ptr conn_ptr = tcp_session::create( *_netIoService, size * MB_UNIT );
			conn_ptr->set_net_id(net_id);
			conn_ptr->set_server_id(server_id);

// 			LogI << "Connect to server: " << server_name << ", ip: " << ip_str << ", port: " << port_str << LogEnd;

			tcp::resolver resolver(conn_ptr->socket().get_io_service());
			tcp::resolver::query query(ip_str, port_str);
			tcp::resolver::iterator iterator = resolver.resolve(query);	

			boost::asio::async_connect(conn_ptr->socket(), iterator,
				boost::bind(&ServerInterface::handleConnectServer,shared_from_this(),conn_ptr,
				boost::asio::placeholders::error));
		}

		void ServerInterface::handleConnectServer( tcp_session_ptr connector,const boost::system::error_code& error )
		{
			if (!error)
			{
				boost::mutex::scoped_lock lock(_serverMutex);
				_serverInside[connector->get_net_id()] = connector;

				connector->start(connector->get_net_id(),_serverHandler, *_logicIoService);

				_serverManager->connectSucceed(connector->get_net_id());
				_serverHandler->server_connect_handler(connector,error.value());
			}
			else
			{
				_serverManager->connectFailed(connector->get_net_id());
			}
		}

		void ServerInterface::server_connect_handler( tcp_session_ptr session,int error_value )
		{

		}

		void ServerInterface::sendToServer( const char* server, msg::hoyo_msg& msg )
		{
			int netId = _serverManager->getNetIdByServerName(server);
			sendToServer( netId, msg );
		}

		void ServerInterface::sendToServer( int net_id, msg::hoyo_msg& msg )
		{
			boost::mutex::scoped_lock lock(_serverMutex);

			auto it = _serverInside.find(net_id);
			if ( it != _serverInside.end() )
			{
				it->second->write_hoyo_msg(msg);
			}
		}

		void ServerInterface::connectorLost( int net_id )
		{
			_serverManager->connectorLost(net_id);
		}

		void ServerInterface::initThread()
		{
			_netIoService = io_service_ptr(new boost::asio::io_service);
			_netWork = work_ptr(new boost::asio::io_service::work(*_netIoService));

			_logicIoService = io_service_ptr(new boost::asio::io_service);
			_logicWork = work_ptr(new boost::asio::io_service::work(*_logicIoService));
		}

		void ServerInterface::recvHeartbeat( int netId )
		{
			_serverManager->connectorHeartBeat(netId);
		}

		void ServerInterface::bindGateServer( msg::hoyo_msg::ptr p, tcp_session_ptr session )
		{
			string m(p->_msg_str_utf8, p->_total_len - msg::msg_offset);
			Json::Reader r;
			Json::Value j;
			r.parse(m, j);
			BindKey key;
			key.server_id = j["server_id"].asInt();
			key.server_index  = j["server_index"].asInt();

			int net = session->get_net_id();
			session->set_server_id(key.server_id);
			session->set_server_index(key.server_index);

			auto it = _gateBindServer.find(key);
			if(it != _gateBindServer.end())
				_gateBindServer.erase(it);
			_gateBindServer.insert(make_pair(key,net));
		}

		int ServerInterface::getNetByServerIdx( int serverId, int serverIdx )
		{
			BindKey key;
			key.server_id = serverId;
			key.server_index = serverIdx;

			auto it = _gateBindServer.find(key);
			return it != _gateBindServer.end() ? it->second : 0;
		}

		void ServerInterface::remNetByServerIdx( int serverId, int serverIdx )
		{
			BindKey key;
			key.server_id = serverId;
			key.server_index = serverIdx;

			auto it = _gateBindServer.find(key);
			if (it != _gateBindServer.end())
				_gateBindServer.erase(it);
		}

		int ServerInterface::getServerId()
		{
			return _configs["server_id"].asInt();
		}

		boost::asio::io_service& ServerInterface::getLogicIoService()
		{
			return *this->_logicIoService;
		}

		void ServerInterface::connectorHeartDead( int netId )
		{
			boost::mutex::scoped_lock lock(_serverMutex);

			auto it = _serverInside.find(netId);
			if ( it != _serverInside.end() )
			{
				it->second->stop();
			}
		}

	}
}

