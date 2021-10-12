/* @(#)23 1.1 src/bldenv/sbtools/include/parse_rc_file.h, bldprocess, bos412, GOLDA411a 93/04/29 12:18:54 */
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * ODE 2.1.1
 */
/* @(#)$RCSfile: parse_rc_file.h,v $ $Revision: 1.6 $ (OSF) $Date: 91/12/05 21:03:56 $ */
/*
 * data structures used to hold parsed rc file for sandboxes
 */

struct arg_list {
    struct arg_list *next;
    char **tokens;
    int ntokens;
    int maxtokens;
};

struct field {
    struct field *next;		/* next field */
    char *name;			/* name of this field */
    struct arg_list *args;	/* args for this field */
};

#define RC_HASHBITS 6		/* bits used in hash function */
#define RC_HASHSIZE (1<<RC_HASHBITS) /* range of hash function */
#define RC_HASHMASK (RC_HASHSIZE-1)	/* mask for hash function */
#define RC_HASH(i) ((i)&RC_HASHMASK)

struct hashent {
    struct hashent *next;
    struct field *field;
};

struct rcfile {
    struct hashent *hashtable[RC_HASHSIZE];
    struct field *list;
    struct field *last;
};
