#include "JME_IPM_RpcClient.h"

using namespace JMEngine;
using namespace JMEngine::net;
namespace JMEngine
{
	namespace rpc
	{
		JME_IPM_RPCCallback::JME_IPM_RPCCallback( JME_IPM_RPCCallback::RpcHandler cb ):
			_cb(cb),
			_checkDead(false)
		{
		}

		JME_IPM_RPCCallback::JME_IPM_RPCCallback( JME_IPM_RPCClientPtr client, JME_IPM_RPCCallback::RpcHandler cb, size_t t, JME_IPM_RPCCallback::RpcDeadHandler dcb, int methodId ):
			_cb(cb),
			_checkDead(true)
		{
			_dt = DeadTimePtr(new boost::asio::deadline_timer(JMECore.getLogicioService()));
			_dt->expires_from_now(boost::posix_time::seconds(t));
			_dt->async_wait(boost::bind(JME_IPM_RPCClient::RpcDeadCallback, client, boost::asio::placeholders::error, methodId, dcb));
		}


		JME_IPM_RPCCallback::JME_IPM_RPCCallbackPtr JME_IPM_RPCCallback::create( JME_IPM_RPCCallback::RpcHandler cb )
		{
			return JME_IPM_RPCCallbackPtr(new JME_IPM_RPCCallback(cb));
		}

		JME_IPM_RPCCallback::JME_IPM_RPCCallbackPtr JME_IPM_RPCCallback::create( JME_IPM_RPCClientPtr client, JME_IPM_RPCCallback::RpcHandler cb, size_t t, JME_IPM_RPCCallback::RpcDeadHandler dcb, int methodId )
		{
			return JME_IPM_RPCCallbackPtr(new JME_IPM_RPCCallback(client, cb, t, dcb, methodId));
		}

		JMEngine::rpc::JME_IPM_RPCClient::JME_IPM_RPCClientPtr JME_IPM_RPCClient::create(const char* name)
		{
			return JMEngine::rpc::JME_IPM_RPCClient::JME_IPM_RPCClientPtr(new JME_IPM_RPCClient(name));
		}

		bool JME_IPM_RPCClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_IPM_RPCCallback::RpcHandler cb )
		{
			try
			{
				boost::mutex::scoped_lock lock(_mutex);

				JME_IPM_RPC r(_sname, ++_mid, method, params);

				_cbs[_mid] = JME_IPM_RPCCallback::create(cb);
			}
			catch(const char* e)
			{
				LogE << "Call rpc function failed, error: " << e << LogEnd;
			}
			return false;
		}

		bool JME_IPM_RPCClient::callRpcMethod( const char* method, const google::protobuf::Message* params, JME_IPM_RPCCallback::RpcHandler cb, size_t dt, JME_IPM_RPCCallback::RpcDeadHandler dcb )
		{
			try
			{
				boost::mutex::scoped_lock lock(_mutex);

				JME_IPM_RPC r(_sname, ++_mid, method, params);

				_cbs[_mid] = JME_IPM_RPCCallback::create(shared_from_this(), cb, dt, dcb, _mid);
			}
			catch(const char* e)
			{
				LogE << "Call rpc function failed, error: " << e << LogEnd;
			}
			return false;
		}

		void JME_IPM_RPCClient::RpcDeadCallback( JME_IPM_RPCClientPtr client, const boost::system::error_code& err, int methodId, JME_IPM_RPCCallback::RpcDeadHandler dcb )
		{
			if (err)
				return;

			client->removeDeadRPC(methodId);
			dcb();
		}

		void JME_IPM_RPCClient::removeDeadRPC( int methodId )
		{
			boost::mutex::scoped_lock lock(_mutex);

			auto it = _cbs.find(methodId);
			if (it != _cbs.end())
			{
				_cbs.erase(it);
			}
		}

		JME_IPM_RPCClient::JME_IPM_RPCClient( const char* name ):
			_sname(name),
			_mid(0)
		{
		}
	}
}