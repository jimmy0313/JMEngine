#ifndef JME_TcpSession_h__
#define JME_TcpSession_h__

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "net/JME_Message.h"
#include "net/JME_ReadBuffer.h"
#include "net/JME_NetHandler.h"

using boost::asio::ip::tcp;

const static int RPCSession = 1;
namespace JMEngine
{
	namespace net
	{
		class TcpSession :
			public boost::enable_shared_from_this<TcpSession>
		{
			enum SessionStatus
			{
				Disconnected = 1,
				Connected = 2,
				Connecting = 3,
			};
		public:
			enum GameStatus
			{
				NullGame = 0,
				InGame = 1,
			};
		public:
			typedef boost::shared_ptr<TcpSession> TcpSessionPtr;
			typedef boost::weak_ptr<TcpSession> TcpSessionWeakPtr;

			TcpSession();
			TcpSession(NetHandler* net_handler, size_t n = MaxMsgLength, size_t reconnect = 5); // default buffer size
			~TcpSession();

			static TcpSessionPtr create(NetHandler::NetHandlerPtr net_handler, size_t n_buff_size, size_t reconnect = 0);
			static TcpSessionPtr create(NetHandler* net_handler, size_t n_buff_size, size_t reconnect = 0);

			static void	destory(TcpSession* p);	

			inline int getNetId() { return _net_id; }
			inline void setNetId(int id) { _net_id = id; }

			inline const string& getIp() { return _ip; }
			inline const string& getPort() { return _port; }

			inline void setGameStatus(GameStatus status) { _gameStatus = status; }
			inline GameStatus getGameStatus() { return _gameStatus; }

			inline void setAsyncConnect(bool b) { _asyncConnect = b; } 

			void connect(const string& ip, const string& port);
			void reconnect();

			void resetReadBuffer();

			boost::asio::ip::tcp::socket& socket();

			bool isOk();

			// start current tcp session
			void start(int net_id);

			// stop tcp session
			void stop();

			// write message 
			void writeLock(const char* dataPtr, const int len);
			bool writeMessage(const Message& msg);
			bool writeMessage(Message::MessagePtr msg);

		private:
			void onConnect();
			void onReconnect(boost::shared_ptr<boost::asio::deadline_timer> t);

			bool checkWriteBuffer(const Message& msg);

			void writeNolock(const char* dataPtr, const int len);
			// internal read or write message
			void writeImpl(const char* data_ptr,int len);

			void handleWrite(boost::system::error_code error);
			void handleRead(boost::system::error_code error,size_t bytes_transferred);	
			void handleConnect(const boost::system::error_code& e);

			void postReadNull();
			void postWriteNull();
			
			tcp::socket _socket;
			bool _asyncConnect;	//是否需要异步连接， 用于rpc client时， 一般设为false

			NetHandler* _netHandlerPtr;
			
			boost::mutex _writeMutex;
			boost::mutex _readMutex;
			char* _writeBuffer;
			size_t _writeBufferSize;
			size_t _writeBufferOffest;
			ReadBuffer _buff;

			int _net_id;
				
			bool _writing;
			int _writeBlockTimes;

			bool _reading;
			int _readBlockTimes;

			size_t _reconnectInterval;	//自动重连间隔, 0表示不自动重连
			string _ip;
			string _port;

			SessionStatus _status;	//连接中
			GameStatus _gameStatus;
		};
	}
}

#endif // JME_TcpSession_h__
