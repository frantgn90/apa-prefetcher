#include <stdio.h>
#include <params.h>

#define

struct pattern_table_entry
{
    4BIT_FIELD c_sig;
    4BIT_FIELD c_delta[N_PT_DELTAS_PER_ENTRY];
    7BIT_FIELD delta[N_PT_DELTAS_PER_ENTRY];
    1BIT_FIELD valid[N_PT_DELTAS_PER_ENTRY];
};
typedef struct pattern_table_entry pattern_table_entry_t;

patterh_table_entry_t PT[N_PT_ENTRIES];

/* Exposed functions */

void pt_init();
void pt_update(SIGNATURE_FIELD signature, 7BIT_FIELD delta);
int  pt_get_deltas(SIGNATURE_FIELD signature, double c_threshold,
        7BIT_FIELD *delta, double *c);



