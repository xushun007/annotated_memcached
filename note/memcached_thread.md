# Memcached线程分析
徐顺 2013-08-28

## 概述

Memcached线程分析主要集中两个方面：  

* 线程的创建
* 线程的调度

Memcached的线程池模型采用**Master-Worker**模型：

* 主线程负责监听、建立连接，并将其分发至工作线程处理；
* 工作线程处理该连接的读写事件。

## 工作流程

![](../img/master-worker.png)

* Main线程根据配置创建一定数量的worker线程；
* Main线程和worker线程都独自拥有一个libevent的reactor实例，Main线程的reactor主要监听客户端的连接请求，当新请求到来时通过事件回调函数accept连接，创建对应的sfd，封装成CQ_ITEM实例，然后通过轮询机制放到对应的worker线程里面的CQ队列；
* Worker线程的reactor实例维护该线程所有相关联的sfd的读写事件，当对应sfd有I/0事件发生的时候，会回调主的处理函数(即分析客户端情况，进行创建/删除/查询等操作)；
* Main线程和worker线程的协作方式通过管道实现。Worker线程监听一个pipe的读事件，如果该pipe可读，表明有新客户端请求，因此Main线程在accept之后，将新的客户端请求放到worker线程中的CQ队列，然后向对应的pipe写数据即可。

## 核心数据结构  

### CQ_ITEM

	/* CQ_ITEM封装原始套接字，表示一个新的连接. */
	typedef struct conn_queue_item CQ_ITEM;
	struct conn_queue_item {
	    int               sfd;              /* 原始套接字 */
	    enum conn_states  init_state;       /* 初始状态 */
	    int               event_flags;      /* 事件标志 */
	    int               read_buffer_size; /* 读缓冲区大小 */
	    enum network_transport     transport;
	    CQ_ITEM          *next;
	};


### CQ连接队列，一工作线程一队列  

	/* CQ_ITEM队列 */
	typedef struct conn_queue CQ;
	struct conn_queue {
	    CQ_ITEM *head;
	    CQ_ITEM *tail;
	    pthread_mutex_t lock;
	    pthread_cond_t  cond;
	};

### 主线程和工作线程结构

	/* 主线程结构体 */
	typedef struct {
	    pthread_t thread_id;        /* 线程ID */
	    struct event_base *base;    /* libevent的实例event_base */
	} LIBEVENT_DISPATCHER_THREAD;



	/* 每个工作线程包含一个libevent的实例event_base、一对读写管道pipe 、 一个连接队列 */
	typedef struct {
	    pthread_t thread_id;        /* 线程ID */
	    struct event_base *base;    /* 此线程的libevent句柄 */
	    struct event notify_event;  /* 此事件对象与下面的notify_receive_fd描述符关联，主线程通过此事件通知工作线程有新连接的到来 */
	    int notify_receive_fd;      /* 与main thread通信的管道(pipe)的接收端描述符 */
	    int notify_send_fd;         /* 与main thread通信的管道(pipe)的发送端描述符*/
	    struct thread_stats stats;  /* 线程相关的统计/状态信息 */
	    struct conn_queue *new_conn_queue; /* 连接队列，由主线程添加，等待工作线程处理 */
	    cache_t *suffix_cache;      /* 后缀缓存 */
	    uint8_t item_lock_type;     /* 锁 */
	} LIBEVENT_THREAD;


### 网络连接

	/**
	 * 对网络连接的封装
	 * Memcached主要通过设置/转换连接的不同状态，来处理事件(核心函数是drive_machine)
	 */
	typedef struct conn conn;
	struct conn {
	    int    sfd;
	    sasl_conn_t *sasl_conn;
	    enum conn_states  state; /* 此连接状态，用于标记此连接在运行过程中的各个状态,其取值范围由conn_states枚举定义。      */
	    struct event event;
	    short  ev_flags;
	    short  which;   /** 哪个事件被触发 */
	    LIBEVENT_THREAD *thread; /* 服务此连接的工作线程 */
	    ....
	};


## 核心代码

### 工作线程初始化

    /**
	 * 初始化线程子系统，创建各种工作线程，由-t设定，默认为4
	 *
	 * 1. 锁和条件变量初始化
	 * 2. 工作线程结构体空间的分配与初始化
	 * 3. 启动工作线程
	 *
	 * nthreads  代表 worker 事件处理线程数目
	 * main_base 主线程的Event base
	 */
	void thread_init(int nthreads, struct event_base *main_base) {
	    int         i;
	    int         power;

	    /*  锁和条件变量的初始化 */

	    /* 缓存锁，用来同步缓存的存取 */
	    pthread_mutex_init(&cache_lock, NULL);

	    /* 状态锁，用来同步统计数据的存取 */
	    pthread_mutex_init(&stats_lock, NULL);

	    /* 线程初始化锁，用来同步init_count数 */
	    pthread_mutex_init(&init_lock, NULL);
	    /* 线程初始化条件变量，用来通知所有工作线程都完成初始化 */
	    pthread_cond_init(&init_cond, NULL);

	    /* 用来同步空闲连接链表的存取 */
	    pthread_mutex_init(&cqi_freelist_lock, NULL);
	    cqi_freelist = NULL;

	    /* Want a wide lock table, but don't waste memory */
	    /* 获取一个宽范围的锁表，但也不要浪费内存 */
	    if (nthreads < 3) {
	        power = 10;
	    } else if (nthreads < 4) {
	        power = 11;
	    } else if (nthreads < 5) {
	        power = 12;
	    } else {
	        /* 8192 buckets, and central locks don't scale much past 5 threads */
	        power = 13;
	    }

	    item_lock_count = hashsize(power);  // 2**power

	    /* 锁表的初始化 */
	    item_locks = calloc(item_lock_count, sizeof(pthread_mutex_t));
	    if (! item_locks) {
	        perror("Can't allocate item locks");
	        exit(1);
	    }
	    for (i = 0; i < item_lock_count; i++) {
	        pthread_mutex_init(&item_locks[i], NULL);
	    }
	    pthread_key_create(&item_lock_type_key, NULL);
	    pthread_mutex_init(&item_global_lock, NULL);

	    /* 工作线程结构体空间分配 */
	    threads = calloc(nthreads, sizeof(LIBEVENT_THREAD));
	    if (! threads) {
	        perror("Can't allocate thread descriptors");
	        exit(1);
	    }

	    /* 设置主线程的相关属性 */
	    dispatcher_thread.base = main_base;
	    dispatcher_thread.thread_id = pthread_self();

	    /* 初始化工作线程的相关属性 */
	    for (i = 0; i < nthreads; i++) {
	        int fds[2];
	        if (pipe(fds)) {
	            perror("Can't create notify pipe");
	            exit(1);
	        }

	        /* 管道用于主线程通知工作线程，新连接的到来 */
	        threads[i].notify_receive_fd = fds[0];
	        threads[i].notify_send_fd = fds[1];

	        setup_thread(&threads[i]);
	        /* 为 libevent base保留三个 fd，另外两个预留给管道 */
	        stats.reserved_fds += 5;
	    }

	    /* 完成所有的 libevent 设置后创建 worker 线程 ，
	     * worker_libevent函数调用event_base_loop()函数，循环监听该线程注册的IO事件。   */
	    for (i = 0; i < nthreads; i++) {
	        create_worker(worker_libevent, &threads[i]);
	    }

	    /* 等所有工作线程都已启动，主线程才开始接受连接 */
	    pthread_mutex_lock(&init_lock);
	    wait_for_thread_registration(nthreads);
	    pthread_mutex_unlock(&init_lock);
	}

	/** 创建一个工作线程 */
	static void create_worker(void *(*func)(void *), void *arg) {
	    pthread_t       thread;
	    pthread_attr_t  attr;
	    int             ret;

	    pthread_attr_init(&attr);

	    if ((ret = pthread_create(&thread, &attr, func, arg)) != 0) {
	        fprintf(stderr, "Can't create thread: %s\n",
	                strerror(ret));
	        exit(1);
	    }
	}


### 工作线程监听事件初始化
	/*
	 * 配置工作线程的信息
	 *
	 * 1. 创建工作线程的libevent实例
	 * 2. 设置工作线程接收管道的监听事件 
	 * 3. 创建连接队列并初始化
	 */
	static void setup_thread(LIBEVENT_THREAD *me) {
	    me->base = event_init();
	    if (! me->base) {
	        fprintf(stderr, "Can't allocate event base\n");
	        exit(1);
	    }

	    /* Listen for notifications from other threads */
	    event_set(&me->notify_event, me->notify_receive_fd,
	              EV_READ | EV_PERSIST, thread_libevent_process, me);
	    event_base_set(me->base, &me->notify_event);

	    if (event_add(&me->notify_event, 0) == -1) {
	        fprintf(stderr, "Can't monitor libevent notify pipe\n");
	        exit(1);
	    }

	    me->new_conn_queue = malloc(sizeof(struct conn_queue));
	    if (me->new_conn_queue == NULL) {
	        perror("Failed to allocate memory for connection queue");
	        exit(EXIT_FAILURE);
	    }
	    cq_init(me->new_conn_queue);

	    if (pthread_mutex_init(&me->stats.mutex, NULL) != 0) {
	        perror("Failed to initialize mutex");
	        exit(EXIT_FAILURE);
	    }

	    me->suffix_cache = cache_create("suffix", SUFFIX_SIZE, sizeof(char*),
	                                    NULL, NULL);
	    if (me->suffix_cache == NULL) {
	        fprintf(stderr, "Failed to create suffix cache\n");
	        exit(EXIT_FAILURE);
	    }
	}


### 主线程处理新连接

	/**
	 * 分发一个新连接到工作线程
	 *
	 */
	void dispatch_conn_new(int sfd, enum conn_states init_state, int event_flags,
	                       int read_buffer_size, enum network_transport transport) {

	    /* 新建一个连接队列的item */
	    CQ_ITEM *item = cqi_new();
	    char buf[1];

	    /* 选择目标线程：轮询，平衡负载 */
	    int tid = (last_thread + 1) % settings.num_threads;

	    LIBEVENT_THREAD *thread = threads + tid;

	    last_thread = tid;

	    /* item初始化 */
	    item->sfd = sfd;
	    item->init_state = init_state;
	    item->event_flags = event_flags;
	    item->read_buffer_size = read_buffer_size;
	    item->transport = transport;

	    /* 将item添加至目标线程的连接队列中 */
	    cq_push(thread->new_conn_queue, item);

	    Memcached_CONN_DISPATCH(sfd, thread->thread_id);
	    buf[0] = 'c';
	    /* 向工作线程的写fd中写入一字节, 实现主从线程的通信 */
	    if (write(thread->notify_send_fd, buf, 1) != 1) {
	        perror("Writing to thread notify pipe");
	    }
	}

### 工作处理新连接

	/**
	 * 工作线程处理一个新连接
	 * 当工作线程的读管道上有数据来的时候，触发此方法的调用
	 */
	static void thread_libevent_process(int fd, short which, void *arg) {
	    LIBEVENT_THREAD *me = arg;
	    CQ_ITEM *item;
	    char buf[1];

	    if (read(fd, buf, 1) != 1)
	        if (settings.verbose > 0)
	            fprintf(stderr, "Can't read from libevent pipe\n");

	    switch (buf[0]) {
	    case 'c':
	    item = cq_pop(me->new_conn_queue);

	    if (NULL != item) {
	        /* 根据item对象，创建conn对象，该对象负责客户端与该工作线程之间的通信 */
	        conn *c = conn_new(item->sfd, item->init_state, item->event_flags,
	                           item->read_buffer_size, item->transport, me->base);
	        if (c == NULL) {
	            if (IS_UDP(item->transport)) {
	                fprintf(stderr, "Can't listen for events on UDP socket\n");
	                exit(1);
	            } else {
	                if (settings.verbose > 0) {
	                    fprintf(stderr, "Can't listen for events on fd %d\n",
	                        item->sfd);
	                }
	                close(item->sfd);
	            }
	        } else {
	            c->thread = me;
	        }
	        cqi_free(item);
	    }
	        break;
	    /* we were told to flip the lock type and report in */
	    case 'l':
	    me->item_lock_type = ITEM_LOCK_GRANULAR;
	    register_thread_initialized();
	        break;
	    case 'g':
	    me->item_lock_type = ITEM_LOCK_GLOBAL;
	    register_thread_initialized();
	        break;
	    }
	}