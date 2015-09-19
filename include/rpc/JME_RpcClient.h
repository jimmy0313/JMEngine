#ifndef JME_RpcClient_h__
#define JME_RpcClient_h__

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/recursive_mutex.hpp"

#include "google/protobuf/message.h"

#include "JME_RpcHandler.h"
#include "JME_TcpSession.h"
#include "JME_NetHandler.h"
#include "JME_Core.h"

using namespace std;
namespace Json
{
	class Value;
}

namespace JMEngine
{
	namespace rpc
	{
		class JME_RpcClient;
		typedef boost::shared_ptr<JME_RpcClient> JME_RpcClientPtr;

		class JME_RpcCallback
		{
		public:
			typedef boost::function<void(const string& response)> RpcHandler;
			typedef boost::function<void()> RpcDeadHandler;
			typedef boost::shared_ptr<boost::asio::deadline_timer> DeadTimePtr;
			typedef boost::shared_ptr<JME_RpcCallback> JME_RpcCallbackPtr;

		public:
			JME_RpcCallback(RpcHandler cb);
			JME_RpcCallback(JME_RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

			static JME_RpcCallback::JME_RpcCallbackPtr create(RpcHandler cb);
			static JME_RpcCallback::JME_RpcCallbackPtr create(JME_RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

		public:
			RpcHandler _cb;	//回调函数
			DeadTimePtr _dt;	//超时时间
			bool _checkDead;
		};


		class JME_RpcClient : 
			public JMEngine::net::JME_NetHandler,
			public boost::enable_shared_from_this<JME_RpcClient>
		{
		public:
			typedef boost::shared_ptr<JME_RpcClient> JME_RpcClientPtr;

			friend class JME_RpcCallback;
		public:
			JME_RpcClient(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			~JME_RpcClient();

			static JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr create(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			
			bool callRpcMethod(const char* method, const google::protobuf::Message* params);
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb);	//返回值为真 表示参数
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb, size_t dt, JME_RpcCallback::RpcDeadHandler dcb);	//返回值为真 表示参数

			void sessionConnectSucceed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session);
			void sessionConnectFailed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionDisconnect(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionReceiveMessage(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JMEngine::net::JME_MessagePtr msg);
			void sessionReadError(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);

		public:
			void start();
			void stop();

		protected:
			void removeDeadRPC(int methodId);

			static void RpcDeadCallback(JME_RpcClientPtr client, const boost::system::error_code& err, int methodId, JME_RpcCallback::RpcDeadHandler dcb);

		private:
			JMEngine::net::JME_TcpSession::JME_TcpSessionWeakPtr _session;

			int _methodId;
			map<int, JME_RpcCallback::JME_RpcCallbackPtr> _cbs;

			boost::recursive_mutex _mutex;
		};

	}
}
#endif // JME_RpcClient_h__
