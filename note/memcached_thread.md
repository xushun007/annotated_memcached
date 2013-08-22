## memcached线程分析
徐顺 2013-08-20

memcached线程分析主要集中两个方面：  

* 线程的创建
* 线程的调度

memcached的线程池模型采用**Master-Worker**模型：

* 主线程负责监听、建立连接，并将其分发至工作线程处理；
* 工作线程处理该连接的读写事件。
![](../img/master-worker.png)

### 核心数据结构  

CQ_ITEM是对原始套接字的封装，表示一个新的连接

	/* An item in the connection queue. */
	typedef struct conn_queue_item CQ_ITEM;
	struct conn_queue_item {
	    int               sfd;              /* 原始套接字 */
	    enum conn_states  init_state;       /* 初始状态 */
	    int               event_flags;      /* 事件标志 */
	    int               read_buffer_size; /* 读缓冲区大小 */
	    enum network_transport     transport;
	    CQ_ITEM          *next;
	};


CQ连接队列，一工作线程一队列  

	/* CQ_ITEM队列 */
	typedef struct conn_queue CQ;
	struct conn_queue {
	    CQ_ITEM *head;
	    CQ_ITEM *tail;
	    pthread_mutex_t lock;
	    pthread_cond_t  cond;
	};