#JMEngine
开发环境选择<br>
windows: vs2012以上(vs2012为支持可变模板参数,编译器需要使用vc_compilerCTPNov2012,external提供下载)<br>
linux: gcc 4.8.1以上<br>

c++ 网游服务器框架代码

网络：网络层使用boost::asio编写, 并再此之上封装了一个异步rpc调用模块, rpc协议使用protobuf, 支持超时

client -> server
```cpp
class ClientHandler final : public JME_NetHandler
{
public:
	void sessionConnectSucceed(JME_TcpSession::JME_TcpSessionPtr session)
	{
		session->start(1);
	}
	void sessionConnectFailed(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{
	}
	void sessionDisconnect(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{
	}
	void sessionReceiveMessage(JME_TcpSession::JME_TcpSessionPtr session,  JME_Message::JME_MessagePtr msg)
	{
	}
	void sessionReadError(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{
	}
};
int main()
{
	JMECore.start();
	auto client_handler = boost::shared_ptr<ClientHandler>(new ClientHandler);
	auto client_session = JME_TcpSession::create(client_handler, 10240, 5);

 	client_session->connect("127.0.0.1", "2010");

	while (1)
	{
		string cmd;
		cin >> cmd;

		if (!cmd.compare("quit"))
		{
			break;
		}
	}
	JMECore.stop();
	return 0;
};

```

server <- client
```cpp
class AcceptorHandler : 
	public JME_NetHandler
{
public:
	void sessionConnectSucceed(JME_TcpSession::JME_TcpSessionPtr session)
	{
		session->start(1);
	}
	void sessionConnectFailed(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{
	}
	void sessionDisconnect(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{
	}
	void sessionReceiveMessage(JME_TcpSession::JME_TcpSessionPtr session,  JME_Message::JME_MessagePtr msg)
	{
	}
	void sessionReadError(JME_TcpSession::JME_TcpSessionPtr session, boost::system::error_code e)
	{		
	}
};
int main()
{
	JMECore.start();

	auto acceptor_handler = boost::shared_ptr<AcceptorHandler>(new AcceptorHandler);
	auto acceptor = JME_TcpAcceptor::create(acceptor_handler, 2010);
	acceptor->accept(0, 1024);

	while (1)
	{
		string cmd;
		cin >> cmd;

		if (!cmd.compare("quit"))
		{
			break;
		}
	}
	JMECore.stop();
	return 0;
};
```
日志：自主封装的日志库,支持按文件大小分割日志, 动态更改日志级别, 日志输出到文件采用独立线程

内存：使用开源内存池 nedmalloc

数据库：封装有一个通用的数据库连接池(个人觉得非常好用)
```cpp

typedef JMEngine::db::DBHelper<mongo::DBClientConnection> MongoDBHelper;
typedef JMEngine::db::DBPool<mongo::DBClientConnection> MongoDBPool;

bool connect(mongo::DBClientConnection* conn, const string& ip_str, const string& db_name, const string& name, const string& pwd)
{
	string err = "";
	if (!conn->connect(ip_str, err))
	{
		LOGE(err);
		return false;
	}
	if (!conn->auth(db_name, name, pwd, err))
	{
		LOGE(err);
		return false;
	}
	return true;
}
int main()
{
	MongoDBPool::DBPoolPtr _gdDBPool = MongoDBPool::create(n, boost::bind(&connect, _1, ip_str, db_name, name, pwd));
	try
	{
		MongoDBHelper conn(_gdDBPool);
		mongo::BSONObj b = conn->findOne(db_name_str, query.obj(), fileds);
	
		if(b.isEmpty())
			return 1;
		Json::Value val;
		Json::Reader reader;
		reader.parse(b.jsonString(),val);
		val.removeMember("_id");
		return 0;
	}
	catch(const JMEngine::db::JME_DBException& err)
	{
		LOGE(err.what());
	}
	return 1;
}

```

2015/12/31 新增 简单udp监听
```cpp

class UdpHandler :
	public JME_UdpNetHandler
{
public:
	void onReceive(JME_UdpSessionPtr session, JME_MessagePtr msg)
	{		
		//逻辑线程处理接收到的消息
		session->writeMessage(*msg);
	}
	void onWrite(JME_UdpSessionPtr session)
	{
		//处理完一条消息后,才能继续接受下一条消息
		session->startRead();
	}
};
int main()
{
	JMECore.start();

	auto udp_handler = boost::shared_ptr<UdpHandler>(new UdpHandler);
	auto udp_session = JME_UdpSession::create(udp_handler, 6002, 10240);
	udp_session->startRead();

	while (1)
	{
		string cmd;
		cin >> cmd;

		if (!cmd.compare("quit"))
		{
			break;
		}
	}
	JMECore.stop();
	return 0;
};

```
相关第三方库列表
protobuf
boost.1.49.0

相关使用示例在 TestServers项目里
地址： https://git.oschina.net/JimmyH/TestServers