#ifndef JME_GLog_h__
#define JME_GLog_h__


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

using namespace std;
using namespace JMEngine::log;

#define logger JMEngine::log::GLog::getInstance()
#define GLogger logger.preLog(__FUNCTION__,__LINE__)

#define GLogTrace(color) \
	logger.trace(color,__FUNCTION__,__LINE__)

#define GLogDebug(color)  \
	logger.debug(color,__FUNCTION__,__LINE__)

#define GLogInfo(color) \
	logger.info(color,__FUNCTION__,__LINE__) 

#define GLogWarn(color)  \
	logger.warn(color,__FUNCTION__,__LINE__)

#define GLogError(color) \
	logger.error(color,__FUNCTION__,__LINE__)

#define GLogFatal(color) \
	logger.fatal(color,__FUNCTION__,__LINE__)


#define SetLogName(n) \
	logger.setLogName(n)

#define LogT \
    { logger.lock(); \
	logger.trace(JMEngine::log::White,__FUNCTION__,__LINE__)

#define LogD  \
    { logger.lock(); \
	logger.debug(JMEngine::log::Yellow,__FUNCTION__,__LINE__)

#define LogI \
    {  logger.lock(); \
	logger.info(JMEngine::log::Green,__FUNCTION__,__LINE__) 

#define LogW  \
    {  logger.lock(); \
	logger.warn(JMEngine::log::Pink,__FUNCTION__,__LINE__)

#define LogE \
    { logger.lock(); \
	logger.error(JMEngine::log::Red,__FUNCTION__,__LINE__)

#define LogF \
    { logger.lock(); \
	logger.fatal(JMEngine::log::Red,__FUNCTION__,__LINE__)

#define LogEnd \
	logger.end_line(__FUNCTION__,__LINE__); \
	logger.unlock();} 

#define color_red(A)	Red		<< A <<  logger.getCurrentColor()
#define color_pink(A)	Pink	<< A <<  logger.getCurrentColor()
#define color_green(A)	Green	<< A <<  logger.getCurrentColor()
#define color_white(A)	White	<< A <<  logger.getCurrentColor()
#define color_yellow(A)	Yellow	<< A <<  logger.getCurrentColor()

namespace JMEngine
{
	namespace log
	{
		typedef std::map<string,int> class_logLevel_map;
		
		class GLog
		{
		public:
			GLog(){};
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

			//write log
			GLog& preLog(GLogLevel level,GLogColor color,const char* functionName,int lineNumber);

			GLog& trace(GLogColor color,const char* functionName,int lineNumber);
			GLog& debug(GLogColor color,const char* functionName,int lineNumber);
			GLog& info(GLogColor color,const char* functionName,int lineNumber);
			GLog& warn(GLogColor color,const char* functionName,int lineNumber);
			GLog& error(GLogColor color,const char* functionName,int lineNumber);
			GLog& fatal(GLogColor color,const char* functionName,int lineNumber);

			GLogColor getCurrentColor() {return White;}
			std::string end_line(const char* functionName,int lineNumber);
			//设置颜色
			void setColor(GLogColor color);

			GLog &operator<<(const int& msg);
			GLog &operator<<(const short& msg);
			GLog &operator<<(const unsigned int& msg);
			GLog &operator<<(const unsigned short& msg);
			GLog &operator<<(const unsigned long& msg);
			GLog &operator<<(const float& msg);
			GLog &operator<<(const double& msg);
			GLog &operator<<(const char& msg);
			GLog &operator<<(const char* msg);
			GLog &operator<<(const string& msg);
			GLog &operator<<(const time_t& msg);
			GLog &operator<<(const boost::system::error_code& msg);
			GLog &operator<<(GLogColor color);

			void lock() { _log_mutex.lock();}
			void unlock() {_log_mutex.unlock();}

			private:
				static string serverName;
				static string logPathName;
				static string lastLogFullFileName;
				static ofstream logOfstream;
				static tm lastDateTime;
				static GLogLevel fileLogLevel;//全局日志级别
				static GLogLevel screenLogLevel;//屏幕日志打印级别
				static class_logLevel_map classLogLevelMap;//单个类日志级别配置	
				static string fileIdx;	//用于按大小进行文件分割
				static size_t fileSize;	//分割标准

				bool isOutPutFile;
				bool isOutPutScreen;

				// for supporting multiple threads
				boost::mutex _log_mutex;

				void readClassLogLevelConfig(std::string cfg);

				void openOrCreateFile(string fileName);

				bool checkAndUpdateLastCreateFileDate();

				void checkLogFile();

				string getCurrentDateTime();
				std::string getClassString(std::string function_str);

				bool checkToFileLogLevel(GLogLevel level,string& className,bool check_father=false);
				bool checkToScreenLogLevel(GLogLevel level,string& className,bool check_father=false);

				template <typename F>
				GLog& logToFile(F f);

				template <typename T>
				GLog& logToScreen(T t);
		};

		class GLogTimer
		{
		public :
			GLogTimer(const char * func_name) : _func_name(func_name)
			{
				_startTime = boost::get_system_time();
			}
			~GLogTimer()
			{
				boost::system_time endTime = boost::get_system_time();
				int elapse = (int)(endTime - _startTime).total_milliseconds();
				LogI << "process [" << _func_name << "] consume " << elapse << "ms" << LogEnd;
			}
		private:
			std::string _func_name;  // trace function name
			boost::system_time _startTime;
		};
	}
}

#endif // JME_GLog_h__