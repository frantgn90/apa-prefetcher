//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"

#include "st.h"
#include "pt.h"
#include "pf.h"
#include "ghr.h"
#include "params.h"

void l2_prefetcher_initialize(int cpu_num)
{
    st_init();
    pt_init();
    pf_init();
    //ghr_init();
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    unsigned long long int page_offset = (addr&0xFFF); // 4KB pages
    unsigned long long int page = (addr>>12);

    SIGNATURE_FIELD *signature;
    6BIT_FIELD *last_offset;
    16BIT_FIELD tag = page & ((1<<ST_TAG_SIZE)-1)
    BOOL exist=st_get_signature(tag, signature, last_offset);

    if (exist)
    {
        new_delta=page_offset-*last_offset;
        pt_update(*signature, new_delta);
        st_update(tag, page_offset);
        st_get_signature(page, signature, last_offset);

        7BIT_FIELD *delta;
        double * conficene;
        n_deltas = pt_get_deltas(*signature, Tp, delta, confidence);

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
            l2_prefetch_line(0, base_addr, pf_addr, fill_level);
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

        l2_prefetch_line(0, base_addr, pf_addr, fill_level);
#endif
    }
    else
    {
        // Just update st_update with the new signature
        st_update(page, page_offset);
    }
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
  printf("Prefetcher heartbeat stats\n");
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  printf("Prefetcher final stats\n");
}
