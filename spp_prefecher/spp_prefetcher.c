//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"

#include <st.h>
#include <pt.h>
#include <pf.h>
#include <ghr.h>
#include <params.h>

void l2_prefetcher_initialize(int cpu_num)
{
    st_init();
    pt_init();
    pf_init();
    //ghr_init();
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    unsigned long long int page = (addr>>PAGE_ADDR_BITS);
    unsigned long long int block = LRB_MASK(addr>>(PAGE_ADDR_BITS-BLOCK_ADDR_BITS), BLOCK_ADDR_BITS); 

    ST_SIGNATURE signature;
    ST_LAST_OFFSET last_block;
    ST_TAG tag=LRB_MASK(page, ST_TAG_SIZE);

    BOOL exist=st_get_signature(tag, &signature, &last_block);

    if (exist)
    {
        PT_DELTA new_delta=LRB_MASK(block-last_block, PT_DELTA_SIZE);
        pt_update(signature, new_delta);
        st_update(tag, block);
        st_get_signature(tag, &signature, &last_block /* dummy */);

        PT_DELTA *delta;
        double *confidence;
        unsigned int n_deltas = pt_get_deltas(signature, Tp, &delta, &confidence);
        if (n_deltas == 0)
            return;

#ifdef LOOKAHEAD
        int i;
        for(i=0; i<n_deltas; ++i)
        {   
            int fill_level;
            if (confidence[i] > Tf)
                fill_level=FILL_L2;
            else
                fill_level=FILL_LLC;

            unsigned long long int pf_addr = addr+delta[i];
            l2_prefetch_line(0, addr, pf_addr, fill_level);
        }
#else
        int i;
        double max_confidence=0;
        int max_confidence_i;
        for (i=0; i<n_deltas; ++i)
        {
            if (confidence[i] > max_confidence)
            {
                max_confidence=confidence[i];
                max_confidence_i=i;
            }
        }
        int fill_level;
        if (max_confidence > Tf)
            fill_level=FILL_L2;
        else
            fill_level=FILL_LLC;

        unsigned long long int pf_addr = addr+(delta[max_confidence_i]<<BLOCK_ADDR_BITS);
        l2_prefetch_line(0, addr, pf_addr, fill_level);
#endif
        free(delta);
        free(confidence);
    }
    else
    {
        // Just update st_update with the new signature
        st_update(tag, block);
    }
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
  printf("Prefetcher heartbeat stats\n");
  printf("ST used entries: %u\n", st_used_entries());
  printf("ST replacements: %u\n", st_collisions);
  printf("PT used entries: %u\n", pt_used_entries());
  printf("PT replacements: %u\n", pt_collisions);
  //printf("PF used entries: %d", pf_used_entries());
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  printf("Prefetcher final stats\n");
}
