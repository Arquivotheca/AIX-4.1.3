/* @(#)28       1.2  src/bos/usr/ccs/bin/lex/hash.h, cmdlang, bos411, 9428A410j 7/25/91 12:17:30 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991  
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef H_HASH
#define H_HASH

#define PCRANK(i)       (wcrank?((crank_t *)(wcrank->table[i].info)):0)
#define PMATCH(i)       (wmatch?((match_t *)(wmatch->table[i].info)):0)

struct hash_data_s
{
    wchar_t             id;                     /* id of entry */
    unsigned int        next;                   /* address of collision */
    void                *info;                  /* hashed information */
};
typedef struct hash_data_s hash_data_t;

struct hash_s
{
    int                 size;
    int                 top;
    int                 current;
    int                 prev;
    hash_data_t         *table;
};
typedef struct hash_s hash_t;

struct crank_s
{
    int                 verify;
    int                 advance;
};
typedef struct crank_s crank_t;

struct match_s
{
    int                 cindex;
    int                 cmatch;
    int                 list;
};
typedef struct match_s match_t;

struct xccl_s
{
    int                 type;
    int                 cindex;
    int                 verify;
    int                 advance;
};
typedef struct xccl_s xccl_t;

#endif
