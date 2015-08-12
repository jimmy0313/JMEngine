#ifndef JME_ReadBuffer_h__
#define JME_ReadBuffer_h__

#include <vector>
#include <string.h>
#include <nedmalloc.h>
#include "JME_Message.h"
#include "JME_GLog.h"
#include "boost/interprocess/ipc/message_queue.hpp"
#include "boost/interprocess/sync/named_mutex.hpp"
#include "boost/interprocess/sync/scoped_lock.hpp"
namespace JMEngine
{
	namespace net
	{
		class JME_BufferInterface
		{
		public:
			enum
			{
				ReadBufferError = -1,
				ReadBufferNoMessage = 0,
			};
		};

		class JME_ReadBuffer : 
			public JME_BufferInterface
		{
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

			int getMessage(JME_Message::JME_MessagePtr msgPtr)
			{
				onCheckBuffer();
				int len_ = nextMessageLength();

				if (len_ >= MessageHeaderLength && _buffOffest - _handleOffest >= len_)
				{
					msgPtr = JME_Message::create(_handleOffest, len_);
					_handleOffest += len_;
				}
				return len_;
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
				LogW << "Buffer will be empty" << LogEnd;
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

					LogT << "move buffer size " << blen << LogEnd; 
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

		class JME_WriteBuffer : 
			public JME_BufferInterface
		{
		public:
			JME_WriteBuffer(size_t n)
			{
				_buffer = (char*)nedalloc::nedmalloc(n);
				_writeBufferOffest = 0;
				_writeBufferSize = n;
			}
			~JME_WriteBuffer(void)
			{
				nedalloc::nedfree(_buffer);
			}

			char* getBuffer()
			{
				return _buffer;
			}

		public:
			char* _buffer;	//缓冲区起始地址
			size_t _writeBufferSize;
			size_t _writeBufferOffest;
		};

		using namespace boost::interprocess;
		class JME_SHM_Buffer : 
			public JME_BufferInterface
		{
		public:
			JME_SHM_Buffer(const string& name, size_t n):
				_buffSize(n),
				_buffOffest(0),
				_handleOffest(0)
			{
				memcpy(_name, name.c_str(), name.length());
				_name[name.length()] = '\0';
				try
				{
					auto buffer = shared_memory_object(create_only, _name, read_write);

					buffer.truncate(sizeof(JME_SHM_Buffer) + _buffSize);
					mapped_region mmap(buffer, read_write);

					memcpy(mmap.get_address(), this, sizeof(JME_SHM_Buffer));
					memset((char*)mmap.get_address() + sizeof(JME_SHM_Buffer), 0, _buffSize);
				}
				catch(interprocess_exception& e)
				{
					LogW << e.what() << LogEnd;
				}
			}

			~JME_SHM_Buffer()
			{
 				shared_memory_object::remove(_name);
			}

			static JME_SHM_Buffer* create(const string& name, size_t n)
			{
				return new JME_SHM_Buffer(name, n);
			}

			void writeBuffer(const char* data, size_t len)
			{
				auto startTime = boost::get_system_time();

				try
				{
					auto buffer = shared_memory_object(open_only, _name, read_write);
					mapped_region mmap(buffer, read_write);

					auto shm_buff = (JME_SHM_Buffer*)mmap.get_address();

					onCheckBuffer(shm_buff);

					if (shm_buff->_buffSize - shm_buff->_buffOffest < len)
						buffer.truncate(mmap.get_size() * 2);

					memcpy((char*)mmap.get_address() + sizeof(JME_SHM_Buffer) + shm_buff->_buffOffest, data, len);
					shm_buff->_buffOffest += len;
				}
				catch(interprocess_exception& e)
				{
					LogW << e.what() << LogEnd;
				}
				auto endTime = boost::get_system_time();
				int elapse = (int)(endTime - startTime).total_milliseconds();
				LogT << "consume " << elapse << " ms" << LogEnd;
			}

			int getMessage(JME_Message::JME_MessagePtr* msgPtr)
			{
				auto startTime = boost::get_system_time();

				try
				{
					auto buffer = shared_memory_object(open_only, _name, read_write);
					mapped_region mmap(buffer, read_write);
	
					auto shm_buff = (JME_SHM_Buffer*)mmap.get_address();
	
					int len_ = nextMessageLength(shm_buff);
	
					if (len_ >= MessageHeaderLength)
					{
						(*msgPtr).swap(JME_Message::create((char*)shm_buff + sizeof(JME_SHM_Buffer) + shm_buff->_handleOffest, len_));
	
						shm_buff->_handleOffest  += len_;
					}
					auto endTime = boost::get_system_time();
					int elapse = (int)(endTime - startTime).total_milliseconds();
					LogT << "consume " << elapse << " ms" << LogEnd;

					return len_;
				}
				catch(interprocess_exception& e)
				{
					LogW << e.what() << LogEnd;
				}

			}

			int nextMessageLength(JME_SHM_Buffer* shm_buff)
			{				
				if (shm_buff->_buffOffest - shm_buff->_handleOffest >= MessageHeaderLength)
				{

					auto header = (MessageHeader*)((char*)shm_buff + sizeof(JME_SHM_Buffer) + shm_buff->_handleOffest);
					if (header->_totalLen < MessageHeaderLength)
						return ReadBufferError;	//-1 表示读缓冲区出错， 无法正确解析消息包

					if (shm_buff->_buffOffest - shm_buff->_handleOffest >= header->_totalLen)
						return header->_totalLen;
				}
				return ReadBufferNoMessage;
			}

			void onCheckBuffer(JME_SHM_Buffer* shm_buff)
			{
				if (shm_buff->_handleOffest > shm_buff->_buffSize / 2 + 1)	//缓冲区前一半均为已处理缓存， 这时候可以使用memcpy将剩余缓冲区数据移动到前面， 避免使用memmove
				{
					int blen = shm_buff->_buffOffest - shm_buff->_handleOffest;
					
					memcpy((char*)shm_buff + sizeof(JME_SHM_Buffer), ((char*)shm_buff + sizeof(JME_SHM_Buffer) + shm_buff->_handleOffest), blen);
					shm_buff->_handleOffest = 0;
					shm_buff->_buffOffest = blen;
					memset((char*)shm_buff + sizeof(JME_SHM_Buffer) + shm_buff->_buffOffest, 0, shm_buff->_buffSize - blen);

					LogT << "move buffer size " << blen << LogEnd; 
				}
			}
		private:
			char _name[30];
			interprocess_mutex _mutex;
			size_t _buffSize;	//缓冲区大小

			size_t _buffOffest;	//缓冲区当前偏移, 由写端进行修改

			size_t _handleOffest;//处理偏移，当前处理的位置, 由读端修改
		};
	}
}



#endif // JME_ReadBuffer_h__
