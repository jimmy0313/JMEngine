#ifndef JME_RpcClient_h__
#define JME_RpcClient_h__

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

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

#define RPCDeadCB(rpc, error, methodId) \
	if(error) \
		return; \
	rpc->removeDeadRPC(methodId);

namespace JMEngine
{
	namespace rpc
	{

		class JME_RpcCallback
		{
		public:
			typedef boost::function<void(const JME_Rpc& response)> RpcHandler;
			typedef boost::function<void(const boost::system::error_code&, int)> RpcDeadHandler;
			typedef boost::shared_ptr<boost::asio::deadline_timer> DeadTimePtr;
			typedef boost::shared_ptr<JME_RpcCallback> JME_RpcCallbackPtr;

		public:
			JME_RpcCallback(RpcHandler cb);
			JME_RpcCallback(RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

			static JME_RpcCallback::JME_RpcCallbackPtr create(RpcHandler cb);
			static JME_RpcCallback::JME_RpcCallbackPtr create(RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

		public:
			RpcHandler _cb;	//回调函数
			RpcDeadHandler _dcb;	//超时回调函数
			DeadTimePtr _dt;	//超时时间
			bool _checkDead;
			int _methodId;	//rpc调用id， 用于超时时，从map里面移除rpc对象
		};


		class JME_RpcClient : 
			public JMEngine::net::JME_NetHandler,
			public boost::enable_shared_from_this<JME_RpcClient>
		{
		public:
			typedef boost::shared_ptr<JME_RpcClient> JME_RpcClientPtr;
		public:
			JME_RpcClient(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			~JME_RpcClient();

			static JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr create(const string& ip, const string& port, size_t buffSize, size_t reconnect);

			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb);	//返回值为真 表示参数
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_RpcCallback::RpcHandler cb, size_t dt, JME_RpcCallback::RpcDeadHandler dcb);	//返回值为真 表示参数

			void removeDeadRPC(int methodId);

			void sessionConnectSucceed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session);
			void sessionConnectFailed(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionDisconnect(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);
			void sessionReceiveMessage(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, JMEngine::net::JME_MessagePtr msg);
			void sessionReadError(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e);

		public:
			void start();
			void stop();

		private:
			JMEngine::net::JME_TcpSession::JME_TcpSessionPtr _session;

			int _methodId;
			map<int, JME_RpcCallback::JME_RpcCallbackPtr> _cbs;

			boost::mutex _mutex;
		};

	}
}
#endif // JME_RpcClient_h__
