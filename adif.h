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

#define ADIF_DATE_LEN 16
typedef struct {
    char           *their_call;
    char           *name;
    char           *country;
    char           *qth;
    maidenhead_t    my_grid;
    maidenhead_t    their_grid;
    float           distance_km;
    float           bearing_sent;
    float           bearing_rcvd;
    int             num_qsos;
    int             confirmed;
    char            first_contact[ADIF_DATE_LEN];
    char            last_contact[ADIF_DATE_LEN];
    UT_hash_handle  hh;
} adif_station_t;

#define GRID_NAME_LEN 4
typedef struct {
    char            name[GRID_NAME_LEN + 1];
    maidenhead_t    mh;
    int             num_stations;
    int             num_confirmed_stations;
    int             num_qsos;
    char            first_contact[ADIF_DATE_LEN];
    char            last_contact[ADIF_DATE_LEN];
    UT_hash_handle  hh;
} adif_grid_t;

typedef struct {
    char           *name;
    int             num_stations;
    int             num_confirmed_stations;
    int             num_qsos;
    UT_hash_handle  hh;
} adif_country_t;

typedef struct {
    int             grid_max_qsos;
    int             num_stations;
    int             num_confirmed_stations;
    int             num_qsos;
    int             num_countries;
    int             num_confirmed_countries;
    char            first_contact[ADIF_DATE_LEN];
    char            last_contact[ADIF_DATE_LEN];
    adif_grid_t    *grids;
    adif_station_t *stations;
    adif_country_t *countries;
} adif_data_t;

adif_data_t    *load_adif_mem(char *buf, size_t len);
adif_data_t    *load_adif_fp(FILE * fp);


int             walk_stations(adif_station_t * stations,
                              int (*cb)(adif_station_t *, void *arg,
                                        int last_item), void *arg);
int             walk_grids(adif_grid_t * grids,
                           int (*cb)(adif_grid_t *, void *arg,
                                     int last_item), void *arg);

int             print_stations(FILE *, adif_station_t * stations);

void            free_data(adif_data_t * data);

#endif
