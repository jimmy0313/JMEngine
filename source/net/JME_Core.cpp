#include "JME_Core.h"
#include "JME_GLog.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

namespace JMEngine
{
	namespace net
	{
		IoServiceCore::IoServiceCore( void )
		{
			_netThread = JME_Thread::create();
			_netThread->run();

			LogT << "Create net thread " << _netThread->getThreadId() << LogEnd;

			_logicThread = JME_Thread::create();
			_logicThread->run();

			LogT << "Create logic thread " << _logicThread->getThreadId() << LogEnd;
		}

		IoServiceCore::~IoServiceCore( void )
		{
			_netThread->stop();
			_logicThread->stop();
		}

		boost::asio::io_service& IoServiceCore::getNetIoService()
		{
			return *_netThread->ioService();
		}

		boost::asio::io_service& IoServiceCore::getLogicioService()
		{
			return *_logicThread->ioService();
		}

		JME_Thread::JME_Thread()
		{
			_ioService = ioServicePtr(new boost::asio::io_service);
			_work = workPtr(new boost::asio::io_service::work(*_ioService));
		}

		JME_Thread::~JME_Thread()
		{
			stop();
		}

		JME_Thread::JME_ThreadPtr JME_Thread::create()
		{
			return JME_ThreadPtr(new JME_Thread);
		}

		void JME_Thread::stop()
		{
			_ioService->stop();
			_thread->join();
		}

		void JME_Thread::run()
		{
			_thread = threadPtr(new boost::thread(boost::bind(&boost::asio::io_service::run, _ioService)));
		}

	}
}
