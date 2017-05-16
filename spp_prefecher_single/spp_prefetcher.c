//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"


/********************************************/
/**************** PARAMETERS ****************/
/********************************************/

#include <stdlib.h>
#include <assert.h>

/* Utilities */
#define INCREMENT(var, size) ((var+1) & ((1<<size)-1))
#define LRB_MASK(var, size) (var & ((1<<size)-1))
#define SLRB_MASK(var, s1, s2) ((((1<<(s2-s1))-1)*(var&(1<<(s1-1))))<<s1|LRB_MASK(var,s1))
#define ADDR_TO_PAGE(addr) (addr>>PAGE_BLOCK_OFFSETS)
#define ADDR_TO_BLOCK(addr) LRB_MASK(addr>>BLOCK_OFFSET_BITS, PAGE_OFFSET_BITS)

#define TRUE  1
#define FALSE 0

typedef unsigned int BOOL;

/* Typedef for field sizes */
typedef unsigned short ST_VALID;
typedef unsigned short ST_TAG;
typedef unsigned short ST_LAST_OFFSET;
typedef unsigned int   ST_SIGNATURE;
typedef unsigned short ST_LRU;

typedef unsigned int   PT_CSIG;
typedef unsigned int   PT_CDELTA;
typedef          short PT_DELTA;
typedef unsigned short PT_VALID;
typedef unsigned short PT_LRU;

typedef unsigned short PF_VALID;
typedef unsigned short PF_TAG;
typedef unsigned short PF_USEFUL;
typedef unsigned int   PF_AC_CTOTAL;
typedef unsigned int   PF_AC_CUSEFUL;
typedef unsigned short PF_LRU;

typedef         double GHR_CONFIDENCE;
typedef unsigned short GHR_VALID;

/* Data sizes */
#define PAGE_OFFSET_BITS 6 // 4KB / 64B = 64 blocks/page => 6 bits
#define BLOCK_OFFSET_BITS 6 // 64B => 6 bits
#define PAGE_BLOCK_OFFSETS (PAGE_OFFSET_BITS+BLOCK_OFFSET_BITS)

#define ST_VALID_SIZE 1
#define ST_TAG_SIZE   16
#define ST_LAST_OFFSET_SIZE 6
#define ST_SIGNATURE_SIZE 12
#define ST_LRU_SIZE 8
#define ST_SIGNATURE_SHIFT 3

#define PT_CSIG_SIZE 12
#define PT_CDELTA_SIZE 12
#define PT_DELTA_SIZE 7
#define PT_VALID_SIZE 1

#define PF_VALID_SIZE 1
#define PF_TAG_SIZE 6
#define PF_USEFUL_SIZE 1
#define PF_AC_CTOTAL_SIZE 30
#define PF_AC_CUSEFUL_SIZE 30
#define PF_LRU_SIZE 4 

#define GHR_CONFIDENCE_SIZE 8
#define GHR_VALID_SIZE 1

/* Structure sizes */
#define N_ST_ENTRIES 256
#define N_PT_ENTRIES 1024
#define N_PT_DELTAS_PER_ENTRY 8
#define N_PF_ENTRIES 512
#define N_PF_WAYS 16
#define N_PF_ADDR_BITS 10// need 10 for 1024, pf is direct mapped
#define N_GHR_ENTRIES 16

/* Threshold */
#define Tp 0.25 // Confident threshold to prefetch
#define Tf 0.9 // Confident threshold  to prefetch to L2 or LLC

/* stats */
unsigned int stats_filtered_pref;
unsigned int total_lookahead_depth;
unsigned int n_lookahead_depth;

/*****************************************/
/**************** HEADERS ****************/
/*****************************************/
/***** ST *****/
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

/***** PT *****/
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

/***** PF *****/
#define PF_ADDR_TO_INDEX(pf_addr)\
    (LRB_MASK(pf_addr>>(PAGE_BLOCK_OFFSETS), N_PF_ADDR_BITS))

#define PF_ADDR_TO_TAG(pf_addr)\
    (LRB_MASK(pf_addr>>(BLOCK_OFFSET_BITS), PF_TAG_SIZE))

struct prefetch_filter_entry
{
    PF_TAG tag[N_PF_WAYS];
    PF_USEFUL useful[N_PF_WAYS];
    PF_VALID valid[N_PF_WAYS];
    PF_LRU lru[N_PF_WAYS];
    //PF_TAG tag;
    //PF_USEFUL useful;
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

/***** GHR *****/
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

unsigned int ghr_collisions;
unsigned int ghr_predictions;

/* Exposed functions */
void ghr_init();
void ghr_update(ST_SIGNATURE signature, GHR_CONFIDENCE confidence, ST_LAST_OFFSET last_offset, PT_DELTA delta);
BOOL ghr_get_signature(ST_LAST_OFFSET offset, ST_SIGNATURE *signature, GHR_CONFIDENCE *c);
int ghr_used_entries();


/***************************************************/
/**************** MODULES FUNCTIONS ****************/
/***************************************************/
/***** ST *****/
/* Not exposed functions */

BOOL st_get_entry(ST_TAG tag, signature_table_entry_t **entry);
signature_table_entry_t* st_allocate_entry(ST_TAG tag);
void update_lru_by_touch(signature_table_entry_t *entry);

void st_init()
{
    assert (ST_SIGNATURE_SIZE <= sizeof(ST_SIGNATURE)<<3);
    assert (ST_TAG_SIZE <= sizeof(ST_TAG)<<3);
    assert (ST_LAST_OFFSET_SIZE <= sizeof(ST_LAST_OFFSET)<<3);
    assert (ST_LRU_SIZE <= sizeof(ST_LRU)<<3);

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        ST[i].valid=0;
        ST[i].lru=N_ST_ENTRIES-1;
        ST[i].signature=0;
        ST[i].last_offset=0;
    }
    st_collisions=0;
}

BOOL st_get_signature(ST_TAG tag, ST_SIGNATURE *signature, ST_LAST_OFFSET *last_offset)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, &entry);

    if (exists)
    {
        if (entry->signature == 0) // There is not history yet
            return FALSE;

        *signature = entry->signature;
        *last_offset = entry->last_offset;
        update_lru_by_touch(entry);
        return TRUE;
    }
    return FALSE;
}

void st_set_signature(ST_TAG tag, ST_SIGNATURE signature, ST_LAST_OFFSET last_offset)
{
    signature_table_entry_t *entry;
    BOOL exists=st_get_entry(tag, &entry);

    if (!exists)
    {
        entry=st_allocate_entry(tag);
    }
    entry->signature=signature;
    entry->last_offset=last_offset;
    update_lru_by_touch(entry);
}


void st_update(ST_TAG tag, ST_LAST_OFFSET offset)
{
    signature_table_entry_t *entry;
    BOOL exists = st_get_entry(tag, &entry);

    if (!exists)
    {
        entry = st_allocate_entry(tag);
        entry->signature = 0;
    }
    else
    {
        PT_DELTA new_delta = offset - entry->last_offset;
        ST_SIGNATURE new_signature = generate_signature(entry->signature, new_delta);
        entry->signature = LRB_MASK(new_signature, ST_SIGNATURE_SIZE);
    }
    entry->last_offset = LRB_MASK(offset, ST_LAST_OFFSET_SIZE);
    update_lru_by_touch(entry);
}

ST_SIGNATURE generate_signature(ST_SIGNATURE signature, PT_DELTA delta)
{
    signature = signature << ST_SIGNATURE_SHIFT;

    // Signature LOW-PART
    ST_SIGNATURE lp_signature = signature ^ delta;
    lp_signature = LRB_MASK(lp_signature, PT_DELTA_SIZE);

    // Aignature HIGH-PART
    ST_SIGNATURE mask = ((1<<ST_SIGNATURE_SIZE)-1)&((1<<PT_DELTA_SIZE)-1)<<PT_DELTA_SIZE;
    signature = signature & mask;

    // Result
    return (signature | lp_signature);

}

unsigned int st_used_entries()
{
    unsigned int result=0;

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        if (ST[i].valid)
            result++;
    }
    return result;
}
    
/* Not exposed functions */

BOOL st_get_entry(ST_TAG tag, signature_table_entry_t **entry)
{
    int i;
    *entry=NULL;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        if (ST[i].valid && ST[i].tag == tag)
        {
            *entry = &ST[i];
            return TRUE;
        }
    }
    return FALSE;
}

signature_table_entry_t* st_allocate_entry(ST_TAG tag)
{
    signature_table_entry_t* least_used_entry;
    ST_LRU max_lru=0;

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        signature_table_entry_t* entry = &ST[i];
        if (!entry->valid)
        {
            entry->tag=tag;
            entry->signature=0;
            entry->valid=1;
            return entry;
        }

        if (entry->lru > max_lru)
        {
            least_used_entry = entry;
            max_lru = entry->lru;
        }
    }

    least_used_entry->tag=tag;
    least_used_entry->last_offset=0; // not sure if needed
    least_used_entry->signature=0;   // not sure if needed
    st_collisions++;

    return least_used_entry;
}

void update_lru_by_touch(signature_table_entry_t *entry)
{
    if (entry->lru==0)
    {
        return;
    }

    int i;
    for (i=0; i<N_ST_ENTRIES; ++i)
    {
        if (ST[i].lru < entry->lru)
        {
            ST[i].lru += 1;
        }
    }
    entry->lru=0;
}

/***** PT *****/
/* Not exposed functions */

void pt_normalize_counters(pattern_table_entry_t *entry);

void pt_init()
{
    assert (PT_CSIG_SIZE <= sizeof(PT_CSIG)<<3);
    assert (PT_CDELTA_SIZE <= sizeof(PT_CDELTA)<<3); 
    assert (PT_DELTA_SIZE <= sizeof(PT_DELTA)<<3);
    assert (PT_VALID_SIZE <= sizeof(PT_VALID)<<3);

    int i;
    for (i=0; i<N_PT_ENTRIES; ++i)
    {
        PT[i].c_sig=0;
        int j;
        for (j=0; j<N_PT_DELTAS_PER_ENTRY; ++j)
        {
            PT[i].c_delta[j]=0;
            PT[i].valid[j]=0;
        }
    }
    pt_collisions=0;
}

void pt_update(ST_SIGNATURE signature, PT_DELTA delta)
{
    unsigned int index=signature%N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    BOOL done=FALSE;
    PT_DELTA min_c_delta=(1<<PT_DELTA_SIZE)-1;
    unsigned int min_c_delta_index;
    int invalid_entry_i=-1;

    int j;
    for (j=0; j<N_PT_DELTAS_PER_ENTRY && !done; ++j)
    {
        if (entry->delta[j] == delta && entry->valid[j]) 
        {
            PT_CDELTA prev_cdelta=entry->c_delta[j];
            entry->c_delta[j]=INCREMENT(entry->c_delta[j],PT_CDELTA_SIZE);

            if (prev_cdelta > entry->c_delta[j]) // overflow!
            {
                entry->c_delta[j]=prev_cdelta;
                pt_normalize_counters(entry);
                prev_cdelta=entry->c_delta[j];
                entry->c_delta[j]=INCREMENT(entry->c_delta[j],PT_CDELTA_SIZE);
                assert (prev_cdelta+1 == entry->c_delta[j]);

            }
            done=TRUE;
        }
        else if (entry->c_delta[j] < min_c_delta && entry->valid[j])
        {
            min_c_delta=entry->c_delta[j];
            min_c_delta_index=j;
        }
        else if (!entry->valid[j])
            invalid_entry_i=j;
    }

    if (!done && invalid_entry_i!=-1) // Put it on free entry
    {
        //entry->delta[j]=LRB_MASK(delta,PT_DELTA_SIZE);
        entry->delta[invalid_entry_i]=delta;
        entry->c_delta[invalid_entry_i]=1;
        entry->valid[invalid_entry_i]=1;
        done=TRUE;
    }

    if (!done) // Replace delta with less confident
    {
        //entry->c_sig-=entry->c_delta[min_c_delta_index];
        //entry->delta[min_c_delta_index]=LRB_MASK(delta,PT_DELTA_SIZE);
        entry->delta[min_c_delta_index]=delta;
        entry->c_delta[min_c_delta_index]=1;
        pt_collisions++;
    }

    PT_CSIG prev_csig=entry->c_sig;
    entry->c_sig=INCREMENT(entry->c_sig, PT_CSIG_SIZE);

    if (prev_csig > entry->c_sig) // overflow!
    {
        entry->c_sig=prev_csig;
    }
}

int pt_get_deltas(ST_SIGNATURE signature, double c_threshold, PT_DELTA **delta, double **c)
{
    int index=signature % N_PT_ENTRIES;
    pattern_table_entry_t *entry=&PT[index];

    *delta = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(PT_DELTA));
    *c = malloc(N_PT_DELTAS_PER_ENTRY*sizeof(double));
    int n_deltas=0;

    int i;
    unsigned int total_cdelta=0;
    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
        total_cdelta+=entry->c_delta[i];

    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i])
        {
            //double confidence=entry->c_delta[i]/(double)entry->c_sig;
            double confidence=entry->c_delta[i]/(double)total_cdelta;

            assert (confidence <= 1);
            if (confidence >= c_threshold)
            {
                (*delta)[n_deltas]=entry->delta[i];
                (*c)[n_deltas]=confidence;
                n_deltas++;
            }
        }
    }

    return n_deltas;
}

unsigned int pt_used_entries()
{
    unsigned int result=0;

    int i;
    for (i=0; i<N_PT_ENTRIES; ++i)
    {
        int j;
        for (j=0; j<N_PT_DELTAS_PER_ENTRY; ++j)
        {
            if (PT[i].valid[j])
                result++;
        }
    }
    return result;
}

void pt_normalize_counters(pattern_table_entry_t *entry)
{
    PT_CDELTA min_cdelta=((1<<PT_CDELTA_SIZE)-1);
    int i;
    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i] && entry->c_delta[i] < min_cdelta)
            min_cdelta=entry->c_delta[i];
    }

    if (min_cdelta == 1)
        min_cdelta=2;

    for (i=0; i<N_PT_DELTAS_PER_ENTRY; ++i)
    {
        if (entry->valid[i])
            if (entry->c_delta[i] > 1)
                entry->c_delta[i]/=min_cdelta;
    }
    entry->c_sig/=min_cdelta;
}


/***** PF *****/
/* Not exposed functions */

BOOL pf_get_entry(unsigned long long int pf_addr, prefetch_filter_entry_t **set, int *way);

void pf_init()
{
    assert (PF_VALID_SIZE <= sizeof(PF_VALID)<<3);
    assert (PF_TAG_SIZE <= sizeof(PF_TAG)<<3);
    assert (PF_USEFUL_SIZE <= sizeof(PF_USEFUL)<<3);

    int i;
    for (i=0; i<N_PF_ENTRIES; ++i)
    {
        int j;
        for (j=0; j<N_PF_WAYS; ++j)
        {
            PF[i].valid[j]=0;
            PF[i].useful[j]=0;
            PF[i].lru[j]=(1<<PF_LRU_SIZE)-1;
        }
    }
    pf_collisions=0;
    c_total=0;
    c_useful=0;
    alfa=0.5;
}

void pf_remove_entry(unsigned long long pf_addr)
{
    int way;
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, &way);

    if (exists)
    {
        set->valid[way]=0;
        set->useful[way]=0;
    }
}

BOOL pf_exist_entry(unsigned long long pf_addr)
{
    return (pf_get_entry(pf_addr, NULL, NULL));
}

void pf_insert_entry(unsigned long long pf_addr)
{
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, NULL);

    assert(!exists);

    int way=rand()%N_PF_WAYS;

    int j;
    for (j=0; j<N_PF_WAYS; ++j)
    {
        if (!set->valid[j])
        {
            way=j;
            break;
        }
    }

    if (set->valid[way])
    {
        pf_collisions+=1;
    }
    else
    {
        set->valid[way]=1;
    }

    PF_TAG tag = PF_ADDR_TO_TAG(pf_addr);
    set->tag[way]=tag;
    set->useful[way]=0;
}

void pf_increment_total()
{
    PF_AC_CTOTAL prev_total = c_total;
    c_total=INCREMENT(c_total, PF_AC_CTOTAL_SIZE);

    if (prev_total > c_total) /* overflow detection */
    {
        c_total/=c_useful;
        c_useful=1;
    }
}

void pf_increment_useful(unsigned long long pf_addr)
{
    int way;
    prefetch_filter_entry_t *set;
    BOOL exists=pf_get_entry(pf_addr, &set, &way);

    if (exists && !set->useful[way])
    {
        set->useful[way]=1;
        c_useful=INCREMENT(c_useful, PF_AC_CUSEFUL_SIZE);
    }
    /*else if(!exists)
    {
        pf_insert_entry(pf_addr);
        exists=pf_get_entry(pf_addr, &set, &way);
        assert(exists);

        set->useful[way]=1;
    }*/
}

double pf_get_alfa()
{
    if (c_total == 0)
        return 0.9;

    double alfa=c_useful/(double)c_total;

    if (alfa >= 1) /* because overflow */
        alfa=0.9;

    return alfa;

}

/* Not exposed functions */
    
BOOL pf_get_entry(unsigned long long int pf_addr, prefetch_filter_entry_t **set, int *way)
{
    int set_i=PF_ADDR_TO_INDEX(pf_addr)%N_PF_ENTRIES;
    PF_TAG tag=PF_ADDR_TO_TAG(pf_addr);
    
    if (set != NULL) *set=&PF[set_i];

    int j;
    for (j=0; j<N_PF_WAYS; ++j)
    {
        if (PF[set_i].valid[j] && PF[set_i].tag[j] == tag)
        {
            if (way != NULL) *way=j;
            return TRUE;
        }
    }
    return FALSE;
}


/***** GHR *****/
void ghr_init()
{
    assert(ST_SIGNATURE_SIZE <= sizeof(ST_SIGNATURE)<<3);
    assert(ST_LAST_OFFSET_SIZE <= sizeof(ST_LAST_OFFSET)<<3);
    assert(PT_DELTA_SIZE <= sizeof(PT_DELTA)<<3);
    assert(GHR_CONFIDENCE_SIZE <= sizeof(GHR_CONFIDENCE)<<3);

    int i;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        GHR[i].valid=0;
    }

    ghr_collisions=0;
    ghr_predictions=0;
}

void ghr_update(ST_SIGNATURE signature, GHR_CONFIDENCE confidence,
        ST_LAST_OFFSET last_offset, PT_DELTA delta)
{
    int index=signature%N_GHR_ENTRIES;

    if (GHR[index].valid)
        ghr_collisions++;

    GHR[index].signature=signature;
    GHR[index].confidence=confidence;
    GHR[index].last_offset=last_offset;
    GHR[index].delta=delta;
    GHR[index].valid=1;
}

BOOL ghr_get_signature(ST_LAST_OFFSET offset, ST_SIGNATURE *signature, GHR_CONFIDENCE *c)
{
    int i;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        if (GHR[i].valid)
        {
            ST_LAST_OFFSET ghr_offset=GHR[i].last_offset+GHR[i].delta-(1<<BLOCK_OFFSET_BITS);
            if (ghr_offset == offset)
            {
                *signature = generate_signature(GHR[i].signature, GHR[i].delta);
                *c = GHR[i].confidence;
                ghr_predictions++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

int ghr_used_entries()
{
    int i, result=0;
    for (i=0; i<N_GHR_ENTRIES; ++i)
    {
        if (GHR[i].valid) 
            result++;
    }
    return result;
}

/****** Helpers prototype *******/
int perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr, int ll, unsigned long long int base_addr);


void l2_prefetcher_initialize(int cpu_num)
{
    stats_filtered_pref=0;
    total_lookahead_depth=0;
    n_lookahead_depth=0;

    st_init();
    pt_init();
    pf_init();
    ghr_init();
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    pf_increment_useful(addr);
    unsigned int page = (addr>>(PAGE_BLOCK_OFFSETS));
    unsigned int block = LRB_MASK(addr>>(BLOCK_OFFSET_BITS), PAGE_OFFSET_BITS);

    ST_SIGNATURE signature;
    ST_LAST_OFFSET last_block;
    ST_TAG tag=LRB_MASK(page, ST_TAG_SIZE);

    // 1. Who to update at PT
    BOOL exist=st_get_signature(tag, &signature, &last_block);

    if (exist)
    {
        //PT_DELTA new_delta=LRB_MASK(block-last_block, PT_DELTA_SIZE);
        PT_DELTA new_delta=block-last_block;
        if (new_delta == 0)
            return;
        // 2. Update pattern with last access
        pt_update(signature, new_delta);
        // 3. Update signature
        st_update(tag, block);
        // 4. Get last signature to perform the prefetching
        ST_LAST_OFFSET dummy;
        st_get_signature(tag, &signature, &dummy);

        // 5. Get prediction
        PT_DELTA *delta;
        double *confidence;
        unsigned int n_deltas = pt_get_deltas(signature, Tp, &delta, &confidence);

        // 6. Perform prefetch. There is no Pd_prev, so is 1
        if (n_deltas > 0)
        {
            int lookahead_level=perform_prefetches(delta, confidence, 
                    n_deltas, 1, signature, addr, 1, addr); 
            if (lookahead_level>0)
            {
                total_lookahead_depth+=lookahead_level;
                n_lookahead_depth+=1;
            }
        }

        free(delta);
        free(confidence);
    }
    else
    {
        GHR_CONFIDENCE c;
        BOOL ghr_detect = ghr_get_signature(block, &signature, &c);
        if (ghr_detect)
        {
            st_set_signature(tag, signature, block);
            
            PT_DELTA *delta;
            double *confidence;
            unsigned int n_deltas = pt_get_deltas(signature, Tp, &delta, &confidence);
            if (n_deltas > 0)
            {
                int lookahead_level=perform_prefetches(delta, confidence, 
                    n_deltas, c, signature, addr, 1, block); 
                total_lookahead_depth+=lookahead_level;
                n_lookahead_depth+=1;
            }

        }
        else
            st_update(tag, block);
    }
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
    if (evicted_addr != 0 && prefetch)
        pf_remove_entry(evicted_addr);
    
    if (!pf_exist_entry(addr))
      pf_insert_entry(addr);
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{

    double avg_deph=0;
    if (n_lookahead_depth > 0)
    {
        avg_deph=(total_lookahead_depth/(double)n_lookahead_depth);
    }
    printf("* Lookahead avg. depth (%u/%u): %f\n", total_lookahead_depth, n_lookahead_depth, avg_deph);
    printf("* Lookahed throttle: Cu=%d Ct=%d alfa = %f\n", c_useful, c_total, pf_get_alfa());
    printf("* PF stats: filtered=%u repl=%u\n", stats_filtered_pref, pf_collisions);
//  printf("ST stats: used=%u repl=%u\n", st_used_entries(), st_collisions);
    printf("* PT stats: used=%u repl=%u\n", pt_used_entries(), pt_collisions);
    printf("GHR stats; used=%u repl=%u pred=%u\n", ghr_used_entries(), 
          ghr_collisions, ghr_predictions);
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  printf("Prefetcher final stats\n");
}

/******* Helpers ********/

int perform_prefetches(PT_DELTA *delta, double *confidence, 
        unsigned int n_deltas, double Pd_prev, ST_SIGNATURE base_signature,
        unsigned long long int addr, int ll, unsigned long long int base_addr)
{

    int res_ll=ll;
    assert (n_deltas > 0);

    /* In addition to throttling when Pd falls below the Tp, SPP also stops
     * prefetching if there are not enough L2 read queue resources.
     * Therefore, SPP does not issue prefetches when the number of empty L2 read
     * queue entry becomes less than the number of L1 MSHR
     */

	int remain_rq = L2_READ_QUEUE_SIZE-get_l2_read_queue_occupancy(0);

    if (remain_rq < 8)
        return ll-1;

    // Confidence modulation
    double alfa = pf_get_alfa();

    double highest_pd = 0;
    int i, highest_pd_i;
    unsigned long long int highest_pd_addr;

    for(i=0; i<n_deltas; ++i)
    {   
        double Pd = confidence[i]*Pd_prev;
        assert (Pd <= 1);

        if (ll > 1) /* Is lookahead */
            Pd=Pd*alfa;

        if (Pd < Tp) /* Threshold filter */
            continue;
        
        int fill_level;
        if (Pd > Tf)
            fill_level=FILL_L2;
        else
            fill_level=FILL_LLC;

        // Prefetching address
		unsigned long long int pf_addr = ((addr >> 6) + delta[i]) << 6;
        //unsigned long long int pf_addr = addr+(delta[i]<<BLOCK_OFFSET_BITS);
        BOOL same_page = (ADDR_TO_PAGE(base_addr) == ADDR_TO_PAGE(pf_addr));

        /*
         * Note that SPP does not stop looking even further ahead for
         * more prefetch candidates after coming to the end of a page
         * and updating the GHR.
         */
        if (Pd > highest_pd)
        {
            highest_pd = Pd;
            highest_pd_i = i;
            highest_pd_addr = pf_addr;
        }

        if (!pf_exist_entry(pf_addr) && same_page)
        {

            int res=l2_prefetch_line(0, base_addr, pf_addr, fill_level);

            if (res != 1)
                printf("****** WTF ******\n");
            else
            {
                pf_insert_entry(pf_addr);
                pf_increment_total();
            }

        }
        else if (!same_page /*&& ll == 0*/) /* Not lookahead */
        {
            /* 
             * When there is a pretech request that goes beyond the current
             * page, a conventional streaming prefetcher must stop prefetching
             * because it is impossible to predict the next phyisical page number.
             * So we have to update the ghr
             */
            ST_LAST_OFFSET last_offset=ADDR_TO_BLOCK(addr);
            ghr_update(base_signature, Pd, last_offset, delta[i]);
        }
        else if (pf_exist_entry(pf_addr))
        {
            stats_filtered_pref+=1;
        }
    }

    
    /* SPP only generates a single lookahead signature, choosing the
     * candidate with the highest confidence
     */
    if (highest_pd > 0)
    {
        ST_SIGNATURE new_signature = generate_signature(base_signature, 
                delta[highest_pd_i]);
        PT_DELTA *new_delta;
        double *new_confidence;
        unsigned int new_n_deltas = pt_get_deltas(new_signature, Tp, 
                &new_delta, &new_confidence);

        if (new_n_deltas > 0)
        {
            res_ll=perform_prefetches(new_delta, new_confidence, new_n_deltas, 
                highest_pd, new_signature, highest_pd_addr, ll+1, base_addr);
        }

        free (new_delta);
        free (new_confidence);
    }
    return res_ll;
}
