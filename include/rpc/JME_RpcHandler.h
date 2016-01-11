#ifndef JME_RpcHandler_h__
#define JME_RpcHandler_h__

#include <string>
#include <map>

#include "boost/shared_ptr.hpp"
#include "boost/function.hpp"

#include "google/protobuf/message.h"

#include "log/JME_GLog.h"

using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class RpcHandlerInterface
		{
		public:
			typedef boost::shared_ptr<RpcHandlerInterface> RpcHandlerInterfacePtr;

			//rpc���������� ������� ����Message, rpc����ฺ���ͷ�Message
			typedef boost::function<google::protobuf::Message*(const string& params)> RpcHandler;
		public:
			static RpcHandlerInterface::RpcHandlerInterfacePtr create();
			static void regRpcHandler(const char* method, RpcHandler handler);
			static google::protobuf::Message* execRpcHandler(const string& method, const string& params);
		private:
			static map<string, RpcHandlerInterface::RpcHandler>& getRpcHandler();
		};
	}
}
#endif // JME_RpcHandler_h__
