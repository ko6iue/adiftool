/*
 * Copyright (c) 2026, Matt Massie (KO6IUE). See LICENSE file.
 */
#include <stdio.h>
#include <assert.h>

#include "./maidenhead.h"

int
main(void)
{
    maidenhead_t    from;
    maidenhead_t    to;
    float           distance,
                    bearing;

    // all characters valid
    assert(populate_maidenhead(&from, "JK42", 4) == 4);
    // Bad character position 0
    assert(populate_maidenhead(&to, "ZZ", 2) == 0);
    // Bad character position 1
    assert(populate_maidenhead(&to, "CZ", 2) == 1);
    // Bad character position 2 
    assert(populate_maidenhead(&to, "CCA1", 4) == 2);
    // Odd number of characters, invalid args
    assert(populate_maidenhead(&to, "CCA", 3) == -1);
    assert(populate_maidenhead(&to, "CC1", 3) == -1);
    assert(populate_maidenhead(&to, "CC10A", 5) == -1);
    // Too many characters
    assert(populate_maidenhead(&to, "AB12CD34EF56GH78", 12) == -1);
    // Not enough characters
    assert(populate_maidenhead(&to, "", 0) == -1);

    assert(populate_maidenhead(&from, "AA00AA00", 8) == 8);
    assert(populate_maidenhead(&to, "AR00AX09", 8) == 8);
    // maidenhead_print(&from);
    // maidenhead_print(&to);
    distance = maidenhead_distance_km(&from, &to);
    bearing = maidenhead_bearing_degrees(&from, &to);
    // printf("distance=%f bearing=%f\n", distance, bearing);
    assert((distance >= 19013.80 && distance <= 19013.90));
    assert((bearing == 0.0));



    printf("OK\n");

    return 0;
}
