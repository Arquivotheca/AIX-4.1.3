/* @(#)32       1.22  src/bos/usr/bin/smit/csmitlocal.h, cmdsmit, bos41J, 9514A_all 4/4/95 13:06:19 */
/*
 * COMPONENT_NAME: (CMDSMIT) SMIT -- System Management Interface Tool
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define SM_OS_MAX 6
#define DSMIT_OBJREPOS    "/usr/share/DSMIT"
#define ODMDIR_CMD "ODMDIR=/etc/objrepos;export ODMDIR; "

#define SMIT_ORI_SET_NUM   98
#define SMIT_PROG_SET_NUM  99
#define SMIT_HELP_SET_NUM 100
#define EXIT_IF_EXIT(ret)    \
    ( (ret == ASL_EXIT) ? (smit_exit(0, FALSE), exit(0), 0 ) : 0 )

#define _ILS_MACROS
#include "smit_cat.h"
#include <sys/limits.h>
#include <sys/param.h>
#include <nl_types.h>
extern nl_catd catd;
#include "smit_msg.h"
#define MSGSTR(msg_num, string) \
                catgets(catd, SMIT_PROG_SET_NUM, msg_num, string)
#define MSGSTR_HELP(msg_num, string) \
                catgets(catd, SMIT_HELP_SET_NUM, msg_num, string)

/*** START DSMIT SECURITY CHANGE ***/
#ifdef _DSMIT
 #include "cred.h"
 #include "sec_init_exit.h"
#endif

#include <pwd.h>
/*** END DSMIT SECURITY CHANGE ***/

#include "asl.h"
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#ifdef _AIX
 #include <odmi.h>
#endif
#ifdef _ORI
#include <oricfg.h>
#include <oridef.h>
#include <ori.h>
#endif
#ifdef _ORI2
#include <ksccfg.h>
#include <kscdef.h>
#include <ori2.h>
#endif
#if ! defined(_IBM5A)
#include <locale.h>
#endif
#include <signal.h>
#include "smit.h"
#include "smit_class.h"
#include <stdio.h>
#if ! defined(_IBM5A)
#include <stdlib.h>
#endif
#include <string.h>
#if defined(_IBM5A)
#include <sys/select.h>
#else
#include <sys/poll.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <time.h>
#include <unistd.h>
#include <iconv.h>
#include <langinfo.h>
#include <ctype.h>

/*
 * Data Structure Notes
 *
 *      When retrieving arrays of objects from the ODM for subsequent display
 *      in ASL, SMIT will (depending on the screen type, etc.) build
 *      arrays of structures of type struct sm_menu_opt or struct sm_cmd_opt.
 *      For such arrays, a corresponding array of structures of type
 *      ASL_SCR_INFO will be constructed such that a given index (offset by 1)
 *      indexes corresponding entries in the SMIT[n-1] and ASL[n] arrays.
 *      The zero and maximum ASL_SCR_INFO indices are exceptions
 *      to this rule.  Since the index-zero entry of ASL_SCR_INFO
 *      correspones to the screen title and overall screen information,
 *      information in this entry is usually corresponds to a
 *      struct sm_cmd_hdr
 *      or struct sm_name_hdr * structure.  (struct sm_menu_opt is a special
 *      case here, since it serves both roles.)
 */

#define EMPTY_STRING    ""
#define AGC_CHAR        '\xb7'          /* the "a Grave Capital" 1-byte char */


#define SM_TABLE_MAX           300
#define SM_IO_BUFFER_SIZE     4096
#define SM_CMD_LINE_STRLEN   33808
#define SM_ID_STRLEN           512
#define SM_ID_SEQ_NUM_STRLEN    16
#define SM_CRITERIA_STRLEN     256
#define SM_INPUT_LINE          256

#define SM_HOST_MAX             2000 

#define SMIT_CAT_FILE_NAME      "smit.cat"
#define SM_TOP_MENU_OPTION_ID   "top_menu"
#define SM_GENERAL_HELP_MSG_ID  "18001"
#define SM_NO_HELP_MSG_ID       "00001"

#define DSMIT_MODIFY_COLLECTIVE_ID      "wc_incexc_DH"
#define DSMIT_TOP_MENU_OPTION_ID        "root"

#define SCR_MAX         scr_type->max_index
#define SCR_CUR         scr_type->cur_index
#define OPEN		1
#define CLOSE		0

                                        /* ---- malloct() control ----       */
#define MALLOC_BLOCK 8000               /* how much to get/free for sbrk()   */
                                        /* avoidance                         */
#define MALLOC_RESET 10                 /* how many mallocs to do before     */
                                        /* resetting counter                 */


#define SMIT_FREE(addr) smit_free(addr, __LINE__)
#define SMIT_MALLOCT(addr) smit_malloct(addr, __LINE__)
#define SMIT_REALLOCT(addr, size) smit_realloct(addr, size, __LINE__)

#define SM_MAX_OPTS SM_TABLE_MAX		/* Max number of sm_cmd_opts: Defect 151356 */

/* Variables */

                                        /* ---- ODM defs                     */
extern int      odmcf_errno;            /* gives ODM error numbers           */
extern int      odmtrace;               /* TRUE turns on ODM logging         */
extern int      odm_init_status;   /* odm_initialize() return code      */

                                        /* ---- Misc. global params          */
extern int     ghost_flag; /* TRUE means just displayed ghost   */
                                        /* dialogue or name select screen    */
extern FILE   *fp_sm_log;               /* log file pointer                  */
extern FILE   *fp_sm_script;            /* script file pointer               */
extern char    fast_out_buf[];/* for faster windowing              */
                                        /* ---- command line setable flags   */
extern int      help_report;/* TRUE means generate help report   */
extern int      no_execute_X;/* TRUE means don't                  */
                                        /*      exec() any cmd_to_*          */
extern int      no_execute_x;/* TRUE means don't                  */
                                        /*      exec() cmd_to_exec           */
extern int      trace_flag;/* TRUE means log trace messages     */
extern int      trace_flag_discover_value;/*TRUE means already displayed value*/
extern int      verbose_flag;/* TRUE means log verbose messages   */
extern int      debug_flag;/* TRUE meand log debug messages     */
extern int      menu_flag;/* TRUE means lookup menu            */
extern int      name_hdr_flag;/* TRUE means lookup name selector   */
extern int      dialogue_flag;/* TRUE means lookup dialogue        */
extern int      search_flag;/* TRUE means lookup in search path  */
                                        /* order                             */
extern int      stderr_flag;/* TRUE means write log info.        */
                                        /* to stderr                         */
extern int      filter_flag;/* TRUE means make SMIT work         */
                                        /* reasonably as a filter            */
extern char     *smit_objrepos; /* alternate objrepos path           */
extern char     *log_path; /* alternate log file path           */
extern char     *script_path; /* alternate script file path        */
extern char     *env_var_log_path; /* for environment variable  */
extern char     *env_var_script_path; /* for environment variable  */
                                        /* for environment variable          */
extern char     env_var_verbose_flag [];
                                        /* for environment variable          */
extern char     env_var_trace_flag   [];
                                        /* for environment variable          */
extern char     env_var_debug_flag   [];
					/* DSMIT variable		     */
extern int      DSMIT;
extern int      SERVER_HOST;            /* Defect 151356 */
extern char*    data_file[SM_HOST_MAX+1][SM_TABLE_MAX];

/* -------------------------------------------------- xsystem defs --------- */
extern int      select();               /* system subroutine                 */
extern int      pid;                    /* child process id                  */
#if defined(_IBM5A)
int             (*tstat)();             /* previous setting SIGTERM          */
int             (*istat)();             /* previous setting SIGINT           */
int             (*qstat)();             /* previous setting SIGQUIT          */
#else
extern void     (*tstat)(int);          /* previous setting SIGTERM          */
extern void     (*istat)(int);          /* previous setting SIGINT           */
extern void     (*qstat)(int);          /* previous setting SIGQUIT          */
extern void     (*cstat)(int);          /* previous setting SIGCONT          */
#endif
extern int      stdinfd[2];             /* file descriptors for pipe to      */
                                        /* child's stdin                     */
extern int      stdoutfd[2];            /* file descriptors for pipe to      */
                                        /* child's stdout                    */
extern int      stderrfd[2];            /* file descriptors for pipe to      */
                                        /* child's stderr                    */
extern unsigned int    xsystem_status;  /* exit status of child process      */
extern int             sigcld_flag;     /* set TRUE when xsystem_term        */
                                        /* catches SIGCLD signal             */
/*-------------------------------------------------- xsystem defs ---------- */

#ifdef _ORI
extern ORI_HANDLE      ori_handle; /* ORI help database handle        */
extern int      ori_open_flag;  /* TRUE means ORI datbase is open    */
#endif

#ifdef _ORI2
extern ORI_HANDLE      ori_handle; /* ORI help database handle        */
extern int      ori_open_flag;  /* TRUE means ORI datbase is open    */
#endif

/* Routines */

#ifdef _IBM5A
char            *malloc();      /* system call                               */
char            *realloc();     /* system call                               */
#endif
extern int      sys_nerr;
extern char     *sys_errlist[];

extern
ASL_RC  add_param();            /* adds param. & value to cmd. str.          */
extern
ASL_RC  cat_discover_name_value();
                                /* adds name/value to                        */
                                /* cmd_to_discover formatted string          */
extern
ASL_RC  cat_param();            /* cats a stripped param. value to cmd. str. */
extern
ASL_RC  cat_substituted_postfix();
                                /* does name/value substitution for          */
                                /* "*_postfix" fields                        */
#ifdef _ORI 
extern
int     clean_line();           /* deletes <<...>> stuff from help text      */
#endif
#ifdef _ORI2
extern
void    clean_line();           /* deletes <<...>> stuff from help text      */
#endif

extern
char    *cmd_status_line();
                                /* generates command status lines for        */
                                /* command status window                     */
extern
ASL_RC  do_smit_list();         /* sets up data structs. for smit_list() call*/
extern
char    *discover_defaults();   /* gets defaults from cmd_to_discover for    */
                                /* dialogues                                 */
void    err_odm_free_list();    /* reports odm_free_list errors              */
extern
ASL_RC  fill_in_defaults();     /* uses cmd_to_discover output to fill in    */
                                /* dialogue value fields                     */
ASL_RC  fill_in_common_defaults();

extern
ASL_RC  fill_cmd_hdr();         /* fills cmd_hdr structure with values       */
                                /* returned by odmget()                      */
extern
ASL_RC  fill_cmd_opt();         /* fills cmd_opt structure with values       */
                                /* returned by odmget()                      */
extern
ASL_RC  fill_scr_info();        /* fills scr_info structure according to     */
                                /* cmd_opt values                            */
extern
ASL_RC  fill_menu_opt();        /* fills menu_opt structure with values      */
                                /* returned by odmget()                      */
extern
ASL_RC  fill_name_hdr();        /* fills name_hdr structure with values      */
                                /* returned by odmget()                      */
extern
ASL_RC  format_help();          /* formats ORI help text                     */
extern
char    *get_cat_msg();         /* gets a catalog message from the           */
                                /* appropriate location                      */
extern
ASL_RC  get_list_screen();      /* prepare scr_type/scr_info for included    */
                                /* hosts in the collective                   */
#if defined(_IBM5A)
char    *getenv();              /* sys. call; get value of environment var.  */
#endif
extern
char    *lookup_discover_value();
                                /* finds value for a name in                 */
                                /* command_to_discover format str.           */
extern
ASL_RC  malloct_null();         /* converts NULL pointers to empty strings   */
extern
ASL_RC  next_screen();          /* selects next screen routine based on      */
                                /* selected menu item                        */
extern
ASL_RC  reduce_sbrks();         /* periodic memory pre-allocator             */
extern
void    realloc_strcpy();       /* strcpy to enlarged target field           */
extern
void    set_common_signals();   /* specificies common action for signals     */
extern
ASL_RC  smit_command();         /* runs the cmd_to_execute                   */
extern
ASL_RC  smit_dialogue();        /* does the dialogue screen                  */
extern
ASL_RC  smit_standard_dialogue(); /* does the standard dialogue screen       */
extern
void    smit_exit();            /* closes files, terminates ODM/ASL, etc.    */
extern
void    my_exit();              /* removes /tmp/WC... file                   */
extern
void    smit_free();            /* free() with some debug logging            */
#ifdef _ORI
extern
ASL_RC  smit_gen_help();        /* general help on tool                      */
extern
ASL_RC  smit_help();            /* context sensitive help on objects         */
#else
extern void     asl_beep();
#endif
#ifdef _ORI2
extern
ASL_RC  smit_gen_help();        /* general help on tool                      */
extern
ASL_RC  smit_help();            /* context sensitive help on objects         */
#else
extern void     asl_beep();
#endif

extern
void    smit_init();            /* initialization of files, etc.             */
extern
ASL_RC  smit_list();            /* runs cmd_to_list and gets selection       */
extern
char    *smit_malloct();        /* malloc() with error checking              */
extern
ASL_RC  smit_menu();            /* does the menu screen                      */
extern
ASL_RC  smit_menu_fast();       /* does the menu "self alias" fast path case */
extern
ASL_RC  smit_name_select();     /* does the name select screen               */
extern
int     smit_strcmp();        /* for qsort() on sm_cmd_opt and sm_menu_opt */
                                /* ODM object classes                        */
extern
char    *smit_odm_perror();     /* gets an ODM error message                 */
extern
char    *smit_ori_perror();     /* gets an ORI error message                 */
extern
char    *smit_perror();         /* gets a catalogue message for a system     */
                                /* subroutine error                          */
extern
char    *smit_realloct();       /* realloc() with error checking             */
extern
ASL_RC  smit_reduce_sbrks();    /* periodic memory pre-allocator             */
extern
void    smit_signal_error ();   /* normal signal error handler               */
extern
void    interrupt_handler();    /* SIGINT signal handler                     */
extern
ASL_RC  strcat_max();           /* strcat() with error checking              */
extern
ASL_RC  strcpy_max();           /* strcat() with error checking              */
extern
char    *time_stamp();          /* generates  time stamp for log entries  */
extern
ASL_RC  com_dialogue_in_failure();
extern
ASL_RC  update_com_dialogue();
extern
ASL_RC  update_screens();
extern
int     xsystem();              /* like system() with pipes to stdout/stderr */
extern
void    xsystem_term();         /* xsystem() interrupt handler               */

extern
char    *smit_use_new_db();     /* opens a new db defined from smit stanza   */
extern
FILE    *pgleader_popen();      /* like popen, but sets child to pgleader    */
extern
int	ck_upd_image();		/* checks to see if the file is a valid image*/

ASL_RC  get_list_hosts();
ASL_RC  restore_collective();
ASL_RC  select_homogeneous_group();
ASL_RC  parse_cmd_opts();
ASL_RC  fill_cmd_line_opt();
ASL_RC  open_log();

/*---------------------------------------------------------------------------*/
/* array for cmd_opts that are filled from the command line with -p          */
/*---------------------------------------------------------------------------*/

typedef struct {
        char    *host_name;
        char    *values[10];
} CMDOPT_INFO;

extern CMDOPT_INFO G_cmd_line_opt[SM_HOST_MAX];
extern int G_cmd_opt_num;
extern int G_num_opts;
extern int G_run_mode;
extern int G_startup_msgs;
#define SREX           1
#define CREX           2
#define DGRAM_MAX_VALUE_SIZE    8196

/*---------------------------------------------------------------------------*/
/* array element for each host, included in the collective                   */
/*---------------------------------------------------------------------------*/

#ifndef _HOST_INFO
typedef struct {
        char    *host_name;
        char    *value_set;
} HOST_INFO;

int srex_child_flag = 0;

#define _HOST_INFO
#endif
