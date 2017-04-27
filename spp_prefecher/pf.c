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
}

void pf_remove_entry(PF_TAG tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    if (entry->tag == tag) // Because this tag could had been preempted
    {
        entry->valid=0;
    }
}

BOOL pf_exist_entry(PF_TAG tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    return entry->valid && entry->tag==tag;
}

void pf_insert_entry(PF_TAG tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];

    // If this assertion fails it means that we will prefetch same
    // location twice. It should be avoided.
    //
    assert (entry->tag != tag);

    if (entry->valid)
    {
        pf_collisions+=1;
    }
    else
    {
        entry->valid=1;
    }

    entry->tag=LRB_MASK(tag, PF_TAG_SIZE);
    entry->useful=0;
}

void pf_set_useful(PF_TAG tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    if (entry->tag==tag)
    {
        entry->useful=1;
    }
}

void pf_increment_total()
{
    c_total=INCREMENT(c_total, PF_AC_CTOTAL_SIZE);
}

void pf_increment_useful(PF_TAG tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    assert (entry->valid==1);
    if (entry->tag == tag)
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
    return c_useful/c_total;
}
