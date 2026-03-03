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
#ifndef MAIDENHEAD_H
#define MAIDENHEAD_H

typedef struct {
    float           lat;
    float           lon;
} latlon_t;

#define MAIDENHEAD_MAXLEN 16
typedef struct {
    char            mh[MAIDENHEAD_MAXLEN];
    latlon_t        res_degrees;
    latlon_t        center;
    latlon_t        sw_corner;
    latlon_t        nw_corner;
    latlon_t        se_corner;
    latlon_t        ne_corner;
    latlon_t        random;
} maidenhead_t;

int             maidenhead_is_null(maidenhead_t * mh);

void            maidenhead_print(FILE * fp, maidenhead_t * mh);
// rval -1 on bad arguments, 0 OK, positive is offset of invalid character
int             maidenhead_init(maidenhead_t * mh,
                                const char *grid, const int len);
float           maidenhead_distance_km(maidenhead_t * from,
                                       maidenhead_t * to);
float           maidenhead_bearing_degrees(maidenhead_t * from,
                                           maidenhead_t * to);

#endif
