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
#include <ctype.h>              // isspace
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
    fprintf(fp, "   first QSO: %s\n", station->first_contact);
    fprintf(fp, "    last QSO: %s\n", station->last_contact);
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

adif_data_t    *
load_adif_fp(FILE *fp)
{
    size_t          fsize;
    char           *data;
    adif_data_t    *rval;

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

    rval = load_adif_mem(data, fsize);
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
            free_counters(station->bands);
            free_counters(station->modes);
            free(station);
        }
    }
}

void
free_grids(adif_grid_t *grids)
{
    adif_grid_t    *grid,
                   *tmp;
    if (grids) {
        HASH_ITER(hh, grids, grid, tmp) {
            HASH_DEL(grids, grid);
            free(grid);
        }
    }
}

void
free_countries(adif_country_t *countries)
{
    adif_country_t *country,
                   *tmp;
    if (countries) {
        HASH_ITER(hh, countries, country, tmp) {
            HASH_DEL(countries, country);
            // NOTE: country->name is freed in station
            free(country);
        }
    }
}

void
free_data(adif_data_t *data)
{
    if (!data) {
        return;
    }
    free_stations(data->stations);
    free_grids(data->grids);
    free_countries(data->countries);
    free(data);
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

adif_grid_t    *
grid_create(char *name)
{
    adif_grid_t    *grid;
    grid = (adif_grid_t *) malloc(sizeof(*grid));
    assert(grid);
    memset(grid, 0, sizeof(*grid));
    memcpy(grid->name, name, GRID_NAME_LEN);
    grid->name[GRID_NAME_LEN] = '\0';
    return grid;
}

int
build_grid_data(adif_station_t *station, void *arg, int last_item)
{
    (void) last_item;
    adif_data_t    *data = (adif_data_t *) arg;
    char            base_mh[GRID_NAME_LEN + 1];
    adif_grid_t    *grid;

    memcpy(base_mh, &station->their_grid.mh, GRID_NAME_LEN);
    base_mh[GRID_NAME_LEN] = '\0';
    HASH_FIND_STR(data->grids, base_mh, grid);
    if (grid == NULL) {
        grid = grid_create(base_mh);
        strncpy(grid->first_contact, station->first_contact,
                sizeof(grid->first_contact));
        strncpy(grid->last_contact, station->last_contact,
                sizeof(grid->last_contact));

        HASH_ADD_STR(data->grids, name, grid);
    } else {
        if (strcmp(station->first_contact, grid->first_contact) < 0) {
            strncpy(grid->first_contact, station->first_contact,
                    sizeof(grid->first_contact));
        }
        if (strcmp(station->last_contact, grid->last_contact) > 0) {
            strncpy(grid->last_contact, station->last_contact,
                    sizeof(grid->last_contact));
        }
    }
    grid->num_stations++;
    if (station->confirmed) {
        grid->num_confirmed_stations++;
    }
    grid->num_qsos += station->num_qsos;
    return 0;
}

int
build_country_data(adif_station_t *station, void *arg, int last_item)
{
    (void) last_item;
    adif_data_t    *data = (adif_data_t *) arg;
    adif_country_t *country;
    if (!station->country) {
        return 0;
    }
    HASH_FIND_STR(data->countries, station->country, country);
    if (country == NULL) {
        country = (adif_country_t *) malloc(sizeof(*country));
        assert(country);
        memset(country, 0, sizeof(*country));
        country->name = station->country;
        HASH_ADD_STR(data->countries, name, country);
    }
    country->num_stations++;
    if (station->confirmed) {
        country->num_confirmed_stations++;
    }
    country->num_qsos += station->num_qsos;
    return 0;
}

void
build_grid_summary_data(adif_data_t *data)
{
    adif_grid_t    *grid,
                   *tmp = NULL;
    data->grid_max_qsos = -1;
    int             first = 1;

    HASH_ITER(hh, data->grids, grid, tmp) {
        if (first) {
            strncpy(data->first_contact, grid->first_contact,
                    sizeof(data->first_contact));
            strncpy(data->last_contact, grid->last_contact,
                    sizeof(data->last_contact));
            first = 0;
        } else {
            if (strcmp(grid->first_contact, data->first_contact) < 0) {
                strncpy(data->first_contact, grid->first_contact,
                        sizeof(data->first_contact));
            }
            if (strcmp(grid->last_contact, data->last_contact) > 0) {
                strncpy(data->last_contact, grid->last_contact,
                        sizeof(data->last_contact));
            }
        }
        if (grid->num_qsos > data->grid_max_qsos) {
            data->grid_max_qsos = grid->num_qsos;
        }
        data->num_qsos += grid->num_qsos;
        data->num_stations += grid->num_stations;
        data->num_confirmed_stations += grid->num_confirmed_stations;
    }
}

void
build_country_summary_data(adif_data_t *data)
{
    adif_country_t *country,
                   *tmp = NULL;
    data->num_countries = HASH_COUNT(data->countries);
    data->num_confirmed_countries = 0;
    HASH_ITER(hh, data->countries, country, tmp) {
        if (country->num_confirmed_stations >= 1) {
            data->num_confirmed_countries++;
        }
    }
}

void
summarize_data(adif_data_t *data)
{
    if (!data) {
        return;
    }
    // Build grid data
    walk_stations(data->stations, &build_grid_data, data);
    // Build country data
    walk_stations(data->stations, &build_country_data, data);
    // Build grid summary information
    build_grid_summary_data(data);
    // Build country summary data
    build_country_summary_data(data);
}

char           *
strtoupper(char *in)
{
    char           *str = in;
    if (!in) {
        return NULL;
    }
    while (*str) {
        *str = toupper(*str);
        str++;
    }
    return in;
}

void
update_named_counters(char *name, adif_counter_t **counters)
{
    adif_counter_t *query;
    if (!name || !counters) {
        return;
    }
    HASH_FIND_STR(*counters, name, query);
    if (!query) {
        query = (adif_counter_t *) malloc(sizeof(*query));
        query->name = strdup(name);
        query->count = 1;
        HASH_ADD_STR(*counters, name, query);
    } else {
        query->count++;
    }
}

void
free_counters(adif_counter_t *counters)
{
    adif_counter_t *counter,
                   *tmp;
    if (counters) {
        HASH_ITER(hh, counters, counter, tmp) {
            HASH_DEL(counters, counter);
            free(counter->name);
            free(counter);
        }
    }
}

#define FIELD_DELIMITER "<"
#define PAIR_DELIMITER ">"
#define ATTR_DELIMITER ":"

adif_data_t    *
load_adif_mem(char *buf, size_t buf_len)
{
    (void) buf_len;
    adif_data_t    *data = NULL;
    adif_station_t *station,
                   *query;
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
        "lotw_qsl_rcvd", "qso_date", "mode", "band", NULL
    };
    char            date_field[ADIF_DATE_LEN];
    char            band_field[16];
    char            mode_field[16];

    data = (adif_data_t *) malloc(sizeof *data);
    assert(data);
    memset(data, 0, sizeof *data);

    station = (adif_station_t *) malloc(sizeof *station);
    assert(station);
    memset(station, 0, sizeof(*station));

    for (field = strtok_r(buf, FIELD_DELIMITER, &fieldptr);
         field != NULL;
         field = strtok_r(NULL, FIELD_DELIMITER, &fieldptr)) {

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
                HASH_FIND_STR(data->stations, station->their_call, query);
                if (query == NULL) {
                    // Data from a new station
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
                    query = (adif_station_t *) malloc(sizeof(*query));
                    memcpy(query, station, sizeof(*station));

                    strncpy(query->first_contact, date_field,
                            sizeof(query->first_contact));
                    strncpy(query->last_contact, date_field,
                            sizeof(query->last_contact));

                    // Add a copy of this new valid QSO
                    HASH_ADD_STR(data->stations, their_call, query);
                    // Set our working QSO to zero
                    memset(station, 0, sizeof(*station));
                } else {
                    // Data from an existing station
                    query->num_qsos += 1;
                    if (!query->confirmed && station->confirmed) {
                        query->confirmed = 1;
                    }

                    if (strcmp(date_field, query->first_contact) < 0) {
                        strncpy(query->first_contact, date_field,
                                sizeof(query->first_contact));
                    }
                    if (strcmp(date_field, query->last_contact) > 0) {
                        strncpy(query->last_contact, date_field,
                                sizeof(query->last_contact));
                    }
                }
                update_named_counters(strtoupper(mode_field),
                                      &query->modes);
                update_named_counters(strtoupper(band_field),
                                      &query->bands);
            }
            // Cleanup / reset station for more data
            free_station_strdups(station);
            memset(station, 0, sizeof(*station));
            memset(date_field, 0, sizeof(date_field));
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
            maidenhead_init(&station->their_grid, value, strlen(value));
            break;
        case 6:                // my_gridsquare
            maidenhead_init(&station->my_grid, value, strlen(value));
            break;
        case 7:                // eqsl_qsl_rcvd
        case 8:                // dcl_qsl_rcvd
        case 9:                // qsl_rcvd
        case 10:               // lotw_qsl_rcvd
            if (!station->confirmed && (strcasecmp("y", value) == 0)) {
                station->confirmed = 1;
            }
            break;
        case 11:               // qso_date
            memset(date_field, 0, sizeof(date_field));
            strncpy(date_field, value, sizeof(date_field) - 1);
            break;
        case 12:               // mode
            memset(mode_field, 0, sizeof(mode_field));
            strncpy(mode_field, value, sizeof(mode_field) - 1);
            break;
        case 13:               // band
            memset(band_field, 0, sizeof(band_field));
            strncpy(band_field, value, sizeof(band_field) - 1);
            break;
        }
    }

    // Free our working QSO
    free_station_strdups(station);
    free(station);

    summarize_data(data);
    return data;
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
walk_grids(adif_grid_t *grids,
           int (*cb)(adif_grid_t *, void *arg, int is_last), void *arg)
{
    int             rval,
                    nitems;
    adif_grid_t    *grid,
                   *tmp;
    if (grids && cb) {
        nitems = HASH_COUNT(grids);
        HASH_ITER(hh, grids, grid, tmp) {
            nitems--;
            rval = (*cb) (grid, arg, nitems == 0);
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
