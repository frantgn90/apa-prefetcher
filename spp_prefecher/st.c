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
    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        ST[i].valid=0;
        ST[i].lru=N_ST_ENTRIES-1;
        ST[i].signature=0;
        ST[i].last_offset=0;
    }
}

BOOL st_get_signature(16BIT_FIELD tag, SIGNATURE_FIELD *signature)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, entry);

    if (exists)
    {
        signature = &entry->signature;
        update_lru_by_touch(entry);
        return TRUE;
    }
    return FALSE;
}

void st_update(16BIT_FIELD tag, 6BIT_FIELD offset)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, entry);

    if (!exists)
    {
        entry = st_allocate_entry(tag);
    }

    4BIT_FIELD new_delta = offset-entry->last_offset;
    entry->signature = (entry->signature << 3) ^ new_delta;
    entry->last_offset = offset;

    update_lru_by_touch(entry);
}
    
/* Not exposed functions */

BOOL st_get_entry(16BIT_FIELD tag, signature_table_entry_t *entry)
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

signature_table_entry_t* st_allocate_entry(16BIT_FIELD tag)
{
    signature_table_entry_t* least_used_entry
    6BIT_FIELD max_lru;

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
