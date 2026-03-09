/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2026, Matt Massie
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <ctype.h>
#include "./counter.h"

void
counter_print(FILE *fp, adif_counter_t *cp)
{
    adif_counter_t *counter,
                   *tmp;
    if (cp) {
        HASH_ITER(hh, cp, counter, tmp) {
            fprintf(fp, "%s=%d\n", counter->name, counter->count);
        }
    }
}

void
counter_increment(adif_counter_t **cp, const char *name, const int incr)
{
    adif_counter_t *query;
    if (!cp || !name || strlen(name) <= 0) {
        return;
    }
    HASH_FIND_STR(*cp, name, query);
    if (query == NULL) {
        query = (adif_counter_t *) calloc(1, sizeof(*query));
        query->name = strdup(name);
        HASH_ADD_STR(*cp, name, query);
    }
    query->count += incr;
}

void
counter_merge(adif_counter_t **dst, adif_counter_t *src)
{
    adif_counter_t *counter,
                   *tmp;
    if (dst && src) {
        HASH_ITER(hh, src, counter, tmp) {
            counter_increment(dst, counter->name, counter->count);
        }
    }
}

void
counter_free(adif_counter_t **cp)
{
    adif_counter_t *counter,
                   *tmp;
    if (cp && *cp) {
        HASH_ITER(hh, *cp, counter, tmp) {
            HASH_DEL(*cp, counter);
            free(counter->name);
            free(counter);
        }
        *cp = NULL;
    }
}
