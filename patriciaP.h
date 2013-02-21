/*
 * Copyright (c) 2012, Todd Hayton <thayton@neekanee.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 * Redistributions of source code must retain the above copyright 
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PATRICIAP_H
#define PATRICIAP_H

#include "patriciaP.h"

typedef enum {
    PN_LEAF,
    PN_NODE
} pn_type_t;

typedef struct pnode {
    pn_type_t     pn_type; /* leaf or internal node */
    struct pnode *pn_up;   /* parent pointer */
    union {
        struct { /* leaf node */
            void    *pn_Key;
            size_t   pn_Keysz;
            void    *pn_Val;
            size_t   pn_Valsz;
        } pn_leaf;
        struct { /* internal node */
            int            pn_Bit;
            struct pnode * pn_Cld[2]; /* children */
        } pn_node;
    } pn_u;
} pnode_t;

#define pn_key    pn_u.pn_leaf.pn_Key
#define pn_keysz  pn_u.pn_leaf.pn_Keysz
#define pn_val    pn_u.pn_leaf.pn_Val
#define pn_valsz  pn_u.pn_leaf.pn_Valsz
#define pn_bit    pn_u.pn_node.pn_Bit
#define pn_cld    pn_u.pn_node.pn_Cld

struct ptrie {
    pnode_t     *pt_root; /* top of trie */
    pnode_t     *pt_list; /* freelist of patricia nodes */
    size_t       pt_size; /* num nodes in trie */

    uint32_t     pt_parms; /* configurable settings */
    ptrie_iter_t pt_iter;  /* default iterator */

    void       *(*pt_malloc_func)(size_t); /* malloc function */
    void        (*pt_free_func)(void *);   /* free function */

    size_t       pt_keysz;                  /* fixed size keys */
    size_t     (*pt_keysz_func)(void *key); /* variable length string keys */
};

#ifndef ABSVAL
#define ABSVAL(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef NOT
#define NOT !
#endif

#define BITS_PER_BYTE 8

/*
 * Given child index 0 return 1
 * Given child index 1 return 0
 */
#define OTHER_CLDIDX(c) ((c) ^ 1)

/*
 * Tests to determine if a given node is 
 * the left or right child of its parent.
 */
#define NODE_IS_LCLD(n) ((n) == (n)->pn_up->pn_cld[0])
#define NODE_IS_RCLD(n) ((n) == (n)->pn_up->pn_cld[1])

/*
 * Determine if bit at index 'bit' is set in 
 * 'key' of size 'keysz' bytes.
 * 
 * Bits are counted from most significant bit to
 * least significant bit, with the most significant 
 * bit indexed at 1.
 *
 * Eg, given bit string 00100000 the only bit that is
 * set is at index 3:
 *
 *    12345678
 *    --------
 *    00100000
 */
static inline int getbit(void *key, size_t keysz, int bit)
{
    uint8_t *ptr = (uint8_t *)key;
    if (bit) {
        bit--;
        /* return 0 if bit out of range */
        if ((bit >> 3) > keysz)
            return 0;
        return ptr[bit >> 3] & 1<<(7 - (bit & 7)) ? 1 : 0;
    }
    return 0;
}

/***********************************************************###**
 * Return 0 if keys are equal
 *
 * If keys are not equal then the absolute value of keycmp()
 * will be the first bit where the keys differ. 
 *
 * The sign of the return value tells us which key was 1 and
 * which was 0 at offset diffbit-
 *
 * If the return value is positive it means that:
 *
 *     bit(key1, diffbit) = 1
 *     bit(key2, diffbit) = 0
 *
 * If the return value is negative it means that
 *
 *     bit(key1, diffbit) = 0
 *     bit(key2, diffbit) = 1
 ***********************************************************###*/
static inline int keycmp(void *key1, size_t key1sz, void *key2, size_t key2sz)
{
    int      i;
    int      j;
    uint8_t *ptr1 = (uint8_t *)key1;
    uint8_t *ptr2 = (uint8_t *)key2;

    for (i = 0; *ptr1 == *ptr2; ptr1++, ptr2++, i++) {
        if (key1sz == key2sz) {
            if (i+1 == key1sz)
                return 0;
        } 
    }

    i = i*8 + 1;

    for (j = 1<<7; (*ptr1 & j) == (*ptr2 & j); i++, j >>= 1)
        /**/;
    
    return (*ptr1 & j) ? i : -i;
}

static inline int keyseq(void *key1, size_t key1sz, void *key2, size_t key2sz)
{
    return key1sz == key2sz && memcmp(key1, key2, key1sz) == 0;
}

/* 
 * New patricia nodes are put onto a freelist PN_FREELIST_BLKSZ 
 * nodes at a time 
 */
#define PN_FREELIST_BLKSZ 1024

#endif /* PATRICIAP_H */
