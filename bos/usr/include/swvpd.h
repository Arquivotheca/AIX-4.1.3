/* @(#)95       1.46  src/bos/usr/include/swvpd.h, cmdswvpd, bos41J, 9510A_all 2/27/95 12:15:25 */
#ifndef __H_SWVPD
#define __H_SWVPD
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: swvpd.h (libswvpd.a)
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <odmi.h>

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* The following structures form the interface to the ODM data  */
/* bases for the SWVPD.                                         */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

struct lpp {
        long _id;
        long _reserved;
        long _scratch;
        char name[145];
        long size;
        short state;
        long cp_flag;
        char *group;
        char magic_letter[2];
        short ver;
        short rel;
        short mod;
        short fix;
        char *description;
        short lpp_id;
        };
#define lpp_Descs 12

#define MAX_LPP_NAME 145
#define MAX_LPP_MLTR 2

struct product {
        long _id;
        long _reserved;
        long _scratch;
        char lpp_name[145];
        char comp_id[20];
        short update;
        long cp_flag;
        char fesn[10];
        char *name;
        short state;
        short ver;
        short rel;
        short mod;
        short fix;
        char ptf[10];
        short media;
        char sceded_by[10];
        char *fixinfo;
        char *prereq;
        char *description;
        char *supersedes;
        };
#define product_Descs 18

#define MAX_PROD_COMP 20
#define MAX_PROD_FESN 10
#define MAX_PROD_PTF  10
#define MAX_PROD_SCED 10

struct history {
        long _id;
        long _reserved;
        long _scratch;
        short lpp_id;
        short event;
        short ver;
        short rel;
        short mod;
        short fix;
        char ptf[10];
        char corr_svn[40];
        char cp_mod[10];
        char cp_fix[10];
        char login_name[18];
        short state;
        long time;
        char *comment;
        };
#define history_Descs 14

#define MAX_HIST_PTF  10
#define MAX_HIST_CSVN 40
#define MAX_HIST_CPMD 10
#define MAX_HIST_CPFX 10
#define MAX_HIST_LOG  18

struct inventory {
        long _id;
        long _reserved;
        long _scratch;
        short lpp_id;
        short private;
        short file_type;
        short format;
        char loc0[128];
        char *loc1;
        char *loc2;
        long size;
        long checksum;
        };
#define inventory_Descs 9

#define MAX_INV_LOC0 128

struct fix {
        long _id;
        long _reserved;
        long _scratch;
        char name[16];
        char abstract[60];
        char type[2];
        char *filesets;
        char *symptom;
        };
#define fix_Descs 5


/* Table Constants */
#define LPP_TABLE               0
#define PRODUCT_TABLE           1
#define HISTORY_TABLE           2
#define INVENTORY_TABLE         3
#define FIX_TABLE 	        4
#define COMMUNITY_TABLE         5
#define MERGING_LPP_TABLE     256


/* table structure typedefs - to be used outsid swvpd rtns */
typedef struct lpp              lpp_t ;
typedef struct product          prod_t ;
typedef struct inventory        inv_t ;
typedef struct history          hist_t ;
typedef struct fix              fix_t ;


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* The following symbols are to be used to specify the fields   */
/* of the interface structures to the commands that expect a    */
/* key_field_id parameter                                       */
/* The symbols (for any single structure) can be combined into  */
/* a composite field specification by 'or'ing these symbols     */
/* The symbol 'VPD_ALL' may not be so combined and specifies    */
/* all the fields in the structure.                             */
/* The masks are defined such that they correspond in order     */
/* with the fields in the structure. That must be preserved     */
/* by any modifications or updates to these definitions         */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define BIT(n) (1<<n)

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
#define VPD_ALL                 BIT(32)

/* Masks common to all SWVPD structures                         */
/* Following must not be combined with other field masks        */

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Field masks for lpp structure                                */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  LPP_NAME               BIT(0)
#define  LPP_SIZE               BIT(1)
#define  LPP_STATE              BIT(2)
#define  LPP_CP_FLAG            BIT(3)
#define  LPP_GROUP              BIT(4)
#define  LPP_MAGIC              BIT(5)
#define  LPP_VER                BIT(6)
#define  LPP_REL                BIT(7)
#define  LPP_MOD                BIT(8)
#define  LPP_FIX                BIT(9)
#define  LPP_DESC               BIT(10)
#define  LPP_LPPID              BIT(11)

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* field masks for product structure                            */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  PROD_LPP_NAME          BIT(0)
#define  PROD_COMP_ID           BIT(1)
#define  PROD_UPDATE            BIT(2)  /* decimal 131,072 */ 
#define  PROD_CP_FLAG           BIT(3)
#define  PROD_FESN              BIT(4)
#define  PROD_NAME              BIT(5)
#define  PROD_STATE             BIT(6)
#define  PROD_VER               BIT(7)
#define  PROD_REL               BIT(8)
#define  PROD_MOD               BIT(9)
#define  PROD_FIX               BIT(10)
#define  PROD_PTF               BIT(11)
#define  PROD_MEDIA             BIT(12)
#define  PROD_SCEDED_BY         BIT(13)
#define  PROD_FIXINFO           BIT(14)
#define  PROD_PREREQ            BIT(15)
#define  PROD_DESCRIPTION       BIT(16)
#define  PROD_SUPERSEDES        BIT(17)

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Field masks for history structure                            */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  HIST_LPP_ID            BIT(0)
#define  HIST_EVENT             BIT(1)
#define  HIST_VER               BIT(2)
#define  HIST_REL               BIT(3)
#define  HIST_MOD               BIT(4)
#define  HIST_FIX               BIT(5)
#define  HIST_PTF               BIT(6)
#define  HIST_CORR_SVN          BIT(7)
#define  HIST_CP_MOD            BIT(8)
#define  HIST_CP_FIX            BIT(9)
#define  HIST_LOGIN_NAME        BIT(10)
#define  HIST_STATE             BIT(11)
#define  HIST_TIME              BIT(12)
#define  HIST_COMMENT           BIT(13)

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Field masks for inventory structure                          */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  INV_LPP_ID             BIT(0)
#define  INV_PRIVATE            BIT(1)
#define  INV_FILE_TYPE          BIT(2)
#define  INV_FORMAT             BIT(3)
#define  INV_LOC0               BIT(4)
#define  INV_LOC1               BIT(5)
#define  INV_LOC2               BIT(6)
#define  INV_SIZE               BIT(7)
#define  INV_CHECKSUM           BIT(8)

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Field masks for fix structure                                */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  FIX_NAME               BIT(0)
#define  FIX_ABSTRACT           BIT(1)
#define  FIX_TYPE               BIT(2)
#define  FIX_FILESETS           BIT(3)
#define  FIX_SYMPTOM            BIT(4)


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Definitions for the state values that can be assumed by the  */
/* enumerated fields in the structures.                         */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'state' field in either lpp or product            */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  ST_AVAILABLE           1
#define  ST_APPLYING            2
#define  ST_APPLIED             3
#define  ST_COMMITTING          4
#define  ST_COMMITTED           5
#define  ST_REJECTING           6
#define  ST_BROKEN              7
#define  ST_DEINSTALLING        8
	/* the following are applied to all parts of an LPP when */
	/* that LPP is replaced by another install of that LPP	 */
	/* When the new version is committed entries flagged     */
	/* as 'hold' will be removed				 */
#define  ST_APPLY_HOLD		9
#define  ST_COMMIT_HOLD		10

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'event' field in history                          */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  HIST_APPLY             1
#define  HIST_COMMIT            2
#define  HIST_REJECT            3
#define  HIST_CLEANUP           4
#define  HIST_DEINSTALL         5

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'state' field in history                          */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define   HIST_ACTIVE           1
/* above is old, use HIST_COMPLETE as prefered code		*/
#define   HIST_COMPLETE         1
#define   HIST_PENDING          2
#define   HIST_BROKEN           3
#define   HIST_CANCELLED        4

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for the lpp control record.  This is the control 
   record used for lpp_id generation.  LCR = Lpp Control Record */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define LPP_CTL_NAME            "__SWVPD_CTL__"

/*<><><><><><><><><><><>*/
/*  LCR cp_flag values  */
/*<><><><><><><><><><><>*/

#define LCR_BIRON_BIT           BIT(0)  /* used by installp and inurid */ 


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'cp_flag' field in lpp or product                 */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  LPP_USER               BIT(0)
#define  LPP_ROOT               BIT(1)
#define  LPP_SHARE              BIT(2)
#define  LPP_MICRO              BIT(3)
#define  LPP_INSTAL             BIT(4)
#define  LPP_UPDATE             BIT(5)
#define  LPP_31_FMT             BIT(6)
#define  LPP_BOSBOOT		BIT(12)
#define  LPP_COMPAT             BIT(13)
#define  LPP_MIGRATING          BIT(14)
#define  LPP_INSTALLED_ON_32    BIT(15)
#define  LPP_OEM_SPECIFIC       BIT(16)


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Macros using the cp_flag bits                                */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  COMPAT(t)              (t & LPP_COMPAT)
#define  MIGRATING(t)           (t & LPP_MIGRATING)
#define  INSTALLED_ON_32(t)     (t & LPP_INSTALLED_ON_32)


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'media' field in product                          */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  PROD_MED_DISK          1
#define  PROD_MED_TAPE          2
#define  PROD_MED_FILE          3

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'update' field in product                         */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define INU_TRUE     1   /* This record IS an update record     */
#define INU_FALSE    0   /* This record is NOT an update record */


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'type' field in inventory                         */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  INV_CFG                BIT(0)
                /* File is known configuration file
                 * to be processed as such, size/checksum
                 * may be expected to be changed
                 */
#define  INV_ARC                BIT(1)
                /* File is archive file, size/checksum may
                 * be changed
                 */
#define  INV_SRC                BIT(2)
                /* File is part of source distribution
                 */

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* Values for 'format' field in inventory                       */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define  INV_FILE               1
                /* loc0 is file name
                 * loc1 is list of links to that file
                 * loc2 is list of symlinks to that file
                 */
#define  INV_ARCHIVED           2
                /* loc0 is file as distributed
                 * loc1 is archive file that receives file
                 */


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*VPD Error messages                                            */

#define VPD_OK                  0
#define VPD_SYS                 -2
#define VPD_BADCNV              -3
#define VPD_BADCOL              -4
#define VPD_BADTBL              -5
#define VPD_SQLMAX              -6
#define VPD_NOMATCH             -7
#define VPD_NOID                -8


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/* generally useful constants                                   */

                                        /* number of tables supported   */
#define N_tbls                  5

                                        /* number of paths supported    */
#define N_paths                 2

                                        /* longest field that can be    */
                                        /* used in a search (lpp_name)  */
#define MAX_TBL_CHAR            145

                                        /* longest name for field that  */
                                        /* can be used in a search      */
                                        /* (time_last_update)           */
#define MAX_TBL_NAME            16

                                        /* buffer size for queries      */
#define MAX_QUERY_LEN		MAX_ODMI_CRIT

                                        /* size of largest table struct */
#define MAX_TBL_SIZE            sizeof(struct product)

                                        /* default paths to vpd tables  */
#define VPD_LOCAL_PATH          "/usr/lib/objrepos"
#define VPD_REMOTE_PATH         "/etc/objrepos"

#define  VPD_ROOT_PATH          VPD_REMOTE_PATH
#define  VPD_USR_PATH           VPD_LOCAL_PATH
#define  VPD_SHARE_PATH         "/usr/share/lib/objrepos"

#endif

