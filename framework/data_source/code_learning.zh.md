#DataSource 

提供通用的数据获取功能，不提供发送功能，目前支持的协议有 file rtmp http/s

## IDataSource

DataSource的抽象接口

DataSource在创建的时候必须要有一个uri，然后提供了两个open函数，两个open函数的意义不太一样

### int Open(int flags)

打开创建时传入的uri，flags目前作为预留接口，

###int Open(const std::string &url）