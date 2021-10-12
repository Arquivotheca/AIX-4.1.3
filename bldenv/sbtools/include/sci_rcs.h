/* @(#)25 1.1 src/bldenv/sbtools/include/sci_rcs.h, bldprocess, bos412, GOLDA411a 93/04/29 12:19:08 */
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

# define STATIC static

/*
 * The innards of this abstract data type are subject to change.
 */
struct sci_elem {
  char * name;
  char * set;
  char * ver_merge;
  char * ver_user;
  char * ver_config;
  char * ver_ancestor;
  char * ver_latest;
  char * leader;
  BOOLEAN same13;                                /* ver_merge = ver_ancestor */
  BOOLEAN same23;                                 /* ver_user = ver_ancestor */
  BOOLEAN defunct;
  BOOLEAN skip;                           /* file was in resub tracking file */
  BOOLEAN locked;
  BOOLEAN need_merge;
  BOOLEAN merged_up;                      /* did the user merge-up the file? */
  BOOLEAN has_user_branch;
  BOOLEAN has_merge_branch;
  int called_getancestor;
  int status;
  struct sci_elem *next;
};

struct sci_list {
  int serial_num;             /* Used to confirm that this is a valid object */
  int elem_num;                        /* The number of elements in the list */
  struct sci_elem * head;
  struct sci_elem * tail;
};

typedef struct sci_list * SCI_LIST;
typedef struct sci_elem * SCI_ELEM;

SCI_ELEM sci_first ( );
SCI_ELEM sci_next ( );
