#ifndef _PT_H
#define _PT_H

#include <stdio.h>
#include <params.h>

struct pattern_table_entry
{
    PT_CSIG c_sig;
    PT_CDELTA c_delta[N_PT_DELTAS_PER_ENTRY];
    PT_DELTA delta[N_PT_DELTAS_PER_ENTRY];
    PT_VALID valid[N_PT_DELTAS_PER_ENTRY];
};
typedef struct pattern_table_entry pattern_table_entry_t;

pattern_table_entry_t PT[N_PT_ENTRIES];

/* Metrics */
unsigned int pt_collisions;

/* Exposed functions */

void pt_init();
void pt_update(ST_SIGNATURE signature, PT_DELTA delta);
int pt_get_deltas(ST_SIGNATURE signature, double c_threshold, PT_DELTA **delta, double **c);
unsigned int pt_used_entries();

#endif
