/*
 * test-st.c
 * Copyright (C) 2017 Juan Francisco Martínez <juan.martinez[AT]bsc[dot]es>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdlib.h>
#include <st.h>

#define N_TAGS 2
#define N_TESTS_PER_TAG 10

int main(int argc, char **argv)
{
    ST_TAG tags[N_TAGS];

    int i;
    for (i=0; i<N_TAGS; ++i)
    {
        int r=rand();
        tags[i]=LRB_MASK(r, ST_TAG_SIZE);
    }

    st_init();

    for (i=0; i<N_TAGS; ++i)
    {
        int j;
        for (j=0; j<N_TESTS_PER_TAG; ++j)
        {
            int r=rand();
            ST_LAST_OFFSET offset=LRB_MASK(r, ST_LAST_OFFSET_SIZE);
            st_update(tags[i], offset);
            
            ST_SIGNATURE read_signature;
            ST_LAST_OFFSET read_last_offset;
            st_get_signature(tags[i], &read_signature, &read_last_offset);

            printf("TAG: %d OFFSET: %d -> SIGNATURE: %d OFFSET: %d\n",
                    tags[i], offset, read_signature, read_last_offset);
        }
    }
}




