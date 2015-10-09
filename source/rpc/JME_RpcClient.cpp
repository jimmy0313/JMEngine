#include "JME_RpcClient.h"
#include "JME_Message.h"
#include "JME_Core.h"
#include "rpc.pb.h"

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

		JME_RpcCallback::JME_RpcCallback( JME_RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId ):
			_cb(cb),
			_checkDead(true)
		{
			_dt = DeadTimePtr(new boost::asio::deadline_timer(JMECore.getLogicioService()));
			_dt->expires_from_now(boost::posix_time::seconds(t));
			_dt->async_wait(boost::bind(JME_RpcClient::RpcDeadCallback, client, boost::asio::placeholders::error, methodId, dcb));
		}

		JME_RpcCallback::JME_RpcCallbackPtr JME_RpcCallback::create( RpcHandler cb )
		{
			return JME_RpcCallbackPtr(new JME_RpcCallback(cb));
		}

		JME_RpcCallback::JME_RpcCallbackPtr JME_RpcCallback::create( JME_RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId )
		{
			return JME_RpcCallbackPtr(new JME_RpcCallback(client, cb, t, dcb, methodId));
		}

		JME_RpcClient::JME_RpcClient( const string& ip, const string& port, size_t buffSize, size_t reconnect ):
			_methodId(0)
		{
			auto session = JMEngine::net::JME_TcpSession::create(this, buffSize, reconnect);
			_session = session;
			session->connect(ip, port);
		}

		JME_RpcClient::~JME_RpcClient()
		{
			if (!_session.expired())
			{
				auto session = _session.lock();
				session->stop();
			}
		}

		JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr JME_RpcClient::create( const string& ip, const string& port, size_t buffSize, size_t reconnect )
		{
			return JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr(new JME_RpcClient(ip, port, reconnect, buffSize));
		}

		bool JME_RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb )
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
					_cbs[_methodId] = JME_RpcCallback::create(cb);
				}

				rpc.set_method(method);
				rpc.set_params(params->SerializeAsString());

				auto m(boost::move(rpc.SerializeAsString()));
				JME_Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		bool JME_RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb, size_t dt, JME_RpcCallback::RpcDeadHandler dcb )
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
					_cbs[_methodId] = JME_RpcCallback::create(shared_from_this(), cb, dt, dcb, _methodId);
				}

				rpc.set_method(method);
				rpc.set_params(params->SerializeAsString());

				auto m(boost::move(rpc.SerializeAsString()));
				JME_Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		bool JME_RpcClient::callRpcMethod(const char* method, const google::protobuf::Message* params)
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
				JME_Message msg(RPCMessage, m);	

				return _session.lock()->writeMessage(msg);
			}
			catch(std::exception& e)
			{
				LOGE("Call rpc function failed, error ==> [ %s ]", e.what());
			}
			return false;
		}

		void JME_RpcClient::sessionConnectSucceed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session )
		{
			session->start(RPCSession);

			LOGI("Connect to rpc server [ %s:%s ] succeed", session->getIp(), session->getPort());
		}

		void JME_RpcClient::sessionConnectFailed( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			LOGW("Connect to rpc server [ %s:%s ] failed, error ==> [ %d:%s ]", session->getIp(), session->getPort(), e.value(), e.message());
		}

		void JME_RpcClient::sessionDisconnect( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			session->reconnect();
		}

		void JME_RpcClient::sessionReceiveMessage( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JME_MessagePtr msg )
		{			
			string m = msg->getMessageStr();
			jme_rpc rpc;
			rpc.ParseFromString(m);

			JME_RpcCallback::JME_RpcCallbackPtr cb;

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

		void JME_RpcClient::sessionReadError( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			session->resetReadBuffer();
			session->reconnect();
		}

		void JME_RpcClient::removeDeadRPC( int methodId )
		{
			boost::recursive_mutex::scoped_lock lock(_mutex);

			auto it = _cbs.find(methodId);
			if (it != _cbs.end())
			{
				_cbs.erase(it);
			}
		}

		void JME_RpcClient::RpcDeadCallback( JME_RpcClientPtr client, const boost::system::error_code& err, int methodId, JME_RpcCallback::RpcDeadHandler dcb )
		{
			if (err)
				return;

			client->removeDeadRPC(methodId);
			dcb();
		}
	}
}