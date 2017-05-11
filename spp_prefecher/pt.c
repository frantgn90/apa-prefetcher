/*
 * APA Prefetcher
 *
 * Pattern table
 * The pattern table is indexed by the history signatures generated by the ST
 * and sotres predicted delta patterns. The PT also estimates the path confidence
 * that a given delta pattern will yeld a useful prefetch.
 * Unlike ST, whose entries corresponds to individual physical pages, each PT
 * entry is shared globally by all pages. So if page A and page B share the same
 * access pattern, they will generate the same signature, which indexes the same
 * PT entry.
 */

#include <pt.h>

/* Not exposed functions */

void pt_normalize_counters(pattern_table_entry_t *entry);

void pt_init()
{
    assert (PT_CSIG_SIZE <= sizeof(PT_CSIG)<<3);
    assert (PT_CDELTA_SIZE <= sizeof(PT_CDELTA)<<3); 
    assert (PT_DELTA_SIZE <= sizeof(PT_DELTA)<<3);
    assert (PT_VALID_SIZE <= sizeof(PT_VALID)<<3);

    int i;
    for (i=0; i<N_PT_ENTRIES; ++i)
    {
        PT[i].c_sig=0;
        int j;
        for (j=0; j<N_PT_DELTAS_PER_ENTRY; ++j)
        {
            PT[i].c_delta[j]=0;
            PT[i].valid[j]=0;
        }
    }
    pt_collisions=0;
}

void pt_update(ST_SIGNATURE signature, PT_DELTA delta)
{
    unsigned int index=signature%N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    BOOL done=FALSE;
    PT_DELTA min_c_delta=(1<<PT_DELTA_SIZE)-1;
    unsigned int min_c_delta_index;
    int invalid_entry_i=-1;

    int j;
    for (j=0; j<N_PT_DELTAS_PER_ENTRY && !done; ++j)
    {
        if (entry->delta[j] == delta && entry->valid[j]) 
        {
            PT_CDELTA prev_cdelta=entry->c_delta[j];
            entry->c_delta[j]=INCREMENT(entry->c_delta[j],PT_CDELTA_SIZE);

            if (prev_cdelta > entry->c_delta[j]) // overflow!
            {
                entry->c_delta[j]=prev_cdelta;
                pt_normalize_counters(entry);
                prev_cdelta=entry->c_delta[j];
                entry->c_delta[j]=INCREMENT(entry->c_delta[j],PT_CDELTA_SIZE);
                assert (prev_cdelta+1 == entry->c_delta[j]);

            }
            done=TRUE;
        }
        else if (entry->c_delta[j] < min_c_delta && entry->valid[j])
        {
            min_c_delta=entry->c_delta[j];
            min_c_delta_index=j;
        }
        else if (!entry->valid[j])
            invalid_entry_i=j;
    }

    if (!done && invalid_entry_i!=-1) // Put it on free entry
    {
        //entry->delta[j]=LRB_MASK(delta,PT_DELTA_SIZE);
        entry->delta[invalid_entry_i]=delta;
        entry->c_delta[invalid_entry_i]=1;
        entry->valid[invalid_entry_i]=1;
        done=TRUE;
    }

    if (!done) // Replace delta with less confident
    {
        //entry->c_sig-=entry->c_delta[min_c_delta_index];
        //entry->delta[min_c_delta_index]=LRB_MASK(delta,PT_DELTA_SIZE);
        entry->delta[min_c_delta_index]=delta;
        entry->c_delta[min_c_delta_index]=1;
        pt_collisions++;
    }

    PT_CSIG prev_csig=entry->c_sig;
    entry->c_sig=INCREMENT(entry->c_sig, PT_CSIG_SIZE);

    if (prev_csig > entry->c_sig) // overflow!
    {
        entry->c_sig=prev_csig;
    }
}

int pt_get_deltas(ST_SIGNATURE signature, double c_threshold, PT_DELTA **delta, double **c)
{
    int index=signature % N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    *delta = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(PT_DELTA));
    *c = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(double));
    int n_deltas=0;

    int i;
    unsigned int total_cdelta=0;
    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
        total_cdelta+=entry->c_delta[i];

    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i])
        {
            //double confidence=entry->c_delta[i]/(double)entry->c_sig;
            double confidence=entry->c_delta[i]/(double)total_cdelta;

            assert (confidence <= 1);
            if (confidence >= c_threshold)
            {
                (*delta)[n_deltas]=entry->delta[i];
                (*c)[n_deltas]=confidence;
                n_deltas++;
            }
        }
    }

    return n_deltas;
}

unsigned int pt_used_entries()
{
    unsigned int result=0;

    int i;
    for (i=0; i<N_PT_ENTRIES; ++i)
    {
        int j;
        for (j=0; j<N_PT_DELTAS_PER_ENTRY; ++j)
        {
            if (PT[i].valid[j])
                result++;
        }
    }
    return result;
}

void pt_normalize_counters(pattern_table_entry_t *entry)
{
    PT_CDELTA min_cdelta=((1<<PT_CDELTA_SIZE)-1);
    int i;
    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i] && entry->c_delta[i] < min_cdelta)
            min_cdelta=entry->c_delta[i];
    }

    if (min_cdelta == 1)
        min_cdelta=2;

    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i])
            if (entry->c_delta[i] > 1)
                entry->c_delta[i]/=min_cdelta;
    }
    entry->c_sig/=min_cdelta;
}
