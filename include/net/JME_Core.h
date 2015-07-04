#ifndef JME_Core_h__
#define JME_Core_h__

#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/detail/singleton.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/lexical_cast.hpp"
#include <vector>

#define JMECore boost::detail::thread::singleton<JMEngine::net::IoServiceCore>::instance()

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace JMEngine
{
	namespace net
	{
		class JME_Thread final
		{
		public:
			friend class IoServiceCore;

			typedef boost::shared_ptr<JME_Thread> JME_ThreadPtr;
			typedef boost::shared_ptr<io_service> ioServicePtr;
			typedef boost::shared_ptr<io_service::work> workPtr;
			typedef boost::shared_ptr<thread> threadPtr;

		public:
			JME_Thread();
			~JME_Thread();

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

			boost::asio::io_service& getNetIoService();
			boost::asio::io_service& getLogicioService();

			void stop();

		private:
			JME_Thread::JME_ThreadPtr _netThread;
			JME_Thread::JME_ThreadPtr _logicThread;
		};
	}
}



#endif // JME_Core_h__
