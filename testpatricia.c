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

#include <netinet/in.h>
#include <arpa/inet.h>

#include "patricia.h"

static void test_1(void);
static void test_2(void);
static void test_3(void);
static void test_4(void);
static void test_5(void);
static void test_6(void);

int main(int argc, char **argv)
{

    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();

    exit(0);
}

static void
test_1(void)
{
    ptrie_t      *ptrie;
    ptrie_iter_t  ptit;
    char         *key;
    char         *val;

    fprintf(stderr, "\ntest_1\n");

    ptrie = ptrie_new();

    ptrie_add(ptrie, "0001", "one");
    ptrie_add(ptrie, "0010", "two");
    ptrie_add(ptrie, "0011", "three");

    val = ptrie_get(ptrie, "0010");
  
    fprintf(stderr, "val = %s\n", val);

    /* user-defined iterator */
    ptrie_iter_init(ptrie, 0, &ptit);
    while (ptrie_iter_next(ptrie, &ptit, (void **)&key, (void **)&val))
        fprintf(stderr, "%s => %s\n", key, val);

    fprintf(stderr, "\n");

    /* use default iterator */
    ptrie_iter_init(ptrie, 0, NULL);
    while (ptrie_iter_next(ptrie, NULL, (void **)&key, (void **)&val))
        fprintf(stderr, "%s => %s\n", key, val);

    fprintf(stderr, "\n");

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "\n");

    foreach_ptrie_key(ptrie, 0, &key) {
        fprintf(stderr, "key: %s\n", key);
    }

    fprintf(stderr, "\n");

    foreach_ptrie_val(ptrie, 0, &val) {
        fprintf(stderr, "val: %s\n", val);
    }
}

static void
test_2(void)
{
    ptrie_t *ptrie;
    char    *key;
    char    *val;

    fprintf(stderr, "\ntest_2\n");

    ptrie = ptrie_new();

    fprintf(stderr, "adding one, two, three...\n");

    ptrie_add(ptrie, "0001", "one");
    ptrie_add(ptrie, "0010", "two");
    ptrie_add(ptrie, "0011", "three");

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "deleting three\n");
    ptrie_del(ptrie, "0011");

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "deleting two\n");
    ptrie_del(ptrie, "0010");

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "deleting one\n");
    ptrie_del(ptrie, "0001");

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    return;
}

static void
test_3(void)
{
    ptrie_t *ptrie;
    void    *n1, *n2, *n3;
    char    *key;
    char    *val;

    fprintf(stderr, "\ntest_3\n");

    ptrie = ptrie_new();

    fprintf(stderr, "adding one, two, three...\n");

    ptrie_add2(ptrie, "0001", "one", &n1);
    ptrie_add2(ptrie, "0010", "two", &n2);
    ptrie_add2(ptrie, "0011", "three", &n3);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 3\n");
    ptrie_del_pnode(ptrie, n3);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 2\n");
    ptrie_del_pnode(ptrie, n2);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 1\n");
    ptrie_del_pnode(ptrie, n1);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "\nadding one, two, three...\n");

    ptrie_add2(ptrie, "0001", "one", &n1);
    ptrie_add2(ptrie, "0010", "two", &n2);
    ptrie_add2(ptrie, "0011", "three", &n3);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 1\n");
    ptrie_del_pnode(ptrie, n1);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 2\n");
    ptrie_del_pnode(ptrie, n2);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

    fprintf(stderr, "=> deleting node 3\n");
    ptrie_del_pnode(ptrie, n3);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", key, val);
    }

}

void
test_4(void)
{
    ptrie_t        *ptrie;
    struct in_addr *key;
    char           *val;

    struct node {
        struct in_addr addr;
        char *str;
    } *pn, nodes[] = {
        { .addr.s_addr = inet_addr("192.168.1.1"), .str = "192.168.1.1" },
        { .addr.s_addr = inet_addr("192.168.2.1"), .str = "192.168.2.1" },
        { .addr.s_addr = inet_addr("192.168.3.1"), .str = "192.168.3.1" },
        { .addr.s_addr = 0, .str = 0 }
    };

    fprintf(stderr, "\ntest_4\n");

    ptrie = ptrie_new();
    ptrie_set_parm(ptrie, PTRIEPARM_KEYSZ, (void *)sizeof(struct in_addr));

    for (pn = nodes; pn->str; pn++)
        ptrie_add(ptrie, &pn->addr, pn->str);

    foreach_ptrie_keyval(ptrie, 0, &key, &val) {
        fprintf(stderr, "%s => %s\n", inet_ntoa(*key), val);
    }

    for (pn = nodes; pn->str; pn++) {
        fprintf(stderr, "ptrie_get(%s) => %s\n",
                inet_ntoa(pn->addr), (char *)ptrie_get(ptrie, &pn->addr));
    }
}

void
test_5(void)
{
    ptrie_t *ptrie;
    char    *key;

    fprintf(stderr, "\ntest5\n");

    ptrie = ptrie_new();
    ptrie_add(ptrie, "a", NULL);
    ptrie_add(ptrie, "aa", NULL);
    ptrie_add(ptrie, "ab", NULL);
    ptrie_add(ptrie, "aac", NULL);
    ptrie_add(ptrie, "aac1", NULL);
    ptrie_add(ptrie, "aac2", NULL);
    ptrie_add(ptrie, "aac3", NULL);
    ptrie_add(ptrie, "b", NULL);
    ptrie_add(ptrie, "c", NULL);

    foreach_ptrie_key(ptrie, 0, &key) {
        fprintf(stderr, "%s\n", key);
    }

    fprintf(stderr, "strings with prefix \"a\"\n");

    foreach_ptrie_key_with_prefix(ptrie, 0, "a", strlen("a")*8, &key) {
        fprintf(stderr, "%s\n", key);        
    }

    fprintf(stderr, "strings with prefix \"b\"\n");

    foreach_ptrie_key_with_prefix(ptrie, 0, "b", strlen("b")*8, &key) {
        fprintf(stderr, "%s\n", key);        
    }

    fprintf(stderr, "strings with prefix \"aac\"\n");

    foreach_ptrie_key_with_prefix(ptrie, 0, "aac", strlen("aac")*8, &key) {
        fprintf(stderr, "%s\n", key);        
    }
}

void
test_6(void)
{
    ptrie_t        *ptrie;
    struct in_addr  prefix;
    struct in_addr *key;
    struct node {
        struct in_addr addr;
        char *str;
    } *pn, nodes[] = {
        { .addr.s_addr = inet_addr("192.168.1.1"), .str = "192.168.1.1" },
        { .addr.s_addr = inet_addr("192.168.2.1"), .str = "192.168.2.1" },
        { .addr.s_addr = inet_addr("192.168.3.1"), .str = "192.168.3.1" },
        { .addr.s_addr = 0, .str = 0 }
    };

    /*
     *             [23]
     *            /    \
     * 192.168.1.1      [24]
     *                 /    \
     *      192.168.2.1      192.168.3.1
     */
    fprintf(stderr, "\ntest5\n");

    ptrie = ptrie_new();
    ptrie_set_parm(ptrie, PTRIEPARM_KEYSZ, (void *)sizeof(struct in_addr));


    for (pn = nodes; pn->str; pn++)
        ptrie_add(ptrie, &pn->addr, pn->str);
    
    foreach_ptrie_key(ptrie, 0, &key) {
        fprintf(stderr, "%s\n", inet_ntoa(*key));
    }

    fprintf(stderr, "\naddresses with prefix \"192.168/16\"\n");

    /* 192.168/16 */
    prefix.s_addr = inet_addr("192.168.0.0");
    foreach_ptrie_key_with_prefix(ptrie, 0, &prefix, 16, &key) {
        fprintf(stderr, "%s\n", inet_ntoa(*key));        
    }

    fprintf(stderr, "\naddresses with prefix \"192.168.2/23\"\n");
    /* 
     * 192.168.2/23
     *
     * Should be 
     * 192.168.2.1
     * 192.168.3.*
     */
    prefix.s_addr = inet_addr("192.168.2.0");
    foreach_ptrie_key_with_prefix(ptrie, 0, &prefix, 23, &key) {
        fprintf(stderr, "%s\n", inet_ntoa(*key));        
    }

    fprintf(stderr, "\naddresses with prefix \"192.168.2/24\"\n");
    /* 
     * 192.168.2/24
     *
     * Should be 
     * 192.168.2.1
     */
    prefix.s_addr = inet_addr("192.168.2.0");
    foreach_ptrie_key_with_prefix(ptrie, 0, &prefix, 24, &key) {
        fprintf(stderr, "%s\n", inet_ntoa(*key));        
    }

}
