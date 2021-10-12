/* @(#)32       1.4.1.3  src/bos/usr/include/swvpd0.h, cmdswvpd, bos411, 9428A410j 6/11/93 12:08:20 */

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS: Internal include file for swvpd routines.
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __H_SWVPD0
#define __H_SWVPD0

#include <odmi.h>
#include <swvpd.h>
#include <swvpd1.h>

#define VPD_LOCAL       0
#define VPD_REMOTE      1

#ifndef FALSE
#       define FALSE    (1==2)
#       define TRUE     (!FALSE)
#endif


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* The following types are used to define the global information*/
/* used by all the swvpd routines.                              */
typedef struct tbl_info {                       /* for each table       */
  int   t_open ;                                /* is table open        */
  int   t_lock ;                                /* is table locked      */
  struct Class *t_class ;                       /* Class ctl block addr */
  } ;


typedef struct path_info {                      /* info for each path   */
  char  *p_name ;                               /* path name            */
  struct tbl_info table [ N_tbls ];             /* info for each table  */
  } ;

typedef struct vpd_type {
  int    vpd_debug ;                            /* is debug enabled     */
  int    odminit ;                              /* is odminitialized    */
  int    cur_path ;                             /* current path index   */
  int    table_cols [ N_tbls ] ;                /* cols/fields for table*/
  struct ClassElem *table_elem [ N_tbls ] ;     /* ptrs to element desc */
                                                /* tables for each table*/
  struct path_info path [ N_paths ] ;           /* for each path info   */
  };

extern struct vpd_type vpd_ctl ;                /* the swvpd global     */
                                                /* external structure   */

#define  TABLE(n)  vpd_ctl.path[vpd_ctl.cur_path].table[n].t_class
#define  T_OPEN(n) vpd_ctl.path[vpd_ctl.cur_path].table[n].t_open
#define  T_LOCK(n) vpt_ctl.path[vpd_ctl.cur_path].table[n].t_lock

/* Definitions of routines internal to libswvpd */

int vpdbldsql (int    tbl_id,
               int    key_mask,
               void * tbl_ptr,
               char * buffer);

int vpdinit (int tbl_id);

int vpdresclr (int newpath);

int vpdterm (void);

#endif



