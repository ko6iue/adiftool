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
#include <regex.h>

#include "adif.h"

#define PMATCH_LEN(P) ((P)->rm_eo - (P)->rm_so)

int
print_qso(struct adi_qso *qso, void *arg)
{
    FILE           *fp = (FILE *) arg;
    fprintf(fp, "**********\n");
    fprintf(fp, "       call: %s\n", qso->their_call);
    fprintf(fp, "     # QSOs: %d\n", qso->num_qsos);
    fprintf(fp, "       name: %s\n", qso->name);
    fprintf(fp, "    country: %s\n", qso->country);
    fprintf(fp, "        qth: %s\n", qso->qth);
    fprintf(fp, "    my grid\n");
    maidenhead_print(fp, &qso->my_grid);
    fprintf(fp, " their grid\n");
    maidenhead_print(fp, &qso->their_grid);
    fprintf(fp, "distance km: %f\n", qso->distance_km);
    fprintf(fp, "bearing deg: %f\n", qso->bearing_degrees);
    fprintf(fp, "**********\n");
    return 0;
}

// Minimum useful 
int
valid_qso(struct adi_qso *qso)
{
    if (!qso) {
        return 0;
    }
    if (strlen(qso->their_call) <= 0) {
        return 0;
    }
    if (strlen(qso->their_grid.mh) <= 0) {
        return 0;
    }
    if (strlen(qso->my_grid.mh) <= 0) {
        return 0;
    }
    return 1;
}

struct adi_qso *
load_qsos_fp(FILE *fp)
{
    regex_t         regex;
    int             rval;
    struct adi_qso *qsos = NULL,
        *qso,
        *query;
    size_t          nmatch = 3, // all, key, value
        val_len,
        line_len = 0;
    regmatch_t      pmatch[nmatch];
    char           *line = NULL,
        *key,
        *val;

    if (!fp) {
        return NULL;
    }

    rval =
        regcomp(&regex,
                "^<(call|name|country|qth|gridsquare|my_gridsquare|eor)[^>]*>(.*)$",
                REG_EXTENDED | REG_NEWLINE | REG_ICASE);
    if (rval) {
        return NULL;
    }

    qso = (struct adi_qso *) malloc(sizeof *qso);
    if (qso == NULL) {
        return NULL;
    }
    memset(qso, 0, sizeof(*qso));

    while (getline(&line, &line_len, fp) != -1) {
        rval = regexec(&regex, line, nmatch, pmatch, 0);
        if (!rval) {
            val_len = PMATCH_LEN(&pmatch[2]);
            key = line + pmatch[1].rm_so;
            val = line + pmatch[2].rm_so;
            switch (key[0]) {
            case 'c':
            case 'C':
                if (key[1] == 'a' || key[1] == 'A') {
                    qso->their_call = strndup(val, val_len);
                } else {
                    qso->country = strndup(val, val_len);
                }
                break;
            case 'n':
            case 'N':
                qso->name = strndup(val, val_len);
                break;
            case 'q':
            case 'Q':
                qso->qth = strndup(val, val_len);
                break;
            case 'g':
            case 'G':
                populate_maidenhead(&qso->their_grid, val, val_len);
                break;
            case 'm':
            case 'M':
                populate_maidenhead(&qso->my_grid, val, val_len);
                break;
            case 'e':
            case 'E':{
                    // Process record
                    if (valid_qso(qso)) {
                        HASH_FIND_STR(qsos, qso->their_call, query);
                        if (query == NULL) {
                            // Add new person
                            qso->distance_km =
                                maidenhead_distance_km(&qso->my_grid,
                                                       &qso->their_grid);
                            qso->bearing_degrees =
                                maidenhead_bearing_degrees(&qso->my_grid,
                                                           &qso->
                                                           their_grid);
                            qso->num_qsos = 1;
                            HASH_ADD_STR(qsos, their_call, qso);
                            qso = (struct adi_qso *)
                                malloc(sizeof(*qso));
                            if (qso == NULL) {
                                return NULL;
                            }
                        } else {
                            // Station we already know about: process
                            // summary info.
                            query->num_qsos += 1;
                            // TODO: see if we have better maidenhead
                            // info
                            // Check if they moved etc, summarize band
                            // info
                        }
                    }
                    memset(qso, 0, sizeof(*qso));
                }
                break;
            }
        }
    }

    free(qso);                  // free last qso we created and didn't
    // populate
    regfree(&regex);
    free(line);
    return qsos;
}

struct adi_qso *
load_qsos_mem(char *buf, size_t len)
{
    FILE           *fp = fmemopen(buf, len, "r");
    struct adi_qso *qsos = load_qsos_fp(fp);
    fclose(fp);
    return qsos;
}


int
walk_qsos(struct adi_qso *qsos, int (*cb)(struct adi_qso *, void *arg),
          void *arg)
{
    int             rval;
    struct adi_qso *qso,
                   *tmp;
    if (qsos && cb) {
        HASH_ITER(hh, qsos, qso, tmp) {
            rval = (*cb) (qso, arg);
            if (rval) {
                // premature stop
                return rval;
            }
        }
    }
    return 0;
}

int
print_qsos(FILE *fp, struct adi_qso *qsos)
{
    return walk_qsos(qsos, &print_qso, (void *) fp);
}

void
free_qsos(struct adi_qso *qsos)
{
    struct adi_qso *qso,
                   *tmp;
    if (qsos) {
        HASH_ITER(hh, qsos, qso, tmp) {
            HASH_DEL(qsos, qso);
            free(qso->name);
            free(qso->qth);
            free(qso->their_call);
            free(qso->country);
            free(qso);
        }
    }
}
