#include "net/JME_UdpSession.h"
#include "log/JME_GLog.h"
#include "net/JME_Core.h"
namespace JMEngine
{
	namespace net
	{

		UdpSession::UdpSession(JMEngine::net::UdpNetHandler::UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size):
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

		UdpSession::~UdpSession()
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

		JMEngine::net::UdpSession::JME_UdpSessionPtr UdpSession::create(JMEngine::net::UdpNetHandler::UdpNetHandlerPtr handler, unsigned short port, unsigned int buffer_size)
		{
			return JMEngine::net::UdpSession::JME_UdpSessionPtr(new UdpSession(handler, port, buffer_size));
		}

		void UdpSession::doRead()
		{
			if (_isReading)
				return;
		
			boost::system::error_code ec;
			_socket.async_receive_from(boost::asio::buffer(_buffer.getBuffer(),_buffer.getAvailableBufferSize()), _ep, boost::bind(&UdpSession::onRead, this, _1, _2));
		}

		void UdpSession::startRead()
		{
			JMECore.getNetIoService().post(boost::bind(&UdpSession::doRead, this));
		}

		void UdpSession::onRead(boost::system::error_code ec, std::size_t bytes_recvd)
		{
			_isReading = false;
			if (ec || bytes_recvd <= 0)
				return doRead();

			_buffer.readNewData(bytes_recvd);
			char* data_ptr = 0;
			size_t l = 0;

			int ret = _buffer.getMessage(&data_ptr,&l);
			if (ReadBuffer::ReadBufferError == ret || ReadBuffer::ReadBufferNoMessage == ret)
			{
				_buffer.reset();
				return doRead();
			}
			// 正常响应包处理
			JMECore.getLogicioService().post(
				boost::bind(&UdpNetHandler::onReceive, _handler, shared_from_this(), Message::create(data_ptr, l)));
		}

		void UdpSession::writeMessage(const Message& msg)
		{
			if (_isWriting)
				return;
		
			memcpy(_writeBuffer + _writeBufferOffest, (char*)&msg, MessageHeaderLength);
			_writeBufferOffest += MessageHeaderLength;
			memcpy(_writeBuffer + _writeBufferOffest, msg._msgData, msg.messageDataLen());
			_writeBufferOffest += msg.messageDataLen();
		
			doWrite();
		}

		void UdpSession::doWrite()
		{
			_socket.async_send_to(boost::asio::buffer(_writeBuffer, _writeBufferOffest), _ep, 
				boost::bind(&UdpSession::onWrite, this, _1, _2));
		}

		void UdpSession::onWrite(boost::system::error_code ec, std::size_t bytes_writed)
		{
			_isWriting = false;
			_writeBufferOffest = 0;
		
			JMECore.getLogicioService().post(
				boost::bind(&UdpNetHandler::onWrite, _handler, shared_from_this()));
		}

	}
}