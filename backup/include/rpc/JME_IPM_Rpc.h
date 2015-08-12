#ifndef JME_IPM_Rpc_h__
#define JME_IPM_Rpc_h__

#include <string>

#include "google/protobuf/message.h"
#include "boost/interprocess/ipc/message_queue.hpp"

using namespace std;

const static int MaxRPCMethodLen = 30;

namespace JMEngine
{
	namespace rpc
	{
		class JME_IPM_RPC
		{
		public:
			JME_IPM_RPC(const string& name, size_t method_id, const char* method, const google::protobuf::Message* param);
		public:
			int _methodId;	//
			char _method[MaxRPCMethodLen];
			size_t _len;	//参数大小
		};
	}
}
#endif // JME_IPM_Rpc_h__
