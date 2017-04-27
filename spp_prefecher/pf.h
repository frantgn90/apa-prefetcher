#include <stdio.h>
#include <params.h>

struct prefetch_filter_entry
{
    PF_VALID valid;
    PF_TAG tag;
    PF_USEFUL useful;
};
typedef struct prefetch_filter_entry prefetch_filter_entry_t;

prefetch_filter_entry_t PF[N_PF_ENTRIES];

/* Control variables */
PF_AC_CTOTAL c_total;
PF_AC_CUSEFUL c_useful;

/* Debug variables */
int pf_collisions;

/* Exposed functions */
void pf_init();
void pf_remove_entry(PF_TAG tag);
BOOL pf_exist_entry(PF_TAG tag);
void pf_insert_entry(PF_TAG tag);
void pf_set_useful(PF_TAG tag);
void pf_increment_total();
void pf_increment_useful(PF_TAG tag);
double pf_get_alfa();
