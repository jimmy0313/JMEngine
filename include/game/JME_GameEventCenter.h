#ifndef JME_GameEventCenter_h__
#define JME_GameEventCenter_h__


/********************************************************************
	created:	2014/06/20
	author:		huangzhi
	
	purpose:	一个通用的游戏事件管理类
	warning:	有一个比较难缠的问题--事件重复注册，只能由使用者去避免
*********************************************************************/

#include "boost/signals2.hpp"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include <string>
#include "JME_Singleton.h"

using namespace std;

namespace JMEngine
{
	namespace game
	{
		template<class T>
		class EventObserver
		{
		public:
			typedef void (*EventHandler)(const T&);
			typedef boost::signals2::signal<void(const T&)>	Signal;
			typedef boost::function<void(const T&)> EventHandlerFunction;
			typedef boost::signals2::connection	EventConnector;
			typedef boost::shared_ptr<EventConnector> EventConnectorPtr;
			typedef boost::function<void(const T&)>	RemoteHandler;
		public:
			EventConnector connect(int slot, EventHandlerFunction handler)
			{
				return _signal.connect(slot,handler);
			}
			void disconnect(EventHandler handler)
			{
				_signal.disconnect(handler);
			}
			void callback(const T& evt)
			{
				_signal(evt);
			}
			bool empty()
			{
				return _signal.empty();
			}
		private:
			Signal _signal;	
		};

		template<class T>
		class GameEventCenter
		{
		public:
			typedef EventObserver<T> Observer;
			typedef map<T,Observer*> EventMap;
			enum EventSlot{ DEFAULT_SLOT = 50 };	//事件执行优先级,值越小,优先级越高

		public:
			//************************************
			// Method:    regEventHandler
			// FullName:  JMEngine::game::GameEventCenter<T>::regEventHandler
			// Access:    public 
			// Returns:   void
			// Qualifier: 监听事件，回调函数为使用boost::bind包装过的函数对象, 同时获取一个连接对象， 通过该对象， 可以移除事件监听
			// Parameter: const T & evt
			// Parameter: typename Observer::EventHandlerFunction handler
			// Parameter: typename Observer::EventConnectorPtr conn
			//************************************
			void regEventHandler(const T& evt, typename Observer::EventHandlerFunction handler, typename Observer::EventConnectorPtr conn)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					*conn = it->second->connect(DEFAULT_SLOT,handler);
				}
				else
				{
					Observer* obs = new Observer;
					*conn = obs->connect(DEFAULT_SLOT,handler);
					_events.insert(make_pair(evt,obs));
				}
			}
			//************************************
			// Method:    regEventHandler
			// FullName:  JMEngine::game::GameEventCenter<T>::regEventHandler
			// Access:    public 
			// Returns:   void
			// Qualifier: 根据指定优先级(slot) 监听事件
			// Parameter: int slot
			// Parameter: const T & evt
			// Parameter: typename Observer::EventHandlerFunction handler
			// Parameter: typename Observer::EventConnectorPtr conn
			//************************************
			void regEventHandler(int slot, const T& evt, typename Observer::EventHandlerFunction handler, typename Observer::EventConnectorPtr conn)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					*conn = it->second->connect(slot,handler);
				}
				else
				{
					Observer* obs = new Observer;
					*conn = obs->connect(slot,handler);
					_events.insert(make_pair(evt,obs));
				}
			}
			//************************************
			// Method:    regEventHandler
			// FullName:  JMEngine::game::GameEventCenter<T>::regEventHandler
			// Access:    public 
			// Returns:   void
			// Qualifier: 监听事件， 回调函数为c风格函数指针
			// Parameter: const T & evt
			// Parameter: typename Observer::EventHandler handler
			//************************************
			void regEventHandler(const T& evt, typename Observer::EventHandler handler)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					it->second->connect(DEFAULT_SLOT,handler);
				}
				else
				{
					Observer* obs = new Observer;
					obs->connect(DEFAULT_SLOT,handler);
					_events.insert(make_pair(evt,obs));
				}
			}
			//************************************
			// Method:    regEventHandler
			// FullName:  JMEngine::game::GameEventCenter<T>::regEventHandler
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: int slot
			// Parameter: const T & evt
			// Parameter: typename Observer::EventHandler handler
			//************************************
			void regEventHandler(int slot, const T& evt, typename Observer::EventHandler handler)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					it->second->connect(slot,handler);
				}
				else
				{
					Observer* obs = new Observer;
					obs->connect(slot,handler);
					_events.insert(make_pair(evt,obs));
				}
			}

			//remove event's handler
			//************************************
			// Method:    removeEventHandler
			// FullName:  JMEngine::game::GameEventCenter<T>::removeEventHandler
			// Access:    public 
			// Returns:   void
			// Qualifier: 移除对指定事件的监听， 不可用于回调函数为boost::bind函数对象的场景(该场景需要使用连接对象)
			// Parameter: const T & evt
			// Parameter: typename Observer::EventHandler handler
			//************************************
			void removeEventHandler(const T& evt, typename Observer::EventHandler handler)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					Observer* obs = it->second;
					obs->disconnect(handler);
				}
			}
			//exec event handler
			//exec native event handler
			//************************************
			// Method:    eventCallback
			// FullName:  JMEngine::game::GameEventCenter<T>::eventCallback
			// Access:    public 
			// Returns:   void
			// Qualifier: 仅执行本地事件回调
			// Parameter: const T & evt
			//************************************
			void eventCallback(const T& evt)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					Observer* obs = it->second;
					obs->callback(evt);
					if ( obs->empty() )
					{
						delete obs;
						_events.erase(it);
					}
				}
			}

			//************************************
			// Method:    remoteEventCallback
			// FullName:  JMEngine::game::GameEventCenter<T>::remoteEventCallback
			// Access:    public 
			// Returns:   void
			// Qualifier: 仅抛出远程事件, 无本地事件回调
			// Parameter: const T & evt
			// Parameter: typename Observer::RemoteHandler handler
			//************************************
			void remoteEventCallback(const T& evt, , typename Observer::RemoteHandler handler)
			{
				handler(evt);
			}

			//exec remote event handler ext
			//************************************
			// Method:    eventCallback
			// FullName:  JMEngine::game::GameEventCenter<T>::eventCallback
			// Access:    public 
			// Returns:   void
			// Qualifier: 执行本地事件的同时抛出远程事件
			// Parameter: const T & evt
			// Parameter: typename Observer::RemoteHandler handler
			//************************************
			void eventCallback(const T& evt, typename Observer::RemoteHandler handler)
			{
				auto it = _events.find(evt);
				if ( it != _events.end() )
				{
					Observer* obs = it->second;
					obs->callback(evt);
					if ( obs->empty() )
					{
						delete obs;
						_events.erase(it);
					}
				}
				handler(evt);
			}

		private:
			EventMap _events;
			boost::recursive_mutex	_eventMutex;
		};
	}
}

#endif // JME_GameEventCenter_h__
