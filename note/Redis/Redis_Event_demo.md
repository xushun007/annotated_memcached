# Redis网络事件框架及Demo

徐顺  2014-4-15

为什么需要事件库(来源于http://blog.ddup.us/?p=114)【FAQ】

* Q：你期望一个网络服务器如何工作？
* A：在它监听的端口等待连接的到来并且为之服务。
* Q：在一个描述符上调用accept时会阻塞，你是如何处理的？
* A：先保存这个描述符然后在描述符上进行非阻塞的read/write操作。
* Q：为什么要进行非阻塞的read/write？
* A：如果操作正阻塞在文件（Unix中socket也是看作文件的）上，那么服务器还怎么再接受其他的连接请求呢。
* Q：我觉得我要在socket上做很多的非阻塞的操作，是这样吗？
* A：是的，这也正是事件库为你完成的工作。
* Q：那么事件库是如何工作的？
* A：它使用操作系统提供的带计时器的polling方式。
* Q：那么有没有什么开源的事件库能完成你描述的功能吗？
* A：是的，Libevent和Libev是我能想起来的你说的那样的库。
* Q：Redis使用的这样的的事件库来处理socket I/O的吗？
* A：不是的，因为一些原因Redis使用了自己的事件库。


## 事件处理机制

Redis是单线程模型(bgsave等功能会启动子进程)，通过异步多路复用机制的处理客户请求。

Redis的事件模型在不同的系统中提供不同的实现，充分利用当前平台最好的异步处理方式。

* Linux： ae_epoll.c  epoll模型的实现
* FreeBSD: ae_kqueue.c // kqueue模型的实现
* Sun _DTRACE_VERSION: ae_evport.c
* Other: select.c      // select模型的实现

Redis支持两种基本事件：文件事件(FileEvent)和时间时间(TimeEvent).
前者基于操作系统的异步机制(epoll/kqueue)时间文件事件，后者是Redis自己实现的定时器。

## Redis 事件框架
Redis没有选择流行的事件处理框架libevent(Memcached使用)和libevent，而是自己实现了一个精简，高性能的异步网络事件框架。

Libevent为了迎合通用性造成代码庞大而牺牲了在特定平台的不少性能。

Redis的异步网络事件模块与其他模块相对独立，可独立出来，将其复用，其代码相当精炼，可读性很好，值得学习和应用。


我们将抽取其事件处理模块代码`(ae.h, ae.c, ae_epoll.c, anet.h, anet.c)`，源码来源于Redis2.6, 实现一个简单的回显服务器。
Redis的内存管理是应用zmalloc()、zfree()等系列函数，可以简单的将其替换成原始的malloc()和free()函数即可。

## 实例
项目地址: [https://github.com/microheart/Redis_Event_Demo](https://github.com/microheart/Redis_Event_Demo)，获取源码，直接make编译即可。
Server监听23456端口，不断处理来自客户端的请求；同时启动一个定时器，类似于Redis中的ServerCron。

### 启动Server, 监听默认端口
* $ ./server

### 启动客户端
* $ telnet 10.64.40.36 23456 

### 附上main.c源码

	/**
	 * 
	 * 基于Redis网络事件框架Demo
	 * 
	 * @author xushun 
	 *  email : xushun007@gmail.com
	 */


	#include <stdio.h>
	#include <errno.h>
	#include <signal.h>
	#include <string.h>
	#include "ae.h"
	#include "anet.h"

	#define MAX_LEN 10240
	#define MAX_REQUEST_SIZE 5120

	#define PORT 23456
	#define IP_ADDR_LEN 40

	#define TIMER_LOOP_CYCLE 10000

	// 读数据状态
	#define OK 0
	#define ERR -1
	#define NO_READY 1

	// 存放错误信息的字符串
	char g_err_string[MAX_LEN];

	// 事件循环体
	aeEventLoop *g_event_loop = NULL;

	// 客户端结构体
	typedef struct client {
		int fd;                    // 文件描述符
		char ipaddr[IP_ADDR_LEN];  // client IP 地址
		int port;                  // 端口
		char request[MAX_LEN];     // 客户端请求字符串
		char response[MAX_LEN];    // 响应字符串
		int len;                   // 缓冲长度
		int rsplen;
	}client_t;

	client_t* createClient() {
		client_t* c = malloc(sizeof(client_t));
		if(c == NULL) {
			fprintf(stderr, "alloc mem failure.");
			exit(1);
		}
		c->fd = -1;
		c->len = 0;
		c->rsplen = 0;
		return c;
	}

	// 定时器
	int PrintTimer(struct aeEventLoop *eventLoop, long long id, void *clientData)
	{
		static int i = 0;
		printf("Timer: %d\n", i++);
		// TIMER_LOOP_CYCLE/1000 秒后再次执行该函数
		return TIMER_LOOP_CYCLE;
	}

	//停止事件循环
	void StopServer()
	{
		aeStop(g_event_loop);
	}

	// 客户退出处理函数
	void ClientClose(aeEventLoop *el, client_t* c, int err)
	{
		//如果err为0，则说明是正常退出，否则就是异常退出
		if( 0 == err )
			printf("Client quit: %d\n", c->fd);
		else if( -1 == err )
			fprintf(stderr, "Client Error: %s\n", strerror(errno));

		//删除结点，关闭文件
		aeDeleteFileEvent(el, c->fd, AE_READABLE);
		close(c->fd);
		free(c);
	}

	// 响应客户
	void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask) {	
		client_t* c = privdata;
		int nwritten = c->rsplen;
		int res, sentlen = 0;
		
		printf("Request From %s:%d : %s\n", c->ipaddr, c->port, c->response);
		while(nwritten) {
		    res = write(fd, c->response + sentlen, c->rsplen - sentlen);

		    // 写入出错
		    if (res == -1) {
			if (errno == EAGAIN) {
			    continue;
			} else {
			    fprintf(stderr, "send response to client failure.\n");
			    ClientClose(el, c, res);
			}
		    }
	            
	            nwritten -= res;
	            sentlen += res;

	            if (sentlen == c->rsplen) {
	                c->rsplen = 0;
	            }
		}

		aeDeleteFileEvent(el,c->fd,AE_WRITABLE);
	}

	int processBuffer(aeEventLoop *el, client_t* c, int res) {
		char *newline = strstr(c->request,"\r\n");
		int reqlen;	

		if(newline == NULL) {
			if(c->len > MAX_REQUEST_SIZE) {
				fprintf(stderr,"Protocol error: too big request");
				ClientClose(el, c, res);
				return ERR;
			}
			return NO_READY;
		}

		reqlen = newline - c->request + 2;
		memcpy(c->response, c->request, reqlen);
		c->rsplen = reqlen;
		c->len -= reqlen;
		if(c->len)
			memmove(c->request, c->request+reqlen, c->len);

		return OK;
	}

	// 读取客户端数据
	void ReadFromClient(aeEventLoop *el, int fd, void *privdata, int mask)
	{
		int res;
		client_t* c = privdata;
		res = read(fd, c->request + c->len, MAX_LEN - c->len);
		if( res <= 0 )
		{
			ClientClose(el, c, res);
			return;
		}
		c->len += res;	
		
		if(processBuffer(el, c, res) == OK) {
			if(aeCreateFileEvent(el, fd, AE_WRITABLE, sendReplyToClient, c) == AE_ERR)  {
				fprintf(stderr, "Can't Register File Writeable Event.\n");
				ClientClose(el, c, res);
			}			
		}
		
	}

	//接受新连接
	void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask)
	{
		client_t* c = createClient();

		c->fd = anetTcpAccept(g_err_string, fd, c->ipaddr, &c->port);
		if(c->fd == ANET_ERR) {
			fprintf(stderr, "Accepting client connection: %s", g_err_string);
			free(c);
			return;	
		}
		printf("Connected from %s:%d\n", c->ipaddr, c->port);
		
		if( aeCreateFileEvent(el, c->fd, AE_READABLE, ReadFromClient, c) == AE_ERR )
		{
			fprintf(stderr, "Create File Event fail: fd(%d)\n", c->fd);
			close(c->fd);
	                free(c);
		}
	}

	int main()
	{

		printf("Start\n");
		signal(SIGINT, StopServer);

		//初始化网络事件循环
		g_event_loop = aeCreateEventLoop(1024*10);

		//设置监听事件
		int fd = anetTcpServer(g_err_string, PORT, NULL);
		if( ANET_ERR == fd )
			fprintf(stderr, "Open port %d error: %s\n", PORT, g_err_string);
		if( aeCreateFileEvent(g_event_loop, fd, AE_READABLE, AcceptTcpHandler, NULL) == AE_ERR )
			fprintf(stderr, "Unrecoverable error creating server.ipfd file event.");

		//设置定时事件
		aeCreateTimeEvent(g_event_loop, 1, PrintTimer, NULL, NULL);

		//开启事件循环
		aeMain(g_event_loop);

		//删除事件循环
		aeDeleteEventLoop(g_event_loop);

		printf("End\n");
		return 0;
	}