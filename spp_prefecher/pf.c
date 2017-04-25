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
    int i;
    for (i=0; i<N_PF_ENTRIES; ++i)
    {
        PF[i].valid=0;
        PF[i].useful=0;
    }
    pf_preemptions=0;
    c_total=0;
    c_useful=0;
}

void pf_remove_entry(6BIT_FIELD tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    if (entry->tag == tag) // Because this tag could had been preempted
    {
        entry->valid=0;
    }
}

BOOL pf_exist_entry(6BIT_FIELD tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    return entry->valid && entry->tag==tag;
}

void pf_insert_entry(6BIT_FIELD tag)
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

    entry->tag=tag;
    entry->useful=0;
}

void pf_set_useful(6BIT_FIELD tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    if (entry->tag==tag)
    {
        entry->useful=1;
    }
}

void pf_increment_total()
{
    c_total+=1;
}

void pf_increment_useful(6BIT_FIELD tag)
{
    prefetch_filter_entry_t *entry=&PF[tag%N_PF_ENTRIES];
    assert (entry->valid==1);
    if (entry->tag == tag)
    {
        if (!entry->useful)
        {
            entry->useful=1;
            c_useful+=1;
        }
    }
}
