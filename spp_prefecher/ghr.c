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

    ghr_collisions=0;
    ghr_predictions=0;
}

void ghr_update(ST_SIGNATURE signature, GHR_CONFIDENCE confidence,
        ST_LAST_OFFSET last_offset, PT_DELTA delta)
{
    int index=signature%N_GHR_ENTRIES;

    if (GHR[index].valid)
        ghr_collisions++;

    GHR[index].signature=signature;
    GHR[index].confidence=confidence;
    GHR[index].last_offset=last_offset;
    GHR[index].delta=delta;
    GHR[index].valid=1;
}

BOOL ghr_get_signature(ST_LAST_OFFSET offset, ST_SIGNATURE *signature, GHR_CONFIDENCE *c)
{
    int i;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        if (GHR[i].valid)
        {
            ST_LAST_OFFSET ghr_offset=GHR[i].last_offset+GHR[i].delta-(1<<BLOCK_OFFSET_BITS);
            if (ghr_offset == offset)
            {
                *signature = generate_signature(GHR[i].signature, GHR[i].delta);
                *c = GHR[i].confidence;
                ghr_predictions++;
                return TRUE;
            }
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
