#JMEngine

第三方库列表
protobuf
boost.1.49.0

相关使用示例在 TestServers项目里
地址： https://git.oschina.net/JimmyH/TestServers

PS:
由于不怎么会用git创建分支，所以 暂时把修改的部分在backup中提交

这是一个利用共享内存作为服务器间交互的分支, 主要在socket的读写缓冲区做的手脚 
当前版本连接成功后, 如果连接ip为“127.0.0.1” 则根据ip+port 生成唯一共享内存名

已知问题： 可能由于实现问题, 效率远远不如直接socket版