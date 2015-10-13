#JMEngine
c++ 网游服务器框架代码

网络：网络层使用boost::asio编写, 并再此之上封装了一个异步rpc调用模块, rpc协议使用protobuf, 支持超时

日志：自主封装的日志库,支持按文件大小分割日志, 动态更改日志级别, 日志输出到文件采用独立线程

内存：使用开源内存池 nedmalloc

数据库：封装有一个通用的数据库连接池(个人角色非常好用)

相关第三方库列表
protobuf
boost.1.49.0

相关使用示例在 TestServers项目里
地址： https://git.oschina.net/JimmyH/TestServers