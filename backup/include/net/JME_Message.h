#ifndef JME_Message_h__
#define JME_Message_h__

#include <memory>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <nedmalloc.h>
#include <string>

using namespace std;
namespace JMEngine
{
	namespace net
 	{
		static size_t MaxMsgLength = 819200;
		enum
		{ 
			MessageHeaderLength = 14
		};
#pragma pack(push,1)
		struct MessageHeader
		{
		public:
			MessageHeader(int msgId = 0):
				_totalLen(MessageHeaderLength),
				_msgId(msgId),
				_netId(0),
				_bindId(0)
			{
			}
		public:
			int _totalLen;
			short _msgId;
			int _netId;
			int _bindId;
		};
		struct JME_Message : public MessageHeader
		{
			typedef	boost::shared_ptr<JMEngine::net::JME_Message>	JME_MessagePtr;
			const char*	_msgData;

			JME_Message():
				MessageHeader()
			{
			}
			JME_Message(short msgId):
				MessageHeader(msgId)
			{
			}
			JME_Message(short msgId, const string& msgStr):
				MessageHeader(msgId)
			{
				_totalLen = msgStr.size() + MessageHeaderLength;
				_msgData = msgStr.data();
			}
			JME_Message(const char* data_ptr)
			{
				memcpy(this, data_ptr, MessageHeaderLength);
				size_t msgSize = _totalLen - MessageHeaderLength;
				char* msgPtr = (char*)this + sizeof(JME_Message);			
				memcpy(msgPtr, data_ptr + MessageHeaderLength, msgSize);
				_msgData = msgPtr;
			}
			size_t messageDataLen() const
			{
				return _totalLen - MessageHeaderLength;
			}

			string getMessageStr()
			{
				return string(_msgData, messageDataLen());
			}
			static void	destory(JME_Message* msg)
			{
				nedalloc::nedfree(msg);
			}
			static JME_MessagePtr create(const char* dataPtr,size_t len)
			{
				void* m = nedalloc::nedmalloc(len + sizeof(long));
				return JME_MessagePtr(new(m) JME_Message(dataPtr), destory);
			}
		};
#pragma pack(pop)
	}
}
#endif // JME_Message_h__
