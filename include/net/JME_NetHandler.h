#ifndef JME_NetHandler_h__
#define JME_NetHandler_h__

#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>

namespace JMEngine
{
	namespace net
	{
		class JME_TcpSession;
		typedef boost::shared_ptr<JME_TcpSession> JME_TcpSessionPtr;

		class JME_Message;
		typedef boost::shared_ptr<JME_Message> JME_MessagePtr;
		class JME_NetHandler
		{		
		public:
			typedef boost::shared_ptr<JME_NetHandler> JME_NetHandlerPtr;
		public:
			virtual void sessionConnectSucceed(JME_TcpSessionPtr session) = 0;	//该函数将由逻辑线程调用
			virtual void sessionConnectFailed(JME_TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
			virtual void sessionDisconnect(JME_TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
			virtual void sessionReceiveMessage(JME_TcpSessionPtr session, JME_MessagePtr msg) = 0;	//该函数将由逻辑线程调用
			virtual void sessionReadError(JME_TcpSessionPtr session, boost::system::error_code e) = 0; //该函数将由逻辑线程调用
		};
	}
}
#endif // JME_NetHandler_h__
