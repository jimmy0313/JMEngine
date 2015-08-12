#ifndef JME_GLogLevel_h__
#define JME_GLogLevel_h__

#include <string>

namespace JMEngine
{
	namespace log
	{
		enum GLogLevel
		{
			GLog_ALL = 1,//��͵ȼ��ģ���������־��¼
			GLog_TRACE = 2,//����
			GLog_DEBUG =3 ,//����
			GLog_INFO = 4,//��Ϣ
			GLog_WARN = 5,//����
			GLog_ERROR = 6,//����
			GLog_FATAL = 7,//��������
			GLog_OFF = 8//����ߵȼ��ģ��ر�������־��¼
		};
	}
}
#endif // JME_GLogLevel_h__