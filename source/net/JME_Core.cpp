#include "net/JME_Core.h"
#include "log/JME_GLog.h"


namespace JMEngine
{
	namespace net
	{
		IoServiceCore::IoServiceCore( void )
		{
			_netThread = Thread::create();

			_logicThread = Thread::create();
		}

		IoServiceCore::~IoServiceCore( void )
		{
			stop();
		}

		boost::asio::io_service& IoServiceCore::getNetIoService()
		{
			return *_netThread->ioService();
		}

		boost::asio::io_service& IoServiceCore::getLogicioService()
		{
			return *_logicThread->ioService();
		}

		void IoServiceCore::stop()
		{
			_netThread->stop();
			_logicThread->stop();

			for (auto& it : _namedThread)
			{
				it.second->stop();
			}
		}

		IoServiceCore* IoServiceCore::getInstance()
		{
			static IoServiceCore* _pInstance = new IoServiceCore;

			return _pInstance;
		}

		void IoServiceCore::start()
		{
			_netThread->run();
			LOGT("Create net thread [ %s ]", _netThread->getThreadId());

			_logicThread->run();
			LOGT("Create main logic thread [ %s ]", _logicThread->getThreadId());
		}

		Thread::ThreadPtr IoServiceCore::getThreadByName(const char* name)
		{
			auto it = _namedThread.find(name);
			if (it == _namedThread.end())
				return Thread::ThreadPtr();
			return it->second;
		}

		Thread::ThreadPtr IoServiceCore::createThreadByName(const char* name)
		{
			auto thread = getThreadByName(name);
			if (thread)
				return thread;
			return createNamedThread(name);
		}

		Thread::ThreadPtr IoServiceCore::createNamedThread(const char* name)
		{
			auto thread = Thread::create();
			thread->run();

			LOGT("Create named thread [ %s -- %s ]", name, thread->getThreadId());

			_namedThread.insert(make_pair(name, thread));
			return thread;
		}

		Thread::Thread()
		{
			_ioService = ioServicePtr(new boost::asio::io_service);
			_work = workPtr(new boost::asio::io_service::work(*_ioService));
		}

		Thread::~Thread()
		{
			stop();
		}

		Thread::ThreadPtr Thread::create()
		{
			return ThreadPtr(new Thread);
		}

		void Thread::stop()
		{
			_ioService->stop();
			_thread->join();
		}

		void Thread::run()
		{
			_thread = threadPtr(new boost::thread(boost::bind(&boost::asio::io_service::run, _ioService)));
		}

// #if _MSC_VER >= 1800

		JME_TaskThread::JME_TaskThread():
			_stop(false)
		{
			_thread = boost::thread([this]()
			{
				while (!this->_stop)
				{
					boost::function<void()> task;
					{
						boost::unique_lock<boost::mutex> lock(_mutex);
						this->_condition.wait(lock, [this]{return this->_stop || !this->_tasks.empty();});
						if(this->_stop && this->_tasks.empty()) 
							return;
						task = boost::move(this->_tasks.front());
						this->_tasks.pop();
					}
					task();
				}
			});
		}
// #endif // _MSC_VER >= 1800

	}
}
