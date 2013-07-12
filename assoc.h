/* associative array */

/* hashtable的初始化 */
void assoc_init(const int hashpower_init);

/* 根据key、nkey和hash值查找对应的item ， 不存在返回null */
item *assoc_find(const char *key, const size_t nkey, const uint32_t hv);

/** 将item插入hashtable中 */
int assoc_insert(item *item, const uint32_t hv);

/** 从hashtable中删除相应的item */
void assoc_delete(const char *key, const size_t nkey, const uint32_t hv);

/** 尚未定义 */
void do_assoc_move_next_bucket(void);

/** 启动hashtable维护线程*/
int start_assoc_maintenance_thread(void);

/** 停止hashtable维护线程*/
void stop_assoc_maintenance_thread(void);

/* 决定桶的个数: 2**hashpower */
extern unsigned int hashpower;
