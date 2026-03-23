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
#include <math.h>

#include "./latlon.h"

#define EARTH_RADIUS_KM 6371.0

double
degrees_to_rads(double degrees)
{
    return degrees * M_PI / 180.0;
}

double
rads_to_degrees(double rads)
{
    return rads * 180.0 / M_PI;
}

#define SIN_POW2(theta) (pow(sin((theta)/2), 2))

// Uses the haversine equation
double
latlon_distance_km(latlon_t from, latlon_t to)
{
    double          lat1 = degrees_to_rads(from.lat);
    double          lat2 = degrees_to_rads(to.lat);
    double          lon1 = degrees_to_rads(from.lon);
    double          lon2 = degrees_to_rads(to.lon);
    double          square_half_cord = SIN_POW2(lat2 - lat1) +
        cos(lat1) * cos(lat2) * SIN_POW2(lon2 - lon1);
    double          angular_distance =
        2 * atan2(sqrt(square_half_cord), sqrt(1 - square_half_cord));
    return angular_distance * EARTH_RADIUS_KM;
}

double
latlon_bearing_degrees(latlon_t from, latlon_t to)
{
    double          lat1 = degrees_to_rads(from.lat);
    double          lon1 = degrees_to_rads(from.lon);
    double          lat2 = degrees_to_rads(to.lat);
    double          lon2 = degrees_to_rads(to.lon);
    double          delta_lon = lon2 - lon1;
    double          theta =
        rads_to_degrees(atan2(sin(delta_lon) * cos(lat2),
                              (cos(lat1) * sin(lat2)) -
                              (sin(lat1) * cos(lat2) * cos(delta_lon))));
    return fmod(theta + 360.0, 360.0);
}

int
latlon_destination(latlon_t start, double distance_km,
                   double bearing_degrees, latlon_t *dest)
{
    double          lon;
    if (!(dest && distance_km >= 0
          && (bearing_degrees >= 0.0 && bearing_degrees <= 360.0))) {
        return -1;
    }
    double          angular_distance = distance_km / EARTH_RADIUS_KM;
    dest->lat = rads_to_degrees(asin(sin(degrees_to_rads(start.lat)) *
                                     cos(angular_distance) +
                                     cos(degrees_to_rads(start.lat)) *
                                     sin(angular_distance) *
                                     cos(degrees_to_rads
                                         (bearing_degrees))));
    lon =
        start.lon +
        rads_to_degrees(atan2
                        (sin(degrees_to_rads(bearing_degrees)) *
                         sin(angular_distance) *
                         cos(degrees_to_rads(start.lat)),
                         cos(angular_distance) -
                         sin(degrees_to_rads(start.lat)) *
                         sin(degrees_to_rads(dest->lat))));
    dest->lon = fmod(lon + 540.0, 360.0) - 180.0;
    return 0;
}
