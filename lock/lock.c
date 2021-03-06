



#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
//referecne https://github.com/divfor/atomic_hash/blob/master/src/atomic_hash.c
static int lock;
static int count = 0;
#define atomic_cas(dst, old, new) __sync_bool_compare_and_swap((dst), (old), (new))
#define atomic_lock(ptr)\
while(!atomic_cas(ptr,0,1))
#define atomic_unlock(ptr)\
while(!atomic_cas(ptr,1,0))

void *add_func(void *arg)
{
        int i=0;
        for(i=0;i<10000;++i){
                atomic_lock(&lock);
                count++;
                atomic_unlock(&lock);
        }
        return NULL;
}

void *sub_func(void *arg)
{
        int i=0;
        for(i=0;i<10000;++i){
                atomic_lock(&lock);
                count++;
                atomic_unlock(&lock);
        }
        return NULL;
}
int main()
{
        pthread_t id[20];
        int i = 0;

        for(i=0;i<20;++i){
                pthread_create(&id[i],NULL,add_func,NULL);
        }

        for(i=0;i<20;++i){
                pthread_join(id[i],NULL);
        }

        pthread_t sub_id[10];
        for(i=0;i<10;++i){
                pthread_create(&sub_id[i],NULL,sub_func,NULL);
        }

        for(i=0;i<10;++i){
                pthread_join(sub_id[i],NULL);
        }
         add_func(NULL);
         printf("%d\n",count);
        return 0;
}
