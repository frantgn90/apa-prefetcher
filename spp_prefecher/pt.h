#include <stdio.h>
#include <params.h>

#define

struct pattern_table_entry
{
    4BIT_FIELD c_sig;
    4BIT_FIELD c_delta[N_PT_DELTAS_PER_ENTRY];
    7BIT_FIELD delta[N_PT_DELTAS_PER_ENTRY];
};
typedef struct pattern_table_entry pattern_table_entry_t;

patterh_table_entry_t PT[N_PT_ENTRIES];


