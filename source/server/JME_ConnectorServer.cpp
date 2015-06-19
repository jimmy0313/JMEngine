#include "JME_ConnectorServer.h"
#include "JME_Core.h"
#include "JME_JsonFile.h"

namespace JMEngine
{
	namespace server
	{


		JME_ConnectorServer::JME_ConnectorServer()
		{

		}

		JME_ConnectorServer::~JME_ConnectorServer()
		{

		}

		void JME_ConnectorServer::init()
		{
			_config = JMEngine::file::load_jsonfile_val("./config/server_cfg.json");

			unsigned short port = _config["port"].asInt();

			_acceptorPtr = JMEngine::net::JME_TcpAcceptor::create(JME_NetHandler::JME_NetHandlerPtr(this), port);

			onInit();

			//初始化完成后开始接受客户端连接

			size_t clientBufferSize = _config["client_buffer_size"].asUInt();
			_acceptorPtr->accept(0, clientBufferSize);
		}

		void JME_ConnectorServer::onInit()
		{

		}

		void JME_ConnectorServer::sessionConnectSucceed( JME_TcpSessionPtr session )
		{
			LogT << "New client connected from " << session->socket().remote_endpoint().address().to_string() << LogEnd;

			static size_t id = 1;
			session->start(++id);

			_clientSessions[id] = session;
		}

		void JME_ConnectorServer::sessionConnectFailed( JME_TcpSessionPtr session, boost::system::error_code e )
		{
			LogW << "Connect to " << session->getIp() << ":" << session->getPort() << " failed" << LogEnd;
		}

		void JME_ConnectorServer::sessionDisconnect( JME_TcpSessionPtr session, boost::system::error_code e )
		{

		}

		void JME_ConnectorServer::sessionReceiveMessage( JME_TcpSessionPtr session, const char* dataPtr, const int len )
		{

		}

		void JME_ConnectorServer::sessionReadError( JME_TcpSessionPtr session, boost::system::error_code e )
		{
			try
			{
				_clientSessions.erase(session->getNetId());
			}
			catch(std::exception& e)
			{
				LogD << e.what() << LogEnd;
			}
		}

	}
}