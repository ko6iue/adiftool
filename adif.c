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
            counter_free(&station->bands);
            counter_free(&station->modes);
            free_station_strdups(station);
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
            counter_free(&grid->bands);
            counter_free(&grid->modes);
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
    counter_free(&data->bands);
    counter_free(&data->modes);
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

int
build_grid_data(adif_station_t *station, adif_data_t *data)
{
    adif_grid_t    *grid;
    char            grid_name[GRID_NAME_LEN + 1];

    // Truncate the grid to GRID_NAME_LEN in size
    strncpy(grid_name, station->their_grid.mh, GRID_NAME_LEN);
    grid_name[GRID_NAME_LEN] = '\0';

    HASH_FIND_STR(data->grids, grid_name, grid);
    if (grid == NULL) {
        // Add new grid
        grid = (adif_grid_t *) calloc(1, sizeof(*grid));
        strncpy(grid->name, grid_name, sizeof(grid->name) - 1);
        HASH_ADD_STR(data->grids, name, grid);

        // Initialize new grid
        strncpy(grid->first_contact, station->first_contact,
                sizeof(grid->first_contact));
        strncpy(grid->last_contact, station->last_contact,
                sizeof(grid->last_contact));

    } else {
        // Update known grid
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
    counter_merge(&grid->modes, station->modes);
    counter_merge(&grid->bands, station->bands);
    return 0;
}

int
build_country_data(adif_station_t *station, adif_data_t *data)
{
    adif_country_t *country;
    if (!station->country) {
        return 0;
    }
    HASH_FIND_STR(data->countries, station->country, country);
    if (country == NULL) {
        country = (adif_country_t *) calloc(1, sizeof(*country));
        // TODO ????
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
        counter_merge(&data->modes, grid->modes);
        counter_merge(&data->bands, grid->bands);
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

int
by_count(const adif_counter_t *a, const adif_counter_t *b)
{
    return (b->count - a->count);
}

void
sort_mode_bands(adif_data_t *data)
{
    adif_station_t *station,
                   *stationtmp;
    adif_grid_t    *grid,
                   *gridtmp;
    // Sort modes and bands at all levels
    HASH_ITER(hh, data->stations, station, stationtmp) {
        if (station->modes) {
            HASH_SORT(station->modes, by_count);
        }
        if (station->bands) {
            HASH_SORT(station->bands, by_count);
        }
    }
    HASH_ITER(hh, data->grids, grid, gridtmp) {
        if (grid->modes) {
            HASH_SORT(grid->modes, by_count);
        }
        if (grid->bands) {
            HASH_SORT(grid->bands, by_count);
        }
    }
    if (data->modes) {
        HASH_SORT(data->modes, by_count);
    }
    if (data->bands) {
        HASH_SORT(data->bands, by_count);
    }
}

void
summarize_data(adif_data_t *data)
{
    adif_station_t *station,
                   *stationtmp;
    if (!data) {
        return;
    }
    // Build grid data
    HASH_ITER(hh, data->stations, station, stationtmp) {
        build_grid_data(station, data);
    }
    HASH_ITER(hh, data->stations, station, stationtmp) {
        build_country_data(station, data);
    }
    // Build grid summary information
    build_grid_summary_data(data);
    // Build country summary data
    build_country_summary_data(data);
    // Sort the band/modes at all levels
    sort_mode_bands(data);
}

char           *
__strtoupper(char *in)
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
update_station_mode_band(adif_station_t *station, char *mode, char *band)
{
    if (strlen(mode) > 0) {
        counter_increment(&station->modes, mode, 1);
    }
    if (strlen(band) > 0) {
        counter_increment(&station->bands, band, 1);
    }
}

typedef struct {
    adif_data_t    *data;
    adif_station_t *cur;
    char           *value;
    char            date_field[ADIF_DATE_LEN];
    char            band_field[16];
    char            mode_field[16];
} adif_callback_arg_t;

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

void
eor_cb(adif_callback_arg_t *args)
{
    adif_station_t *station;
    if (valid_station(args->cur)) {
        // process this useable QSO record
        HASH_FIND_STR(args->data->stations, args->cur->their_call,
                      station);
        if (station == NULL) {
            // Set key and add to hash
            station = (adif_station_t *) calloc(1, sizeof(*station));
            station->their_call = args->cur->their_call;
            HASH_ADD_STR(args->data->stations, their_call, station);

            // Initialize non-key fields
            station->name = args->cur->name;
            station->country = args->cur->country;
            station->qth = args->cur->qth;
            memcpy(&station->my_grid, &args->cur->my_grid,
                   sizeof(station->my_grid));
            memcpy(&station->their_grid, &args->cur->their_grid,
                   sizeof(station->their_grid));

            station->distance_km =
                maidenhead_distance_km(&args->cur->my_grid,
                                       &args->cur->their_grid);
            station->bearing_sent =
                maidenhead_bearing_degrees(&args->cur->my_grid,
                                           &args->cur->their_grid);
            station->bearing_rcvd =
                maidenhead_bearing_degrees(&args->cur->their_grid,
                                           &args->cur->my_grid);
            station->num_qsos = 1;
            station->confirmed = args->cur->confirmed;

            strncpy(station->first_contact, args->date_field,
                    sizeof(station->first_contact));
            strncpy(station->last_contact, args->date_field,
                    sizeof(station->last_contact));

            // This sets all strdup strings to NULL to prevent
            // data from being freed with free_station_strdups
            // since we're using them in this station.
            memset(args->cur, 0, sizeof(*args->cur));
        } else {
            // Data from an existing station
            station->num_qsos += 1;
            if (!station->confirmed && args->cur->confirmed) {
                station->confirmed = 1;
            }

            if (strcmp(args->date_field, station->first_contact) < 0) {
                strncpy(station->first_contact, args->date_field,
                        sizeof(station->first_contact));
            }
            if (strcmp(args->date_field, station->last_contact) > 0) {
                strncpy(station->last_contact, args->date_field,
                        sizeof(station->last_contact));
            }
        }
        update_station_mode_band(station, args->mode_field,
                                 args->band_field);
    }
    // Cleanup / reset for more next record
    free_station_strdups(args->cur);
    memset(args->cur, 0, sizeof(*args->cur));
    memset(args->date_field, 0, sizeof(args->date_field));
    memset(args->mode_field, 0, sizeof(args->mode_field));
    memset(args->band_field, 0, sizeof(args->band_field));
}

void
call_cb(adif_callback_arg_t *args)
{
    args->cur->their_call = strdup(args->value);
}

void
name_cb(adif_callback_arg_t *args)
{
    args->cur->name = strdup(args->value);
}

void
country_cb(adif_callback_arg_t *args)
{
    args->cur->country = strdup(args->value);
}

void
qth_cb(adif_callback_arg_t *args)
{
    args->cur->qth = strdup(args->value);
}

void
gridsquare_cb(adif_callback_arg_t *args)
{
    maidenhead_init(&args->cur->their_grid, args->value,
                    strlen(args->value));
}

void
my_gridsquare_cb(adif_callback_arg_t *args)
{
    maidenhead_init(&args->cur->my_grid, args->value, strlen(args->value));
}

void
qsl_cb(adif_callback_arg_t *args)
{
    if (!args->cur->confirmed && (strcasecmp("y", args->value) == 0)) {
        args->cur->confirmed = 1;
    }
}

void
date_cb(adif_callback_arg_t *args)
{
    memset(args->date_field, 0, sizeof(args->date_field));
    strncpy(args->date_field, args->value, sizeof(args->date_field) - 1);
}

void
mode_cb(adif_callback_arg_t *args)
{

    memset(args->mode_field, 0, sizeof(args->mode_field));
    strncpy(args->mode_field, args->value, sizeof(args->mode_field) - 1);
}

void
band_cb(adif_callback_arg_t *args)
{
    memset(args->band_field, 0, sizeof(args->band_field));
    strncpy(args->band_field, args->value, sizeof(args->band_field) - 1);
}

#define FIELD_DELIMITER "<"
#define PAIR_DELIMITER ">"
#define ATTR_DELIMITER ":"

adif_data_t    *
load_adif_mem(char *buf, size_t buf_len)
{
    (void) buf_len;
    int             i;
    char           *field,
                   *fieldattrs,
                   *name;
    char           *fieldptr = NULL,
        *attrptr = NULL,
        *pairptr = NULL;
    struct {
        char           *name;
        void            (*cb)(adif_callback_arg_t * arg);
    } field_handler[] = {
        {.name = "eor",.cb = &eor_cb},
        {.name = "call",.cb = &call_cb},
        {.name = "name",.cb = &name_cb},
        {.name = "country",.cb = &country_cb},
        {.name = "qth",.cb = &qth_cb},
        {.name = "gridsquare",.cb = &gridsquare_cb},
        {.name = "my_gridsquare",.cb = &my_gridsquare_cb},
        {.name = "eqsl_qsl_rcvd",.cb = &qsl_cb},
        {.name = "dcl_qsl_rcvd",.cb = &qsl_cb},
        {.name = "qsl_rcvd",.cb = &qsl_cb},
        {.name = "lotw_qsl_rcvd",.cb = &qsl_cb},
        {.name = "qso_date",.cb = &date_cb},
        {.name = "mode",.cb = &mode_cb},
        {.name = "band",.cb = &band_cb},
        {.name = NULL}
    };
    adif_callback_arg_t args;

    memset(args.date_field, 0, sizeof(args.date_field));
    memset(args.band_field, 0, sizeof(args.band_field));
    memset(args.mode_field, 0, sizeof(args.mode_field));

    args.data = (adif_data_t *) calloc(1, sizeof *args.data);
    if (!args.data) {
        return NULL;
    }
    args.cur = (adif_station_t *) calloc(1, sizeof *args.cur);
    if (!args.cur) {
        free(args.data);
        return NULL;
    }

    for (field = strtok_r(buf, FIELD_DELIMITER, &fieldptr);
         field != NULL;
         field = strtok_r(NULL, FIELD_DELIMITER, &fieldptr)) {

        fieldattrs = strtok_r(field, PAIR_DELIMITER, &pairptr);
        assert(fieldattrs);
        args.value = strtok_r(NULL, PAIR_DELIMITER, &pairptr);
        name = strtok_r(fieldattrs, ATTR_DELIMITER, &attrptr);
        assert(name);
        // TODO: consider processing field value len and type
        // Not necessary for now (if ever).

        for (i = 0; field_handler[i].name; ++i) {
            if (strcasecmp(name, field_handler[i].name) == 0) {
                break;
            }
        }

        if (field_handler[i].name) {
            trim_end_space(args.value);
            field_handler[i].cb(&args);
        }
    }

    free_station_strdups(args.cur);
    free(args.cur);
    summarize_data(args.data);
    return args.data;
}
