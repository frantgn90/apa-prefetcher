/*
 * test-pt.c
 * Copyright (C) 2017 Juan Francisco Martínez <juan.martinez[AT]bsc[dot]es>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdlib.h>
#include <pt.h>

#define N_SIGNATURES 2
#define N_TESTS_PER_TAG 10

int main(int argc, char **argv)
{
    ST_TAG signs[N_SIGNATURES];

    int i;
    for (i=0; i<N_SIGNATURES; ++i)
    {
        int r=rand();
        signs[i]=LRB_MASK(r, ST_SIGNATURE_SIZE);
    }

    pt_init();

    for (i=0; i<N_SIGNATURES; ++i)
    {
        int j;
        for (j=0; j<N_TESTS_PER_TAG; ++j)
        {
            int r=rand();
            PT_DELTA delta=LRB_MASK(r, PT_DELTA_SIZE);

            pt_update(signs[i], delta);
            
            PT_DELTA *deltas;
            double *c;
            deltas = malloc(sizeof(PT_DELTA*));
            c = malloc(sizeof(double*));

            int n_deltas = pt_get_deltas(signs[i], 0, &deltas, &c);

            printf("UPD {SIGN: %d, DELTA: %d} READ {", signs[i], delta);
            int k;
            for (k=0; k<n_deltas; ++k)
            {
                printf("%d:%f,", deltas[k], c[k]);
            }
            printf("}\n");
        }
    }
}




