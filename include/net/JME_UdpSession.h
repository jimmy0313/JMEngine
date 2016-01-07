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
		class JME_UdpSession :
			public boost::enable_shared_from_this<JME_UdpSession>
		{
		public:
			typedef boost::shared_ptr<JME_UdpSession> JME_UdpSessionPtr;

		public:
			JME_UdpSession(JMEngine::net::JME_UdpNetHandler::JME_UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size);
			~JME_UdpSession();

			static JMEngine::net::JME_UdpSession::JME_UdpSessionPtr create(JMEngine::net::JME_UdpNetHandler::JME_UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size);
		
			void startRead();
			
			void writeMessage(const JME_Message& msg);
		private:
			void doRead();
			void onRead(boost::system::error_code ec, std::size_t bytes_recvd);
		
			void doWrite();
			void onWrite(boost::system::error_code ec, std::size_t bytes_writed);
		public:
			boost::asio::ip::udp::socket _socket;
			boost::asio::ip::udp::endpoint _ep;
		
			JME_UdpNetHandler::JME_UdpNetHandlerPtr _handler;
			unsigned short _port;

			JME_ReadBuffer _buffer;
		
			char* _writeBuffer;
			size_t _writeBufferSize;
			size_t _writeBufferOffest;
		
			bool _isReading;
			bool _isWriting;
		};
	}
}
#endif // JME_UdpSession_h__
