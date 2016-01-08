#ifndef JME_Message_h__
#define JME_Message_h__

#include <memory>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "memory/nedmalloc.h"

using namespace std;

const static short RPCMessage = 1;	//rpc 消息
const static short HeartBeat = 2;	//心跳 消息
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
		struct Message : public MessageHeader
		{
			typedef	boost::shared_ptr<JMEngine::net::Message>	MessagePtr;
			const char*	_msgData;

			Message():
				MessageHeader()
			{
			}
			Message(short msgId):
				MessageHeader(msgId)
			{
			}
			Message(short msgId, const string& msgStr):
				MessageHeader(msgId)
			{
				_totalLen = msgStr.size() + MessageHeaderLength;
				_msgData = msgStr.data();
			}
			Message(short msgId, int netId, const string& msgStr):
				MessageHeader(msgId)
			{
				_netId = netId;
				_totalLen = msgStr.size() + MessageHeaderLength;
				_msgData = msgStr.data();
			}
			Message(const char* data_ptr)
			{
				memcpy(this, data_ptr, MessageHeaderLength);
				size_t msgSize = _totalLen - MessageHeaderLength;
				char* msgPtr = (char*)this + sizeof(Message);			
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
			static void	destory(Message* msg)
			{
				nedalloc::nedfree(msg);
			}
			static MessagePtr create(const char* dataPtr,size_t len)
			{
				void* m = nedalloc::nedmalloc(len + sizeof(long));
				return MessagePtr(new(m) Message(dataPtr), destory);
			}
		};
#pragma pack(pop)
	}
}
#endif // JME_Message_h__
