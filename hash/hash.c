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
#include <rte_ring.h>
#include <rte_ethdev.h>
//#include <eal_common_thread.h>

#include <rte_hash.h>
#include <rte_jhash.h>

#define HASH_ENTRIES 1024//16384
struct rte_hash* Hash_map;


#define NUM_MBUFS 8191*16
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#define string_size 128
#define ring_size  1024*1024*8

struct rte_mempool *mbuf_pool;  //指向内存池结构的指针变量
struct rte_ring *ring; //无锁队列


struct  five_t
{
    /* data */
    unsigned char src_ip[4];
    unsigned char dst_ip[4];
    unsigned int src_port;
    unsigned int dst_port;
    unsigned int proto;
};

static int
lcore_hello(__attribute__((unused)) void *arg)
{
        unsigned lcore_id;
        int ret;
        lcore_id = rte_lcore_id();
        
        

        struct five_t key={0};
        // {
        //     .src_ip = {1,1,1,1},
        //     .dst_ip = {2,2,2,2},
        //     .src_port = 80,
        //     .dst_port = 88,
        //     .proto = 6
        // }; 
        ret = rte_hash_lookup(Hash_map, &key);
        if(ret<0)
        {
            printf("no find hash_map\n");
        }
        if(rte_hash_add_key_data(Hash_map,&key,lcore_id)<0)
        {
            printf("add_key_data failed\n");
        };
        
        printf("hello from core %u\n", lcore_id);
        return 0;
}

int
main(int argc, char **argv)
{
        int ret;
        unsigned lcore_id; 
        unsigned nb_ports,i; 

        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");

        /* call lcore_hello() on every slave lcore */

    nb_ports = rte_eth_dev_count(); //获取当前有效网口的个数
        if (nb_ports < 2 || (nb_ports & 1))     //如果有效网口数小于2或有效网口数为奇数0，则出错
            rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

        //此函数为rte_mempoll_create()的封装
        mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
            MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    // ring = rte_ring_create("message_ring",
    //             ring_size, rte_socket_id(), 0x0008 | 0x0010);

        //创建hash表
        struct rte_hash_parameters params={0};
        //memset(params,0,sizeof(params));
        params.entries = HASH_ENTRIES;
        params.key_len = sizeof(int);
        params.hash_func = rte_jhash;
        params.hash_func_init_val = 0;
        params.socket_id = rte_socket_id;
        params.name = "five_table_hash";
        params.reserved =0;

        Hash_map = rte_hash_create(&params);
        if(Hash_map==NULL)
        {
            printf("create hash failed\n");
        
        }
        // RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        //         rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
        // }
    
        /* call it on master lcore too */
        //printf("port num:%d\n",rte_eth_dev_count());
        //lcore_hello(NULL); 

        printf("hash entry num:%u\n",rte_hash_count(Hash_map));
        //rte_eal_mp_wait_lcore();
        return 0;
}