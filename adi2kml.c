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
#include "./adif.h"
#include "./kml.h"

int
count_qsos(struct adi_qso *qso, void *arg)
{
    (void) (qso);               // unused
    int            *i = (int *) arg;
    (*i) += 1;
    return 0;
}

int
main(int argc, char *argv[])
{
    FILE           *fp = NULL;
    struct adi_qso *qsos = NULL;
    FILE           *kmlfp;
    int             num_qsos = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [adi file] [kml file]\n", argv[0]);
        fprintf(stderr, "Example: %s my.adi my.kml\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "File error: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    qsos = load_qsos_fp(fp);
    fclose(fp);
    if (qsos == NULL)
        exit(EXIT_FAILURE);

    kmlfp = fopen(argv[2], "w");
    if (!kmlfp) {
        fprintf(stderr, "Error writing to %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    write_kml(kmlfp, qsos);
    fclose(kmlfp);

    walk_qsos(qsos, &count_qsos, &num_qsos);
    printf("Processed %d QSOs\n", num_qsos);

    // print_qsos(stdout, qsos);

    free_qsos(qsos);
    return 0;
}
