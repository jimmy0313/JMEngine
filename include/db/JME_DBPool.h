#ifndef JME_DBPool_h__
#define JME_DBPool_h__

/********************************************************************
	created:	2014/06/20
	created:	20:6:2014   11:02
	author:		huangzhi
	
	purpose:	һ��ͨ�õ����ݿ����ӳ�ģ�壬ͨ����ͬ���ݿ���������ʵ����ģ��
				DBConnDelegatorһ���Žӽӿڣ�DBHelperͨ���˽ӿڶ���������ӳ�˽�к���
				DBHelperһ�������࣬���а���һ�����������һ��ָ�����ӳصĶ���
				DBPool���ӳأ���������Ĵ�������������˽�з�������ȡ���ͷ���������
				
				�ⲿʹ��ֻ��ͨ��DBHelper��ȡ���ݿ������������迼�����������ͷ�����
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

		// �����ָ࣬��һ�������ӳأ�ͨ�������ȡ��������
		template<class T>
		class DBConnDelegator
		{
		public:
			virtual T* getConn() = 0;	// ��ȡ��������
			virtual void releaseConn(T*) = 0;	// �ͷ�����
		};

		// �����࣬ ����һ��conn����һ���������
		template<class T>
		class DBHelper
		{
		public:
			typedef boost::shared_ptr<DBConnDelegator<T> > DelegtorPtr;
		public:
			DBHelper(DBConnDelegator<T>* delegtor)
			{
				_delegator = (DBPool<T>*)delegtor;
				_conn = _delegator->getConn();	// ��ȡ����
				if (nullptr == _conn)
					throw JME_DBException("Try to get db connector failed !!!");
			}

			~DBHelper()
			{
				if(nullptr != _conn)	// �ͷ�����
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
			friend class DBHelper;	//��Ԫ���������˽�з���
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
				//��ʼ�����ӳ�
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
			// ��ȡ��ʵ���Ӷ��󣬽��������������
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