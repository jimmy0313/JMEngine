#ifndef JME_GLog_h__
#define JME_GLog_h__

#include <list>
#include <string>
#include <map>
#include "JME_GLogLevel.h"
#include "JME_GLogColor.h"
#include <iostream>
#include <fstream>
#include <boost/system/error_code.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include "boost/thread.hpp"
#include "boost/asio.hpp"

using namespace std;
using namespace JMEngine::log;

#define logger JMEngine::log::GLog::getInstance()

#define LOGE( ...) \
	logger.log(GLog_ERROR, JMEngine::log::Red, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGI(...) \
	logger.log(GLog_INFO, JMEngine::log::Green, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGT(...) \
	logger.log(GLog_TRACE, JMEngine::log::White, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGD(...) \
	logger.log(GLog_DEBUG, JMEngine::log::Yellow, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGW(...) \
	logger.log(GLog_WARN, JMEngine::log::Pink, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define LOGF(...) \
	logger.log(GLog_WARN, JMEngine::log::Red, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

namespace JMEngine
{
	namespace log
	{		
		class GLog
		{
		public:
			GLog();
			~GLog();

			static std::string getGLogLevelName(GLogLevel enumValue);
			static std::string getGLogLevelShortName(GLogLevel enumValue);
			static GLogLevel getGLogLevelByValue(int enumValue);

			static GLog& getInstance();//获取日志单例对象引用

			void setLogLevel(GLogLevel level);
			GLogLevel getLogLevel();

			void readConfig(std::string cfg="./config/log_cfg.json");//读取配置
			void printConfig();//打印配置
			void setLogName(const char * logName) { serverName = logName;}

			//设置颜色
			void setColor(GLogColor color);

			template<class T1, class... T2>
			void log(GLogLevel level, GLogColor color, const char* file, const char* function, int line, const char* log, T1&& t1, T2&&... t2);

			template<class T1>
			void log(GLogLevel level, GLogColor color, const char* file, const char* function, int line, T1&& t1);
		private:

			template<class T1>
			void format(boost::format& fmt, T1&& t1);

			template<class T1, class... T2>
			void format(boost::format& fmt, T1&& t1, T2&&... t2);

			template<class T1, class... T2>
			string format(const char* fmt, T1&& t1, T2&&... t2);

			void run(boost::shared_ptr<boost::asio::deadline_timer> dt);
		private:
			static string serverName;
			static string logPathName;
			static string lastLogFullFileName;
			static ofstream logOfstream;
			static tm lastDateTime;
			static GLogLevel fileLogLevel;//全局日志级别
			static GLogLevel screenLogLevel;//屏幕日志打印级别
			static int fileIdx;	//用于按大小进行文件分割
			static size_t fileSize;	//分割标准

			// for supporting multiple threads
			boost::mutex _log_mutex;

			void readClassLogLevelConfig(std::string cfg);

			void openOrCreateFile(string fileName);

			bool checkAndUpdateLastCreateFileDate();

			void checkLogFile();

			string getCurrentDateTime();

			bool checkToFileLogLevel(GLogLevel level);
			bool checkToScreenLogLevel(GLogLevel level);

			list<string>* _waitLogList;
			list<string>* _writeLogList;

			boost::shared_ptr<boost::asio::io_service> _logIoService;
			boost::shared_ptr<boost::asio::io_service::work> _logWorker;
			boost::shared_ptr<boost::thread> _logThread;
		};

		template<class T1>
		void JMEngine::log::GLog::log(GLogLevel level, GLogColor color, const char* file, const char* function, int line, T1&& t1)
		{
			try
			{
				bool isOutFile = checkToFileLogLevel(level);
				bool isOutScreen = checkToScreenLogLevel(level);

				if (!isOutFile && !isOutScreen)
					return;

				boost::format fmt("[%s] [%s:%d:%s:%s] [%s] %s\n");

				auto ntm = boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
				auto thread = boost::lexical_cast<string>(boost::this_thread::get_id());

				auto logStr = boost::str(fmt % ntm % file % line % function % thread % getGLogLevelName(level) % t1);

				if (isOutScreen)
				{
					_log_mutex.lock();
					setColor(color);
					cout << logStr;
					setColor(GLogColor(White));
					_log_mutex.unlock();
				}

				if (isOutFile)
				{
					_log_mutex.lock();
					_waitLogList->emplace_back(logStr);
					_log_mutex.unlock();
				}
			}
			catch(const std::exception& e)
			{
				logger.log(GLog_ERROR, JMEngine::log::Red, file, function, line, "format log error ==> [ %s ]", e.what());
			}
		}

		template<class T1, class... T2>
		void JMEngine::log::GLog::log(GLogLevel level, GLogColor color, const char* file, const char* function, int line, const char* log, T1&& t1, T2&&... t2)
		{
			try
			{
				bool isOutFile = checkToFileLogLevel(level);
				bool isOutScreen = checkToScreenLogLevel(level);

				if (!isOutFile && !isOutScreen)
					return;

				auto str = format(log, t1, t2...);

				this->log(level, color, file, function, line, str);
			}
			catch(const std::exception& e)
			{
				logger.log(GLog_ERROR, JMEngine::log::Red, file, function, line, "format log error ==> [ %s ]", e.what());
			}
		}

		template<class T1, class... T2>
		void JMEngine::log::GLog::format(boost::format& fmt, T1&& t1, T2&&... t2)
		{
			fmt % t1;
			format(fmt, t2...);
		}

		template<class T1>
		void JMEngine::log::GLog::format(boost::format& fmt, T1&& t1)
		{
			fmt % t1;
		}

		template<class T1, class... T2>
		string JMEngine::log::GLog::format(const char* fmt, T1&& t1, T2&&... t2)
		{
			boost::format fmt_(fmt);
			format(fmt_, t1, t2...);

			return fmt_.str();
		}
	}
}

#endif // JME_GLog_h__