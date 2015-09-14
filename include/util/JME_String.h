#ifndef JME_String_h__
#define JME_String_h__

#include <vector>
#include <string>
#include <boost/cstdint.hpp>

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

		void split(const string& str, vector<string>& strVec, const char sign = '|');

		int stringLength(const string& inStr);
		int stringLength(const char* str, int len);
	}
}
#endif // JME_String_h__