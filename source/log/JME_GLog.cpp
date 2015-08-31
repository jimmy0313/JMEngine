#include <iostream>
#include <string>
#include <sys/stat.h>
#include "JME_GLog.h"
#include "JME_GLogLevel.h"
#include "JME_GLogColor.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/make_shared.hpp"
#include <ctime>
#include "json/json.h"
#include "json/JME_JsonFile.h"

#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;
using namespace JMEngine::log;
using namespace boost::posix_time;

namespace JMEngine
{
	namespace log
	{
		static GLog* m_pInstance = NULL;
		string GLog::serverName="log";
		string GLog::logPathName="./log/";
		string GLog::lastLogFullFileName="";
		ofstream GLog::logOfstream;
		tm GLog::lastDateTime;
		GLogLevel GLog::fileLogLevel=GLog_TRACE;
		GLogLevel GLog::screenLogLevel=GLog_TRACE;
		int GLog::fileIdx=1;
		size_t GLog::fileSize=0;

		GLog& GLog::getInstance()
		{
			if ( m_pInstance == NULL )
			{
				m_pInstance = new GLog();
				m_pInstance->setColor(White);
			}

			return *m_pInstance;
		}
	
		void GLog::readClassLogLevelConfig(std::string cfg) //读取单个类日志级别配置
		{
			const std::string file_name = cfg;
			Json::Value logConfig_Value = JMEngine::file::load_jsonfile_val(file_name);

			fileLogLevel=getGLogLevelByValue(logConfig_Value["fileLogLevel"].asInt());
			screenLogLevel=getGLogLevelByValue(logConfig_Value["screenLogLevel"].asInt());
			serverName=logConfig_Value["serverName"].asString();
			logPathName=logConfig_Value["logPathName"].asString();
			fileSize=logConfig_Value["splitSize"].asUInt() * 1024;

			if(boost::filesystem::is_directory(logPathName)==false)
			{
				boost::filesystem::create_directory(logPathName);
			}
		}
		

		void GLog::readConfig(std::string cfg)
		{
			readClassLogLevelConfig(cfg);
			printConfig();
		}

		void GLog::printConfig()
		{
			cout << "Type level : ALL=1,TRACE=2,DEBUG=3,INFO=4,WARN=5,ERROR=6,FATAL=7,OFF=8" << endl;
			cout << "----------------------------------------------------" << endl;
			cout << "Log file level:   " << getGLogLevelName(fileLogLevel) <<endl;
			cout << "Log screen level: " << getGLogLevelName(screenLogLevel) <<endl;
			cout << "Log name:         " << serverName <<endl;
			cout << "Log file dir:	   " << logPathName <<endl;
			cout << "====================================================" << endl;
		}

		string GLog::getGLogLevelName( GLogLevel enumValue )
		{
			switch (enumValue)
			{
			case GLog_ALL:
				return "ALL";
			case GLog_TRACE:
				return "TRACE";
			case GLog_DEBUG:
				return "DEBUG";
			case GLog_INFO:
				return "INFO";
			case GLog_WARN:
				return "WARN";
			case GLog_ERROR:
				return "ERROR";
			case GLog_FATAL:
				return "FATAL";
			case GLog_OFF:
				return "OFF";
			}
			return "";
		}
		string GLog::getGLogLevelShortName( GLogLevel enumValue )
		{
			switch (enumValue)
			{
			case GLog_ALL:
				return "ALL";
			case GLog_TRACE:
				return "T";
			case GLog_DEBUG:
				return "D";
			case GLog_INFO:
				return "I";
			case GLog_WARN:
				return "W";
			case GLog_ERROR:
				return "E";
			case GLog_FATAL:
				return "F";
			case GLog_OFF:
				return "OFF";
			}
			return "";
		}
		GLogLevel GLog::getGLogLevelByValue(int enumValue)
		{
			switch (enumValue)
			{
			case 1:
				return GLog_ALL;
			case 2:
				return GLog_TRACE;
			case 3:
				return GLog_DEBUG;
			case 4:
				return GLog_INFO;
			case 5:
				return GLog_WARN;
			case 6:
				return GLog_ERROR;
			case 7:
				return GLog_FATAL;
			case 8:
				return GLog_OFF;
			}

			return GLog_ALL;
		}

		void GLog::openOrCreateFile( string fileName )
		{
			if (fileName.compare(lastLogFullFileName))
			{
				if (logOfstream.is_open())
				{
					logOfstream.close();
				}
				lastLogFullFileName = fileName;
			}
			if (!logOfstream.is_open())
			{
				logOfstream.open(lastLogFullFileName.c_str(),ios_base::app);
			}
		}

		bool GLog::checkAndUpdateLastCreateFileDate()
		{
			if(boost::filesystem::is_directory(logPathName)==false)
				boost::filesystem::create_directory(logPathName);

			boost::posix_time::ptime p = second_clock::local_time();
			struct tm now=boost::posix_time::to_tm(p);
			
			if (now.tm_year != this->lastDateTime.tm_year
				|| now.tm_mon != this->lastDateTime.tm_mon
				|| now.tm_mday != this->lastDateTime.tm_mday
				)
			{
				lastDateTime = now;
				fileIdx = 1;
				return true;
			}
			else
			{

				return false;
			}
		}

		string GLog::getCurrentDateTime()
		{
			boost::format fmt("%d%02d%02d");
			return (fmt % (lastDateTime.tm_year + 1900) % (lastDateTime.tm_mon + 1) % lastDateTime.tm_mday).str();
		}

		bool GLog::checkToFileLogLevel(GLogLevel level)
		{
			return fileLogLevel <= level;//日志级别小于全局
		}

		bool GLog::checkToScreenLogLevel(GLogLevel level)
		{
			return screenLogLevel <= level;
		}
		
		void GLog::setColor(GLogColor color)
		{
			#ifdef WIN32
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
				SetConsoleTextAttribute(hConsole,color);
			#else
				printf("\033[%d;40;1m",color); 
			#endif

		}

		GLog::GLog()
		{
			_waitLogList = new list<string>;
			_writeLogList = new list<string>;

			_logIoService = boost::make_shared<boost::asio::io_service>();
			_logWorker = boost::make_shared<boost::asio::io_service::work>(*_logIoService);
			_logThread = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, _logIoService));

			auto dt = boost::make_shared<boost::asio::deadline_timer>(*_logIoService);
			dt->expires_from_now(boost::posix_time::seconds(15));
			dt->async_wait(boost::bind(&GLog::run, this, dt));
		}

		GLog::~GLog()
		{
			printf("\033[0m");

			_logIoService->stop();
			_logThread->join();

			delete _waitLogList;
			delete _writeLogList;
		}

		void GLog::setLogLevel( GLogLevel level )
		{
			screenLogLevel = level;
		}

		JMEngine::log::GLogLevel GLog::getLogLevel()
		{
			return screenLogLevel;
		}

		void GLog::checkLogFile()
		{
			checkAndUpdateLastCreateFileDate();
			boost::format fmt("%s%s.%s.log.%d");

			string tempName = (fmt % this->logPathName % this->serverName % getCurrentDateTime() % fileIdx).str();

			while (1)
			{
				struct stat fno;
				int result = stat(tempName.c_str(), &fno);
				if (0 == result && fno.st_size >= fileSize && fileSize != 0)
				{
					tempName = (fmt % this->logPathName % this->serverName % getCurrentDateTime() % ++fileIdx).str();
				}
				else
				{
					return openOrCreateFile(tempName);
				}
			}
		}

		void GLog::run(boost::shared_ptr<boost::asio::deadline_timer> dt)
		{
			//交换两个队列
			{
				//如果没有等待写入文件的日志
				boost::mutex::scoped_lock l(_log_mutex);
				if (_waitLogList->empty())
					return;

				list<string>* list = _writeLogList;
				_writeLogList = _waitLogList;
				_waitLogList = list;
			}

			checkLogFile();
			for (auto& log : *_writeLogList)
			{
				logOfstream << log;
			}
			logOfstream.flush();

			_writeLogList->clear();
			
			dt->expires_from_now(boost::posix_time::seconds(15));
			dt->async_wait(boost::bind(&GLog::run, this, dt));
		}

	}
}
