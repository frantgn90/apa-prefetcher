/*
 * Prefetch Filter
 * The main objective of the PF is to decrease redundant prefetch requests, 
 * and to track prefetching accuracy. The PF is a direct-mapped filter that
 * records prefetched cache lines. SPP always checks the PF first, before it
 * issues prefetches.
 */

#include <pf.h>

/* Not exposed functions */

BOOL pf_get_entry(unsigned long long int pf_addr, prefetch_filter_entry_t **set, int *way);

void pf_init()
{
    assert (PF_VALID_SIZE <= sizeof(PF_VALID)<<3);
    assert (PF_TAG_SIZE <= sizeof(PF_TAG)<<3);
    assert (PF_USEFUL_SIZE <= sizeof(PF_USEFUL)<<3);

    int i;
    for (i=0; i<N_PF_ENTRIES; ++i)
    {
        int j;
        for (j=0; j<N_PF_WAYS; ++j)
        {
            PF[i].valid[j]=0;
            PF[i].useful[j]=0;
            PF[i].lru[j]=(1<<PF_LRU_SIZE)-1;
        }
    }
    pf_collisions=0;
    c_total=0;
    c_useful=0;
    alfa=0.5;
}

void pf_remove_entry(unsigned long long pf_addr)
{
    int way;
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, &way);

    if (exists)
    {
        set->valid[way]=0;
        set->useful[way]=0;
    }
}

BOOL pf_exist_entry(unsigned long long pf_addr)
{
    return (pf_get_entry(pf_addr, NULL, NULL));
}

void pf_insert_entry(unsigned long long pf_addr)
{
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, NULL);

    assert(!exists);

    int way=rand()%N_PF_WAYS;

    int j;
    for (j=0; j<N_PF_WAYS; ++j)
    {
        if (!set->valid[j])
        {
            way=j;
            break;
        }
    }

    if (set->valid[way])
    {
        pf_collisions+=1;
    }
    else
    {
        set->valid[way]=1;
    }

    PF_TAG tag = PF_ADDR_TO_TAG(pf_addr);
    set->tag[way]=tag;
    set->useful[way]=0;
}

void pf_increment_total()
{
    PF_AC_CTOTAL prev_total = c_total;
    c_total=INCREMENT(c_total, PF_AC_CTOTAL_SIZE);

    if (prev_total > c_total) /* overflow detection */
    {
        c_total/=c_useful;
        c_useful=1;
    }
}

void pf_increment_useful(unsigned long long pf_addr)
{
    int way;
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, &way);

    if (exists && !set->useful[way])
    {
        set->useful[way]=1;
        c_useful=INCREMENT(c_useful, PF_AC_CUSEFUL_SIZE);
    }
    /*else if(!exists)
    {
        pf_insert_entry(pf_addr);
        exists=pf_get_entry(pf_addr, &set, &way);
        assert(exists);

        set->useful[way]=1;
    }*/
}

double pf_get_alfa()
{
    if (c_total == 0)
        return 0.9;

    double alfa=c_useful/(double)c_total;

    if (alfa >= 1) /* because overflow */
        alfa=0.9;

    return alfa;

}

/* Not exposed functions */
    
BOOL pf_get_entry(unsigned long long int pf_addr, prefetch_filter_entry_t **set, int *way)
{
    int set_i=PF_ADDR_TO_INDEX(pf_addr)%N_PF_ENTRIES;
    PF_TAG tag=PF_ADDR_TO_TAG(pf_addr);
    
    if (set != NULL) *set=&PF[set_i];

    int j;
    for (j=0; j<N_PF_WAYS; ++j)
    {
        if (PF[set_i].valid[j] && PF[set_i].tag[j] == tag)
        {
            if (way != NULL) *way=j;
            return TRUE;
        }
    }
    return FALSE;
}
