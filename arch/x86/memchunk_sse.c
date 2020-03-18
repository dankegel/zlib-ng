/* memchunk_sse.c -- SSE inline functions to copy small data chunks.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#ifndef MEMCHUNK_SSE_H_
#define MEMCHUNK_SSE_H_

#include "zbuild.h"
#include "zutil.h"

#ifdef X86_SSE2
#include <immintrin.h>

typedef __m128i memchunk_t;

#define HAVE_CHUNKMEMSET_1
#define HAVE_CHUNKMEMSET_2
#define HAVE_CHUNKMEMSET_4
#define HAVE_CHUNKMEMSET_8

static inline void chunkmemset_1(uint8_t *from, memchunk_t *chunk) {
    *chunk = _mm_set1_epi8(*(int8_t *)from);
}

static inline void chunkmemset_2(uint8_t *from, memchunk_t *chunk) {
    *chunk = _mm_set1_epi16(*(int16_t *)from);
}

static inline void chunkmemset_4(uint8_t *from, memchunk_t *chunk) {
    *chunk = _mm_set1_epi32(*(int32_t *)from);
}

static inline void chunkmemset_8(uint8_t *from, memchunk_t *chunk) {
    *chunk = _mm_set1_epi64x(*(int64_t *)from);
}

static inline void loadchunk(uint8_t const *s, memchunk_t *chunk) {
    *chunk = _mm_loadu_si128((__m128i *)s);
}

static inline void storechunk(uint8_t *out, memchunk_t *chunk) {
    _mm_storeu_si128((__m128i *)out, *chunk);
}

#define CHUNKSIZE        chunksize_sse2
#define CHUNKCOPY        chunkcopy_sse2
#define CHUNKCOPY_SAFE   chunkcopy_safe_sse2
#define CHUNKUNROLL      chunkunroll_sse2
#define CHUNKMEMSET      chunkmemset_sse2
#define CHUNKMEMSET_SAFE chunkmemset_safe_sse2

#include "memchunk_tpl.h"

#endif
#endif
