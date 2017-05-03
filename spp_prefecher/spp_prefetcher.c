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


/****** Helpers prototype *******/
void perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr);


void l2_prefetcher_initialize(int cpu_num)
{
    st_init();
    pt_init();
    pf_init();
    //ghr_init();
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    unsigned long long int page = (addr>>(PAGE_OFFSET_BITS+BLOCK_OFFSET_BITS));
    unsigned long long int block = LRB_MASK(addr>>(BLOCK_OFFSET_BITS), BLOCK_OFFSET_BITS);

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

        // There is no Pd_prev
        perform_prefetches(delta, confidence, n_deltas, 1, signature, addr); 

        free(delta);
        free(confidence);
    }
    else
    {
        // Just update st_update with the new signature
        st_update(tag, block);
    }

    pf_increment_useful(addr);
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
    pf_remove_entry(evicted_addr);
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

/******* Helpers ********/

void perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr)
{
    if (n_deltas == 0)
        return;

    /* In addition to throttling when Pd falls below the Tp, SPP also stops
     * prefetching if there are not enough L2 read queue resources.
     * Therefore, SPP does not issue prefetches when the number of empty L2 read
     * queue entry becomes less than the number of L1 MSHR
     */

    int l2_read_queue_occ = get_l2_read_queue_occupancy(0);

    if (l2_read_queue_occ < (3/4)*L2_READ_QUEUE_SIZE)
        return;

    // Confidence modulation
    double alfa = pf_get_alfa();

    int i;
    double highest_pd = 0;
    int highest_pd_i;
    unsigned long long int highest_pd_addr;

    for(i=0; i<n_deltas; ++i)
    {   
        double Pd = confidence[i]*Pd_prev;
        if (alfa*Pd < Tp)
            continue;
        
        int fill_level;
        if (Pd > Tf)
            fill_level=FILL_L2;
        else
            fill_level=FILL_LLC;

        // Prefetching
        unsigned long long int pf_addr = addr+(delta[i]<<BLOCK_OFFSET_BITS);
        BOOL same_page = (ADDR_TO_PAGE(addr) == ADDR_TO_PAGE(pf_addr));

        if (!pf_exist_entry(pf_addr) && same_page)
        {
            if (Pd > highest_pd)
            {
                highest_pd = Pd;
                highest_pd_i = i;
                highest_pd_addr = pf_addr;
            }

            l2_prefetch_line(0, addr, pf_addr, fill_level);
            pf_insert_entry(pf_addr);
            pf_increment_total();
        }
        else if (!same_page)
        {
            // TODO: GHR Time!
        }
    }

    /* SPP only generates a single lookahead signature, choosing the
     * candidate with the highest confidence
     */
    if (highest_pd > 0)
    {
        ST_SIGNATURE new_signature = (base_signature << (PT_DELTA_SIZE-1))\
            ^ delta[highest_pd_i];
        PT_DELTA *new_delta;
        double *new_confidence;
        unsigned int new_n_deltas = pt_get_deltas(new_signature, Tp, 
                &new_delta, &new_confidence);

        perform_prefetches(new_delta, new_confidence, new_n_deltas, highest_pd,
                new_signature, highest_pd_addr);
    }
}
