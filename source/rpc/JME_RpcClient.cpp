#include "rpc/JME_RpcClient.h"
#include "net/JME_Message.h"
#include "net/JME_Core.h"
#include "rpc/JME_RpcPb.h"

using namespace JMEngine;
using namespace JMEngine::net;
namespace JMEngine
{
	namespace rpc
	{

		RpcCallback::RpcCallback( RpcHandler cb ):
			_cb(cb),
			_checkDead(false)
		{
		}

		RpcCallback::RpcCallback( RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId ):
			_cb(cb),
			_checkDead(true)
		{
			_dt = DeadTimePtr(new boost::asio::deadline_timer(JMECore.getLogicioService()));
			_dt->expires_from_now(boost::posix_time::seconds(t));
			_dt->async_wait(boost::bind(RpcClient::RpcDeadCallback, client, boost::asio::placeholders::error, methodId, dcb));
		}

		RpcCallback::RpcCallbackPtr RpcCallback::create( RpcHandler cb )
		{
			return RpcCallbackPtr(new RpcCallback(cb));
		}

		RpcCallback::RpcCallbackPtr RpcCallback::create( RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId )
		{
			return RpcCallbackPtr(new RpcCallback(client, cb, t, dcb, methodId));
		}

		RpcClient::RpcClient( const string& ip, const string& port, size_t buffSize, size_t reconnect ):
			_methodId(0)
		{
			auto session = JMEngine::net::TcpSession::create(this, buffSize, reconnect);
			_session = session;
			session->connect(ip, port);
		}

		RpcClient::~RpcClient()
		{
			if (!_session.expired())
			{
				auto session = _session.lock();
				session->stop();
			}
		}

		JMEngine::rpc::RpcClient::RpcClientPtr RpcClient::create( const string& ip, const string& port, size_t buffSize, size_t reconnect )
		{
			return JMEngine::rpc::RpcClient::RpcClientPtr(new RpcClient(ip, port, reconnect, buffSize));
		}

		bool RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, RpcCallback::RpcHandler cb )
		{
			if(!_session.lock()->isOk())
			{
				LOGE("Remote server is not connected");
				return false;
			}

			try
			{
				jme_rpc rpc;

				{
					boost::recursive_mutex::scoped_lock lock(_mutex);
					rpc.set_rpc_id(++_methodId);
					_cbs[_methodId] = RpcCallback::create(cb);
				}

				rpc.set_method(method);
				rpc.set_params(params->SerializeAsString());

				auto m(boost::move(rpc.SerializeAsString()));
				Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		bool RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, RpcCallback::RpcHandler cb, size_t dt, RpcCallback::RpcDeadHandler dcb )
		{
			if(!_session.lock()->isOk())
			{
				LOGE("Remote server is not connected");
				return false;
			}

			try
			{
				jme_rpc rpc;

				{
					boost::recursive_mutex::scoped_lock lock(_mutex);
					rpc.set_rpc_id(++_methodId);
					_cbs[_methodId] = RpcCallback::create(shared_from_this(), cb, dt, dcb, _methodId);
				}

				rpc.set_method(method);
				rpc.set_params(params->SerializeAsString());

				auto m(boost::move(rpc.SerializeAsString()));
				Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		bool RpcClient::callRpcMethod(const char* method, const google::protobuf::Message* params)
		{
			if(!_session.lock()->isOk())
			{
				LOGE("Remote server is not connected");
				return false;
			}

			try
			{
				jme_rpc rpc;
				{
					boost::recursive_mutex::scoped_lock lock(_mutex);
					rpc.set_rpc_id(++_methodId);
				}
				rpc.set_method(method);
				rpc.set_params(params->SerializeAsString());

				auto m(boost::move(rpc.SerializeAsString()));
				Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		void RpcClient::sessionConnectSucceed( JMEngine::net::TcpSession::TcpSessionPtr session )
		{
			session->start(RPCSession);

			LOGI("Connect to rpc server [ %s:%s ] succeed", session->getIp(), session->getPort());
		}

		void RpcClient::sessionConnectFailed( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{
			LOGW("Connect to rpc server [ %s:%s ] failed, error ==> [ %d:%s ]", session->getIp(), session->getPort(), e.value(), e.message());
		}

		void RpcClient::sessionDisconnect( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{
			session->reconnect();
		}

		void RpcClient::sessionReceiveMessage( JMEngine::net::TcpSession::TcpSessionPtr session, MessagePtr msg )
		{			
			string m = msg->getMessageStr();
			jme_rpc rpc;
			rpc.ParseFromString(m);

			RpcCallback::RpcCallbackPtr cb;

			{
				boost::recursive_mutex::scoped_lock lock(_mutex);
				auto it = _cbs.find(rpc.rpc_id());
				if (it != _cbs.end())
				{
					cb = it->second;
					_cbs.erase(it);
				}
			}
			if (cb)
			{
				if (cb->_checkDead)
					cb->_dt->cancel();

				cb->_cb(rpc.params());
			}
		}

		void RpcClient::sessionReadError( JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e )
		{
			session->resetReadBuffer();
			session->reconnect();
		}

		void RpcClient::removeDeadRPC( int methodId )
		{
			boost::recursive_mutex::scoped_lock lock(_mutex);

			auto it = _cbs.find(methodId);
			if (it != _cbs.end())
			{
				_cbs.erase(it);
			}
		}

		void RpcClient::RpcDeadCallback( RpcClientPtr client, const boost::system::error_code& err, int methodId, RpcCallback::RpcDeadHandler dcb )
		{
			if (err)
				return;

			client->removeDeadRPC(methodId);
			dcb();
		}
	}
}