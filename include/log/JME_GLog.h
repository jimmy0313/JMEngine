#ifndef JME_GLog_h__
#define JME_GLog_h__

#include <list>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <boost/system/error_code.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include "boost/thread.hpp"
#include "boost/asio.hpp"

#include "log/JME_GLogLevel.h"
#include "log/JME_GLogColor.h"

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
		private:
			GLog();
			GLog(const GLog&){}
			GLog& operator = (const GLog&){}

		public:
			virtual ~GLog();

			static GLog& getInstance();//获取日志单例对象引用

			static string getGLogLevelName(GLogLevel level);
			static GLogLevel getGLogLevelByValue(int val);

			void setLogLevel(GLogLevel level);
			GLogLevel getLogLevel();

			void readConfig(string cfg="./config/log.json");//读取配置
			void printConfig();//打印配置

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
			string _name;
			string _log_path;
			string _last_log_file;
			ofstream _log_stream;
			tm _last_time;				
			GLogLevel _file_level;			//写入文件日志级别
			GLogLevel _screen_level;		//屏幕打印日志级别
			int _split_index;				//用于按大小进行文件分割
			size_t _split_size;				//分割标准
			unsigned int _write_interval;	//写入文件间隔

			// for supporting multiple threads
			boost::mutex _log_mutex;

			void readClassLogLevelConfig(const string& cfg);

			void openOrCreateFile(const string& file);

			bool checkAndUpdateLastCreateFileDate();

			void checkLogFile();

			string getCurrentDateTime();

			bool checkToFileLogLevel(GLogLevel level);
			bool checkToScreenLogLevel(GLogLevel level);

			list<string>* _waiting_list;
			list<string>* _writing_list;

			boost::shared_ptr<boost::asio::io_service> _log_ioservice;
			boost::shared_ptr<boost::asio::io_service::work> _log_worker;
			boost::shared_ptr<boost::thread> _log_thread;
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

				auto p = boost::posix_time::second_clock::local_time();
				struct tm now = boost::posix_time::to_tm(p);

				boost::format fmt_time("%d-%02d-%02d %d:%d:%d");
				auto ntm =  (fmt_time % (now.tm_year + 1900) % (now.tm_mon + 1) % now.tm_mday % now.tm_hour % now.tm_min % now.tm_sec).str();
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
					_waiting_list->emplace_back(logStr);
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