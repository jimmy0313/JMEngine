#include "JME_UdpSession.h"
#include "JME_GLog.h"
#include "JME_Core.h"
namespace JMEngine
{
	namespace net
	{

		JME_UdpSession::JME_UdpSession(JMEngine::net::JME_UdpNetHandler::JME_UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size):
			_socket(JMECore.getNetIoService(), boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),
			_handler(handler),
			_port(port),
			_buffer(buffer_size),
			_isReading(false),
			_isWriting(false)
		{
			_writeBuffer = (char*)nedalloc::nedmalloc(buffer_size);
			_writeBufferOffest = 0;
			_writeBufferSize = buffer_size;
		}

		JME_UdpSession::~JME_UdpSession()
		{
			nedalloc::nedfree(_writeBuffer);
			try 
			{
				_socket.close();
			}
			catch (std::exception& e)
			{
				
			}
		}

		JMEngine::net::JME_UdpSession::JME_UdpSessionPtr JME_UdpSession::create(JMEngine::net::JME_UdpNetHandler::JME_UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size)
		{
			return JMEngine::net::JME_UdpSession::JME_UdpSessionPtr(new JME_UdpSession(handler, port, buffer_size));
		}

		void JME_UdpSession::doRead()
		{
			if (_isReading)
				return;
		
			boost::system::error_code ec;
			_socket.async_receive_from(boost::asio::buffer(_buffer.getBuffer(),_buffer.getAvailableBufferSize()), _ep, boost::bind(&JME_UdpSession::onRead, this, _1, _2));
		}

		void JME_UdpSession::startRead()
		{
			JMECore.getNetIoService().post(boost::bind(&JME_UdpSession::doRead, this));
		}

		void JME_UdpSession::onRead(boost::system::error_code ec, std::size_t bytes_recvd)
		{
			_isReading = false;
			if (ec || bytes_recvd <= 0)
				return doRead();

			_buffer.readNewData(bytes_recvd);
			char* data_ptr = 0;
			size_t l = 0;

			int ret = _buffer.getMessage(&data_ptr,&l);
			if (JME_ReadBuffer::ReadBufferError == ret || JME_ReadBuffer::ReadBufferNoMessage == ret)
			{
				_buffer.reset();
				return doRead();
			}
			// 正常响应包处理
			JMECore.getLogicioService().post(
				boost::bind(&JME_UdpNetHandler::onReceive, _handler, shared_from_this(), JME_Message::create(data_ptr, l)));
		}

		void JME_UdpSession::writeMessage(const JME_Message& msg)
		{
			if (_isWriting)
				return;
		
			memcpy(_writeBuffer + _writeBufferOffest, (char*)&msg, MessageHeaderLength);
			_writeBufferOffest += MessageHeaderLength;
			memcpy(_writeBuffer + _writeBufferOffest, msg._msgData, msg.messageDataLen());
			_writeBufferOffest += msg.messageDataLen();
		
			doWrite();
		}

		void JME_UdpSession::doWrite()
		{
			_socket.async_send_to(boost::asio::buffer(_writeBuffer, _writeBufferOffest), _ep, 
				boost::bind(&JME_UdpSession::onWrite, this, _1, _2));
		}

		void JME_UdpSession::onWrite(boost::system::error_code ec, std::size_t bytes_writed)
		{
			_isWriting = false;
			_writeBufferOffest = 0;
		
			JMECore.getLogicioService().post(
				boost::bind(&JME_UdpNetHandler::onWrite, _handler, shared_from_this()));
		}

	}
}