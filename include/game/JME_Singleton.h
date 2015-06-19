#ifndef JME_Singleton_h__
#define JME_Singleton_h__

/********************************************************************
	created:	2015/06/19
	author:		huangzhi
	
	purpose:	һ�������ĺ궨�壬 ����� THREAD_LOCAL_VARIABLE  ���������ֲ߳̾������� ���ڶ��̳߳�����ͬʱ�������໥����
	warning:	
*********************************************************************/


namespace JMEngine
{
	namespace game
	{
#ifdef THREAD_LOCAL_VARIABLE 

#define CreateSingletonClass(varName) \
private:	\
	varName(){}	\
	virtual ~varName(){} \
	varName(const varName&){} \
	varName& operator = (const varName&) {} \
public: \
	static varName*	getInstance() \
	{ \
		static boost::thread_specific_ptr<varName> _pInstance;	\
		if (NULL == _pInstance.get())	\
		{ \
			_pInstance.reset(new varName); \
		} \
		return _pInstance.get(); \
	} 
#else

#define CreateSingletonClass(varName) \
private:	\
	varName(){}	\
	virtual ~varName(){} \
	varName(const varName&){} \
	varName& operator = (const varName&) {} \
public: \
	static varName*	getInstance() \
	{ \
		static varName* _pInstance = new varName; \
		return _pInstance; \
	} 
#endif
	}
}
#endif // JME_Singleton_h__
