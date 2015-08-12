#include "JME_Rpc.h"
#include "JME_GLog.h"
#include "boost/lexical_cast.hpp"
namespace JMEngine
{
	namespace rpc
	{
		size_t JME_IPM_Rpc::_mid;
		boost::mutex JME_IPM_Rpc::_mutex;
		JME_IPM_RPC_Helper JME_IPM_Rpc::_removeMQ("ipm_rpc_mq");
		boost::interprocess::message_queue JME_IPM_Rpc::_mq(boost::interprocess::create_only, "ipm_rpc_mq", 65535, sizeof(JME_IPM_Rpc));
		
		JMEngine::rpc::JME_Rpc::JME_Rpc( const string& response )
		{
			parseFromString(response);
		}

		JME_Rpc::JME_Rpc( int id, const char* method, const google::protobuf::Message* para ) :
			_methodId(id),
			_method(method),
			_params(para->SerializeAsString())
		{

		}

		JME_Rpc::JME_Rpc( int id, const char* method, const string& para ) :
			_methodId(id),
			_method(method),
			_params(para)
		{

		}

		std::string JME_Rpc::serializeAsString() const
		{
			Json::Value val;
			val["method_id"] = _methodId;
			val["method"] = _method;
			val["params"] =_params;
			return val.toSimpleString();
		}

		void JME_Rpc::parseFromString( const string& str )
		{
			Json::Reader reader;
			Json::Value val;

			reader.parse(str, val);

			_methodId = val["method_id"].asInt();
			_method = val["method"].asString();
			_params = val["params"].asString();
		}


		JME_IPM_Rpc::JME_IPM_Rpc( const char* method, const google::protobuf::Message* param )
		{
			_mutex.lock();
			_methodId = ++_mid;
			_mutex.unlock();

			size_t mlen = sizeof(method);
			assert(mlen < MaxRPCMethodLen);

			memcpy(_method, method, mlen);

			_len = param->ByteSize();

			try
			{
				string mname = "ipm_rpc_param." + boost::lexical_cast<string>(_methodId);

				boost::interprocess::message_queue mq(boost::interprocess::create_only, mname.c_str(), 1, _len);


				char* params = new char[_len];
				assert(param->SerializeToArray((void*)params, _len));

				mq.send((void*)params, mlen, 0);

				SafeDelete(params);

				_mq.send(this, sizeof(JME_IPM_Rpc), 0);
			}
			catch(boost::interprocess::interprocess_exception& e)
			{
				LogE << e.what() << LogEnd;

				throw e.what();
			}
		}


		JME_IPM_RPC_Helper::JME_IPM_RPC_Helper( const char* mq )
		{
			boost::interprocess::message_queue::remove(mq);
		}

	}
}
