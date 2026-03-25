// example1.c - Demonstrates miniz.c's compress() and uncompress()
// functions (same as zlib's).
// Public domain, May 15 2011, Rich Geldreich, richgel99@gmail.com. See
// "unlicense" statement at the end of tinfl.c.
#include <stdio.h>
#include "miniz.h"

int
data_compress(unsigned char **cmp, size_t *cmp_len, unsigned char *uncmp,
              size_t uncmp_len)
{
    int             rval;

    if (!(cmp && cmp_len && uncmp)) {
	printf("Bad args\n");
        // bad args
        return -1;
    }
    *cmp_len = compressBound(uncmp_len);
    *cmp = (unsigned char *) malloc(*cmp_len);
    if (!*cmp) {
        return -1;
    }
    rval = compress(*cmp, cmp_len, uncmp, uncmp_len);
    if (rval != Z_OK) {
        free(*cmp);
	printf("Compress failed\n");
        return -1;
    }
    printf("Compression from %zu to %zu bytes\n", uncmp_len, *cmp_len);
    return 0;
}
