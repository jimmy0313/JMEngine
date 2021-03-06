#ifndef JME_Acceptor_h__
#define JME_Acceptor_h__

#include <string>

#include "boost/shared_ptr.hpp"
#include "boost/asio.hpp"

#include "net/JME_TcpSession.h"
#include "net/JME_NetHandler.h"

using namespace std;
namespace JMEngine
{
	namespace net
	{
		class TcpAcceptor
		{
		public:
			typedef boost::shared_ptr<TcpAcceptor> TcpAcceptorPtr;
			typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> AcceptorPtr;
		public:
			TcpAcceptor(JMEngine::net::NetHandler::NetHandlerPtr handler, unsigned short port);
			~TcpAcceptor();

			static JMEngine::net::TcpAcceptor::TcpAcceptorPtr create(JMEngine::net::NetHandler::NetHandlerPtr handler, unsigned short port);

			void accept(size_t reconnect, size_t bufferSize);
		private:
			void handlerAccept(JMEngine::net::TcpSession::TcpSessionPtr session, size_t reconnect, size_t bufferSize, const boost::system::error_code& error);
		public:
			NetHandler::NetHandlerPtr _acceptorHandler;
			unsigned short _port;

			AcceptorPtr _acceptor;
		};
	}
}
#endif // JME_Acceptor_h__
