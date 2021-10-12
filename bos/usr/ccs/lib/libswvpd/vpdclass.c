/* @(#)34       1.15  src/bos/usr/ccs/lib/libswvpd/vpdclass.c, libswvpd, bos411, 9428A410j 3/31/94 12:47:21 */
/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: swvpd.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Contains the initialized data structures used by swvpd       */
/* routines.  vpd_ctl is the only external object and contains  */
/* all the globally accessed objects needed by swvpd routines   */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/


#include <swvpd0.h>


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*    LPP table                                                 */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
static struct ClassElem lpp_ClassElem[] =  {
 { "name",ODM_CHAR, 12,145, NULL,NULL,0,NULL ,-1,0},
 { "size",ODM_LONG, 160, 4, NULL,NULL,0,NULL ,-1,0},
 { "state",ODM_SHORT, 164, 2, NULL,NULL,0,NULL ,-1,0},
 { "cp_flag",ODM_LONG, 168, 4, NULL,NULL,0,NULL ,-1,0},
 { "group",ODM_VCHAR, 172,22, NULL,NULL,0,NULL ,-1,0},
 { "magic_letter",ODM_CHAR, 176,2, NULL,NULL,0,NULL ,-1,0},
 { "ver",ODM_SHORT, 178, 2, NULL,NULL,0,NULL ,-1,0},
 { "rel",ODM_SHORT, 180, 2, NULL,NULL,0,NULL ,-1,0},
 { "mod",ODM_SHORT, 182, 2, NULL,NULL,0,NULL ,-1,0},
 { "fix",ODM_SHORT, 184, 2, NULL,NULL,0,NULL ,-1,0},
 { "description",ODM_VCHAR, 188,82, NULL,NULL,0,NULL ,-1,0},
 { "lpp_id",ODM_SHORT, 192, 2, NULL,NULL,0,NULL ,-1,0},
 };

struct StringClxn lpp_STRINGS[] = {
 "lpp.vc", 0,NULL,NULL,0,0,0
};
static struct StringClxn lpp_STRINGS_R[] = {
 "lpp.vc", 0,NULL,NULL,0,0,0
 };
struct Class lpp_CLASS[] = {
 ODMI_MAGIC, "lpp", sizeof(struct lpp), lpp_Descs, lpp_ClassElem, lpp_STRINGS,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };

static struct Class lpp_CLASS_R[] = {
 ODMI_MAGIC, "lpp", sizeof(struct lpp), lpp_Descs, lpp_ClassElem, lpp_STRINGS_R,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*    Product table                                             */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

static struct ClassElem product_ClassElem[] =  {
 { "lpp_name",ODM_CHAR, 12,145, NULL,NULL,0,NULL ,-1,0},
 { "comp_id",ODM_CHAR, 157,20, NULL,NULL,0,NULL ,-1,0},
 { "update",ODM_SHORT, 178, 2, NULL,NULL,0,NULL ,-1,0},
 { "cp_flag",ODM_LONG, 180, 4, NULL,NULL,0,NULL ,-1,0},
 { "fesn",ODM_CHAR, 184,10, NULL,NULL,0,NULL ,-1,0},
 { "name",ODM_VCHAR, 196,42, NULL,NULL,0,NULL ,-1,0},
 { "state",ODM_SHORT, 200, 2, NULL,NULL,0,NULL ,-1,0},
 { "ver",ODM_SHORT, 202, 2, NULL,NULL,0,NULL ,-1,0},
 { "rel",ODM_SHORT, 204, 2, NULL,NULL,0,NULL ,-1,0},
 { "mod",ODM_SHORT, 206, 2, NULL,NULL,0,NULL ,-1,0},
 { "fix",ODM_SHORT, 208, 2, NULL,NULL,0,NULL ,-1,0},
 { "ptf",ODM_CHAR, 210,10, NULL,NULL,0,NULL ,-1,0},
 { "media",ODM_SHORT, 220, 2, NULL,NULL,0,NULL ,-1,0},
 { "sceded_by",ODM_CHAR, 222,10, NULL,NULL,0,NULL ,-1,0},
 { "fixinfo",ODM_VCHAR, 232,1024, NULL,NULL,0,NULL ,-1,0},
 { "prereq",ODM_VCHAR, 236,1024, NULL,NULL,0,NULL ,-1,0},
 { "description",ODM_VCHAR, 240,1024, NULL,NULL,0,NULL ,-1,0},
 { "supersedes",ODM_VCHAR, 244,512, NULL,NULL,0,NULL ,-1,0},
 };

struct StringClxn product_STRINGS[] = {
 "product.vc", 0,NULL,NULL,0,0,0
 };
struct StringClxn product_STRINGS_R[] = {
 "product.vc", 0,NULL,NULL,0,0,0
 };
struct Class product_CLASS[] = {
 ODMI_MAGIC, "product", sizeof(struct product), product_Descs, product_ClassElem
, product_STRINGS,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };

static struct Class product_CLASS_R[] = {
 ODMI_MAGIC, "product", sizeof(struct product), product_Descs, product_ClassElem , product_STRINGS_R,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*    History table                                             */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

static struct ClassElem history_ClassElem[] =  {
 { "lpp_id",ODM_SHORT, 12, 2, NULL,NULL,0,NULL ,-1,0},
 { "event",ODM_SHORT, 14, 2, NULL,NULL,0,NULL ,-1,0},
 { "ver",ODM_SHORT, 16, 2, NULL,NULL,0,NULL ,-1,0},
 { "rel",ODM_SHORT, 18, 2, NULL,NULL,0,NULL ,-1,0},
 { "mod",ODM_SHORT, 20, 2, NULL,NULL,0,NULL ,-1,0},
 { "fix",ODM_SHORT, 22, 2, NULL,NULL,0,NULL ,-1,0},
 { "ptf",ODM_CHAR, 24,10, NULL,NULL,0,NULL ,-1,0},
 { "corr_svn",ODM_CHAR, 34,40, NULL,NULL,0,NULL ,-1,0},
 { "cp_mod",ODM_CHAR, 74,10, NULL,NULL,0,NULL ,-1,0},
 { "cp_fix",ODM_CHAR, 84,10, NULL,NULL,0,NULL ,-1,0},
 { "login_name",ODM_CHAR, 94,18, NULL,NULL,0,NULL ,-1,0},
 { "state",ODM_SHORT, 112, 2, NULL,NULL,0,NULL ,-1,0},
 { "time",ODM_LONG, 116, 4, NULL,NULL,0,NULL ,-1,0},
 { "comment",ODM_VCHAR, 120,192, NULL,NULL,0,NULL ,-1,0},
 };
static struct StringClxn history_STRINGS[] = {
 "history.vc", 0,NULL,NULL,0,0,0
 };
static struct StringClxn history_STRINGS_R[] = {
 "history.vc", 0,NULL,NULL,0,0,0
 };
static struct Class history_CLASS[] = {
 ODMI_MAGIC, "history", sizeof(struct history), history_Descs, history_ClassElem, history_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
static struct Class history_CLASS_R[] = {
 ODMI_MAGIC, "history", sizeof(struct history), history_Descs, history_ClassElem, history_STRINGS_R,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*    Inventory table                                           */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

static struct ClassElem inventory_ClassElem[] =  {
 { "lpp_id",ODM_SHORT, 12, 2, NULL,NULL,0,NULL ,-1,0},
 { "private",ODM_SHORT, 14, 2, NULL,NULL,0,NULL ,-1,0},
 { "file_type",ODM_SHORT, 16, 2, NULL,NULL,0,NULL ,-1,0},
 { "format",ODM_SHORT, 18, 2, NULL,NULL,0,NULL ,-1,0},
 { "loc0",ODM_CHAR, 20,128, NULL,NULL,0,NULL ,-1,0},
 { "loc1",ODM_VCHAR, 148,128, NULL,NULL,0,NULL ,-1,0},
 { "loc2",ODM_VCHAR, 152,128, NULL,NULL,0,NULL ,-1,0},
 { "size",ODM_LONG, 156, 4, NULL,NULL,0,NULL ,-1,0},
 { "checksum",ODM_LONG, 160, 4, NULL,NULL,0,NULL ,-1,0},
 };
static struct StringClxn inventory_STRINGS[] = {
 "inventory.vc", 0,NULL,NULL,0,0,0
 };
static struct StringClxn inventory_STRINGS_R[] = {
 "inventory.vc", 0,NULL,NULL,0,0,0
 };
static struct Class inventory_CLASS[] = {
 ODMI_MAGIC, "inventory", sizeof(struct inventory), inventory_Descs, inventory_ClassElem, inventory_STRINGS,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };
static struct Class inventory_CLASS_R[] = {
 ODMI_MAGIC, "inventory", sizeof(struct inventory), inventory_Descs, inventory_ClassElem, inventory_STRINGS_R,FALSE,NULL,NULL,NULL,0,NULL,0,"", 0,-ODMI_MAGIC
 };

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*    Fix table                                                 */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

static struct ClassElem fix_ClassElem[] =  {
 { "name",ODM_CHAR, 12,16, NULL,NULL,0,NULL ,-1,0},
 { "abstract",ODM_CHAR, 28,60, NULL,NULL,0,NULL ,-1,0},
 { "type",ODM_CHAR, 88,2, NULL,NULL,0,NULL ,-1,0},
 { "filesets",ODM_VCHAR, 92,64, NULL,NULL,0,NULL ,-1,0},
 { "symptom",ODM_VCHAR, 96,64, NULL,NULL,0,NULL ,-1,0},
 };
struct StringClxn fix_STRINGS[] = {
 "fix.vc", 0,NULL,NULL,0,0,0
 };
struct StringClxn fix_STRINGS_R[] = {
 "fix.vc", 0,NULL,NULL,0,0,0
 };
struct Class fix_CLASS[] = {
 ODMI_MAGIC, "fix", sizeof(struct fix), fix_Descs, fix_ClassElem, fix_STRINGS,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };
struct Class fix_CLASS_R[] = {
 ODMI_MAGIC, "fix", sizeof(struct fix), fix_Descs, fix_ClassElem, fix_STRINGS_R,FALSE,NULL,NULL,0,0,NULL,0,"", 0,-ODMI_MAGIC
 };

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* This should be the only external structure for the swvpd     */
/* It contains all the control information for all the shared   */
/* information                                                  */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/


struct vpd_type vpd_ctl
            = { FALSE,                          /* vpd debugging off    */
                FALSE,                          /* odm not init         */
                VPD_LOCAL,                      /* current path         */

                 lpp_Descs,                     /* number of cols/fields*/
                 product_Descs,                 /* for each table type  */
                 history_Descs,
                 inventory_Descs,
		 fix_Descs,

                 lpp_ClassElem,                 /* access class element */
                 product_ClassElem,             /*    for each class    */
                 history_ClassElem,
                 inventory_ClassElem,
                 fix_ClassElem,
                                                /* end element table @s */
                                                /* start path info      */
                NULL,                           /* local path pointer   */
                 FALSE, FALSE, lpp_CLASS ,      /* init for each table  */
                 FALSE, FALSE, product_CLASS ,  /*  not open, not locked*/
                 FALSE, FALSE, history_CLASS ,  /*  control block addr  */
                 FALSE, FALSE, inventory_CLASS,
                 FALSE, FALSE, fix_CLASS ,
                                                /* end table            */
                                                /* end local path info  */

                NULL,                           /* remote path pointer  */
                 FALSE, FALSE, lpp_CLASS_R ,    /* init for each table  */
                 FALSE, FALSE, product_CLASS_R ,
                 FALSE, FALSE, history_CLASS_R ,
                 FALSE, FALSE, inventory_CLASS_R,
                 FALSE, FALSE, fix_CLASS_R
                                                /* end table            */
                                                /* end remote path info */
              } ;                               /* end initial values   */

