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

#include "boost/random.hpp"

#define Rnd Random::getInstance


class JME_RandSeed
{
public:
	friend class JME_Random;
	JME_RandSeed(){}

	JME_RandSeed(int prob, void* bind):
		_prob(prob),
		_bind(bind),
		_begin(0),
		_end(0)
	{
	}
public:
	int _prob;	//概率
	void* _bind;	//绑定的对象
protected:
	int _begin;
	int _end;
};

class JME_Random
{
public:
public:
	JME_Random(void){ _randomSeed = time(NULL); };
	virtual ~JME_Random(void){};

	int randomInt()
	{
		boost::mt19937 gen(time(NULL) + _randomSeed);
		boost::uniform_int<> dist(0, INT_MAX);

		boost::variate_generator<boost::mt19937, boost::uniform_int<> > die(gen, dist);

		return _randomSeed = die();
	}
	int randomInt(int start, int end)
	{
		assert(start <= end);

		boost::mt19937 gen(time(NULL) + _randomSeed);
		boost::uniform_int<> dist(start, end);

		boost::variate_generator<boost::mt19937, boost::uniform_int<> > die(gen, dist);

		return _randomSeed = die();
	}

	bool randomGreater(int perc)
	{
		int ra = randomInt(1, 100);
		return ra >= perc;
	}

	void* randomBySeeds(std::vector<JME_RandSeed>& seeds)
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
				return it->_bind;
		}
		return nullptr;
	}
private:
	time_t _randomSeed;
};
#endif // JME_Random_h__
