/* @(#)36  1.9  src/bos/usr/bin/lslpp/lslpp.h, cmdswvpd, bos411, 9428A410j 6/6/94 18:21:13 */

/*
 *
 *
 * COMPONENT_NAME: CMDSWVPD
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <check_prereq.h>
#include        <stdio.h>
#include        <time.h>
#include        <string.h>
#undef MSG_NOMEM
#undef USRLOC_E
#undef SHARELOC_E
#undef OFLG_E
#undef INVALIDFLG_E
#undef INU_SHRTOOMANY_E
#undef INU_USRTOOMANY_E
#undef MAX_LPP_FULLNAME_LEN
#include        <swvpd.h>
#include        <locale.h>
#include        <stdlib.h>
#include        <ctype.h>
#include        <odmi.h>
#include        <swvpd_str.h>

void process_L_flag (void);

#ifndef TRUE
#define TRUE    (1==1)
#define FALSE   (1!=1)
#endif

/* The following defines are copies of defines from instal/inudef.h */
#ifndef NIL
#define NIL(t)     ((t *)0)
#endif
                                     /* lpp.opt.opt.opt.opt + NULL      */
#define MAX_LPP_FULLNAME_LEN     MAX_LPP_NAME

                                     /* MAX_LPP_NAME + 1 (blank)        */
                                     /* + 2 (ver) + 1 (period) + 2 (rel)*/
                                     /* + 1 (period) + 4 (mod)          */
                                     /* + 1 (period) + 4 (fix)          */
#define MAX_LPP_NAME_PLUS_LEN    MAX_LPP_FULLNAME_LEN+16

#define MAX_PTF_ID_LEN     10
#define PROGNAME   "lslpp"

/* magic letter constant in lpp_t                               */
#define LPP_IBM_MAGIC   'I'

/* unknown setting for lpp/product state                        */
#define ST_UNKNOWN      (-1)

/* Command-line options.                                        */
#define FLAGSTR         "lhpdifqcAaBIO:JL?"

/* The following are mutually exclusive, and describe which     */
/* type of information is to be printed                         */
#define   FLAG_LIST             'l'
#define   FLAG_HISTORY          'h'
#define   FLAG_PREREQS          'p'
#define   FLAG_DEPENDENTS       'd'
#define   FLAG_PRODID           'i'
#define   FLAG_FILES            'f'
#define   FLAG_APARS            'A'
#define   FLAG_CAP_L            'L'
/*
** The following can be added to the above, to describe the output
** format.  Quiet means no header line; colons means that output
** fields are separated by colons, not whitespace.
*/
#define   FLAG_QUIET            'q'
#define   DEFINE_QUIET          "q"
#define   FLAG_COLONS           'c'
#define   DEFINE_COLONS         "c"
#define   FLAG_ALL              'a'
#define   DEFINE_ALL            "a"
#define   FLAG_FIXES            'B'
#define   DEFINE_FIXES          "B"
#define   FLAG_LPPS             'I'
#define   DEFINE_LPPS           "I"
#define   FLAG_OPTIONS          'O'
#define   DEFINE_OPTIONS        "O"
#define   FLAG_SMIT             'J'
#define   DEFINE_SMIT           "J"

/* Max number of fixes per LPP this code will handle without overflowing */
#define   MAX_FIXES     (100)

/* Max number of input operands                                 */
#define   MAX_INPUTS    (100)


/* Exit codes                                                   */
#define   NO_ERROR      VPD_OK
#define   ERROR         VPD_OK+1

/* Line length for the report output                            */
#define MAX_FLD_WIDTH           255
#define LPP_FLD_WIDTH           20
#define DESC_FLD_WIDTH          32
#define SMIT_DESC_FLD_WIDTH     21
#define SMIT_SHORT_DESC_FLD_WIDTH     12
#define STATE_FLD_WIDTH         12
#define FIX_FLD_WIDTH           7
#define FIX_FLD_WIDTH_FOR_L     12
#define GAP_WIDTH               7
#define HSTATE_FLD_WIDTH        10
#define EVENT_FLD_WIDTH         10
#define USER_FLD_WIDTH          10
#define DATE_FLD_WIDTH          10
#define TIME_FLD_WIDTH          8
#define REL_FLD_WIDTH           15
#define PRQ_FLD_WIDTH           32
#define PRQ_FLD_WIDTH_LONG      55
#define DEPEND_FLD_WIDTH        36
#define DEPEND_STATE_FLD_WIDTH  16
#define PID_FLD_WIDTH           10
#define FID_FLD_WIDTH           10
#define PNM_FLD_WIDTH           20
#define VENDOR_FLD_WIDTH        6
#define VENDR_FLD_WIDTH         5
#define FILE_FLD_WIDTH          20
#define APAR_FLD_WIDTH          80
#define DESC_FLD_WIDTH_FOR_L    45
#define STATE_FLD_WIDTH_FOR_L   6
#define SMIT_DESC_FLD_WIDTH_FOR_L    45
#define SMIT_STATE_FLD_WIDTH_FOR_L   6


#define MAX_DESC_LENGTH         4095
#define APAR_SEPERATOR \
    "-----------------------------------------------------------------------"

#define NAME_LEV_STATE           1      /* Used to determine print format   */
#define _32_PTF                  2      /*   in print_fileset()             */
#define _32_PTF_W_LEV            3
#define LEV_STATE                4
#define MAX_LEV_LENGTH           7

#define NAME_LEV_SPACE           2
#define LEV_STATE_SPACE          2
#define STATE_DESC_SPACE         2

#define MAX_NAME_LEV_FOR_1_LINE  34
#define SMIT_MAX_NAME_LEV_FOR_1_LINE 18
#define START_OF_STATE_COL       39
#define SMIT_START_OF_STATE_COL  24

#define FULL_LINE_LENGTH         80
#define MAX_DASH_LINE_LENGTH     76
#define SMIT_FULL_LINE_LENGTH    60

#define HIST_LEVEL_SPACE         25
#define HIST_STATE_SPACE         12
#define HIST_EVENT_SPACE         12
#define HIST_DATE_SPACE          12
#define HIST_TIME_SPACE          12
#define HIST_FILESET_HDR_SPACE   15
#define HIST_LEVEL_HDR_SPACE      9

#define IS_MAINT_LEV(fix)     (IF_LPP_PKG_PTF_TYPE_C (fix->cp_flag) || \
                               IF_LPP_PKG_PTF_TYPE_ML (fix->cp_flag))

#define MAX_LINE_WIDTH   79
#define MAX_USE_AS_ARG   7

char            format[255];
static char     source[22];
static int      max_name_lev_for_1_line;
static int      start_of_desc;
static int      max_desc_length;
static char     smit_hash[2];
static int      max_state_len;

/* used to control the list of product structures read          */

struct prod_list_ctl    /* structure of addresses and control   */
{ /* information                          */
   int             max; /* number of entries allocated for ptrs */
   int             used;        /* number of ptrs that have values      */
   prod_t         *ptrs[1];     /* ptrs to prod_t structs, actual number */
}              *Fix_List;       /* is set at time struct is malloc'ed   */

#define HAVENT_PRINTED_THIS_OPTION(cur_fix, prev_printed)               \
          ((prev_printed == NIL (prod_t))                            || \
           (strcmp (cur_fix->lpp_name, prev_printed->lpp_name) != 0))

#define FL(n) Fix_List->ptrs[n] /* access n'th pointer          */
#define FL_LAST Fix_List->ptrs[Fix_List->used -1 ]
/* access last valid entry in Fix_List  */
#define FL_USED Fix_List->used  /* get count of used entries            */

#define LSLPP_ST_OBSOLETE  11  /* used to index state array for special state
                                  known only to lslpp for MIGRATING filesets */
#ifdef MAIN_PROGRAM
   int      no_header;      /* dont print header flag               */
   int      colons;         /* colon separated output flag          */
   int      smitified;      /* indicates if output format needs     */
                            /*      to be smitified                 */
   int      all_info;       /* all information                      */
   int      flagavailable=0;/* ON if -a is used                     */
   int      microcode;      /* include microcode information        */
   int      ptf_inputs_only;        /* inputs are only ptf ids      */
   int      base_levels_only;       /* only show base levels        */
   int      path_specified;         /* the user specified the       */
                                    /*       search paths           */
   int      root_path;      /* root path was specified              */
   int      usr_path;       /* /usr path was specified              */
   int      share_path;     /* /usr/share path was specified        */

   /* Max size of input parameters                                 */

   char      **inputs;
   int       *inputs_gotone;
   int       input_count;
   char      command_flag;
   char     progname[50] = PROGNAME;
#else
   extern int      no_header;      /* dont print header flag               */
   extern int      colons;         /* colon separated output flag          */
   extern int      smitified;      /* indicates if output format needs     */
                                   /*      to be smitified                 */
   extern int      all_info;       /* all information                      */
   extern int      flagavailable;  /* ON if -a is used                     */
   extern int      microcode;      /* include microcode information        */
   extern int      ptf_inputs_only;        /* inputs are only ptf ids      */
   extern int      base_levels_only;        /* only show base levels       */
   extern int      path_specified;         /* the user specified the       */
                                           /*       search paths           */
   extern int      root_path;      /* root path was specified              */
   extern int      usr_path;       /* /usr path was specified              */
   extern int      share_path;     /* /usr/share path was specified        */

   extern char      **inputs;
   extern int       *inputs_gotone;
   extern int       input_count;
   extern char      command_flag;
   extern char     progname[50];
#endif

char * Lines (int fld_width);

char * WrapText (char * source,
                 int    fld_width,
                 int  * offset);

char   get_type (fix_info_type * fix, prod_t * prod_rec);

void print_description (
char * desc,                    
int    WrapText_fld_width,     
int    state_str_length,      
int    start_of_state_col,   
int    max_state_length,    
char * smit_hash);

int free_unused_memory (prod_t * entry);
int free_done_memory (prod_t * entry);

char * LPP_State (prod_t * fix_record);

int fl_plus (void);
int fl_minus (void);

int get_name (prod_t * fix_record, char   * name);

int sort_history (hist_t * hist_list[],
                  int      left,
                  int      right);

int sort_u_product (void);

int usage (void);

int remove_blanks (char * str);

char get_state (fix_info_type * fix);

fix_info_type * locate_fix_local (char * name,
                                  char * ptf);

void print_legend (void);

void sort_inputs (int   input_count,
                  char  *inputs[]);

char * get_description (long cp_flag, char * name, char * desc);


