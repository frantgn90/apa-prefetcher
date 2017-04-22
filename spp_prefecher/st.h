#include <stdio.h>
#include <params.h>

struct signature_table_entry
{
   1BIT_FIELD valid;
   16BIT_FIELD tag;
   6BIT_FIELD last_offset;
   SIGNATURE_FIELD signature;
   6BIT_FIELD lru;
};
typedef struct signature_table_entry signature_table_entry_t;

signature_table_entry_t ST[N_ST_ENTRIES];

/* Exposed functions */

void st_init();
BOOL st_get_signature(16BIT_FIELD tag, 12BIT_FIELD *signature);
void st_update(16BIT_FIELD tag, 6BIT_FIELD offset);
