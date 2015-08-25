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
		template<class T1, class T2>
		class JME_Dispatcher
		{
		public:
			typedef boost::shared_ptr<JME_Dispatcher> JME_DispatcherPtr;
			typedef boost::function<void(const T1 client/*指定客户端的代表，可以是一个session， 也可以是一个id，或者一个字符串*/, const T2 params)> MessageHandler;
			typedef map<int, MessageHandler> MessageHandlerMap;
		public:

			//************************************
			// Method:    regMessageHandler
			// FullName:  JMEngine::game::JME_Dispatcher<T>::regMessageHandler
			// Access:    public static 
			// Returns:   void
			// Qualifier: reg handler for msgId
			// Parameter: int msgId
			// Parameter: MessageHandler handler
			//************************************
			static void regMessageHandler(int msgId, MessageHandler handler);

			//************************************
			// Method:    regMessageHandler
			// FullName:  JMEngine::game::JME_Dispatcher<T>::regMessageHandler
			// Access:    public static 
			// Returns:   void
			// Qualifier: reg handler from beginMsg to endMsg
			// Parameter: int beginMsg
			// Parameter: int endMsg
			// Parameter: MessageHandler handler
			//************************************

			static void regMessageHandler(int beginMsg, int endMsg, MessageHandler handler);
			//************************************
			// Method:    execMessageHandler
			// FullName:  JMEngine::game::JME_Dispatcher<T>::execMessageHandler
			// Access:    public static 
			// Returns:   void
			// Qualifier: call msg handler for msgId
			// Parameter: int msgId
			// Parameter: JMEngine::net::JME_TcpSessionPtr session
			// Parameter: const T & params
			//************************************
			static void execMessageHandler(int msgId, const T1 client, const T2 params);

		private:
			static MessageHandlerMap _handleMap;
		};

		template<class T1, class T2>
		void JMEngine::game::JME_Dispatcher<T1, T2>::regMessageHandler( int beginMsg, int endMsg, MessageHandler handler )
		{
			for (int i = beginMsg; i <= endMsg; i++)
			{
				regMessageHandler(i, handler);
			}
		}

		template<class T1, class T2>
		void JMEngine::game::JME_Dispatcher<T1, T2>::regMessageHandler( int msgId, MessageHandler handler )
		{
			auto res = _handleMap.insert(make_pair(msgId,handler)); 
			if(!res.second)
			{
				LogE << "Message {" << msgId << "} had been registered" << LogEnd;
				abort();
			}
		}

		template<class T1, class T2>
		void JMEngine::game::JME_Dispatcher<T1, T2>::execMessageHandler( int msgId, const T1 client, const T2 params )
		{
			auto it = _handleMap.find(msgId);
			if(it != _handleMap.end())
			{
				it->second(client, params);
			}
			else
			{
				LogW << "Can't find message handler for msg {" << msgId << "}" << LogEnd;
			}
		}

		template<class T1, class T2>
		typename JME_Dispatcher<T1, T2>::MessageHandlerMap JME_Dispatcher<T1, T2>::_handleMap;
	}
}
#endif // JME_Dispatcher_h__
