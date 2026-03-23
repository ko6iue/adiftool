/*
 * Copyright (c) 2026, Matt Massie (KO6IUE). See LICENSE file.
 */
#include <stdio.h>
#include <assert.h>

#include "./latlon.h"

void
print_latlon(char *label, latlon_t ll)
{
    printf("%16s: lat: %f lon: %f\n", label, ll.lat, ll.lon);
}

int
main(void)
{
    // charles de gaulle
    latlon_t        paris = {.lat = 49.009341,.lon = 2.523797 };
    // heathrow
    latlon_t        london = {.lat = 51.467771,.lon = -0.459082 };
    latlon_t        dest;
    float           distance,
                    bearing;

    print_latlon("paris", paris);
    print_latlon("london", london);

    assert(latlon_distance_km(paris, paris) == 0);

    distance = latlon_distance_km(paris, london);
    printf("%16s: %f\n", "distance", distance);
    assert(distance >= 345.960 && distance <= 345.962);

    bearing = latlon_bearing_degrees(paris, london);
    printf("%16s: %f\n", "bearing", bearing);

    assert(latlon_destination(paris, distance, bearing, &dest) == 0);
    print_latlon("destination", dest);

    assert(dest.lat >= london.lat - london.lat * 0.001 &&
           dest.lat <= london.lat + london.lat * .001);


    printf("OK\n");
    return 0;
}
