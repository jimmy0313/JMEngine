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
				LogE <<  e.what() << e.code() << LogEnd;
			}	
		}

		// read message from socket 
		// if message package is ready , begin to process it
		void JME_TcpSession::handleRead(boost::system::error_code error,size_t bytes_transferred)
		{
			if(_status == Disconnected)
				return;

			_reading = false;
			if ( !error )
			{	
				if (!_local)
				{
					try 
					{
						_readBlockTimes = 0;
						auto buff = (JME_ReadBuffer*)_readBuffer;
						std::size_t len = _socket.read_some(boost::asio::buffer(buff->getBuffer(),buff->getAvailableBufferSize()), error);

						if ( !error )
						{
							buff->readNewData(len);
							while(1)
							{
								char* data_ptr = 0;
								size_t l = 0;

								int c = buff->getMessage(&data_ptr,&l);
								if (JME_ReadBuffer::ReadBufferError == c)
								{
									_status = Disconnected;

									boost::system::error_code ec;

									JMECore.getLogicioService().post(
										boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), ec));

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

						LogE <<  e.message() << LogEnd;

						JMECore.getLogicioService().post(
							boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));
					}
				}
			}

			if  ( !error || error == boost::asio::error::would_block || error == boost::asio::error::try_again )			
			{
				if (!_local)
				{
					_readBlockTimes ++;
					if ( _readBlockTimes >= 10 )
					{
						JMECore.getLogicioService().post(
							boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), error));
						_status = Disconnected;

						return;
					}
				}
				try
				{
					boost::this_thread::sleep(boost::posix_time::microseconds(1));
					postReadNull();
				}
				catch (std::exception& e)
				{
					LogE << e.what() << LogEnd;
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

				auto buff = (JME_WriteBuffer*)_writeBuffer;
				if ( buff->_writeBufferOffest > 0 ) 
				{					
					std::size_t len = 0;
					try
					{
						do 
						{	
							std::size_t wlen = _socket.write_some(boost::asio::buffer(buff->_buffer + len, buff->_writeBufferOffest), error);
							if ( wlen <= 0 )
							{
								if ( !(error == boost::asio::error::would_block || error == boost::asio::error::try_again) )
								{

								}
								// copy unwritten data to top
								if ( len > 0  )
								{
									memmove(buff->_buffer,buff->_buffer + len, buff->_writeBufferOffest);
								}
								break;
							}
							buff->_writeBufferOffest -= wlen;
							len += wlen;
						}while ( buff->_writeBufferOffest > 0 );
					}
					catch (std::exception& e)
					{
						LogE <<  e.what() << LogEnd;
						LogE <<  "************ socket operation error in handle_write function \t ***********" << LogEnd;
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
					LogE << e.what() << LogEnd;
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
				LogE << "socket not connected, can't send data net_id : " << _net_id << LogEnd;
				return false;
			}

			boost::mutex::scoped_lock lock(_writeMutex);

			if (_local)
			{
				auto buff = (JME_SHM_Buffer*)_writeBuffer;
				buff->writeBuffer((const char*)&msg, MessageHeaderLength);			
				buff->writeBuffer(msg._msgData, msg.messageDataLen());
			}
			else
			{

				if (!checkWriteBuffer(msg))
				{
					try
					{
						postWriteNull();
					}
					catch (std::exception& e)
					{
						LogE << e.what() << LogEnd;
					}
					return false;
				}
				if ( _writeBuffer )
				{
					writeNolock((const char*)&msg, MessageHeaderLength);			
					writeNolock(msg._msgData, msg.messageDataLen());

					return true;
				}

				LogE << "Write data buffer is null!!!" << LogEnd;

				return false;
			}
		}

		bool JME_TcpSession::writeMessage( JME_Message::JME_MessagePtr msg )
		{
			return writeMessage(*msg);
		}

		void JME_TcpSession::writeLock(const char* data_ptr,int len)
		{
			if (!isOk())
			{
				LogE << "socket not connected, can't send data, net_id : " << _net_id << LogEnd;
				return;
			}

			if(!_socket.is_open())
			{
				LogE <<  "Writing socket has been closed net id:" << _net_id  << LogEnd;
				return;
			}

			boost::mutex::scoped_lock lock(_writeMutex);
			writeImpl(data_ptr,len);
		}

		void JME_TcpSession::writeNolock(const char* data_ptr,int len)
		{
			if (!isOk())
			{
				LogE << "socket not connected, can't send data, net_id : " << _net_id << LogEnd;
				return;
			}

			if(!_socket.is_open())
			{
				LogE <<  "Writing socket has been closed net id:" << _net_id  << LogEnd;
				return;
			}
			writeImpl(data_ptr,len);
		}

		void JME_TcpSession::writeImpl( const char* data_ptr,int len )
		{
			auto buff = (JME_WriteBuffer*)_writeBuffer;
			int destSize = buff->_writeBufferOffest + len;

			if ( destSize > buff->_writeBufferSize)
			{
				return;
			}
			else
			{
				memcpy(buff->_buffer + buff->_writeBufferOffest, data_ptr, len);
				buff->_writeBufferOffest += len;
			}

			try
			{
				postWriteNull();
			}
			catch (std::exception& e)
			{
				LogE << e.what() << LogEnd;
			}
		}

		JME_TcpSession::JME_TcpSession():
			_socket(JMECore.getNetIoService()),
			_readBuffer(NULL),
			_writeBuffer(NULL),
			_writing(false),
			_writeBlockTimes(0),
			_reading(false),
			_readBlockTimes(0),
			_reconnectInterval(0),
			_status(Disconnected),
			_gameStatus(NullGame),
			_asyncConnect(true)
		{
		}

		JME_TcpSession::JME_TcpSession( JME_NetHandler* net_handler, size_t n /*= MaxMsgLength*/, size_t reconnect/* = 5*/ ):
			_socket(JMECore.getNetIoService()),
			_readBuffer(NULL),
			_writeBuffer(NULL),
			_netHandlerPtr(net_handler),
			_writing(false),
			_writeBlockTimes(0),
			_reading(false),
			_readBlockTimes(0),
			_reconnectInterval(reconnect),
			_status(Disconnected),
			_gameStatus(NullGame),
			_asyncConnect(true)
		{
		}


		JME_TcpSession::~JME_TcpSession()
		{			
			boost::mutex::scoped_lock lock(_writeMutex);

			try
			{
				_socket.close();
			}
			catch(boost::system::system_error e)
			{
				LogE <<  e.what() << e.code() << LogEnd;
			}
			_writing = false;
			_reading = false;

			delete _writeBuffer;
			_writeBuffer = NULL;

			delete _readBuffer;
			_readBuffer = NULL;
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
					LogE <<  e.what() << " : " << e.code() << LogEnd;
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

			if (_local)
				_socket.get_io_service().post(boost::bind(&JME_TcpSession::localRead, shared_from_this()));

			_socket.async_read_some(
				boost::asio::null_buffers(),
				boost::bind(&JME_TcpSession::handleRead,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}

		void JME_TcpSession::postWriteNull()
		{
			if(!_socket.is_open() || _writing)
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

				try
				{
					auto local_ip = _socket.local_endpoint().address().to_string();
					auto port = _socket.local_endpoint().port();
					LogT << "Connect from local {" << local_ip << ":" << port << "} to remote {" << _ip << ":" << _port << "} succeed" << LogEnd;

					createWriteReadBuffer(true);

					JMECore.getLogicioService().post(
						boost::bind(&JME_NetHandler::sessionConnectSucceed, _netHandlerPtr, shared_from_this()));
				}
				catch(boost::system::system_error e)
				{
					LogE << e.what() << LogEnd;
				}
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
			auto buff = (JME_WriteBuffer*)_writeBuffer;

			int destSize = buff->_writeBufferOffest + msg._totalLen;

			if ( destSize > buff->_writeBufferSize)
			{
				return false;
			}
			return true;
		}

		void JME_TcpSession::resetReadBuffer()
		{

		}

		void JME_TcpSession::createWriteReadBuffer(bool client)
		{
			auto ip = client ? _socket.local_endpoint().address().to_string() : _socket.remote_endpoint().address().to_string();
			auto port = boost::lexical_cast<string>(client ? _socket.local_endpoint().port() : _socket.remote_endpoint().port());
			
			if (!ip.compare("127.0.0.1"))	//本机连接
				_local = true;

			if (_local)
			{
				auto wname = "ipm_write." + ip + "." + port;
				auto rname = "ipm_read." + ip + "." + port;

				if (client)
				{
					_readBuffer = JME_SHM_Buffer::create(rname, 1024);
					_writeBuffer = JME_SHM_Buffer::create(wname, 10240);
				}
				else
				{
					_readBuffer = JME_SHM_Buffer::create(wname, 1024);
					_writeBuffer = JME_SHM_Buffer::create(rname, 10240);
				}
			}
			else
			{
				_readBuffer = new JME_ReadBuffer(10240);
				_writeBuffer = new JME_WriteBuffer(10240);
			}
		}

		void JME_TcpSession::localRead()
		{
			_reading = false;
			while(1)
			{
				JME_Message::JME_MessagePtr msgPtr;
				auto buff = (JME_SHM_Buffer*)_readBuffer;
				int c = buff->getMessage(&msgPtr);
				if (JME_ReadBuffer::ReadBufferError == c)
				{
					_status = Disconnected;

					boost::system::error_code ec;

					JMECore.getLogicioService().post(
						boost::bind(&JME_NetHandler::sessionReadError, _netHandlerPtr, shared_from_this(), ec));

					return;
				}
				else if (JME_ReadBuffer::ReadBufferNoMessage == c)
				{
					//没有新的完整消息包
					break;
				}
				// 正常响应包处理

				JMECore.getLogicioService().post(
					boost::bind(&JME_NetHandler::sessionReceiveMessage, _netHandlerPtr, shared_from_this(), msgPtr));
			}
			try
			{
				boost::this_thread::sleep(boost::posix_time::microseconds(1));
				postReadNull();
			}
			catch (std::exception& e)
			{
				LogE << e.what() << LogEnd;
			}
		}

	}
}

