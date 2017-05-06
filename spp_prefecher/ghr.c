#include <ghr.h>

void ghr_init()
{
    assert(ST_SIGNATURE_SIZE <= sizeof(ST_SIGNATURE)<<3);
    assert(ST_LAST_OFFSET_SIZE <= sizeof(ST_LAST_OFFSET)<<3);
    assert(PT_DELTA_SIZE <= sizeof(PT_DELTA)<<3);
    assert(GHR_CONFIDENCE_SIZE <= sizeof(GHR_CONFIDENCE)<<3);

    int i;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        GHR[i].valid=0;
    }
}

void ghr_update(ST_SIGNATURE signature, GHR_CONFIDENCE confidence,
        ST_LAST_OFFSET last_offset, PT_DELTA delta)
{
    assert (last_offset + delta >= (1<<BLOCK_OFFSET_BITS));

    int index=signature%N_GHR_ENTRIES;

    GHR[index].signature=signature;
    GHR[index].confidence=confidence;
    GHR[index].last_offset=last_offset;
    GHR[index].delta=delta;
}

BOOL ghr_get_signature(ST_LAST_OFFSET offset, ST_SIGNATURE &signature)
{
    int i;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        ST_LAST_OFFSET_SIZE ghr_offset=GHR[i].last_offset\
                                   +GHR[i].delta-(1<<BLOCK_OFFSET_BITS);
        if (ghr_offset == offset)
        {
            *signature = (GHR[i].signature<<(PT_DELTA_SIZE-1))^GHR[i].delta;
            return TRUE;
        }
    }
    return FALSE;
}

int ghr_used_entries()
{
    int i, result=0;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        if (GHR[i].valid) 
            result++;
    }
    return result;
}
