#ifndef _ST_H
#define _ST_H

#include <stdio.h>
#include <params.h>

struct signature_table_entry
{
   ST_VALID valid;
   ST_TAG tag;
   ST_LAST_OFFSET last_offset;
   ST_SIGNATURE signature;
   ST_LRU lru;
};
typedef struct signature_table_entry signature_table_entry_t;

signature_table_entry_t ST[N_ST_ENTRIES];

/* Metrics */

unsigned int st_collisions;

/* Exposed functions */

void st_init();
BOOL st_get_signature(ST_TAG tag, ST_SIGNATURE *signature, ST_LAST_OFFSET *last_offset);
void st_update(ST_TAG tag, ST_LAST_OFFSET offset);
unsigned int st_used_entries();
void st_set_signature(ST_TAG tag, ST_SIGNATURE signature, ST_LAST_OFFSET last_offset);
ST_SIGNATURE generate_signature(ST_SIGNATURE signature, PT_DELTA delta);

#endif
