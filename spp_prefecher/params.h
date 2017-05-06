#ifndef __PARAMS_H
#define __PARAMS_H

#include <stdlib.h>
#include <assert.h>

/* Utilities */
#define INCREMENT(var, size) ((var+1) & ((1<<size)-1))
#define LRB_MASK(var, size) (var & ((1<<size)-1))
#define ADDR_TO_PAGE(addr) (addr>>PAGE_BLOCK_OFFSETS)

#define TRUE  1
#define FALSE 0

typedef unsigned int BOOL;

/* Typedef for field sizes */
typedef unsigned short ST_VALID;
typedef unsigned short ST_TAG;
typedef unsigned short ST_LAST_OFFSET;
typedef unsigned short ST_SIGNATURE;
typedef unsigned short ST_LRU;

typedef unsigned short PT_CSIG;
typedef          short PT_CDELTA;
typedef unsigned short PT_DELTA;
typedef unsigned short PT_VALID;

typedef unsigned short PF_VALID;
typedef unsigned short PF_TAG;
typedef unsigned short PF_USEFUL;
typedef unsigned short PF_AC_CTOTAL;
typedef unsigned short PF_AC_CUSEFUL;

typedef unsigned short GHR_CONFIDENCE;
typedef unsigned short GHR_VALID;

/* Data sizes */
#define PAGE_OFFSET_BITS 6 // 4KB / 64B = 64 blocks/page => 6 bits
#define BLOCK_OFFSET_BITS 6 // 64B => 6 bits
#define PAGE_BLOCK_OFFSETS (PAGE_OFFSET_BITS+BLOCK_OFFSET_BITS)

#define ST_VALID_SIZE 1
#define ST_TAG_SIZE   16
#define ST_LAST_OFFSET_SIZE 6
#define ST_SIGNATURE_SIZE 12
#define ST_LRU_SIZE 6

#define PT_CSIG_SIZE 6
#define PT_CDELTA_SIZE 4
#define PT_DELTA_SIZE 7
#define PT_VALID_SIZE 1

#define PF_VALID_SIZE 1
#define PF_TAG_SIZE 6
#define PF_USEFUL_SIZE 1
#define PF_AC_CTOTAL_SIZE 10
#define PF_AC_CUSEFUL_SIZE 10

#define GHR_CONFIDENCE_SIZE 8
#define GHR_VALID_SIZE 1

/* Structure sizes */
#define N_ST_ENTRIES 256
#define N_PT_ENTRIES 512
#define N_PT_DELTAS_PER_ENTRY 4
#define N_PF_ENTRIES 1024
#define N_PF_ADDR_BITS 10 // need 10 for 1024, pf is direct mapped
#define N_GHR_ENTRIES 8

/* Threshold */
#define Tp 0.5 // Confident threshold to prefetch
#define Tf 0.75 // Confident threshold  to prefetch to L2 or LLC

/* stats */
unsigned int stats_filtered_pref;

#endif
