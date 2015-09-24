#ifndef JME_String_h__
#define JME_String_h__

#include <vector>
#include <string>
#include <boost/cstdint.hpp>
#include "boost/format.hpp"
#include "JME_GLog.h"

using namespace std;
using namespace boost;

namespace JMEngine
{
	namespace tools
	{
		//GB to utf8
		void GB2312ToUTF_8(string& pOut,char *pText, int pLen);
		void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer);
		void UnicodeToUTF_8(char* pOut,wchar_t* pText);

		void UTF_8ToGB2312(string &pOut, char *pText, int pLen);
		void UnicodeToGB2312(char* pOut,wchar_t uData);
		void UTF_8ToUnicode(wchar_t* pOut,char *pText);

		void split(const string& str, vector<string>& strVec, const char* sign);

		int stringLength(const string& inStr);
		int stringLength(const char* str, int len);

		template<class... T>
		string createString(const char* fmt, const T... t);

		template<class T1, class... T2>
		void format(boost::format& fmt, const T1 t1, const T2... t2);

		template<class T>
		void format(boost::format& fmt, const T t);
	}

	template<class T1, class... T2>
	void JMEngine::tools::format(boost::format& fmt, const T1 t1, const T2... t2)
	{
		fmt % t1;
		format(fmt, t2...);
	}

	template<class T>
	void JMEngine::tools::format(boost::format& fmt, const T t)
	{
		fmt % t;
	}

	template<class... T>
	string JMEngine::tools::createString(const char* fmt, const T... t)
	{
		try
		{
			boost::format fmt_(fmt);
			format(fmt_, t...);
			return fmt_.str();
		}
		catch(const std::exception& e)
		{
			LOGE("%s with [ %s ]", e.what() , fmt);
		}
		return "";
	}

}
#endif // JME_String_h__