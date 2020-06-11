/* functable.h -- Struct containing function pointers to optimized functions
 * Copyright (C) 2017 Hans Kristian Rosbach
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef FUNCTABLE_H_
#define FUNCTABLE_H_

#include "deflate.h"

struct functable_s {
    Pos      (* insert_string)      (deflate_state *const s, const Pos str, unsigned int count);
    Pos      (* quick_insert_string)(deflate_state *const s, const Pos str);
    uint32_t (* adler32)            (uint32_t adler, const unsigned char *buf, size_t len);
    uint32_t (* crc32)              (uint32_t crc, const unsigned char *buf, uint64_t len);
    void     (* slide_hash)         (deflate_state *s);
    int32_t  (* compare258)         (const unsigned char *src0, const unsigned char *src1);
    int32_t  (* longest_match)      (deflate_state *const s, Pos cur_match);
    uint32_t (* chunksize)          (void);
    uint8_t* (* chunkcopy)          (uint8_t *out, uint8_t const *from, unsigned len);
    uint8_t* (* chunkcopy_safe)     (uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
    uint8_t* (* chunkunroll)        (uint8_t *out, unsigned *dist, unsigned *len);
    uint8_t* (* chunkmemset)        (uint8_t *out, unsigned dist, unsigned len);
    uint8_t* (* chunkmemset_safe)   (uint8_t *out, unsigned dist, unsigned len, unsigned left);
};

ZLIB_INTERNAL extern __thread struct functable_s functable;

#endif
