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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "./maidenhead.h"

void
maidenhead_print(FILE *fp, maidenhead_t *mh)
{
    if (!fp) {
        return;
    }
    if (maidenhead_is_null(mh)) {
        fprintf(fp, "NULL maidenhead\n");
    } else {
        fprintf(fp, "       grid: %s\n", mh->mh);
        fprintf(fp, "  sw corner: (%f, %f)\n",
                mh->sw_corner.lat, mh->sw_corner.lon);
        fprintf(fp, "res degrees: (%f, %f)\n",
                mh->res_degrees.lat, mh->res_degrees.lon);
        fprintf(fp, "     center: (%f, %f)\n", mh->center.lat,
                mh->center.lon);
    }
}

int
maidenhead_is_null(maidenhead_t *mh)
{
    return (!mh || strlen(mh->mh) == 0);
}

// rval: number of valid characters
int
calc_offsets(int *offsets, int offsetlen, const char *grid,
             const int gridlen)
{
    int             i;
    if (gridlen > offsetlen) {
        return -1;
    }
    memset(offsets, 0, offsetlen);

    for (i = 0; i < 2 && i < gridlen; i++) {
        if (grid[i] >= 'a' && grid[i] <= 'r') {
            offsets[i] = grid[i] - 'a';
        } else if (grid[i] >= 'A' && grid[i] <= 'R') {
            offsets[i] = grid[i] - 'A';
        } else {
            return i;
        }
    }
    for (i = 2; i < 4 && i < gridlen; i++) {
        if (grid[i] >= '0' && grid[i] <= '9') {
            offsets[i] = grid[i] - '0';
        } else {
            return i;
        }
    }
    for (i = 4; i < 6 && i < gridlen; i++) {
        if (grid[i] >= 'a' && grid[i] <= 'x') {
            offsets[i] = grid[i] - 'a';
        } else if (grid[i] >= 'A' && grid[i] <= 'X') {
            offsets[i] = grid[i] - 'A';
        } else {
            return i;
        }
    }
    for (i = 6; i < 8 && i < gridlen; i++) {
        if (grid[i] >= '0' && grid[i] <= '9') {
            offsets[i] = grid[i] - '0';
        } else {
            return i;
        }
    }
    // All characters were valid
    return gridlen;
}

// Use a fixed seed so that output is identical
// for a given input. Random but predictable.
unsigned int    rand_seed = 0xDEADBEEF;

double
random_value_in_range(float min, float max)
{
    return min + rand_r(&rand_seed) / (double) RAND_MAX *(max - min);
}

int
maidenhead_random_location(maidenhead_t *mh, latlon_t *ll)
{
    if (maidenhead_is_null(mh) || !ll) {
        return -1;
    }
    ll->lon = random_value_in_range(mh->sw_corner.lon,
                                    mh->sw_corner.lon +
                                    mh->res_degrees.lon);
    ll->lat = random_value_in_range(mh->sw_corner.lat,
                                    mh->sw_corner.lat +
                                    mh->res_degrees.lat);
    return 0;
}


#define GRID_MAXLEN 8
// rval -1 on bad arguments
// returns 
int
populate_maidenhead(maidenhead_t *mh, const char *grid, const int len)
{
    int             rval;
    int             i;
    int             offsets[GRID_MAXLEN];
    float           res[GRID_MAXLEN / 2][2] =
        { {10, 20}, {1, 2}, {(2.5 / 60), (5.0 / 60)}, {(2.5 / 600),
                                                       (5.0 / 600)}
    };

    if (len == 0 || len % 2 || len > GRID_MAXLEN || mh == NULL)
        return -1;

    rval = calc_offsets(offsets, GRID_MAXLEN, grid, len);
    if (rval != len) {
        // return offset of invalid character
        return rval;
    }
    memcpy(mh->mh, grid, len);
    mh->sw_corner.lat = -90.0;
    mh->sw_corner.lon = -180.0;
    for (i = 0; i < len; i += 2) {
        mh->sw_corner.lon += (offsets[i] * res[(i / 2)][1]);
    }
    for (i = 1; i < len; i += 2) {
        mh->sw_corner.lat += (offsets[i] * res[(i / 2)][0]);
    }
    mh->res_degrees.lat = res[(len / 2) - 1][0];
    mh->res_degrees.lon = res[(len / 2) - 1][1];
    // Center
    mh->center.lat = mh->sw_corner.lat + mh->res_degrees.lat / 2;
    mh->center.lon = mh->sw_corner.lon + mh->res_degrees.lon / 2;
    // NW corner
    mh->nw_corner.lat = mh->sw_corner.lat + mh->res_degrees.lat;
    mh->nw_corner.lon = mh->sw_corner.lon;
    // NE corner
    mh->ne_corner.lat = mh->sw_corner.lat + mh->res_degrees.lat;
    mh->ne_corner.lon = mh->sw_corner.lon + mh->res_degrees.lon;
    // SE corner
    mh->se_corner.lat = mh->sw_corner.lat;
    mh->se_corner.lon = mh->sw_corner.lon + mh->res_degrees.lon;
    maidenhead_random_location(mh, &mh->random);
    return len;
}

float
degrees_to_rads(float degrees)
{
    return degrees * M_PI / 180;
}

float
rads_to_degrees(float rads)
{
    return rads * 180 / M_PI;
}

float
haversine(float theta)
{
    return pow(sin(theta / 2), 2);
}

float
maidenhead_distance_km(maidenhead_t *from, maidenhead_t *to)
{
    float           volumetric_mean_radius_earth_km = 6371.0;
    float           lat1 = degrees_to_rads(from->center.lat);
    float           lon1 = degrees_to_rads(from->center.lon);
    float           lat2 = degrees_to_rads(to->center.lat);
    float           lon2 = degrees_to_rads(to->center.lon);
    float           square_half_cord = haversine(lat2 - lat1) +
        cos(lat1) * cos(lat2) * haversine(lon2 - lon1);
    float           angular_distance =
        2 * atan2(sqrt(square_half_cord), sqrt(1 - square_half_cord));
    return angular_distance * volumetric_mean_radius_earth_km;
}

float
maidenhead_bearing_degrees(maidenhead_t *from, maidenhead_t *to)
{
    float           lat1 = degrees_to_rads(from->center.lat);
    float           lon1 = degrees_to_rads(from->center.lon);
    float           lat2 = degrees_to_rads(to->center.lat);
    float           lon2 = degrees_to_rads(to->center.lon);
    float           delta_lon = lon2 - lon1;
    float           tmp = rads_to_degrees(atan2(sin(delta_lon) * cos(lat2),
                                                (cos(lat1) * sin(lat2)) -
                                                (sin(lat1) * cos(lat2) *
                                                 cos(delta_lon))));
    if (tmp < 0) {
        return 360 + tmp;
    } else {
        return tmp;
    }
}
