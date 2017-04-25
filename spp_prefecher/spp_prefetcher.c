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
    if (!cache_hit)
    {
        // Prefetcher's work
        //
        int page_offset = (addr&0xFFF);
        int page = (addr&0xFFFF000)>>12;

        SIGNATURE_FIELD *signature;
        6BIT_FIELD *last_offset;
        BOOL exist=st_get_signature(page, signature, last_offset);

        if (exist)
        {
            new_delta=page_offset-*last_offset;
            pt_update(*signature, new_delta);
            st_update(page, page_offset);
            st_get_signature(page, signature, last_offset);

            7BIT_FIELD *delta;
            double * conficene;
            n_deltas = pt_get_deltas(*signature, Tp, delta, confidence);

            int i;
            for(i=0; i<n_deltas; ++i)
            {   
                int fill_level;
                if (confidence[i] > Tf)
                {
                    fill_level=FILL_L2;
                }
                else
                {
                    fill_level=FILL_LLC;
                }

                // TODO: Calcule prefetch addr
                // TODO: Perform prefetch also in the deph dimension not just witdh
                l2_prefetch_line(0, base_addr, pf_addr, fill_level);
            }
        }
    }
    else 
    {
        // Update Prefetcher Filter
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
