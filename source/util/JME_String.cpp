#include "JME_String.h"
#include "JME_GLog.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#ifdef WIN32
#include <Windows.h> // for	MultiByteToWideChar functions
#endif

#ifdef LINUX
#include <iconv.h>
#endif // LINUX

namespace hoyosvr
{
	namespace tools
	{

#ifdef LINUX		
		int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
		{
			iconv_t cd;
			//int rc;
			char **pin = &inbuf;
			char **pout = &outbuf;

			cd = iconv_open(to_charset,from_charset);
			if (cd==0) 
				return -1;

			memset(outbuf,0,outlen);

			size_t s_in_len = inlen;
			size_t s_out_len = outlen;

			if (iconv(cd,pin,&s_in_len,pout,&s_out_len)==-1) 
				return -1;

			iconv_close(cd);
			return 0;
		}
#endif

		void UnicodeToUTF_8(char* pOut,wchar_t* pText)
		{
			// ע�� WCHAR�ߵ��ֵ�˳��,���ֽ���ǰ�����ֽ��ں�
			char* pchar = (char *)pText;

			pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
			pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
			pOut[2] = (0x80 | (pchar[0] & 0x3F));

			return;
		}	
		void UTF_8ToUnicode(wchar_t* pOut,char *pText)
		{
			char* uchar = (char *)pOut;

			uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
			uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

			return;
		}
		void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer)
		{
#ifdef WIN32	
			::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1);
#endif
#ifdef LINUX			
			code_convert("gbk","utf-8",gbBuffer,sizeof(gbBuffer), (char*)pOut, sizeof(pOut));
#endif
		}
		void UnicodeToGB2312(char* pOut,wchar_t uData)
		{
#ifdef WIN32		
			::WideCharToMultiByte(CP_ACP,NULL,&uData,1,pOut,sizeof(wchar_t),NULL,NULL);
#endif
#ifdef LINUX
			code_convert("utf-8","gbk",(char*)uData,sizeof(uData), pOut, sizeof(pOut));
#endif
		}


		void GB2312ToUTF_8(string& pOut,char *pText, int pLen)
		{
			if( 0 == pLen)
				return;

			char buf[4];
			int nLength = pLen* 3;
			char* rst = new char[nLength];
			memset(buf,0,4);
			memset(rst,0,nLength);	
#ifdef WIN32	
			int i = 0;
			int j = 0;      
			while(i < pLen)
			{
				//�����Ӣ��ֱ�Ӹ��ƾ���
				if( *(pText + i) >= 0)
				{
					rst[j++] = pText[i++];
				}
				else
				{
					wchar_t pbuffer;
					Gb2312ToUnicode(&pbuffer,pText+i);

					UnicodeToUTF_8(buf,&pbuffer);

					unsigned short int tmp = 0;
					tmp = rst[j] = buf[0];
					tmp = rst[j+1] = buf[1];
					tmp = rst[j+2] = buf[2];    

					j += 3;
					i += 2;
				}
			}
			rst[j] = ' ';
#endif  

#ifdef LINUX
			code_convert("gbk","utf-8",pText,pLen, rst, nLength);
#endif
			pOut = rst;             
			delete []rst;
		}
		void UTF_8ToGB2312(string &pOut, char *pText, int pLen)
		{
			char * newBuf = new char[pLen+1];
			memset(newBuf, 0, sizeof(char) * (pLen + 1));

			char Ctemp[4];
			memset(Ctemp,0,4);
			int j = 0;	
#ifdef WIN32
			int i =0;

			while(i < pLen)
			{
				if( *(pText + i) >= 0 )
				{
					newBuf[j++] = pText[i++];                       
				}
				else                 
				{
					wchar_t Wtemp;
					UTF_8ToUnicode(&Wtemp,pText + i);

					UnicodeToGB2312(Ctemp,Wtemp);

					newBuf[j] = Ctemp[0];
					newBuf[j + 1] = Ctemp[1];

					i += 3;    
					j += 2;   
				}
			}
#endif  

#ifdef LINUX
			code_convert("utf-8","gbk",pText,pLen, newBuf, pLen);
			j = pLen;
#endif
			newBuf[j]='\0';

			pOut = newBuf;
			delete []newBuf;    
		}  

		void split( const string& str, vector<string>& strVec, const char sign /*= '|'*/ )
		{
			boost::split( strVec, str, boost::is_any_of( "|" ), boost::token_compress_on );
		}
	}  
}