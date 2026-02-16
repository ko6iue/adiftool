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
#ifndef ADIF_H
#define ADIF_H
#include <stdio.h>
#include "maidenhead.h"
#include "uthash.h"

#define GRID_LEN 16

struct adi_qso {
    char           *their_call;
    char           *name;
    char           *country;
    char           *qth;
    struct maidenhead my_grid;
    struct maidenhead their_grid;
    float           distance_km;
    float           bearing_degrees;
    int             num_qsos;
    UT_hash_handle  hh;
};

int             print_qso(struct adi_qso *qso, void *arg, int last_item);
struct adi_qso *load_qsos_mem(char *buf, size_t len);
struct adi_qso *load_qsos_fp(FILE * fp);
int             walk_qsos(struct adi_qso *qsos,
                          int (*cb)(struct adi_qso *, void *arg,
                                    int last_item), void *arg);
int             print_qsos(FILE *, struct adi_qso *qsos);
void            free_qsos(struct adi_qso *qsos);

#endif
