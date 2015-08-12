#include "JME_IPM_Rpc.h"
#include "boost/lexical_cast.hpp"
#include "JME_GLog.h"

#define SafeDelete(p) \
	delete[] p; \
	p = NULL;
namespace JMEngine
{
	namespace rpc
	{
		JME_IPM_RPC::JME_IPM_RPC( const string& name, size_t method_id, const char* method, const google::protobuf::Message* param )
		{
			_methodId = method_id;

			size_t mlen = sizeof(method);
			assert(mlen < MaxRPCMethodLen);

			memcpy(_method, method, mlen);

			_len = param->ByteSize();

			try
			{
				string mname = name + ".ipm_rpc_send." + boost::lexical_cast<string>(_methodId);

				boost::interprocess::message_queue mq(boost::interprocess::create_only, mname.c_str(), 1, _len);

				string pname = name + ".ipm_rpc_mq";

				boost::interprocess::message_queue pmq(boost::interprocess::open_only, pname.c_str());

				char* params = new char[_len];
				assert(param->SerializeToArray((void*)params, _len));

				mq.send((void*)params, mlen, 0);

				SafeDelete(params);

				pmq.send(this, sizeof(JME_IPM_RPC), 0);
			}
			catch(boost::interprocess::interprocess_exception& e)
			{
				LogE << e.what() << LogEnd;

				throw e.what();
			}
		}

	}
}