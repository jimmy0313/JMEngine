#ifndef _NA_MSG_BASE_
#define _NA_MSG_BASE_
#include <memory>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <nedmalloc.h>
#include <string.h>


// message encrypt / decrypt key
#define HOYOGAME_ENCRYPT_KEY	"hoyo"

#define HEARTBEAT		1
#define SERVERBIND		2

namespace hoyosvr
{
	namespace msg
	{
		static size_t MAX_MSG_LENGTH =	819200;
enum{ msg_offset = 14};
#pragma pack(push,1)
		struct msg_base
		{
			msg_base(short type = 0):_total_len(14),_type(type),_net_id(0),_player_id(0){}
			int   _total_len;
			short _type;
			int	  _net_id;
			int	  _player_id;
		};
		struct hoyo_msg : public msg_base
		{
			typedef	boost::shared_ptr<hoyosvr::msg::hoyo_msg>	ptr;
			const char*			_msg_str_utf8;
			hoyo_msg()
			{
				_type = 0;
				_total_len = msg_offset;
			}
			hoyo_msg(short op_type)
			{
				_type = op_type;
				_total_len = msg_offset;
			}
			hoyo_msg(short op_type,std::string& msg_str_utf8)
			{
                _type = op_type;
				_total_len = msg_str_utf8.size() + msg_offset;
				_msg_str_utf8 = msg_str_utf8.data();
			}
			hoyo_msg(const char* data_ptr)
			{
				memcpy(this,data_ptr,msg_offset);
				size_t str_size = _total_len - msg_offset;
				char* msg_ptr = (char*)this + sizeof(hoyo_msg);			
				memcpy(msg_ptr,data_ptr+msg_offset,str_size);
				_msg_str_utf8 = msg_ptr;
			}
			static void	destory(hoyo_msg* p)
			{
				nedalloc::nedfree(p);
			}
			static ptr	create(const char* data_ptr,size_t len)
			{
				void* m = nedalloc::nedmalloc(len+sizeof(long));
				return ptr(new(m) hoyo_msg(data_ptr),destory);
			}
		};
#pragma pack(pop)
	}
}
#endif
