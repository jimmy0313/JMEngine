#ifndef JME_RpcHandler_h__
#define JME_RpcHandler_h__

#include <string>
#include <map>

#include "boost/shared_ptr.hpp"
#include "boost/function.hpp"

#include "JME_GLog.h"
#include "JME_Rpc.h"
#include "JME_TcpSession.h"
using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class JME_RpcServer;
		typedef boost::shared_ptr<JME_RpcServer> JME_RpcServerPtr;

		class JME_RpcHandler
		{
		public:
			typedef boost::shared_ptr<JME_RpcHandler> JME_RpcHandlerPtr;
			typedef boost::function<string(const string& params)> RpcHandler;
		public:
			template<class T>
			static void bindHandler();

			static JME_RpcHandler::JME_RpcHandlerPtr create();
			static void regRpcHandler(const char* method, RpcHandler handler);
			static string execRpcHandler(const string& method, const string& params);

		private:
			static map<string, RpcHandler> _handlers;
		};

		template<class T>
		void JMEngine::rpc::JME_RpcHandler::bindHandler()
		{
			auto handler = boost::shared_ptr<T>(new T);
			handler->init();
		}

	}
}
#endif // JME_RpcHandler_h__
