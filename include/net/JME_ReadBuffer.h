#ifndef JME_ReadBuffer_h__
#define JME_ReadBuffer_h__

#include <vector>
#include <string.h>
#include <nedmalloc.h>
#include "JME_Message.h"
#include "JME_GLog.h"

namespace JMEngine
{
	namespace net
	{
		class JME_ReadBuffer
		{
		public:
			enum
			{
				ReadBufferError = -1,
				ReadBufferNoMessage = 0,
			};
		public:
			JME_ReadBuffer(size_t n)
			{
				_buffer = (char*)nedalloc::nedmalloc(n);
				_buffSize = n;
				_buffOffest = _buffer;
				_handleOffest = _buffer;
			}
			~JME_ReadBuffer(void)
			{
				nedalloc::nedfree(_buffer);
			}

			char* getBuffer()
			{
				return _buffOffest;
			}

			void reset()
			{
				_buffOffest = _buffer;
				_handleOffest = _buffer;
			}
			int getMessage(char** buff, size_t* len)
			{
				onCheckBuffer();
				int len_ = nextMessageLength();
				
				if (len_ >= MessageHeaderLength && _buffOffest - _handleOffest >= len_)
				{
					*buff = _handleOffest;
					*len = len_;

					_handleOffest += len_;
				}
				return len_;
			}

			void readNewData(size_t len)
			{
				_buffOffest += len;
				onCheckBufferWaining();
			}
			
			size_t getAvailableBufferSize()
			{
				return _buffSize - (_buffOffest - _buffer);
			}

		private:
			void onCheckBufferWaining()
			{
#define BUFFER_WAINING
#ifdef BUFFER_WAINING
				if (_buffOffest - _buffer < _buffSize / 10 * 9)
					return;
				LOGE("Buffer will be empty");
#endif // BUFFER_WAINING
			}

			void onCheckBuffer()
			{
				if (_handleOffest - _buffer > _buffSize / 2 + 1)	//缓冲区前一半均为已处理缓存， 这时候可以使用memcpy将剩余缓冲区数据移动到前面， 避免使用memmove
				{
					int blen = _buffOffest - _handleOffest;

					memcpy(_buffer, _handleOffest, blen);
					_handleOffest = _buffer;
					_buffOffest = _buffer + blen;
					memset(_buffOffest, 0, _buffSize - blen);

					LOGT("move buffer size %d", blen);
				}
			}

			int nextMessageLength()
			{
				if (_buffOffest - _handleOffest >= MessageHeaderLength)
				{
					int len = *(int*)_handleOffest;
					if (len < MessageHeaderLength)
						return ReadBufferError;	//-1 表示读缓冲区出错， 无法正确解析消息包

					if (_buffOffest - _handleOffest >= len)
						return len;
				}
				return ReadBufferNoMessage;
			}
		private:
			char* _buffer;	//缓冲区起始地址
			size_t _buffSize;	//缓冲区大小

			char* _buffOffest;//缓冲区当前偏移

			char* _handleOffest;//处理偏移，当前处理的位置
		};
	}
}



#endif // JME_ReadBuffer_h__
