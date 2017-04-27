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

void pt_init()
{
    assert (PT_CSIG, sizeof(PT_CSIG_SIZE));
    assert (PT_CDELTA, sizeof(PT_CDELTA_SIZE)); 
    assert (PT_DELTA, sizeof(PT_DELTA_SIZE));
    assert (PT_VALID, sizeof(PT_VALID_SIZE));

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
}

void pt_update(ST_SIGNATURE signature, PT_DELTA delta)
{
    unsigned int index=signature%N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    BOOL done=FALSE;
    PT_DELTA min_c_delta=(1<<PT_DELTA_SIZE)-1;
    unsigned int min_c_delta_index=NULL;

    int j;
    for (j=0; j<N_PT_DELTAS_PER_ENTRY && !done; ++j)
    {
        if (!entry->valid[j])
        {
            entry->delta[j]=MASK(delta,PT_DELTA_SIZE);
            entry->c_delta[j]=1;
            entry->valid=1;
            done=TRUE;
        }
        else if (entry->delta[j] == delta)
        {
            entry->c_delta[j]=INCREMENT(entry->c_delta[j],PT_CDELTA_SIZE);
            done=TRUE;
        }
        else if (entry->delta[j] < min_c_delta)
        {
            min_c_delta=entry->delta[j];
            min_c_delta_index=j;
        }
    }

    if (!done) // Replace delta with less confident
    {
        assert(min_c_delta_index != NULL);
        entry->c_sig-=entry->c_delta[min_c_delta_index];
        entry->delta[min_c_delta_index]=MASK(delta,PT_DELTA_SIZE);
        entry->c_delta[min_c_delta_index]=1;
    }

    entry->c_sig=INCREMENT(entry->c_sig, PT_CSIG_SIZE);
}

int pt_get_deltas(ST_SIGNATURE signature, 
        unsigned double c_threshold, PT_DELTA *delta, unsigned double *c)
{
    int index=signature % N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    delta = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(PT_DELTA));
    c = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(unsigned double));
    int n_deltas=0;

    int i;
    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        unsigned double confidence=entry->c_delta[i]/entry->c_sig;
        if (confidence >= c_threshold)
        {
            delta[n_deltas]=entry->delta[i];
            c[n_deltas]=confidence;
            n_deltas++;
        }
    }

    return n_deltas;
}
