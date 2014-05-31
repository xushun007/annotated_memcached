# Memcached源码结构
徐顺 2013-08-18

<!--

* assoc.h/assoc.c 	Hash表管理
* cache.h/cache.c	缓存管理(非核心)，此缓存非对象缓存
* daemonc.c	守护进程的实现
* hash.h/hash.c	Hash算法
* items.h/items.c	Item对象管理，数据对象存在Item中
* memcached.h/memcache.c	核心数据结构、命令解析、主函数
* protocol_binary.h	二进制协议
* sasl_defs.h/sasl_defs.c	SASL验证
* slab.h/slab.c	Slab Allocate内存分配管理
* stat.h/stat.c	数据状态统计管理
* thread.c	线程管理
* trace.h	宏定义
* util.h/util.c	实用函数，主要为字符串到整数的转换

![](../img/mem_source.jpg)

-->

## Memcached1.4.15源文件
<table class="table table-bordered table-striped table-condensed">
   <tr>
      <td>文件名</td>
      <td>描述</td>
   </tr>
   <tr>
      <td>assoc.h/assoc.c </td>
      <td>Hash表管理</td>
   </tr>
   <tr>
      <td>cache.h/cache.c</td>
      <td>缓存管理(非核心)，此缓存非对象缓存</td>
   </tr>
   <tr>
      <td>daemonc.c</td>
      <td>守护进程的实现</td>
   </tr>
   <tr>
      <td>hash.h/hash.c</td>
      <td>Hash算法</td>
   </tr>
   <tr>
      <td>items.h/items.c</td>
      <td>Item对象管理，数据对象存在Item中</td>
   </tr>
   <tr>
      <td>memcached.h/memcache.c</td>
      <td>核心数据结构、命令解析、主函数</td>
   </tr>
   <tr>
      <td>protocol_binary.h</td>
      <td>二进制协议</td>
   </tr>
   <tr>
      <td>sasl_defs.h/sasl_defs.c</td>
      <td>SASL验证</td>
   </tr>
   <tr>
      <td>slab.h/slab.c</td>
      <td>Slab Allocate内存分配管理</td>
   </tr>
   <tr>
      <td>stat.h/stat.c</td>
      <td>数据状态统计管理</td>
   </tr>
   <tr>
      <td>thread.c</td>
      <td>线程管理</td>
   </tr>
   <tr>
      <td>trace.h</td>
      <td>宏定义</td>
   </tr>
   <tr>
      <td>util.h/util.c</td>
      <td>实用函数，主要为字符串到整数的转换</td>
   </tr>
   <tr>
      <td></td>
   </tr>
</table>