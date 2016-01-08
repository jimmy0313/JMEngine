#ifndef JME_Core_h__
#define JME_Core_h__

#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/detail/singleton.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/function.hpp"
#include <boost/bind.hpp>
#include "boost/thread/future.hpp"
#include "boost/utility/result_of.hpp"
#include <vector>
#include <queue>

// #define JMECore boost::detail::thread::singleton<JMEngine::net::IoServiceCore>::instance()
#define JMECore (*JMEngine::net::IoServiceCore::getInstance())

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace JMEngine
{
	namespace net
	{
		class Thread final
		{
		public:
			friend class IoServiceCore;

			typedef boost::shared_ptr<Thread> JME_ThreadPtr;
			typedef boost::shared_ptr<io_service> ioServicePtr;
			typedef boost::shared_ptr<io_service::work> workPtr;
			typedef boost::shared_ptr<thread> threadPtr;

		public:
			Thread();
			~Thread();

			static JME_ThreadPtr create();
			
			void run();
			void stop();

			ioServicePtr ioService() { return _ioService; }
			string getThreadId() { return boost::lexical_cast<string>(_thread->get_id()); }
		private:
			ioServicePtr _ioService;	
			workPtr _work;
			threadPtr _thread;
		};

		class IoServiceCore :
			private boost::noncopyable
		{
			typedef boost::shared_ptr<io_service> io_service_ptr;
			typedef boost::shared_ptr<io_service::work> work_ptr;
			typedef boost::shared_ptr<thread> thread_ptr;

		public:
			IoServiceCore(void);
			~IoServiceCore(void);

			static IoServiceCore* getInstance();

			boost::asio::io_service& getNetIoService();
			boost::asio::io_service& getLogicioService();

			void start();
			void stop();

		private:
			Thread::JME_ThreadPtr _netThread;
			Thread::JME_ThreadPtr _logicThread;
		};

/*#if _MSC_VER >= 1800*/



		class JME_TaskThread final
		{
		public:
			JME_TaskThread();

			template<typename F>
			auto post(F&& f) -> typename boost::result_of<F()>::type;

		private:
			bool _stop;
			boost::thread _thread;
			queue<boost::function<void()> > _tasks;
			boost::mutex _mutex;
			boost::condition_variable _condition;
		};

		template<typename F>
		auto JMEngine::net::JME_TaskThread::post( F&& f ) -> typename boost::result_of<F()>::type
		{
			typedef typename boost::result_of<F()>::type return_type;
			auto task = boost::make_shared<boost::packaged_task<return_type>>(
				boost::bind(boost::forward<F>(f))
			);

			boost::unique_future<return_type> res = task->get_future();
			{
				boost::unique_lock<boost::mutex> lock(_mutex);
				if(!_stop) 
					_tasks.emplace([task](){(*task)();});
			}
			_condition.notify_one();
// 			res.timed_wait(boost::posix_time::microsec(5));
			return res.get();
		}

		class JME_TaskThreadPool final
		{

		};
// #endif // _MSC_VER >= 1800
	}
}



#endif // JME_Core_h__
