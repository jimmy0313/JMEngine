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
namespace JMEngine
{
	namespace rpc
	{
		class JME_RpcClient final : 
			public JMEngine::net::JME_NetHandler,
			public boost::enable_shared_from_this<JME_RpcClient>
		{
		public:
			typedef boost::shared_ptr<JME_RpcClient> JME_RpcClientPtr;
 			typedef boost::function<void(const JME_Rpc& response)> RpcCBHandler;
		public:
			JME_RpcClient(const string& ip, const string& port, size_t buffSize, size_t reconnect);
			~JME_RpcClient();

			static JMEngine::rpc::JME_RpcClient::JME_RpcClientPtr create(const string& ip, const string& port, size_t buffSize, size_t reconnect);

			bool callRpcMethod(const char* method, const google::protobuf::Message* params, RpcCBHandler cb);	//返回值为真 表示参数

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
			map<int, RpcCBHandler> _cbs;

			boost::mutex _mutex;
		};
	}
}
#endif // JME_RpcClient_h__
