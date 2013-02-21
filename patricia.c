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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "patricia.h"
#include "patriciaP.h"

static pnode_t *newpar(ptrie_t *pt, int diffbit, pnode_t *cld1, pnode_t *cld2);
static pnode_t *newcld(ptrie_t *pt, void *key, size_t keysz, void *val);

static pnode_t *pnode_new(ptrie_t *pt, pn_type_t type);
static void     pnode_free(ptrie_t *pt, pnode_t *pn);

static pnode_t *ptrie_del0(ptrie_t *pt, void *key, size_t keysz, pnode_t *pn);
static void    *fmalloc(size_t size);
static size_t keysize(ptrie_t *pt, void *key);

/***********************************************************###**
 * Alloc/initialize a new ptrie along with a freelist for
 * the pnodes.
 ***********************************************************###*/
ptrie_t *
ptrie_new(void)
{
    int      i;
    ptrie_t *pt;
    pnode_t *pn;

    pt = fmalloc(sizeof(*pt));
    pt->pt_root = NULL;
    pt->pt_list = NULL;
    pt->pt_size = 0;
    pt->pt_keysz = 0;
    pt->pt_keysz_func = (size_t (*)(void *))strlen; /* default assumes string keys */
    pt->pt_malloc_func = fmalloc;
    pt->pt_free_func = free;

    for (i = 0; i < PN_FREELIST_BLKSZ; i++) {
        pn = pt->pt_malloc_func(sizeof(*pn));
        pn->pn_cld[0] = pt->pt_list;
        pt->pt_list = pn;
    }

    return pt;
}

/***********************************************************###**
 * ptrie destructor
 ***********************************************************###*/
void
ptrie_free(ptrie_t *ptrie)
{
    /* XXX TODO */
    return;
}

void 
ptrie_add(ptrie_t *pt, void *key, void *val)
{
    ptrie_add2(pt, key, val, NULL);
}

/*
 *
 * (1) Insert 0001 into an empty trie.
 *  
 *     Since the trie is empty we just create a new leaf node
 *     into which we store the 0001 key:
 *
 *        (0001)
 *
 * (2) Insert 1001 into trie
 *
 *     Search through trie using 1001 as the search key until
 *     until we arrive at a leaf node. Since there's only one
 *     node in the trie we stop at leaf node (0001).
 * 
 *     Now we compare the key being inserted, 1001, with the
 *     key in the leaf node we reached:
 *
 *         1234
 *         ----
 *         0001   # leaf node key
 *         1001   # key being inserted
 *
 *     The first bit at which the keys differ is bit 1, so we
 *     create an internal node whose difference bit is set to 
 *     1. Since bit 1 of key 0001 is 0 we make this key the 
 *     left child of our internal node. Similarly, since bit 
 *     1 of key 1001 is 1 we make this node the right child 
 *     of our new internal node. The trie now looks like this:
 *  
 *                [1]
 *               /   \
 *         (0001)     (1001)
 *
 * (3) Insert 0010 into trie
 *
 *     Search through trie using 0010 as the search key until
 *     until we arrive at a leaf node. Since the first node in 
 *     the trie is an internal node with its difference bit set 
 *     to 1, we we examine bit 1 of our search key 0010. 
 *
 *     Bit 1 of our search key is 0 so we follow the left child
 *     of the internal node at which point we reach leaf node
 *     (0001). 
 * 
 *     Now we compare the key being inserted, 0010, with the
 *     key in the leaf node we reached:
 *
 *         1234
 *         ----
 *         0001   # leaf node key
 *         0010   # key being inserted
 *
 *     The first bit at which the keys differ is bit 3, so we 
 *     create an internal node whose difference bit is set to
 *     3. Since bit 3 of the leaf node key 0001 is 0 we make
 *     this key the left child of our new internal node.
 *     Similarly, since bit 3 of the key being inserted is 1
 *     we make this right child of our internal node. 
 *
 *     The new internal node is now the parent of leaf node
 *     1001 and the child of 1001's old parent, the internal
 *     node whose difference bit is 1:      
 *     
 *                [1]
 *               /   \
 *            [3]     (1001)
 *           /   \
 *     (0001)     (0010)
 */
void 
ptrie_add2(ptrie_t *pt, void *key, void *val, void **pnode)
{
    pnode_t  *pn;
    pnode_t **lk;     /* orig parent to child link */
    pnode_t  *nnode;  /* new internal node */
    pnode_t  *nleaf;  /* new child node */
    size_t    keysz;
    int       diffbit;

    keysz = keysize(pt, key);

    if (pt->pt_size == 0 ||
        pt->pt_root == NULL) {
        pt->pt_root = newcld(pt, key, keysz, val);
        pt->pt_root->pn_up = NULL;
        pt->pt_size++;

        if (pnode)
            *pnode = pt->pt_root;

        return;
    }

    for (pn = pt->pt_root; pn->pn_type == PN_NODE; /**/) 
        pn = pn->pn_cld[getbit(key, keysz, pn->pn_bit)];

    diffbit = keycmp(key, keysz, pn->pn_key, pn->pn_keysz);
    if (diffbit == 0) {
        return;     /* duplicate! */
    }

    /*
     * Traverse down the tree, using the key we are
     * adding as a search key so that we can determine
     * the difference bit that will be used for this 
     * key. 
     * 
     * As we traverse, "pn" points to the node we are 
     * currently visiting and "lk" points to the link 
     * we used to get to "pn".
     *
     *
     *       pt->pt_root            pt->pt_root
     *             |                      |
     *      lk---> |                      |
     *      pn--> [1]                    [1]
     *           /   \         lk-----> /   \
     *        [3]     (1001)   pn--> [3]     (1001)
     *       /   \                  /   \
     * (0001)     (0010)      (0001)     (0010)
     *      
     */
    lk = &pt->pt_root;
    pn =  pt->pt_root; 

    while (pn->pn_type == PN_NODE) {
        /* 
         * Traverse tree until we either reach a leaf node or an
         * internal node whose difference bit is larger than diffbit
         */
        if (pn->pn_bit > ABSVAL(diffbit)) {
            break;
        } else {
            lk = &pn->pn_cld[getbit(key, keysz, pn->pn_bit)];
            pn = *lk;
        }
    }

    /*
     * Before:
     *                    pt_root
     *                       | 
     *                       | <- lk          
     *                      pn        
     *                     /  \       
     *
     * After:  
     *
     *  bit(nleaf[diffbit]) = 0    bit(nleaf[diffbit]) = 1
     * 
     *          |                           | 
     *          | <- lk                     | <- lk
     *        nnode             OR        nnode
     *       /     \                     /     \
     *  nleaf       pn                 pn      nleaf
     *             /  \               /  \
     *
     */
    nleaf = newcld(pt, key, keysz, val);
    nnode = newpar(pt, diffbit, nleaf, pn);

    nleaf->pn_up = nnode;
    nnode->pn_up = pn->pn_up;
    pn->pn_up = nnode;

    *lk = nnode;

    pt->pt_size++;
    
    if (pnode)
        *pnode = nleaf;

    return;
}

int 
ptrie_haskey(ptrie_t *pt, void *key)
{
    return ptrie_get(pt, key) != NULL;
}

void *
ptrie_get(ptrie_t *pt, void *key)
{
    pnode_t *pn;
    size_t   keysz;

    if (pt->pt_size == 0 ||
        pt->pt_root == NULL) {
        return NULL;
    }

    keysz = keysize(pt, key);

    for (pn = pt->pt_root; pn->pn_type == PN_NODE; /**/) 
        pn = pn->pn_cld[getbit(key, keysz, pn->pn_bit)];

    if (keyseq(key, keysz, pn->pn_key, pn->pn_keysz)) {
        return pn->pn_val;
    }

    return NULL;
}

/***********************************************************###**
 * Find node in ptrie for which all children nodes match prefix 
 * up to the first nbits
 ***********************************************************###*/
void *
ptrie_get_prefix(ptrie_t *pt, void *prefix, size_t nbits)
{
    pnode_t *pn;
    pnode_t *in;
    size_t   pfxsz;
    int      diffbit;

    pfxsz = keysize(pt, prefix);

    if (pt->pt_size == 0 ||
        pt->pt_root == NULL) {
        return NULL;
    }

    for (pn = pt->pt_root; pn->pn_type == PN_NODE; /**/) 
        pn = pn->pn_cld[getbit(prefix, pfxsz, pn->pn_bit)];

    diffbit = keycmp(prefix, pfxsz, pn->pn_key, pn->pn_keysz);
    
    if (diffbit == 0 || nbits < diffbit) {
        for (in = pn->pn_up; in && nbits < in->pn_bit; in = pn->pn_up) 
            pn = in;
    }

    return pn;
}

/***********************************************************###**
 * ptrie_del(ptrie, 0010)
 *
 * before:                     after:
 *                [1]                   [1]
 *               /   \                 /   \
 *            [3]     (1001)     (0001)     (1001)
 *           /   \
 *     (0001)     (0010)
 *
 ***********************************************************###*/
void 
ptrie_del(ptrie_t *pt, void *key)
{
    size_t keysz;

    if (pt->pt_size == 0 ||
        pt->pt_root == NULL) {
        return;
    }

    keysz = keysize(pt, key);
    pt->pt_root = ptrie_del0(pt, key, keysz, pt->pt_root);
}

/***********************************************************###**
 * Ptrie delete helper function
 ***********************************************************###*/
static pnode_t * 
ptrie_del0(ptrie_t *pt, void *key, size_t keysz, pnode_t *pn)
{
    int i; /* child index */

    if (pn->pn_type == PN_NODE) {
        i = getbit(key, keysz, pn->pn_bit);
        pn->pn_cld[i] = ptrie_del0(pt, key, keysz, pn->pn_cld[i]);
    
        if (pn->pn_cld[i] == NULL) {
            pnode_t *fn; /* internal node we'll free */

            /*                                     
             *                                    +--------->[1] pn->pn_up
             *                                    |         /   \
             * pn->pn_cld[OTHER_CLDIDX(i)]->pn_up |   pn [3]     (1001)
             *                                    |     /   \
             *                                    (0001)     NULL
             */
            pn->pn_cld[OTHER_CLDIDX(i)]->pn_up = pn->pn_up;

            /*                                    +--------->[1]
             *                                    |         /   \
             * pn->pn_cld[OTHER_CLDIDX(i)]->pn_up |   fn [3]     (1001)
             *                                    |     /   \
             *                                 pn (0001)     NULL
             */
            fn = pn;
            pn = pn->pn_cld[OTHER_CLDIDX(i)];
            pnode_free(pt, fn);
        }
    } else if (keyseq(key, keysz, pn->pn_key, pn->pn_keysz)) {
        pnode_free(pt, pn);
        pn = NULL;
        pt->pt_size--;
    }

    return pn;
}


/***********************************************************###**
 * ptrie_del_pnode(pn=0010)
 *
 *             gp [1]
 *               /   \
 *         in [3]     (1001)
 *           /   \
 *  oc (0001)     (0010) pn
 ***********************************************************###*/
void
ptrie_del_pnode(ptrie_t *pt, void *pnode)
{
    int      i;  /* child index */
    pnode_t *in; /* internal node parent */
    pnode_t *gp; /* grand parent node */
    pnode_t *oc; /* other child */
    pnode_t *pn; /* node being deleted */

    pn = (pnode_t *)pnode;

    if (NOT pn->pn_type == PN_LEAF)
        return;

    if (NOT pn->pn_up) {
        /* tree is made up of single leaf node */
        pnode_free(pt, pn);
        pt->pt_size--;
        return; 
    }

    in = pn->pn_up;
    gp = in->pn_up;

    i = getbit(pn->pn_key, pn->pn_keysz, in->pn_bit);

    oc = in->pn_cld[OTHER_CLDIDX(i)];
    oc->pn_up = in->pn_up;

    if (gp) {
        i = getbit(pn->pn_key, pn->pn_keysz, gp->pn_bit);
        gp->pn_cld[i] = oc;
    } else {
        pt->pt_root = oc;
    }

    pnode_free(pt, in);
    pnode_free(pt, pn);
    pt->pt_size--;

    return;
}

void 
ptrie_set_parm(ptrie_t *pt, uint32_t parm, void *value)
{
    switch (parm) {
    case PTRIEPARM_KEYSZ:
        pt->pt_keysz = (size_t) value;
        break;

    case PTRIEPARM_KEYSZ_FUNC:
        pt->pt_keysz_func = (size_t (*)(void *)) value;
        break;

    case PTRIEPARM_MALLOC_FUNC:
        pt->pt_malloc_func = (void *(*)(size_t)) value;
        break;

    case PTRIEPARM_FREE_FUNC:
        pt->pt_free_func = (void (*)(void *)) value;
        break;

    default:
        break;
    }
}


int 
ptrie_size(ptrie_t *pt)
{
    return pt ? pt->pt_size : 0;
}

void 
ptrie_iter_init(ptrie_t *pt, void *root, ptrie_iter_t *ptit) 
{
    pnode_t *pn;
    
    if (ptit == NULL)
        ptit = &pt->pt_iter;

    if (NOT root)
        root = pt->pt_root;

    ptit->root = root;

    if (pt->pt_size == 0 ||
        pt->pt_root == NULL) {
        ptit->pn = NULL;
        return;
    }

    if (pt->pt_root->pn_type == PN_LEAF) {
        ptit->pn = pt->pt_root;
        return;
    }

    /* find left-most child of root*/
    for (pn = ptit->root; pn->pn_type == PN_NODE; pn = pn->pn_cld[0])
        /**/;

    ptit->pn = pn;
    return;
}

int 
ptrie_iter_next(ptrie_t *pt, ptrie_iter_t *ptit, void **key, void **val)
{
    pnode_t *pn;

    if (NOT ptit)
        ptit = &pt->pt_iter;

    if (NOT ptit->pn)  
        return 0; /* finished traversal */

    pn = ptit->pn;

    if (key) *key = pn->pn_key;
    if (val) *val = pn->pn_val;
    
    if (pn != ptit->root && NODE_IS_RCLD(pn)) {
        for (pn = pn->pn_up; pn != ptit->root; pn = pn->pn_up) 
            if (NODE_IS_LCLD(pn))
                break;
    }

    if (pn == ptit->root) {
        ptit->pn = NULL;
        return 1;
    }

    /* 
     * If we make it here, pn is a left child. 
     * In that case, we set pn to the right 
     * child. Then we find traverse down until 
     * we find the left-most leaf node.
     */
    pn = pn->pn_up->pn_cld[1];
    while (pn->pn_type == PN_NODE)
        pn = pn->pn_cld[0];

    ptit->pn = pn;
    return 1;
}

static pnode_t *
newpar(ptrie_t *pt, int diffbit, pnode_t *cld1, pnode_t *cld2)
{
    pnode_t *pn = pnode_new(pt, PN_NODE);

    pn->pn_bit = ABSVAL(diffbit); 

    if (diffbit < 0) {
        /* 
         * bit(key(cld1)) = 0 
         * bit(key(cld2)) = 1
         */
        pn->pn_cld[0] = cld1;
        pn->pn_cld[1] = cld2;
    } else {
        /* 
         * bit(key(cld1)) = 1 
         * bit(key(cld2)) = 0
         */
        pn->pn_cld[0] = cld2;
        pn->pn_cld[1] = cld1;
    }

    return pn;
}

static pnode_t *
newcld(ptrie_t *pt, void *key, size_t keysz, void *val)
{
    pnode_t *pn = pnode_new(pt, PN_LEAF);

    pn->pn_key   = key;
    pn->pn_keysz = keysz;

    pn->pn_val   = val;
    pn->pn_valsz = 0;

    return pn;
}

static size_t
keysize(ptrie_t *pt, void *key)
{
    if (pt->pt_keysz)
        return pt->pt_keysz;
    else if (pt->pt_keysz_func)
        return (*pt->pt_keysz_func)(key);
    else {
        fprintf(stderr, "Error: unable to determine key size\n");
        exit(1);
    }
}

static pnode_t *
pnode_new(ptrie_t *pt, pn_type_t type)
{
    pnode_t *pn;

    if (pt->pt_list == NULL) {
        int i;
        for (i = 0; i < PN_FREELIST_BLKSZ; i++) {
            pn = pt->pt_malloc_func(sizeof(*pn));
            pn->pn_cld[0] = pt->pt_list;
            pt->pt_list = pn;
        }
    }

    pn = pt->pt_list;
    pt->pt_list = pn->pn_cld[0];
    pn->pn_type = type;

    return pn;
}

static void 
pnode_free(ptrie_t *pt, pnode_t *pn)
{
    pn->pn_cld[0] = pt->pt_list;
    pt->pt_list = pn;
}

static void *
fmalloc(size_t size) 
{
    void *data = malloc(size);
    if (data == NULL) {
        fprintf(stderr, "fmalloc - malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    return data;
}
