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
#include <assert.h>
#include "./geojson.h"

#define GRID_NAME_LEN 4

struct grid_info {
    char            name[GRID_NAME_LEN + 1];
    int             num_stations;
    int             num_qsos;
    UT_hash_handle  hh;
};

#define DEBUG_JSON 0

void
json_obj_open(FILE *fp)
{
    fprintf(fp, "{");
#if DEBUG_JSON
    fprintf(fp, "\n");
#endif
}

void
json_obj_close(FILE *fp)
{
    fprintf(fp, "}");
#if DEBUG_JSON
    fprintf(fp, "\n");
#endif
}

// This needs to be improved.
// For now, just converting quotes to space
char           *
escape_quote(char *in)
{
    size_t          i;
    if (!in) {
        return in;
    }
    for (i = 0; i < strlen(in); i++) {
        if (in[i] == '"') {
            in[i] = ' ';
        } else if (in[i] == '\'') {
            in[i] = ' ';
        }
    }
    return in;
}

void
json_attr(FILE *fp, char *name)
{
    fprintf(fp, "\"%s\":", escape_quote(name));
}

void
json_val(FILE *fp, char *val)
{
    fprintf(fp, "\"%s\"", escape_quote(val));
#if DEBUG_JSON
    fprintf(fp, "\n");
#endif
}

void
mh_print_coordinates(FILE *fp, struct maidenhead *mh)
{
    char           *fmt = "[%.6f,%.6f]";
    fprintf(fp, fmt, mh->sw_corner.lon, mh->sw_corner.lat);
    fprintf(fp, ",");
    fprintf(fp, fmt, mh->nw_corner.lon, mh->nw_corner.lat);
    fprintf(fp, ",");
    fprintf(fp, fmt, mh->ne_corner.lon, mh->ne_corner.lat);
    fprintf(fp, ",");
    fprintf(fp, fmt, mh->se_corner.lon, mh->se_corner.lat);
    fprintf(fp, ",");
    fprintf(fp, fmt, mh->sw_corner.lon, mh->sw_corner.lat);
}

int
write_geojson_station(adif_station_t *station, void *arg, int last_item)
{
    FILE           *fp = (FILE *) arg;
    latlon_t        ll;
    if (!station) {
        return -1;
    }
    struct maidenhead *mh = &station->their_grid;

    json_obj_open(fp);          // start feature
    json_attr(fp, "type");
    json_val(fp, "Feature");
    fprintf(fp, ",");
    json_attr(fp, "properties");
    json_obj_open(fp);          // start properties
    if (station->name) {
        json_attr(fp, "name");
        json_val(fp, station->name);
        fprintf(fp, ",");
    }
    if (!maidenhead_is_null(&station->their_grid)) {
        json_attr(fp, "grid");
        json_val(fp, station->their_grid.mh);
        fprintf(fp, ",");
        json_attr(fp, "lat");
        fprintf(fp, "%.6f,", mh->center.lat);
        json_attr(fp, "lon");
        fprintf(fp, "%.6f,", mh->center.lon);
    }
    if (!maidenhead_is_null(&station->my_grid)) {
        json_attr(fp, "distance");
        fprintf(fp, "%.1f,", station->distance_km);
        json_attr(fp, "bearing_sent");
        fprintf(fp, "%.1f,", station->bearing_sent);
        json_attr(fp, "bearing_rcvd");
        fprintf(fp, "%.1f,", station->bearing_rcvd);
    }
    if (station->qth) {
        json_attr(fp, "QTH");
        json_val(fp, station->qth);
        fprintf(fp, ",");
    }
    if (station->num_qsos) {
        json_attr(fp, "QSOs");
        fprintf(fp, "%d,", station->num_qsos);
    }
    if (station->country) {
        json_attr(fp, "country");
        json_val(fp, station->country);
        fprintf(fp, ",");
    }
    // call is last because it's required
    // and we don't want to have a stray ','
    json_attr(fp, "call");
    json_val(fp, station->their_call);
    json_obj_close(fp);         // end properties
    fprintf(fp, ",");
    json_attr(fp, "geometry");
    json_obj_open(fp);          // start geometry
    json_attr(fp, "type");
    json_val(fp, "Point");
    fprintf(fp, ",");
    json_attr(fp, "coordinates");

    mh->random_location(mh, &ll);
    fprintf(fp, "[%.6f,%.6f,0.0]", ll.lon, ll.lat);
    json_obj_close(fp);         // end geometry
    json_obj_close(fp);         // end feature
    if (!last_item) {
        fprintf(fp, ",");
    }
    return 0;
}

void
free_gridinfo(struct grid_info *table)
{
    struct grid_info *s,
                   *tmp;
    HASH_ITER(hh, table, s, tmp) {
        HASH_DEL(table, s);
        free(s);
    }
}

void
write_gridinfo(FILE *fp, struct grid_info *table)
{
    struct grid_info *s,
                   *tmp;
    struct maidenhead mh;
    int             first = 1;

    if (!fp || !table) {
        return;
    }

    HASH_ITER(hh, table, s, tmp) {
        populate_maidenhead(&mh, s->name, GRID_NAME_LEN);
        if (first) {
            first = 0;
        } else {
            fprintf(fp, ",");
        }
        json_obj_open(fp);      // start feature
        json_attr(fp, "type");
        json_val(fp, "Feature");
        fprintf(fp, ",");
        json_attr(fp, "properties");
        json_obj_open(fp);      // start properties
        json_attr(fp, "name");
        json_val(fp, s->name);
        fprintf(fp, ",");
        json_attr(fp, "num_qsos");
        fprintf(fp, "%d,", s->num_qsos);
        json_attr(fp, "num_stations");
        fprintf(fp, "%d,", s->num_stations);
        json_attr(fp, "lat_range");
        // Note: if you change grid resolution
        // you may need to use a floats here
        fprintf(fp, "[%d,%d],",
                (int) mh.sw_corner.lat, (int) mh.nw_corner.lat);
        json_attr(fp, "lon_range");
        fprintf(fp, "[%d,%d]",
                (int) mh.sw_corner.lon, (int) mh.se_corner.lon);
        json_obj_close(fp);     // end properties
        fprintf(fp, ",");
        json_attr(fp, "geometry");
        json_obj_open(fp);      // start geometry
        json_attr(fp, "type");
        json_val(fp, "Polygon");
        fprintf(fp, ",");
        json_attr(fp, "coordinates");
        fprintf(fp, "[[");
        mh_print_coordinates(fp, &mh);
        fprintf(fp, "]]");
        json_obj_close(fp);     // end geometry
        json_obj_close(fp);     // end feature
    }
}

struct grid_info *
create_grid_info(char *name, int num_qsos)
{
    struct grid_info *rval;
    rval = (struct grid_info *) malloc(sizeof(*rval));
    assert(rval);
    memset(rval, 0, sizeof(*rval));
    memcpy(rval->name, name, GRID_NAME_LEN);
    rval->num_qsos = num_qsos;
    rval->num_stations = 1;
    return rval;
}

int
save_grid_info(adif_station_t *station, void *arg, int last_item)
{
    (void) last_item;
    struct grid_info **table = (struct grid_info **) arg;
    char            base_mh[GRID_NAME_LEN + 1];
    struct grid_info *tmp = NULL;
    memcpy(base_mh, &station->their_grid.mh, GRID_NAME_LEN);
    base_mh[GRID_NAME_LEN] = '\0';
    if (!*table) {
        tmp = create_grid_info(base_mh, station->num_qsos);
        HASH_ADD_STR(*table, name, tmp);
    } else {
        HASH_FIND_STR(*table, base_mh, tmp);
        if (tmp == NULL) {
            // New grid
            tmp = create_grid_info(base_mh, station->num_qsos);
            HASH_ADD_STR(*table, name, tmp);
        } else {
            // Known grid
            tmp->num_stations++;
            tmp->num_qsos += station->num_qsos;
        }
    }
    return 0;
}

struct grid_info *
generate_grid_info_table(adif_station_t *stations)
{
    struct grid_info *table = NULL;
    if (stations) {
        walk_stations(stations, &save_grid_info, &table);
        return table;
    } else {
        return NULL;
    }
}

int
max_grid_qsos(struct grid_info *table)
{
    struct grid_info *s,
                   *tmp = NULL;
    int             rval = -1;
    HASH_ITER(hh, table, s, tmp) {
        if (s->num_qsos > rval) {
            rval = s->num_qsos;
        }
    }
    return rval;
}

void
write_open_featurecollection(FILE *fp, char *name)
{
    json_obj_open(fp);          // start feature collection
    json_attr(fp, "type");
    json_val(fp, "FeatureCollection");
    fprintf(fp, ",");
    json_attr(fp, "name");
    json_val(fp, name);
    fprintf(fp, ",");
}

void
write_station_featurecollection(FILE *fp, adif_station_t *stations)
{
    write_open_featurecollection(fp, "stations");
    json_attr(fp, "features");
    fprintf(fp, "[");           // open features array
    walk_stations(stations, &write_geojson_station, fp);
    fprintf(fp, "]");           // close features array
    json_obj_close(fp);         // close feature collections
}

void
write_grid_featurecollection(FILE *fp, adif_station_t *stations)
{
    struct grid_info *table = NULL;
    int             max_qsos;

    table = generate_grid_info_table(stations);
    max_qsos = max_grid_qsos(table);

    write_open_featurecollection(fp, "grids");
    json_attr(fp, "features");
    fprintf(fp, "[");
    if (table) {
        write_gridinfo(fp, table);
        free_gridinfo(table);
    }
    fprintf(fp, "]");
    fprintf(fp, ",");
    json_attr(fp, "properties");
    json_obj_open(fp);          // open properties
    json_attr(fp, "max_qsos");
    fprintf(fp, "%d", max_qsos);
    json_obj_close(fp);         // close properties
    json_obj_close(fp);         // close feature collections
}

// Use 'jq . <json>' to validate
void
write_geojson(FILE *fp, adif_station_t *stations)
{
    if (!fp || !stations) {
        return;
    }
    fprintf(fp, "[");           // open top-level array
    write_station_featurecollection(fp, stations);
    fprintf(fp, ",");
    write_grid_featurecollection(fp, stations);
    fprintf(fp, "]");           // close top-level array
}
