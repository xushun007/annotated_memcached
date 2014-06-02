# 内存管理

徐顺 2014-4-20

Redis通过对通用的malloc()，realloc(), free()和calloc()函数进行简单的封装来构造自己的内存管理。
分别与之对应的函数为zmalloc(), zrealloc(), zfree()和zcalloc()。

### 好处
* 可以利用内存池等手段提高内存的分配性能
* 可以掌握更多的内存信息

Redis在申请需要的size字节之外，还会额外多申请一块定长(PREFIX_SIZE)的长度用来记录所申请的内存大小，一遍更好的监控内存使用情况。

如果系统中包含TCMALLOC(Google perftools的一个组件)，则会使用tc_malloc等系列方法代替malloc()等方法。

