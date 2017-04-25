#include <stdio.h>
#include <params.h>

struct prefetch_filter_entry
{
    1BIT_FIELD valid;
    6BIT_FIELD tag;
    1BIT_FIELD useful;
}
typedef struct prefetch_filter_entry prefetch_filter_entry_t;

prefetch_filter_entry_t PF[N_PF_ENTRIES];

/* Control variables */
int c_total;
int c_useful;

/* Debug variables */
int pf_collisions;

/* Exposed functions */
void pf_init();
void pf_remove_entry(6BIT_FIELD tag);
BOOL pf_exist_entry(6BIT_FIELD tag);
void pf_insert_entry(6BIT_FIELD tag);
void pf_set_useful(6BIT_FIELD tag);
void pf_increment_total();
void pf_increment_useful(6BIT_FIELD tag);
