#ifndef JME_Random_h__
#define JME_Random_h__

/********************************************************************
	created:	2015/06/19
	author:		huangzhi
	
	purpose:	简单的随机数封装， 对随机种子进行简单处理
	warning:	
*********************************************************************/

#include <assert.h>
#include <time.h>
#include <random>
#include <algorithm>
#include <vector>
#include "JME_Singleton.h"

#define Rnd Random::getInstance

class JME_RandSeed
{
public:
	friend class JME_Random;
	JME_RandSeed(){}

	JME_RandSeed(int prob, int bindId):
		_prob(prob),
		_bindId(bindId),
		_begin(0),
		_end(0)
	{
	}
public:
	int _prob;	//概率
	int _bindId;	//绑定的相关ID
protected:
	int _begin;
	int _end;
};

class JME_Random
{
public:
	static JME_Random* getInstance()
	{
		static JME_Random* pInstance = new JME_Random;
		return pInstance;
	}

	int randomInt()
	{
		srand(_randomSeed);
		int ra = std::max(1, rand());
		_randomSeed += ra;
		return ra;
	}
	int randomInt(int start, int end)
	{
		assert(start <= end);

		int ra = randomInt() % (end - start + 1) + start;
		return ra;
	}

	bool randomGreater(int perc)
	{
		int ra = randomInt(1, 100);
		return ra >= perc;
	}

	int randomBySeeds(std::vector<JME_RandSeed>& seeds)
	{
		assert(!seeds.empty());

		for (size_t i = 0; i < seeds.size(); ++i)
		{
			if (0 == i)
				seeds[i]._begin = 1;
			else
				seeds[i]._begin = seeds[i - 1]._end;

			seeds[i]._end = seeds[i]._begin + seeds[i]._prob;
		}

		int rmax = seeds.rbegin()->_end;

		int ra = randomInt(1, rmax - 1);
		for (auto it = seeds.begin(); it != seeds.end(); ++it)
		{
			if (ra >= it->_begin && ra < it->_end)
				return it - seeds.begin();
		}
		return -1;
	}
private:
	JME_Random(void){ _randomSeed = time(NULL); };
	virtual ~JME_Random(void){};
	JME_Random(const JME_Random&){};
	JME_Random& operator = (const JME_Random&){};

private:
	time_t _randomSeed;
};
#endif // JME_Random_h__
