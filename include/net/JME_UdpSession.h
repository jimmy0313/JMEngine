#ifndef JME_UdpSession_h__
#define JME_UdpSession_h__

#include "boost/asio.hpp"
#include "net/JME_NetHandler.h"
#include "net/JME_ReadBuffer.h"
#include "boost/enable_shared_from_this.hpp"

#include "net/JME_Message.h"
namespace JMEngine
{
	namespace net
	{
		class UdpSession :
			public boost::enable_shared_from_this<UdpSession>
		{
		public:
			typedef boost::shared_ptr<UdpSession> UdpSessionPtr;

		public:
			UdpSession(JMEngine::net::UdpNetHandler::UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size);
			~UdpSession();

			static JMEngine::net::UdpSession::UdpSessionPtr create(JMEngine::net::UdpNetHandler::UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size);
		
			void startRead();
			
			void writeMessage(const Message& msg);
		private:
			void doRead();
			void onRead(boost::system::error_code ec, std::size_t bytes_recvd);
		
			void doWrite();
			void onWrite(boost::system::error_code ec, std::size_t bytes_writed);
		public:
			boost::asio::ip::udp::socket _socket;
			boost::asio::ip::udp::endpoint _ep;
		
			UdpNetHandler::UdpNetHandlerPtr _handler;
			unsigned short _port;

			ReadBuffer _buffer;
		
			char* _writeBuffer;
			size_t _writeBufferSize;
			size_t _writeBufferOffest;
		
			bool _isReading;
			bool _isWriting;
		};
	}
}
#endif // JME_UdpSession_h__
