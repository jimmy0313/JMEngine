#ifndef JME_Rpc_h__
#define JME_Rpc_h__

#include <string>

#include "google/protobuf/message.h"
#include "json/json.h"

using namespace std;
namespace JMEngine
{
	namespace rpc
	{
		class JME_Rpc
		{
		public:
			explicit JME_Rpc(const string& response)
			{
				parseFromString(response);
			}
			JME_Rpc(int id, const char* method, const google::protobuf::Message* para):
				_rpcId(id),
				_method(method),
				_params(para->SerializeAsString())
			{
			}
			JME_Rpc(int id, const char* method, const string& para):
				_rpcId(id),
				_method(method),
				_params(para)
			{
			}

			string serializeAsString() const
			{
				Json::Value val;
				val["rpc_id"] = _rpcId;
				val["method"] = _method;
				val["params"] =_params;
				return val.toSimpleString();
			}

			void parseFromString(const string& str)
			{
				Json::Reader reader;
				Json::Value val;
				
				reader.parse(str, val);

				_rpcId = val["rpc_id"].asInt();
				_method = val["method"].asString();
				_params = val["params"].asString();
			}

		public:
			int	_rpcId;
			string _method;
			string _params;
		};
	}
}
#endif // JME_Rpc_h__
