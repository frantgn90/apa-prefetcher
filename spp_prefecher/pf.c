/*
 * Prefetch Filter
 * The main objective of the PF is to decrease redundant prefetch requests, 
 * and to track prefetching accuracy. The PF is a direct-mapped filter that
 * records prefetched cache lines. SPP always checks the PF first, before it
 * issues prefetches.
 */

#include <pf.h>

void pf_init()
{
    assert (PF_VALID_SIZE <= sizeof(PF_VALID)<<3);
    assert (PF_TAG_SIZE <= sizeof(PF_TAG)<<3);
    assert (PF_USEFUL_SIZE <= sizeof(PF_USEFUL)<<3);

    int i;
    for (i=0; i<N_PF_ENTRIES; ++i)
    {
        PF[i].valid=0;
        PF[i].useful=0;
    }
    pf_collisions=0;
    c_total=0;
    c_useful=0;
    alfa=0.5;
}

void pf_remove_entry(unsigned long long pf_addr)
{
    unsigned int index = ADDR_TO_INDEX(pf_addr);
    PF_TAG tag = ADDR_TO_TAG(pf_addr);

    prefetch_filter_entry_t *entry=&PF[index%N_PF_ENTRIES];
    if (entry->tag == tag) // Because this tag could had been preempted
    {
        entry->valid=0;
    }
}

BOOL pf_exist_entry(unsigned long long pf_addr)
{
    unsigned int index = ADDR_TO_INDEX(pf_addr);
    PF_TAG tag = ADDR_TO_TAG(pf_addr);

    prefetch_filter_entry_t *entry=&PF[index%N_PF_ENTRIES];
    return entry->valid && entry->tag==tag;
}

void pf_insert_entry(unsigned long long pf_addr)
{
    unsigned int index = ADDR_TO_INDEX(pf_addr);
    PF_TAG tag = ADDR_TO_TAG(pf_addr);

    prefetch_filter_entry_t *entry=&PF[index%N_PF_ENTRIES];

    if (entry->valid)
    {
        pf_collisions+=1;
    }
    else
    {
        entry->valid=1;
    }

    entry->tag=tag;
    entry->useful=0;
}

void pf_set_useful(unsigned long long pf_addr)
{
    unsigned int index = ADDR_TO_INDEX(pf_addr);
    PF_TAG tag = ADDR_TO_TAG(pf_addr);

    prefetch_filter_entry_t *entry=&PF[index%N_PF_ENTRIES];
    if (entry->tag==tag)
    {
        entry->useful=1;
    }
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
    unsigned int index = ADDR_TO_INDEX(pf_addr);
    PF_TAG tag = ADDR_TO_TAG(pf_addr);

    prefetch_filter_entry_t *entry=&PF[index%N_PF_ENTRIES];

    if (entry->valid && entry->tag == tag)
    {
        if (!entry->useful)
        {
            entry->useful=1;
            c_useful=INCREMENT(c_useful, PF_AC_CUSEFUL_SIZE);
        }
    }
}

double pf_get_alfa()
{
    if (c_total == 0)
        return 0.5;

    double alfa=c_useful/(double)c_total;

    if (alfa >= 1) /* because overflow */
        alfa=0.5;

    return alfa;

}
