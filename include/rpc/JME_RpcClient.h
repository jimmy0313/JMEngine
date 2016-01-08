#ifndef JME_RpcClient_h__
#define JME_RpcClient_h__

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/recursive_mutex.hpp"

#include "google/protobuf/message.h"

#include "rpc/JME_RpcHandler.h"
#include "net/JME_TcpSession.h"
#include "net/JME_NetHandler.h"
#include "net/JME_Core.h"

using namespace std;
namespace Json
{
	class Value;
}

namespace JMEngine
{
	namespace rpc
	{
		class RpcClient;
		typedef boost::shared_ptr<RpcClient> RpcClientPtr;

		class RpcCallback
		{
		public:
			typedef boost::function<void(const string& response)> RpcHandler;
			typedef boost::function<void()> RpcDeadHandler;
			typedef boost::shared_ptr<boost::asio::deadline_timer> DeadTimePtr;
			typedef boost::shared_ptr<RpcCallback> RpcCallbackPtr;

		public:
			RpcCallback(RpcHandler cb);
			RpcCallback(RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

			static RpcCallback::RpcCallbackPtr create(RpcHandler cb);
			static RpcCallback::RpcCallbackPtr create(RpcClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

		public:
			RpcHandler _cb;	//回调函数
			DeadTimePtr _dt;	//超时时间
			bool _checkDead;
		};


		class RpcClient : 
			public JMEngine::net::NetHandler,
			public boost::enable_shared_from_this<RpcClient>
		{
		public:
			typedef boost::shared_ptr<RpcClient> RpcClientPtr;

			friend class RpcCallback;
		public:
			RpcClient(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			~RpcClient();

			static JMEngine::rpc::RpcClient::RpcClientPtr create(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			
			bool callRpcMethod(const char* method, const google::protobuf::Message* params);
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, RpcCallback::RpcHandler cb);	//返回值为真 表示参数
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, RpcCallback::RpcHandler cb, size_t dt, RpcCallback::RpcDeadHandler dcb);	//返回值为真 表示参数

			void sessionConnectSucceed(JMEngine::net::TcpSession::TcpSessionPtr session);
			void sessionConnectFailed(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);
			void sessionDisconnect(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);
			void sessionReceiveMessage(JMEngine::net::TcpSession::TcpSessionPtr session, JMEngine::net::MessagePtr msg);
			void sessionReadError(JMEngine::net::TcpSession::TcpSessionPtr session, boost::system::error_code e);

		public:
			void start();
			void stop();

		protected:
			void removeDeadRPC(int methodId);

			static void RpcDeadCallback(RpcClientPtr client, const boost::system::error_code& err, int methodId, RpcCallback::RpcDeadHandler dcb);

		private:
			JMEngine::net::TcpSession::TcpSessionWeakPtr _session;

			int _methodId;
			map<int, RpcCallback::RpcCallbackPtr> _cbs;

			boost::recursive_mutex _mutex;
		};

	}
}
#endif // JME_RpcClient_h__
