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

#ifndef PATRICIA_H
#define PATRICIA_H

/* configurable parameters */
#define PTRIEPARM_KEYSZ       0
#define PTRIEPARM_KEYSZ_FUNC  1
#define PTRIEPARM_MALLOC_FUNC 2
#define PTRIEPARM_FREE_FUNC   3

typedef struct ptrie ptrie_t;
typedef struct ptrie_iter ptrie_iter_t;

struct ptrie_iter {
    void *pn; /* current node */
    void *root; /* root of subtree we're iterating over */
};

/* public api */
extern ptrie_t *ptrie_new(void);
extern void     ptrie_free(ptrie_t *ptrie);

extern void     ptrie_set_parm(ptrie_t *ptrie, uint32_t parm, void *value);
extern void    *ptrie_get_parm(ptrie_t *ptrie, uint32_t parm);

extern void     ptrie_add(ptrie_t *ptrie, void *key, void *val);
extern void     ptrie_add2(ptrie_t *ptrie, void *key, void *val, void **pnode);

extern void    *ptrie_get(ptrie_t *ptrie, void *key);
extern void    *ptrie_get_prefix(ptrie_t *ptrie, void *prefix, size_t nbits);

extern void     ptrie_del(ptrie_t *ptrie, void *key);
extern void     ptrie_del_pnode(ptrie_t *ptrie, void *pnode);

extern int      ptrie_size(ptrie_t *ptrie);
extern int      ptrie_haskey(ptrie_t *ptrie, void *key);

extern void     ptrie_iter_init(ptrie_t *ptrie, void *root, ptrie_iter_t *iter);
extern int      ptrie_iter_next(ptrie_t *ptrie, ptrie_iter_t *iter, void **key, void **val);

/* iterate over items in the trie */
#define foreach_ptrie_keyval(ptrie, iter, key, val) \
    for (ptrie_iter_init(ptrie, 0, iter);           \
         ptrie_iter_next(ptrie, iter, (void **)key, (void **)val); /**/)

#define foreach_ptrie_key(ptrie, iter, key) \
    for (ptrie_iter_init(ptrie, 0, iter);   \
         ptrie_iter_next(ptrie, iter, (void **)key, (void **)0); /**/)

#define foreach_ptrie_val(ptrie, iter, val) \
    for (ptrie_iter_init(ptrie, 0, iter);   \
         ptrie_iter_next(ptrie, iter, (void **)0, (void **)val); /**/)

#define foreach_ptrie_keyval_with_prefix(ptrie, iter, prefix, nbits, key, val) \
    for (ptrie_iter_init(ptrie, ptrie_get_prefix(ptrie, prefix, nbits), iter); \
         ptrie_iter_next(ptrie, iter, (void **)key, (void **)val); /**/)

#define foreach_ptrie_key_with_prefix(ptrie, iter, prefix, nbits, key)         \
    for (ptrie_iter_init(ptrie, ptrie_get_prefix(ptrie, prefix, nbits), iter); \
         ptrie_iter_next(ptrie, iter, (void **)key, (void **)0); /**/)

#define foreach_ptrie_val_with_prefix(ptrie, iter, prefix, nbits, val)         \
    for (ptrie_iter_init(ptrie, ptrie_get_prefix(ptrie, prefix, nbits), iter); \
         ptrie_iter_next(ptrie, iter, (void **)0, (void **)val); /**/)


#endif /* PATRICIA_H */
