#ifndef JME_NetHandler_h__
#define JME_NetHandler_h__

#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace JMEngine
{
	namespace net
	{
		class TcpSession;
		typedef boost::shared_ptr<TcpSession> TcpSessionPtr;

		class Message;
		typedef boost::shared_ptr<Message> MessagePtr;
		class NetHandler
		{		
		public:
			typedef boost::shared_ptr<NetHandler> NetHandlerPtr;
		public:
			virtual void sessionConnectSucceed(TcpSessionPtr session) = 0;	//该函数将由逻辑线程调用
			virtual void sessionConnectFailed(TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
			virtual void sessionDisconnect(TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
			virtual void sessionReceiveMessage(TcpSessionPtr session, MessagePtr msg) = 0;	//该函数将由逻辑线程调用
			virtual void sessionReadError(TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
		};
	
		class UdpSession;
		typedef boost::shared_ptr<UdpSession> UdpSessionPtr;
	
		class UdpNetHandler
		{
		public:
			typedef boost::shared_ptr<UdpNetHandler> UdpNetHandlerPtr;
		
		public:
			virtual void onReceive(UdpSessionPtr session, MessagePtr msg) = 0;
			virtual void onWrite(UdpSessionPtr session) = 0;
		};
	}
}
#endif // JME_NetHandler_h__
