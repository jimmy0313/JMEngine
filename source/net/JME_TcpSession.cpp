#include <nedmalloc.h>
#include "boost/bind.hpp"

#include "JME_GLog.h"
#include "JME_TcpSession.h"
#include "JME_Core.h"
namespace JMEngine
{
	namespace net
	{
		void JME_TcpSession::start(int net_id)
		{
			try
			{
				_ip = _socket.remote_endpoint().address().to_string();
				_port = boost::lexical_cast<string>(_socket.remote_endpoint().port());
				_net_id = net_id;
				_status = Connected;
				_socket.non_blocking(true);
				postReadNull();
			}
			catch(boost::system::system_error e)
			{
				LOGE("Start session work failed, error ==> [ %d:%s ]", e.code(), e.what());
			}
		}

		// read message from socket 
		// if message package is ready , begin to process it
		void JME_TcpSession::handleRead(boost::system::error_code error,size_t bytes_transferred)
		{
			_reading = false;
			if ( !error )
			{				
				try 
				{
					_readBlockTimes = 0;
					std::size_t len = _socket.read_some(boost::asio::buffer(_buff.getBuffer(),_buff.getAvailableBufferSize()), error);

					if ( !error )
					{
						_buff.readNewData(len);
						while(1)
						{
							char* data_ptr = 0;
							size_t l = 0;

							int c = _buff.getMessage(&data_ptr,&l);
							if (JME_ReadBuffer::ReadBufferError == c)
							{
								_status = Disconnected;

								boost::system::error_code ec;

								JMECore.getLogicioService().post(
									boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));

								return;
							}
							else if (JME_ReadBuffer::ReadBufferNoMessage == c)
							{
								//没有新的完整消息包
								break;
							}
							// 正常响应包处理

							JMECore.getLogicioService().post(
								boost::bind(&JME_NetHandler::sessionReceiveMessage, _netHandlerPtr, shared_from_this(), JME_Message::create(data_ptr, l)));
						}
					}
				}
				catch (boost::system::error_code& e)
				{
					_status = Disconnected;

					LOGE("Try to read error ==> [ %d:%s ]", e.value(), e.message());

					JMECore.getLogicioService().post(
						boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));
				}
			}

			if  ( !error || error == boost::asio::error::would_block || error == boost::asio::error::try_again )			
			{
				_readBlockTimes ++;
				if ( _readBlockTimes >= 10 )
				{
					JMECore.getLogicioService().post(
						boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));
					_status = Disconnected;

					return;
				}
				try
				{
					boost::this_thread::sleep(boost::posix_time::microseconds(1));
					postReadNull();
				}
				catch (std::exception& e)
				{
					LOGE("Try to sleep error ==> [ %s ]", e.what());
				}
			}
			else
			{
// 				if ( error != boost::asio::error::eof && 
// 					error != boost::asio::error::bad_descriptor && )
// 				{
// 
// 				}
				if (error.value() == 995)
					return;

				_status = Disconnected;

				JMECore.getLogicioService().post(
					boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));
			}
		}

		void JME_TcpSession::handleWrite(boost::system::error_code error)
		{
			_writing = false;
			if (!error)
			{	
				boost::mutex::scoped_lock lock(_writeMutex);		
				_writeBlockTimes = 0;
				if ( _writeBufferOffest > 0 ) 
				{					
					std::size_t len = 0;
					try
					{
						do 
						{	
							std::size_t wlen = _socket.write_some(boost::asio::buffer(_writeBuffer + len,_writeBufferOffest), error);
							if ( wlen <= 0 )
							{
								if ( !(error == boost::asio::error::would_block || error == boost::asio::error::try_again) )
								{

								}
								// copy unwritten data to top
								if ( len > 0  )
								{
									memmove(_writeBuffer,_writeBuffer+len,_writeBufferOffest);
								}
								break;
							}
							_writeBufferOffest -= wlen;
							len += wlen;
						}while ( _writeBufferOffest > 0 );
					}
					catch (std::exception& e)
					{
						LOGE("Try to write error ==> [ %s ]", e.what());
					}
				}
			}


			if (!error || error == boost::asio::error::would_block || error == boost::asio::error::try_again)
			{
				_writeBlockTimes ++;

				if ( _writeBlockTimes >= 10 )
				{
					return;
				}
				try
				{
					postWriteNull();
				}
				catch (std::exception& e)
				{
					LOGE("Try to write error ==> [ %s ]", e.what());
				}
			}
			else
			{
			}
		}

		bool JME_TcpSession::writeMessage( const JME_Message& msg )
		{
			if (!isOk())
			{
				return false;
			}

			boost::mutex::scoped_lock lock(_writeMutex);

			if (!checkWriteBuffer(msg))
			{
				try
				{
					postWriteNull();
				}
				catch (std::exception& e)
				{
					LOGE("Try to write error ==> [ %s ]", e.what());
				}
				return false;
			}
			if ( _writeBuffer )
			{
				writeNolock((const char*)&msg, MessageHeaderLength);			
				writeNolock(msg._msgData, msg.messageDataLen());

				return true;
			}

			LOGE("Write buffer is null");

			return false;
		}

		bool JME_TcpSession::writeMessage( JME_Message::JME_MessagePtr msg )
		{
			return writeMessage(*msg);
		}

		void JME_TcpSession::writeLock(const char* data_ptr,int len)
		{
			if (!isOk())
				return;

			if(!_socket.is_open())
				return;

			boost::mutex::scoped_lock lock(_writeMutex);
			writeImpl(data_ptr,len);
		}

		void JME_TcpSession::writeNolock(const char* data_ptr,int len)
		{
			if (!isOk())
				return;

			if(!_socket.is_open())
				return;

			writeImpl(data_ptr,len);
		}

		void JME_TcpSession::writeImpl( const char* data_ptr,int len )
		{
			int destSize = _writeBufferOffest + len;

			if ( destSize > _writeBufferSize)
			{
				return;
			}
			else
			{
				memcpy(_writeBuffer + _writeBufferOffest,data_ptr,len);
				_writeBufferOffest += len;
			}

			try
			{
				postWriteNull();
			}
			catch (std::exception& e)
			{
				LOGE("Try to write error ==> [ %s ]", e.what());
			}
		}

		JME_TcpSession::JME_TcpSession():
			_socket(JMECore.getNetIoService()),
			_buff(0),
			_writeBufferSize(0),
			_writing(false),
			_writeBlockTimes(0),
			_reading(false),
			_readBlockTimes(0),
			_reconnectInterval(0),
			_status(Disconnected),
			_gameStatus(NullGame),
			_asyncConnect(true)
		{
			_writeBuffer = NULL;
			_writeBufferOffest = 0;

		}

		JME_TcpSession::JME_TcpSession( JME_NetHandler* net_handler, size_t n /*= MaxMsgLength*/, size_t reconnect/* = 5*/ ):
			_socket(JMECore.getNetIoService()),
			_buff(n),
			_netHandlerPtr(net_handler),
			_writeBufferSize(n),
			_writing(false),
			_writeBlockTimes(0),
			_reading(false),
			_readBlockTimes(0),
			_reconnectInterval(reconnect),
			_status(Disconnected),
			_gameStatus(NullGame),
			_asyncConnect(true)
		{
			_writeBuffer = (char*)nedalloc::nedmalloc(n);
			_writeBufferOffest = 0;
		}


		JME_TcpSession::~JME_TcpSession()
		{			
			boost::mutex::scoped_lock lock(_writeMutex);
			nedalloc::nedfree(_writeBuffer);
			try
			{
				_socket.close();
			}
			catch(boost::system::system_error e)
			{
				LOGE("Try to clost socket error ==> [ %d:%s ]", e.code().value(), e.what());
			}
			_writeBuffer = 0;
			_writeBufferOffest = 0;
			_writing = false;
			_reading = false;
		}

		void JME_TcpSession::stop()
		{
			try
			{
				_reconnectInterval = 0;
				if ( _socket.is_open() )
				{				
					_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					_socket.close();
				}
			}
			catch(boost::system::system_error e)
			{
				// 107 means : shutdown: Transport endpoint is not connected system:107
				// so we don't need to record this kind of error message
				if ( e.code().value() != 107 )
				{
					LOGE("Try to clost socket error ==> [ %d:%s ]", e.code().value(), e.what());
				}
			}

			_writing = false;
			_reading = false;
		}

		JME_TcpSession::JME_TcpSessionPtr JME_TcpSession::create( JME_NetHandler::JME_NetHandlerPtr net_handler, size_t n_buff_size, size_t reconnect/* = 0*/ )
		{
			void* m = nedalloc::nedmalloc(sizeof(JME_TcpSession));
			return JME_TcpSessionPtr(new(m) JME_TcpSession(net_handler.get(), n_buff_size, reconnect),destory);
		}

		JMEngine::net::JME_TcpSessionPtr JME_TcpSession::create( JME_NetHandler* net_handler, size_t n_buff_size, size_t reconnect /*= 0*/ )
		{
			void* m = nedalloc::nedmalloc(sizeof(JME_TcpSession));
			return JME_TcpSessionPtr(new(m) JME_TcpSession(net_handler, n_buff_size, reconnect),destory);
		}

		void JME_TcpSession::destory(JME_TcpSession* p)
		{
			p->~JME_TcpSession();
			nedalloc::nedfree(p);
		}

		void JME_TcpSession::postReadNull()
		{
			if(!_socket.is_open() || _reading)
				return;

			_reading = true;
			_socket.async_read_some(
				boost::asio::null_buffers(),
				boost::bind(&JME_TcpSession::handleRead,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}

		void JME_TcpSession::postWriteNull()
		{
			if(!_socket.is_open() || _writing || _writeBufferOffest == 0)
				return;

			_writing = true;
			_socket.async_write_some(
				boost::asio::null_buffers(),
				boost::bind(&JME_TcpSession::handleWrite,
				shared_from_this(),
				boost::asio::placeholders::error));
		}

		void JME_TcpSession::connect( const string& ip, const string& port )
		{
			_ip = ip;
			_port = port;

			onConnect();
		}

		void JME_TcpSession::handleConnect( const boost::system::error_code& e )
		{
			if(e)
			{
				_status = Disconnected;

				JMECore.getLogicioService().post(
					boost::bind(&JME_NetHandler::sessionConnectFailed, _netHandlerPtr, shared_from_this(), e));

				if (_reconnectInterval)
				{
					boost::shared_ptr<deadline_timer> t(new boost::asio::deadline_timer(_socket.get_io_service()));
					t->expires_from_now(boost::posix_time::seconds(_reconnectInterval));
					t->async_wait(boost::bind(&JME_TcpSession::onReconnect, shared_from_this(), t));
				}
			}
			else
			{
				_status = Connected;

				JMECore.getLogicioService().post(
					boost::bind(&JME_NetHandler::sessionConnectSucceed, _netHandlerPtr, shared_from_this()));
			}
		}

		void JME_TcpSession::reconnect()
		{
			onConnect();
		}

		void JME_TcpSession::onConnect()
		{
			if (Disconnected != _status)
				return;

			_status = Connecting;

			if (_asyncConnect)
			{
				tcp::resolver resolver(_socket.get_io_service());
				tcp::resolver::query query(_ip, _port);
				tcp::resolver::iterator iterator = resolver.resolve(query);	

				boost::asio::async_connect(_socket, iterator,
					boost::bind(&JME_TcpSession::handleConnect, shared_from_this(), boost::asio::placeholders::error));
			}
			else
			{
				_socket.close();

				boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string(_ip), boost::lexical_cast<int>(_port));
				boost::system::error_code ec;
				_socket.connect(ep, ec);

				handleConnect(ec);
			}
		}

		void JME_TcpSession::onReconnect( boost::shared_ptr<boost::asio::deadline_timer> t )
		{
			onConnect();
		}

		bool JME_TcpSession::isOk()
		{
			return _status == Connected;
		}

		boost::asio::ip::tcp::socket& JME_TcpSession::socket()
		{
			return _socket;
		}

		bool JME_TcpSession::checkWriteBuffer( const JME_Message& msg )
		{
			int destSize = _writeBufferOffest + msg._totalLen;

			if ( destSize > _writeBufferSize)
			{
				return false;
			}
			return true;
		}

		void JME_TcpSession::resetReadBuffer()
		{
			_buff.reset();
		}

	}
}

