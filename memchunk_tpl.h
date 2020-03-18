/* memchunk_tpl.h -- inline functions to copy small data chunks.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* Returns the chunk size */
uint32_t CHUNKSIZE(void) {
    return sizeof(memchunk_t);
}

/* Behave like memcpy, but assume that it's OK to overwrite at least
   memchunk_t bytes of output even if the length is shorter than this,
   that the length is non-zero, and that `from` lags `out` by at least
   sizeof memchunk_t bytes (or that they don't overlap at all or simply that
   the distance is less than the length of the copy).

   Aside from better memory bus utilisation, this means that short copies
   (memchunk_t bytes or fewer) will fall straight through the loop
   without iteration, which will hopefully make the branch prediction more
   reliable. */
uint8_t* CHUNKCOPY(uint8_t *out, uint8_t const *from, unsigned len) {
    memchunk_t chunk;
    --len;
    loadchunk(from, &chunk);
    storechunk(out, &chunk);
    out += (len % sizeof(memchunk_t)) + 1;
    from += (len % sizeof(memchunk_t)) + 1;
    len /= sizeof(memchunk_t);
    while (len > 0) {
        loadchunk(from, &chunk);
        storechunk(out, &chunk);
        out += sizeof(memchunk_t);
        from += sizeof(memchunk_t);
        --len;
    }
    return out;
}

/* Behave like chunkcopy, but avoid writing beyond of legal output. */
uint8_t* CHUNKCOPY_SAFE(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe) {
    if ((safe - out) < (ptrdiff_t)sizeof(memchunk_t)) {
        if (len & 8) {
            memcpy(out, from, 8);
            out += 8;
            from += 8;
        }
        if (len & 4) {
            memcpy(out, from, 4);
            out += 4;
            from += 4;
        }
        if (len & 2) {
            memcpy(out, from, 2);
            out += 2;
            from += 2;
        }
        if (len & 1) {
            *out++ = *from++;
        }
        return out;
    }
    return CHUNKCOPY(out, from, len);
}

/* Perform short copies until distance can be rewritten as being at least
   sizeof memchunk_t.

   This assumes that it's OK to overwrite at least the first
   2*sizeof(memchunk_t) bytes of output even if the copy is shorter than this.
   This assumption holds because inflate_fast() starts every iteration with at
   least 258 bytes of output space available (258 being the maximum length
   output from a single token; see inflate_fast()'s assumptions below). */
uint8_t* CHUNKUNROLL(uint8_t *out, unsigned *dist, unsigned *len) {
    unsigned char const *from = out - *dist;
    memchunk_t chunk;
    while (*dist < *len && *dist < sizeof(memchunk_t)) {
        loadchunk(from, &chunk);
        storechunk(out, &chunk);
        out += *dist;
        *len -= *dist;
        *dist += *dist;
    }
    return out;
}

/* Copy DIST bytes from OUT - DIST into OUT + DIST * k, for 0 <= k < LEN/DIST.
   Return OUT + LEN. */
uint8_t* CHUNKMEMSET(uint8_t *out, unsigned dist, unsigned len) {
    /* Debug performance related issues when len < sizeof(uint64_t):
       Assert(len >= sizeof(uint64_t), "chunkmemset should be called on larger chunks"); */
    Assert(dist > 0, "cannot have a distance 0");

    unsigned char *from = out - dist;
    memchunk_t chunk;
    unsigned sz = sizeof(chunk);
    if (len < sz) {
        do {
            *out++ = *from++;
            --len;
        } while (len != 0);
        return out;
    }

#ifdef HAVE_CHUNKMEMSET_1
    if (dist == 1) {
        chunkmemset_1(from, &chunk);
    } else
#endif
#ifdef HAVE_CHUNKMEMSET_2
    if (dist == 2) {
        chunkmemset_2(from, &chunk);
    } else
#endif
#ifdef HAVE_CHUNKMEMSET_3
    if (dist == 3) {
        return chunkmemset_3(out, from, dist, len);
    } else
#endif
#ifdef HAVE_CHUNKMEMSET_4
    if (dist == 4) {
        chunkmemset_4(from, &chunk);
    } else
#endif
#ifdef HAVE_CHUNKMEMSET_6
    if (dist == 6) {
        return chunkmemset_6(out, from, dist, len);
    } else
#endif
#ifdef HAVE_CHUNKMEMSET_8
    if (dist == 8) {
        chunkmemset_8(from, &chunk);
    } else
#endif
    if (dist == sz) {
        loadchunk(from, &chunk);
    } else {
        out = CHUNKUNROLL(out, &dist, &len);
        return CHUNKCOPY(out, out - dist, len);
    }

    unsigned rem = len % sz;
    len -= rem;
    while (len) {
        storechunk(out, &chunk);
        out += sz;
        len -= sz;
    }

    /* Last, deal with the case when LEN is not a multiple of SZ. */
    if (rem)
        memcpy(out, from, rem);
    out += rem;

    return out;
}

uint8_t* CHUNKMEMSET_SAFE(uint8_t *out, unsigned dist, unsigned len, unsigned left) {
    if (left < (unsigned)(3 * sizeof(memchunk_t))) {
        while (len > 0) {
            *out = *(out - dist);
            out++;
            --len;
        }
        return out;
    }
    return CHUNKMEMSET(out, dist, len);
}
