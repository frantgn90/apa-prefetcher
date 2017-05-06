#include <stdio.h>
#include <params.h>

struct ghr_table_entry
{
    ST_SIGNATURE signature;
    ST_LAST_OFFSET last_offset;
    PT_DELTA delta;
    GHR_CONFIDENCE confidence;
    GHR_VALID valid;

};
typedef struct ghr_table_entry ghr_table_entry_t;

ghr_table_entry_t GHR[N_GHR_ENTRIES];

/* Metrics */


/* Exposed functions */

void ghr_init();
void ghr_update(ST_SIGNATURE signature, GHR_CONFIDENCE confidence, ST_LAST_OFFSET last_offset, PT_DELTA delta);
BOOL ghr_get_signature(ST_LAST_OFFSET offset, ST_SIGNATURE &signature);
int ghr_used_entries();
