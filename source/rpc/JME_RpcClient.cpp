#include "JME_RpcClient.h"
#include "JME_Message.h"
#include "JME_Rpc.h"
#include "JME_Core.h"
using namespace JMEngine;
using namespace JMEngine::net;
namespace JMEngine
{
	namespace rpc
	{

		JME_RpcCallback::JME_RpcCallback( RpcHandler cb ):
			_cb(cb),
			_checkDead(false)
		{
		}

		JME_RpcCallback::JME_RpcCallback( RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId ):
			_cb(cb),
			_checkDead(true),
			_methodId(methodId)
		{
			_dt = DeadTimePtr(new boost::asio::deadline_timer(JMECore.getLogicioService()));
			_dt->expires_from_now(boost::posix_time::seconds(t));
			_dt->async_wait(boost::bind(dcb, boost::asio::placeholders::error, methodId));
		}

		JME_RpcCallback::JME_RpcCallbackPtr JME_RpcCallback::create( RpcHandler cb )
		{
			return JME_RpcCallbackPtr(new JME_RpcCallback(cb));
		}

		JME_RpcCallback::JME_RpcCallbackPtr JME_RpcCallback::create( RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId )
		{
			return JME_RpcCallbackPtr(new JME_RpcCallback(cb, t, dcb, methodId));
		}

		JME_RpcClient::JME_RpcClient( const string& ip, const string& port, size_t buffSize, size_t reconnect ):
			_methodId(0)
		{
			_session = JMEngine::net::JME_TcpSession::create(JME_RpcClientPtr(this), buffSize, reconnect);
			_session->connect(ip, port);
		}

		JME_RpcClient::~JME_RpcClient()
		{
			_session->stop();
		}

		JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr JME_RpcClient::create( const string& ip, const string& port, size_t buffSize, size_t reconnect )
		{
			return JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr(new JME_RpcClient(ip, port, reconnect, buffSize));
		}

		bool JME_RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb )
		{
			if(!_session->isOk())
			{
				LogE << "Remote server is not connected" << LogEnd;
				return false;
			}

			try
			{
				boost::mutex::scoped_lock lock(_mutex);

				JME_Rpc r(++_methodId, method, params);
				string m = r.serializeAsString();
				JME_Message msg(1, m);	
				_cbs[_methodId] = JME_RpcCallback::create(cb);

				return _session->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LogE << "Call rpc function failed, error: " << e.what() << LogEnd;
			}
			return false;
		}

		bool JME_RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb, size_t dt, JME_RpcCallback::RpcDeadHandler dcb )
		{
			try
			{
				boost::mutex::scoped_lock lock(_mutex);

				JME_Rpc r(++_methodId, method, params);
				string m = r.serializeAsString();
				JME_Message msg(1, m);	
				_cbs[_methodId] = JME_RpcCallback::create(cb, dt, dcb, _methodId);

				return _session->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LogE << "Call rpc function failed, error: " << e.what() << LogEnd;
			}
			return false;
		}

		void JME_RpcClient::sessionConnectSucceed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session )
		{
			_session->start(1);

			LogT << "RPC server{" << _session->getIp() << ":" << _session->getPort() << "}" << " connect succeed" << LogEnd;
		}

		void JME_RpcClient::sessionConnectFailed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			LogW << "Connect to RPC server {" << session->getIp() << ":" << _session->getPort() << "} failed" << LogEnd;
		}

		void JME_RpcClient::sessionDisconnect( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			session->reconnect();
		}

		void JME_RpcClient::sessionReceiveMessage( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JME_MessagePtr msg )
		{			
			string m = msg->getMessageStr();
			JME_Rpc r(m);

			boost::mutex::scoped_lock lock(_mutex);
			
			auto it = _cbs.find(r._rpcId);
			if (it != _cbs.end())
			{
				if (it->second->_checkDead)
					it->second->_dt->cancel();

				it->second->_cb(r);

				_cbs.erase(it);
			}
		}

		void JME_RpcClient::sessionReadError( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			session->resetReadBuffer();
			session->reconnect();
		}

		void JME_RpcClient::removeDeadRPC( int methodId )
		{
			boost::mutex::scoped_lock lock(_mutex);

			auto it = _cbs.find(methodId);
			if (it != _cbs.end())
			{
				_cbs.erase(it);
			}
		}
	}
}