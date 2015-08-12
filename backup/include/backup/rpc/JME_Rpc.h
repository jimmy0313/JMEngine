#ifndef JME_Rpc_h__
#define JME_Rpc_h__

#include <string>
#include "boost/thread/mutex.hpp"
#include "google/protobuf/message.h"
#include "json/json.h"
#include "boost/interprocess/ipc/message_queue.hpp"

using namespace std;

const static int MaxRPCMethodLen = 30;

#define SafeDelete(p) \
	delete[] p; \
	p = NULL;
namespace JMEngine
{
	namespace rpc
	{
		class JME_Rpc
		{
		public:
			explicit JME_Rpc(const string& response);
			JME_Rpc(int id, const char* method, const google::protobuf::Message* para);
			JME_Rpc(int id, const char* method, const string& para);

			string serializeAsString() const;

			void parseFromString(const string& str);

		public:
			int	_methodId;
			string _method;
			string _params;
		};

		class JME_IPM_RPC_Helper : 
			public boost::noncopyable
		{
		public:
			friend class JME_IPM_Rpc;
		protected:
			JME_IPM_RPC_Helper(const char* mq);
		};
		class JME_IPM_Rpc
		{
		public:
			JME_IPM_Rpc(const char* method, const google::protobuf::Message* param);
		public:
			int _methodId;
			char _method[MaxRPCMethodLen];
			size_t _len;

		private:
			static boost::mutex _mutex;
			static size_t _mid;
			static JME_IPM_RPC_Helper _removeMQ;
			static boost::interprocess::message_queue _mq;
		};
	}
}
#endif // JME_Rpc_h__
