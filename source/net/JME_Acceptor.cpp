#include "net/JME_Acceptor.h"
#include "net/JME_Core.h"
#include "net/JME_TcpSession.h"
namespace JMEngine
{
	namespace net
	{


		TcpAcceptor::TcpAcceptor( JMEngine::net::NetHandler::NetHandlerPtr handler, unsigned short port ):
			_acceptorHandler(handler),
			_port(port)
		{
			_acceptor = AcceptorPtr(new boost::asio::ip::tcp::acceptor(JMECore.getNetIoService(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)));
		}

		TcpAcceptor::~TcpAcceptor()
		{

		}

		JMEngine::net::TcpAcceptor::JME_TcpAcceptorPtr TcpAcceptor::create( JMEngine::net::NetHandler::NetHandlerPtr handler, unsigned short port )
		{
			return JMEngine::net::TcpAcceptor::JME_TcpAcceptorPtr(new TcpAcceptor(handler, port));
		}

		void TcpAcceptor::accept(size_t reconnect, size_t bufferSize)
		{
			auto session = TcpSession::create(_acceptorHandler, bufferSize, reconnect);

			_acceptor->async_accept(session->socket(),
				boost::bind(&TcpAcceptor::handlerAccept, this, session, reconnect, bufferSize, boost::asio::placeholders::error));
		}

		void TcpAcceptor::handlerAccept( JMEngine::net::TcpSession::TcpSessionPtr session, size_t reconnect, size_t bufferSize, const boost::system::error_code& error )
		{
			if (!error)
			{
				accept(reconnect, bufferSize);

				JMECore.getLogicioService().post(
					boost::bind(&NetHandler::sessionConnectSucceed, _acceptorHandler, session));
			}
			else if (error.value() != 995)
			{
				accept(reconnect, bufferSize);
				
				session->stop();
			}
			else
			{
				session->stop();
			}
		}

	}
}