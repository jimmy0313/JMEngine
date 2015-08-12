#ifndef JME_GLogColor_h__
#define JME_GLogColor_h__

namespace JMEngine
{
	namespace log
	{
		enum GLogColor
		{
			//Black=0,
#ifdef WIN32
			Pink=13,
			Green=10,
			Red=12,
			Yellow=14,
			White=7
#else
			Pink=35,
			Green=32,
			Red=31,
			Yellow=33,
			White=37
#endif
		};
	}
}
#endif // JME_GLogColor_h__