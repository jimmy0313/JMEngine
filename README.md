#JMEngine

第三方库列表
protobuf
boost.1.49.0

相关使用示例在 JMEngineTest项目里
地址： https://git.oschina.net/JimmyH/JMEngineTest

RPC 使用示例:
client:

int main()
{
	auto rpc = JME_RpcClient::create("127.0.0.1", "3001", 5, 1024);

	while (1)
	{
		//role_list_proto 是使用protobuf定义的协议
		role_list_proto p;
		p.set_id(1);
		p.set_sid(1);

		rpc->callRpcMethod("f1", &p, f1);
		rpc->callRpcMethod("f2", &p, f1, 3, boost::bind(f3, "f2"));	//远程调用函数f2, 回调函数f1, 3秒后超时回调f3
		rpc->callRpcMethod("f3", &p, f1, 3, boost::bind(f3, "f3")); //远程调用函数f3, 回调函数f1, 3秒后超时回调f3

		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}
	return 0;
};

server:

class rpcHandler : public JME_RpcHandler
{
public:
	void init()
	{
		regRpcHandler("f1", rpcHandler::f1);
		regRpcHandler("f2", rpcHandler::f2);
	}

	static void f1(JME_RpcServer::JME_RpcServerPtr conn, JMEngine::net::JME_TcpSessionPtr session, const JME_Rpc& params)
	{
		string m = "f1 rpc response";
		JME_Rpc res(params._rpcId, __FUNCTION__, m);
 		conn->response(session, res);

		LogT << "recv rpc client call method " << __FUNCTION__ << ", rpc id: " << params._rpcId << LogEnd;
	}

	static void f2(JME_RpcServer::JME_RpcServerPtr conn, JMEngine::net::JME_TcpSessionPtr session, const JME_Rpc& params)
	{
		string m = "f2 rpc response";
		JME_Rpc res(params._rpcId, __FUNCTION__, m);
		conn->response(session, res);

		LogT << "recv rpc client call method " << __FUNCTION__ << ", rpc id: " << params._rpcId << LogEnd;
	} 
};

int main()
{
	auto h1 = new rpcHandler;
	h1->init();

	auto rpc = JME_RpcServer::create(JME_RpcHandler::create(), 3001, 1024);

	while (1)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}
	return 0;
};