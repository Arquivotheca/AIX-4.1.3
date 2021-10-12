/* @(#)48  1.52  src/bos/usr/include/inu_toc.h, cmdinstl, bos41J, 9510A_all 2/27/95 12:15:22 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: IF_3_1
 *              IF_3_1_INSTALL
 *              IF_3_1_UPDATE
 *              IF_3_2
 *              IF_3_X
 *              IF_4_1
 *              IF_ACT_UPDATE
 *              IF_DUPE
 *              IF_GOLD_UPDATE
 *              IF_INSTALL
 *              IF_UPDATE
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

#ifndef __H_TOC
#define __H_TOC

#include <stdio.h>

/*----------------------------------------------------------------------
 * Level_t
 *----------------------------------------------------------------------*/
typedef struct {
    short sys_ver;
    short sys_rel;
    short sys_mod;
    short sys_fix;
    short ver;
    short rel;
    short mod;
    short fix;
    char ptf[10];
} Level_t;

/*----------------------------------------------------------------------
 * Netls_t
 *----------------------------------------------------------------------*/
typedef struct {
   char *vendor_id;
   char *prod_id;
   char *prod_ver;
} Netls_t;

/*----------------------------------------------------------------------
 * typedefs
 *----------------------------------------------------------------------*/
typedef struct OptionRef__ OptionRef_t;
typedef struct BffRef__    BffRef_t;
typedef struct Option__    Option_t;
typedef struct TOC__       TOC_t;

/*----------------------------------------------------------------------
 * OptionRef_t
 *----------------------------------------------------------------------*/
struct OptionRef__ {
  Option_t      *option;
  OptionRef_t   *next;
};

/*----------------------------------------------------------------------
 * Option_t
 *---------------------------------------------------------------------*/
struct Option__ {
  char  name[MAX_LPP_FULLNAME_LEN];/* Identifying name for this backup-set */
  char  prodname[MAX_LPP_FULLNAME_LEN];/* Id'ing product name for backup-set */
  char  quiesce;        /* Indicates if sub-system must stop            */
  char  content;        /* Indicates the contents of the option         */
  char  vpd_tree;       /* Flag indicating the tree we're dealing with  */
  char  lang[MAX_LANG_LEN+1]; /* supported language                     */
  char *desc;           /* the description of the lpp                   */
  int   op_checked;     /* has this option already been checked         */
  int   operation;      /* The type of operation to be performed        */
  int   op_type;        /* The type of option it is.                    */
  char  *fixdata;       /* Fix data (not fixinfo) for this option       */
  int   Status;         /* Result of the requested operation.           */
  char  *size;          /* List of size requirements for option         */
  char  *prereq;        /* List of requisite LPP options                */
  char  *supersedes;    /* List of PTFs that are within this opt        */
  BffRef_t *bff;        /* bff struct ptr that this option is part of   */
  Option_t *next;       /* The next option in this linked list of opts  */
  Option_t *SelectedList; /* Ptr to the same option in the Sop          */
  Level_t  level;       /* version, release, modification, fix, ptf     */
  int   flag;           /*     A generic 32 bit set of flags            */
                        /*  Bit Number      Flag Description            */
                        /*     1       -   0 ==> This opt is NOT a dupe */
                        /*             -   1 ==> this opt IS a dupe     */
                        /*     2       -   currently undefined          */
                        /*     3       -   currently undefined          */
                        /*     4       -   currently undefined          */
                        /*     5       -   currently undefined          */
                        /*     6       -   currently undefined          */
                        /*     7       -   currently undefined          */
                        /*     8       -   currently undefined          */
  Netls_t  *netls;      /* version, release, modification, fix, ptf     */
  Option_t *hash_next;  /* For hashing purposes */
};
/*---------------------------------
 * Values for [quiesce] above
 *-------------------------------*/
# define QUIESCE_YES    'Y'     /* Both /usr and root trees */
# define QUIESCE_NO     'N'     /* Micro code               */

#define QUIESCE_YES_BOSBOOT_YES 'B'     /* Quiesce and perform bosboot       */
#define QUIESCE_NO_BOSBOOT_YES  'b'     /* Don't Quiesce but perform bosboot */

#define QUIESCE_TO_BOSBOOT(m)   \
        ((m == QUIESCE_YES_BOSBOOT_YES) || (m == QUIESCE_NO_BOSBOOT_YES))

/*---------------------------------
 * Values for [content] above
 *-------------------------------*/
# define CONTENT_BOTH       'B'     /* Both /usr and root trees */
# define CONTENT_MCODE      'D'     /* Micro code               */
# define CONTENT_SHARE      'H'     /* /usr/share tree          */
# define CONTENT_MRI        'M'     /* MRI (3.1 only)           */
# define CONTENT_OBJECT     'O'     /* Object code (3.1 only)   */
# define CONTENT_PUBS       'P'     /* Pubs (3.1 only)          */
# define CONTENT_USR        'U'     /* /usr tree only           */
# define CONTENT_UNKNOWN    ' '     /* content unknown      */

/*---------------------------------
 * Values for [vpd_tree] above
 *-------------------------------*/
# define VPDTREE_USR    'U'         /* operation deals with /usr tree */
# define VPDTREE_ROOT   'M'         /* operation deals with root tree */
# define VPDTREE_SHARE  'S'         /* operation deals with /usr/share tree */

/*---------------------------------
 * Values for [op_type] above
 *--------------------------------*/
# define OP_TYPE_INSTALL        1          /* option is an install image    */
# define OP_TYPE_UPDATE         2          /* option is an update image     */
# define OP_TYPE_3_1            4          /* option is for release 3.1     */
# define OP_TYPE_3_2            8          /* option is for release 3.2     */
# define OP_TYPE_4_1           16          /* option is for release 4.1     */
# define OP_TYPE_VERSION_MASK  (4 | 8 | 16)         /* bits 3, 4, and 5     */
# define OP_TYPE_C_UPDT        32          /* option is a 'C' update type   */
# define OP_TYPE_E_UPDT        64          /* option is an 'E' update type  */
# define OP_TYPE_ML_UPDT      128          /* option is an 'ML' update type */
# define OP_TYPE_M_UPDT       256          /* option is an 'M' update type  */
# define OP_TYPE_MIGRATING    512          /* option is partially migrated  */
# define OP_TYPE_BOSBOOT     4096          /* Must be same as LPP_BOSBOOT   */

/*---------------------------------
 * MACROS for [op_type] above
 *--------------------------------*/
#define IF_INSTALL(t)        (t & OP_TYPE_INSTALL)
#define IF_UPDATE(t)         (t & OP_TYPE_UPDATE)
#define IF_3_1(t)            (t & OP_TYPE_3_1)
#define IF_3_2(t)            (t & OP_TYPE_3_2)
#define IF_3_X(t)           ((t & OP_TYPE_3_1) || (t & OP_TYPE_3_2))
#define IF_4_1(t)            (t & OP_TYPE_4_1)
#define IF_3_1_INSTALL(t)    (IF_3_1(t) && IF_INSTALL(t))
#define IF_3_1_UPDATE(t)     (IF_3_1(t) && IF_UPDATE(t))
#define IF_3_2_UPDATE(t)     (IF_3_2(t) && IF_UPDATE(t))
#define IF_C_UPDT(t)         (t & OP_TYPE_C_UPDT)
#define IF_E_UPDT(t)         (t & OP_TYPE_E_UPDT)
#define IF_ML_UPDT(t)        (t & OP_TYPE_ML_UPDT)
#define IF_M_UPDT(t)         (t & OP_TYPE_M_UPDT)
#define IF_MIGRATING(t)      (t & OP_TYPE_MIGRATING)
#define IF_BOSBOOT(t)        (t & OP_TYPE_BOSBOOT)

/*---------------------------------
 * Values for [operation] above
 *--------------------------------*/
# define OP_APPLY               0   /* operation is to apply */
# define OP_APPLYCOMMIT         1   /* operation is to apply and commit */
# define OP_COMMIT              2   /* operation is to commit */
# define OP_CLEANUP_APPLY       3   /* operation is to cleanup */
# define OP_CLEANUP_COMMIT      4   /* operation is to cleanup */
# define OP_REJECT              5   /* operation is to reject */
# define OP_STATUS              6   /* operation is to show status */
# define OP_INFO                7   /* operation is to show instruction info */
# define OP_DEINSTALL           8   /* operation is deinstall */
# define OP_PIF_FAILURE         9   /* operation is non-existent  */


/*---------------------------------
 * Values for [Status] above
 *--------------------------------*/
# define STAT_SUCCESS           0
# define STAT_IFREQ_FAIL        1
# define STAT_CANCEL            2
# define STAT_BYPASS            2
# define STAT_FAILURE           3
# define STAT_CLEANUP           4
# define STAT_CLEANUP_SUCCESS   5
# define STAT_CLEANUP_FAILED    6
# define STAT_EXPAND_FAIL       7
# define STAT_FAILURE_INUCONVERT 8

/*---------------------------------
 * Pre-installation Failure values
 * for [Status]
 *--------------------------------- */
# define STAT_NOT_FOUND_ON_MEDIA         9
# define STAT_REQUISITE_FAILURE         10
# define STAT_ALREADY_SUPERSEDED        11
# define STAT_ALREADY_INSTALLED         12
# define STAT_TO_BE_SUPERSEDED          13
# define STAT_CAN_BE_SUPERSEDED         14
# define STAT_ROOT_CAN_BE_SUPERSEDED    15

/*
# define STAT_BROKEN_NEEDS_COMMIT       16
*/

# define STAT_BASE_MUST_BE_COMMITTED    17
# define STAT_BASE_ALREADY_INSTALLED    18
# define STAT_NUTTIN_TO_APPLY           19
# define STAT_NUTTIN_TO_COMMIT          20
# define STAT_NUTTIN_TO_REJECT          21
# define STAT_NUTTIN_TO_DEINSTL         22
# define STAT_MUST_APPLY_ROOT_TOO       23
# define STAT_ALREADY_COMMITTED         24

/*
# define STAT_OTHER_BROKENS_NEED_COMMIT 25
*/

# define STAT_WARN_DEINST_3_1           26
# define STAT_WARN_DEINST_3_2           27
# define STAT_FAILED_PRE_D              28
# define STAT_NO_DEINST_BOS             29
# define STAT_WARN_DEINST_MIG           31

# define STAT_PART_INCONSIST            32
# define STAT_NOTHING_FOUND_ON_MEDIA    33
# define STAT_DUPE_VERSION              34

# define STAT_BROKEN                    35
# define STAT_OTHER_BROKENS             36

# define STAT_COMMITTED_CANT_REJECT     37

# define STAT_ALL_KW_FAILURE            39
# define STAT_NO_USR_MEANS_NO_ROOT      40
# define STAT_NO_FORCE_APPLY_PTF        41

# define STAT_SUP_OF_BROKEN             42

# define STAT_OEM_MISMATCH		43
# define STAT_OEM_REPLACED		44
# define STAT_OEM_BASELEVEL		45

/*---------------------------------
 * Values for [ReqStatus] above
 *--------------------------------*/
# define REQSTAT_FAILED -1          /* prereq check failed */
# define REQSTAT_PASSED  1          /* prereq check passed */
# define REQSTAT_NOTCKD  0          /* prereq check has not been done */

/*---------------------------------
 * Macros for [flag] above
 *--------------------------------*/
#define IF_DUPE(x)  (x & 1)         /* see if 1st bit is on yes ==> is a dupe */
#define IF_SELECTED(x)  (x & 2)     /* If user EXPLICITLY requested this op   */
#define IF_OTHER_PART_ON_SOP(x) (x & 4)
                                    /* set for one part of pkg if sop has both 
                                       usr and root parts. */

#define SET_DUPE_BIT(x)      (x | 1)      /* set the dupe bit to true      */
#define SET_SELECTED_BIT(x)  (x | 2)      /* Set the requested bit to true */
#define SET_OTHER_PART_ON_SOP_BIT(x) (x | 4) /* Set the "other part" bit */



/*----------------------------------------------------------------------
 * BffRef_t
 *----------------------------------------------------------------------*/
struct BffRef__ {
  int    vol;            /* Volume on which this BFF located     */
  int    off;            /* Offset into the above volume of BFF  */
  int    size;           /* Size of this bff in bytes            */
  char   fmt;            /* Media Format                     */
  char   platform;       /* The platform for this tape       */
  int    action;         /* Code indicating install/update (I/U) */
  char * action_string;  /* String value of action. */
  char * path;           /* The media on which this bff resides  */
  int    crc;            /* Cyclical Redundancy Check        */
  int    flags;
  OptionRef_t   *options;   /* LPP Options in this BFF      */
  BffRef_t      *next;      /* Next BFF in TOC          */
};

/*---------------------------------
 * Values for [fmt] above
 *--------------------------------*/
# define FMT_3_1    '1'
# define FMT_3_1_1  '2'
# define FMT_3_2    '3'
# define FMT_4_1    '4'


/*---------------------------------
 * Values for [action] above
 *--------------------------------*/
# define ACT_INSTALL          0
# define ACT_UNKNOWN          1
# define ACT_OTHER            2
# define ACT_SING_UPDT        3
# define ACT_MULT_UPDT        4
# define ACT_GOLD_UPDT        5
# define ACT_EN_PKG_UPDT      6
# define ACT_EN_MEM_UPDT      7
# define ACT_INSTALLP_UPDT    8
# define ACT_REQUIRED_UPDT    9
# define ACT_CUM_UPDT        10
# define ACT_MAINT_LEV_UPDT  11

/*---------------------------------
 * Macros for [action] above
 *--------------------------------*/

#define  IF_ACT_UPDATE(t) ((t >= ACT_SING_UPDT && \
                            t <= ACT_MAINT_LEV_UPDT) ? 1 : 0)


#define  IF_GOLD_UPDATE(t) ((t==ACT_GOLD_UPDT) ? 1 : 0)

/*---------------------------------
 * Values for [platform] above
 *--------------------------------*/
# define PLAT_PS2       'P'
# define PLAT_6000      'R'
# define PLAT_S370      'E'
# define PLAT_UNKNOWN   ' '


/*---------------------------------
 * Values for [flags] above.
 *---------------------------------*/

#define BFF_VISITED 1
#define BFF_PASSED  2
#define BFF_FAILED  4

/*----------------------------------------------------------------------
 * TOC_t
 *----------------------------------------------------------------------*/
struct TOC__ {
           int   vol;        /* Volume number                            */
  unsigned int   dt;         /* Creation time of first volume in set     */
           char  media[PATH_MAX];
           int   hdr_fmt;    /* Format of header */
           int   type;       /* PRELOAD, TAPE_BFF, TAPE_TOC, BFF */

  BffRef_t  *content;
  Option_t  *options;
};
/*---------------------------------
 * Values for [type] above
 *--------------------------------*/
# define TYPE_DISK      0
# define TYPE_TAPE_SKD  1
# define TYPE_TAPE_BFF  2
# define TYPE_FLOP_SKD  3
# define TYPE_FLOP_BFF  4

/*---------------------------------
 * Values for [hdr_fmt] above.
 *--------------------------------*/
# define TOC_FMT_NONE       0
# define TOC_FMT_3_1        1
# define TOC_FMT_3_2        2

#define TOC_HASH_SIZE 800 

#endif  /* ifndef __H_TOC   */
