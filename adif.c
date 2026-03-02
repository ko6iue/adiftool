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
#include <assert.h>

#include "./adif.h"

int
print_station(adif_station_t *station, void *arg, int last_item)
{
    (void) last_item;
    FILE           *fp = (FILE *) arg;
    fprintf(fp, "**********\n");
    fprintf(fp, "        call: %s\n", station->their_call);
    fprintf(fp, "      # QSOs: %d\n", station->num_qsos);
    fprintf(fp, "        name: %s\n", station->name);
    fprintf(fp, "     country: %s\n", station->country);
    fprintf(fp, "         qth: %s\n", station->qth);
    fprintf(fp, "     my grid\n");
    maidenhead_print(fp, &station->my_grid);
    fprintf(fp, "  their grid\n");
    maidenhead_print(fp, &station->their_grid);
    fprintf(fp, " distance km: %f\n", station->distance_km);
    fprintf(fp, "bearing sent: %f\n", station->bearing_sent);
    fprintf(fp, "bearing rcvd: %f\n", station->bearing_rcvd);
    fprintf(fp, "**********\n");
    return 0;
}

// Minimum usable station
int
valid_station(adif_station_t *station)
{
    if (!station) {
        return 0;
    }
    if (!station->their_call || strlen(station->their_call) <= 0) {
        return 0;
    }
    if (maidenhead_is_null(&station->their_grid)) {
        return 0;
    }
    return 1;
}

adif_station_t *
load_stations_fp(FILE *fp)
{
    size_t          fsize;
    char           *data;
    adif_station_t *rval;

    if (!fp) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = malloc(fsize + 1);
    if (data == NULL) {
        return NULL;
    }
    assert(fread(data, fsize, 1, fp) == 1);
    data[fsize] = '\0';

    rval = load_stations_mem(data, fsize);
    free(data);
    return rval;
}

void
free_station_strdups(adif_station_t *station)
{
    if (station) {
        free(station->name);
        free(station->qth);
        free(station->their_call);
        free(station->country);
    }
}

void
free_stations(adif_station_t *stations)
{
    adif_station_t *station,
                   *tmp;
    if (stations) {
        HASH_ITER(hh, stations, station, tmp) {
            HASH_DEL(stations, station);
            free_station_strdups(station);
            free(station);
        }
    }
}

void
trim_end_space(char *p)
{
    char           *q;
    if (p) {
        for (q = p + strlen(p) - 1; q >= p; q--) {
            if (!isspace(*q)) {
                break;
            }
        }
        *(q + 1) = '\0';
    }
}

#define FIELD_LIMITER "<"
#define PAIR_DELIMITER ">"
#define ATTR_DELIMITER ":"

adif_station_t *
load_stations_mem(char *buf, size_t buf_len)
{
    (void) buf_len;
    adif_station_t *stations = NULL,
        *station,
        *tmp;
    int             i;
    char
                   *field,
                   *fieldattrs,
                   *name,
                   *value;
    char           *fieldptr = NULL,
        *attrptr = NULL,
        *pairptr = NULL;
    const char     *fields[] =
        { "eor", "call", "name", "country", "qth", "gridsquare",
        "my_gridsquare", "eqsl_qsl_rcvd", "dcl_qsl_rcvd", "qsl_rcvd",
        "lotw_qsl_rcvd", NULL
    };

    station = (adif_station_t *) malloc(sizeof *station);
    assert(station);
    memset(station, 0, sizeof(*station));

    for (field = strtok_r(buf, FIELD_LIMITER, &fieldptr);
         field != NULL; field = strtok_r(NULL, FIELD_LIMITER, &fieldptr)) {

        fieldattrs = strtok_r(field, PAIR_DELIMITER, &pairptr);
        assert(fieldattrs);
        value = strtok_r(NULL, PAIR_DELIMITER, &pairptr);
        name = strtok_r(fieldattrs, ATTR_DELIMITER, &attrptr);
        assert(name);
        // TODO: consider processing field value len and type
        // Not necessary for now (if ever).

        for (i = 0; fields[i]; ++i) {
            if (strcasecmp(name, fields[i]) == 0) {
                break;
            }
        }
        trim_end_space(value);

        switch (i) {
        case 0:                // EOR
            if (valid_station(station)) {
                // process this useable QSO record
                HASH_FIND_STR(stations, station->their_call, tmp);
                if (tmp == NULL) {
                    // Add QSO from a new station
                    station->distance_km =
                        maidenhead_distance_km(&station->my_grid,
                                               &station->their_grid);
                    station->bearing_sent =
                        maidenhead_bearing_degrees(&station->my_grid,
                                                   &station->their_grid);
                    station->bearing_rcvd =
                        maidenhead_bearing_degrees(&station->their_grid,
                                                   &station->my_grid);
                    station->num_qsos = 1;
                    tmp = (adif_station_t *) malloc(sizeof(*tmp));
                    memcpy(tmp, station, sizeof(*station));
                    // Add a copy of this new valid QSO
                    HASH_ADD_STR(stations, their_call, tmp);
                    // Set our working QSO to zero
                    memset(station, 0, sizeof(*station));
                } else {
                    // Process QSO from existing station
                    tmp->num_qsos += 1;
                    if (!tmp->confirmed && station->confirmed) {
                        tmp->confirmed = 1;
                    }
                }
            }
            // Reset working QSO and start again
            free_station_strdups(station);
            memset(station, 0, sizeof(*station));
            break;
        case 1:                // call
            station->their_call = strdup(value);
            break;
        case 2:                // name
            station->name = strdup(value);
            break;
        case 3:                // country
            station->country = strdup(value);
            break;
        case 4:                // qth
            station->qth = strdup(value);
            break;
        case 5:                // gridsquare
            populate_maidenhead(&station->their_grid, value,
                                strlen(value));
            break;
        case 6:                // my_gridsquare
            populate_maidenhead(&station->my_grid, value, strlen(value));
            break;
        case 7:                // eqsl_qsl_rcvd
        case 8:                // dcl_qsl_rcvd
        case 9:                // qsl_rcvd
        case 10:               // lotw_qsl_rcvd
            if (!station->confirmed && (strcasecmp("y", value) == 0)) {
                station->confirmed = 1;
            }
            break;
        }
    }

    // Free our working QSO
    free_station_strdups(station);
    free(station);
    return stations;
}


int
walk_stations(adif_station_t *stations,
              int (*cb)(adif_station_t *, void *arg, int is_last),
              void *arg)
{
    int             rval,
                    nitems;
    adif_station_t *station,
                   *tmp;
    if (stations && cb) {
        nitems = HASH_COUNT(stations);
        HASH_ITER(hh, stations, station, tmp) {
            nitems--;
            rval = (*cb) (station, arg, nitems == 0);
            if (rval) {
                // abort on error. cb returns 0 on success.
                return rval;
            }
        }
    }
    return 0;
}

int
print_stations(FILE *fp, adif_station_t *stations)
{
    return walk_stations(stations, &print_station, (void *) fp);
}
