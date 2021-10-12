/* @(#)35       1.4  src/bos/usr/include/swvpd1.h, cmdinstl, bos411, 9428A410j 5/27/94 08:35:14 */

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS: Internal include file for swvpd routines.
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This file contains definitions that would normally be placed in swvpd.h, but
   due to this being a shipped file it was decided that a new header be created
   instead. */

#ifndef __H_SWVPD1
#define __H_SWVPD1

#define  LPP_32_FMT             BIT(7)
#define  LPP_41_FMT             BIT(8)

#define  LPP_PKG_PTF_TYPE_C     BIT(9)
#define  LPP_PKG_PTF_TYPE_E     BIT(10)
#define  LPP_PKG_PTF_TYPE_ML    BIT(11)

#define  LPP_PKG_PTF_TYPE       ( BIT(9) | BIT(10) | BIT(11) )
#define  LPP_PKG_PTF_TYPE_M     ( BIT(9) | BIT(10) | BIT(11) )

#define  IF_LPP_PKG_PTF_TYPE_C(t)\
                                 ((t & LPP_PKG_PTF_TYPE) == LPP_PKG_PTF_TYPE_C)
#define  IF_LPP_PKG_PTF_TYPE_E(t)\
                                 ((t & LPP_PKG_PTF_TYPE) == LPP_PKG_PTF_TYPE_E)
#define  IF_LPP_PKG_PTF_TYPE_ML(t)\
                                 ((t & LPP_PKG_PTF_TYPE) == LPP_PKG_PTF_TYPE_ML)
#define  IF_LPP_PKG_PTF_TYPE_M(t)\
                                 ((t & LPP_PKG_PTF_TYPE) == LPP_PKG_PTF_TYPE_M)


/* VPD_STRING_DELIMITER is used to pack multiple strings into one in the vpd.
   This keeps us from having to create new string fields in the vpd.
   Currently used to put supersede info at the end of the prereq field for
   maintenance update packages and to put the description info from the
   toc at the head of the fixinfo (apar info) field.  */

#define VPD_STRING_DELIMITER "\xfe\n"

int vpdadd (int    tbl_id,
            void * tbl_ptr);

int vpdchg (int    tbl_id,
            int    key_mask,
            void * tbl_ptr);

int vpdchgadd (int    tbl_id,
               int    key_mask,
               void * tbl_ptr);

int vpdchgall (int    tbl_id,
               int    key_mask,
               void * tbl_ptr);

int vpdchgget (int    tbl_id,
               void * tbl_ptr);

int vpddel (int    tbl_id,
            int    key_mask,
            void * tbl_ptr);

int vpddelall (int    tbl_id,
               int    key_mask,
               void * tbl_ptr);

void vpd_free_vchars (int    tbl_id,
                      void * tbl_ptr);

int vpdget (int    tbl_id,
            int    key_mask,
            void * tbl_ptr);

int vpdgetnxt (int    tbl_id,
               void * tbl_ptr);

int vpdlocal (void);

int vpdlocalpath (char * path);

int vpdremote (void);

int vpdremotepath (char * path);

int vpdreslpp_name (char  * n_ptr,
                    short * i_ptr);

char * vpdreslpp_id (short   l_id,
                     char  * n_ptr);

#endif
