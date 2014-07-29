annotated_memcached_source
==========================

对memecached1.4.15源码进行注释

memcached项目位置：https://github.com/memcached/memcached.git


## 源码笔记

徐顺 2013-08-10

## 大纲

`Memcached`是一个高性能的分布式内存对象缓存系统，用于动态 Web 应用以减轻数据库负载。  
此系列文章通过阅读源码对`Memcached1.4.15`的核心部分进行了解析，希望和对Memcached感兴趣的朋友们一起学习，共同进步。在阅读文章中，如有建议敬请反馈(xsshun@gmail.com)，谢谢。

* [Memcached简介](note/memcached_feature.md)
* [Memcached实战](note/memcached_in_action.md)
* [Memcached源码结构](note/memcached_source_arch.md)
* [Memcached内存管理](note/memcached_memory.md)
* [Memcached线程分析](note/memcached_thread.md)
* [Memcached Hash机制](note/memcached_hash.md)
* [Memcached一致性哈希](note/memcached_consistent_hash.md)


## Redis 笔记

2014-3-18

Redis是一个高性能，基于内存的key-value存储系统。
和Memcached类似，它支持存储的value类型相对更多，且Redis会周期性的把更新的数据写入磁盘或者把修改操作写入追加的记录文件。

此系列文章通过阅读``Redis2.6`的源码，做了几篇笔记，希望和对Redis感兴趣的朋友们一起学习，共同进步。在阅读文章中，如有建议敬请反馈，谢谢。


* [Redis工作流程解析](note/Redis/Redis_main_flow.md)
* [Redis基本数据结构](note/Redis/Redis_datastruct.md)
* [Redis网络事件框架及Demo](note/Redis/Redis_Event_demo.md)
* [Redis协议](note/Redis/Redis_Protocal.md)
* [Redis内存](note/Redis/Redis_Memory.md)
* [Redis调试](note/Redis/Redis_debug.md)
* [Redis应用场景](note/Redis/Redis_application.md)

基于Redis网络事件框架模型echo服务器，项目地址: [https://github.com/microheart/Redis_Event_Demo](https://github.com/microheart/Redis_Event_Demo)

参考文献：

[Redis 设计与实现](http://origin.redisbook.com/en/latest/)

## Hadoop 笔记

2014-6

[Hadoop笔记](https://github.com/microheart/hadoop_note)