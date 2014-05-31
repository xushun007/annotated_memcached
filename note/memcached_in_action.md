# Memcached实战
徐顺 2013-08-15

Memcached 是一个高性能的分布式内存对象缓存系统，用于动态 Web 应用以减轻数据库负载。它通过在内存中缓存数据和对象来减少读取数据库的次数，从而提高动态、数据库驱动网站的速度。

## Memcached安装

Ubuntu下安装Memcached

	$wget https://github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz
	$./configure --prefix=/usr/local
	$make
	$sudo make install


	$wget http://Memcached.googlecode.com/files/Memcached-1.4.15.tar.gz
	$./configure --with-libevent=/usr/local
	$make
	$make install


## Memcached使用

	$/usr/local/bin/Memcached -d -u nobody -m 1024 -p 11210 -l 10.11.12.70 -P /opt/Memcached/pid/m11210.pid

###主要启动参数

**启动方式**：

* -d 		以守护程序（daemon）方式运行
* -u root 	指定用户，如果当前为 root ，需要使用此参数指定用户
* -P /tmp/a.pid	保存PID到指定文件

**内存设置**：

* -m 1024 	数据内存数量，不包含Memcached本身占用，单位为 MB
* -M 		在总内存用完后，不允许通过LRU算法把还没过期的数据删掉，即禁止LRU淘汰机制
* -n 48		初始chunk=key+suffix+value+32结构体的大小，默认48字节
* -f 1.25 	增长因子，相邻的slabclass的chunk的大小关系，默认1.25
* -L		启用大内存页，可以降低内存浪费，改进性能

**连接设置**：

* -l 127.0.0.1 	监听的 IP 地址，本机可以不设置此参数
* -p 11211 	TCP端口，默认为11211，可以不设置
* -U 11211	UDP端口，默认为11211，0为关闭

**并发设置**：

* -c 1024	最大并发连接数，默认1024，最好是200
* -t 4		线程数，默认4。由于Memcached采用NIO，所以更多线程没有太多作用
* -R 20		每个event连接最大并发数，默认20
* -C		禁用CAS命令（可以禁止版本计数，减少开销）

### 使用限制

* 最大键长为250字节，大于该长度无法存储，常量KEY_MAX_LENGTH 250 控制
* 单个item最大数据是1MB，超过1MB数据不予存储，常量POWER_BLOCK 1048576 进行控制，它是默认的slab大小
* 最大30天的数据过期时间
* 32位机器进程内存限制

## Memcached命令列表

* 存储命令：set/add/replace/append/prepend/cas，指示服务器储存一些由键值标识的数据
* 读取命令：get=bget?/gets，指示服务器返回与所给键值相符合的数据
* 删除命令：delete，指示服务器删除指定键值相同的数据
* 统计命令：stats/settings/items/sizes/slabs，用于查询服务器的运行状态
* 其他命令：incr/decr/version/quit

## Memcached缺陷

* 无法备份，重启无法恢复
* 没有持久化，重启全部丢失
* 任何机器都可以telnet，需要放在防火墙后
* slab分配存在空间浪费
* 没有合理的日志
