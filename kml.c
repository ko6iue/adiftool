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
#include "./kml.h"

void
write_description(adif_station_t *station, FILE *fp)
{
    char           *cdata_open = "<![CDATA[";
    char           *cdata_close = "]]>";
    fprintf(fp, "<description>\n");
    fprintf(fp, "%s", cdata_open);
    if (station->name) {
        fprintf(fp, "<h1>%s</h1>\n", station->name);
    }
    if (station->their_call) {
        fprintf(fp, "<b><a href=\"https://www.qrz.com/db/%s\">"
                "QRZ Page</a></b><br/>\n", station->their_call);
    }
    if (station->qth) {
        fprintf(fp, "<b>QTH</b>: %s<br/>\n", station->qth);
    }
    fprintf(fp, "<b>Number of QSOs</b>: %d<br/>\n", station->num_qsos);

    if (station->country) {
        fprintf(fp, "<b>Country</b>: %s<br/>\n", station->country);
    }

    if (!maidenhead_is_null(&station->their_grid)) {
        fprintf(fp, "<b>Grid</b>: %s<br/>\n", station->their_grid.mh);
    }

    if (!maidenhead_is_null(&station->my_grid)) {
        fprintf(fp, "<b>Distance</b>: %.1f km<br/>\n",
                station->distance_km);
        fprintf(fp, "<b>Bearing Sent</b>: %.1f&deg;</br>\n",
                station->bearing_sent);
        fprintf(fp, "<b>Bearing Received</b>: %.1f&deg;</br>\n",
                station->bearing_rcvd);
    }

    fprintf(fp, "%s", cdata_close);
    fprintf(fp, "</description>\n");
}

void
print_kml_point_style(FILE *fp)
{
    int             i;
    char           *base_href =
        "http://maps.google.com/mapfiles/kml/paddle/%d.png";

    for (i = 1; i <= 10; i++) {
        fprintf(fp, "<Style id=\"pointStyle%02d\">\n", i);
        fprintf(fp, "<IconStyle><Icon>\n");
        fprintf(fp, "<href>");
        fprintf(fp, base_href, i);
        fprintf(fp, "</href>\n");
        fprintf(fp, "</Icon></IconStyle>\n");
        fprintf(fp, "</Style>\n");
    }
}

void
print_kml_point(adif_station_t *station, FILE *fp)
{
    int             icon_num = station->num_qsos;
    maidenhead_t   *mh;
    if (icon_num > 10) {
        icon_num = 10;
    }
    fprintf(fp, "<Placemark>\n");
    fprintf(fp, "<name>%s</name>\n", station->their_call);
    write_description(station, fp);
    fprintf(fp, "<styleUrl>#pointStyle%02d</styleUrl>\n", icon_num);
    fprintf(fp, "<Point><coordinates>");
    mh = &station->their_grid;
    fprintf(fp, "%.6f,%.6f,0", mh->random.lon, mh->random.lat);
    fprintf(fp, "</coordinates></Point>\n");
    fprintf(fp, "</Placemark>\n");
}

void
print_kml_box(adif_station_t *station, FILE *fp)
{
    maidenhead_t   *mh = NULL;
    const char     *fmt = "%.6f,%.6f,0\n";
    if (!station || !fp) {
        return;
    }
    mh = &station->their_grid;
    fprintf(fp, "<Placemark>\n");
    fprintf(fp, "<name>%s grid</name>\n", station->their_call);
    fprintf(fp, "<LineString><tessellate>1</tessellate>\n");
    fprintf(fp, "<coordinates>");
    fprintf(fp, fmt, mh->sw_corner.lon, mh->sw_corner.lat);
    fprintf(fp, fmt, mh->nw_corner.lon, mh->nw_corner.lat);
    fprintf(fp, fmt, mh->ne_corner.lon, mh->ne_corner.lat);
    fprintf(fp, fmt, mh->se_corner.lon, mh->se_corner.lat);
    fprintf(fp, fmt, mh->sw_corner.lon, mh->sw_corner.lat);
    fprintf(fp, "</coordinates>\n");
    fprintf(fp, "</LineString>\n");
    fprintf(fp, "</Placemark>\n");
}

int
print_kml_record(adif_station_t *station, FILE *fp)
{
    print_kml_point(station, fp);
    print_kml_box(station, fp);
    return 0;
}

void
write_kml(FILE *fp, adif_data_t *data)
{
    adif_station_t *station,
                   *tmp;
    if (!data) {
        return;
    }
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp,
            "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n<Document>\n");
    fprintf(fp, "<name>KO6IUE ADIF to KML converter</name>\n");
    print_kml_point_style(fp);
    HASH_ITER(hh, data->stations, station, tmp) {
        print_kml_record(station, fp);
    }
    fprintf(fp, "</Document>\n</kml>\n");
}
