#ifndef JME_DBPool_h__
#define JME_DBPool_h__

/********************************************************************
	created:	2014/06/20
	created:	20:6:2014   11:02
	author:		huangzhi
	
	purpose:	一个通用的数据库连接池模板，通过不同数据库驱动对象，实例化模板
				DBConnDelegator一个桥接接口，DBHelper通过此接口对象访问连接池私有函数
				DBHelper一个连接类，其中包含一个驱动对象和一个指向连接池的对象
				DBPool连接池，驱动对象的创建，还有两个私有方法，获取与释放驱动对象
				
				外部使用只需通过DBHelper获取数据库驱动对象，无需考虑驱动对象释放问题
*********************************************************************/
#include "boost/thread/mutex.hpp"
#include "boost/function.hpp"
#include <deque>
#include <string>
#include "JME_GLog.h"

using namespace std;
namespace JMEngine
{
	namespace db
	{
		class JME_DBException
		{
		public:
			explicit JME_DBException(const string& err):
				_err(err)
			{
			}
			explicit JME_DBException(const char* err):
				_err(err)
			{
			}

			const string& what() const { return _err; }
		public:
			string _err;
		};
		template<class T> class DBPool;

		// 代理类，指向一个个连接池，通过此类获取单个连接
		template<class T>
		class DBConnDelegator
		{
		public:
			virtual T* getConn() = 0;	// 获取单个连接
			virtual void releaseConn(T*) = 0;	// 释放连接
		};

		// 连接类， 包含一个conn对象，一个代理对象
		template<class T>
		class DBHelper
		{
		public:
			typedef boost::shared_ptr<DBConnDelegator<T> > DelegtorPtr;
		public:
			DBHelper(DBConnDelegator<T>* delegtor)
			{
				_delegator = (DBPool<T>*)delegtor;
				_conn = _delegator->getConn();	// 获取连接
				if (nullptr == _conn)
					throw JME_DBException("Try to get db connector failed !!!");
			}

			~DBHelper()
			{
				if(nullptr != _conn)	// 释放连接
					_delegator->releaseConn(_conn);
				_delegator = nullptr;
			}

			T* operator ->() { return _conn; }

		private:
			T* _conn;
			DBConnDelegator<T>*	_delegator;
		};	

		template<class T>
		class DBPool : public DBConnDelegator<T>
		{
		public:
			typedef deque<T*> ConnDeque;
			typedef boost::function<bool(T*)>	ConnectHandler;
			typedef boost::shared_ptr<DBPool<T> > DBPoolPtr;

			template<class T1> 
			friend class DBHelper;	//友元，允许访问私有方法
		public:
			DBPool(size_t n, ConnectHandler handler):
				_connectHandler(handler)
			{
				init(n, handler);
			}
			~DBPool()
			{
				for (size_t i = 0; i < _connPool.size(); i++)
				{
					delete _connPool[i];
				}
			}

			static typename DBPool<T>::DBPoolPtr create(size_t n, ConnectHandler handler)
			{
				return typename DBPool<T>::DBPoolPtr(new DBPool<T>(n, handler));
			}
			
		private:
			void init(size_t n, ConnectHandler handler)
			{
				if (!n)
				{
					LOGE("DB pool num must > 0");
					abort();
				}
				//初始化连接池
				for (size_t i = 0; i < n; i++)
				{
					T* conn = new T;
					if (handler(conn))
					{
						_connPool.push_back(conn);
					}
				}
				LOGI("Init %d db connector complete", _connPool.size());
			}
		private:
			// 获取真实连接对象，仅允许连接类访问
			T* getConn()
			{
 				T* conn = nullptr;

				_mutex.lock();
				if(!_connPool.empty())
				{
					conn = *_connPool.begin();
					_connPool.pop_front();
					_mutex.unlock();
				}
				else
				{
					_mutex.unlock();

					conn = new T;
					if( !_connectHandler(conn) )
					{
						delete conn;
						conn = nullptr;
					}
				}
				return conn;
			}

			void releaseConn(T* conn)
			{
				_mutex.lock();
				_connPool.push_back(conn);
				_mutex.unlock();
			}
		private:
			ConnDeque _connPool;
			ConnectHandler _connectHandler;
			boost::mutex _mutex;
		}; 

	}
}
#endif // JME_DBPool_h__