// General

#include <assert.h>

/* Utilities */
#define INCREMENT(var, size) (var+1 % (1<<size))
#define LRB_MASK(var, size) (var & ((1<<size)-1))

typedef unsigned int BOOL;
#define TRUE  1
#define FALSE 0

/* Typedef for field sizes */
typedef unsigned short ST_VALID;
typedef unsigned short ST_TAG;
typedef unsigned short ST_LAST_OFFSET;
typedef unsigned short ST_SIGNATURE;
typedef unsigned short ST_LRU;

typedef unsigned short PT_CSIG;
typedef unsigned short PT_CDELTA;
typedef unsigned short PT_DELTA;
typedef unsigned short PT_VALID;

typedef unsigned short PF_VALID;
typedef unsigned short PF_TAG;
typedef unsigned short PF_USEFUL;
typedef unsigned short PF_AC_CTOTAL;
typedef unsigned short PF_AC_USEFUL;

typedef unsigned short GHB_SIGNATURE;
typedef unsigned short GHB_CONFIDENCE;
typedef unsigned short GHB_LAST_OFFSET;
typedef unsigned short GHB_DELTA;

typedef 12BIT_FIELD SIGNATURE_FIELD

/* Data sizes */
#define ST_VALID_SIZE 1
#define ST_TAG_SIZE   16
#define ST_LAST_OFFSET_SIZE 6
#define ST_SIGNATURE_SIZE 12
#define ST_LRU_SIZE 6

#define PT_CSIG_SIZE 4
#define PT_CDELTA_SIZE 4
#define PT_DELTA_SIZE 7
#define PT_VALID_SIZE 1

#define PF_VALID_SIZE 1
#define PF_TAG_SIZE 6
#define PF_USEFUL_SIZE 1
#define PF_AC_CTOTAL_SIZE 10
#define PF_AC_CUSEFUL_SIZE 10

#define GHR_SIGNATURE_SIZE 12
#define GHR_CONFIDENCE_SIZE 8
#define GHR_LAST_OFFSET_SIZE 6
#define GHR_DELTA_SIZE 7


/* Structure sizes */
#define N_ST_ENTRIES 256
#define N_PT_ENTRIES 512
#define N_PT_DELTAS_PER_ENTRY 4
#define N_PF_ENTRIES 1024

/* Threshold */
#define Tp 0.7 // Confident threshold to prefetch
#define Tf 0.5 // Confident threshold  to prefetch to L2 or LLC
