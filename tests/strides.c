/*
 * stride bench
 * ============
 * Little and simple piece of code that perform accesses to memory at
 * level of cache clock. The strides of the cache blocks accessed can
 * be configured by mean of the STRIDES array. Remember to change also
 * the N_STRIDES
 */

#include <stdio.h>

#define N_VALUES (1<<12) // 4KB
#define N_STRIDE 1
#define BLOCK_SIZE 64 // 64B

#define N_ACCESSES 100000

unsigned int STRIDES[N_STRIDE]={1};

int main()
{
    int VALUES[N_VALUES/sizeof(int)];
    
    int i;
    int stride_i=0;
    int access_i=0;

    int res=0;
    for (i=0; i<N_ACCESSES; ++i)
    {
        int value = VALUES[access_i];

        access_i = ((access_i+STRIDES[stride_i])*(BLOCK_SIZE/sizeof(int)))%N_VALUES;
        stride_i = (stride_i+1)%N_STRIDE;

        VALUES[access_i]=value;
    }

}
