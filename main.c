#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

//#include <rte_ring_core.h>
// #include<pthread.h>
// pthread_mutex_t mutex;
// pthread_mutex_init(mutex);

//原子锁
static int lock;
#define atomic_cas(dst, old, new) __sync_bool_compare_and_swap((dst), (old), (new))
#define atomic_lock(ptr)\
while(!atomic_cas(ptr,0,1))
#define atomic_unlock(ptr)\
while(!atomic_cas(ptr,1,0))


#define RX_RING_SIZE 128        //接收环大小
#define TX_RING_SIZE 512        //发送环大小

#define NUM_MBUFS 8191*16
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#define string_size 128
#define ring_size  1024*1024*8

static int cont=0;
static int val=1;
struct rte_mempool *mbuf_pool;  //指向内存池结构的指针变量
struct rte_ring *ring; //无锁队列
static void en_queue(char * strng_val)
{
    //void * msg;
    // if (rte_mempool_get(mbuf_pool, &msg) < 0)
    //     return 0;
    //snprintf((char *)msg, string_size, "%s", "helloworld");
                //pthread_mutex_lock(&mutex);

    if (rte_ring_enqueue(ring, strng_val) < 0) {
            rte_mempool_put(mbuf_pool, strng_val);
        }
            //pthread_mutex_unlock(&mutex);
}

static void de_queue()
{
    void *recv_msg;
    unsigned vv,core_id;
    rte_mempool_get(mbuf_pool, &recv_msg);//每次获取的recv_msg的长度是可以变化的的
    if (rte_ring_dequeue(ring, &recv_msg) < 0){
        //usleep(5);
        //printf("Received-------------------------: '%s'", (char *)recv_msg);
        //return 0;
    }
    //pthread_mutex_lock(&mutex);

    //pthread_mutex_unlock(&mutex);
     sscanf(recv_msg,"hello from core %u:---data:%u\n",&core_id,&vv);
    // printf("%u--%u\n",vv,val);
    // if(vv==val)
    // {
    //     val+=1;
         printf("Received:%u  -- : '%s'",strlen(recv_msg),  (char *)recv_msg);
    // }else
    // {
    //     val = vv+1;
    //     printf("end!!!!---\n");
    // }
    //pthread_mutex_unlock(&mutex);
    rte_mempool_put(mbuf_pool, recv_msg);
}

static int
lcore_hello(__attribute__((unused)) void *arg)
{
        unsigned lcore_id;
        lcore_id = rte_lcore_id();
        void *msg;
        int num=10000;
        //printf("hello world-- %d\n",lcore_id);
        while(num--)
        {
            if (rte_mempool_get(mbuf_pool, &msg) < 0)
                return 0;
            //printf("-----\n");
            //usleep(5);
            //pthread_mutex_lock(&mutex);
            atomic_lock(&lock);
            cont+=1;
            atomic_unlock(&lock);
            //pthread_mutex_unlock(&mutex);
            sprintf(msg,"hello from core %u:---data:%u\n", lcore_id,cont);
            //lock_guard<mutex> myguard(my_mutex);
            en_queue(msg);
             
            //de_queue();
        }
        //rte_mempool_put(mbuf_pool, msg);
        return 0;
}

static int 
another_lcore(__attribute__((unused)) void *arg)
{
        unsigned lcore_id;
        lcore_id = rte_lcore_id();
        //printf("another thing core-- %d\n",lcore_id);
        while (1)
        {
            while (!rte_ring_empty(ring))
            {
                /* code */
                de_queue();
                usleep(500);
            }
            
        }
        return 0;
}
int
main(int argc, char **argv)
{
        int ret;
        unsigned lcore_id; 

	unsigned nb_ports,i;              //网口个数
	uint8_t portid;                 //网口号，临时的标记变量


        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");


nb_ports = rte_eth_dev_count(); //获取当前有效网口的个数
	if (nb_ports < 2 || (nb_ports & 1))     //如果有效网口数小于2或有效网口数为奇数0，则出错
		rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

	//此函数为rte_mempoll_create()的封装
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

        ring = rte_ring_create("message_ring",
                ring_size, rte_socket_id(), 0x0008 | 0x0010);
        
    //     void  * msg;
    //     if (rte_mempool_get(mbuf_pool, &msg) < 0)
    //         return 0;

    //     snprintf((char *)msg, string_size, "%s", "helloworld");
    //     if (rte_ring_enqueue(ring, msg) < 0) {
    //             rte_mempool_put(mbuf_pool, msg);
    //         }

    //  void *recv_msg;
        
    //         if (rte_ring_dequeue(ring, &recv_msg) < 0){
    //             usleep(5);
    //         }
    //         printf("Received: '%s'\n", (char *)recv_msg);
    //         rte_mempool_put(mbuf_pool, recv_msg);




        /* call lcore_hello() on every slave lcore */
        // RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        //         //rte_eal_mp_remote_launch(lcore_hello, NULL, CALL_MASTER);//CALL_MASTER SKIP_MASTER
        //         if(lcore_id!=0)
        //         {
        //             rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
        //         }else
        //         {
        //             rte_eal_remote_launch(another_lcore, NULL, lcore_id);
        //         }               
        // }
        rte_eal_mp_remote_launch(lcore_hello, NULL, SKIP_MASTER); //在此函数中调用rte_eal_remote_launch
        /* call it on master lcore too */
        printf("port num:%d\n",rte_eth_dev_count());
        //printf("ring size:%d\n",rte_ring_count(ring));
        lcore_hello(NULL); 
        printf("ring size:%d\n",rte_ring_count(ring));

        while (rte_ring_count(ring))
        {
            /* code */
            printf("%d---",rte_ring_count(ring));
            de_queue();
        }
        rte_eal_mp_wait_lcore();
       
        rte_ring_free(ring);
        //rte_pktmbuf_free(mbuf_pool);
        printf("bye \n");

    // flags:标识是单消费者/生产者或者多消费者/生产者
    
        // struct rte_ring *ring = rte_ring_create("message_ring",
        //         ring_size, rte_socket_id(), 0x0001);
        // struct rte_mempool *message_pool = rte_mempool_create(
        //         "message_pool", pool_size,
        //         string_size, pool_cache, 0,
        //         NULL, NULL, NULL, NULL,
        //         rte_socket_id(),0x0001);

        // void *msg = NULL;
        // printf("2");
        // if (rte_mempool_get(message_pool, &msg) < 0)
        //     return 0;
        // printf("3");
        // snprintf((char *)msg, string_size, "%s", "helloworld");
        // if (rte_ring_enqueue(ring, msg) < 0) {
        //     rte_mempool_put(message_pool, msg);
        // }
        
        
        //     void *recv_msg;
        
        //     if (rte_ring_dequeue(ring, &recv_msg) < 0){
        //         usleep(5);
        //     }
        
        //     printf("Received: '%s'\n", (char *)recv_msg);
        
        //     rte_mempool_put(message_pool, recv_msg);
        
// //创建
// //单生产单消费，根据自己需要设置
// #define RING_SIZE 131072
 
// struct rte_ring* rte_list = rte_ring_create("CircleRing_1", RING_SIZE,  SOCKET_ID_ANY, RING_F_SC_DEQ | RING_F_SP_ENQ);
//     if (rte_list == NULL)
//         printf("Write to file rte ring create error!\n");
//     else
//         printf("Write to file rte ring create success:%d!\n",m_max_cache_size);
// static int count = 1;
//     if (!rte_list)
//         return;

// //使用，也可以一次入多个，一次取多个
// int enqueueList(char* pbuf)
// {
//     int ret = rte_ring_enqueue(cirList, (void *) pbuf);
//     if (unlikely(ret != 0)) {
//         if(count++%100000 == 0)
//         {
//             count = 1;
//             printf("rte_ring_enqueue error : %d !\n",ret);
//         }
//         delete pbuf;
//         pbuf = NULL;
//         return ret;
//     }
//     return ret;
// }

// //出队，取数据
// #define DEQUEUE_COUNT 64
// void dequeueRing(int count)
// {
// 	strParams* params[DEUEUE_COUNT]
// 	 while (count>0 && unlikely(rte_ring_dequeue_bulk(cirList, (void**)params, DEQUEUE_COUNT )!=0)){
//                 count= (uint16_t)RTE_MIN(rte_ring_count(cirList), DEQUEUE_COUNT );
//             }
//             if (unlikely(count== 0)){
//                 usleep(100);
//                 return;
//             }

//             for(int i = 0; i < count; i++)
//             {  
//                 strParams* param = params[i];
//                 if(!param)
//                     continue;
//                 processParam(param);
//             }
// }

        return 0;
}
