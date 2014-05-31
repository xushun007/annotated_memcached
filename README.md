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
