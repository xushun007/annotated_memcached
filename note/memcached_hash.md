# Memcached Hash机制 
徐顺  2013-09-03

Memcached的底层内存分配通过slab分配机制完成，键值检索由Hash机制完成，以提供检索效率，Hash冲突的解决方案是开链法。

Hash+Slab

* Item：一个存储项，存储一个具体的key-value结构
* Slab：item的申请都是通过slab机制分配，以支持高效的内存分配和减少内存碎片

slab机制的存在对hash表是透明的，而hash表对slab层也是不可见的，Item对象让这两层成功解耦。


## Hash实现

* 通过hash值确定item在hash表中位置
* 如果发生冲突，就采用链表存储
* 如果现在总的item个数大于数组大小的1.5,则尝试扩展hash表
    * Memcached中有两个hash 表，一个是“主hash 表”（primary_hashtable），另外一个是“原有hash表”（old_hashtable）
    * 每次操作的时候，先会检测表是否正处于扩展(expanding)状态，如果扩展还没完成时，先
