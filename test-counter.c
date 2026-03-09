/*
 * Copyright (c) 2026, Matt Massie (KO6IUE). See LICENSE file.
 */
#include <stdio.h>
#include <assert.h>
#include "./counter.h"

void
assert_counter_value(adif_counter_t *cp, char *name, int val){
	adif_counter_t *query;
	HASH_FIND_STR(cp, name, query);
	assert(query != NULL);
	assert(strcmp(query->name, name) == 0);
	assert(query->count == val);
}

int
main(void)
{
    adif_counter_t *c1 = NULL, *c2 = NULL, *merged = NULL, *n = NULL;

    // c1
    counter_increment(&c1, "X", 1);
    counter_increment(&c1, "X", 3);
    counter_increment(&c1, "Y", 10);
    counter_increment(&c1, "Y", 100);
    
    // c2
    counter_increment(&c2, "Z", 123);
    counter_increment(&c2, "X", 40);

    // merge c1 c2
    counter_merge(&merged, c1);
    counter_merge(&merged, c2);

    // c1 check
    assert(HASH_COUNT(c1) == 2);
    assert_counter_value(c1, "X", 4);
    assert_counter_value(c1, "Y", 110);

    // c2 check
    assert_counter_value(c2, "X", 40);
    assert_counter_value(c2, "Z", 123);
    assert(HASH_COUNT(c2) == 2);

    // merge check
    assert_counter_value(merged, "X", 44);
    assert_counter_value(merged, "Y", 110);
    assert_counter_value(merged, "Z", 123);
    assert(HASH_COUNT(merged) == 3);

    // free twice
    counter_free(&c1);
    counter_free(&c1);

    counter_free(&c2);
    counter_free(&merged);
    counter_free(&n);

    printf("OK\n");
    return 0;
}
