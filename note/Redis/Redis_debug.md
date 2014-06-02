# 调试Redis 

徐顺 2014-4-22

## 编译及安装
* wget http://download.redis.io/releases/redis-2.8.8.tar.gz
* tar xzf redis-2.8.8.tar.gz
* cd redis-2.8.8/src
* vi Makefile ; 修改OPTIMIZATION?=-O2 为OPTIMIZATION?=-O0
* make CFLAGS="-g "
* make PREFIX=/usr/local/redis install
 
## 调试


* 查看redis进程： ps  aux|grep redis
* gdb附加到进程： gdb -p  进程id
* (gdb)r     重新开始不然不会从main函数开始
* (gdb)break main                  设置断点
* (gdb)list                  查看代码
* (gdb)p  变量名       查看变量内容，使用p查看变量，这个时候已经可以查看
* (gdb)n               单步调试
* (gdb)s               单步调试，若是函数，进入函数体内
* (gdb)b dict.c:dictAdd   为函数加入断点