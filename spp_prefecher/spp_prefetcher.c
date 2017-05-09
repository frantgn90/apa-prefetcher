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
int perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr, int ll, unsigned long long int base_addr);


void l2_prefetcher_initialize(int cpu_num)
{
    stats_filtered_pref=0;

    st_init();
    pt_init();
    pf_init();
    ghr_init();
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    pf_increment_useful(addr);
    unsigned int page = (addr>>(PAGE_BLOCK_OFFSETS));
    unsigned int block = LRB_MASK(addr>>(BLOCK_OFFSET_BITS), PAGE_OFFSET_BITS);

    ST_SIGNATURE signature;
    ST_LAST_OFFSET last_block;
    ST_TAG tag=LRB_MASK(page, ST_TAG_SIZE);

    // 1. Who to update at PT
    BOOL exist=st_get_signature(tag, &signature, &last_block);

    if (exist)
    {
        //PT_DELTA new_delta=LRB_MASK(block-last_block, PT_DELTA_SIZE);
        PT_DELTA new_delta=block-last_block;
        if (new_delta == 0)
            return;
        // 2. Update pattern with last access
        pt_update(signature, new_delta);
        // 3. Update signature
        st_update(tag, block);
        // 4. Get last signature to perform the prefetching
        ST_LAST_OFFSET dummy;
        st_get_signature(tag, &signature, &dummy);

        // 5. Get prediction
        PT_DELTA *delta;
        double *confidence;
        unsigned int n_deltas = pt_get_deltas(signature, Tp, &delta, &confidence);

        // 6. Perform prefetch. There is no Pd_prev, so is 1
        int lookahead_level=perform_prefetches(delta, confidence, 
                n_deltas, 1, signature, addr, 1, addr); 

        free(delta);
        free(confidence);
    }
    else
    {
        GHR_CONFIDENCE c;
        BOOL ghr_detect = ghr_get_signature(block, &signature, &c);
        if (ghr_detect)
        {
            st_set_signature(tag, signature, block);
            
            PT_DELTA *delta;
            double *confidence;
            unsigned int n_deltas = pt_get_deltas(signature, Tp, &delta, &confidence);
            int lookahead_level=perform_prefetches(delta, confidence, 
                n_deltas, c, signature, addr, 1, block); 
        }
        else
            st_update(tag, block);
    }
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
    if (evicted_addr && prefetch)
        pf_remove_entry(evicted_addr);
    
    if (!pf_exist_entry(addr))
    {
        pf_insert_entry(addr);
    }
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
    printf("c_useful=%d c_total=%d alfa = %f\n", c_useful, c_total, pf_get_alfa());
    printf("PF stats: filtered=%u repl=%u\n", stats_filtered_pref, pf_collisions);
//  printf("ST stats: used=%u repl=%u\n", st_used_entries(), st_collisions);
//  printf("PT stats: used=%u repl=%u\n", pt_used_entries(), pt_collisions);
//  printf("GHR stats; used=%u repl=%u pred=%u\n", ghr_used_entries(), 
//          ghr_collisions, ghr_predictions);
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

int perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr, int ll, unsigned long long int base_addr)
{

    if (n_deltas == 0)
        return ll-1;

    /* In addition to throttling when Pd falls below the Tp, SPP also stops
     * prefetching if there are not enough L2 read queue resources.
     * Therefore, SPP does not issue prefetches when the number of empty L2 read
     * queue entry becomes less than the number of L1 MSHR
     */

	int remain_rq = L2_READ_QUEUE_SIZE-get_l2_read_queue_occupancy(0);

    if (remain_rq < L2_MSHR_COUNT)
        return ll-1;

    // Confidence modulation
    double alfa = pf_get_alfa();

    double highest_pd = 0;
    int i, highest_pd_i;
    unsigned long long int highest_pd_addr;

    for(i=0; i<n_deltas; ++i)
    {   
        double Pd = confidence[i]*Pd_prev;
        assert (Pd <= 1);

        if (ll > 1) /* Is lookahead */
            Pd=Pd*alfa;

        if (Pd < Tp) /* Threshold filter */
            continue;
        
        int fill_level;
        if (Pd > Tf)
            fill_level=FILL_L2;
        else
            fill_level=FILL_LLC;

        // Prefetching
		unsigned long long int pf_addr = ((addr >> 6) + delta[i]) << 6;
        //unsigned long long int pf_addr = addr+(delta[i]<<BLOCK_OFFSET_BITS);
        BOOL same_page = (ADDR_TO_PAGE(base_addr) == ADDR_TO_PAGE(pf_addr));

        /*
         * Note that SPP does not stop looking even further ahead for
         * more prefetch candidates after coming to the end of a page
         * and updating the GHR.
         */
        if (Pd > highest_pd)
        {
            highest_pd = Pd;
            highest_pd_i = i;
            highest_pd_addr = pf_addr;
        }

        if (!pf_exist_entry(pf_addr) && same_page)
        {

            //if (ADDR_TO_BLOCK(addr) == ADDR_TO_BLOCK(pf_addr))
            //    continue;

            int res=l2_prefetch_line(0, base_addr, pf_addr, fill_level);

            if (res != 1)
            {
                printf("****** WTF ******\n");
            }
            else
            {
                pf_insert_entry(pf_addr);
                pf_increment_total();
            }

        }
        else if (!same_page /*&& ll == 1*/) /* Not lookahead */
        {
            /* 
             * When there is a pretech request that goes beyond the current
             * page, a conventional streaming prefetcher must stop prefetching
             * because it is impossible to predict the next phyisical page number.
             * So we have to update the ghr
             */
            ST_LAST_OFFSET last_offset=ADDR_TO_BLOCK(addr);
            ghr_update(base_signature, Pd, last_offset, delta[i]);
        }
        else if (pf_exist_entry(pf_addr))
            stats_filtered_pref+=1;
    }

    
    /* SPP only generates a single lookahead signature, choosing the
     * candidate with the highest confidence
     */
    int res_ll=ll;
    if (highest_pd > 0)
    {
        //ST_SIGNATURE new_signature = (base_signature << (PT_DELTA_SIZE-1))\
        //    ^ delta[highest_pd_i];
        ST_SIGNATURE new_signature = (base_signature, delta[highest_pd_i]);
        PT_DELTA *new_delta;
        double *new_confidence;
        unsigned int new_n_deltas = pt_get_deltas(new_signature, Tp, 
                &new_delta, &new_confidence);

        res_ll=perform_prefetches(new_delta, new_confidence, new_n_deltas, 
                highest_pd, new_signature, highest_pd_addr, ll+1, base_addr);
    }
    return res_ll;
}
