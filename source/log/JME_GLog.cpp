#include <iostream>
#include <string>
#include <sys/stat.h>
#include "log/JME_GLog.h"
#include "log/JME_GLogLevel.h"
#include "log/JME_GLogColor.h"
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
		const static int MinWriteLogInterval = 15;
		GLog& GLog::getInstance()
		{
			static auto pInstance = new GLog;
			return *pInstance;
		}
	
		void GLog::readClassLogLevelConfig(const string& cfg) //读取单个类日志级别配置
		{
			auto conf = JMEngine::file::load_jsonfile_val(cfg);

			_file_level = getGLogLevelByValue(conf["file_level"].asInt());
			_screen_level = getGLogLevelByValue(conf["screen_level"].asInt());
			_name = conf["name"].asString();
			_log_path = conf["log_path"].asString();
			_split_size = conf["split_size"].asUInt() * 1024;

			_write_interval = std::max(conf["write_interval"].asInt(), MinWriteLogInterval);
			if(!boost::filesystem::is_directory(_log_path))
			{
				boost::filesystem::create_directory(_log_path);
			}
		}
		

		void GLog::readConfig(std::string cfg)
		{
			readClassLogLevelConfig(cfg);
			printConfig();
		}

		void GLog::printConfig()
		{
			cout << "==========================================================================" << endl;
			cout << "| Type level : ALL=1,TRACE=2,DEBUG=3,INFO=4,WARN=5,ERROR=6,FATAL=7,OFF=8 |" << endl;
			cout << "--------------------------------------------------------------------------" << endl;
			cout << "| Log file level:   " << getGLogLevelName(_file_level) <<endl;
			cout << "| Log screen level: " << getGLogLevelName(_screen_level) <<endl;
			cout << "| Log name:         " << _name <<endl;
			cout << "| Log file dir:	 " << _log_path <<endl;
			cout << "==========================================================================" << endl;
		}

		string GLog::getGLogLevelName( GLogLevel level )
		{
			switch (level)
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

		GLogLevel GLog::getGLogLevelByValue(int val)
		{
			switch (val)
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

		void GLog::openOrCreateFile( const string& file )
		{
			if (file.compare(_last_log_file))
			{
				if (_log_stream.is_open())
				{
					_log_stream.close();
				}
				_last_log_file = file;
			}
			if (!_log_stream.is_open())
			{
				_log_stream.open(_last_log_file.c_str(),ios_base::app);
			}
		}

		bool GLog::checkAndUpdateLastCreateFileDate()
		{
			if(boost::filesystem::is_directory(_log_path)==false)
				boost::filesystem::create_directory(_log_path);

			boost::posix_time::ptime p = second_clock::local_time();
			struct tm now = boost::posix_time::to_tm(p);
			
			if (now.tm_year != this->_last_time.tm_year
				|| now.tm_mon != this->_last_time.tm_mon
				|| now.tm_mday != this->_last_time.tm_mday
				)
			{
				_last_time = now;
				_split_index = 1;
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
			return (fmt % (_last_time.tm_year + 1900) % (_last_time.tm_mon + 1) % _last_time.tm_mday).str();
		}

		bool GLog::checkToFileLogLevel(GLogLevel level)
		{
			return _file_level <= level;//日志级别小于全局
		}

		bool GLog::checkToScreenLogLevel(GLogLevel level)
		{
			return _screen_level <= level;
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

		GLog::GLog():
			_name(""),
			_log_path("./log/"),
			_last_log_file(""),
			_file_level(GLog_TRACE),
			_screen_level(GLog_TRACE),
			_split_index(1),
			_split_size(0),
			_write_interval(MinWriteLogInterval)
		{
			_waiting_list = new list<string>;
			_writing_list = new list<string>;

			_log_ioservice = boost::make_shared<boost::asio::io_service>();
			_log_worker = boost::make_shared<boost::asio::io_service::work>(*_log_ioservice);
			_log_thread = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, _log_ioservice));

			auto dt = boost::make_shared<boost::asio::deadline_timer>(*_log_ioservice);
			dt->expires_from_now(boost::posix_time::seconds(_write_interval));
			dt->async_wait(boost::bind(&GLog::run, this, dt));
		}

		GLog::~GLog()
		{
			printf("\033[0m");

			_log_ioservice->stop();
			_log_thread->join();

			delete _waiting_list;
			delete _writing_list;
		}

		void GLog::setLogLevel( GLogLevel level )
		{
			_screen_level = level;
		}

		JMEngine::log::GLogLevel GLog::getLogLevel()
		{
			return _screen_level;
		}

		void GLog::checkLogFile()
		{
			checkAndUpdateLastCreateFileDate();

			if (_split_size <= 0)
			{
				boost::format fmt("%s%s.%s.log");
				string log_name = (fmt % this->_log_path % this->_name % getCurrentDateTime()).str();
				return openOrCreateFile(log_name);
			}

			boost::format fmt("%s%s.%s.log.%d");
			string log_name = (fmt % this->_log_path % this->_name % getCurrentDateTime() % _split_index).str();
			while (1)
			{
				struct stat fno;
				int result = stat(log_name.c_str(), &fno);
				if (0 == result && fno.st_size >= _split_size && _split_size != 0)
				{
					log_name = (fmt % this->_log_path % this->_name % getCurrentDateTime() % ++_split_index).str();
				}
				else
				{
					return openOrCreateFile(log_name);
				}
			}
		}

		void GLog::run(boost::shared_ptr<boost::asio::deadline_timer> dt)
		{
			//交换两个队列
			{
				//如果没有等待写入文件的日志
				boost::mutex::scoped_lock l(_log_mutex);
				if (_waiting_list->empty())
				{
					dt->expires_from_now(boost::posix_time::seconds(_write_interval));
					dt->async_wait(boost::bind(&GLog::run, this, dt));
					return;
				}
				list<string>* list = _writing_list;
				_writing_list = _waiting_list;
				_waiting_list = list;
			}

			checkLogFile();
			for (auto& log : *_writing_list)
			{
				_log_stream << log;
			}
			_log_stream.flush();

			_writing_list->clear();

			dt->expires_from_now(boost::posix_time::seconds(_write_interval));
			dt->async_wait(boost::bind(&GLog::run, this, dt));
		}

	}
}
