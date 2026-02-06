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
#include "adif.h"
#include <stdio.h>

void
write_description(struct adi_qso *qso, FILE *fp)
{
    char           *cdata_open = "<![CDATA[";
    char           *cdata_close = "]]>";
    fprintf(fp, "<description>\n");
    fprintf(fp, "%s", cdata_open);
    fprintf(fp, "<h1>%s</h1>\n", qso->name);
    fprintf(fp, "<b><a href=\"https://www.qrz.com/db/%s\">"
	    "QRZ Page</a></b><br/>\n", qso->their_call);
    fprintf(fp, "<b>QTH</b>: %s<br/>\n", qso->qth);
    fprintf(fp, "<b>Number of QSOs</b>: %d<br/>\n", qso->num_qsos);
    fprintf(fp, "<b>Grid</b>: %s<br/>\n", qso->their_grid.mh);
    fprintf(fp, "<b>Country</b>: %s<br/>\n", qso->country);
    fprintf(fp, "<b>Distance</b>: %.1f km<br/>\n", qso->distance_km);
    fprintf(fp, "<b>Bearing</b>: %.1f&deg;</br>\n", qso->bearing_degrees);
    fprintf(fp, "%s", cdata_close);
    fprintf(fp, "</description>\n");
}

void
print_kml_point(struct adi_qso *qso, FILE *fp)
{
    fprintf(fp, "<Placemark>\n");
    fprintf(fp, "<name>%s</name>\n", qso->their_call);
    write_description(qso, fp);
    fprintf(fp, "<Point><coordinates>");
    fprintf(fp, "%.6f,%.6f,0",
	    qso->their_grid.lon_center, qso->their_grid.lat_center);
    fprintf(fp, "</coordinates></Point>\n");
    fprintf(fp, "</Placemark>\n");
}

void
print_kml_box(struct adi_qso *qso, FILE *fp)
{
    struct maidenhead *mh = NULL;
    if (!qso || !fp) {
	return;
    }
    mh = &qso->their_grid;
    fprintf(fp, "<Placemark>\n");
    fprintf(fp, "<name>%s grid</name>\n", qso->their_call);
    fprintf(fp, "<LineString><tessellate>1</tessellate>\n");
    fprintf(fp, "<coordinates>");
    fprintf(fp, "%.6f,%.6f,0\n%.6f,%.6f,0\n%.6f,%.6f,0\n",
	    // sw corner
	    mh->lon_sw_corner, mh->lat_sw_corner,
	    // nw corner
	    mh->lon_sw_corner, mh->lat_sw_corner + mh->lat_res_degrees,
	    // ne corner
	    mh->lon_sw_corner + mh->lon_res_degrees,
	    mh->lat_sw_corner + mh->lat_res_degrees);
    fprintf(fp, "%.6f,%.6f,0\n%.6f,%.6f,0",
	    // se corner
	    mh->lon_sw_corner + mh->lon_res_degrees, mh->lat_sw_corner,
	    // sw corner
	    mh->lon_sw_corner, mh->lat_sw_corner);
    fprintf(fp, "</coordinates>\n");
    fprintf(fp, "</LineString>\n");
    fprintf(fp, "</Placemark>\n");
}


int
print_kml_record(struct adi_qso *qso, void *arg)
{
    FILE           *fp = (FILE *) arg;
    print_kml_point(qso, fp);
    print_kml_box(qso, fp);
    return 0;
}

int
count_qsos(struct adi_qso *qso, void *arg)
{
    (void) (qso); // unused
    int            *i = (int *) arg;
    (*i) += 1;
    return 0;
}

void
write_kml(FILE *fp, struct adi_qso *qsos)
{
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp,
	    "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n<Document>\n");
    fprintf(fp, "<name>KO6IUE ADIF to KML converter</name>\n");
    walk_qsos(qsos, &print_kml_record, fp);
    fprintf(fp, "</Document>\n</kml>\n");
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
