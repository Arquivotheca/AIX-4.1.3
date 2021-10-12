/* @(#)87	1.7  src/bos/usr/ccs/lib/libodm/odmlib.h, libodm, bos411, 9428A410j 2/8/93 13:50:38 */
/*
 * COMPONENT_NAME: LIBODM odmi.h
 *
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
#ifndef _ODMLIB_H
#define _ODMLIB_H

struct  ClassFileHdr {
        struct  ClassHdr        Hdr;
        struct  Class           Class;
        };

struct  ClxnFileHdr {
        struct  ClassHdr        Hdr;
        struct  StringClxn      StringClxn;
        };

#define if_err_ret_err(n,t,e) if(((int)(n)) == -1){if(e)odmcf_errno=e;return(t-1);}
#define if_null_ret_err(n,t,e) if(((int)(n)) == 0){if(e)odmcf_errno=e;return(t-1);}
#define if_true_ret_err(n,t,e) if((int)(n)){if(e)odmcf_errno=e;return(t-1);}
#define ret_err(t,e) { odmcf_errno = e; return(t-1);}

/* Defines to allow the read-only open */
/* The 'open' variable of the 'Class' structure is used */
/* to indicate if the file is open and if it is READ_ONLY */
/* The first bit of the 'open' variable indicates if the file */
/* is already open. The second bit indicates if the file is   */
/* read-only.                                                 */

#define CLASS_IS_OPEN     1
#define OPENED_AS_READ_ONLY 2

int                   add_lock_to_table();
int                   add_vchar();
struct StringClxn    *addr_clxn();
struct Crit          *breakcrit();
int                   change_vchar();
void                  child_died();
int                   close_clxn();
int                   cmpkmch();
int                   convert_to_binary();
char                 *convert_to_hex_ascii();
int                   create_clxn();
int                   destroy_clxn();
int                   get_ascii_phrase();
int                   get_offsets();
int                   get_one_byte_from_ascii();
int                   get_string_dboff();
char                 *get_value_from_string();
int                   get_vchar();
int                   init_class();
int                   init_clxn();
int                   legal_size();
struct StringClxn    *mount_clxn();
int                   note_class();
struct StringClxn    *open_clxn();
int                   pid_in_list();
int                   print_odm_trace();
int                   raw_add_obj();
char                 *raw_add_str();
CLASS_SYMBOL          raw_addr_class();
int                   raw_close_class();
int                   raw_close_clxn();
char                 *raw_find_byid();
char                 *raw_find_obj();
int                   raw_rm_obj();
int                   remove_lock_from_table();
int                   verify_class_structure();
void                  odm_searchpath();

#ifdef R5A
char *malloc(), *realloc();
#endif

#endif /* _ODMLIB_H */
