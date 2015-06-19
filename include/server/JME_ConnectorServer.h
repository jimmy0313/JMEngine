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
		// �������пͻ������ӵķ������� ͨ�������ϵ� ���ط�����
		class JME_ConnectorServer :
			public JMEngine::net::JME_NetHandler,
			public boost::enable_shared_from_this<JME_ConnectorServer>
		{
		public:
			JME_ConnectorServer();
			virtual ~JME_ConnectorServer();

			//���������һЩ��Ҫ�ĳ�ʼ��
			void init();

			virtual void sessionConnectSucceed(JME_TcpSessionPtr session);	//�ú������������̵߳���
			virtual void sessionConnectFailed(JME_TcpSessionPtr session, boost::system::error_code e); //�ú������������̵߳���
			virtual void sessionDisconnect(JME_TcpSessionPtr session, boost::system::error_code e); //�ú��������ⲿ�̵߳���
			virtual void sessionReceiveMessage(JME_TcpSessionPtr session, const char* dataPtr, const int len);	//�ú��������ⲿ�̵߳���
			virtual void sessionReadError(JME_TcpSessionPtr session, boost::system::error_code e); //�ú��������ⲿ�̵߳���
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
