#include "net/JME_Acceptor.h"
#include "net/JME_Core.h"
#include "net/JME_TcpSession.h"
namespace JMEngine
{
	namespace net
	{


		JME_TcpAcceptor::JME_TcpAcceptor( JMEngine::net::JME_NetHandler::JME_NetHandlerPtr handler, unsigned short port ):
			_acceptorHandler(handler),
			_port(port)
		{
			_acceptor = AcceptorPtr(new boost::asio::ip::tcp::acceptor(JMECore.getNetIoService(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)));
		}

		JME_TcpAcceptor::~JME_TcpAcceptor()
		{

		}

		JMEngine::net::JME_TcpAcceptor::JME_TcpAcceptorPtr JME_TcpAcceptor::create( JMEngine::net::JME_NetHandler::JME_NetHandlerPtr handler, unsigned short port )
		{
			return JMEngine::net::JME_TcpAcceptor::JME_TcpAcceptorPtr(new JME_TcpAcceptor(handler, port));
		}

		void JME_TcpAcceptor::accept(size_t reconnect, size_t bufferSize)
		{
			auto session = JME_TcpSession::create(_acceptorHandler, bufferSize, reconnect);

			_acceptor->async_accept(session->socket(),
				boost::bind(&JME_TcpAcceptor::handlerAccept, this, session, reconnect, bufferSize, boost::asio::placeholders::error));
		}

		void JME_TcpAcceptor::handlerAccept( JMEngine::net::JME_TcpSession::JME_TcpSessionPtr session, size_t reconnect, size_t bufferSize, const boost::system::error_code& error )
		{
			if (!error)
			{
				accept(reconnect, bufferSize);

				JMECore.getLogicioService().post(
					boost::bind(&JME_NetHandler::sessionConnectSucceed, _acceptorHandler, session));
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