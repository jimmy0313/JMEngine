#ifndef ConnectorServer_h__
#define ConnectorServer_h__

#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"

#include "JME_Acceptor.h"
#include "JME_NetHandler.h"
#include "JME_TcpSession.h"

#include <map>

#include "json/json.h"

using namespace std;
using namespace JMEngine;
using namespace JMEngine::net;
namespace JMEngine
{
	namespace server
	{
		// 管理所有客户端连接的服务器， 通常意义上的 网关服务器
		class JME_ConnectorServer :
			public JMEngine::net::JME_NetHandler,
			public boost::enable_shared_from_this<JME_ConnectorServer>
		{
		public:
			JME_ConnectorServer();
			virtual ~JME_ConnectorServer();

			//在这里进行一些必要的初始化
			void init();

			virtual void sessionConnectSucceed(JME_TcpSessionPtr session);	//该函数将由网络线程调用
			virtual void sessionConnectFailed(JME_TcpSessionPtr session, boost::system::error_code e); //该函数将由网络线程调用
			virtual void sessionDisconnect(JME_TcpSessionPtr session, boost::system::error_code e); //该函数将由外部线程调用
			virtual void sessionReceiveMessage(JME_TcpSessionPtr session, const char* dataPtr, const int len);	//该函数将由外部线程调用
			virtual void sessionReadError(JME_TcpSessionPtr session, boost::system::error_code e); //该函数将由外部线程调用
		protected:
			virtual void onInit();


		private:
			Json::Value _config;

			JMEngine::net::JME_TcpAcceptor::JME_TcpAcceptorPtr _acceptorPtr;

		protected:
			map<int,JME_TcpSessionPtr>	_clientSessions;
		};
	}
}
#endif // ConnectorServer_h__
