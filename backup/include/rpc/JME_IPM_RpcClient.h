#ifndef JME_IPM_RpcClient_h__
#define JME_IPM_RpcClient_h__

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/interprocess/ipc/message_queue.hpp"

#include "google/protobuf/message.h"

#include "JME_RpcHandler.h"
#include "JME_TcpSession.h"
#include "JME_NetHandler.h"
#include "JME_IPM_Rpc.h"
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
		class JME_IPM_RPCClient;
		typedef boost::shared_ptr<JME_IPM_RPCClient> JME_IPM_RPCClientPtr;

		class JME_IPM_RPCCallback
		{
		public:
			typedef boost::function<void(const char* param, size_t len)> RpcHandler;
			typedef boost::function<void()> RpcDeadHandler;
			typedef boost::shared_ptr<boost::asio::deadline_timer> DeadTimePtr;
			typedef boost::shared_ptr<JME_IPM_RPCCallback> JME_IPM_RPCCallbackPtr;

		public:
			JME_IPM_RPCCallback(RpcHandler cb);
			JME_IPM_RPCCallback(JME_IPM_RPCClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

			static JME_IPM_RPCCallback::JME_IPM_RPCCallbackPtr create(RpcHandler cb);
			static JME_IPM_RPCCallback::JME_IPM_RPCCallbackPtr create(JME_IPM_RPCClientPtr client, RpcHandler cb, size_t t, RpcDeadHandler dcb, int methodId);

		public:
			RpcHandler _cb;	//回调函数
			DeadTimePtr _dt;	//超时时间
			bool _checkDead;
		};

		class JME_IPM_RPCClient : 
			public boost::enable_shared_from_this<JME_IPM_RPCClient>
		{
		public:
			typedef boost::shared_ptr<JME_IPM_RPCClient> JME_IPM_RPCClientPtr;

		public:
			JME_IPM_RPCClient(const char* name);
			~JME_IPM_RPCClient();

			static JMEngine::rpc::JME_IPM_RPCClient::JME_IPM_RPCClientPtr create(const char* name);

			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_IPM_RPCCallback::RpcHandler cb);	//返回值为真 表示参数
			bool callRpcMethod(const char* method, const google::protobuf::Message* params, JME_IPM_RPCCallback::RpcHandler cb, size_t dt, JME_IPM_RPCCallback::RpcDeadHandler dcb);	//返回值为真 表示参数

			static void RpcDeadCallback(JME_IPM_RPCClientPtr client, const boost::system::error_code& err, int methodId, JME_IPM_RPCCallback::RpcDeadHandler dcb);

		protected:
			void removeDeadRPC(int methodId);

		private:
			string _sname;
			size_t _mid;
			map<int, JME_IPM_RPCCallback::JME_IPM_RPCCallbackPtr> _cbs;

			boost::mutex _mutex;
		};
	}
}
#endif // JME_IPM_RpcClient_h__
