#include <stdio.h>

#define N_VALUES (1<<12) // 4KB
#define N_STRIDE 1

#define N_ACCESSES 1000

unsigned int STRIDES[N_STRIDE]={1};

int main()
{
    int VALUES[N_VALUES];
    
    int i;
    int stride_i=0;
    int access_i=0;

    for (i=0; i<N_ACCESSES; ++i)
    {
        int value = VALUES[access_i];

        access_i = (access_i+STRIDES[stride_i])%N_VALUES;
        stride_i = (stride_i+1)%N_STRIDE;
    }

}
