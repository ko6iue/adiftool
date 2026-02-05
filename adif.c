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

// See adi_qso for keys we care about
// KEY_LEN is large enough to hold them
// We ignore all others
#define KEY_LEN 32
#define VAL_LEN 128

#define PMATCH_LEN(P) ((P)->rm_eo - (P)->rm_so)

int
print_qso(struct adi_qso *qso, void *arg)
{
    printf("**********\n");
    printf("       call: %s\n", qso->their_call);
    printf("     # QSOs: %d\n", qso->num_qsos);
    printf("       name: %s\n", qso->name);
    printf("    country: %s\n", qso->country);
    printf("        qth: %s\n", qso->qth);
    printf("    my grid\n");
    maidenhead_print(&qso->my_grid);
    printf(" their grid\n");
    maidenhead_print(&qso->their_grid);
    printf("distance km: %f\n", qso->distance_km);
    printf("bearing deg: %f\n", qso->bearing_degrees);
    printf("**********\n");
    return 0;
}

char           *
my_strcpy(char *dest, int dstlen, char *src, int srclen)
{
    int             safe_len = srclen;
    if (srclen > (dstlen - 1)) {
	safe_len = dstlen - 1;
    }
    memcpy(dest, src, safe_len);
    dest[safe_len] = '\0';
    return dest;
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
    size_t          nmatch = 3,	// all, key, value
	key_len,
	val_len,
	line_len = 0;
    regmatch_t      pmatch[nmatch];
    char           *line = NULL,
	key[KEY_LEN],
	val[KEY_LEN];

    if (!fp) {
	return NULL;
    }

    rval =
	regcomp(&regex, "^<([^:0-9]+){1,1}[:0-9]*>(.*)$",
		REG_EXTENDED | REG_NEWLINE);
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
	    // We silently ignore keys longer than KEY_LEN
	    // since we don't process them anyway
	    key_len = PMATCH_LEN(&pmatch[1]);
	    val_len = PMATCH_LEN(&pmatch[2]);
	    if (key_len < KEY_LEN) {
		my_strcpy(key, sizeof(key), line + pmatch[1].rm_so,
			  key_len);
		my_strcpy(val, sizeof(val), line + pmatch[2].rm_so,
			  val_len);

		if (!strcmp(key, "call")) {
		    my_strcpy(qso->their_call, sizeof(qso->their_call),
			      val, val_len);
		} else if (!strcmp(key, "name")) {
		    my_strcpy(qso->name, sizeof(qso->name), val, val_len);
		} else if (!strcmp(key, "country")) {
		    my_strcpy(qso->country, sizeof(qso->country),
			      val, val_len);
		} else if (!strcmp(key, "qth")) {
		    my_strcpy(qso->qth, sizeof(qso->qth), val, val_len);
		} else if (!strcmp(key, "gridsquare")) {
		    populate_maidenhead(&qso->their_grid, val, val_len);
		} else if (!strcmp(key, "my_gridsquare")) {
		    populate_maidenhead(&qso->my_grid, val, val_len);
		} else if (!strcmp(key, "eor")) {
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
			    qso = (struct adi_qso *) malloc(sizeof(*qso));
			    if (qso == NULL) {
				return NULL;
			    }
			} else {
			    // Station we already know about: process
			    // summary info.
			    query->num_qsos += 1;
			    // TODO: see if we have better maidenhead info
			    // Check if they moved etc, summarize band
			    // info
			}
		    }
		    memset(qso, 0, sizeof(*qso));
		} else {
		    // printf("Ignoring key=%s\n", key);
		}

		/*
		 * printf("%d (%d,%d) %s=%s %s", rval, pmatch[2].rm_so,
		 * pmatch[2].rm_eo, key, val, line); 
		 */
	    }
	}
    }

    free(qso);			// free last qso we created and didn't
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
print_qsos(struct adi_qso *qsos)
{
    return walk_qsos(qsos, &print_qso, NULL);
}

void
free_qsos(struct adi_qso *qsos)
{
    struct adi_qso *qso,
                   *tmp;
    if (qsos) {
	HASH_ITER(hh, qsos, qso, tmp) {
	    HASH_DEL(qsos, qso);
	    free(qso);
	}
    }
}
