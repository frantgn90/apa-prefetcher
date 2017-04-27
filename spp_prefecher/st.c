/*
 * APA Prefetcher
 *
 * Signature table
 * Is designed tu capture memory access patterns within 4KB physical pages, and
 * to compress the previous deltas in taht page into a 12-bit history signature.
 * The ST tracks the 256 most recently accessed apges ,and sotres the last block
 * accessd in each of those pages in order to calculate the delta signature of
 * the current memory access.
 *
 */

#include <st.h>

/* Not exposed functions */

BOOL st_get_entry(16BIT_FIELD tag, signature_table_entry_t *entry);
signature_table_entry_t* st_allocate_entry(16BIT_FIELD tag);
void update_lru_by_touch(signature_table_entry_t *entry);

void st_init()
{
    assert (ST_SIGNATURE_SIZE <= sizeof(ST_SIGNATURE_SIZE));
    assert (ST_TAG_SIZE <= sizeof(ST_TAG_SIZE));
    assert (ST_LAST_OFFSET_SIZE <= sizeof(ST_LAST_OFFSET));
    assert (ST_SIGNATURE_SIZE <= sizeof(ST_SIGNATURE));
    assert (ST_LRU_SIZE <= sizeof(ST_LRU));

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        ST[i].valid=0;
        ST[i].lru=N_ST_ENTRIES-1;
        ST[i].signature=0;
        ST[i].last_offset=0;
    }
}

BOOL st_get_signature(ST_TAG tag, ST_SIGNATURE *signature, ST_LAST_OFFSET *last_offset)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, entry);

    if (exists)
    {
        signature = &entry->signature;
        last_offset = &entry->last_offset;
        update_lru_by_touch(entry);
        return TRUE;
    }
    return FALSE;
}

void st_update(ST_TAG tag, ST_LAST_OFFSET offset)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, entry);

    if (!exists)
    {
        entry = st_allocate_entry(tag);
    }

    PT_DELTA new_delta = offset - entry->last_offset;
    new_delta = LRB_MASK(new_delta,PT_DELTA_SIZE);

    ST_SIGNATURE new_signature = (entry->signature << PT_DELTA_SIZE-1) ^ new_delta;
    entry->signature = LRB_MASK(new_signature, ST_SIGNATURE_SIZE);
    entry->last_offset = LRB_MASK(offset, ST_LAST_OFFSET_SIZE);

    update_lru_by_touch(entry);
}
    
/* Not exposed functions */

BOOL st_get_entry(ST_TAG tag, signature_table_entry_t *entry)
{
    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        if (ST[i].valid && ST[i].tag == tag)
        {
            entry = &ST[i];
            return TRUE;
        }
    }
    return FALSE;
}

signature_table_entry_t* st_allocate_entry(ST_TAG tag)
{
    signature_table_entry_t* least_used_entry;
    ST_LRU max_lru;

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        signature_table_entry_t* entry = &ST[i];
        if (!entry->valid)
        {
            entry->tag=tag;
            entry->signature=0;
            entry->valid=1
            return entry;
        }

        if (entry->lru > max_lru)
        {
            least_used_entry = entry;
            max_lru = entry->lru;
        }
    }

    least_used_entry->tag=tag;
    least_used_entry->last_offset=0; // not sure if needed
    least_used_entry->signature=0;   // not sure if needed

    return least_used_entry;
}

void update_lru_by_touch(signature_table_entry_t *entry)
{
    if (entry->lru==0)
    {
        return;
    }

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        if (ST[i].lru < entry->lru)
        {
            ST[i].lru += 1;
        }
    }
    entry->lru=0;
}
