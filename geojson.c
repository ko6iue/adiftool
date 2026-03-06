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
#include <assert.h>
#include "./geojson.h"

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
mh_print_coordinates(FILE *fp, maidenhead_t *mh)
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
    if (!station) {
        return -1;
    }
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
        fprintf(fp, "%.6f,", station->their_grid.random.lat);
        json_attr(fp, "lon");
        fprintf(fp, "%.6f,", station->their_grid.random.lon);
    }
    if (!maidenhead_is_null(&station->my_grid)) {
        json_attr(fp, "qth_lat");
        fprintf(fp, "%6f,", station->my_grid.center.lat);
        json_attr(fp, "qth_lon");
        fprintf(fp, "%6f,", station->my_grid.center.lon);
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
    json_attr(fp, "confirmed");
    fprintf(fp, "%s,", station->confirmed ? "true" : "false");
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

    fprintf(fp, "[%.6f,%.6f,0.0]",
            station->their_grid.random.lon,
            station->their_grid.random.lat);
    json_obj_close(fp);         // end geometry
    json_obj_close(fp);         // end feature
    if (!last_item) {
        fprintf(fp, ",");
    }
    return 0;
}

int
write_geojson_grid(adif_grid_t *grid, void *arg, int last_item)
{
    FILE           *fp = (FILE *) arg;
    maidenhead_t    mh;
    if (!fp || !grid) {
        return -1;
    }

    maidenhead_init(&mh, grid->name, GRID_NAME_LEN);
    json_obj_open(fp);          // start feature
    json_attr(fp, "type");
    json_val(fp, "Feature");
    fprintf(fp, ",");
    json_attr(fp, "properties");
    json_obj_open(fp);          // start properties
    json_attr(fp, "name");
    json_val(fp, grid->name);
    fprintf(fp, ",");
    json_attr(fp, "num_qsos");
    fprintf(fp, "%d,", grid->num_qsos);
    json_attr(fp, "num_stations");
    fprintf(fp, "%d,", grid->num_stations);
    json_attr(fp, "num_confirmed_stations");
    fprintf(fp, "%d,", grid->num_confirmed_stations);
    json_attr(fp, "confirmed");
    fprintf(fp, "%s,",
            grid->num_confirmed_stations >= 1 ? "true" : "false");
    json_attr(fp, "lat_range");
    // Note: if you change grid resolution
    // you may need to use a floats here
    fprintf(fp, "[%d,%d],",
            (int) mh.sw_corner.lat, (int) mh.nw_corner.lat);
    json_attr(fp, "lon_range");
    fprintf(fp, "[%d,%d]", (int) mh.sw_corner.lon, (int) mh.se_corner.lon);
    json_obj_close(fp);         // end properties
    fprintf(fp, ",");
    json_attr(fp, "geometry");
    json_obj_open(fp);          // start geometry
    json_attr(fp, "type");
    json_val(fp, "Polygon");
    fprintf(fp, ",");
    json_attr(fp, "coordinates");
    fprintf(fp, "[[");
    mh_print_coordinates(fp, &mh);
    fprintf(fp, "]]");
    json_obj_close(fp);         // end geometry
    json_obj_close(fp);         // end feature
    if (!last_item) {
        fprintf(fp, ",");
    }
    return 0;
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
write_station_featurecollection(FILE *fp, adif_data_t *data)
{
    if (!data) {
        return;
    }
    write_open_featurecollection(fp, "stations");
    json_attr(fp, "features");
    fprintf(fp, "[");           // open features array
    if (data->stations) {
        walk_stations(data->stations, &write_geojson_station, fp);
    }
    fprintf(fp, "]");           // close features array
    json_obj_close(fp);         // close feature collections
}

void
write_grid_featurecollection(FILE *fp, adif_data_t *data)
{
    write_open_featurecollection(fp, "grids");
    json_attr(fp, "features");
    fprintf(fp, "[");
    if (data->grids) {
        walk_grids(data->grids, &write_geojson_grid, fp);
    }
    fprintf(fp, "]");
    json_obj_close(fp);
}

int
write_global_information(FILE *fp, adif_data_t *data)
{
    int             rval = 0;
    json_obj_open(fp);
    // TODO: better error checks and reporting
    if (!data->stations) {
        json_attr(fp, "success");
        fprintf(fp, "false,");
        json_attr(fp, "error_msg");
        json_val(fp, "No data found in file");
        rval = 1;
    } else {
        json_attr(fp, "success");
        fprintf(fp, "true,");
        json_attr(fp, "grid_max_qsos");
        fprintf(fp, "%d,", data->grid_max_qsos);
        json_attr(fp, "total_stations");
        fprintf(fp, "%d,", data->num_stations);
        json_attr(fp, "total_confirmed_stations");
        fprintf(fp, "%d,", data->num_confirmed_stations);
        json_attr(fp, "total_qsos");
        fprintf(fp, "%d,", data->num_qsos);
        json_attr(fp, "total_countries");
        fprintf(fp, "%d,", data->num_countries);
        json_attr(fp, "total_confirmed_countries");
        fprintf(fp, "%d", data->num_confirmed_countries);
    }
    json_obj_close(fp);
    return rval;
}

void
write_geojson(FILE *fp, adif_data_t *data)
{
    if (!fp || !data) {
        return;
    }
    json_obj_open(fp);
    json_attr(fp, "globals");
    if (write_global_information(fp, data) == 0) {
        fprintf(fp, ",");
        json_attr(fp, "stations");
        write_station_featurecollection(fp, data);
        fprintf(fp, ",");
        json_attr(fp, "grids");
        write_grid_featurecollection(fp, data);
    }
    json_obj_close(fp);
}
