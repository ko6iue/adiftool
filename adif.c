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
print_qso(struct adi_qso *qso, void *arg, int last_item)
{
    (void) last_item;
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

// Minimum usable QSO
int
valid_qso(struct adi_qso *qso)
{
    if (!qso) {
        return 0;
    }
    if (!qso->their_call || strlen(qso->their_call) <= 0) {
        return 0;
    }
    if (maidenhead_is_null(&qso->their_grid)) {
        return 0;
    }
    return 1;
}

struct adi_qso *
load_qsos_fp(FILE *fp)
{
    size_t          fsize;
    char           *data;
    struct adi_qso *rval;

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

    rval = load_qsos_mem(data, fsize);
    free(data);
    return rval;
}

void
free_qso_strdups(struct adi_qso *qso)
{
    if (qso) {
        free(qso->name);
        free(qso->qth);
        free(qso->their_call);
        free(qso->country);
    }
}

void
free_qsos(struct adi_qso *qsos)
{
    struct adi_qso *qso,
                   *tmp;
    if (qsos) {
        HASH_ITER(hh, qsos, qso, tmp) {
            HASH_DEL(qsos, qso);
            free_qso_strdups(qso);
            free(qso);
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

struct adi_qso *
load_qsos_mem(char *buf, size_t buf_len)
{
    (void) buf_len;
    struct adi_qso *qsos = NULL,
        *qso,
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
        "my_gridsquare", NULL
    };

    qso = (struct adi_qso *) malloc(sizeof *qso);
    assert(qso);
    memset(qso, 0, sizeof(*qso));

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
            if (valid_qso(qso)) {
                // process this useable QSO record
                HASH_FIND_STR(qsos, qso->their_call, tmp);
                if (tmp == NULL) {
                    // Add QSO from a new station
                    qso->distance_km =
                        maidenhead_distance_km(&qso->my_grid,
                                               &qso->their_grid);
                    qso->bearing_degrees =
                        maidenhead_bearing_degrees(&qso->my_grid,
                                                   &qso->their_grid);
                    qso->num_qsos = 1;
                    tmp = (struct adi_qso *) malloc(sizeof(*tmp));
                    memcpy(tmp, qso, sizeof(*qso));
                    // Add a copy of this new valid QSO
                    HASH_ADD_STR(qsos, their_call, tmp);
                    // Set our working QSO to zero
                    memset(qso, 0, sizeof(*qso));
                } else {
                    // Process QSO from existing station
                    tmp->num_qsos += 1;
                }
            }
            // Reset working QSO and start again
            free_qso_strdups(qso);
            memset(qso, 0, sizeof(*qso));
            break;
        case 1:                // call
            qso->their_call = strdup(value);
            break;
        case 2:                // name
            qso->name = strdup(value);
            break;
        case 3:                // country
            qso->country = strdup(value);
            break;
        case 4:                // qth
            qso->qth = strdup(value);
            break;
        case 5:                // gridsquare
            populate_maidenhead(&qso->their_grid, value, strlen(value));
            break;
        case 6:                // my_gridsquare
            populate_maidenhead(&qso->my_grid, value, strlen(value));
            break;
        }
    }

    // Free our working QSO
    free_qso_strdups(qso);
    free(qso);
    return qsos;
}


int
walk_qsos(struct adi_qso *qsos,
          int (*cb)(struct adi_qso *, void *arg, int is_last), void *arg)
{
    int             rval,
                    nitems;
    struct adi_qso *qso,
                   *tmp;
    if (qsos && cb) {
        nitems = HASH_COUNT(qsos);
        HASH_ITER(hh, qsos, qso, tmp) {
            nitems--;
            rval = (*cb) (qso, arg, nitems == 0);
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
