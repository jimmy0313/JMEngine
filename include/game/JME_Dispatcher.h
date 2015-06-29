#ifndef JME_Dispatcher_h__
#define JME_Dispatcher_h__

/********************************************************************
	created:	2015/06/19
	author:		huangzhi
	
	purpose:	消息分发类， 游戏逻辑 继承至该类， 初始化注册消息回调函数， 收到消息后自动回调
	warning:	
*********************************************************************/


#include "boost/shared_ptr.hpp"
#include <boost/thread/detail/singleton.hpp>

#include "JME_TcpSession.h"
namespace JMEngine
{
	namespace game
	{
		template<class T>
		class JME_Dispatcher
		{
		public:
			typedef boost::shared_ptr<JME_Dispatcher> JME_DispatcherPtr;
			typedef boost::function<void(JMEngine::net::JME_TcpSessionPtr session, const T& params)> MessageHandler;
			typedef map<int, MessageHandler> MessageHandlerMap;
		public:

			static void regMessageHandler(int msgId, MessageHandler handler);
			static void execMessageHandler(int msgId, JMEngine::net::JME_TcpSessionPtr session, const T& params);

		private:
			static MessageHandlerMap _handleMap;
		};

		template<class T>
		void JMEngine::game::JME_Dispatcher<T>::regMessageHandler( int msgId, MessageHandler handler )
		{
			auto res = _handleMap.insert(make_pair(msgId,handler)); 
			if(!res.second)
			{
				LogE << "Message {" << msgId << "} had been registered" << LogEnd;
				abort();
			}
		}

		template<class T>
		void JMEngine::game::JME_Dispatcher<T>::execMessageHandler( int msgId, JMEngine::net::JME_TcpSessionPtr session, const T& params )
		{
			auto it = _handleMap.find(msgId);
			if(it != _handleMap.end())
			{
				it->second(session, params);
			}
			else
			{
				LogW << "Can't find message handler for msg {" << msgId << "}" << LogEnd;
			}
		}

		template<class T>
		typename JME_Dispatcher<T>::MessageHandlerMap JME_Dispatcher<T>::_handleMap;
	}
}
#endif // JME_Dispatcher_h__
