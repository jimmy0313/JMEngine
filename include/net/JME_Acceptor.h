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
		class JME_TcpAcceptor
		{
		public:
			typedef boost::shared_ptr<JME_TcpAcceptor> JME_TcpAcceptorPtr;
			typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> AcceptorPtr;
		public:
			JME_TcpAcceptor(JMEngine::net::JME_NetHandler::JME_NetHandlerPtr handler, unsigned short port);
			~JME_TcpAcceptor();

			static JMEngine::net::JME_TcpAcceptor::JME_TcpAcceptorPtr create(JMEngine::net::JME_NetHandler::JME_NetHandlerPtr handler, unsigned short port);

			void accept(size_t reconnect, size_t bufferSize);
		private:
			void handlerAccept(JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, size_t reconnect, size_t bufferSize, const boost::system::error_code& error);
		public:
			JME_NetHandler::JME_NetHandlerPtr _acceptorHandler;
			unsigned short _port;

			AcceptorPtr _acceptor;
		};
	}
}
#endif // JME_Acceptor_h__
