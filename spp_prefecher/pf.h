#ifndef _PF_H
#define _PF_H

#include <stdio.h>
#include <params.h>

#define ADDR_TO_INDEX(pf_addr)\
    (LRB_MASK(pf_addr>>(PAGE_BLOCK_OFFSETS), N_PF_ADDR_BITS))

#define ADDR_TO_TAG(pf_addr)\
    (LRB_MASK(pf_addr>>(BLOCK_OFFSET_BITS), PF_TAG_SIZE))

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
double alfa;

/* Debug variables */
int pf_collisions;

/* Exposed functions */
void pf_init();
void pf_remove_entry(unsigned long long int pf_addr);
BOOL pf_exist_entry(unsigned long long int pf_addr);
void pf_insert_entry(unsigned long long int  pf_addr);
void pf_set_useful(unsigned long long int pf_addr);
void pf_increment_total();
void pf_increment_useful(unsigned long long int pf_addr);
double pf_get_alfa();

#endif
