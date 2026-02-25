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
#include "./geojson.h"
#include "./cmdline.h"

int
runit(struct gengetopt_args_info *args_info)
{
    FILE           *infp = stdin;
    adif_station_t *stations = NULL;
    FILE           *outfp = stdout;

    if (strcmp(args_info->input_arg, "-")) {
        infp = fopen(args_info->input_arg, "r");
    }
    if (infp == NULL) {
        fprintf(stderr, "fopen input file error: %s\n",
                args_info->input_arg);
        return EXIT_FAILURE;
    }
    stations = load_stations_fp(infp);
    fclose(infp);

    if (stations == NULL) {
        return EXIT_FAILURE;
    }

    if (strcmp(args_info->output_arg, "-")) {
        outfp = fopen(args_info->output_arg, "w");
    }
    if (!outfp) {
        fprintf(stderr, "fopen output file error: %s\n",
                args_info->output_arg);
        return EXIT_FAILURE;
    }

    if (args_info->geojson_flag) {
        write_geojson(outfp, stations);
    } else {
        write_kml(outfp, stations);
    }
    fclose(outfp);

    printf("Processed %d stations\n", HASH_COUNT(stations));

    free_stations(stations);
    return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
    int             rval;
    struct gengetopt_args_info args_info;
    if (cmdline_parser(argc, argv, &args_info) != 0) {
        exit(EXIT_FAILURE);
    }
    rval = runit(&args_info);
    cmdline_parser_free(&args_info);
    return rval;
}
