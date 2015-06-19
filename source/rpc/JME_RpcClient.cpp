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

		bool JME_RpcClient::callRpcMethod( const char* method, const google::protobuf::Message* params, RpcCBHandler cb )
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
				_cbs[_methodId] = cb;

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
				it->second(r);

				_cbs.erase(it);
			}
		}

		void JME_RpcClient::sessionReadError( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e )
		{
			session->resetReadBuffer();
			session->reconnect();
		}

	}
}