/* @(#)63       1.32.1.9  src/bos/usr/include/asl.h, libasl, bos411, 9436B411a 9/6/94 15:26:04 */

#ifndef _H_ASL
#define _H_ASL

/*
 * COMPONENT_NAME: (libasl) ASL -- AIX Screen Library
 *
 * FUNCTIONS: include file definitions.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if ! defined(_IBM5A)
#include <stdarg.h>
#endif

extern int      asl_use_back_key;       /* TRUE = use "F10=Back nn" key on   */
                                        /* main screens                      */
                                        /* default setting = FALSE           */
extern int      asl_list_return_help;   /* TRUE = return ASL_HELP on non-    */
                                        /* diag list screens                 */
                                        /* default setting = FALSE           */
extern int      from_list_help;

#define ASL_MIN_LINES   24
#define ASL_MIN_COLS    80

/*---------------------------------------------------------------------------*/
/*      ASL_RC ASL function-key/return codes                                 */
/*---------------------------------------------------------------------------*/

typedef enum {
                                /* -------- normal return codes -----        */
    ASL_CANCEL          = 0,    /* ESC maps here ("pseudo pfkey")            */
    ASL_HELP            = 1,    
    ASL_REDRAW          = 2,    /* processed internally, not returned        */
    ASL_EXIT            = 3,
    ASL_LIST            = 4,
    ASL_DEFAULT         = 5,    /* processed internally, not returned        */
    ASL_COMMAND         = 6,
    ASL_COMMIT          = 7,    /* actually select in most cases             */
    ASL_PRINT           = 8,    /* processed internally, not returned        */
    ASL_SHELL           = 9,    /* processed internally, not returned        */
    ASL_EDIT            = 10,   /* processed internally, not returned        */
                                /* we map ESC+0 to same code as PF10         */
    ASL_BACK            = 11,   /* returned if asl_use_back_key = TRUE       */
    ASL_ENTER           = 12,   /* ENTER maps here ("pseudo pfkey")          */
    ASL_ENTER_DO        = 13,   /* pseudo key, not returned                  */
    ASL_REAL_COMMIT     = 14,   /* pseudo key, not returned                  */
                                /* otherwise ASL_CANCEL is returned          */
    ASL_FIND            = 15,
    ASL_FIND_NEXT       = 16,
                                /* ------ DSMIT specific keys   --------     */
    ASL_MACHINE         = 17,
    ASL_FIELD           = 18,
    ASL_COLLECTIVE      = 19,
    ASL_SCHEDULER       = 20,   /* added in dsmit-scheduler                  */

    ASL_OK              = 1000, /* "OK"; no error or no other special action */

                                /* -------- error return codes ---------     */
    ASL_FAIL                    = (-1),         /* unspecified err. or FALSE */

    ASL_ERR_NO_SUCH_TERM        = (-1000),      /* no entry for TERM         */
    ASL_ERR_TERMINFO_GET        = (-1001),      /* couldn't access terminfo  */
    ASL_ERR_NO_TERM             = (-1002),      /* no tty on stdout          */
    ASL_ERR_INITSCR             = (-1004),      /* libcur initscr() failed   */
    ASL_ERR_SCREEN_SIZE         = (-1005),      /* need min. 24 X 80 screen  */
    ASL_ERR_TERM_TYPE           = (-1006)       /* need vi class terminal    */
} ASL_RC;

/* #define ASL_NUM_PFKEYS 16 */
#define ASL_NUM_PFKEYS 23      /* ---- DSMIT imposed                        */ 


/*---------------------------------------------------------------------------*/
/*      ASL_NOTE parameter constants                                         */
/*---------------------------------------------------------------------------*/

typedef enum {                  /* ---- asl_note() status values             */
    ASL_MSG             = 0,    /* popup message, waits for user ack.        */
    ASL_ERR_MSG         = 1,    /* popup error message, waits for user ack.  */
    ASL_NO_MSG          = 2,    /* no popup                                  */
    ASL_INFORM_MSG      = 3,    /* transient popup informative message       */
    ASL_CANCEL_MSG      = 4,    /* invoke cancel popup                       */
    ASL_EXIT_MSG        = 5,    /* invoke exit popup                         */
    ASL_COMMIT_MSG      = 6,    /* invoke ask popup                          */
    ASL_SHELL_MSG       = 7,    /* invoke shell popup                        */
    ASL_PRINT_MSG       = 8,    /* invoke print popup                        */
    ASL_COMMAND_MSG     = 9,    /* invoke ask popup                          */
    ASL_HELP_MSG        =10,    /* invoke help popup                         */
    ASL_MSG_CONTINUE    =11,    /* popup message, waits for user ack.        */
    ASL_SYS_ERR_MSG     =12,    /* popup error message, waits for user ack.  */
                                /* user refered to log (if it exists) for    */
                                /* full error message.                       */
                                /* ------ DSMIT specific messages            */
    ASL_DSMIT_INTERRUPT_MSG= 13, /* invoke interrupt popup                   */
    ASL_DSMIT_WHEEL= 14         /* spin the wheel			     */

} ASL_STATUS_CODE;

typedef enum {                  /* ---- asl_note() record values             */
    ASL_LOG             = 0,    /* make a log file entry                     */
    ASL_NO_LOG          = 1,    /* no logging of any kind                    */
    ASL_VERBOSE         = 2,    /* log if asl_init() verbose option selected */
    ASL_TRACE           = 3,    /* log if asl_init() trace option selected   */
    ASL_DEBUG           = 4     /* log if asl_init() debug option selected   */
} ASL_RECORD_CODE;

/*---------------------------------------------------------------------------*/
/*      ASL_SCREEN screen_codes (menu/popup types)                           */
/*---------------------------------------------------------------------------*/

typedef enum {
    ASL_SINGLE_MENU_SC  = 0,    /* single selection full screen panel        */
    ASL_DIALOGUE_SC     = 1,    /* full screen dialogue panel                */
    ASL_SINGLE_LIST_SC  = 2,    /* single selection popup                    */
    ASL_CONTEXT_HELP_SC = 3,    /* contextual help popup                     */
    ASL_OUTPUT_SC       = 4,    /* output full screen panel                  */
    ASL_ACK_MSG_SC      = 5,    /* user acknowleged message popup            */
    ASL_EDIT_SC         = 6,    /* long value field edit popup               */
    ASL_MULTI_LIST_SC   = 7,    /* multiple selection popup                  */
    ASL_COMMAND_SC      = 8,    /* show command popup                        */
    ASL_PRINT_LIST_SC   = 9,    /* print screen popup                        */
    ASL_INFORM_MSG_SC   = 10,   /* information only message popup            */
    ASL_OUTPUT_LEAVE_SC = 11,   /* output full screen panel and leave (it up)*/
    ASL_EXIT_SC         = 12,   /* exit system popup                         */
    ASL_CANCEL_SC       = 13,   /* cancel dialogue popup                     */
                                /* output full screen panel and leave (it up)*/
    ASL_DIAG_OUTPUT_LEAVE_SC    = 14,
                                /* output screen w/o showing active pf keys  */
    ASL_DIAG_NO_KEYS_ENTER_SC   = 15,   
    ASL_DIAG_ENTER_SC    = 16,  /* output a non-selection full screen panel  */
    ASL_DIAG_DIALOGUE_SC = 17,  /* dialogue panel for diagnostics            */
                                /* help popup for diagnostics dialogue       */
    ASL_DIAG_DIALOGUE_HELP_SC   = 18,   
                                /* list popup for dialogue list type items   */
    ASL_DIAG_DIALOGUE_LIST_SC   = 19,   
                                /* selection panel for diagnostics           */
    ASL_DIAG_LIST_CANCEL_EXIT_SC = 20,
                                /* multi selection panel for diagnostics     */
    ASL_DIAG_LIST_COMMIT_SC     = 21,  
                                /* accept cancel, exit, or enter full screen */
    ASL_DIAG_KEYS_ENTER_SC      = 22,   
    ASL_GENERAL_HELP_SC = 23,   /* general help panel                        */
    ASL_COLOR_LIST_SC   = 24,   /* single selection popup for HFT attributes */
                                /* output full screen panel and leave (it up)*/
    ASL_OUTPUT_LEAVE_NO_SCROLL_SC = 25,
                                /* output dialogue panel & leave (it up)     */
    ASL_DIALOGUE_LEAVE_SC       = 26,
                                /* output full screen panel and leave (it up)*/
    ASL_DIAG_LEAVE_NO_KEYS_SC   = 27,
                                /* next 3 SC's add HELP key to the equivalent*/
                                /* ASL_DIAG SC's                             */
    ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC = 28,
    ASL_DIAG_ENTER_HELP_SC      = 29,
    ASL_DIAG_LIST_COMMIT_HELP_SC = 30,
                                /* ------- DSMIT specific screen types ------*/
    ASL_DSMIT_DIALOGUE_SC       = 31,
    ASL_DSMIT_DIALOGUE_FIELD_SC = 32,
    ASL_DSMIT_SINGLE_MENU_SC    = 33,
    ASL_DSMIT_INFORM_SC         = 34, /* user acknowledged popup              */
    ASL_DSMIT_OUTPUT_SC         = 35,
    ASL_SEARCH_SC		= 36,
    ASL_DSMIT_WHEEL_SC		= 37
} ASL_SCREEN_CODE;

#define ASL_IS_DIAG_SC_CODE(it) (                       \
       (it) == ASL_DIAG_OUTPUT_LEAVE_SC                 \
    || (it) == ASL_DIAG_NO_KEYS_ENTER_SC                \
    || (it) == ASL_DIAG_ENTER_SC                        \
    || (it) == ASL_DIAG_DIALOGUE_SC                     \
    || (it) == ASL_DIAG_DIALOGUE_HELP_SC                \
    || (it) == ASL_DIAG_DIALOGUE_LIST_SC                \
    || (it) == ASL_DIAG_LIST_CANCEL_EXIT_SC             \
    || (it) == ASL_DIAG_LIST_COMMIT_SC                  \
    || (it) == ASL_DIAG_KEYS_ENTER_SC                   \
    || (it) == ASL_DIAG_LEAVE_NO_KEYS_SC                \
    || (it) == ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC        \
    || (it) == ASL_DIAG_ENTER_HELP_SC                   \
    || (it) == ASL_DIAG_LIST_COMMIT_HELP_SC             \
   )

/*---------------------------------------------------------------------------*/
/*  Public ASL routines                                                      */
/*---------------------------------------------------------------------------*/

extern  void    asl_beep();             /* rings the bell                    */
extern  ASL_RC  asl_check_file_entries(); /* -- DSMIT use only               */
extern  ASL_RC  asl_clear_screen();     /* clear screen                      */
extern  ASL_RC  asl_execute();          /* ---- Diagnostics use only         */
extern  ASL_RC  asl_get_list();         /* convert string to list structure  */
extern  ASL_RC  asl_init();             /* Initial setup of curses & signals */
extern  ASL_RC  asl_quit();             /* Closeout curses                   */
extern  ASL_RC  asl_read();             /* ---- Diagnostics use only         */
extern  void    asl_redraw();           /* refresh screen using curscr       */
extern  void    asl_reset();            /* ---- SMIT use only                */
extern  void    asl_restore();          /* ---- SMIT use only                */
extern  ASL_RC  asl_restore_tty();      /* ---- SMIT use only                */
extern  ASL_RC  asl_restore_tty_colors(); /* -- SMIT use only                */
extern  ASL_RC  asl_ring();             /* copies subfield from ring         */
extern  ASL_RC  asl_save_tty();         /* ---- SMIT use only                */
extern  ASL_RC  asl_screen();           /* screen displays                   */
extern  void    asl_set_changed();      /* ---- SMIT use only                */

#if defined(_IBM5A)
extern  ASL_RC  asl_note();             /* send messages to popups/log file  */
extern  ASL_RC  asl_vnote();            /* ---- Diagnostics use only         */
#else
extern  ASL_RC asl_note(
    ASL_STATUS_CODE     status,         /* error/message status              */
    ASL_RECORD_CODE     record,         /* where to record note              */
    char                *format,        /* NLprintf() style format           */
    ...
);
extern  ASL_RC asl_vnote (
    ASL_STATUS_CODE     status,         /* error/message status              */
    ASL_RECORD_CODE     record,         /* where to record note              */
    char                *format,        /* NLprintf() style format           */
    va_list             arg             /* vsprint() style argument list     */
);
#endif

/*--------------------------------------------------------------------------*/
#define ASL_DIAG_LINES  24
#define ASL_DIAG_COLS   80
/*--------------------------------------------------------------------------*/

                                    /* asl parameter binary switch values */
#define ASL_YES ((char) ('y'))
#define ASL_NO  ((char) ('n'))

#define ASL_READ_BUF_SIZE   100
#define ASL_MSG_MAX_STRLEN  32000
#define ASL_MAX_VALUE_SIZE  32760 
/*#define ASL_MAX_INPUT_BUF   2048*/
				/* Defect 150763. Increased the maximum size */
				/* of the input value.  This may cause problems*/
				/* because it may make the command too long. */
#define ASL_MAX_INPUT_BUF   (ASL_MAX_VALUE_SIZE / 2)

/*---------------------------------------------------------------------------*/
/* These are the (header) "type" parameters for asl displays                 */
/*---------------------------------------------------------------------------*/

typedef struct {
                                /* ---- generally "universal" parameters --- */
                                /* ASL_SINGLE_MENU_SC, ASL_DIALOGUE_SC,      */
                                /* ASL_SINGLE_LIST_SC, ASL_OUTPUT_SC         */
    ASL_SCREEN_CODE     screen_code;
    long    max_index;  /* 0 orig. max index for array of asl_scr_info items */
                /* current index for item that cursor "points" to; init. as 1*/
    long    cur_index;  
                /* current offset for scroll window position; init. as 0     */
    long    cur_win_offset;     
                /* current index for scroll window position; init. as 0      */
    long    cur_win_index;      

                            /* ---- generally "menu" related parameters ---- */

                /* ASL_YES, ASL_NO = default; allow mult. selects for list   */
    char    multi_select;       

                        /* ---- generally "dialogue" related parameters ---- */

    long    text_size;  /* size of display field to use for discriptive text,*/
                        /* set to 0 for default                              */
    char    ask;        /* ask user if they really want to execute command   */

} ASL_SCR_TYPE;

#define ASL_DIALOGUE_TEXT_SIZE          50
#define ASL_DEFAULT_TEXT_SIZE           50
#define ASL_DEFAULT_LIST_TEXT_SIZE      70
#define ASL_DIAG_DIALOGUE_TEXT_SIZE     30

/*---------------------------------------------------------------------------*/
/* array element for each display item, first entry is for title,            */
/* last entry is for message line                                            */ 
/*---------------------------------------------------------------------------*/

typedef struct {
                        /* ---- generally "universal" parameters ----        */

                    /* [keep the text field the first field in this struct.] */
        char    *text;          /* the text line(s) to be displayed          */
        long    line_num;       /* asl's internal screen line number         */

                        /* ---- generally "menu" related parameters ----     */

                        /* ASL_YES, ASL_NO; for info., can't be selected     */
        char    non_select;     
        char    item_flag;      /* leading flag char */
                /* ASL_YES/ASL_NO = default; item has been selected by user  */
        char    multi_select_flag;

                        /* ---- generally "dialogue" related parameters ---- */

        char    op_type;        /* type of operations allowed on this field  */
        char    entry_type;     /* type of (user) entry allowed in the field */
                /* ASL_YES, ASL_YES_NON_EMPTY, ASL_EXCEPT_WHEN_EMPTY         */
                /* ASL_NO = default;                                         */
                /* ASL_YES or ASL_YES_NON_EMPTY means display required flag  */
        char    required;       
                /* ASL_YES, ASL_NO = default; field changed from def. value  */
        char    changed;        
                /* disp. text of allowed/default choice(s, seperated by ",") */
        char    *disp_values;   
                /* MUST point to string (buffer) of size (entry_size + 1) if */
                /* there is ANY way values may be changed (typein/list/ring) */
                /* Also used to hold ASL_DIAG_..._SC title-line screen num.  */ 
        char    *data_value;    
                /* maximum size of (entered_)value that can be entered OR    */
                /* returned (incl. a "return" of anything from disp_values)  */
        long    entry_size;     
                /* ASL_YES, ASL_NO = default; multiselect allowed for list   */
        char    multi_select;           
                /* current  index of def. value, -1 if entered_value active  */
        long    cur_value_index;        
        long    default_value_index;    /* 0 origin index of default value   */

} ASL_SCR_INFO;
                                /* asl_cmd_info.required (+ ASL_YES, ASL_NO) */
#define ASL_YES_NON_EMPTY       ((char) ('+'))
#define ASL_EXCEPT_WHEN_EMPTY   ((char) ('?'))
                                /* asl_scr_info.disp_values                  */
#define ASL_RING_SEP        ((char) (','))
                                /* asl_scr_info.op_type (field operations)   */
#define ASL_NOOP_ENTRY      ((char) ('n'))
#define ASL_RING_ENTRY      ((char) ('r'))
#define ASL_LIST_ENTRY      ((char) ('l'))

                                /* asl_scr_info.entry_type (entry types)     */
#define ASL_TEXT_ENTRY        ((char) ('t'))
#define ASL_RAW_TEXT_ENTRY    ((char) ('r'))
#define ASL_NUM_ENTRY         ((char) ('#'))
#define ASL_SIGNED_NUM_ENTRY  ((char) ('-'))
#define ASL_HEX_ENTRY         ((char) ('x'))
#define ASL_FILE_ENTRY        ((char) ('f'))
#define ASL_NO_ENTRY          ((char) ('n'))
#define ASL_INVISIBLE_ENTRY   ((char) ('i'))

#ifdef DEBUG 
#define ASL_TRACER(msg) \
asl_note(ASL_NO_MSG, ASL_DEBUG, "## %14.14s:%4.4d %s", __FILE__, __LINE__, msg) 
#else
#define ASL_TRACER(msg) 
#endif

#endif /* _H_ASL */
