#ifndef JME_RpcHandler_h__
#define JME_RpcHandler_h__

#include <string>
#include <map>

#include "boost/shared_ptr.hpp"
#include "boost/function.hpp"

#include "google/protobuf/message.h"

#include "log/JME_GLog.h"
#include "net/JME_TcpSession.h"
using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class RpcServer;
		typedef boost::shared_ptr<RpcServer> RpcServerPtr;

		class RpcHandlerInterface
		{
		public:
			typedef boost::shared_ptr<RpcHandlerInterface> RpcHandlerInterfacePtr;

			//rpc处理函数函数 负责分配 返回Message, rpc服务侧负责释放Message
			typedef boost::function<google::protobuf::Message*(const string& params)> RpcHandler;
		public:
			template<class T>
			static void bindHandler();

			static RpcHandlerInterface::RpcHandlerInterfacePtr create();
			static void regRpcHandler(const char* method, RpcHandler handler);
			static google::protobuf::Message* execRpcHandler(const string& method, const string& params);

		private:
			static map<string, RpcHandler> _handlers;
		};

		template<class T>
		void JMEngine::rpc::RpcHandlerInterface::bindHandler()
		{
			auto handler = boost::shared_ptr<T>(new T);
			handler->init();
		}

	}
}
#endif // JME_RpcHandler_h__
