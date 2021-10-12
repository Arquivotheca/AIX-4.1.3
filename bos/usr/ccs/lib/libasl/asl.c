static char sccsid[] = "@(#)10  1.105.2.36  src/bos/usr/ccs/lib/libasl/asl.c, libasl, bos41J, 9520A_all 5/12/95 13:06:06";

/*
 * COMPONENT_NAME: (libasl) ASL -- AIX Screen Library
 *
 * FUNCTIONS:
 *      asl_beep,
 *      asl_center_first_title_line,
 *      asl_change_field,
 *      asl_check_file_entries,
 *      asl_clear_screen,
 *      asl_clear_buffer_screen,
 *      asl_copy_view,
 *      asl_copy_view_line,
 *      asl_curses_rc,
 *      asl_cursor,
 *      asl_display_initial_value,
 *      asl_display_value,
 *      asl_edit,
 *      asl_execute,
 *      asl_flush_input,
 *      asl_free,
 *      asl_function_key,
 *      asl_get_list,
 *      asl_getch,
 *      asl_hilite_off,
 *      asl_hilite_on,
 *      asl_help,
 *      asl_init,
 *      asl_init_screen,
 *      asl_input_field_size,
 *      asl_list_wprintw,
 *      asl_locator,
 *      asl_make_action_keys,
 *      asl_malloct,
 *      asl_max_line,
 *      asl_msg,
 *      asl_note,
 *      asl_num_lines,
 *      asl_print_screen,
 *      asl_quit,
 *      asl_read,
 *      asl_realloct,
 *      asl_reduce_sbrks,
 *      asl_restore,
 *      asl_restore_tty,
 *      asl_restore_tty_colors,
 *      asl_ring,
 *      asl_ring_list,
 *      asl_save_tty,
 *      asl_screen,
 *      asl_scroll_screen,
 *      asl_setup,
 *      asl_set_changed,
 *      asl_shell,
 *      asl_signal,
 *      asl_std_edit_keys,
 *      asl_strcat_max,
 *      asl_strcpy_max,
 *      asl_superbox,
 *      asl_term_screen,
 *      asl_up_down_screen,
 *      asl_waddstr,
 *      asl_wprintw,
 *      asl_vnote.
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



#define	_ILS_MACROS
#include <errno.h>
#include <mbstr.h>

#include <cur00.h>
#include <cur02.h>

#define ASL_PROG_SET_NUM 1
#include "asl_cat.h"

#include <sys/limits.h>
#include <nl_types.h>
static nl_catd asl_catd;

#include "asl_msg.h"
#define MSGSTR(msg_num, string) \
                catgets(asl_catd, ASL_PROG_SET_NUM, msg_num, string)


#include "asl.h"
#include <math.h>
#include <memory.h>

#include <stdarg.h>

#include <string.h>
#include <sys/types.h>
#include <sys/signal.h>

#ifndef _ASL41
#include <sys/hft.h>
#endif

#include <unistd.h>
#include <ctype.h>

#undef index
int     asl_use_back_key     = FALSE;   /* TRUE = use "F10=Back nn" key on   */
                                        /* main screens, with nn given by    */
                                        /* asl_scr_info[0].cur_value_index   */
                                        /* else use nn = 1, ret = ASL_CANCEL */
int     asl_list_return_help = FALSE;   /* TRUE = return ASL_HELP on non-    */
                                        /* diag list screens                 */
int     from_list_search = 0;
int     from_list_help = 0;

static char selection_array[5000];

extern char* search_string();

#define KEY_SLASH	   0x2f
#define ASL_MAX_LEAVE_BYTES 10000

#ifdef _DSMIT
#ifndef _NO_NETLS
extern void heartbeat(void);    /* checks in with srvr before chk_per expires */
#endif
#endif


#if ! defined(_IBM5A)
extern char do_cursor;
#endif
#define EMPTY_STRING    ""
typedef struct {
    ASL_SCREEN_CODE     screen_code;    /* type of screen to be displayed    */

    WINDOW  *save_win;          /* window to save previous curscr state      */
    int     y_save;             /* previous cursor line                      */
    int     x_save;             /* previous cursor column                    */
    int     save_cursor_state;  /* previous cursor state                     */
    WINDOW  *main_win;          /* window for non-scrolled stuff             */
    WINDOW  *frame_win;         /* window for border/frame around popups     */
    WINDOW  *scroll_data;       /* nondisplay win.; holds disp. scroll dat   */
    WINDOW  *text_subwin;       /* subwindow of scroll_data for name info.   */

    int    popup;               /* TRUE means this screen is a popup         */
    int    no_scroll;           /* TRUE means no scrol (indicators) needed   */
    int    x_pop_offset;        /* x offset from origin of stdscr            */
    int    y_pop_offset;        /* y offset from origin of stdscr            */
    int    main_win_lines;      /* number of lines in main window            */
    int    main_win_cols;       /* number of columns in main window          */

    int    x_text_size;         /* size of text field display region         */
    int    x_value_size;        /* size of value field display region        */
    int    x_cur_col;           /* curr. absolute location into value string */
    int    x_cur_offset;        /* curr. offset of display region from start */
                                /* of value                                  */
    int    x_character_offset;  /* character pointed to by x_cur_offset      */
    int    x_cur_character;     /* current character into value string       */
    int    y_scroll_view_size;  /* number of lines in scroll area            */
    int    y_scroll_data_size;  /* number of lines in scroll_data            */
    int    y_view_cur;          /* location within scroll view window        */
    int    y_view_offset;       /* offset of scroll area origin from         */
                                /* scroll_data origin                        */
    int    x_view_cur;          /* location within scroll view window        */
    int    x_scroll_data_size;  /* number of display columns in scroll view  */
				/* used currenly on lists		     */

                                /* ---- screen column positions ----         */
    int    x_status_flag;       /* required parameter indicator              */
    int    x_text;              /* text field starts here                    */
    int    x_value_l_flag;      /* where left (value) bracket goes           */
    int    x_value;             /* value field starts here                   */
    int    x_value_r_flag;      /* where right (value) bracket goes          */
    int    x_list_flag;         /* where list/ring flag goes                 */
    int    x_type_flag;         /* where entry field type indicator goes     */
                                /* ---- screen row positions ----            */
    int    y_title;             /* first title line                          */
    int    y_title_last;        /* last title line                           */
    int    y_instruct;          /* first instruction line                    */
    int    y_instruct_last;     /* last instruction line                     */
    int    y_more_upper;        /* [more ...] scroll indicator               */
    int    y_scroll;            /* first scrolling region line               */
    int    y_scroll_last;       /* last scrolling region line                */
    int    y_more_lower;        /* [more ...] scroll indicator               */
    int    y_pfkeys;            /* first pfkey line                          */
    int    y_pfkeys_last;       /* last pfkey line                           */

    int    button_down;         /* indicates current mouse button state      */
    int    left_button_down;    /* indicates current mouse button state      */
    int    right_button_down;   /* indicates current mouse button state      */
    int    hw_cur_x;            /* hardware x cursor pos. on stdscr win.     */
    int    hw_cur_y;            /* hardware y cursor pos. on stdscr win.     */

    int    type;                /* type of current input character           */
    wchar_t  c;                 /* current input character                   */
} ASL_SCREEN;

                        /* -------- ASL functions -------------------------- */
        void    asl_beep();     /* rings the bell                            */
static  void    asl_center_first_title_line();
                        /* extracts first line from string and centers it    */
static  ASL_RC  asl_change_field();
                        /* update the content of a dialogue entry field      */
  	ASL_RC  asl_check_file_entries();
                        /* check validity of entry fields of type file       */
        ASL_RC  asl_clear_screen();
                        /* clear screen                                      */
        ASL_RC  asl_clear_buffer_screen();
                        /* clear screen buffer and refresh if specified      */
static  void    asl_copy_view();
                        /* copy scroll region from buf. win. to display win. */
static  void    asl_copy_view_line();
                        /* copy scroll line from buf. win. to display win.   */
static  void    asl_curses_rc();/*set cursor type                            */
#define ASL_CURSES_RC(return_code) asl_curses_rc(return_code, __LINE__)
static  void    asl_cursor();   /* set cursor type                           */
static  ASL_RC  asl_display_initial_value();
                        /* displays the initial value of a new entry field   */
static  void    asl_display_value();
                        /* update a dialogue entry field in display window   */
static  ASL_RC  asl_edit();     /* generate edit popup                       */
        ASL_RC  asl_execute();  /* execute another program using execv       */
static  void    asl_flush_input();
                        /* throw away pending input                          */
static  void    asl_free();     /* free() with some debug logging            */
#define         ASL_FREE(addr) asl_free(addr, __LINE__)
static  ASL_RC  asl_function_key();
                        /* see if input char. is a currently valid funt. key */
        ASL_RC  asl_get_list(); /* generate and get result from list popup   */
static  wchar_t    asl_getch();    /* like getch()                           */
static  void    asl_hilite_off();
                        /* turn off hilighting on a specified line segment   */
static  void    asl_hilite_on();
                        /* turn on hilighting on a specified line segment    */
static  ASL_RC  asl_help();     /* generate response to help request         */
        ASL_RC  asl_init();     /* initial setup of curses                   */
static  ASL_RC  asl_init_screen();
                        /* create and initialize ASL screen object           */
static  int     asl_input_field_size();
                        /* give size of input field                          */
static  int     asl_list_waddstr();
                        /* handles locator stuff, calls WADDSTR()            */
static  ASL_RC  asl_locator();
                        /* translate mouse motion to a cursor or special key */
static  int     asl_make_action_keys();
                        /* put up a the pfkey part of display                */
static  char    *asl_malloct(); /* malloc() with error checking              */
#define         ASL_MALLOCT(addr) asl_malloct(addr, __LINE__)
static  int     asl_max_line();
                        /* determine maximun line length in multiline string */
static  ASL_RC  asl_msg();      /* display a message popup on the screen     */
                        /* returns num. of cols. needed to disp. a wchar_t   */
/*      ASL_RC  asl_note();        send messages to popups/log file          */
static  int     asl_num_lines();
                        /* counts lines in a string                          */
static  ASL_RC  asl_print_screen();
                        /* copies the current screen image to log file       */
        ASL_RC  asl_quit();     /* closeout curses                           */
        ASL_RC  asl_read();
                        /* special read for ASL_DIAG_OUTPUT_LEAVE_SC screen  */
static  char    *asl_realloct();
                        /* realloc() with error checking                     */
        void    asl_redraw();
static  void    asl_reduce_sbrks();
                        /* periodic memory pre-allocator                     */
        void    asl_reset();
                        /* like asl_init, but assumes curses still active    */
        void    asl_restore();  /* restores "normal" tty I/O modes           */
        ASL_RC  asl_restore_tty();
                        /* ---- SMIT use only                                */
        ASL_RC  asl_restore_tty_colors();
                        /* ---- SMIT use only                                */
        ASL_RC  asl_ring();
                        /* uses specified ring index to get field from ring  */
static  ASL_RC  asl_ring_list();
                        /* uses a ring field to generate a list popup        */
        ASL_RC  asl_save_tty(); /* ---- SMIT use only                        */
        ASL_RC  asl_screen();   /* screen displays                           */
static  void    asl_scroll_screen();
void    asl_set_changed();      /* ---- SMIT use only                        */
                        /* simulates a viewport scroll operation             */
static  void    asl_setup();    /* does key table initialization             */
static  ASL_RC  asl_shell();    /* does shell escape                         */
static  void    asl_signal();   /* catches job control signals               */
static  ASL_RC  asl_std_edit_keys();
                        /* edits a field based on current character          */
static  ASL_RC  asl_strcat_max();
                        /* strcat() with error checking                      */
static  ASL_RC  asl_strcpy_max();
                        /* strcat() with error checking                      */
static  void    asl_term_screen();
                        /* restores terminal/keyboard characteristics        */
static  ASL_RC  asl_up_down_screen();
                        /* handles vertical cursor motion                    */
static  int     asl_waddstr();
                        /* calls WADDSTR(), but prevents 1st auto-line-wrap  */
static  ASL_RC  asl_wprintw();
                        /* LS'd wprintw that checks for buffer overflow     */
/*      ASL_RC  asl_vnote();                                                 */
                        /* send messages to popups/log file, vargs interface */
int     mbs_width();    /* added for defect 141879                           */



#define ENDWIN()                endwin()
#define CBOX(win)               cbox(win)
#define DELWIN(win)             delwin(win)
#define NEWWIN(aa,bb,cc,dd)     newwin(aa,bb,cc,dd)
#define SUBWIN(win,aa,bb,cc,dd) subwin(win,aa,bb,cc,dd)
#define WMOVE(win,yy,xx)        wmove(win,yy,xx)
#define WCHGAT(win,ww,mm)       wchgat(win,ww,mm)
#define WADDSTR(win,str)        waddstr(win,str)
#define WADDCH(win,ch)          waddch(win,ch)
#define WCLEAR(win)             wclear(win)
#define CLEAROK(win,bb)         clearok(win,bb)
#define WCOLOROUT(win,bb)       wcolorout(win,bb)
#define WCOLOREND(win)          wcolorend(win)
#define OVERWRITE(win,win_2)    overwrite(win,win_2)
#define WREFRESH(win)           wrefresh(win)
#define TOUCHWIN(win)           touchwin(win)
#define RESETTY(bool)           resetty(bool)
#define CRESETTY(bool)          cresetty(bool)
#define CSAVETTY(bool)          csavetty(bool)
#define RESTORE_COLORS()        restore_colors()
                        /* ---- for asl_init() ----                          */
static  ASL_RC  (*asl_gen_help)();      /* routine for general help on tool  */
static  FILE    *asl_log_file = NULL;   /* ASL's log file                    */
static  int     asl_trace_log;          /* TRUE if trace info. to be logged  */
static  int     asl_verbose_log;        /* TRUE if verbose logging desired   */
static  int     asl_debug_log;          /* TRUE if logging of debugging      */
                                        /* information desired               */
static  int     asl_filter_flag = FALSE;/* TRUE if AIX filter mode operation */
                                        /* desired                           */
static  int     asl_init_done = FALSE;  /* TRUE between asl_init() and       */
                                        /* asl_quit()                        */
static  int     asl_endwin_done = FALSE;/* TRUE if asl_init() did endwin()   */
                                        /* due to some error                 */
static  int     asl_normal_mode = TRUE; /* TRUE if normal attributes are set */
static  int     asl_cursor_on = TRUE;
                        /* TRUE if cursor turned on (should start in this    */
                        /* state since will initially attempt to turn off,   */
                        /* but only if present state != target state.        */
static  WINDOW  *asl_MACRO_win = NULL;
static  int     asl_MACRO_ret  = 0;

#define ASL_WPRINTW_BUF_SIZE 2048

# define max(a,b)       ((a) < (b) ? (b) : (a))
# define min(a,b)       ((a) > (b) ? (b) : (a))
#ifndef abs
# define abs(x)         ((x) >= 0  ? (x) : -(x))
#endif


/*------------------------------------------*/
/* Escape key character combinations        */
/*------------------------------------------*/

#define ESC      0x01B       /*   Esc       */
#define ESC0     0x200       /*   Esc + 0   */
#define ESC1     0x201       /*   Esc + 1   */
#define ESC2     0x202       /*   Esc + 2   */
#define ESC3     0x203       /*   Esc + 3   */
#define ESC4     0x204       /*   Esc + 4   */
#define ESC5     0x205       /*   Esc + 5   */
#define ESC6     0x206       /*   Esc + 6   */
#define ESC7     0x207       /*   Esc + 7   */
#define ESC8     0x208       /*   Esc + 8   */
#define ESC9     0x209       /*   Esc + 9   */
#define ESC_V    0x246
#define ESC_LT   0x20C
#define ESC_GT   0x20E
#define ESC_G    0x237

/* --- DSMIT specific ----------------------*/

#define ESC_M    0x23d
#define ESC_F    0x236
#define ESC_C    0x233
#define ESC_R    0x242
#define ESC_S    0x243

/*-----------------------------------------------------*/
/* US 101-key Translate table; EMACS / vi edit keys    */
/*-----------------------------------------------------*/

#define CTRL_A    0x01     /* beginning of field       */
#define CTRL_B    0x02     /* previous character       */
#define CTRL_C    0x03     /*                          */
#define CTRL_D    0x04     /* reserved                 */
#define CTRL_E    0x05     /* end of field             */
#define CTRL_F    0x06     /* next character           */
#define CTRL_G    0x07     /*                          */
#define CTRL_J    0x0A     /* Enter                    */
#define CTRL_K    0x0B     /* kill field               */
#define CTRL_L    0x0C     /* last line                */
#define CTRL_M    0x0D     /*                          */
#define CTRL_N    0x0E     /* next line                */
#define CTRL_O    0x0F     /* open line                */
#define CTRL_P    0x10     /* previous line            */
#define CTRL_R    0x12     /* insert (replace)   (xxx) */
#define CTRL_T    0x14     /*                          */
#define CTRL_U    0x15     /*                          */
#define CTRL_V    0x16     /* page down                */
#define CTRL_X    0x18     /* delete char         (vi) */
#define CTRL_Y    0x19     /*                          */
#define CTRL_CARET 0x1e    /* page up                  */
#define INSERT    0x10B    /* insert key pressed       */

/*---------------------------------------------------------------------------*/
/*      ASL function key list bit flags                                      */
/*---------------------------------------------------------------------------*/

#define ASL_HELP_KEY    (1 << ASL_HELP)
#define ASL_CANCEL_KEY  (1 << ASL_CANCEL)
#define ASL_REDRAW_KEY  (1 << ASL_REDRAW)
#define ASL_EXIT_KEY    (1 << ASL_EXIT)
#define ASL_LIST_KEY    (1 << ASL_LIST)
#define ASL_DEFAULT_KEY (1 << ASL_DEFAULT)
#define ASL_COMMAND_KEY (1 << ASL_COMMAND)
                                    /* F7 now either select or edit          */
#define ASL_SELECT_KEY  (1 << ASL_COMMIT)
#define ASL_EDIT_KEY    (1 << ASL_COMMIT)
#define ASL_PRINT_KEY   (1 << ASL_PRINT)
#define ASL_SHELL_KEY   (1 << ASL_SHELL)
#define ASL_BACK_KEY    (1 << ASL_BACK)
#define ASL_ENTER_KEY   (1 << ASL_ENTER)
#define ASL_FIND_KEY    (1 << ASL_FIND)
#define ASL_FIND_NEXT_KEY   (1 << ASL_FIND_NEXT)
                                        /* ---- DSMIT specific ------   */
#define ASL_MACHINE_KEY         (1 << ASL_MACHINE)
#define ASL_FIELD_KEY           (1 << ASL_FIELD)
#define ASL_COLLECTIVE_KEY      (1 << ASL_COLLECTIVE)
#define ASL_SCHEDULER_KEY       (1 << ASL_SCHEDULER)/*added in dsmit-scheduler*/
                                        /* -------------------------    */


#define ASL_KEY_ENTER   (0xA)
#define ASL_KEY_NO_OP   (0)
#define ASL_ABS(x)      ((x) < 0 ? -(x) : (x))
#define SCR_MAX \
                scr_type->max_index     /* maximum valid scr_info index      */
#define SCR_CUR \
                scr_type->cur_index     /* curr. (cursored) scr_info index   */

        /* allows up to 256 columns of 2-byte characters plus '\0'           */
#define ASL_WINDOW_LINE_MAX 513
        /* can't grow because "ESC+digit" PF key convention allows only 0--9 */
        /* (but has extension to hold ENTER=DO)                              */

typedef struct {                /* maps function keys into key labels and    */
                                /* corresponding asl return codes            */
    char        *label;         /* label text for selection "keys"           */
    long        return_code;    /* function key return code                  */
} ASL_KEY_MAP [ASL_NUM_PFKEYS];

ASL_KEY_MAP asl_key_map;        /* main key map                              */
#define ASL_PFKEYS_PER_LINE 4   /* num. of pfkeys is for full screen         */

#define ASL_HELP_SC_KEYS         ASL_CANCEL_KEY | ASL_REDRAW_KEY   \
                                | ASL_PRINT_KEY | ASL_ENTER_KEY

#define ASL_COMMON_KEYS         ASL_CANCEL_KEY  | ASL_REDRAW_KEY   \
                                | ASL_PRINT_KEY | ASL_ENTER_KEY    \
                                | ASL_BACK_KEY

#define ASL_OUTPUT_KEYS         ASL_CANCEL_KEY  | ASL_REDRAW_KEY   \
                                | ASL_PRINT_KEY | ASL_HELP_KEY     \
                                | ASL_BACK_KEY  | ASL_SHELL_KEY    \
                                | ASL_COMMAND_KEY

#define ASL_DIAG_KEYS           ASL_CANCEL_KEY  | ASL_EXIT_KEY
        
                                /* ---- DSMIT specific keys --- */                      
#define ASL_DSMIT_KEYS          ASL_MACHINE_KEY | ASL_FIELD_KEY    \
                                | ASL_COLLECTIVE_KEY | ASL_SCHEDULER_KEY

typedef int ASL_KEY_LIST [];    /* gives map of enabled keys for each screen */

/* ------------------------------------------------------------------------- */
/* ---- Ordering of items below correspond to definitions in asl.h;          */
/* ---- change with great care! ----                                         */
                                /* ---- default key list for each screen --- */
static ASL_KEY_LIST asl_key_list = {
                                        /*  0 ASL_SINGLE_MENU_SC             */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SHELL_KEY,
                                        /*  1 ASL_DIALOGUE_SC                */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SHELL_KEY
                    | ASL_LIST_KEY | ASL_DEFAULT_KEY
                    | ASL_EDIT_KEY | ASL_COMMAND_KEY,
                                        /*  2 ASL_SINGLE_LIST_SC             */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_FIND_KEY
                    | ASL_FIND_NEXT_KEY,
                                        /*  3 ASL_CONTEXT_HELP_SC            */
    ASL_HELP_SC_KEYS | ASL_HELP_KEY,
                                        /*  4 ASL_OUTPUT_SC                  */
    ASL_OUTPUT_KEYS | ASL_FIND_KEY | ASL_FIND_NEXT_KEY,
                                        /*  5 ASL_ACK_MSG_SC                 */
    ASL_COMMON_KEYS | ASL_HELP_KEY,
                                        /*  6 ASL_EDIT_SC                    */
    ASL_COMMON_KEYS | ASL_HELP_KEY,
                                        /*  7 ASL_MULTI_LIST_SC              */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SELECT_KEY
                    | ASL_FIND_KEY | ASL_FIND_NEXT_KEY,
                                        /*  8 ASL_COMMAND_SC                 */
    ASL_COMMON_KEYS | ASL_HELP_KEY,
                                        /*  9 ASL_PRINT_LIST_SC              */
    ASL_COMMON_KEYS | ASL_HELP_KEY,
    0,                                  /* 10 ASL_INFORM_MSG_SC              */
    0,                                  /* 11 ASL_OUTPUT_LEAVE_SC            */
    ASL_COMMON_KEYS | ASL_HELP_KEY,     /* 12 ASL_EXIT_SC                    */
    ASL_COMMON_KEYS | ASL_HELP_KEY,     /* 13 ASL_CANCEL_SC                  */
    ASL_DIAG_KEYS,                      /* 14 ASL_DIAG_OUTPUT_LEAVE_SC       */
    ASL_DIAG_KEYS   | ASL_ENTER_KEY,    /* 15 ASL_DIAG_NO_KEYS_ENTER_SC      */
    ASL_DIAG_KEYS   | ASL_ENTER_KEY,    /* 16 ASL_DIAG_ENTER_SC              */
                                        /* 17 ASL_DIAG_DIALOGUE_SC           */
    ASL_CANCEL_KEY  | ASL_REDRAW_KEY
                    | ASL_HELP_KEY  | ASL_EXIT_KEY   | ASL_SELECT_KEY
                    | ASL_LIST_KEY  | ASL_DEFAULT_KEY,
    ASL_DIAG_KEYS   | ASL_ENTER_KEY,    /* 18 ASL_DIAG_DIALOGUE_HELP_SC      */
    ASL_DIAG_KEYS   | ASL_ENTER_KEY,    /* 19 ASL_DIAG_DIALOGUE_LIST_SC      */
    ASL_DIAG_KEYS,                      /* 20 ASL_DIAG_LIST_CANCEL_EXIT_SC   */
                                        /* 21 ASL_DIAG_LIST_COMMIT_SC        */
    ASL_DIAG_KEYS   | ASL_SELECT_KEY,
    ASL_DIAG_KEYS   | ASL_ENTER_KEY,    /* 22 ASL_DIAG_KEYS_ENTER_SC         */
    ASL_HELP_SC_KEYS,                   /* 23 ASL_GENERAL_HELP_SC            */
                                        /* 24 ASL_COLOR_LIST_SC              */
    ASL_CANCEL_KEY  | ASL_ENTER_KEY | ASL_BACK_KEY,
    0,                                  /* 25 ASL_OUTPUT_LEAVE_NO_SCROLL_SC  */
                                        /* 26 ASL_DIALOGUE_LEAVE_SC          */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SHELL_KEY
                    | ASL_LIST_KEY | ASL_DEFAULT_KEY
                    | ASL_EDIT_KEY | ASL_COMMAND_KEY,
    0,                                  /* 27 ASL_DIAG_LEAVE_NO_KEYS_SC      */
    ASL_DIAG_KEYS   | ASL_HELP_KEY,     /* 28 ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC
*/
                                        /* 29 ASL_DIAG_ENTER_HELP_SC         */
    ASL_DIAG_KEYS   | ASL_ENTER_KEY  | ASL_HELP_KEY,
                                        /* 30 ASL_DIAG_LIST_COMMIT_HELP_SC   */
    ASL_DIAG_KEYS   | ASL_SELECT_KEY | ASL_HELP_KEY,
                                        /* 31 ASL_DSMIT_DIALOGUE_SC          */ 
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SHELL_KEY
                    | ASL_LIST_KEY | ASL_DEFAULT_KEY  | ASL_DSMIT_KEYS
                    | ASL_EDIT_KEY | ASL_COMMAND_KEY,
                                        /* 32 ASL_DSMIT_DIALOGUE_FIELD_SC    */
    ASL_COMMON_KEYS | ASL_HELP_KEY 
                    | ASL_LIST_KEY | ASL_DEFAULT_KEY  | ASL_FIELD_KEY
                    | ASL_EDIT_KEY,
                                        /* 33 ASL_DSMIT_SINGLE_MENU_SC       */
    ASL_COMMON_KEYS | ASL_HELP_KEY | ASL_SHELL_KEY | ASL_COLLECTIVE_KEY,
    ASL_CANCEL_KEY,                     /* 34 ASL_DSMIT_INFORM_SC            */
                                        /* 35 ASL_DSMIT_OUTPUT_SC            */
    ASL_OUTPUT_KEYS | ASL_FIND_KEY | ASL_FIND_NEXT_KEY
};
/* ------------------------------------------------------------------------- */

                                        /* ---- asl_malloct() control ----   */
#define MALLOC_BLOCK 4000               /* how much to get/free for sbrk()   */
                                        /* avoidance                         */
#define MALLOC_RESET 20                 /* how many mallocs to do before     */
                                        /* resetting counter                 */

int             diag_client = FALSE;    /* TRUE after diag. screen code seen */
static int      insert_on = FALSE;      /* TRUE if insert currently on       */
static wchar_t  asl_edit_data[ASL_MAX_INPUT_BUF];  /* edit buffer       */
                                        /* fill for "invisible" fields       */
static char background[ASL_MAX_VALUE_SIZE + 1];


int DSMIT = FALSE;                      /* variable denoting if runnint dsmit*/

/*---------------------------------------------------------------------------*/
/*      ASL_SCREEN      create/manage a screen display                       */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Puts up the type of screen specified by scr_type->screen_code,
 *      and allocates memory for associated screen data structures.
 *      Then the big switch statement selects the appropriate logic to
 *      be used for responding to user typeins.
 *      Finally the screen is terminated, freeing screen data
 *      structure memory.
 *
 *      State information (relative scroll view, cursor location) between
 *      calls for the same logical screen is maintained in the caller's
 *      scr_type & scr_info data structures.
 *
 * RETURNS:
 *      ASL_CANCEL
 *      ASL_ENTER
 *      ASL_HELP
 *      ASL_LIST
 *      ASL_COMMAND
 *      ASL_COMMIT
 *      ASL_ENTER
 */

ASL_RC asl_screen (scr_type, scr_info)
    ASL_SCR_TYPE       *scr_type;       /* overall screen-related info.      */
    ASL_SCR_INFO       scr_info[];      /* line or entry-specific scr. info. */
{
ASL_RC          ret;                    /* return code                       */
ASL_SCREEN      *s;                     /* the main scr. state data structs. */
int             index;                  /* indexes scr_info entries          */
int             offset;                 /* seperation in number of lines     */


ASL_TRACER("> asl_screen");
if (asl_init_done != TRUE)              /* help Diagnostics discover         */
{                                        /* asl_init()/asl_term() phase       */
                                        /* errors in complicated/nested      */
                                        /* subroutines/processes             */
    fprintf(stderr, MSGSTR(ERR_1820_001, M_ERR_1820_001));
    return ASL_FAIL;
}
if ( ASL_IS_DIAG_SC_CODE(scr_type->screen_code))
    diag_client = TRUE;
asl_cursor(FALSE);                      /* turn off cursor                   */
                                        /* create and initialize an ASL scr. */
ret = asl_init_screen(&s, scr_type, scr_info);
if (ret != ASL_OK)
{
    asl_term_screen(s, ret);            /* free up screen data structures    */
    ASL_TRACER("< asl_screen");
    return ret;
}

if((from_list_help == 1) || (from_list_search == 1))
{
    for(index=1; index<SCR_MAX; index++)
    {
      if(selection_array[index] == ASL_YES)
      {
          scr_info[index].multi_select_flag = ASL_YES;
          WMOVE(s-> scroll_data, scr_info[index].line_num, s-> x_status_flag);
          WADDCH(s-> scroll_data, '>');
      }
    }
    offset = scr_info[SCR_CUR].line_num  - s-> y_view_cur - s-> y_view_offset;
                                            /* update display                */

    asl_scroll_screen(s, offset, scr_type, scr_info,TRUE,TRUE);
    from_list_help = 0;
}

ret = ASL_OK;                           /* for 1st pass of while loops below */
if (    scr_type->screen_code != ASL_OUTPUT_LEAVE_SC
     && scr_type->screen_code != ASL_DIAG_OUTPUT_LEAVE_SC
     && scr_type->screen_code != ASL_DIAG_LEAVE_NO_KEYS_SC )
    asl_flush_input();                  /* heave "between the screens" input */

switch (scr_type->screen_code) {        /* screen-specific input handling    */
    case ASL_EXIT_SC:
    case ASL_CANCEL_SC:
        while ( ret == ASL_OK ) {
            s-> c = (wchar_t) asl_getch(&(s->type)); /* wait for user input  */
                                        /* check for pfkeys                  */
            ret = asl_function_key(s, scr_type, scr_info, FALSE);
            if ( ret == ASL_FAIL ) {
                if (s-> c != KEY_LOCESC)
                                        /* not locator input                 */
                    asl_beep();         /* no valid pfkeys; no other         */
                                        /* valid possibilities               */
                ret = ASL_OK;
            }
        } /* while */
        break;
    case ASL_INFORM_MSG_SC:
        break;                          /* just return once display is up    */
    case ASL_SINGLE_MENU_SC:
    case ASL_DSMIT_SINGLE_MENU_SC:
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
        while ( ret == ASL_OK )
        {
           static ASL_SCR_TYPE   s_scr_type;
           static ASL_SCR_INFO   *s_scr_info=NULL;
           static char  *pattern = NULL;     /* search pattern */
           static int enter_flag = 0;
           static char edit_data_value[ASL_MAX_INPUT_BUF];

           if(scr_type->screen_code == ASL_SINGLE_LIST_SC)
           {
             if(from_list_search)     /* if returned from "Find" screen   */
             {
                if(enter_flag)        /* user hit Enter in "Find" screen   */
                {
                  enter_flag = 0;
                  if(s_scr_info[1].data_value[0] != '\0')
                  {
                    int cursor_pos;
                    int byte_position;
                    char *text_ptr;
                    char *loc2;
                    int tmp_cur_pos = SCR_CUR + 1;


                    from_list_search = 0;
                    if(tmp_cur_pos >= SCR_MAX)
                       tmp_cur_pos = 1;
                    pattern = s_scr_info[1].data_value;
                    for(index=tmp_cur_pos; index<SCR_MAX; index++)
                    {
                      text_ptr = scr_info[index].text;
                      cursor_pos = 0;
                      byte_position = 0;
                      while(cursor_pos --)
                         byte_position += NLchrlen(text_ptr + byte_position);

                      loc2=search_string(&text_ptr[byte_position],&pattern);
                      if(loc2 != NULL)
                      {
                        SCR_CUR = index;
                        WMOVE(s-> scroll_data,  scr_info[SCR_CUR].line_num,
                              s-> x_status_flag);
                        offset = scr_info[SCR_CUR].line_num  - s-> y_view_cur
                                        - s-> y_view_offset;
                                            /* update display                */
                        asl_scroll_screen(s,offset,scr_type,scr_info,TRUE,TRUE);
                        ret = ASL_OK;
                        break;
                      }

                      if(index == SCR_MAX - 1)
                      {                       /* search from the beginning  */
                        if(tmp_cur_pos != 1)  /* of the list                */
                        {
                           index = 0;
                           tmp_cur_pos = 1;
                        }
                        else
                        {
                           ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                  MSGSTR(MSG_PATTERN_NOT_FOUND, 
					 M_MSG_PATTERN_NOT_FOUND));
                           if(ret == ASL_ENTER || ret == ASL_CANCEL)
                              ret = ASL_OK;
                           if(ret == ASL_EXIT)
                              return ret;
                        }
                      }
                    }
                  }
                }
                from_list_search = 0;
             }
            }

            s-> c = (wchar_t) asl_getch(&(s->type));  /* wait for user input */
                                /* translate mouse activity to keypress      */
            if (ASL_OK == asl_locator(s))
            {
                                        /* enter = select + commit           */
                if (s-> type == ASL_KEY_ENTER || s-> type == KEY_NEWL)
                {
                    if (scr_info[SCR_CUR].non_select == ASL_YES)
                    {
                                        /* this is not a selectable item     */
                        if (s-> screen_code == ASL_SINGLE_LIST_SC)
                        {
                            ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                     MSGSTR(MSG_NON_SELECT, M_MSG_NON_SELECT));
                            asl_cursor(TRUE);
                            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                            {
                                ret = ASL_OK;
                            }
                        }
                        else
                        {
                            asl_beep();
                                        /* restore cursor                    */
                            asl_scroll_screen(s, 0,
                                   scr_type, scr_info, FALSE, TRUE);
                            ret = ASL_OK;
                        }
                    }
                    else
                    {
                                        /* indicate selection has been  made */
                        ret = ASL_COMMIT;
                        if ((s-> screen_code == ASL_DIAG_LIST_COMMIT_SC) ||
                            (s-> screen_code == ASL_DIAG_LIST_COMMIT_HELP_SC))
                            ret = ASL_ENTER;
                    }
                }
                else if (s-> type == KEY_SLASH)
                {
                   if(scr_type->screen_code == ASL_SINGLE_LIST_SC)
                   {
                     if (!s_scr_info)
                     {
                        s_scr_type.cur_index = 1;
                        s_scr_type.max_index = 3;
                        s_scr_type.cur_win_index = 0;
                        s_scr_type.cur_win_offset = 0;
                        s_scr_type.screen_code = ASL_EDIT_SC;
                        s_scr_info = (ASL_SCR_INFO *)
                                  ASL_MALLOCT (sizeof (ASL_SCR_INFO) *
                                               s_scr_type.max_index+1);
                        s_scr_info[s_scr_type.max_index].text =
                           MSGSTR(MSG_062, M_MSG_062);
                        s_scr_info[s_scr_type.max_index].entry_type =
                                                 ASL_TEXT_ENTRY;
                        s_scr_info[0].text =
                           MSGSTR(MSG_FIND_STRING, M_MSG_FIND_STRING);
                        s_scr_info[0].entry_type = ASL_TEXT_ENTRY;
                        s_scr_info[1].disp_values = NULL;
                        s_scr_info[1].data_value = NULL;
                        s_scr_info[1].entry_size = ASL_DEFAULT_LIST_TEXT_SIZE;
                        s_scr_info[1].entry_type = ASL_TEXT_ENTRY;
                        s_scr_info[1].text = NULL;

                     }

                     /* We use s_scr_info[1].text to save which cmd_opt we   */
                     /* are in.  This will tell us if we are in:             */
                     /*   o same cmd_opt as previous -- display prev pattern */
                     /*   o new cmd_opt -- display no pattern                */

                     if((s_scr_info[1].text == NULL) ||         /* if new    */
                        (strcmp(s_scr_info[1].text, scr_info[0].text) != 0))
                     {
                        s_scr_info[1].text = scr_info[0].text;
                        edit_data_value[0] = '\0';
                     }

                     for(index=1; index<SCR_MAX; index++)
                       selection_array[index]=scr_info[index].multi_select_flag;

                     s_scr_info[1].data_value = edit_data_value;
                     ret = asl_screen(&s_scr_type, s_scr_info);
 
                     if(ret == ASL_ENTER || ret == ASL_CANCEL)
                     {
                       if(ret == ASL_ENTER)
                          enter_flag = 1;
                       from_list_search = 1;
                       ret = ASL_OK;
                     }
                   }
                }
                else if (s->c == 'n')  /* user hin 'n' in list screen */
                {
                   if(scr_type->screen_code == ASL_SINGLE_LIST_SC)
                   {
                       if(edit_data_value[0] != '\0')
                       {
                          enter_flag = 1;
                          from_list_search = 1;
                       }
                   }
                }

                else if (s-> type == KEY_RIGHT)
                {
                    /*vscroll (s-> main_win , 0, 1);*/
                    if((scr_type-> screen_code == ASL_SINGLE_LIST_SC) &
                       (s->x_view_cur<(s->x_scroll_data_size-s->main_win_cols)))
                    {
                            s-> x_view_cur += __max_disp_width;
                            asl_copy_view(s);
                            WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur, 
                                  s-> x_value);
                            TOUCHWIN(s-> main_win);
                            WREFRESH(s-> main_win);
                     }
                     else
                            asl_beep();
                }
                else if (s-> type == KEY_LEFT)
                {
                    /*vscroll (s-> main_win , 0, -1);*/
                    if((scr_type-> screen_code == ASL_SINGLE_LIST_SC) &
                       (s-> x_view_cur > 0))
                    {
                            s-> x_view_cur -= __max_disp_width;
                            asl_copy_view(s);
                            WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur,
                                  s-> x_value);
                            TOUCHWIN(s-> main_win);
                            WREFRESH(s-> main_win);
                    }
                    else
                            asl_beep();
                                        /* check for vertical movement       */
                }
                else if(ASL_FAIL == asl_up_down_screen(s, scr_type, scr_info))
                {
                                        /* check for pfkeys                  */
                    ret = asl_function_key(s, scr_type, scr_info, FALSE);
                    if ( ASL_FAIL == ret )
                    {
                                        /* not below threshold locator input */
                        if (s-> c != KEY_LOCESC)
                                        /* not a valid pfkey;                */
                                        /* no other possibilities            */
                            asl_beep();
                        ret = ASL_OK;
                    }
                    if(ret == ASL_CANCEL || ret == ASL_EXIT)
                    {
                        ASL_FREE(s_scr_info); s_scr_info = NULL;
                    }
                }
            }
        } /* while */
        break;
    case ASL_MULTI_LIST_SC:
        while ( ret == ASL_OK )
        {
           static ASL_SCR_TYPE   s_scr_type;
           static ASL_SCR_INFO   *s_scr_info=NULL;
           static char  *pattern = NULL;     /* search pattern */
           static int enter_flag = 0;
           static int num_items_selected = 0;
           static char edit_data_value[ASL_MAX_INPUT_BUF];

           if(from_list_search)   /* if returned from "Find" screen  */
           {
              if(enter_flag)      /* user hit Enter in "Find" screen */
              {
                enter_flag = 0;
                if(s_scr_info[1].data_value[0] != '\0')
                {
                   int cursor_pos;
                   int byte_position;
                   char *text_ptr;
                   char *loc2;
                   int tmp_cur_pos = SCR_CUR + 1;


                   from_list_search = 0;
                   if(tmp_cur_pos >= SCR_MAX)
                      tmp_cur_pos = 1;
                   pattern = s_scr_info[1].data_value;
                   for(index=tmp_cur_pos; index<SCR_MAX; index++)
                   {
                     text_ptr = scr_info[index].text;
                     cursor_pos = 0;
                     byte_position = 0;
                     while(cursor_pos --)
                        byte_position += NLchrlen(text_ptr + byte_position);

                     loc2=search_string(&text_ptr[byte_position],&pattern);
                     if(loc2 != NULL)
                     {
                        SCR_CUR = index;
                        WMOVE(s-> scroll_data,  scr_info[SCR_CUR].line_num,
                              s-> x_status_flag);
                        offset = scr_info[SCR_CUR].line_num  - s-> y_view_cur
                                      - s-> y_view_offset;
                                            /* update display                */

                        asl_scroll_screen(s,offset,scr_type,scr_info,TRUE,TRUE);
                        ret = ASL_OK;
                        break;
                    }

                    if(index == SCR_MAX - 1)
                    {                           /* search from the beginning */
                       if(tmp_cur_pos != 1)     /* of the list               */
                       {
                          index = 0;
                          tmp_cur_pos = 1;
                       }
                       else
                       {
                          ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                  MSGSTR(MSG_PATTERN_NOT_FOUND, 
					 M_MSG_PATTERN_NOT_FOUND));
                          if(ret == ASL_ENTER || ret == ASL_CANCEL)
                             ret = ASL_OK;
                          if(ret == ASL_EXIT)
                             return ret;
                       }
                    }
                  }
                 }
               }
               from_list_search = 0;
            }


            s-> c = (wchar_t) asl_getch(&(s->type)); /* wait for user input  */
                                        /* translate mouse activity          */
                                        /* to keypress                       */
            if (ASL_OK == asl_locator(s))
            {
                                        /* commit the previous selections?   */
                if (s-> type == ASL_KEY_ENTER || s-> type == KEY_NEWL)
                {
                    ret = ASL_COMMIT;
		    /* If no items have been selected, then select the item under
		     * the cursor before exiting.
		     */
		    if (scr_info[SCR_CUR].non_select != ASL_YES && num_items_selected == 0)
		      scr_info[SCR_CUR].multi_select_flag = ASL_YES;			
		    num_items_selected = 0;
                }
                else if (s-> type == KEY_RIGHT)
                {
                    if(s->x_view_cur<(s->x_scroll_data_size-s->main_win_cols))
                    {
                         s-> x_view_cur += __max_disp_width;
                         asl_copy_view(s);
                         WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur,
                               s-> x_value);
                         TOUCHWIN(s-> main_win);
                         WREFRESH(s-> main_win);
                    }
                    else
                        asl_beep();
                }
                else if (s-> type == KEY_LEFT)
                {
                    if (s-> x_view_cur > 0)
                    {
                        s-> x_view_cur -= __max_disp_width;
                        asl_copy_view(s);
                        WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur,
                              s-> x_value);
                        TOUCHWIN(s-> main_win);
                        WREFRESH(s-> main_win);
                    }
                    else
                        asl_beep();
		}
                else if (s-> type == KEY_SLASH)
                {
                   if (!s_scr_info)
                   {
                      s_scr_type.cur_index      = 1;
                      s_scr_type.max_index      = 3;
                      s_scr_type.cur_win_index = 0;
                      s_scr_type.cur_win_offset = 0;
                      s_scr_type.screen_code = ASL_EDIT_SC;
                      s_scr_info = (ASL_SCR_INFO *)
                                ASL_MALLOCT (sizeof (ASL_SCR_INFO) *
                                             s_scr_type.max_index+1);
                      s_scr_info[s_scr_type.max_index].text =
                           MSGSTR(MSG_062, M_MSG_062);
                      s_scr_info[s_scr_type.max_index].entry_type =
                                             ASL_TEXT_ENTRY;
                      s_scr_info[0].text =
                           MSGSTR(MSG_FIND_STRING, M_MSG_FIND_STRING);
                      s_scr_info[0].entry_type = ASL_TEXT_ENTRY;
                      s_scr_info[1].disp_values = NULL;
                      s_scr_info[1].data_value = NULL;
                      s_scr_info[1].entry_size = ASL_DEFAULT_LIST_TEXT_SIZE;
                      s_scr_info[1].entry_type = ASL_TEXT_ENTRY;
                      s_scr_info[1].text = NULL;
                   }

                   /* We use s_scr_info[1].text to save which cmd_opt we are */
                   /* in.  This will tell us if we are in:                   */
                   /*   o same cmd_opt as previous -- display prev pattern   */
                   /*   o new cmd_opt -- display no pattern                  */

                   if (s_scr_info[1].text == NULL ||         /* if new       */
                       strcmp (s_scr_info[1].text, scr_info[0].text))
                   {
                      s_scr_info[1].text =  scr_info[0].text;
                      edit_data_value[0] = '\0';
                   }

                   for(index=1; index<SCR_MAX; index++)
                     selection_array[index] = scr_info[index].multi_select_flag;

                   s_scr_info[1].data_value = edit_data_value;
                   ret = asl_screen(&s_scr_type, s_scr_info);

                   if(ret == ASL_ENTER || ret == ASL_CANCEL)
                   {
                     if(ret == ASL_ENTER)
                        enter_flag = 1;
                     from_list_search = 1;
                     ret = ASL_OK;
                   }
		 }
                else if (s->c == 'n')
                {
                   if(edit_data_value[0] != '\0')
                   {
                      enter_flag = 1;
                      from_list_search = 1;
                   }
                }
                                        /* check for vertical movement       */
                else if(ASL_FAIL == asl_up_down_screen(s, scr_type, scr_info))
                {
                                        /* check for pfkeys                  */
                    ret = asl_function_key(s, scr_type, scr_info, FALSE);
                    if ( ASL_FAIL == ret )
                    {
                                        /* not below threshold locator input */
                        if (s-> type != KEY_LOCESC)
                            asl_beep();
                        ret = ASL_OK;
                    }
                    else if (ret == ASL_CANCEL || ret == ASL_ENTER)
                    {
                        ASL_FREE(s_scr_info); s_scr_info = NULL;
                    }
                    else if (ASL_COMMIT == ret)
                    { /* select toggle?        */
                                                    /* select prohibited?    */
                        if (scr_info[SCR_CUR].non_select == ASL_YES)
                        {
                                            /* This is not a selectable item */
                            ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                     MSGSTR(MSG_NON_SELECT, M_MSG_NON_SELECT));
                            asl_cursor(TRUE);
                            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                                ret = ASL_OK;
                            continue;
                        }
                        else
                        {
                            if (scr_info[SCR_CUR].multi_select_flag == ASL_YES)
                            {
                                scr_info[SCR_CUR].multi_select_flag = ASL_NO;
				num_items_selected--;
                                WMOVE(s-> scroll_data,
                                      scr_info[SCR_CUR].line_num,
                                      s-> x_status_flag);
                                            /* remove select indicator       */
                                WADDCH(s-> scroll_data, ' ');
                                            /* update virtual viewport       */
                            }
                            else
                            {
                                scr_info[SCR_CUR].multi_select_flag = ASL_YES;
				num_items_selected++;
                                WMOVE(s-> scroll_data,
                                      scr_info[SCR_CUR].line_num,
                                      s-> x_status_flag);
                                            /* add select indicator          */
                                WADDCH(s-> scroll_data, '>');
                                            /* update virtual viewport       */
                            }
                        }
                        offset = scr_info[SCR_CUR].line_num
                                    - s-> y_view_cur
                                    - s-> y_view_offset;
                                            /* update display                */
                        asl_scroll_screen(s,
                                          offset,
                                          scr_type, scr_info,
                                          TRUE, TRUE);
                        ret = ASL_OK;
                    }
                }
	      }
        } /* while */
        break;
    case ASL_EDIT_SC:
        asl_cursor(TRUE);               /* turn on block cursor for editing  */
        while ( ret == ASL_OK ) {
            s-> c = (wchar_t) asl_getch(&(s->type));  /* wait for user input */
                                        /* translate mouse activity          */
                                        /* to keypress                       */
            if (ASL_OK == asl_locator(s)) {
                ret = asl_function_key(s, scr_type, scr_info, FALSE);
                if ( ASL_FAIL == ret ) {
                                        /* [skip check for vertical motion]  */
                    asl_change_field(s, scr_type, scr_info);
                    ret = ASL_OK;
                }
            }
        }
        break;
    case ASL_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_FIELD_SC:
    case ASL_DIAG_DIALOGUE_SC:
        asl_cursor(TRUE);               /* turn on block cursor for typeins  */
        insert_on = FALSE;
        while ( ret == ASL_OK ) {
            s-> c = (wchar_t) asl_getch(&(s->type));  /* wait for user input */
                                        /* translate mouse activity          */
                                        /* to keypress                       */
            if (ASL_OK == asl_locator(s)) {
                                        /* check for vertical movement       */
                if ( ASL_FAIL == asl_up_down_screen(s, scr_type, scr_info) )
                {
                                        /* check for implicit ring->list key */
                                        /* translation                       */

                                        /* is this a ring key?               */
                    if (   (s->type == KEY_TAB || s-> type == KEY_BTAB)
                                        /* on non-ring field?                */
                            && (scr_info[SCR_CUR].op_type == ASL_LIST_ENTRY))
                    {
                        ret = asl_note (ASL_MSG, ASL_NO_LOG,
                                        MSGSTR(MSG_USE_PF4, M_MSG_USE_PF4));
                        asl_cursor(TRUE);
                        if (ret == ASL_ENTER || ret == ASL_CANCEL)
                            ret = ASL_OK;
                    }
                    else
                    {
                                        /* check for pfkeys                  */
                        ret = asl_function_key(s, scr_type, scr_info, FALSE);
                    }
                    if ( ASL_FAIL == ret)
                    {
                        ret = asl_change_field(s, scr_type, scr_info);
                        /* reset and display initial value of entry field    */
                    }
                    else if (ASL_DEFAULT == ret)
                    {
                        /* must be on same line as entry field               */
                        if ( scr_info[SCR_CUR].line_num
                                != (s-> y_view_offset + s-> y_view_cur) )
                        {
                            ret = asl_note (ASL_MSG, ASL_NO_LOG,
                                         MSGSTR(ERR_1820_002, M_ERR_1820_002));
                            asl_cursor(TRUE);
                            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                                ret = ASL_OK;
                        }
                        else
                        {
                            scr_info[SCR_CUR].cur_value_index
                                    = scr_info[SCR_CUR].default_value_index;
                            scr_info[SCR_CUR].data_value[0] = '\0';
                            scr_info[SCR_CUR].changed       = ASL_NO;
                            asl_display_initial_value(s, SCR_CUR,
                                                      scr_type, scr_info);
                            asl_copy_view_line(s);
                            WMOVE(s-> main_win,
                                  (s-> y_scroll + s-> y_view_cur),
                                  s-> x_value);
                            TOUCHWIN(s-> main_win);
                            WREFRESH(s-> main_win);

                            if (s-> screen_code  ==  ASL_DIAG_DIALOGUE_SC) {
                                asl_copy_view_line(s);
                                s-> x_cur_col    = 0;
                                s-> x_cur_offset = 0;
                                s-> x_cur_character = 0;
                                s-> x_character_offset = 0;
                                s-> type = -2;
                                WMOVE(s-> main_win,
                                      (s-> y_scroll + s-> y_view_cur),
                                      s-> x_value);
                                TOUCHWIN(s-> main_win);
                                WREFRESH(s-> main_win);
                            }
                            ret = ASL_OK;
                        }
                    }
                    else if (ASL_EDIT == ret)
                    {
                        /* must be on same line as entry field */
                        if ( scr_info[SCR_CUR].line_num
                                != (s-> y_view_offset + s-> y_view_cur)
                                || SCR_CUR == 0)
                        {
                            ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                         MSGSTR(ERR_1820_003, M_ERR_1820_003));
                            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                                ret = ASL_OK;
                            asl_cursor(TRUE);
                        }
                        else
                            ret = asl_edit(s, scr_type, scr_info);
                    }
                    else if (ASL_LIST == ret)
                    {
                        /* must be on same line as entry field */
                        if (scr_info[SCR_CUR].line_num
                                != (s-> y_view_offset + s-> y_view_cur)
                                || SCR_CUR == 0 )
                        {
                            ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                         MSGSTR(ERR_1820_004, M_ERR_1820_004));
                            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                                ret = ASL_OK;
                            asl_cursor(TRUE);
                        }

                          /* added in defect 156444, for the situation that  */
                          /* op_type='r', no disp_values and has cmd_to_list.*/
                          /* This kind of combination is not a valid stanza, */
                          /* since printers use it very often, we have it    */
                          /* fixed here. This is only good in nameselect,    */
                          /* not in dialog.                                  */ 

                        else if ((scr_info[SCR_CUR].op_type == ASL_RING_ENTRY)
                             && (strcmp(scr_info[SCR_CUR].disp_values, "") ==0)
                             && DSMIT)
                        {
                            scr_info[SCR_CUR].op_type = ASL_LIST_ENTRY;
                        }
                        else if (scr_info[SCR_CUR].op_type == ASL_RING_ENTRY)
                        {
                            /* list operation on ring field                  */
                            int         save_list_return_help;
			    if (DSMIT)
			    {
                               char   value_buf[ASL_MAX_INPUT_BUF + 1];
                               char  *ptr;
                               index = 0;
                               asl_ring(scr_info[SCR_CUR].disp_values,
                                     index, value_buf,
                                     scr_info[SCR_CUR].entry_size);
                               if (!strcmp(value_buf, "***") ) {
                                 ptr= strchr(scr_info[SCR_CUR].disp_values,',');
                                 ptr++;
                               } else {
                                 ptr = scr_info[SCR_CUR].disp_values;
                                 if (!strncmp(ptr, "***", 3))
                                 {
                                     ptr = strchr(ptr, ASL_RING_SEP);
                                     ptr++;
                                 }

                               }
                               /* don't want ASL_HELP returned in this case   */
                               save_list_return_help = asl_list_return_help;
                               asl_list_return_help  = FALSE;
                               ret = asl_ring_list(ptr,
                                          &index, scr_info[SCR_CUR].text);
                               if (!strcmp(value_buf, "***") )
                                  index++;
			    } /* if (DSMIT) */
                            else
                            {
                              /* don't want ASL_HELP returned in this case  */
                              save_list_return_help = asl_list_return_help;
                              asl_list_return_help  = FALSE;
                              ret= asl_ring_list(scr_info[SCR_CUR].disp_values,
                                        &index, scr_info[SCR_CUR].text);
                            }
                            asl_list_return_help  = save_list_return_help;
                            if (ret == ASL_OK) {
                                /* ring indicies are zero (not 1) origin     */
                                scr_info[SCR_CUR].cur_value_index = index;
                                /* so backup by using a virtual keypress     */
                                s-> type = KEY_BTAB;
                                ret = asl_change_field(s, scr_type, scr_info);
                                /*
                                }
                                */
                            } else if (ret == ASL_EXIT) {
                                break;
                            } else {
                                TOUCHWIN(s-> main_win);
                                WREFRESH(s-> main_win);
                                ret = ASL_OK;   /* noop if problems          */
                            }
                        } else if ( scr_info[SCR_CUR].op_type
                                   != ASL_LIST_ENTRY ) {
                            ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                         MSGSTR(ERR_1820_005, M_ERR_1820_005));
                            asl_cursor(TRUE);
                            if (ret == ASL_ENTER || ret == ASL_CANCEL) {
                                ret = ASL_OK;
                            }
                        }
                    }
                }
            }
            if (ret == ASL_ENTER) {             /* ENTER == Do(it)           */
                                                /* check required entries    */
                ret = asl_check_file_entries(scr_type, scr_info);
                if (ret == ASL_OK) {
                    ret = ASL_COMMIT;
                } else if (ret == ASL_FAIL) {
                    ret = ASL_OK;
                }                               /* else ret = ASL_EXIT       */
            }
        } /* while */
        break;
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_CONTEXT_HELP_SC:
    case ASL_GENERAL_HELP_SC:
    case ASL_DSMIT_INFORM_SC:
    case ASL_ACK_MSG_SC:
    case ASL_PRINT_LIST_SC:
    case ASL_COMMAND_SC:
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
    {
        /* new code for defect 111623 (adding search function in dsmit    */
        /* output window).                                                 */

        static int  new_entry = 1; /* search in a new output window ?     */
        static int  counter;       /* number of lines in scr_info[1].text */
        static int  current_cursor;
        static char **parsed_buf;
        static char edit_data_value[ASL_MAX_INPUT_BUF];
        static char *loc2 = NULL;
        static int  found = 0;
        static int  search_first_line = 0;
        static ASL_SCR_TYPE   s_scr_type;
        static ASL_SCR_INFO   *s_scr_info=NULL;
        char *tmp_buf = NULL;


        while ( ret == ASL_OK )
        {
           static char  *pattern = NULL;     /* search pattern */
           static int enter_flag = 0;
           static int from_output_search = 0;

           if(scr_type->screen_code == ASL_DSMIT_OUTPUT_SC ||
              scr_type->screen_code == ASL_OUTPUT_SC)
           {
             if(from_output_search)     /* if returned from "Find" screen   */
             {
                if(enter_flag)        /* user hit Enter in "Find" screen   */
                {
                  enter_flag = 0;
                  if(s_scr_info[1].data_value[0] != '\0')
                  {
                    int  cursor_pos;
                    int  byte_position;
                    char *text_ptr;
                    int  tmp_cur_pos;
                    int  cur_col = 0;/* the starting column for hilighting   */
                    int  moved = 0;  /* if the user hits the arrow key after */
                                     /* doing search, moved will be set to 1 */
                          

                    if(new_entry)
                    {
                       char *cur_ptr = NULL;
                       char *newline_ptr = NULL;
                       int  str_len = 0; /* length of the current parsed line */
                       int  i, j;        /* loop counter                      */
                       int  num_of_line = 0; /* each entry occupies how many  */
                                             /* lines in the output window    */

                       tmp_buf = (char*)ASL_MALLOCT(strlen(scr_info[1].text)+2);
                       strcpy(tmp_buf, scr_info[1].text);
                       tmp_buf[strlen(scr_info[1].text)] = '\n';
                       tmp_buf[strlen(scr_info[1].text)+1] = '\0';
                       cur_ptr = tmp_buf;
                       counter = 0;
                       while((newline_ptr = strchr(cur_ptr, '\n')) != NULL)
                       {
                           str_len = newline_ptr - cur_ptr;
                           num_of_line = str_len / s->main_win_cols;
                           if(num_of_line > 0)
                           {
                               if(str_len % s->main_win_cols > 0)
                                  counter  = counter + num_of_line + 1;
                               else
                                  counter  = counter + num_of_line;
                           }
                           else
                               counter ++;
                           cur_ptr = newline_ptr + 1;
                       }

                       /* parse scr_info[1].text into separeate lines, store */
                       /* in parsed_buf in order to do searching.            */

                       parsed_buf =(char**)ASL_MALLOCT(counter * sizeof(char*));
                       cur_ptr = tmp_buf;
                       i = 0;
                       num_of_line = 0;
                       while((newline_ptr = strchr(cur_ptr, '\n')) != NULL)
                       {
                           str_len = newline_ptr - cur_ptr;
                           num_of_line = str_len / s->main_win_cols;
                           if(num_of_line > 0)
                           {
                              for(j = i; j <= num_of_line+i; j++)
                              {
                                 parsed_buf[j] =
                                     (char *)ASL_MALLOCT(s->main_win_cols+1);
                                 strncpy(parsed_buf[j], cur_ptr,
                                         s->main_win_cols);
                                 if(j == num_of_line + i)
                                    parsed_buf[j][str_len - s->main_win_cols*num_of_line] = '\0';
                                 else
                                    parsed_buf[j][s->main_win_cols] = '\0';
                                 cur_ptr = cur_ptr + s->main_win_cols;
                                 if((str_len % s->main_win_cols == 0) &&
                                    (j == num_of_line + i - 1))
                                    break;
                              }
                              if(str_len % s->main_win_cols > 0)
                                 i = j;
                              else
                                 i = i + num_of_line;
                           }
                           else
                           {
                              parsed_buf[i] = (char *)ASL_MALLOCT(str_len+1);
                              strncpy(parsed_buf[i], cur_ptr, str_len);
                              parsed_buf[i][str_len] = '\0';
                              i++;
                           }
                           cur_ptr = newline_ptr + 1;
                       }
                    }   /* if (new_entry) */

                    if(current_cursor != s->y_view_cur + s->y_view_offset)
                        moved = 1;

                    current_cursor = s->y_view_cur + s->y_view_offset;
                    tmp_cur_pos = current_cursor;
                    from_output_search = 0;
                    if(tmp_cur_pos > counter)
                       tmp_cur_pos = 0;
                    pattern = s_scr_info[1].data_value;
                    for(index=tmp_cur_pos; index<counter; index++)
                    {
                      if(new_entry)
                      {
                          text_ptr = parsed_buf[index];
                          new_entry = 0;
                      }
                      else
                      {
                         if(moved)
                             text_ptr = parsed_buf[index];
                         else
                         {
                            if(strcmp(loc2, NULL) != 0 )
                            {
                                text_ptr = loc2;
                                if(tmp_cur_pos == 0)
                                   tmp_cur_pos = -1;
                            }
                            else
                            {
                                if(found)
                                     /* found pattern at the end of the line*/
                                {
                                   if(index == counter - 1)
                                   {
                                               /* search from the beginning  */
                                               /* of the output              */
                                      index = 0;
                                      tmp_cur_pos = 0;
                                   }
                                   else
                                      index++;
                                }
                                text_ptr = parsed_buf[index];
                            }
                         }
                      }
                      cursor_pos = 0;
                      byte_position = 0;
                      while(cursor_pos --)
                         byte_position += NLchrlen(text_ptr + byte_position);

                      loc2=search_string(&text_ptr[byte_position],&pattern);
                      if(loc2 != NULL)
                      {
                        found = 1;
                        if(tmp_cur_pos == 0 && index == 0)
                            search_first_line = 1;

                        current_cursor = index;
                        cur_col = loc2 - parsed_buf[index] -
                                  strlen(s_scr_info[1].data_value) + 1;

                        offset = current_cursor  - s-> y_view_cur
                                        - s-> y_view_offset;
                                            /* update display                */

                        asl_scroll_screen(s,offset,scr_type,scr_info,
                                          TRUE,TRUE);
                        asl_hilite_on(s->scroll_data, current_cursor, cur_col-1,
                                 strlen(s_scr_info[1].data_value), cur_col-1);

                        asl_copy_view(s);
                        WMOVE(s-> main_win, s->y_scroll + s->y_view_cur,
                              cur_col-1);
                        TOUCHWIN(s-> main_win);
                        WREFRESH(s-> main_win);
                        ret = ASL_OK;
                        break;
                      }
                      else
                        found = 0;

                      if(index == counter - 1)
                      {                       /* search from the beginning  */
                        if(tmp_cur_pos != 0)  /* of the output              */
                        {
                           index = -1;
                           tmp_cur_pos = 0;
                        }
                        else
                        {
                           if(search_first_line)
                           {
                              search_first_line = 0;
                              index = -1;
                           }
                           else
                           {
                              ret = asl_note(ASL_MSG, ASL_NO_LOG,
                                    MSGSTR(MSG_PATTERN_NOT_FOUND,
                                           M_MSG_PATTERN_NOT_FOUND));
                              if(ret == ASL_ENTER || ret == ASL_CANCEL)
                                 ret = ASL_OK;
                              if(ret == ASL_EXIT)
                                 return ret;
                           }
                        }
                      }
                    }
                  }
                }
                from_output_search = 0;
             }
            }

            s-> c = (wchar_t) asl_getch (&(s->type));   /* wait for user input*/
            if (s-> type == ASL_KEY_ENTER || s-> type == KEY_NEWL)
            {                                             /* enter*/
                if (s-> screen_code == ASL_OUTPUT_SC ||
                    s-> screen_code == ASL_DSMIT_OUTPUT_SC ||
                    s-> screen_code == ASL_DSMIT_INFORM_SC)
                {
                    asl_beep();
                    continue;
                }
                else
                {
                    ret = ASL_ENTER;
                    break;
                }
            }
            /* translate mouse activity to similar keypress                  */
            if (ASL_OK == asl_locator(s))
            {
                if (s-> type == KEY_SLASH)
                {
                   if(scr_type->screen_code == ASL_DSMIT_OUTPUT_SC ||
                      scr_type->screen_code == ASL_OUTPUT_SC)
                   {
                     if (!s_scr_info)
                     {
                        s_scr_type.cur_index = 1;
                        s_scr_type.max_index = 3;
                        s_scr_type.cur_win_index = 0;
                        s_scr_type.cur_win_offset = 0;
                        s_scr_type.screen_code = ASL_EDIT_SC;
                        s_scr_info = (ASL_SCR_INFO *)
                                  ASL_MALLOCT (sizeof (ASL_SCR_INFO) *
                                               s_scr_type.max_index+1);
                        s_scr_info[s_scr_type.max_index].text =
                           MSGSTR(MSG_062, M_MSG_062);
                        s_scr_info[s_scr_type.max_index].entry_type =
                                                 ASL_TEXT_ENTRY;
                        s_scr_info[0].text =
                           MSGSTR(MSG_FIND_STRING, M_MSG_FIND_STRING);
                        s_scr_info[0].entry_type = ASL_TEXT_ENTRY;
                        s_scr_info[1].disp_values = NULL;
                        s_scr_info[1].data_value = NULL;
                        s_scr_info[1].entry_size = ASL_DEFAULT_LIST_TEXT_SIZE;
                        s_scr_info[1].entry_type = ASL_TEXT_ENTRY;
                        s_scr_info[1].text = NULL;
                     }

                     /* We use s_scr_info[1].text to save which cmd_opt we   */
                     /* are in.  This will tell us if we are in:             */
                     /*   o same cmd_opt as previous -- display prev pattern */
                     /*   o new cmd_opt -- display no pattern                */

                     if((s_scr_info[1].text == NULL) ||         /* if new    */
                        (strcmp(s_scr_info[1].text, scr_info[0].text) != 0))
                     {
                        s_scr_info[1].text = scr_info[0].text;
                        edit_data_value[0] = '\0';
                     }

                     s_scr_info[1].data_value = edit_data_value;
                     ret = asl_screen(&s_scr_type, s_scr_info);

                     if(ret == ASL_ENTER || ret == ASL_CANCEL)
                     {
                       if(ret == ASL_ENTER)
                          enter_flag = 1;
                       from_output_search = 1;
                       ret = ASL_OK;
                     }
                   }
                }
                else if (s->c == 'n')  /* user hin 'n' in list screen */
                {
                   if(scr_type->screen_code == ASL_DSMIT_OUTPUT_SC ||
                      scr_type->screen_code == ASL_OUTPUT_SC)
                   {
                       if(edit_data_value[0] != '\0')
                       {
                          enter_flag = 1;
                          from_output_search = 1;
                       }
                   }
                }
                                        /* check for vertical movement       */
                else if(ASL_FAIL == asl_up_down_screen(s, scr_type, scr_info))
                {
                                        /* check for pfkeys                  */
                    ret = asl_function_key(s, scr_type, scr_info, FALSE);
                    if ( ASL_FAIL == ret )
                    {
                        asl_beep();
                        ret = ASL_OK;
                    }
                }
                else
                {
                    switch (s-> screen_code)
                    {
                        case ASL_DIAG_NO_KEYS_ENTER_SC:
                        case ASL_DIAG_KEYS_ENTER_SC:
                        case ASL_DIAG_ENTER_SC:
                        case ASL_DIAG_ENTER_HELP_SC:
                        case ASL_DIAG_DIALOGUE_HELP_SC:
                            if (s-> no_scroll)
                                asl_beep();
                    }
                }
            }
        } /* while */
        new_entry = 1;
        for(index = 0; index < counter; index ++);
           ASL_FREE(parsed_buf[index]);
        ASL_FREE(parsed_buf);
        ASL_FREE(tmp_buf);
        /* we added this line in defect 143964. Later on we found out   */
        /* that it will break libasl.a, so we commented it. Don't try   */
        /* to do the free for s_scr_info here.                          */ 
        /*                                                              */
        /* ASL_FREE(s_scr_info);                                        */
        /*                                                              */
        counter = 0;
        found = 0;
        current_cursor = 0;
        edit_data_value[0] = '\0';
        loc2 = NULL;
        search_first_line = 0;
        break;
    }
    case ASL_DIALOGUE_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
        asl_cursor(TRUE);               /* turn on block cursor for editing  */
        ret = ASL_OK;                   /* put up screen and leave           */
        break;
    case ASL_DIAG_OUTPUT_LEAVE_SC:
        /* put up screen, get significant input and leave                    */
        ret = asl_read(scr_type->screen_code, FALSE, NULL);
        switch (ret) {
            case ASL_CANCEL:
            case ASL_EXIT:
                break;
            default:
                ret = ASL_OK;
        }
        break;
    case ASL_DSMIT_WHEEL_SC:
	break;
    default:
        ret = asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                 MSGSTR(ERR_1820_006, M_ERR_1820_006));
        if (ret == ASL_ENTER || ret == ASL_CANCEL) {
            ret = ASL_OK;
        }
        break;
} /* switch */

if(scr_type -> screen_code == ASL_MULTI_LIST_SC ||
   scr_type -> screen_code == ASL_SINGLE_LIST_SC)
{
  for(index=1; index<SCR_MAX; index++)
     selection_array[index] = scr_info[index].multi_select_flag;
}

asl_term_screen(s, ret);                /* free up screen data structures    */
ASL_TRACER("< asl_screen");
return ret;

} /* end asl_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_MAKE_ACTION_KEYS                                                 */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Puts the PF keys/action keys at the bottom of the main screen.
 *      Determines how many lines are needed, and draws keys so that
 *      last row of keys are on last row of screen.
 *      Note that the EDIT key is no longer F10, and is remapped to F7
 *      when the keys are drawn.
 *

 * RETURNS:
 *      Gives the screen line number used as the first line of
 *      the action key display line.
 */

static int asl_make_action_keys (s, back_nn)
    ASL_SCREEN  *s;
    int         back_nn;                /* number to show on F10=Back key     */
{
int    for_key_index = 0;
int    key_index = 0;
int    num_keys;
int    num_lines;
int    start_line;
int    quadrant = 0;
int    keys_per_line = ASL_PFKEYS_PER_LINE;

ASL_TRACER("> asl_make_action_keys");
if (s-> popup)
    keys_per_line --;
/* Display key strings centered in their fields                              */
for ( num_keys = 0, key_index = 0 ;
        key_index < ASL_NUM_PFKEYS ;
        key_index ++ )  {
                                        /* count "enabled" keys              */
    num_keys += ( asl_key_list[s-> screen_code] >> key_index ) & 1;
}
                                        /* compute number of lines needed    */
num_lines = ( num_keys + (keys_per_line - 1) ) / keys_per_line;
                                /* compute starting line                     */
start_line = s-> main_win_lines - num_lines;
for_key_index = 0;
while (TRUE) {
                                        /* find next key to be displayed     */
    for ( ; for_key_index < ASL_NUM_PFKEYS ; for_key_index ++ ) {
        /* when asl.h file can be changed, this can be simplified again      */
        /* since index order can be made to match display order again        */
        if (for_key_index == ASL_CANCEL) {
            if (diag_client) {
                key_index = ASL_BACK;   /* not used by diags                 */
            } else {
                key_index = ASL_EXIT;   /* not used by smit                  */
            }
        } else if (for_key_index == ASL_BACK && diag_client) {
            key_index = ASL_EXIT;
        } else if (for_key_index == ASL_EXIT) {
            key_index = ASL_CANCEL;
        } else {
            key_index = for_key_index;
        }
        if ( ( asl_key_list[s-> screen_code] >> key_index ) & 1 )
            break;                      /* found one                         */
    }
    if (for_key_index >= ASL_NUM_PFKEYS)        /* if no more keys, return   */
        break;
                                        /* position for next key             */
    WMOVE(s-> main_win, s-> main_win_lines - num_lines,
           ( (quadrant * s-> main_win_cols) / keys_per_line )   );
                                        /* get text for next key             */
    if (        key_index == ASL_COMMIT
             && (    s-> screen_code == ASL_DIALOGUE_SC
                  || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                  || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                  || s-> screen_code == ASL_DIALOGUE_LEAVE_SC ) ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_EDIT].label);
    } else if ( key_index == ASL_ENTER
             && ! ( ASL_IS_DIAG_SC_CODE(s-> screen_code) ) ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_ENTER_DO].label);
    } else if ( key_index == ASL_COMMIT
             &&  (   s-> screen_code == ASL_DIAG_DIALOGUE_SC
                  || s-> screen_code == ASL_DIAG_LIST_COMMIT_SC
                  || s-> screen_code == ASL_DIAG_LIST_COMMIT_HELP_SC ) ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_REAL_COMMIT].label);
    } else if ( key_index == ASL_BACK ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_BACK].label);
    } else if ( key_index == ASL_CANCEL ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_EXIT].label);
    } else if ( key_index == ASL_EXIT ) {
        WADDSTR(s-> main_win, asl_key_map[ASL_CANCEL].label);
    } else {
        WADDSTR(s-> main_win, asl_key_map[key_index].label);
    }
    quadrant ++;
    if (quadrant == keys_per_line) {    /* need to start new line?           */
        quadrant = 0;
        num_lines --;
    }
    for_key_index ++;
}
return (start_line);

} /* end asl_make_action_keys */


/*---------------------------------------------------------------------------*/
/*      ASL_FUNCTION_KEY                                                     */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Checks the input character to see if it is a function key.
 *      Then checks to see if function key enabled for corresponding screen
 *      (otherwise ASL_FAIL is returned).
 *      Then checks to see if function can be called from here
 *      (in which case ASL_OK is returned indicating no further action needed),
 *      otherwise, the corresponding ASL return code is returned.
 *
 * RETURNS:
 *      ASL_FAIL        -- not a (currently) valid function key
 *      ASL_CANCEL
 *      ASL_ENTER
 *      ASL_HELP
 *      ASL_LIST
 *      ASL_COMMAND
 *      ASL_COMMIT
 *      ASL_BACK
 *      ASL_ENTER
 */

static ASL_RC asl_function_key (s, scr_type, scr_info, noaction)
    ASL_SCREEN          *s;
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related info.      */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific screen     */
                                        /* information                       */
    int                 noaction;       /* TRUE means do not call any        */
                                        /* PF key related functions          */
{
ASL_RC  ret;                            /* return code                       */

ASL_TRACER("> asl_function_key");
switch (s-> type) {                      /* convert ESC codes and PFKEYS      */
                                        /* to asl return codes               */
    case ESC1: case KEY_F(1):
        ASL_TRACER("     F1");
        ret = asl_key_map[1].return_code;
        break;
    case ESC2: case KEY_F(2):
        ASL_TRACER("     F2");
        ret = asl_key_map[2].return_code;
        break;
    case ESC3: case KEY_F(3):
        ASL_TRACER("     F3");
        ret = asl_key_map[0].return_code;       /* F3 now Cancel             */
        break;
    case ESC4: case KEY_F(4):
        ASL_TRACER("     F4");
        ret = asl_key_map[4].return_code;
        break;
    case ESC5: case KEY_F(5):
        ASL_TRACER("     F5");
        ret = asl_key_map[5].return_code;
        break;
    case ESC6: case KEY_F(6):
        ASL_TRACER("     F6");
        ret = asl_key_map[6].return_code;
        break;
    case ESC7: case KEY_F(7):
        ASL_TRACER("     F7");
        if (   s-> screen_code == ASL_MULTI_LIST_SC
            || s-> screen_code == ASL_DIAG_LIST_COMMIT_SC
            || s-> screen_code == ASL_DIAG_LIST_COMMIT_HELP_SC
            || s-> screen_code == ASL_DIAG_DIALOGUE_SC) {
            ret = asl_key_map[7].return_code;           /* ASL_COMMIT         */
        } else {
            ret = asl_key_map[10].return_code;          /* ASL_EDIT           */
        }
        break;
    case ESC8: case KEY_F(8):
        ASL_TRACER("     F8");
        ret = asl_key_map[8].return_code;
        break;
    case ESC9: case KEY_F(9):
        ASL_TRACER("     F9");
        ret = asl_key_map[9].return_code;
        break;
    case ESC0: case KEY_F(10):
        ASL_TRACER("     F10");
        if (diag_client)
            ret = asl_key_map[3].return_code;           /* ASL_EXIT          */
        else
            ret = asl_key_map[11].return_code;          /* ASL_BACK          */
        break;
    case ASL_KEY_ENTER:                 /* enter   maps to index 12          */
    case KEY_NEWL:                      /* newline maps to index 12          */
        ASL_TRACER("     ENTER");
        ret = asl_key_map[12].return_code;      
        break;
    case KEY_SLASH: 
        ret = asl_key_map[15].return_code;
        break;
    case ESC_M:
        ret = asl_key_map[17].return_code;
        break;
    case ESC_F:
        ret = asl_key_map[18].return_code;
        break;
    case ESC_C:
        ret = asl_key_map[19].return_code;
        break;
    case ESC_S:
        ret = asl_key_map[20].return_code;
        break;
    default:
        return ASL_FAIL;                /* not a function key                */
}
/* now check to see if corresponding key enabled                             */
if  (     ((asl_key_list[s-> screen_code] >> ret) & 1)
       || (    (    s-> screen_code == ASL_DIALOGUE_SC
                 || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                 || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                 || s-> screen_code == ASL_DIALOGUE_LEAVE_SC )
            && ret == asl_key_map[ASL_EDIT].return_code
          )
    ) {
    if ( noaction == FALSE )
    switch (ret) {
        case ASL_SHELL:
 	    {
	    char *shell;
            if (!strcmp( (shell = getenv ("SMIT_SHELL")), "n") )
            {
                asl_note (ASL_MSG, ASL_NO_LOG, "The Shell function is not available for this \nsession.");
                free (shell);
                ret = ASL_OK;
                break;
            }
            ret = asl_note (ASL_SHELL_MSG, ASL_LOG,
                            MSGSTR(MSG_001, M_MSG_001));
            if (ret == ASL_ENTER)       /* shell escape confirmed            */
                                        /* saves screen, do fork/exec, etc.  */
                ret = asl_shell();
            else if (ret == ASL_CANCEL) /* shell escape cancelled            */
                ret = ASL_OK;
            break;
	    }
        case ASL_PRINT:
            if (   s-> screen_code == ASL_SINGLE_MENU_SC
                || s-> screen_code == ASL_DSMIT_SINGLE_MENU_SC
                || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                || s-> screen_code == ASL_DIALOGUE_SC   )
                ret = asl_note (ASL_PRINT_MSG, ASL_NO_LOG,
                                MSGSTR(MSG_002, M_MSG_002),
                        scr_info[0].data_value);
            else
                ret = asl_note (ASL_PRINT_MSG, ASL_NO_LOG, EMPTY_STRING);
            if (ret == ASL_ENTER) {
                ret = asl_print_screen();       /* copies screen to log file */
            } else if (ret == ASL_CANCEL) {     /* print screen cancelled    */
                ret = ASL_OK;
            }
            break;
        case ASL_REDRAW:
            asl_redraw();
            ret = ASL_OK;
            break;
        case ASL_HELP:
                                        /* determine what to call for help   */
            ret = asl_help(s-> screen_code, scr_type, scr_info);
            break;
        case ASL_BACK:
            if (asl_use_back_key == FALSE) {
                ret = ASL_CANCEL;
            } else {
                ret = ASL_EXIT;
            }
            break;
        default:
            break;
    } /* switch */
    return ret;
} else {
    return ASL_FAIL;                    /* not enabled                       */
}

} /* end asl_function_key */


/*---------------------------------------------------------------------------*/
/*      ASL_RING                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Indexes (zero origin) into a ring field,
 *      and copies up to (max_size - 1) bytes
 *      of the specified ring sub-field into the result parameter,
 *      which is then zero byte terminated.
 *      Zero-length sub-fields are permitted.
 *
 * RETURNS:
 *      ASL_OK          -- sub-field copied
 *      ASL_FAIL        -- index invalid/out of range
 */

ASL_RC asl_ring(ring, index, result, max_size)
    char        ring[];         /* a ring field                              */
    int         index;          /* a zero origin index of the desired        */
                                /* ring sub-field                            */
    char        result[];       /* copy sub-field here                       */
    int         max_size;       /* size of result to return                  */
                                /* (excluding '\0')                          */
{
char	*tmp_ptr;		/* temp pointer for cur_pos                  */
char    *cur_pos;               /* absolute, zero-origin position            */
char    *next_pos;              /* relative position from cur_pos            */
int     str_length;             /* length of last ring subfield              */

ASL_TRACER("> asl_ring");
if (index < 0)                  /* defense against parameter error           */
    index = 0;
cur_pos = ring;
if (!strncmp(cur_pos,"***", 3)) /*  *** this denotes dsmit         */
{
    tmp_ptr = strchr(cur_pos, ASL_RING_SEP);  /* skip past the '***'         */
    cur_pos = tmp_ptr++;  	/* go one more for the comma         	     */
    if (index == 0)
	index = 1;
}
while (index > 0) {             /* skip index # of delimiters                */
    index --;
                                        /* next delimeter is at this index   */
    next_pos = strchr(cur_pos, ASL_RING_SEP);
                                        /* if not enough field delimiters,   */
    if (next_pos == NULL) {
                                        /* give default                      */
        strcpy(result, EMPTY_STRING); /* default is empty string           */
        return ASL_FAIL;
    }
    cur_pos = next_pos + 1;             /* look at first char past last      */
                                        /* delimiter found                   */
}
                                        /* location of next ring             */
                                        /* subfield delimiter                */
next_pos = strchr(cur_pos, ASL_RING_SEP);
                                        /* this value fills rest of string   */
if (next_pos == NULL) {
    str_length = strlen (cur_pos);    /* length of last ring subfield      */
    if (str_length > max_size) {        /* subfield bigger than result       */
                                        /* size is an error                  */
        asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                  MSGSTR(ERR_1820_008, M_ERR_1820_008),
                  index, max_size, str_length);
    }
                                        /* copy as much as possible */
    asl_strcpy_max(result, cur_pos, max_size + 1);
} else {
    str_length = next_pos - cur_pos;    /* length of this ring subfield      */
    if (str_length > max_size) {        /* subfield bigger than result       */
                                        /* is an error                       */
        asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                  MSGSTR(ERR_1820_008, M_ERR_1820_008),
                  index, max_size, next_pos);
        str_length = max_size - 1;
    }
                                        /* copy as much as possible          */
    strncpy(result, cur_pos, str_length);
    result[str_length] = '\0';
}
return ASL_OK;
} /* end asl_ring */


/*---------------------------------------------------------------------------*/
/*      ASL_READ                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      This routine exists solely for the benefit of,
 *      and for exclusive use by, Diagnostics.
 *
 * RETURNS:
 *      ASL_FAIL -- error condition.
 *              and, depending on the particular screen_code,
 *      ASL_CANCEL
 *      ASL_ENTER
 *      ASL_HELP
 *      ASL_LIST
 *      ASL_COMMAND
 *      ASL_COMMIT
 *      ASL_ENTER
 */

ASL_RC asl_read (screen_code,wait, buf)
    ASL_SCREEN_CODE screen_code;
    int         wait;   /* TRUE means wait for at least 1 char.              */
                        /* before returning                                  */
    char        *buf;   /* buffer for returning input characters             */

{

ASL_SCREEN      *s;             /* data struct. expected by screen routines  */
ASL_RC          ret = ASL_FAIL; /* asl return code                           */
int             j;              /* return input buffer index                 */
int             c;              /* current input character                   */
ASL_SCR_TYPE    *scr_type;      /* overall screen-related information        */
ASL_SCR_INFO    *scr_info;      /* line or entry-specific screen info.       */

if (asl_init_done != TRUE) {    /* help Diagnostics trap their               */
                                /* asl_init/asl_quit phase errors            */
    fprintf(stderr, MSGSTR(ERR_1820_007, M_ERR_1820_007));
    return ASL_FAIL;
}
                                        /* need (pseudo) screens for         */
                                        /* internal routines                 */
s = (ASL_SCREEN *) ASL_MALLOCT( sizeof(ASL_SCREEN) );
s-> screen_code = screen_code;          /* the screen we are impersonating   */
s-> x_cur_col       = 0;
s-> x_cur_offset    = 0;
s-> x_cur_character = 0;
s-> x_character_offset = 0;
s-> type = -1;

if ( !wait )
    nodelay(TRUE);                      /* makes getch() return KEY_NOKEY    */
                                        /* if no data available              */
for (j = 0 ; ; j++) {
    c = asl_getch(&(s-> type));                        /* see what's available              */
    if (buf != NULL)
        buf[j] = c;                     /* ... and copy it                   */
                                        /* check for buffer overrun          */
    if (j >=  (ASL_READ_BUF_SIZE - 1) ) {
        if (buf != NULL)
            buf[1 + j] = '\0';          /* chop rest of string               */
        ret = ASL_OK;
        break;
    }
    s-> c = c;
                                        /* check for (ASL_DIAG_*_SC)         */
                                        /* pfkeys                            */
    ret = asl_function_key(s, scr_type, scr_info, TRUE);
    if (ret != ASL_FAIL) {
        switch (ret) {
            case ASL_EXIT:              /* assumes asl_output panel          */
                                        /* action_keys                       */
            case ASL_CANCEL:            /* assumes asl_output panel          */
                                        /* action_keys                       */
                if (buf != NULL)
                    buf[j] = '\0';      /* chop rest of string               */
                break;
            default:
                break;
        }
    } else {                            /* not a pfkey                       */
        switch ( s-> type ) {
            case KEY_NOKEY:
                if (buf != NULL)
                    buf[j] = '\0';      /* chop rest of string               */
                ret = ASL_OK;           /* done                              */
                break;
            case ASL_KEY_ENTER:
            case KEY_NEWL:
            default:
                break;
        } /* switch */
    }
    if ( ret != ASL_FAIL )
        break;
}

if ( ! wait )
    nodelay(FALSE);                     /* restore ASL's default state       */
ASL_FREE(s);
return ret;

} /* end asl_read */


/*---------------------------------------------------------------------------*/
/*      ASL_PRINT_SCREEN                                                     */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Copies the curscr window to the log file.
 *
 * RETURNS:
 *      ASL_OK
 *      ASL_EXIT        -- due to user response in error message popup
 */

static ASL_RC asl_print_screen()
{
char line[401];         /* line buf. (assume max. of 200 2-byte chars)       */
int     i;              /* line index                                        */
int     x;              /* current screen column                             */
int     y;              /* current screen line                               */
int     max_cols;       /* maximum num. of col. positions to get             */
                        /* on current line                                   */
int     count;          /* number of bytes converted to multbyte             */
wchar_t c;              /* width of a character				     */
int     ret = ASL_OK;   /* our return code                                   */

ASL_TRACER("> asl_print_screen");
if (asl_log_file == NULL)      /* make sure file was specified              */
{
    ret = asl_note (ASL_ERR_MSG, ASL_NO_LOG,
                    MSGSTR(ERR_1820_010, M_ERR_1820_010));
    if (ret == ASL_ENTER || ret == ASL_CANCEL) 
        ret = ASL_OK;
} 
else 
{
    fprintf(asl_log_file, MSGSTR(MSG_003, M_MSG_003));
    for (y = 0 ; y < LINES ; y++) 
    {
        i = 0;
        max_cols = COLS;
        for (x = 0 ; x < max_cols ; x++) 
        {
            WMOVE(curscr, y, x);
            c = winch(curscr);
            count = wctomb (&line[i], c);
            i += count;
            if (wcwidth(c) == 2) {
         /*     max_cols--;    we comment this out to fix defect 108822   */
		x++;
	    }
        }
        line[i] = '\0';
        fprintf(asl_log_file,"%s\n",line);
    }
    fprintf(asl_log_file, MSGSTR(MSG_004, M_MSG_004));
    fflush(asl_log_file);
}
return ret;

} /* end asl_print_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_HILITE_ON                                                       */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Hilights the specified number of characters on the specified line,
 *      starting on the specified column in the specified window.
 *      The cursor is left at the specified cursor column on the same line.
 *
 * RETURNS:
 *      None.
 */

static void asl_hilite_on (win, line, column, width, cursor_column)
    WINDOW      *win;           /* the window to be modified                 */
    int         line;           /* the line containing the field             */
                                /* to be hilighted                           */
    int         column;         /* the starting column for hilighting        */
    int         width;          /* the number of characrters to hilight      */
    int         cursor_column;  /* the column where the cursor should left   */
{
WMOVE(win, line, column);             /* start here                        */
WCHGAT(win, width, STANDOUT);          /* change the attributes             */
                                        /* to produce hilighting             */
WMOVE(win, line, cursor_column);      /* park the cursor here              */

}  /* end asl_hilite_on */


/*---------------------------------------------------------------------------*/
/*      ASL_HILITE_OFF                                                      */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      De-hilights the specified number of characters on the specified line,
 *      starting on the specified column in the specified window.
 *      The cursor is left at the specified cursor column on the same line.
 *
 * RETURNS:
 *      None.
 */

static void asl_hilite_off (win, line, column, width, cursor_column)
    WINDOW      *win;           /* the window to be modified                 */
    int         line;           /* the line containing the field             */
                                /* to be hilighted                           */
    int         column;         /* the starting column for hilighting        */
    int         width;          /* the number of characrters to hilight      */
    int         cursor_column;  /* the column where the cursor should left   */
{
WMOVE(win, line, column);               /* start here                        */
WCHGAT(win, width, NORMAL);             /* change the attributes to          */
                                        /* remove hilighting                 */
WMOVE(win, line, cursor_column);        /* part the cursor here              */

}  /* end asl_hilite_off */


/*---------------------------------------------------------------------------*/
/*      ASL_NUM_LINES                                                        */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Count the number of lines ('\n' delimited or wrapped by libcur) that
 *      are contained in a text string.
 *      The length of the current main display window is used to determine
 *      when wraps will occur.
 *
 * RETURNS:
 *      Gives the number of lines needed to display a string.
 */

static int asl_num_lines(s, str)
    ASL_SCREEN  *s;                     /* main screen data structure        */
    char        *str;                   /* input text string                 */
{
int     line_count = 1;                 /* number of lines is one more       */
                                        /* than number of newlines           */
                                        /* plus wraparounds                  */

int     col_count = 0;                  /* number of columns used in string  */
wchar_t tmp_wc;                         /* temp holder for a wide char       */
int     rc = 0;                         /* return code of mblen()            */
int     len = 0;                        /* length returned by wcwidth()      */

while ( *str ) {                        /* scan for newlines and count them  */
                                        /* until end of string reached       */
    if (*str == '\n') {
        line_count ++;
        col_count = 0;
        if (line_count >= 32767) /* MAXSHORT */
        {
                line_count = 32766;
                break;
        }
    } else {
        mbtowc (&tmp_wc, str, MB_CUR_MAX);
	/* defect 173383: check for tab */
        if (*str == '\t') 
          col_count += 8;
	else
          col_count += (((len = wcwidth(tmp_wc)) == -1) ? 1 : len);
    }
    rc = mblen(str, MB_CUR_MAX);
    if(rc == -1)
    {
       asl_note(ASL_ERR_MSG, ASL_LOG, MSGSTR(MSG_WRONG_CODESET,
                M_MSG_WRONG_CODESET));
       return (-1);
    } 
    str = str + rc;
    
    /* If screen is not a list, then wrap the line.  We don't want to wrap */
    /* on a list, because lists are scrolled windows.                      */
    if ((s->screen_code != ASL_SINGLE_LIST_SC) &&
        (s-> screen_code != ASL_MULTI_LIST_SC)) {
                /* will wrap even if entire string just fits on line         */
                /* because "WMOVE" is done without looking at next char      */
                /* which could just be a '\n' or '\0'                        */
        if ( (col_count > s-> x_text_size)
            || (col_count == s-> x_text_size
                    && (s-> x_text + s-> x_text_size)
                            > s-> main_win_cols) ) 
        {
            line_count ++;
	    /* defect 173383: col_count could have gone more
	       than one past the end of line - do a mod to find
	       out how far. */
            col_count = col_count % s->x_text_size;
            if (line_count >= 32767) /* MAXSHORT */
            {
                    line_count = 32766;
                    break;
            }
        }
    } /* if (screen_code = ASL_MULTI_LIST_SC ) */
}
return (line_count);

} /* end asl_num_lines */


/*---------------------------------------------------------------------------*/
/*      ASL_MAX_LINE                                                         */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Finds the (display) length of longest ('\n' delimited) line in a
 *      text string.
 *
 * RETURNS:
 *      Number of columns needed to display the longest line.
 */

static int asl_max_line(str)
char    *str;                   /* input text string                         */
{
int     max_length = 0;         /* longest line found so far                 */
int     str_length;             /* length of current line                    */
char    *next_str;              /* points to remainder of line               */
char    *str_buf;               /* copy of (sub)string for display           */
                                /* length calculation                        */

str_buf = ASL_MALLOCT(strlen(str) + 1);
strcpy(str_buf, str);
str = str_buf;                  /* reuse pointer on new copy                 */
while ( str ) {                 /* while not on last line                    */
                                /* (when strchr will return  NULL)           */
                                /* is this the last line?                    */
    if ( (next_str = strchr (str, '\n')) != NULL) {
        *next_str = '\0';       /* terminate substring for mbswidth()        */
        next_str ++;            /* skip over (what was a) newline            */
    }
    str_length = mbs_width(str, strlen(str)); /* get display length */
/*    str_length = NLstrdlen(str); */
    if ( str_length > max_length )
        max_length = str_length;        /* record maximum length if changed  */
    str = next_str;
}
ASL_FREE(str_buf);
return (max_length);

} /* end asl_max_line */


/*---------------------------------------------------------------------------*/
/*      ASL_INIT_SCREEN -- initialize display screen/popup                   */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Initializes the ASL_SCREEN data structure which describes the physical
 *      layout of the desired screen.  After everything is properely set up,
 *      the screen itself is displayed.
 *
 * RETURNS:
 *      ASL_OK
 *      ASL_QUIT -- can result from call to asl_note().
 */

static int     real_lines;		/* used to optimize LEAVE screens 	     */

static ASL_RC asl_init_screen(s_ptr, scr_type, scr_info)
    ASL_SCREEN      **s_ptr;        /* keeps pointer returned by malloc()    */
    ASL_SCR_TYPE    *scr_type;      /* overall screen-related info.          */
    ASL_SCR_INFO    scr_info[];     /* line/entry-specific screen info.      */
{
ASL_SCREEN      *s;             /* keeps pointer returned by malloc()        */
ASL_RC  ret = ASL_OK;           /* our return code                           */
char    *str, *next_str;        /* temporary string                          */
int     str_length;
int     str3_length;
int     max_string;
int     item;                   /* index into scr_info                       */
int     y_cur;                  /* cur. y "cursor" (line num.) for display   */
int     c;                      /* temporary character                       */
int     i;                      /* temporary integer                         */
int     back_nn;                /* used for "F10=Back nn"                    */
int     num_of_lines = 0;       /* return value of asl_num_lines             */ 

ASL_TRACER("> asl_init_screen");
if (SCR_MAX < 0)                        /* protect asl from input errors     */
    SCR_MAX = 0;
scr_info[0].non_select       = ASL_YES; /* protect asl from input errors     */
scr_info[SCR_MAX].non_select = ASL_YES; /* protect asl from input errors     */

if (SCR_CUR < 0)                        /* protect asl from input errors     */
    SCR_CUR = 0;
else if (SCR_CUR >= SCR_MAX)            /* protect asl from input errors     */
    SCR_CUR = SCR_MAX - 1;
if  (      asl_use_back_key == TRUE
        && scr_info[0].cur_value_index > 0
        && scr_info[0].cur_value_index < 100
    ) {
    back_nn = scr_info[0].cur_value_index;
} else {                                /* protect asl from input errors     */
    back_nn = 1;
}
if (! asl_normal_mode) {
    asl_normal_mode = TRUE;
    _tty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
    Stty(_tty_ch, &_tty);               /* turn off echoing                  */
}
                                        /* allocate space for screen data    */
s = (ASL_SCREEN *) ASL_MALLOCT( sizeof(ASL_SCREEN) );
*s_ptr = s;

s-> save_win    = NULL;
s-> main_win    = NULL;
s-> frame_win   = NULL;
s-> scroll_data = NULL;
s-> text_subwin = NULL;
s-> save_win    = NULL;
s-> type        = -1;

s-> save_cursor_state = asl_cursor_on;
s-> screen_code = scr_type->screen_code;
                                        /* save current screen image         */
s-> save_win = NEWWIN(LINES, COLS, 0, 0);
ASL_CURSES_RC(s-> save_win);
CLEAROK(s-> save_win, FALSE);           /* prevent auto-clear on             */
                                        /* first WREFRESH()                  */
getyx(curscr, (s-> y_save), (s-> x_save));
OVERWRITE(curscr, s-> save_win);

if (scr_type->screen_code == ASL_DSMIT_WHEEL_SC)
{
    wmove (s->save_win, LINES - 2, (COLS - ASL_DEFAULT_TEXT_SIZE - 1) / 2);
    WADDSTR (s->save_win, scr_info[1].text);
    TOUCHWIN (s->save_win);
    WREFRESH (s->save_win);
    return ASL_FAIL;
}
                                        /* title missing?                    */
if ( scr_info[0].text == NULL || strlen(scr_info[0].text) == 0 ) {
    switch (s-> screen_code) {          /* special cases for standard titles */
        case ASL_GENERAL_HELP_SC:
                    scr_info[0].text    = MSGSTR(MSG_005, M_MSG_005);
                    break;
        case ASL_CONTEXT_HELP_SC:
                    scr_info[0].text    = MSGSTR(MSG_006, M_MSG_006);
                    break;
        case ASL_COMMAND_SC:
                    scr_info[0].text    = MSGSTR(MSG_007, M_MSG_007);
                    break;
        case ASL_DSMIT_INFORM_SC:
                    scr_info[0].text    = MSGSTR(MSG_016, M_MSG_016);
                    break;
        default:
                    scr_info[0].text    = MSGSTR(MSG_008, M_MSG_008);
                    break;
    }

}
                                /* instructions missing?                     */
if ( scr_info[SCR_MAX].text == NULL
        ||  strlen(scr_info[SCR_MAX].text) == 0 ) {
    switch (s-> screen_code) {  /* special cases for standard instructions   */
        case ASL_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_FIELD_SC:
        case ASL_DIALOGUE_LEAVE_SC:
        case ASL_DIAG_DIALOGUE_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_009, M_MSG_009);
                    break;
        case ASL_MULTI_LIST_SC:
		    if (K7 == NULL)
                       scr_info[SCR_MAX].text = MSGSTR(MSG_010_ESC, M_MSG_010_ESC);
		    else
                       scr_info[SCR_MAX].text = MSGSTR(MSG_010, M_MSG_010);
                    break;
        case ASL_DIAG_DIALOGUE_LIST_SC:
        case ASL_SINGLE_MENU_SC:
        case ASL_DSMIT_SINGLE_MENU_SC:
        case ASL_SINGLE_LIST_SC:
        case ASL_COLOR_LIST_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
        case ASL_DIAG_LIST_COMMIT_SC:
        case ASL_DIAG_LIST_COMMIT_HELP_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_011, M_MSG_011);
                    break;
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
        case ASL_CONTEXT_HELP_SC:
        case ASL_GENERAL_HELP_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_051, M_MSG_051);
                    break;
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_012, M_MSG_012);
                    break;
	case ASL_SEARCH_SC:
        case ASL_EDIT_SC:                       /* set by calling routine    */
                    break;
        case ASL_PRINT_LIST_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_014, M_MSG_014);
                    break;
        case ASL_COMMAND_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_019, M_MSG_019);
                    break;
        case ASL_DSMIT_INFORM_SC:
                    scr_info[SCR_MAX].text = MSGSTR(MSG_061, M_MSG_061);
                    break;
        case ASL_ACK_MSG_SC:
        case ASL_EXIT_SC:
        case ASL_CANCEL_SC:
        case ASL_INFORM_MSG_SC:
        default:
                    scr_info[SCR_MAX].text = EMPTY_STRING;
                    break;
    }
}

switch (s-> screen_code) {              /* special cases for window defaults */
    case ASL_DIAG_LIST_CANCEL_EXIT_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
                                        /* set fields to default values      */
                for (item = 1 ; item < SCR_MAX ; item ++) {
                    if (scr_info[item].non_select != ASL_YES)
                        scr_info[item].non_select = ASL_NO;
                }
                s-> popup               = FALSE;
                break;
    case ASL_SINGLE_MENU_SC:
    case ASL_DSMIT_SINGLE_MENU_SC:
                                        /* set fields to default values      */
                for (item = 1 ; item < SCR_MAX ; item ++) {
                    if (scr_info[item].non_select != ASL_YES)
                        scr_info[item].non_select = ASL_NO;
                    scr_info[item].item_flag = ' ';
                }
                s-> popup               = FALSE;
                break;
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:
    case ASL_CONTEXT_HELP_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
                                        /* set fields to default values      */
                for (item = 1 ; item < SCR_MAX ; item ++) {
                    if (scr_info[item].non_select != ASL_YES)
                        scr_info[item].non_select = ASL_NO;
                    scr_info[item].item_flag = ' ';
                }
                s-> popup               = TRUE;
                break;
    case ASL_MULTI_LIST_SC:
                                        /* set fields to default values      */
                for (item = 1 ; item < SCR_MAX ; item ++) {
                    if (scr_info[item].non_select != ASL_YES)
                        scr_info[item].non_select = ASL_NO;
                    scr_info[item].multi_select_flag = ASL_NO;
                    scr_info[item].item_flag = ' ';
                }
                s-> popup = TRUE;
                break;
    case ASL_DSMIT_INFORM_SC:
                                        /* set fields to default values      */
                for (item = 1 ; item < SCR_MAX ; item ++) {
                    scr_info[item].non_select = ASL_YES;
                    scr_info[item].item_flag = ' ';
                }
                s-> popup = TRUE;
                break;
    case ASL_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_FIELD_SC:
    case ASL_DIALOGUE_LEAVE_SC:
    case ASL_DIAG_DIALOGUE_SC:
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    case ASL_GENERAL_HELP_SC:
                s-> popup = FALSE;
                break;
    case ASL_EDIT_SC:
    case ASL_SEARCH_SC:
    case ASL_EXIT_SC:
    case ASL_CANCEL_SC:
    case ASL_COMMAND_SC:
                s-> popup = TRUE;
                break;
    default:
                s-> popup = TRUE;
                break;
} /* switch                                                                  */

if (s-> popup) {                        /* determine number of columns       */
                                        /* and text width                    */
    if (   s-> screen_code == ASL_MULTI_LIST_SC
        || s-> screen_code == ASL_SINGLE_LIST_SC
        || s-> screen_code == ASL_ACK_MSG_SC  /* Defect 174706 added ACK_MSG */
        || s-> screen_code == ASL_CONTEXT_HELP_SC ) {
        s-> main_win_cols    = ASL_DEFAULT_LIST_TEXT_SIZE + 2;
    } else if ( s-> screen_code == ASL_COLOR_LIST_SC ) {
        s-> main_win_cols    = ASL_DEFAULT_LIST_TEXT_SIZE + 2 - 20;
    } else if ( s-> screen_code == ASL_EDIT_SC 
		|| s-> screen_code == ASL_SEARCH_SC) {
        s-> main_win_cols    = COLS - 6;
    } else {
        s-> main_win_cols    = ASL_DEFAULT_TEXT_SIZE + 2;
    }
} else {
                                        /* would size exceed maximum fixed   */
                                        /* line buffers?                     */
    if ( COLS > (i = ((ASL_WINDOW_LINE_MAX - 1) / 2)) )
        s-> main_win_cols = i;
    else
        s-> main_win_cols = COLS;
    if ( diag_client )
        s-> main_win_cols       = ASL_DIAG_COLS;
}
switch (s-> screen_code) {              /* special cases for text field      */
    case ASL_DIAG_DIALOGUE_SC:
#if DIAG_DIALOGUE
                scr_type->text_size = ASL_DIAG_DIALOGUE_TEXT_SIZE;
                break;
#endif
    case ASL_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_FIELD_SC:
    case ASL_DIALOGUE_LEAVE_SC:
                scr_type->text_size = ASL_DIALOGUE_TEXT_SIZE;
                break;
    case ASL_SINGLE_MENU_SC:
    case ASL_DSMIT_SINGLE_MENU_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
    case ASL_MULTI_LIST_SC:
    case ASL_SEARCH_SC:
    case ASL_COMMAND_SC:
    case ASL_EDIT_SC:
    case ASL_INFORM_MSG_SC:
    case ASL_EXIT_SC:
    case ASL_CANCEL_SC:
    case ASL_ACK_MSG_SC:
    case ASL_DSMIT_INFORM_SC:
    case ASL_PRINT_LIST_SC:
    case ASL_GENERAL_HELP_SC:
    case ASL_CONTEXT_HELP_SC:
                scr_type->text_size = s-> main_win_cols - 2;
                break;
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
                scr_type->text_size = s-> main_win_cols;
                break;
    default:
                ret = asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                                MSGSTR(ERR_1820_011, M_ERR_1820_011),
                    s-> screen_code);
                if (ret == ASL_ENTER || ret == ASL_CANCEL) {
                    ret = ASL_OK;
                } else {
                    ASL_TRACER("< asl_init_screen");
                    return ret;
                }
                scr_type->text_size = ASL_DEFAULT_TEXT_SIZE;
                break;
}
                                        /* ---- screen column positions ---- */
s-> x_status_flag   = 0;                /* status/select/required            */
                                        /* parameter indicator               */


if (       s-> screen_code == ASL_OUTPUT_SC
        || s-> screen_code == ASL_DSMIT_OUTPUT_SC
        || s-> screen_code == ASL_OUTPUT_LEAVE_SC
        || s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC
        || s-> screen_code == ASL_DIAG_OUTPUT_LEAVE_SC
        || s-> screen_code == ASL_DIAG_LEAVE_NO_KEYS_SC
        || s-> screen_code == ASL_DIAG_NO_KEYS_ENTER_SC
        || s-> screen_code == ASL_DIAG_KEYS_ENTER_SC
        || s-> screen_code == ASL_DIAG_ENTER_SC
        || s-> screen_code == ASL_DIAG_ENTER_HELP_SC
        || s-> screen_code == ASL_DIAG_DIALOGUE_HELP_SC
    )
    s-> x_text      = 0;                /* output gets full screen           */
                                        /* width for text                    */
else
                                        /* selection number, if present      */
    s-> x_text      = s-> x_status_flag + 2;


s-> x_text_size = scr_type->text_size;
                                /* count lines needed to store               */
                                /* scroll entries                            */
max_string = 0;


{
	int             slen;

	/*
	 * this is so that we can just display the last 10K of data.  I don't
	 * think that you can reasonably fit more than 10K on a single screen
	 * (100x100 window size) and 10K should be small enough to not cause
	 * tremendous performance problems.  I past iterations this was set
	 * at 1500 bytes, which is plenty for an hft, but not for a large
	 * xterm with a smaller font.  10K is too small for the extreme case
	 * but you have to draw the line somewhere. 
	 */

	if (s->screen_code == ASL_OUTPUT_LEAVE_SC && (slen = strlen(scr_info[1].text)) > ASL_MAX_LEAVE_BYTES) {
		real_lines = asl_num_lines(s, scr_info[1].text);
		scr_info[1].text = scr_info[1].text + slen - ASL_MAX_LEAVE_BYTES;
	} else {
		real_lines = 0;
	}
}
	
                                /* nonstandard text field handling           */
                                /* assign line num. to scr_info item entries */
                                /* nonstandard text field handling           */
if (s-> screen_code == ASL_EDIT_SC
   || s-> screen_code == ASL_SEARCH_SC) {
    scr_info[1].line_num = 0;
    y_cur = 1;                  /* does lateral scroll, don't                */
                                /* count '\n' chars                          */
} else {                        /* standard case                             */
    y_cur = 0;
                                /* step through scr_info index               */
                                /* display entries                           */
    for (item = 1 ; item < SCR_MAX ; item++ ) {
                                /* win. line num of 1st line of this index   */
        scr_info[item].line_num = y_cur;
                                        /* count lines                       */
        num_of_lines = asl_num_lines (s, scr_info[item].text);
        if(num_of_lines == -1)
           return ASL_EXIT;
        y_cur = y_cur + num_of_lines;

        str_length = asl_max_line (scr_info[item].text);
        if (max_string < str_length)
            max_string = str_length;
    }
}
s-> y_scroll_data_size  = y_cur;        /* number of (potentially)           */
                                        /* scrollable lines                  */

if (s-> popup) {                        /* determine num. lines in popup     */
    switch (s-> screen_code) {
        case ASL_INFORM_MSG_SC:
            s-> main_win_lines
                    = asl_num_lines (s, scr_info[1].text);
            break;
	case ASL_SEARCH_SC:
        case ASL_EDIT_SC:               /* always 1 line title               */
            s-> main_win_lines
                    =  7 + asl_num_lines (s, scr_info[SCR_MAX].text);
            break;
        case ASL_EXIT_SC:
        case ASL_CANCEL_SC:
            s-> main_win_lines
                    =  4 + asl_num_lines (s, scr_info[0].text)
                         + asl_num_lines (s, scr_info[SCR_MAX].text);
            break;
        case ASL_ACK_MSG_SC:
        case ASL_DSMIT_INFORM_SC:
        case ASL_PRINT_LIST_SC:
        case ASL_CONTEXT_HELP_SC:
            s-> main_win_lines
                    =  5 + asl_num_lines (s, scr_info[0].text)
                         + asl_num_lines (s, scr_info[SCR_MAX].text)
                         + s-> y_scroll_data_size;
            if (scr_info[1].text[0] == '\0')
                s-> main_win_lines -= 2;
            break;
        case ASL_SINGLE_LIST_SC:        /* always 1 line title               */
            s-> main_win_lines
                    =  7 + asl_num_lines (s, scr_info[SCR_MAX].text)
                         + s-> y_scroll_data_size;
            break;
        case ASL_COLOR_LIST_SC:
            s-> main_win_lines
                    =  5 + asl_num_lines (s, scr_info[SCR_MAX].text)
                         + s-> y_scroll_data_size;
            break;
        case ASL_MULTI_LIST_SC:         /* always 1 line title               */
                                        /* + extra PF key line               */
            s-> main_win_lines
                    =  7 + asl_num_lines (s, scr_info[SCR_MAX].text)
                         + s-> y_scroll_data_size;
            break;
        case ASL_COMMAND_SC:
            s-> main_win_lines
                    =  5 + asl_num_lines (s, scr_info[0].text)
                         + asl_num_lines (s, scr_info[SCR_MAX].text)
                         + s-> y_scroll_data_size;
            break;
        case ASL_DIAG_DIALOGUE_HELP_SC:
        case ASL_DIAG_DIALOGUE_LIST_SC:
        default:
            s-> main_win_lines
                    = 14;
            break;
    } /* switch */
    if (s-> screen_code == ASL_COLOR_LIST_SC) {
        if (s-> main_win_lines > (LINES - 2))
            s-> main_win_lines = (LINES - 2);
    } else {
        if (s-> main_win_lines > (LINES - 4))
            s-> main_win_lines = (LINES - 4);
    }
    if ( diag_client ) {
        s-> x_pop_offset = (ASL_DIAG_COLS - s-> main_win_cols) / 2;
        s-> y_pop_offset = ASL_DIAG_LINES - s-> main_win_lines - 1;
    } else {
        s-> x_pop_offset = (COLS - s-> main_win_cols) / 2;
        s-> y_pop_offset = LINES - s-> main_win_lines - 1;
    }
    s-> frame_win = NEWWIN(s->main_win_lines + 2, s-> main_win_cols + 4,
                                s-> y_pop_offset - 1, s-> x_pop_offset - 2);
    ASL_CURSES_RC(s-> frame_win);
    s-> main_win  = SUBWIN(s-> frame_win,
                                s->main_win_lines, s-> main_win_cols + 1,
                                s-> y_pop_offset, s-> x_pop_offset);
    ASL_CURSES_RC(s-> main_win);

    CLEAROK(s-> main_win, FALSE);       /* prevent auto-clear                */
    CLEAROK(s-> frame_win, FALSE);      /* prevent auto-clear                */
    TOUCHWIN(s-> frame_win);
    WREFRESH(s-> frame_win);

    WCOLOROUT(s-> frame_win, Bxa);      /* needed for use with CBOX()        */
    CBOX((s-> frame_win));              /* draw (window) frame               */
    if (s-> screen_code == ASL_COLOR_LIST_SC) {
        TOUCHWIN(s-> frame_win);
        WREFRESH(s-> frame_win);
        OVERWRITE(curscr, stdscr);
    }
    WCOLOREND(s-> main_win);
    WCOLOREND(s-> frame_win);
    TOUCHWIN(s-> frame_win);
    WREFRESH(s-> frame_win);
    switch(s-> screen_code) {           /* special case "early completion"   */
        case ASL_INFORM_MSG_SC:
                                        /* put out message text here         */
                WMOVE(s-> main_win, 1, 0);
                WADDSTR(s-> main_win, scr_info[1].text);
                TOUCHWIN(s-> frame_win);
                WREFRESH(s-> frame_win);
                return ret;             /* display and leave                 */
        case ASL_EXIT_SC:
        case ASL_CANCEL_SC:
                asl_center_first_title_line (s, scr_type, scr_info);
                                        /* put out instruction line(s) here  */
                WMOVE(s-> main_win, 2, 0);
                WADDSTR(s-> main_win, scr_info[2].text);
                                        /* put out message text here         */
                WMOVE(s-> main_win, 4, 0);
                WADDSTR(s-> main_win, scr_info[1].text);
                s-> y_pfkeys_last   = s-> main_win_lines  - 1;
                                        /* draw action keys                  */
                s-> y_pfkeys        = asl_make_action_keys (s, back_nn);
                TOUCHWIN(s-> frame_win);
                WREFRESH(s-> frame_win);
                return ret;             /* skip other "normal" panel stuff   */
        case ASL_ACK_MSG_SC:
        case ASL_DSMIT_INFORM_SC:
        case ASL_PRINT_LIST_SC:
        case ASL_EDIT_SC:
        case ASL_MULTI_LIST_SC:
        case ASL_SEARCH_SC:
        case ASL_SINGLE_LIST_SC:
        case ASL_COLOR_LIST_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
        case ASL_DIAG_DIALOGUE_LIST_SC:
        case ASL_COMMAND_SC:
        default:
                break;
    }
} else {                                /* non-popup case                    */
    s-> x_pop_offset    = 0;
    s-> y_pop_offset    = 0;
    if (diag_client)
        s-> main_win_lines      = ASL_DIAG_LINES;
    else
        s-> main_win_lines      = LINES;
    s-> main_win        = NEWWIN(s->main_win_lines, s-> main_win_cols, 0, 0);
    ASL_CURSES_RC(s-> main_win);
                                        /* only do screen clears explicitly  */
    CLEAROK(s-> main_win, FALSE);
    if ( ASL_IS_DIAG_SC_CODE(s-> screen_code) )
        asl_clear_buffer_screen(TRUE);
}

s-> button_down         = FALSE;        /* current mouse button state        */
s-> left_button_down    = FALSE;        /* current mouse button state        */
s-> right_button_down   = FALSE;        /* current mouse button state        */

if (scr_type->text_size < 0)            /* protect asl from input errors     */
    scr_type->text_size = 0;

switch (s-> screen_code) {
    case ASL_SEARCH_SC:
    case ASL_EDIT_SC:
        s-> x_value_l_flag  = 0;        /* where left (value) bracket goes   */
        s-> x_value         = s-> x_value_l_flag + 1;
                                        /* where right (value) bracket goes  */
        s-> x_value_r_flag  = s-> main_win_cols - 1;
        break;
    case ASL_DIAG_DIALOGUE_SC:
                                        /* where right (value) bracket goes  */
        s-> x_value_r_flag  = s-> main_win_cols - 4;
                                        /* where the list/ring flag goes     */
        s-> x_list_flag     = s-> main_win_cols - 2;
                                        /* where the field type flag goes    */
        s-> x_type_flag     = s-> main_win_cols - 1;
                                        /* where left (value) bracket goes   */
        s-> x_value_l_flag  = s-> x_text + scr_type->text_size + 1;
        s-> x_value_l_flag += (s-> x_value_r_flag - s-> x_value_l_flag)/2 + 2;
        s-> x_value         = s-> x_value_l_flag + 1;
        break;
    default:
                                        /* where left (value) bracket goes   */
        s-> x_value_l_flag  = s-> x_text + scr_type->text_size + 1;
        s-> x_value         = s-> x_value_l_flag + 1;
                                        /* where right (value) bracket goes  */
        s-> x_value_r_flag  = s-> main_win_cols - 4;
                                        /* where the list/ring flag goes     */
        s-> x_list_flag     = s-> main_win_cols - 2;
                                        /* where the field type flag goes    */
        s-> x_type_flag     = s-> main_win_cols - 1;
        break;
}

s-> x_value_size    = s-> x_value_r_flag - s-> x_value;
s-> x_cur_col       = 0;
s-> x_cur_offset    = 0;
s-> x_cur_character = 0;
s-> x_character_offset = 0;
                                        /* ---- screen row positions ----    */
s-> y_title         = 0;
if (s-> popup == TRUE)
    s-> y_title_last    = 0;
else
{
    /* If we're on the title, we want to set the default text size to be the
     * entire line (80) instead of ASL_DIALOGUE_TEXT_SIZE.		     */
    int tmp_text_size;
    tmp_text_size=s->x_text_size;
    s->x_text_size = 80;
    s-> y_title_last    = s-> y_title + asl_num_lines(s, scr_info[0].text) - 1;
    s->x_text_size = tmp_text_size;
}

if (s-> screen_code == ASL_DIAG_NO_KEYS_ENTER_SC ||
    s-> screen_code == ASL_DIAG_LEAVE_NO_KEYS_SC)
                                        /* no action keys in this case       */
    s-> y_pfkeys        = s-> main_win_lines + 1;
else
                                        /* draw action keys                  */
    s-> y_pfkeys        = asl_make_action_keys (s, back_nn);

switch (s-> screen_code) {
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
        s-> y_instruct_last = s-> y_pfkeys - 2;
        s-> y_instruct      = s-> y_instruct_last
                                 - asl_num_lines(s,
                                                 scr_info[SCR_MAX].text) + 1;

        if (strcmp(" ",  scr_info[SCR_MAX].text) == 0  ||  SCR_MAX < 1)
                                        /* No instruction line               */
            s-> y_instruct_last = s-> y_instruct = s-> y_pfkeys;

        s-> y_more_upper    = s-> y_title_last + 2;
                                        /* working up from bottom of screen  */
        s-> y_more_lower    = s-> y_instruct - 2;
        break;
    default:                            /* instruct. lines above scrollable  */
        s-> y_instruct      = s-> y_title_last + 2;
        s-> y_instruct_last
                = s-> y_instruct
                        + asl_num_lines(s, scr_info[SCR_MAX].text) - 1;
        switch (s-> screen_code) {
            case ASL_DIAG_DIALOGUE_SC:
            case ASL_DIAG_LIST_CANCEL_EXIT_SC:
            case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
            case ASL_DIAG_LIST_COMMIT_SC:
            case ASL_DIAG_LIST_COMMIT_HELP_SC:
                if ( strcmp(" ",  scr_info[SCR_MAX].text) == 0
                        ||  SCR_MAX < 1)
                                        /* No instruction line               */
                    s-> y_instruct_last = s-> y_instruct = s-> y_title_last;
                    break;
            default:
                break;
        }
        s-> y_more_upper    = s-> y_instruct_last + 2;
        if( s-> screen_code == ASL_DIAG_DIALOGUE_SC
                &&  strcmp(" ",       scr_info[SCR_MAX].text) != 0)
                                        /* for column titles                 */
            s-> y_more_upper    = s-> y_instruct_last + 1;
                                        /* working up from bottom of screen  */
        s-> y_more_lower    = s-> y_pfkeys - 2;
        break;
}

s-> y_pfkeys_last   = s-> main_win_lines  - 1;
if (     (    s-> screen_code == ASL_ACK_MSG_SC
           || s-> screen_code == ASL_PRINT_LIST_SC )
        && scr_info[1].text[0] == '\0') {
    s-> y_more_upper    = s-> y_instruct_last + 1;
    s-> y_scroll        = s-> y_more_upper;
} else {
    s-> y_scroll        = s-> y_more_upper + 1;
}
s-> y_scroll_last   = s-> y_more_lower - 1;
s-> y_scroll_view_size = s-> y_scroll_last - s-> y_scroll + 1;
                                        /* shrink scroll area if             */
                                        /* bigger than scroll_data           */
if (     (    s-> y_scroll_data_size <= (s-> y_scroll_view_size + 2)
           && (    s-> screen_code != ASL_DIALOGUE_SC
                && s-> screen_code != ASL_DSMIT_DIALOGUE_SC
                && s-> screen_code != ASL_DSMIT_DIALOGUE_FIELD_SC
                && s-> screen_code != ASL_DIALOGUE_LEAVE_SC )
         )
     ||  (    s-> y_scroll_data_size <= (s-> y_scroll_view_size + 1)
           && (    s-> screen_code == ASL_DIALOGUE_SC
                || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                || s-> screen_code == ASL_DIALOGUE_LEAVE_SC )
         )
     ||  (    s-> screen_code == ASL_ACK_MSG_SC
           && scr_info[1].text[0] == '\0')
     ||  (    s-> screen_code == ASL_PRINT_LIST_SC
           && scr_info[1].text[0] == '\0')
            ) {
    s-> no_scroll       = TRUE;
    s-> y_scroll_view_size = s-> y_scroll_data_size;
    if (    s-> screen_code != ASL_DIALOGUE_SC
         && s-> screen_code != ASL_DSMIT_DIALOGUE_SC
         && s-> screen_code != ASL_DSMIT_DIALOGUE_FIELD_SC
         && s-> screen_code != ASL_DIALOGUE_LEAVE_SC ) {
                                        /* move "scroll area" over upper     */
                                        /* scroll indicator                  */
        s-> y_scroll = s-> y_more_upper;
    }
    s-> y_scroll_last   = s-> y_scroll + s-> y_scroll_view_size - 1;
                                        /* for consistency; not (yet)        */
                                        /* needed if no scroll               */
    s-> y_more_lower    = s-> y_scroll_last;
    switch(s-> screen_code) {           /* float up the instruction line     */
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
     /* case ASL_DIAG_LIST_CANCEL_EXIT_SC:                                   */
     /* case ASL_DIAG_LIST_COMMIT_SC:                                        */
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
        case ASL_DIAG_DIALOGUE_LIST_SC:
            s-> y_instruct      = s-> y_scroll_last + 2;
            s-> y_instruct_last = s-> y_instruct
                                    + asl_num_lines(s,
                                                    scr_info[SCR_MAX].text)
                                            - 1;
            break;
        default:
            break;
    } /* switch                                                              */
} else {
    s-> no_scroll       = FALSE;
}
                                        /* protect asl from input errors     */
if (scr_type->cur_win_offset < 0)
    scr_type->cur_win_offset = 0;
if (scr_type->cur_win_offset
        > s-> y_scroll_data_size - s-> y_scroll_view_size)
    scr_type->cur_win_offset
        = s-> y_scroll_data_size - s-> y_scroll_view_size;
                                        /* protect asl from input errors     */
if (scr_type->cur_win_index < 0)
    scr_type->cur_win_index = 0;
if (scr_type->cur_win_index >= s-> y_scroll_view_size)
    scr_type->cur_win_index =  s-> y_scroll_view_size - 1;

if (s-> screen_code == ASL_SINGLE_LIST_SC ||
    s-> screen_code == ASL_MULTI_LIST_SC ) {
    /* Get the maximum columns needed.  It could be the sum of the maximum   */
    /* string and the offset from the left column (s->x_text).  Or it could  */
    /* be the main_win_cols; which means there will be no horizontal scroll  */
    s-> x_scroll_data_size = max ((max_string + s-> x_text), s-> main_win_cols);

    /* put the appropriate new instructions to the screen.                   */
    if (s-> x_scroll_data_size > s-> main_win_cols)
        if (s-> screen_code == ASL_MULTI_LIST_SC)
            if (K7 == NULL)
               scr_info[SCR_MAX].text = MSGSTR(MSG_013_ESC, M_MSG_013_ESC);
            else
               scr_info[SCR_MAX].text = MSGSTR(MSG_013, M_MSG_013);
        else
            scr_info[SCR_MAX].text = MSGSTR(MSG_015, M_MSG_015);

    s-> scroll_data     = NEWWIN  (max (y_cur, s-> y_scroll_data_size),
                               s-> x_scroll_data_size, 0, 0);
    ASL_CURSES_RC(s-> scroll_data);
    s-> text_subwin     = SUBWIN  (s-> scroll_data, y_cur,
                               scr_type->text_size, 0, s-> x_text);
    ASL_CURSES_RC(s-> text_subwin);
} else {
    s-> scroll_data     = NEWWIN  (max (y_cur, s-> y_scroll_data_size),
                               s-> main_win_cols, 0, 0);
    ASL_CURSES_RC(s-> scroll_data);
    s-> text_subwin     = SUBWIN  (s-> scroll_data, y_cur,
                               scr_type->text_size, 0, s-> x_text);
    ASL_CURSES_RC(s-> text_subwin);
}
s-> y_view_cur      = 0;                /* starting loc. within scroll view  */
s-> y_view_offset   = 0;                /* starting offset from scroll_data  */
s-> x_view_cur      = 0;		/* starting loc. within scroll_data  */

switch (s-> screen_code) {
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
    case ASL_DIAG_DIALOGUE_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
        if (strcmp(" ",  scr_info[SCR_MAX].text) == 0  ||  SCR_MAX < 1)
            break;                      /* no instruct. line if single blank */
    default:
                                        /* ---- fill in left justified       */
                                        /* instruct. line(s)                 */
        WMOVE(s-> main_win, s-> y_instruct, 0);
        WADDSTR(s-> main_win, scr_info[SCR_MAX].text);
        if (    s-> screen_code == ASL_OUTPUT_LEAVE_SC
             || s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC
             || s-> screen_code == ASL_OUTPUT_SC
             || s-> screen_code == ASL_DSMIT_OUTPUT_SC)
        {
                                        /* hilight "OK", "failed" indicators */
            char *start;                /* pointer to matching region        */
	    char *end;			/* pointer to the word "stdout"      */
            char *tmp_ptr;
            int  offset;                /* offset of ptr. from start of line */
            int  width=0;               /* width of region to highlight      */
            int  wid=0;
            int counter=0;

            start = strchr(scr_info[SCR_MAX].text, ':');
            start = start + 2;          /* positioned after "command: "      */
            if((end = strstr(start, "stdout")) != NULL)
            {
                tmp_ptr = start;
                while(tmp_ptr < end)
                {
                    counter++;
                    wid = strcspn(tmp_ptr, " ");
                    tmp_ptr = tmp_ptr + wid + 1;
                    width = width + wid;
                    if(*tmp_ptr == ' ')
                        break;
                }
                width = width + counter - 1;
            }
            else
                width  = strcspn(start, " ");

            if (width != 0) {
                offset = start - scr_info[SCR_MAX].text;
                asl_hilite_on (s-> main_win,  s-> y_instruct, offset, width,
                               s-> y_instruct);

            }
        }
        break;
}
                                        /* ---- fill in title                */
if (s-> popup == TRUE  &&  ! ASL_IS_DIAG_SC_CODE(s-> screen_code)) {
    asl_center_first_title_line (s, scr_type, scr_info);
} else {
    str = scr_info[0].text;
    max_string = asl_max_line(str);     /*  display width of hilight bar     */
    y_cur = 0;
    while (str) {
                                        /* is this the last line?            */
        if ( (next_str = strchr (str, '\n')) == NULL) {
                                        /* then get length                   */
            str_length = mbs_width (str, strlen(str));
            /*str_length = NLstrdlen(str); */
        } else {
        /* =============== check for strdlen cases ==============            */
                                        /* calculate line length             */
            str_length = next_str - str;
            next_str ++;                /* skip over newline                 */
        }
        if ( ASL_IS_DIAG_SC_CODE(s-> screen_code) ) {
            WMOVE(s-> main_win, y_cur,  0 );            /* left justify      */
        } else {                                        /* center            */
            if (str_length > s-> main_win_cols)
	    {		
	       /* if line is too long to fit one one screen, break it up     */
	       int blank, index;

	       blank = index = 0;

	       while (index < (str_length / 2) )
	         {
		   index++;
		   if (str[index] != ' ')
			continue;

		   if (str[index] == ' ' && index < s-> main_win_cols)
			blank=index;
		   else 
		        break;

		  } /* while (str[index]) */ 
		  str[blank] = '\n';
		  str_length = blank;
		  next_str = str + blank + 1;
	    }
             WMOVE(s-> main_win, y_cur,
                     ((s-> main_win_cols - str_length) / 2) );
        } /* if (ASL_IS_DIAG_SC_CODE) */

        if (str_length <= s-> main_win_cols)
        {
           i = ASL_WPRINTW_BUF_SIZE / 2;   /* avoid wprintw() buffer overflow   */
           while (i < strlen(str)) {       /* do incremental writes             */
               asl_wprintw (s-> main_win, "%*.*s", i, i, str);
               str_length  -= i;
               str         += i;
           }
	
           asl_wprintw (s-> main_win, "%*.*s", str_length, str_length, str);
           str = next_str;
           y_cur ++;
	} /* if (str_length < s-> main_win_cols) */
    }
}
if ( ASL_IS_DIAG_SC_CODE(s-> screen_code) ) {
                                        /* move menu number to top right     */
    if (scr_info[0].data_value != NULL
            && (str3_length = strlen (scr_info[0].data_value)) != 0 ) {
        WMOVE(s-> main_win, s-> y_title, s-> main_win_cols - str3_length - 1);
        asl_wprintw (s-> main_win, "%*s", str3_length + 1,
                     scr_info[0].data_value);
    }
}

if (    s-> screen_code == ASL_DIAG_NO_KEYS_ENTER_SC
    ||  s-> screen_code == ASL_DIAG_KEYS_ENTER_SC
    ||  s-> screen_code == ASL_DIAG_OUTPUT_LEAVE_SC
    ||  s-> screen_code == ASL_DIAG_LEAVE_NO_KEYS_SC
    ||  s-> screen_code == ASL_DIAG_DIALOGUE_HELP_SC
    ||  s-> screen_code == ASL_DIAG_ENTER_SC
    ||  s-> screen_code == ASL_DIAG_ENTER_HELP_SC)
    for (item = 1 ; item < SCR_MAX ; item++ ) {
        y_cur = scr_info[item].line_num;
                                    /* put in text                           */
        WMOVE(s-> text_subwin, y_cur, 0);
        {
            WADDSTR(s-> text_subwin, scr_info[item].text);
        }
    } /* for (each item ...)                                                 */

else
                                /* ---- fill in num/name/value/flag field(s) */
for (item = 1 ; item < SCR_MAX ; item++ ) {
    y_cur = scr_info[item].line_num;
                                                        /* put in text       */
    WMOVE(s-> text_subwin, y_cur, 0);
    if (           s-> screen_code == ASL_OUTPUT_SC
                || s-> screen_code == ASL_DSMIT_OUTPUT_SC
                || s-> screen_code == ASL_OUTPUT_LEAVE_SC
                || s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC
                || s-> screen_code == ASL_DSMIT_INFORM_SC ) {
                                        /* avoid curses "exact fit" wrap     */
        asl_waddstr (s-> text_subwin, scr_info[item].text, y_cur, s->x_text);
    } else if (    s->screen_code == ASL_ACK_MSG_SC
                || s->screen_code == ASL_COMMAND_SC
                || s->screen_code == ASL_PRINT_LIST_SC ) {
        /* Defect 129464.  If in a popup left justify the screen */
        asl_waddstr (s-> text_subwin, scr_info[item].text, y_cur, s->x_text-2);
    } else if (s-> screen_code == ASL_SINGLE_LIST_SC
                || s-> screen_code == ASL_MULTI_LIST_SC ) {
        /* Changed it from text_subwin to scroll_data to accomodate the      */
        /* longer lines list selection.                                      */
         WMOVE (s-> scroll_data, y_cur, s-> x_text);
         asl_waddstr (s-> scroll_data, scr_info[item].text, y_cur, s->x_text);
    } else if (s-> screen_code == ASL_COLOR_LIST_SC) {
        asl_list_waddstr (s, item, scr_info[item].text);
					/* if in HELP, it needs to be        */
					/* left justified		     */
    } else if (s-> screen_code == ASL_GENERAL_HELP_SC
		|| s-> screen_code == ASL_CONTEXT_HELP_SC) {
	asl_waddstr (s-> text_subwin, scr_info[item].text, y_cur, s->x_text-2);
                                        /* exclude edit popup which          */
                                        /* has no text part                  */
    } else if (s-> screen_code != ASL_EDIT_SC) {
        if (*(scr_info[item].text)) {   /* non-empty string                  */
                                        /* avoid curses "exact fit" wrap     */
	    /* Defect 137815.  Modified fix to 114325 from s->x_text to      */
	    /* s->x_text-2.						     */
            asl_waddstr (s-> text_subwin, scr_info[item].text, y_cur,
				s->x_text-2);
        }
    }
                                        /* ---- "dialogue" screen  cases     */
    if (        s-> screen_code == ASL_DIALOGUE_SC
            || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
            || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
            ||  s-> screen_code == ASL_DIALOGUE_LEAVE_SC
            ||  s-> screen_code == ASL_EDIT_SC
            ||  s-> screen_code == ASL_SEARCH_SC
            ||  s-> screen_code == ASL_DIAG_DIALOGUE_SC) {
                                        /* put in value                      */
        asl_display_initial_value(s, item, scr_type, scr_info);
                                        /* if a (one line) edit popup        */
        if (s-> screen_code == ASL_EDIT_SC
	    || s-> screen_code == ASL_SEARCH_SC)
            break;                      /* only 1 entry,                     */
                                        /* stuff below not needed            */
                                        /* translate for display             */
        switch (scr_info[item].op_type) {
            case ASL_RING_ENTRY:        /* ring                              */
                c = '+';
                break;
            case ASL_LIST_ENTRY:        /* list                              */
                c = '+';
                break;
            case '\0':                  /* default = no op defined           */
                c = ' ';
                break;
            default:                    /* junk                              */
                c = ' ';
                ret = asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                               MSGSTR(ERR_1820_012, M_ERR_1820_012),
                        item, scr_info[item].op_type,
                        item, scr_info[item].text);
                if (ret == ASL_ENTER || ret == ASL_CANCEL) {
                    ret = ASL_OK;
                } else {
                    ASL_TRACER("< asl_init_screen");
                    return ret;
                }
                break;
        } /* switch                                                          */
        WMOVE(s-> scroll_data, y_cur, s-> x_list_flag);
        WADDCH(s-> scroll_data, c);
                                        /* translate for display             */
        switch (scr_info[item].entry_type) {
            case ASL_TEXT_ENTRY:
            case ASL_RAW_TEXT_ENTRY:
                c = ' ';
                break;
            case ASL_NUM_ENTRY:
            case ASL_SIGNED_NUM_ENTRY:
                c = '#';
                break;
            case ASL_HEX_ENTRY:
                c = 'X';
                break;
            case ASL_FILE_ENTRY:
                c = '/';
                break;
            case ASL_NO_ENTRY:
                c = ' ';
                break;
            case ASL_INVISIBLE_ENTRY:
                c = '?';
                break;
            default:
                c = ' ';
                ret = asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                               MSGSTR(ERR_1820_013, M_ERR_1820_013),
                               item, scr_info[item].entry_type,
                               item, scr_info[item].text);
                if (ret == ASL_ENTER || ret == ASL_CANCEL) {
                    ret = ASL_OK;
                } else {
                    ASL_TRACER("< asl_init_screen");
                    return ret;
                }
                break;
        } /* switch */

        WMOVE(s-> scroll_data, y_cur, s-> x_type_flag);
        WADDCH(s-> scroll_data, c);
                                        /* get status flag field             */
        if (scr_info[item].required == ASL_YES_NON_EMPTY)
            c = '*';
        else
            c = ' ';
    } else {                            /* (NOT dialogue screen)             */
        c = scr_info[item].item_flag;   /* get status flag field             */
    } /* if (s-> screen_code ...)                                            */
                                        /* put in status flag field          */
    switch (s-> screen_code) {
        case ASL_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_FIELD_SC:
        case ASL_DIALOGUE_LEAVE_SC:
        case ASL_DIAG_DIALOGUE_SC:
        case ASL_SINGLE_MENU_SC:
        case ASL_DSMIT_SINGLE_MENU_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
        case ASL_DIAG_LIST_COMMIT_SC:
        case ASL_DIAG_LIST_COMMIT_HELP_SC:
        case ASL_SINGLE_LIST_SC:
        case ASL_DSMIT_INFORM_SC:
        case ASL_DIAG_DIALOGUE_LIST_SC:
        case ASL_MULTI_LIST_SC:
                                        /* display if "well behaved"         */
            if   ( iswalnum (c) || iswpunct (c) || c == ' ' ) {
                    WMOVE(s-> scroll_data, y_cur, s-> x_status_flag);
                    WADDCH(s-> scroll_data, c);
            }
            break;
        case ASL_COLOR_LIST_SC:
        case ASL_COMMAND_SC:
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
        default:
            break;
    } /* switch */

} /* for (each item ...) */

switch (s-> screen_code) {
    case ASL_EDIT_SC:
    case ASL_SEARCH_SC:
        break;
    case ASL_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_FIELD_SC:
    case ASL_DIALOGUE_LEAVE_SC:
        WMOVE(s-> main_win, s-> y_more_upper, (s-> x_value) + 2);
        asl_wprintw (s-> main_win, "%-20s",
                     MSGSTR(MSG_ENTRY_FIELDS, M_MSG_ENTRY_FIELDS));
        break;
    case ASL_DIAG_DIALOGUE_SC:
        if (scr_info[SCR_MAX].disp_values != NULL) {
            WMOVE(s-> main_win, s-> y_more_upper - 1,
                  s-> x_text + scr_type->text_size + 1);
            asl_wprintw (s-> main_win, "%-20s",
                         scr_info[SCR_MAX].disp_values);
        }
        if (scr_info[SCR_MAX].data_value != NULL) {
            WMOVE(s-> main_win,
                  s-> y_more_upper - 1, (s-> x_value) + 2);
            asl_wprintw (s-> main_win, "%-20s",
                         scr_info[SCR_MAX].data_value);
        }
        break;
    case ASL_SINGLE_MENU_SC:
    case ASL_DSMIT_SINGLE_MENU_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
    case ASL_MULTI_LIST_SC:
                                        /* column for hardware cursor        */
        s-> x_value = s-> x_text - 1;
        break;
    case ASL_COMMAND_SC:
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
    default:
                                        /* column for hardware cursor        */
        s-> x_value = s-> x_text;
        if (s-> x_value > 0)
            s-> x_value --;
        break;
} /* switch */
                                        /* set up position/hilighting/cursor */
if (s-> popup) {
    TOUCHWIN(s-> frame_win);
    WREFRESH(s-> frame_win);
}

switch (s-> screen_code) {
    case ASL_EDIT_SC:
    case ASL_SEARCH_SC:
        asl_copy_view(s);
        WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur, s-> x_value);
}

switch (s-> screen_code) {
    case ASL_EDIT_SC:
    case ASL_SEARCH_SC:
        asl_copy_view(s);
        WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur, s-> x_value);
        TOUCHWIN(s-> main_win);     /* park the hardware cursor here     */
        WREFRESH (s-> main_win);    /* park the hardware cursor here     */
        break;
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
        asl_scroll_screen(s, 9999999, scr_type, scr_info, TRUE, TRUE);
        break;
    case ASL_DIAG_OUTPUT_LEAVE_SC:
        asl_scroll_screen(s, 0000000, scr_type, scr_info, TRUE, TRUE);
        break;
    default:
                                        /* special case for initial position */
        if  (      (    s-> screen_code == ASL_DIAG_LIST_CANCEL_EXIT_SC
                     || s-> screen_code == ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC
                     || s-> screen_code == ASL_SINGLE_LIST_SC
                     || s-> screen_code == ASL_COLOR_LIST_SC
                     || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                     || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                     || s-> screen_code == ASL_DIALOGUE_SC              )
                && SCR_CUR != 1
                && scr_type->cur_win_offset == 0
                && scr_type->cur_win_index  == 0
            ) {
            asl_scroll_screen(s, scr_info[SCR_CUR].line_num,
                              scr_type, scr_info, TRUE, TRUE);
            break;
        }
                                        /* normal case for initial position  */
        i = scr_type->cur_win_index;    /* mod. by asl_scroll_screen;        */
                                        /* save for 2nd call                 */
        asl_scroll_screen(s,
                      (scr_type->cur_win_offset + s-> y_scroll_view_size - 1),
                       scr_type, scr_info, FALSE, FALSE);
        asl_scroll_screen(s, - (s-> y_scroll_view_size - i - 1),
                          scr_type, scr_info, TRUE, TRUE);
        break;
}
if ( s-> no_scroll == TRUE ) {
    int cursor_line;
    switch ( s-> screen_code) {
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
            if ( strcmp(" ",  scr_info[SCR_MAX].text) == 0 || SCR_MAX < 1) {
                str = scr_info[0].text;
                cursor_line = s-> y_title_last;
            } else {
                str = scr_info[SCR_MAX].text;
                cursor_line = s-> y_instruct_last;
            }
            while ( (next_str = strchr (str, '\n')) != NULL)
                str = next_str + 1;
            if (str_length > s-> main_win_cols)
                str_length = s-> main_win_cols ;
            str_length = mbs_width (str, strlen(str)); /* then get length   */
            /*str_length = NLstrdlen (str);*/
            WMOVE(s-> main_win, cursor_line, str_length);
            TOUCHWIN(s-> main_win);
            WREFRESH(s-> main_win);
            break;
        default:
            break;
    } /* switch */
}
ASL_TRACER("< asl_init_screen");
return ret;

} /* end asl_init_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_CENTER_FIRST_TITLE_LINE                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Extracts first line (of a possibly multiple line) title in scr_info
 *      and centers it on the main output display window.
 *
 * RETURNS:
 *      None.
 */

static void asl_center_first_title_line (s, scr_type, scr_info)
    ASL_SCREEN          *s;             /* main screen data structure        */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related info.      */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific            */
                                        /* screen information                */
{
int     str_length;                     /* length of current line            */
int     i;                              /* index into line                   */
char    *next_str;                      /* points to newline                 */
                                        /* center only first line for title  */
                                        /* is this the last line?            */
if ( (next_str = strchr (scr_info[0].text, '\n')) != NULL )
    *next_str = '\0';

str_length = mbs_width(scr_info[0].text,strlen(scr_info[0].text));

WMOVE(s-> main_win, 0, (s-> main_win_cols - str_length) / 2 );

WADDSTR(s->main_win,scr_info[0].text);

if (next_str != NULL)
   *next_str = '\n';
return;
} /* end asl_center_first_title_line */


/*---------------------------------------------------------------------------*/
/*      ASL_TERM_SCREEN                                                      */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Frees up storage allocated to the currently active window(s),
 *      and restores the previously displayed windows where appropriate.
 *
 * RETURNS:
 *      None.
 */

static void   asl_term_screen(s, return_code)
    ASL_SCREEN  *s;                     /* main screen data structure        */
    ASL_RC      return_code;            /* return code of calling routine    */
{

ASL_TRACER("> asl_term_screen");
switch (s-> screen_code) {
    case ASL_INFORM_MSG_SC:
            break;                      /* leave screen as is                */
    case ASL_DSMIT_WHEEL_SC:
	    ASL_FREE(s);
	    return;
    case ASL_MULTI_LIST_SC:
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_LIST_SC:
    case ASL_ACK_MSG_SC:
    case ASL_DSMIT_INFORM_SC:
    case ASL_PRINT_LIST_SC:
    case ASL_EDIT_SC:
    case ASL_EXIT_SC:
    case ASL_CANCEL_SC:
    case ASL_COMMAND_SC:
    case ASL_CONTEXT_HELP_SC:
    case ASL_GENERAL_HELP_SC:
        if (    s-> screen_code == ASL_EXIT_SC
                && return_code == ASL_EXIT     ) { /*  ASL_EMTER ????? DSMIT */
            break;
        }
        if (s-> screen_code == ASL_COLOR_LIST_SC) {
            asl_clear_screen();
        }
        if (    asl_list_return_help == TRUE
             && (    s-> screen_code == ASL_MULTI_LIST_SC
                  || s-> screen_code == ASL_SINGLE_LIST_SC )
           ) {
            ASL_CURSES_RC(s-> text_subwin);
            DELWIN(s-> text_subwin);
            ASL_CURSES_RC(s-> scroll_data);
            DELWIN(s-> scroll_data);
            break;                      /* leave screen as is since not all  */
                                        /* help/message popups that may be   */
                                        /* displayed next will fill screen   */
        }
        s-> save_win->_csbp = NORMAL;   /* to prevent inverse video spillage */
        ASL_TRACER("asl_term_screen: did curscr->_csbp = NORMAL");
        s-> save_win->_cury = s-> y_save;
        s-> save_win->_curx = s-> x_save;
        TOUCHWIN(s-> save_win);     /* restore previous screen image     */
        WREFRESH(s-> save_win);     /* restore previous screen image     */
        break;
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
    case ASL_OUTPUT_LEAVE_SC:
    case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_LEAVE_NO_KEYS_SC:
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
    case ASL_DIAG_ENTER_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    default:
        ASL_CURSES_RC(s-> text_subwin);
        DELWIN(s-> text_subwin);
        ASL_CURSES_RC(s-> scroll_data);
        DELWIN(s-> scroll_data);
        break;
}
if (    s-> screen_code == ASL_OUTPUT_LEAVE_SC
     || s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC) {
    if (asl_normal_mode) {
        asl_normal_mode = FALSE;
        _tty.c_lflag |= (ECHO|ECHOE|ECHOK|ECHONL);
        Stty(_tty_ch, &_tty);           /* turn on echoing                   */
        ASL_TRACER("+ set asl_normal_mode = FALSE, turned on echoing");
    }
}
ASL_CURSES_RC(s-> main_win);
DELWIN  (s-> main_win);                 /* subwin of frame_win if popup      */
if (s-> popup) {
    ASL_CURSES_RC(s-> frame_win);
    DELWIN(s-> frame_win);
}
ASL_CURSES_RC(s-> save_win);
DELWIN (s-> save_win);
if (    s-> screen_code != ASL_OUTPUT_LEAVE_SC
     && s-> screen_code != ASL_OUTPUT_LEAVE_NO_SCROLL_SC) {
    asl_cursor(s-> save_cursor_state);
}
if (    s-> screen_code != ASL_OUTPUT_LEAVE_SC
     && s-> screen_code != ASL_DIAG_OUTPUT_LEAVE_SC
     && s-> screen_code != ASL_DIAG_LEAVE_NO_KEYS_SC)
    asl_flush_input();                  /* throw away "between the screens"  */
                                        /* input                             */
ASL_FREE(s);
ASL_TRACER("< asl_term_screen");

} /* end asl_term_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_SCROLL_SCREEN                                                    */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Displays the scroll region (virtual viewport) part of the screen.
 *      Cursor is moved by the specified number of lines; scrolling
 *      is done as needed to keep the cursor in the viewport.
 *      Adjusts the hilighting to reflect changes in cursor position.
 *      Copies text from background window to display window if the
 *      scroll region needs to be scrolled, and regenerates scroll indicators.
 *      The copy_view and refresh_view flags allow updating of
 *      the data structures without doing actual screen I/O where this is
 *      needed for performance optimization.
 *
 * RETURNS:
 *      None.
 */

static void asl_scroll_screen(s, increment, scr_type, scr_info, copy_view,
                       refresh_view)
    ASL_SCREEN          *s;             /* the main screen data structure    */
    int                 increment;      /* the number of lines to cursor     */
                                        /* up or down                        */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related            */
                                        /* information                       */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific screen     */
                                        /* information                       */
    int                 copy_view;      /* TRUE means to update viewport     */
                                        /*  region                           */
    int                 refresh_view;   /* TRUE means to refresh the         */
                                        /* viewport                          */
{

int     str_length;
int     line_s;
int     max_lines;

ASL_TRACER("> asl_scroll_screen");
                                        /* turn off cursor related           */
                                        /* highlighting                      */
if (SCR_CUR == 0) {                     /* if on title                       */
    switch (s-> screen_code) {
                                        /* except for ...                    */
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
        case ASL_DIAG_LIST_COMMIT_SC:
        case ASL_DIAG_LIST_COMMIT_HELP_SC:
        case ASL_DIAG_DIALOGUE_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
        case ASL_DIAG_DIALOGUE_LIST_SC:
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
            break;
        default:
                                        /* highlight centered title          */
            str_length = asl_max_line (scr_info[SCR_CUR].text);
            asl_hilite_off (s-> main_win, s-> y_title,
                            max(1, (s-> main_win_cols - str_length) / 2 ),
                            str_length, 0);
            break;
    }
} else {
    if (     s-> screen_code == ASL_OUTPUT_SC
        ||   s-> screen_code == ASL_DSMIT_OUTPUT_SC
        ||   s-> screen_code == ASL_OUTPUT_LEAVE_SC
        ||   s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC
        ||   s-> screen_code == ASL_CONTEXT_HELP_SC
        ||   s-> screen_code == ASL_DIAG_NO_KEYS_ENTER_SC
        ||   s-> screen_code == ASL_DIAG_KEYS_ENTER_SC
        ||   s-> screen_code == ASL_DIAG_ENTER_SC
        ||   s-> screen_code == ASL_DIAG_ENTER_HELP_SC
        ||   s-> screen_code == ASL_DIAG_OUTPUT_LEAVE_SC
        ||   s-> screen_code == ASL_DIAG_LEAVE_NO_KEYS_SC)
        str_length = scr_type->text_size;
    else
        str_length = asl_input_field_size (s-> scroll_data,
                                           s-> y_view_offset + s-> y_view_cur,
                                           s-> x_text,
                                           scr_type->text_size);

    if (s-> screen_code == ASL_SINGLE_LIST_SC ||
        s-> screen_code == ASL_MULTI_LIST_SC)
        asl_hilite_off (s-> scroll_data,
                        s-> y_view_offset + s-> y_view_cur,
                        0,
                        s-> x_scroll_data_size,
                        s-> x_text);
    else
        asl_hilite_off (s-> scroll_data,
                    s-> y_view_offset + s-> y_view_cur,
                    s-> x_text,
                    max(1, str_length),
                    s-> x_text);
    if (    s-> screen_code == ASL_DIALOGUE_SC
        ||  s-> screen_code == ASL_DSMIT_DIALOGUE_SC
        ||  s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
        ||  s-> screen_code == ASL_DIALOGUE_LEAVE_SC
        ||  s-> screen_code == ASL_EDIT_SC
        ||  s-> screen_code == ASL_DIAG_DIALOGUE_SC
    ) {
        s-> x_cur_col    = 0;           /* reset value field pointers        */
        s-> x_cur_offset = 0;           /* reset value field pointers        */
        s-> x_cur_character = 0;
        s-> x_character_offset = 0;
                                        /* unscroll value field              */
        asl_display_value (s, SCR_CUR, scr_type, scr_info);
    }
    asl_copy_view_line(s);
}

if (SCR_CUR == 0) {                     /* if on title                       */
    if (increment > 0) {                /* move to first (scroll-viewable)   */
                                        /* line                              */
        SCR_CUR = 1;
    }
    increment = 0;                      /* done moving                       */
                                        /* first (scroll-viewable) line      */
} else if ( (s-> y_view_offset + s-> y_view_cur) <= 0 ) {
    switch (s-> screen_code) {
        case ASL_SINGLE_MENU_SC:
        case ASL_DSMIT_SINGLE_MENU_SC:
        case ASL_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_SC:
        case ASL_DSMIT_DIALOGUE_FIELD_SC:
        case ASL_DIALOGUE_LEAVE_SC:
            if (increment < 0) {        /* ... move up to title              */
                SCR_CUR   = 0;
                increment = 0;          /* done moving                       */
            }
            break;
        default:
            break;
    }
                                        /* last (scroll-viewable) line       */
} else if ( (s-> y_view_offset + s-> y_view_cur + 1)
           >= s-> y_scroll_data_size) {
    if (increment > 0) {
        increment = 0;                  /* done moving                       */
    }
}
                                        /* "normal scrolling"                */
if (increment > 0) {
    s-> y_view_cur += increment;        /* leap down "viewport"              */
                                        /* overshoot end of viewport?        */
    if ( s-> y_view_cur > (s-> y_scroll_view_size - 1) ) {
                                        /* lines overshooting cur. view      */
        line_s = s-> y_view_cur - (s-> y_scroll_view_size - 1);
                                        /* set to last viewport position     */
        s-> y_view_cur = s-> y_scroll_view_size - 1;
        max_lines = s-> y_scroll_data_size
                        - s-> y_scroll_view_size
                        - s-> y_view_offset;
        if (max_lines < line_s)
            line_s = max_lines;
        s-> y_view_offset += line_s;
        asl_copy_view(s);
    }
} else if (increment < 0) {
    s-> y_view_cur += increment;        /* leap up "viewport"                */
    if ( s-> y_view_cur < 0 ) {         /* overshoot end of viewport?        */
        line_s = - s-> y_view_cur;      /* lines overshooting current view   */
        s-> y_view_cur = 0;             /* set to last viewport position     */
        max_lines = s-> y_view_offset;
        if (max_lines < line_s)
            line_s = max_lines;
        s-> y_view_offset -= line_s;
        asl_copy_view(s);
    }
}

if (SCR_CUR != 0) {                     /* if not title (i.e. in viewport)   */
                                        /* return here if screen regenerated */
    scr_type->cur_win_offset = s-> y_view_offset;
                                        /* return here if screen regenerated */
    scr_type->cur_win_index  = s-> y_view_cur;
                                        /* line num. in view_data window     */
    line_s = s-> y_view_offset + s-> y_view_cur;
                                        /* find corresponding current item   */
    for (SCR_CUR = SCR_MAX - 1 ;
         line_s < scr_info[SCR_CUR].line_num && SCR_CUR > 1 ;
         SCR_CUR --);
}

/* set real_lines to y_scroll_data_size.  This will net out to no change for
 * all but ASL_OUTPUT_LEAVE_SC.  Can't put real_lines into the data struct
 * 'cause we would have to reship diagnostics and that would upset folks.
 * Sooo we have to live with a sub-optimal solution.
 */
 
if (!real_lines || s->screen_code != ASL_OUTPUT_LEAVE_SC)
	real_lines = s->y_scroll_data_size;

if (s-> no_scroll == FALSE) {           /* if scrolling indicators needed    */
                                        /* (re)set scrolling indicators      */
    WMOVE(s-> main_win, s-> y_more_upper, 0);
    if (s-> y_view_offset <= 0) {
        asl_wprintw (s-> main_win,
                     "[%s]                  ",
                     MSGSTR(MSG_TOP, M_MSG_TOP));
    }
    else {
        asl_wprintw (s-> main_win,
                     "[%s...%d]             ",
		     MSGSTR(MSG_MORE, M_MSG_MORE), s-> y_view_offset + real_lines - s->y_scroll_data_size);
    }
    WMOVE(s-> main_win, s-> y_more_lower, 0);
    if ( (s-> y_view_offset + s-> y_scroll_view_size)
        >= s-> y_scroll_data_size ) {
            asl_wprintw (s-> main_win,
                         "[%s]                  ",
                         MSGSTR(MSG_BOTTOM, M_MSG_BOTTOM));
    }
    else {
        asl_wprintw (s-> main_win,
                     "[%s...%d]             ",
                     MSGSTR(MSG_MORE, M_MSG_MORE),
                     real_lines
                     - s-> y_scroll_view_size
                     - s-> y_view_offset);
    }
}
                                        /* turn on cursor related            */
                                        /* highlighting                      */
if (SCR_CUR == 0) {                     /* if on title                       */
    str_length = asl_max_line (scr_info[SCR_CUR].text);
    switch (s-> screen_code) {
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_SC:
        case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
        case ASL_DIAG_LIST_COMMIT_SC:
        case ASL_DIAG_LIST_COMMIT_HELP_SC:
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
            break;
        default:
            asl_hilite_on (s-> main_win, s-> y_title,
                           max(1, (s-> main_win_cols - str_length) / 2),
                           str_length, 0);
            break;
    }
} else {                                /* not on title                      */
    switch (s-> screen_code) {
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
            if (s-> no_scroll) {
                str_length = 0;
                break;
            }
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
        case ASL_CONTEXT_HELP_SC:
            str_length = scr_type->text_size;
            break;
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
            str_length = 0;
            break;
        default:
            str_length
                    = asl_input_field_size (s-> scroll_data,
                                            s-> y_view_offset + s-> y_view_cur,
                                            s-> x_text, scr_type->text_size);
        break;
    }
    switch (s-> screen_code) {
        case ASL_DIAG_OUTPUT_LEAVE_SC:
        case ASL_DIAG_LEAVE_NO_KEYS_SC:
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
        case ASL_OUTPUT_SC:
        case ASL_DSMIT_OUTPUT_SC:
        case ASL_OUTPUT_LEAVE_SC:
        case ASL_OUTPUT_LEAVE_NO_SCROLL_SC:
            break;
        case ASL_SINGLE_LIST_SC:
        case ASL_MULTI_LIST_SC:
           asl_hilite_off (s-> scroll_data,
                s-> y_view_offset + s-> y_view_cur,
                0,
                s-> x_scroll_data_size,
                s-> x_value);

            asl_hilite_on (s->scroll_data,
                s-> y_view_offset + s-> y_view_cur,
                s-> x_text,
                asl_input_field_size  (s-> scroll_data,
                        s-> y_view_offset + s-> y_view_cur,
                        s-> x_text, s-> x_scroll_data_size),
                s-> x_text);
            break;
        default:
            asl_hilite_on (s-> scroll_data,
                           s-> y_view_offset + s-> y_view_cur, s-> x_text,
                           max(1, str_length), s-> x_text);
            break;
    }
    asl_copy_view_line(s);
}
if (copy_view)
    asl_copy_view(s);

if (SCR_CUR == 0) {                     /* if on title                       */
    WMOVE(s-> main_win, 0, s-> main_win_cols - 1);
} else {
    if (    s-> screen_code == ASL_OUTPUT_LEAVE_SC
         || s-> screen_code == ASL_OUTPUT_LEAVE_NO_SCROLL_SC) {
        str_length
                = asl_input_field_size (s-> scroll_data,
                                        s-> y_view_offset + s-> y_view_cur,
                                        s-> x_text, scr_type->text_size);
        WMOVE(s-> main_win,
              s-> y_scroll + s-> y_view_cur,
              s-> x_text + str_length);
    } else {
        WMOVE(s-> main_win, s-> y_scroll + s-> y_view_cur, s-> x_value);
    }
}
if (refresh_view) {
    if (copy_view) {
        s-> main_win->_csbp = NORMAL;   /* prevents inverse video spillage   */
                                        /* on WREFRESH()                     */
        ASL_TRACER("+ asl_scroll_screen: main_win->_csbp = NORMAL");
    }
    TOUCHWIN(s-> main_win);        /* park the hardware cursor here     */
    WREFRESH(s-> main_win);        /* park the hardware cursor here     */
}
ASL_TRACER("< asl_scroll_screen");

} /* end asl_scroll_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_CHANGE_FIELD                                                     */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Changes an entry field in a dialogue screen.
 *
 * RETURNS:
 *      ASL_OK          -- field updated.
 *      ASL_LIST        -- need to return to client go get a list.
 */

static ASL_RC asl_change_field(s, scr_type, scr_info)
    ASL_SCREEN          *s;             /* the main screen data structure    */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related info.      */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific screen     */
                                        /* information                       */
{
ASL_RC  ret = ASL_FAIL;         /* return code                               */
ASL_RC  ret_edit;               /* return code from asl_std_edit_keys()      */
int     c;                      /* the current input character               */
int     y_cur;                  /* current line in view_data                 */
int     str_length;             /* length of field                           */
int     len = 0;                /* length returned by wcwidth()              */

char    value_buf[ASL_MAX_INPUT_BUF];      /* ring value buffer         */

ASL_TRACER("> asl_change_field");
y_cur = s-> y_view_offset + s-> y_view_cur;
if ( scr_info[SCR_CUR].line_num != y_cur || SCR_CUR == 0) {
    return ASL_OK;                      /* ignore; not on same line as value */
}
c = s-> type;

                                        /* ring field advance/retreat        */
if ( (c == KEY_TAB) || (c == KEY_BTAB) ) {
    if ( scr_info[SCR_CUR].op_type == ASL_RING_ENTRY ) {
        s-> x_cur_col    = 0;           /* reset cursor to start of field    */
        s-> x_cur_offset = 0;           /* reset cursor to start of field    */
        s-> x_character_offset = 0;
        s-> x_cur_character = 0;
                                        /* had previous typein, now reset    */
        if (scr_info[SCR_CUR].cur_value_index < 0) {
            scr_info[SCR_CUR].cur_value_index = 0;
        } else if (c == KEY_TAB) {
            scr_info[SCR_CUR].cur_value_index ++;
            /* get new ring value                */
            if ( asl_ring (scr_info[SCR_CUR].disp_values,
                           scr_info[SCR_CUR].cur_value_index,
                           scr_info[SCR_CUR].data_value,
                           scr_info[SCR_CUR].entry_size)
                    == ASL_FAIL) {
                scr_info[SCR_CUR].cur_value_index = 0;
            }
        } else {                        /* assume KEY_BTAB                   */
            if (scr_info[SCR_CUR].cur_value_index <= 0) {
                while ( asl_ring (scr_info[SCR_CUR].disp_values,
                                  scr_info[SCR_CUR].cur_value_index,
                                  scr_info[SCR_CUR].data_value,
                                  scr_info[SCR_CUR].entry_size)
                          == ASL_OK) {
                    scr_info[SCR_CUR].cur_value_index ++;
                }
            }
	    if (!DSMIT || (strncmp (scr_info[SCR_CUR].disp_values, "***", 3) ))
            	scr_info[SCR_CUR].cur_value_index --;
        } 
        if (DSMIT) {
          asl_ring (scr_info[SCR_CUR].disp_values,
                    scr_info[SCR_CUR].cur_value_index,
                    value_buf,
                    scr_info[SCR_CUR].entry_size);
          if((strncmp(scr_info[SCR_CUR].disp_values, "***", 3) == 0) &&
                         scr_info[SCR_CUR].cur_value_index == 0)
            scr_info[SCR_CUR].cur_value_index ++;
        }
        asl_ring (scr_info[SCR_CUR].disp_values,
                  scr_info[SCR_CUR].cur_value_index,
                  scr_info[SCR_CUR].data_value,
                  scr_info[SCR_CUR].entry_size);
        if (scr_info[SCR_CUR].default_value_index
                == scr_info[SCR_CUR].cur_value_index)
            scr_info[SCR_CUR].changed = ASL_NO;
        else
            scr_info[SCR_CUR].changed = ASL_YES;
        ret = ASL_OK;
    } else {                                                                    
        ret = asl_edit(s, scr_type, scr_info);                                  
    }                                                                           
} else if ((scr_info[SCR_CUR].entry_type == ASL_NO_ENTRY) ||
           (scr_info[SCR_CUR].op_type == ASL_RING_ENTRY)) {      
                                        /* Defect 88084 */
				        /* typein not allowed to edit field  */
    ret = asl_edit(s, scr_type, scr_info);
} else if ((scr_info[SCR_CUR].op_type == ASL_RING_ENTRY) &&                     
/* typein not allowed in ring field with entry type of ASL_TEXT_ENTRY */        
    (scr_info[SCR_CUR].entry_type == ASL_TEXT_ENTRY)) {                         
    ret = asl_edit(s, scr_type, scr_info);                                      
} else {                                /* other typeins                     */
    mbstowcs(asl_edit_data,
             scr_info[SCR_CUR].data_value,
             ASL_MAX_INPUT_BUF);
    if (scr_info[SCR_CUR].entry_type == ASL_NO_ENTRY) {
        str_length = strlen(scr_info[SCR_CUR].data_value);
    } else {
        str_length = scr_info[SCR_CUR].entry_size;
    }
    ret_edit = asl_std_edit_keys (s-> c,
                                  str_length,
                                  scr_info[SCR_CUR].entry_type,
                                  &(s-> x_cur_col),
                                  &(s-> x_cur_character),
                                  s-> type);
    while ( s-> x_character_offset > 0 &&  
            s-> x_cur_col < s-> x_cur_offset )
    {
        s-> x_character_offset--;       /* keep cursor in displayed field    */

        if (s->x_character_offset == 0)
        {
          s->x_cur_offset = 0;
          break;
        }

        s-> x_cur_offset -= (((len =
          wcwidth (asl_edit_data[s-> x_character_offset])) == -1) ? 1 : len);
    }
    while ( (s-> x_cur_col - s-> x_cur_offset)  > s-> x_value_size)
    {                                   /* keep cursor in displayed field    */
        s-> x_cur_offset +=  (((len =
          wcwidth (asl_edit_data [s-> x_character_offset])) == -1) ? 1 : len);
        s-> x_character_offset++;
    }
    if ( (s-> x_cur_col - s-> x_cur_offset) == s-> x_value_size) {
        if (s-> x_cur_col == str_length - 1)
        {                               /* keep cursor in displayed field    */
            s-> x_cur_offset += (((len =
               wcwidth(asl_edit_data[s->x_character_offset])) == -1) ? 1 : len);
            s-> x_character_offset ++;
        }
    }
    wcstombs(scr_info[SCR_CUR].data_value,
             asl_edit_data,
             scr_info[SCR_CUR].entry_size + 1);

    if (ret_edit == ASL_OK) {           /* value just changed by             */
                                        /* edit operation?                   */
        asl_set_changed(&scr_info[SCR_CUR]);
    }
    ret = ASL_OK;
}
asl_display_value (s, SCR_CUR, scr_type, scr_info);
asl_copy_view_line(s);
WMOVE(s-> main_win,
       (s-> y_scroll + s-> y_view_cur),
       s-> x_value + s-> x_cur_col - s-> x_cur_offset);
TOUCHWIN(s-> main_win);            /* park the hardware cursor here     */
WREFRESH(s-> main_win);            /* park the hardware cursor here     */

ASL_TRACER("< asl_change_field");
return ret;

} /* end asl_change_field */


/*---------------------------------------------------------------------------*/
/*      ASL_SET_CHANGED                                                      */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Sets the scr_info[].changed field.
 *
 * RETURNS:
 *      None.
 */

void asl_set_changed(scr_info)
    ASL_SCR_INFO       *scr_info;       /* line or entry-specific screen     */
                                        /* information                       */
{
char    value_buf[ASL_MAX_INPUT_BUF];      /* ring value buffer         */

if (scr_info->op_type == ASL_RING_ENTRY) {
    asl_ring (scr_info->disp_values,
              scr_info->default_value_index,
              value_buf,
              scr_info->entry_size);
    if ( strcmp (value_buf, scr_info->data_value) == 0 ) {
        scr_info->changed = ASL_NO;
        scr_info->cur_value_index
                = scr_info->default_value_index;
    } else {
        scr_info->changed = ASL_YES;
                                /* indicate value override by typein */
        scr_info->cur_value_index = -1;
    }
} else {                        /* disp_values has default           */
    if ( strcmp (scr_info->data_value,
                   scr_info->disp_values) == 0 ) {
                                /* has changed back to original      */
                                /* default                           */
        scr_info->changed = ASL_NO;
    } else {
        scr_info->changed = ASL_YES;
                                /* distinguishes deleted field       */
                                /* from "standard" empty field       */
                                /* needing default                   */
        scr_info->cur_value_index = -1;
    }
}
} /* end asl_set_changed */


/*---------------------------------------------------------------------------*/
/*      ASL_UP_DOWN_SCREEN                                                   */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Translates vertical motion keypresses into appropriate screen
 *      updating actions.
 *
 * RETURNS:
 *      ASL_FAIL    -- keypress not processed by this routine.
 *      ASL_OK      -- keypress processed.
 */

static ASL_RC asl_up_down_screen(s, scr_type, scr_info)
    ASL_SCREEN          *s;             /* the main screen data structure    */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related info.      */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific screen     */
                                        /* information                       */
{
    int                 increment;      /* the number of lines to            */
                                        /* cursor up or down                 */
    int                 back_increment = 0;
ASL_RC  ret = ASL_OK;                   /* return code                       */
switch (s-> type) {
    case KEY_UP:                        /* cursor up                         */
        increment =  -1;
        break;
    case KEY_DOWN:                      /* cursor down                       */
        increment = 1;
        break;
    case KEY_NEWL:                      /* cursor down                       */
    case ASL_KEY_ENTER:                 /* cursor down                       */
        if (s-> screen_code == ASL_DIAG_DIALOGUE_SC) {
            increment = 1;
        } else {
            ret = ASL_FAIL;             /* don't handle this key             */
        }
        break;
    case KEY_PPAGE:                     /* page up                           */
        back_increment =   s-> y_view_cur;
        increment      = - (s-> y_scroll_view_size + back_increment - 2);
        break;
    case KEY_NPAGE:                     /* page down                         */
        back_increment =   (s-> y_scroll_view_size - s-> y_view_cur - 1);
        increment      =   (s-> y_scroll_view_size + back_increment - 2);
        break;
    case KEY_HOME:                      /* top of view                       */
        increment = -100000;
        break;
    case KEY_END:                       /* end of view                       */
        increment =  100000;
        break;
    default:
        ret = ASL_FAIL;                 /* don't handle this key             */
        break;
} /* switch */

if (ret == ASL_OK) {
    switch (s-> screen_code) {          /* NOTE!: falls through to default   */
        case ASL_DIAG_NO_KEYS_ENTER_SC:
        case ASL_DIAG_KEYS_ENTER_SC:
        case ASL_DIAG_ENTER_SC:
        case ASL_DIAG_ENTER_HELP_SC:
        case ASL_DIAG_DIALOGUE_HELP_SC:
            if (s-> no_scroll)
                break;
        default:
            switch (s-> type) {
                case KEY_PPAGE:        /* page up                            */
                    asl_scroll_screen(s, increment, scr_type, scr_info,
                                      FALSE, FALSE);
                    if (SCR_CUR == 0) {         /* if landing on title       */
                        back_increment = 0;     /* don't back up             */
                    }
                    increment =   back_increment;
                    break;
                case KEY_NPAGE:        /* page down                          */
                    if (SCR_CUR == 0) {         /* if starting on title      */
                        back_increment = 0;     /* don't back up             */
                    }
                    asl_scroll_screen(s, increment, scr_type, scr_info,
                                      FALSE, FALSE);
                    increment = - back_increment;
                    break;
                default:
                    break;
            }
            asl_scroll_screen(s, increment, scr_type, scr_info, FALSE, TRUE);
            break;
    } /* switch */
} /* if */
return ret;

} /* end asl_up_down_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_DISPLAY_VALUE                                                    */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Updates the screen to reflect changes in the value of an entry field.
 *
 * RETURNS:
 *      None.
 */

static void asl_display_value (s, item, scr_type, scr_info)
    ASL_SCREEN          *s;             /* the main screen data structure    */
    int                 item;           /* index of scr_info entry           */
                                        /* for changed field                 */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related info.      */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific screen     */
                                        /* information                       */
{
int     c_left, c_right;                /* temp. char. vars.                 */
int     str_length;                     /* length of entry minus current     */
                                        /* display offset                    */
int     y_cur;                          /* current line in scroll_data       */

y_cur       = scr_info[item].line_num;
                                        /* size of displayed "view"          */
str_length  = strlen ( &scr_info[item].data_value[s-> x_cur_offset] );
WMOVE(s-> scroll_data, y_cur, s-> x_value);
if (scr_info[item].entry_type == ASL_INVISIBLE_ENTRY) {
    asl_wprintw (s-> scroll_data, "%-*.*s", s-> x_value_size, s-> x_value_size,
                                        /* str_length string of              */
                                        /* background chars                  */
            background + strlen(background) - str_length);
} else {
    asl_wprintw (s-> scroll_data, "%-*.*s", s-> x_value_size, s-> x_value_size,
            &scr_info[item].data_value [s-> x_cur_offset] );
}

if (scr_info[item].changed == ASL_YES) {
    asl_hilite_on (s-> scroll_data, y_cur, s-> x_value,
            min(str_length, s-> x_value_size), s-> x_value);
} else {
    asl_hilite_off (s-> scroll_data, y_cur, s-> x_value,
            min(str_length, s-> x_value_size), s-> x_value);
}

if (s-> x_cur_offset  <=  0) {
    if (scr_info[item].entry_type == ASL_NO_ENTRY) {
        c_left = ' ';
    } else {
        c_left = '[';
    }
} else {
    c_left = '<';
}
WMOVE(s-> scroll_data, y_cur, s-> x_value_l_flag);
WADDCH(s-> scroll_data, c_left);

if (str_length > s-> x_value_size) {    /* if trailing value bigger          */
                                        /* than display field                */
    c_right = '>';
    WMOVE(s-> scroll_data, y_cur, s-> x_value_r_flag);
} else {
    WMOVE(s-> scroll_data, y_cur, s-> x_value_r_flag);
    WADDCH(s-> scroll_data, ' ');      /* make sure old flag gets zapped    */
    if (scr_info[item].entry_type == ASL_NO_ENTRY) {
        c_right = ' ';
    } else {
        c_right = ']';
    }
    WMOVE(s-> scroll_data, y_cur, s-> x_value + str_length);
}
WADDCH(s-> scroll_data, c_right);

} /* end asl_display_value */


/*---------------------------------------------------------------------------*/
/*      ASL_LOCATOR                                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Handles mouse input.  Button presses and mouse movement
 *      are mapped into corresponding function keys or cursor keys.
 *
 * RETURNS:
 *      ASL_FAIL    --  locator motion not above "significance" threshold
 *                      or not meaningful for given screen type.
 *      ASL_OK      --  locator motion/action translated to corresponding
 *                      keypress.
 */

static ASL_RC asl_locator(s)
    ASL_SCREEN  *s;             /* current screen data structure             */
{
int     i, j;                   /* y, x movement deltas                      */
ASL_RC  ret = ASL_OK;           /* return code                               */

ASL_TRACER("> asl_locator");

if (s-> type == KEY_LOCESC) {   /* if locator report ...                     */
                                /* ---- get hf_deltay                        */
    i = (short)(ESCSTR[5]<<8) | (short)(ESCSTR[6]);
                                /* ---- get hf_deltax                        */
    j = 2 * ( (short)(ESCSTR[3]<<8) | (short)(ESCSTR[4]) );
                                /* (2x scaling make relative x-y motions     */
                                /* "feel" right)                             */
                                /* determine if "mostly x" or                */
                                /* "mostly y" movement                       */
    if ( ASL_ABS(i) > ASL_ABS(j) ) {
        if ( i < -6 ) {         /* if motion exceeds activation threshold    */
            s-> c = KEY_DOWN;
        } else if ( i > 6 ) {   /* if motion exceeds activation threshold    */
            s-> c = KEY_UP;
        }
    } else {
        if ( j < -6 ) {         /* if motion exceeds activation threshold    */
            if (    s-> screen_code == ASL_DIALOGUE_SC
                 || s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                 || s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                 || s-> screen_code == ASL_DIAG_DIALOGUE_SC  ) {
                s-> c = KEY_LEFT;
            } else {
                                /* ignore lateral motion on non-entry fields */
                return ASL_FAIL;
            }
        } else if ( j > 6 ) {   /* if motion exceeds activation threshold    */
            if (    s-> screen_code == ASL_DIALOGUE_SC
                 ||  s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                 ||  s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                 ||  s-> screen_code == ASL_DIAG_DIALOGUE_SC) {
                s-> c = KEY_RIGHT;
            } else {
                                /* ignore lateral motion on non-entry fields */
                return ASL_FAIL;
            }
        }
    }
                                /* mouse buttons (overrides motion, if any)  */
#ifndef _ASL41
    if ( (char) ESCSTR[11] == HFBUTTON1 ) {
        s-> button_down = TRUE;
        s-> left_button_down = TRUE;
        ret = ASL_FAIL;         /* "failed" to find something that can be    */
                                /* passed on                                 */
    } else if ( (char) ESCSTR[11] == HFBUTTON2 ) {
        s-> button_down = TRUE;
        s-> right_button_down = TRUE;
        ret = ASL_FAIL;         /* "failed" to find something that can be    */
                                /* passed on */
    } else if (s-> button_down) {
        if (s-> left_button_down) {
            if (        s-> screen_code == ASL_DIALOGUE_SC
                 ||     s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                 ||     s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                 ||     s-> screen_code == ASL_DIAG_DIALOGUE_SC )
                s-> c = KEY_BTAB;
            else if (   s-> screen_code == ASL_MULTI_LIST_SC )
                s-> c = KEY_F(7);
            else
                s-> c = ASL_KEY_ENTER;
        } else if (s-> right_button_down) {
            if (        s-> screen_code == ASL_DIALOGUE_SC
                 ||     s-> screen_code == ASL_DSMIT_DIALOGUE_SC
                 ||     s-> screen_code == ASL_DSMIT_DIALOGUE_FIELD_SC
                 ||     s-> screen_code == ASL_DIAG_DIALOGUE_SC )
                s-> c = KEY_TAB;
            else
                s-> c = ASL_KEY_ENTER;
        }
        s-> button_down = FALSE;
        s-> left_button_down = FALSE;
        s-> right_button_down = FALSE;
    }
#endif

}
ASL_TRACER("< asl_locator");
return ret;

} /* end asl_locator */


/*---------------------------------------------------------------------------*/
/*      ASL_CLEAR_SCREEN                                                     */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Just clears the screen.
 *
 * RETURNS:
 *      ASL_OK    -- screen cleared.
 *      ASL_FAIL  -- not called following asl_init() and before asl_quit().
 */

ASL_RC asl_clear_screen()
{
ASL_TRACER("> asl_clear_screen");
return asl_clear_buffer_screen(TRUE);

} /* end asl_clear_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_CLEAR_BUFFER_SCREEN                                              */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Clears the screen buffer, and refreshes the actual screen
 *      if parameter TRUE.
 *
 * RETURNS:
 *      ASL_OK    -- screen cleared.
 *      ASL_FAIL  -- not called following asl_init() and before asl_quit().
 */

ASL_RC asl_clear_buffer_screen(refresh_flag)
    int         refresh_flag;   /* refresh screen if true                    */
{
ASL_TRACER("> asl_clear_buffer_screen");
if (asl_init_done != TRUE) {    /* Help protect Diagnostics                  */
                                /* from asl_init()/asl_term() phase errors   */
    fprintf(stderr, MSGSTR(ERR_1820_021, M_ERR_1820_021));
    return ASL_FAIL;
}

WCLEAR(stdscr);
stdscr->_csbp = NORMAL;                 /* prevents inverse video spillage   */
                                        /* on WREFRESH()                     */
ASL_TRACER("asl_clear_buffer_screen: did stdscr->_csbp = NORMAL");
if ( refresh_flag == TRUE )
    TOUCHWIN(stdscr);
    WREFRESH(stdscr);
return ASL_OK;

} /* end asl_clear_buffer_screen */


/*---------------------------------------------------------------------------*/
/*      ASL_NOTE                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Presents popup messages and/or logs messages depending on
 *      specified status and record values.
 *      Input is given in style of printf().
 *      Attempts to detect output results that overflow internal buffers.
 *
 * RETURNS:
 *      ASL_OK          -- popup displayed and exited normally.
 *      ASL_FAIL        -- an error occured.
 *      ASL_CANCEL      -- user pressed cancel key.
 *      ASL_EXIT        -- user pressed exit key.
 *      ASL_ENTER       -- user pressed enter key.
 */

/*VARARGS*/
ASL_RC asl_note(
    ASL_STATUS_CODE     status,         /* error/message status              */
    ASL_RECORD_CODE     record,         /* where to record note              */
    const char          *format,        /* printf() style format           */
    ...
)
{
va_list ap;                     /* printf() style variable argument list   */
ASL_RC  rc;                     /* our return code                           */

va_start(ap, format);
rc = (ASL_RC) asl_vnote(status, record, format, ap);
va_end(ap);
return rc;
} /* end asl_note */


/*---------------------------------------------------------------------------*/
/*      ASL_VNOTE                                                            */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Presents popup messages and/or logs messages depending on
 *      specified status and record values.
 *      Input is given in style of printf().
 *      Attempts to detect output results that overflow internal buffers.
 *
 * RETURNS:
 *      ASL_OK          -- popup displayed and exited normally.
 *      ASL_FAIL        -- an error occured.
 *      ASL_CANCEL      -- user pressed cancel key.
 *      ASL_EXIT        -- user pressed exit key.
 *      ASL_ENTER       -- user pressed enter key.
 */
ASL_RC asl_vnote (
    ASL_STATUS_CODE     status,         /* error/message status              */
    ASL_RECORD_CODE     record,         /* where to record note              */
    const char          *format,        /* printf() style format           */
    va_list             arg             /* vsprint() style argument list     */
)
{
int i,j,k;
ASL_RC  ret = ASL_OK;                   /* return code                       */
                                        /* want this to be highest on stack  */
                                        /* in case of overflow               */
char    line_buf[ASL_MSG_MAX_STRLEN + 1];

if (asl_init_done != TRUE && status == ASL_INFORM_MSG) {
    fprintf(stderr, MSGSTR(ERR_1820_017, M_ERR_1820_017));
    return ASL_FAIL;
}
line_buf[0] = '\0';
line_buf[ASL_MSG_MAX_STRLEN] = '\0';
                /* will be overwritten by any non-null output char           */
                /* if the "printed" string is too long                       */
                /* (assuming this printf output is indeed printable)       */
                /* NOTE: can't use normal return value, which is display     */
                /* character count, not byte count!                          */
if (status != ASL_NO_MSG) {
    vsprintf(line_buf, format, arg);  /* all %-initiated printf-style    */
                                        /* formatting done here              */
    if (line_buf[ASL_MSG_MAX_STRLEN] != '\0') {
        line_buf[200] = '\0';           /* show first few lines,             */
                                        /* in case its not all junk          */
        asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                 MSGSTR(ERR_1820_016, M_ERR_1820_016));
    }
    if (asl_init_done == TRUE ) {
        if (status == ASL_SYS_ERR_MSG && asl_log_file != (FILE *) NULL) {
            ret = asl_note(ASL_ERR_MSG, ASL_NO_LOG,
                     MSGSTR(ERR_1820_037, M_ERR_1820_037));
        } else {
            ret = asl_msg(status, line_buf);
        }
    } else {
        if (status == ASL_ERR_MSG || status == ASL_SYS_ERR_MSG) {
            fprintf(stderr, "\n\n    **** %s ****\n\n",
                      MSGSTR(MSG_ERROR, M_MSG_ERROR));
            fprintf(stderr, "\n%s\n", line_buf);
            fflush(stderr);
        } else {
            fprintf(stdout, "\n%s\n", line_buf);
            fflush(stdout);
        }
    }
}
if (asl_log_file == (FILE *) NULL) {
    return ret;
}
if (        (record == ASL_LOG)
        ||  (record == ASL_TRACE   && asl_trace_log   == TRUE)
        ||  (record == ASL_VERBOSE && asl_verbose_log == TRUE)
        ||  (record == ASL_DEBUG   && asl_debug_log   == TRUE) ) {
    char *temp_name;
    FILE *fdtemp;
    if (status == ASL_ERR_MSG || status == ASL_SYS_ERR_MSG)
        fprintf(asl_log_file, "\n\n    **** %s ****\n\n",
                  MSGSTR(MSG_ERROR, M_MSG_ERROR));

    /* we were core dumping when the output was >  ASL_MSG_MAX_STRLEN    */
    /* so now we first write output to a file, then read it back in,     */
    /* then fold it as necessary.				 	 */
    if ((temp_name = (char *)tempnam("/tmp", NULL)) == NULL) {
           asl_note (ASL_ERR_MSG, ASL_LOG, "Unable to create temp file.\n");
           return ASL_FAIL;
        }
    if ((fdtemp = fopen(temp_name, "w")) == NULL) {
           asl_note (ASL_ERR_MSG, ASL_LOG, "Unable to open temp file.\n");
           return ASL_FAIL;
        }
    vfprintf(fdtemp, format, arg);
    fprintf (fdtemp, "\n");
    fclose    (fdtemp);

    /*vsprintf(line_buf, format, arg);   all %-initiated printf-style    */

    if ((fdtemp = fopen(temp_name, "r")) == NULL) {	/* open for read */
           asl_note (ASL_ERR_MSG, ASL_LOG, "Unable to open temp file.\n");
           return ASL_FAIL;
        }
    while (fgets (line_buf, ASL_MSG_MAX_STRLEN, fdtemp))
    {
                                      /* formatting done here            */
      /* Defect 104967:  Checks to see if line is longer than 1023.  If it */
      /* is than insert a new line before printing the rest of the line.   */
        i=0; j=0;
        while (line_buf[i])
        {
          j++;
          if (line_buf[i] == '\n')
             j=0;
          k=j % 1023;
          if(!(j%1023) && line_buf[i] != '\n')
             fprintf(asl_log_file,"\n");
          fprintf(asl_log_file,"%c",line_buf[i]);
          i++;
        } /* while line_buf */
    } /* while fgers */
    /* vfprintf(asl_log_file, format, arg); */
    fclose (fdtemp);
    unlink (temp_name);
    fprintf (asl_log_file, "\n");
    fflush    (asl_log_file);
}
return ret;

} /* end asl_vnote */


/*-----------------------------------------------------------------------*/
/*      ASL_INIT                                                         */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      If there is a problem with terminfo, an attempt is made to
 *      return error codes that provide more specific information.
 *      Starts up libcur and sets desired tty modes.
 *      HFT KSR-locator, block cursor, and PFkey initialization is done.
 *
 * RETURNS:
 *      ASL_OK                  -- curses initialization succeeded.
 *      ASL_ERR_NO_SUCH_TERM    -- specified TERM entry does not exist.
 *      ASL_ERR_TERMINFO_GET    -- terminfo get failed.
 *      ASL_ERR_NO_TERM         -- TERM entry missing.
 *      ASL_ERR_INITSCR         -- initscr() failed.
 *      ASL_ERR_SCREEN_SIZE     -- screen/window size less than minimum.
 */

ASL_RC  asl_init (log_file, trace_log, verbose_log, debug_log,
                  filter_flag, gen_help)
    FILE        *log_file;      /* for logging screens, if not NULL          */
    int         trace_log;      /* TRUE = trace mode info to log file        */
    int         verbose_log;    /* TRUE = verbose mode info log file         */
    int         debug_log;      /* TRUE = add debug info to log file         */
    int         filter_flag;    /* TRUE = OK if no terminal on stdout        */
                                /*        and don't flush stdin              */
    ASL_RC      (*gen_help)();  /* routine called for general help           */
{

int     rc = ASL_OK;            /* return code                               */
char    *term_type;             /* value of TERM                             */

#ifndef _ASL41
asl_catd = catopen("asl.cat", 0);
#else
asl_catd = catopen("asl.cat", NL_CAT_LOCALE);
#endif

asl_gen_help    = gen_help;
asl_log_file    = log_file;     /* optional log file for print               */
                                /* screen and error logging                  */
asl_trace_log   = trace_log;
asl_verbose_log = verbose_log;
asl_debug_log   = debug_log;
if (asl_log_file == NULL && asl_debug_log) {
    asl_log_file = stderr;      /* so debug information can always be safely */
                                /* written out                               */
}

extended(FALSE);                /* turn extended char for getch() off        */

asl_filter_flag = filter_flag;
#if ! defined(_IBM5A)
do_cursor = FALSE;              /* suppress initscr() cursor reset           */
#endif
if (asl_init_done == TRUE)      /* So diagnostics doesn't need to            */
                                /* track our state                           */
                                /* (also so log_file can be reset, etc.)     */
    return ASL_OK;

        /* ---- check to see that command is executed from console ----      */
        /* N.B. setupterm() is a non-libcur routine; it may not necessarily  */
        /* reflect what libcur's initscr() does (i.e., "termcap" stuff).     */

if (initscr() == (WINDOW *) ERR) {
    setupterm(  0,              /* use TERM                                  */
                1,              /* stdout                                    */
                &rc);           /* return with status                        */
    if (rc == 0)
        return ASL_ERR_NO_SUCH_TERM;
    if (rc == -1)
        return ASL_ERR_TERMINFO_GET;
    return ASL_ERR_INITSCR;
}
term_type = getenv("TERM");
                                /* assumes stdout is used for output         */
if ( ! isatty(1) && ! filter_flag ) {
    rc = ASL_ERR_NO_TERM;       /* although not connected to a terminal,     */
                                /* but ASL could still be used in batch mode */
} else if ((COLS < ASL_MIN_COLS) || (LINES < ASL_MIN_LINES)) {
    rc = ASL_ERR_SCREEN_SIZE;   /* doesn't meet minimum screen requirements  */
} else if (strcmp(term_type, "dumb") == 0) {
    rc = ASL_ERR_TERM_TYPE;     /* unsupported terminal type                 */
} else {                        /* everything looks good once we're here     */
    asl_reset();                /* resets, but doesn't alter text on screen  */
    asl_setup();
    asl_init_done = TRUE;
                                /* fill for invisible fields                 */
    memset(background, '*', ASL_MAX_VALUE_SIZE);
    background[ASL_MAX_VALUE_SIZE] = '\0';
/*
    if (asl_catd == (nl_catd) -1) {
        asl_note(ASL_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_034, M_ERR_1820_034));
    }
*/
#if ! defined(_IBM5A)
    if (signal(SIGCONT, SIG_IGN) == SIG_DFL) {  /* job control shell?        */
        signal(SIGCONT, asl_signal);            /* do refresh() as needed    */
    }
    if(getenv("ESCDELAY") == NULL)
       putenv("ESCDELAY=5000");    /* libcur Esc read loop counter for TCP/IP*/
#endif
    ASL_TRACER("> asl_init");
    return ASL_OK;
}
                                /* an error has occured since initscr(),     */
                                /* clean up and quit                         */
ASL_CURSES_RC((int) ENDWIN());  /* undo initscr()                            */
asl_init_done = TRUE;
asl_endwin_done = TRUE;         /* so asl_quit() will become a no-op         */
                                /* if called                                 */
asl_restore();                  /* "normalize" terminal                      */
return rc;

}  /* end asl_init */


/*---------------------------------------------------------------------------*/
/*      ASL_RESET                                                            */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      (Re)sets terminal attributes.
 *      Also:
 *      initialize curses/HFT locator/PFkey stuff,
 *      make keyboard input characters available before '\n',
 *      turn off keyboard character echoing,
 *      translate sequence characters into integers defined in cur02.h.
 *
 * RETURNS:
 *      None.
 */

void asl_reset()
{

                                        /* set up "ASL" terminal I/O modes   */
crmode();

#if ! defined(_IBM5A)
nonl();
#endif
noecho ();
asl_normal_mode = TRUE;
_tty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
Stty(_tty_ch, &_tty);                   /* turn off echoing                  */
keypad (TRUE);
} /* end asl_reset */


/*---------------------------------------------------------------------------*/
/*      ASL_SETUP                                                            */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Initialize key maps.
 *
 * RETURNS:
 *      None.
 */

static void asl_setup()
{
/* ---- Ordering of items below correspond to definitions of ASL_*           */
/* ---- change with care! ----                                               */

asl_key_map[ASL_CANCEL].return_code     = ASL_CANCEL;
if (K10 == NULL)                /* check for presence of function key        */
    asl_key_map[ASL_CANCEL].label       = MSGSTR(MSG_EXIT_ESC, M_MSG_EXIT_ESC);
else
    asl_key_map[ASL_CANCEL].label       = MSGSTR(MSG_EXIT, M_MSG_EXIT);

asl_key_map[ASL_HELP].return_code       = ASL_HELP;
if (K1 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_HELP].label         = MSGSTR(MSG_HELP_ESC, M_MSG_HELP_ESC);
else
    asl_key_map[ASL_HELP].label         = MSGSTR(MSG_HELP, M_MSG_HELP);

asl_key_map[ASL_REDRAW].return_code     = ASL_REDRAW;
if (K2 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_REDRAW].label       = MSGSTR(MSG_REFRESH_ESC, M_MSG_REFRESH_ESC);
else
    asl_key_map[ASL_REDRAW].label       = MSGSTR(MSG_REFRESH, M_MSG_REFRESH);

asl_key_map[ASL_EXIT].return_code       = ASL_EXIT;
if (K3 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_EXIT].label         = MSGSTR(MSG_CANCEL_ESC, M_MSG_CANCEL_ESC);
else
    asl_key_map[ASL_EXIT].label         = MSGSTR(MSG_CANCEL, M_MSG_CANCEL);

asl_key_map[ASL_LIST].return_code       = ASL_LIST;
if (K4 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_LIST].label         = MSGSTR(MSG_LIST_ESC, M_MSG_LIST_ESC);
else
    asl_key_map[ASL_LIST].label         = MSGSTR(MSG_LIST, M_MSG_LIST);

asl_key_map[ASL_DEFAULT].return_code    = ASL_DEFAULT;
if (K5 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_DEFAULT].label      = MSGSTR(MSG_UNDO_ESC, M_MSG_UNDO_ESC);
else
    asl_key_map[ASL_DEFAULT].label      = MSGSTR(MSG_UNDO, M_MSG_UNDO);

asl_key_map[ASL_COMMAND].return_code    = ASL_COMMAND;
if (K6 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_COMMAND].label      = MSGSTR(MSG_COMMAND_ESC, M_MSG_COMMAND_ESC);
else
    asl_key_map[ASL_COMMAND].label      = MSGSTR(MSG_COMMAND, M_MSG_COMMAND);

asl_key_map[ASL_COMMIT].return_code     = ASL_COMMIT;
if (K7 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_COMMIT].label       = MSGSTR(MSG_COMMIT_ESC, M_MSG_COMMIT_ESC);
else
    asl_key_map[ASL_COMMIT].label       = MSGSTR(MSG_COMMIT, M_MSG_COMMIT);

asl_key_map[ASL_PRINT].return_code      = ASL_PRINT;
if (K8 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_PRINT].label        = MSGSTR(MSG_IMAGE_ESC, M_MSG_IMAGE_ESC);
else
    asl_key_map[ASL_PRINT].label        = MSGSTR(MSG_IMAGE, M_MSG_IMAGE);

asl_key_map[ASL_SHELL].return_code      = ASL_SHELL;
if (K9 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_SHELL].label        = MSGSTR(MSG_SHELL_ESC, M_MSG_SHELL_ESC);
else
    asl_key_map[ASL_SHELL].label        = MSGSTR(MSG_SHELL, M_MSG_SHELL);


asl_key_map[ASL_BACK].return_code       = ASL_BACK;
if (K10 == NULL)                /* check for presence of function key        */
    asl_key_map[ASL_BACK].label         = MSGSTR(MSG_EXIT_ESC, M_MSG_EXIT_ESC);
else
    asl_key_map[ASL_BACK].label         = MSGSTR(MSG_EXIT, M_MSG_EXIT);

asl_key_map[ASL_EDIT].return_code       = ASL_EDIT;
if (K7 == NULL)                 /* check for presence of function key        */
    asl_key_map[ASL_EDIT].label         = MSGSTR(MSG_EDIT_ESC, M_MSG_EDIT_ESC);
else
    asl_key_map[ASL_EDIT].label         = MSGSTR(MSG_EDIT, M_MSG_EDIT);

asl_key_map[ASL_ENTER].return_code      = ASL_ENTER;
asl_key_map[ASL_ENTER].label            = MSGSTR(MSG_ENTER, M_MSG_ENTER);

asl_key_map[ASL_ENTER_DO].return_code   = ASL_ENTER;
asl_key_map[ASL_ENTER_DO].label         = MSGSTR(MSG_ENTER_DO, M_MSG_ENTER_DO);

asl_key_map[ASL_REAL_COMMIT].return_code = ASL_COMMIT;
asl_key_map[ASL_REAL_COMMIT].label      = MSGSTR(MSG_REAL_COMMIT, M_MSG_REAL_COMMIT);

asl_key_map[ASL_FIND].return_code       = ASL_FIND;
asl_key_map[ASL_FIND].label             = MSGSTR(MSG_FIND, M_MSG_FIND);

asl_key_map[ASL_FIND_NEXT].return_code  = ASL_FIND_NEXT;
asl_key_map[ASL_FIND_NEXT].label   = MSGSTR(MSG_FIND_NEXT, M_MSG_FIND_NEXT);

                                                /* --- DSMIT specific -------*/
asl_key_map[ASL_MACHINE].return_code    = ASL_MACHINE;
asl_key_map[ASL_MACHINE].label          = MSGSTR(MSG_MACHINE, M_MSG_MACHINE);

asl_key_map[ASL_FIELD].return_code      = ASL_FIELD;
asl_key_map[ASL_FIELD].label            = MSGSTR(MSG_FIELD, M_MSG_FIELD);

asl_key_map[ASL_COLLECTIVE].return_code = ASL_COLLECTIVE;
asl_key_map[ASL_COLLECTIVE].label     = MSGSTR(MSG_COLLECTIVE,M_MSG_COLLECTIVE);

                                      /* added in dsmit-scheduler            */
asl_key_map[ASL_SCHEDULER].return_code = ASL_SCHEDULER;
asl_key_map[ASL_SCHEDULER].label     = MSGSTR(MSG_SCHEDULER,M_MSG_SCHEDULER);

} /* end asl_setup */


/*---------------------------------------------------------------------------*/
/*      ASL_QUIT                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Clears screen and close curses windowing.
 *
 * RETURNS:
 *      ASL_OK          -- curses closed down.
 *      ASL_FAIL        -- asl_quit() called previously.
 */

ASL_RC asl_quit()

{
ASL_TRACER("> asl_quit");
if (asl_endwin_done == TRUE)
    return ASL_FAIL;
asl_cursor(TRUE);                       /* insure block cursor is turned on  */
WMOVE(stdscr, 0, 0);
asl_hilite_off (stdscr, 0, 0, 10, 0);
stdscr->_csbp = NORMAL;                 /* prevent inverse video spillage    */
curscr->_csbp = NORMAL;                 /* prevent prompt hiliting           */
WCLEAR(stdscr);
WMOVE(stdscr, LINES - 1, 0);
WREFRESH(stdscr);
ASL_CURSES_RC((int) ENDWIN());
asl_endwin_done = TRUE;                 /* defend against mistakes by caller */
asl_restore();
    catclose(asl_catd);

return ASL_OK;

} /* end asl_quit */


/*---------------------------------------------------------------------------*/
/*      ASL_RESTORE                                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Restores desired non-libcur tty modes.
 *      Complement of asl_reset.
 *
 * RETURNS:
 *      None.
 */

void asl_restore()
{
                                        /* restore "normal" I/O modes        */
                                        /* set up "ASL" terminal I/O modes   */
/* raw(); ?? */
/*nocrmode();*/
crmode();
#if ! defined(_IBM5A)
nl();
#endif
echo ();
asl_normal_mode = FALSE;
_tty.c_lflag |= (ECHO|ECHOE|ECHOK|ECHONL);
Stty(_tty_ch, &_tty);                   /* turn on echoing                   */
keypad (FALSE);

RESETTY(FALSE);

/* Old HFT stuff was here; save routine until LIBCUR mods ready              */

} /* end asl_restore */


/*---------------------------------------------------------------------------*/
/*      ASL_STD_EDIT_KEYS                                                    */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      This is a simple one line editor that
 *      takes the current input character
 *      and appropriately updates the specified editing
 *      field in the specified window.
 *
 * RETURNS:
 *      ASL_OK      -- edit changed field.
 *      ASL_FAIL    -- edit had no effect on field.
 */

static ASL_RC asl_std_edit_keys (c, data_width, entry_type, cursor_column, cur_character, type)
    wchar_t     c;              /* the current input character               */
    int         data_width;     /* the (MAXIMUM) editing field width         */
    int         entry_type;     /* the editing field data type               */
    int         *cursor_column; /* the current cursor column                 */
    int         *cur_character; /* pointer to current character. current char*/
                                /* and cursor_column might not be equal since*/
                                /* a character may be more than 1 byte long  */
    int         type;           /* the type of input                         */
{
int     delta = FALSE;          /* FALSE means no change has been made       */
                                /* to editing field                          */
int     i;                      /* index over horizonatal character          */
                                /* position                                  */
int     char_ok = FALSE;        /* TRUE means c is insert compatible         */
                                /* with data_type                            */
int     len = 0;                /* length returned bo wcwidth()              */


ASL_TRACER("> asl_std_edit_keys");
switch (type) {

    case KEY_LOCESC:                    /* below threshold locator input     */
            break;                      /* quietly ignore it                 */
    case KEY_LEFT:                      /* ---- CURSOR KEY LEFT MOVEMENT     */
            if (*cursor_column == 0) {
                asl_beep();             /* can't cursor back up over         */
                                        /* beginning of field                */
            } else {
                (*cur_character) --;
                (*cursor_column) -= (((len =
                   wcwidth(asl_edit_data[*cur_character])) == -1) ? 1 : len);
            }
            break;

    case KEY_RIGHT:                     /* ---- CURSOR KEY RIGHT MOVEMENT    */
            if (*cursor_column >= (data_width - 1)) {
                asl_beep();
            } else {
                if (asl_edit_data[*cur_character] == '\0') {
                    asl_beep();         /* not allowed to cross              */
                                        /* rightmost insert point            */
                    break;
                }
                (*cursor_column) += (((len =
                    wcwidth(asl_edit_data[*cur_character])) == -1) ? 1 : len);
                (*cur_character) ++;
            }
            break;

    case KEY_BACKSPACE:                 /* ---- BACKSPACE pressed            */
            if (entry_type == ASL_NO_ENTRY) {
                asl_beep();             /* not allowed to modify field       */
                break;
            }
            if (*cursor_column == 0) {
                asl_beep();             /* can't back up over beginning      */
                                        /* of field                          */
            } else {
                (*cur_character) --;
                (*cursor_column) -= (((len =
                    wcwidth(asl_edit_data[*cur_character])) == -1) ? 1 : len);
                delta = TRUE;
                if (asl_edit_data[*cur_character + 1] == '\0')
                    asl_edit_data[*cur_character] = '\0';
                else
                    asl_edit_data[*cur_character] = ' ';
            }
            break;

    case KEY_DC:                        /* ----- DELETE CHAR                 */
    case CTRL_X:
            if (entry_type == ASL_NO_ENTRY) {
                asl_beep();             /* not allowed to modify field       */
                break;
            }
            if (asl_edit_data[*cur_character] != '\0')
                wcscpy(&asl_edit_data[*cur_character],
                     &asl_edit_data[*cur_character + 1]);
            delta = TRUE;
            break;

    case CTRL_A:                        /* ---- Beginning of line key        */
            *cursor_column = 0;
            *cur_character = 0;
            break;

    case KEY_EOL:                       /* ---- End of line key pressed      */
    case CTRL_E:
            if (data_width == 0) {      /* no motion if no data              */
                *cursor_column = 0;
                *cur_character = 0;
            } else {
                /* modified to stop at trailing whitespace                   */
                /* (which is desirable for non-filled monster fields)        */
                i = *cur_character;
                                        /* find end of string                */
                while (asl_edit_data[i] != '\0')
                    i ++;
                if (i > 0)              /* if nonempty string                */
                    i --;               /* last char preceeding terminator   */
                                        /* find beginning of trailing        */
                                        /* whitespace                        */
                for ( ; i > 0 ; i -- ) {
                                        /* find end of trailing white space  */
                      /*if (asl_edit_data[i] != ' ' && asl_edit_data[i] != '\t')*/
                      if (!iswspace (asl_edit_data[i]))
                        break;
                }
                                        /* if not at end of field            */
                if (i < (data_width - 1)) {
                    if (i > 0)          /* and not at beginning of           */
                                        /* empty field                       */
                        i ++;           /* move over to next insert location */
                }
                *cursor_column = wcswidth (asl_edit_data, i);
                *cur_character = i;
                asl_edit_data[*cur_character + 1] = '\0';
            }
            break;

    case INSERT:                        /* ----- INSERT TOGGLE               */
    case CTRL_R:
            if (entry_type == ASL_NO_ENTRY) {
                asl_beep();             /* not allowed to modify field       */
                break;
            }
            if (insert_on)
                insert_on = FALSE;
            else
                insert_on = TRUE;
            break;

    case CTRL_K:                        /* ---- CTRL K: kill current field   */
            if (entry_type == ASL_NO_ENTRY) {
                asl_beep();             /* not allowed to modify field       */
                break;
            }
            asl_edit_data[*cur_character] = '\0';
            delta = TRUE;
            break;
                                        /* filter out obvious junk           */
    case ESC1: case KEY_F(1):
    case ESC2: case KEY_F(2):
    case ESC3: case KEY_F(3):
    case ESC4: case KEY_F(4):
    case ESC5: case KEY_F(5):
    case ESC6: case KEY_F(6):
    case ESC7: case KEY_F(7):
    case ESC8: case KEY_F(8):
    case ESC9: case KEY_F(9):
    case ESC0: case KEY_F(10):
    /* -- DSMIT specific --- */
    case ESC_M:
    case ESC_F:
/*    case ESC_R:  */
    case ESC_C:
    case ESC_S:
    /* --------------------- */
    case KEY_F(11):
    case KEY_F(12):
    case KEY_F(13):
    case KEY_F(14):
    case KEY_F(15):
    case KEY_F(16):
    case KEY_F(17):
    case KEY_F(18):
    case KEY_F(19):
    case KEY_F(20):
    case KEY_F(21):
    case KEY_F(22):
    case KEY_F(23):
    case KEY_F(24):
    case KEY_BREAK:           /* break - unreliable           */
    case KEY_DL:              /* delete line                  */
    case KEY_IL:              /* insert line                  */
    case KEY_EIC:             /* exit insert character mode   */
    case KEY_CLEAR:           /* clear screen                 */
    case KEY_EOS:             /* clear to end of screen       */
    case KEY_SF:              /* scroll forward toward end    */
    case KEY_SR:              /* scroll backward toward start */
    case KEY_STAB:            /* set tab stop                 */
    case KEY_CTAB:            /* clear tab stop               */
    case KEY_CATAB:           /* clear all tab stops          */
    case KEY_SRESET:          /* soft reset key - unreliable  */
    case KEY_RESET:           /* hard reset key - unreliable  */
    case KEY_PRINT:           /* print or copy                */
    case KEY_LL:              /* lower left (last line)       */
    case KEY_A1:              /* pad upper left               */
    case KEY_A3:              /* pad upper right              */
    case KEY_B2:              /* pad center                   */
    case KEY_C1:              /* pad lower left               */
    case KEY_C3:              /* pad lower right              */
    case KEY_DO:              /* DO key                       */
    case KEY_QUIT:            /* QUIT key                     */
    case KEY_CMD:             /* Command key                  */
    case KEY_PCMD:            /* Previous command key         */
    case KEY_NPN:             /* Next pane key                */
    case KEY_PPN:             /* previous pane key            */
    case KEY_CPN:             /* command pane key             */
    case KEY_END:             /* end key                      */
    case KEY_HLP:             /* help key                     */
    case KEY_SEL:             /* select key                   */
    case KEY_SCR:             /* scroll right key             */
    case KEY_SCL:             /* scroll left key              */
    case KEY_ACT:             /* action key                   */
    case KEY_SF1:             /* Special function key 1       */
    case KEY_SF2:             /* Special function key 2       */
    case KEY_SF3:             /* Special function key 3       */
    case KEY_SF4:             /* Special function key 4       */
    case KEY_SF5:             /* Special function key 5       */
    case KEY_SF6:             /* Special function key 6       */
    case KEY_SF7:             /* Special function key 7       */
    case KEY_SF8:             /* Special function key 8       */
    case KEY_SF9:             /* Special function key 9       */
    case KEY_SF10:            /* Special function key 10      */
    case KEY_UP:
    case KEY_DOWN:
            asl_beep();
            break;

    default:                            /* ---- DEFAULT                      */
            if (entry_type == ASL_NO_ENTRY) {
                asl_beep();             /* not allowed to modify field       */
                break;
            }
            if (*cursor_column >= data_width) {
                asl_beep();             /* can't insert at end of full field */
                break;
            }
            switch (entry_type) {       /* check char. against field type    */
                case ASL_TEXT_ENTRY:
                case ASL_RAW_TEXT_ENTRY:
                case ASL_INVISIBLE_ENTRY:
                        if ( iswprint(c) )
                            char_ok = TRUE;
                        break;
                case ASL_FILE_ENTRY:
                        if ( iswgraph(c) )
                            char_ok = TRUE;
                        break;
                case ASL_NUM_ENTRY:
                case ASL_SIGNED_NUM_ENTRY:
			 if ( iswdigit(c) ||
                            ((*cursor_column == 0) && (c == '-')) ||
                            ((*cursor_column == 0) && (c == '+')) )  
                            char_ok = TRUE;
                        break;
                case ASL_HEX_ENTRY:
                        if (iswxdigit(c))
                            char_ok = TRUE;
                        break;
                case ASL_NO_ENTRY:
                default:
                        break;
            } /* switch */

            if (char_ok == TRUE) {      /* is this fit to print ? */
                i = *cur_character;     /* where insert will be */
                if (insert_on) {
                                        /* find end of string */
                    while (asl_edit_data[i] != '\0' && (i < (data_width - 1)) )
                        i ++;
                    asl_edit_data[i + 1] = '\0';
                                        /* shift right one char */
                    while (i > *cur_character) {
                        asl_edit_data[i] = asl_edit_data[i - 1];
                        i --;
                    }
                    asl_edit_data[i] = c;       /* insert into empty slot    */
                    if (i < (data_width - 1))   /* don't overshoot end       */
                                        /* advance cursor past new char      */
                    {
                        (*cursor_column) += (((len =
                        wcwidth(asl_edit_data[*cur_character])) == -1)? 1:len);
                        (*cur_character) ++;
                    }
                } else {
                                        /* add char over previous one        */
                    if (asl_edit_data[i] == '\0')
                        asl_edit_data[i + 1] = '\0';
                                        /* add char over previous one        */
                    asl_edit_data[i] = c;
                                        /* don't overshoot end               */
                    if (i < (data_width - 1))
                                        /* advance cursor past new char      */
                    {
                        (*cursor_column) += (((len =
                        wcwidth (asl_edit_data[*cur_character]))== -1) ? 1:len);
                        (*cur_character) ++;
                    }
                    else if (i > (data_width - 1))
                        asl_beep();
                }
                delta = TRUE;           /* insert completed                  */
                break;
            } else {
                asl_beep();             /* char not permitted in edit field  */
            }
            break;

} /* switch */

ASL_TRACER("< asl_std_edit_keys");
if (delta) {                            /* return OK if changes were made,   */
                                        /* else "FAIL"                       */
    return (ASL_OK);
} else {
    return (ASL_FAIL);
}

} /* end asl_std_edit_keys */


/*---------------------------------------------------------------------------*/
/*      ASL_INPUT_FIELD_SIZE                                                 */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Determines the display length of an entry_field.
 *      This is determined by
 *      starting at the end of the string and scanning backwards,
 *      skipping tabs and spaces.
 *
 * RETURNS:
 *      Gives number of columns needed to display given field contents.
 */

static int asl_input_field_size (input_window, line, start_column, data_width)
    WINDOW      *input_window;  /* the window containing the input field     */
    int         line;           /* the line containing the input field       */
    int         start_column;   /* the starting column of the input field    */
    int         data_width;     /* max. num. of chars. in the input field    */
{
int     col;                    /* input window column index                 */
int     c;                      /* character from input window               */
                                /* room if input is 2-byte chars and         */
                                /* also for '\0'?                            */
for (col = start_column + data_width - 1 ;
        col >= start_column ;
        col-- ) {
    WMOVE(input_window, line, col);
    c = winch (input_window);           /* grab next character from window   */
    if (c != ' ' && c != '\t')          /* find end of trailing white space  */
        break;
}
return (col - start_column + 1);

} /* end asl_input_field_size */


/*---------------------------------------------------------------------------*/
/*      ASL_MSG                                                              */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Called by asl_note(); sets up appropriate data structures needed to
 *      call asl_screen.  Uses the status parameter to determine
 *      which screen_code and default title/instruction/etc. entries to use.
 *
 * RETURNS:
 *      ASL_OK          -- message displayed.
 *      ASL_EXIT        -- user response in popup.
 */

static ASL_RC asl_msg (status, msg_text)
    ASL_STATUS_CODE     status;         /* wait for user acknowledgement     */
    char                *msg_text;      /* text to be displayed              */
{
ASL_RC          ret;                    /* return code from asl_screen()     */
ASL_SCR_TYPE    win_type;               /* overall screen-related            */
                                        /* information for help              */
ASL_SCR_INFO    win_info[3];            /* line or entry-specific screen     */
                                        /* information for help              */
ASL_SCR_TYPE    scr_type;               /* overall screen-related info.      */
ASL_SCR_INFO    scr_info[3];            /* line or entry-specific            */
                                        /* screen information                */

win_info[0].text        = EMPTY_STRING;
win_info[1].text        = MSGSTR(  MSG_HELP_TEXT_MISSING,
                                 M_MSG_HELP_TEXT_MISSING);
win_info[2].text        = MSGSTR(  MSG_ENTER_TO_RETURN,
                                 M_MSG_ENTER_TO_RETURN);
win_type.cur_win_index  = 0;
win_type.cur_win_offset = 0;
win_type.cur_index      = 1;
win_type.max_index      = 2;
win_type.screen_code    = ASL_GENERAL_HELP_SC;

scr_info[0].text        = EMPTY_STRING; /* use asl_screen() defaults         */
                                        /* if not changed below              */
scr_info[1].text        = msg_text;
scr_info[2].text        = EMPTY_STRING; /* use asl_screen() defaults         */
                                        /* if not changed below              */
scr_type.cur_win_index  = 0;
scr_type.cur_win_offset = 0;
scr_type.cur_index      = 1;
scr_type.max_index      = 2;

switch (status) {
    case ASL_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_016, M_MSG_016);
        scr_info[2].text        = MSGSTR(MSG_019, M_MSG_019);
        break;
    case ASL_MSG_CONTINUE:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_016, M_MSG_016);
        scr_info[2].text        = MSGSTR(MSG_050, M_MSG_050);
        break;
    case ASL_ERR_MSG:
    case ASL_SYS_ERR_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_018, M_MSG_018);
        scr_info[2].text        = MSGSTR(MSG_019, M_MSG_019);
        break;
    case ASL_CANCEL_MSG:
        scr_type.screen_code    = ASL_CANCEL_SC;
        scr_info[0].text        = MSGSTR(MSG_020, M_MSG_020);
        scr_info[2].text        = MSGSTR(MSG_021, M_MSG_021);
        break;
    case ASL_EXIT_MSG:
        scr_type.screen_code    = ASL_EXIT_SC;
        scr_info[0].text        = MSGSTR(MSG_022, M_MSG_022);
        scr_info[2].text        = MSGSTR(MSG_023, M_MSG_023);
        break;
    case ASL_SHELL_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_024, M_MSG_024);
        scr_info[1].text        = EMPTY_STRING;
        scr_info[2].text        = MSGSTR(MSG_025, M_MSG_025);
        break;
    case ASL_PRINT_MSG:
        scr_type.screen_code    = ASL_PRINT_LIST_SC;
        scr_info[0].text        = MSGSTR(MSG_026, M_MSG_026);
        scr_info[2].text        = MSGSTR(MSG_014, M_MSG_014);
        break;
    case ASL_COMMAND_MSG:
        scr_type.screen_code    = ASL_COMMAND_SC;
        break;
    case ASL_COMMIT_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_028, M_MSG_028);
        scr_info[1].text        = EMPTY_STRING;
        scr_info[2].text        = MSGSTR(MSG_029, M_MSG_029);
        break;
    case ASL_DSMIT_INTERRUPT_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_054, M_MSG_054);
        scr_info[2].text        = MSGSTR(MSG_055, M_MSG_055);
        break;
    case ASL_DSMIT_WHEEL:
        scr_type.screen_code    = ASL_DSMIT_WHEEL_SC;
        break;
    case ASL_INFORM_MSG:
        scr_type.screen_code    = ASL_INFORM_MSG_SC;
        break;
    case ASL_HELP_MSG:
        scr_type.screen_code    = ASL_ACK_MSG_SC;
        scr_info[0].text        = MSGSTR(MSG_030, M_MSG_030);
        break;
    default:
        return asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                        MSGSTR(ERR_1820_018, M_ERR_1820_018),
                scr_type.screen_code);
} /* switch */


ret = asl_screen(&scr_type, scr_info);
while (ret == ASL_HELP) {
    switch (status) {
            case ASL_MSG:
                    ret = (*asl_gen_help)();
                    break;
            case ASL_ERR_MSG:
            case ASL_SYS_ERR_MSG:
                    ret = (*asl_gen_help)();
                    break;
            case ASL_SHELL_MSG:
                    win_info[1].text = MSGSTR(MSG_033, M_MSG_033);
                    ret = asl_screen(&win_type, win_info);
                    break;
            case ASL_COMMIT_MSG:
                    win_info[1].text = MSGSTR(MSG_034, M_MSG_034);
                    ret = asl_screen(&win_type, win_info);
                    break;
            case ASL_CANCEL_MSG:
            case ASL_EXIT_MSG:
            case ASL_PRINT_MSG:
            case ASL_COMMAND_MSG:
            case ASL_INFORM_MSG:
            case ASL_HELP_MSG:
                    return ret;
                    break;
            default:
                    break;
    }
    if (ret != ASL_EXIT) {
        ret = asl_screen(&scr_type, scr_info);
        if (ret == ASL_ENTER || ret == ASL_CANCEL) {
            ret = ASL_OK;
        }
    }
}
return ret;

}  /* end asl_msg */


/*-----------------------------------------------------------------------*/
/*      ASL_SHELL                                                        */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Performs shell escape and return.
 *      The current environment is checked to see if the user has
 *      specified a preferred shell, which is then used in the subsequent
 *      exec(); otherwise ksh is used.  Handles resetting of screen
 *      attributes both prior to and after the fork()/exec() so that
 *      libcur is out of the way of the child process, which inherits
 *      our I/O descriptors, and that things are returned to normal
 *      afterwords.
 *
 * RETURNS:
 *      ASL_OK          -- normal return from shell escape.
 *      ASL_EXIT        -- invoked for asl_shell popup by user.
 */

static ASL_RC asl_shell()
{

int     pid;                            /* child's process identifier        */
char    *shell;                         /* will be set to $SHELL             */
int     cur_y;                          /* current y coordinate              */
int     cur_x;                          /* current x coordinate              */
ASL_RC  ret = ASL_OK;                   /* return code                       */
int     exit_status;                    /* return status from shell process  */
void            (*istat)(int);          /* previous setting SIGINT           */
void            (*qstat)(int);          /* previous setting SIGQUIT          */
void            (*cstat)(int);          /* previous setting SIGCLD           */

ASL_TRACER("> asl_shell");
getyx(curscr, (cur_y), (cur_x));        /* save cursor position              */
asl_cursor(TRUE);                       /* insure block cursor is turned on  */
asl_save_tty();                         /* save current tty state            */

if ( (pid = fork()) == 0) {             /* ---- now child ----               */
    asl_restore_tty();                  /* reset previous tty mode           */
    printf(MSGSTR(MSG_NEW_SHELL, M_MSG_NEW_SHELL));
    shell = getenv("SHELL");            /* get preferred shell               */
    /* commented out for defe 126482  */
    /*putenv("ENV=");                      avoid side effects from           */
                                        /* users who                         */
                                        /* have screen resetting             */
                                        /* commands, etc. here               */
    if (shell == NULL)                  /* if SHELL was not specified        */
                                        /* in environment                    */
        shell = "/usr/bin/ksh";         /* start our default                 */
   

    execl (shell, shell, "-i", 0);      /* start interactive $SHELL          */
    printf( MSGSTR(ERR_1820_020, M_ERR_1820_020));
    getchar();
    fflush(NULL);
    _exit(-1);                          /* shell didn't start up, zap child  */
}
                                        /* -------- now parent again         */
                                        /* ignore signals intended for child */
istat = (void (*)()) signal(SIGINT,  SIG_IGN);
qstat = (void (*)()) signal(SIGQUIT, SIG_IGN);
cstat = (void (*)()) signal(SIGCLD,  SIG_DFL);
                                        /* wait until child's shell exits    */
pid = wait(&exit_status);

#ifdef _DSMIT
#ifndef _NO_NETLS
while (pid == -1 && errno == 4)		/* SIGALRM causes wait to crash, so  */
{					/* get license, and restart wait     */
     heartbeat();
     pid=wait(&exit_status);
}
#endif
#endif

asl_note(ASL_NO_MSG, ASL_TRACE, MSGSTR(MSG_035, M_MSG_035), pid, errno);
signal(SIGINT,  istat);
signal(SIGQUIT, qstat);
signal(SIGCLD,  cstat);
asl_note (ASL_NO_MSG, ASL_TRACE, MSGSTR(MSG_037, M_MSG_037), exit_status);

/* Defect 177714.						  */
/* if subshell died due to a signal, we must exit now             */
/* we can't even display a popup.  but at least leave a log entry */
if ((exit_status & 0xFF) != 0) {
  asl_note (ASL_NO_MSG, ASL_LOG,
            MSGSTR(ERR_1820_039, M_ERR_1820_039),
            (getenv("SHELL") ? getenv("SHELL") : "/usr/bin/ksh"));

                                /* message to console as well. */
  fprintf(stderr, MSGSTR(ERR_1820_039, M_ERR_1820_039),
            (getenv("SHELL") ? getenv("SHELL") : "/usr/bin/ksh"));
  fflush(stderr);

  exit(exit_status);            /* shell bombed out */
}

asl_restore_tty_colors();               /* reset previous tty mode           */
curscr->_cury = cur_y;
curscr->_curx = cur_x;
TOUCHWIN(curscr);                   /* redraw previous screen            */
WREFRESH(curscr);                   /* redraw previous screen            */
if (pid == -1) {                        /* if fork failed ...                */
    ret = asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                    MSGSTR(ERR_1820_019, M_ERR_1820_019));
    if (ret == ASL_ENTER || ret == ASL_CANCEL) {
        ret = ASL_OK;
    }
}
ASL_TRACER("< asl_shell");
return ret;

} /* end asl_shell */


/*-----------------------------------------------------------------------*/
/*      ASL_EXECUTE                                                      */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Executes another program. Uses execv() system call.
 *
 *      Handles resetting of screen attributes
 *      both prior to and after the fork()/exec() so that
 *      libcur is out of the way of the child process, which inherits
 *      our I/O descriptors, and that things are returned
 *      to normal afterwords.
 *
 * RETURNS:
 *      ASL_OK          -- command executed.
 *      ASL_FAIL        -- error occured.
 */

ASL_RC asl_execute(command, options, exit_status)
char    *command;                       /* first  arg for execv              */
char    *options[];                     /* second arg for execv              */
int     *exit_status;                   /* exit status from wait             */
{

int     pid;                            /* child's process identifier        */
int     cur_y;                          /* current y coordinate              */
int     cur_x;                          /* current x coordinate              */
ASL_RC  ret = ASL_OK;                   /* return code                       */
int     savmask;
extern  int     errno, sys_nerr;
extern  char    *sys_errlist[];

ASL_TRACER("> asl_execute");
getyx(curscr, (cur_y), (cur_x));        /* save cursor position              */
asl_save_tty();                         /* save current tty state            */

if ( (pid = fork()) == 0) {             /* -------- now child                */
    RESETTY(FALSE);                     /* reset previous tty mode           */
                                        /* have screen resetting             */
                                        /* commands, etc. here               */
    execv(command, options);
                                        /* put perror here????             */
    printf(MSGSTR(ERR_1820_032, M_ERR_1820_032),
             sys_errlist[ (errno <= sys_nerr? errno:0) ], command);

    getchar();
    fflush(NULL);
    _exit(-1);                          /* unsuccessful execv(), zap child   */
}
                                        /* -------- now parent again         */
savmask = sigblock( 1<< (SIGINT-1) );
while ( -1 == wait(exit_status)  )
    ;
(void) sigsetmask(savmask);

RESETTY(FALSE);                         /* reset previous tty mode           */
CRESETTY(FALSE);
if (pid == -1) {                        /* if fork failed ...                */
    asl_note (ASL_SYS_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_031, M_ERR_1820_031),
              command);
    ret = ASL_FAIL;
}
ASL_TRACER("< asl_execute");
return ret;

} /* end asl_execute */


/*-----------------------------------------------------------------------*/
/*      ASL_HELP                                                         */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Determines what response to give to help key.
 *
 * RETURNS:
 *      ASL_OK          -- help display completed.
 *      ASL_HELP        -- need to get help from client.
 *      ASL_EXIT        -- invoked by user from displayed screen.
 */

static ASL_RC asl_help(screen_code, scr_type, scr_info)
    ASL_SCREEN_CODE     screen_code;    /* screen code for screen            */
                                        /* requesting help                   */
    ASL_SCR_TYPE        *scr_type;      /* overall screen-related            */
                                        /* information                       */
    ASL_SCR_INFO        scr_info[];     /* line or entry-specific            */
                                        /* screen information                */
{
ASL_SCR_TYPE    win_type;               /* overall screen-related info.      */
ASL_SCR_INFO    win_info[3];            /* line or entry-specific            */
                                        /* screen information                */
ASL_RC          ret;                    /* return code                       */

ASL_TRACER("> asl_help");
win_info[0].text        = EMPTY_STRING;
win_info[1].text        = MSGSTR(  MSG_HELP_TEXT_MISSING,
                                 M_MSG_HELP_TEXT_MISSING);
win_info[2].text        = MSGSTR(  MSG_ENTER_TO_RETURN,
                                 M_MSG_ENTER_TO_RETURN);
win_type.cur_win_index  = 0;
win_type.cur_win_offset = 0;
win_type.cur_index      = 1;
win_type.max_index      = 2;
win_type.screen_code    = ASL_GENERAL_HELP_SC;

if (asl_list_return_help) {
    if (    screen_code == ASL_SINGLE_LIST_SC
         || screen_code == ASL_MULTI_LIST_SC  ) {
        ASL_TRACER("< asl_help");
        return ASL_HELP;
    }
}
switch (screen_code) {
    case ASL_SINGLE_MENU_SC:
    case ASL_DSMIT_SINGLE_MENU_SC:
    case ASL_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_SC:
    case ASL_DSMIT_DIALOGUE_FIELD_SC:
    case ASL_INFORM_MSG_SC:
    case ASL_DIAG_DIALOGUE_HELP_SC:
    case ASL_DIAG_DIALOGUE_SC:
    case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
    case ASL_DIAG_ENTER_HELP_SC:
    case ASL_DIAG_LIST_COMMIT_HELP_SC:
                ASL_TRACER("< asl_help");
                return ASL_HELP;
    case ASL_CONTEXT_HELP_SC:
    case ASL_ACK_MSG_SC:
                ret = (*asl_gen_help)();
                if (ret == ASL_ENTER || ret == ASL_CANCEL)
                    ret = ASL_OK;
                ASL_TRACER("< asl_help");
                return ret;
    case ASL_OUTPUT_SC:
    case ASL_DSMIT_OUTPUT_SC:
                win_info[1].text        = MSGSTR(MSG_038, M_MSG_038);
                break;                          /* "local" help on tool      */
    case ASL_SINGLE_LIST_SC:
    case ASL_COLOR_LIST_SC:                     /* (no help key presently)   */
    case ASL_DIAG_DIALOGUE_LIST_SC:
                win_info[1].text        = MSGSTR(MSG_039, M_MSG_039);
                break;                          /* "local" help on tool      */
    case ASL_MULTI_LIST_SC:
                win_info[1].text        = MSGSTR(MSG_040, M_MSG_040);
                break;                          /* "local" help on tool      */
    case ASL_DIAG_OUTPUT_LEAVE_SC:
    case ASL_DIAG_NO_KEYS_ENTER_SC:
    case ASL_DIAG_KEYS_ENTER_SC:
                return ASL_OK;
    case ASL_EDIT_SC:
                win_info[1].text        = MSGSTR(MSG_041, M_MSG_041);
                break;
    case ASL_COMMAND_SC:
                win_info[1].text        = MSGSTR(MSG_042, M_MSG_042);
                break;
    case ASL_PRINT_LIST_SC:
                win_info[1].text        = MSGSTR(MSG_063, M_MSG_063);
                break;
    case ASL_EXIT_SC:
                win_info[1].text        = MSGSTR(MSG_044, M_MSG_044);
                break;
    case ASL_CANCEL_SC:
                win_info[1].text        = MSGSTR(MSG_045, M_MSG_045);
                break;
    default:
                ret = asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                                MSGSTR(ERR_1820_024, M_ERR_1820_024),
                                screen_code);
                if (ret == ASL_ENTER || ret == ASL_CANCEL)
                    ret = ASL_OK;
                return ret;
}
ret = asl_screen(&win_type, win_info);
if (ret == ASL_ENTER || ret == ASL_CANCEL)
    ret = ASL_OK;
ASL_TRACER("< asl_help");
return ret;

} /* end asl_help */


/*-----------------------------------------------------------------------*/
/*      ASL_COPY_VIEW                                                    */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Copies a block of text (and associated attributes)
 *      from the backing window of a simulated viewport
 *      to the main display window.
 *
 * RETURNS:
 *      None.
 */

static void asl_copy_view(s)
    ASL_SCREEN  *s;             /* main screen data structure                */
{
int     x_cur;                  /* screen window absolute x coordinate       */
int     y_cur;                  /* scroll area view-relative y coord.        */
int     col_ptr = 0;            /* ptr into the # of display column          */
int     len = 0;                /* length returned by wcwidth()              */

ASL_TRACER("> asl_copy_view");
for (y_cur = 0; y_cur < s-> y_scroll_view_size; y_cur ++ ) {
    if ( WMOVE(s-> main_win, y_cur + s-> y_scroll, 0) != ERR ) {
        for (x_cur = 0; x_cur < s-> main_win_cols; x_cur += col_ptr) {
                                        /* copies char AND attributes        */
            WADDCH( s-> main_win,
                   (s-> scroll_data)->_y[y_cur + s-> y_view_offset][x_cur + s->x_view_cur]);
            (s-> main_win)->_y[s-> y_scroll + y_cur][x_cur] =
                    (s-> scroll_data)->_y[y_cur + s-> y_view_offset][x_cur + s->x_view_cur];
            (s-> main_win)->_a[s-> y_scroll + y_cur][x_cur] =
                    (s-> scroll_data)->_a[y_cur + s-> y_view_offset][x_cur + s->x_view_cur];
            mark_change(s-> main_win, s-> y_scroll + y_cur, x_cur + s-> x_view_cur);
            col_ptr = (((len = wcwidth ( (s-> scroll_data)-> _y[y_cur +
                                s-> y_view_offset][x_cur + s->x_view_cur]))
                         == -1) ? 1 : len);
        }
    }
}

} /* end asl_copy_view */


/*-----------------------------------------------------------------------*/
/*      ASL_COPY_VIEW_LINE                                               */
/*-----------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Copies a line of text (and associated attributes)
 *      from the backing window of a simulated viewport
 *      to the main display window.
 *
 * RETURNS:
 *      None.
 */

static void asl_copy_view_line(s)
    ASL_SCREEN  *s;                     /* main screen data structure        */
{
int     x_cur;                          /* screen window absolute            */
                                        /* x coordinate                      */
int     y_cur;                          /* scroll area view-relative         */
                                        /* y coordinate                      */

ASL_TRACER("> asl_copy_view_line");
y_cur = s-> y_view_cur;
    if ( WMOVE(s-> main_win, y_cur + s-> y_scroll, 0) != ERR ) {
        for (x_cur = 0; x_cur < s-> main_win_cols; x_cur ++) {
                                        /* copies char AND attributes        */
            (s-> main_win)->_y[s-> y_scroll + y_cur][x_cur] =
                    (s-> scroll_data)->_y[y_cur + s-> y_view_offset][x_cur + s->x_view_cur];
            (s-> main_win)->_a[s-> y_scroll + y_cur][x_cur] =
                    (s-> scroll_data)->_a[y_cur + s-> y_view_offset][x_cur + s->x_view_cur];
            mark_change(s-> main_win, s-> y_scroll + y_cur, x_cur);
        }
    }

} /* end asl_copy_view_line */




/*---------------------------------------------------------------------------*/
/*      ASL_RING_LIST                                                        */
/*---------------------------------------------------------------------------*/
/*
 * FUNCTION:
 *      Allows list operation to be invoked on a ring field.
 *
 * RETURNS:
 *      ASL_OK
 *      ASL_CANCEL
 *      ASL_EXIT
 *      ASL_FAIL -- error occured.
 */
static 
ASL_RC asl_ring_list(ring_values, selected_index, title)
    char        *ring_values;           /* ring field                        */
    int         *selected_index;        /* index of currently selected ring  */
                                        /* sub-field                         */
    char        *title;                 /* title for list popup              */
{
                                        /* will hold reformatted ring_values */
                                        /* for list generation               */
char            *out_buf = NULL;
char            *ptr;                   /* for scanning through out_buf      */
ASL_RC          ret;                    /* return code                       */
ASL_SCR_TYPE    scr_type;               /* for asl_screen() call             */
ASL_SCR_INFO    *scr_info;              /* for asl_screen() call; allocated  */
                                        /* in asl_get_list()                 */

ASL_TRACER("> asl_ring_list");
ret = ASL_FAIL;
asl_note (ASL_NO_MSG, ASL_TRACE, MSGSTR(MSG_046, M_MSG_046), ring_values);
                                        /* copy ring field for editing       */
out_buf = (char *) ASL_MALLOCT (strlen(ring_values) + 1);
strcpy(out_buf, ring_values);
ptr = out_buf;
while (*ptr) {                          /* put ring string into              */
                                        /* cmd_to_list stdout format         */
    if (*ptr == ASL_RING_SEP)
        *ptr = '\n';                    /* change ring sub-field delimeters  */
                                        /* to newlines                       */
    ptr = strchr(ptr, ASL_RING_SEP);    /* ASL_RING_SEP must be only 1 byte  */
}
                                        /* now turn string into display      */
                                        /* list items                        */
asl_get_list(out_buf, &scr_type, &scr_info);
if (scr_type.max_index < 2) {
    asl_note(ASL_SYS_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_025, M_ERR_1820_025));
    ret = ASL_FAIL;
} else {
    scr_type.cur_index      = 1;
    scr_type.cur_win_index  = 0;
    scr_type.cur_win_offset = 0;
    scr_type.screen_code = ASL_SINGLE_LIST_SC;
    scr_info[0].text = title;
    ret = asl_screen (&scr_type, scr_info);
    if (ret == ASL_COMMIT) {
        *selected_index = scr_type.cur_index;
        ret = ASL_OK;
    }
}
ASL_FREE (scr_info);
ASL_FREE (out_buf);
return ret;

} /* end asl_ring_list */


/*---------------------------------------------------------------------------*/
/*      ASL_GET_LIST                                                         */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Extracts information from a string that encodes a list
 *      (with newline delimiters) and constructs a scr_type/scr_info parameter
 *      that represents the list entries in the form needed for generating
 *      a popup screen.
 *
 * RETURNS:
 *      ASL_OK.
 */

ASL_RC asl_get_list (out_buf, scr_type, scr_info)
    char                *out_buf;       /* the string with a newline         */
                                        /* delimited list: *** MODIFIED ***  */
    ASL_SCR_TYPE        *scr_type;      /* structure exists prior to call    */
    ASL_SCR_INFO        **scr_info;     /* array dynamically                 */
                                        /* allocated below                   */
{
int     index           = 0;            /* index into scr_info array         */
char    *buf_ptr;                       /* points into out_buf               */
char    *next_ptr;                      /* points into out_buf               */
char    *tmp_ptr;                       /* points into out_buf               */
char    *tmp_ptr_2;                     /* points into out_buf               */
int     finished        = FALSE;        /* loop predicate for constructing   */
                                        /* scr_info array                    */
int     str_length;                     /* length of current list entry      */
int     delta;                          /* size of substring to chop         */


ASL_TRACER("> asl_get_list");
                                        /* allocate title entry              */
*scr_info = (ASL_SCR_INFO *) ASL_MALLOCT ( sizeof(ASL_SCR_INFO) );
buf_ptr   = out_buf;

while (! finished) {
    index ++;                           /* allocate next entry               */
    *scr_info = (ASL_SCR_INFO *)
                    asl_realloct ( *scr_info,
                                   (index + 1 ) * sizeof( ASL_SCR_INFO ) );
    (*scr_info)[index].text       = buf_ptr;
    (*scr_info)[index].non_select = ASL_NO;
    next_ptr = strchr(buf_ptr, '\n'); /* look for end of this line           */
    while(TRUE)
    {                                   /* scan this entry; 1 pass/line      */
        if (next_ptr == NULL)
        {                               /* on the last line?                 */
            finished = TRUE;
                                        /* point at terminator               */
            next_ptr = buf_ptr + strlen(buf_ptr);
            break;
        }
        else
        {                               /* check for continuation pattern    */
            tmp_ptr  = next_ptr + 1;    /* skip newline                      */
                                        /* skip optional whitespace          */
            tmp_ptr  = tmp_ptr + strspn(tmp_ptr, " \t");
                                        /* modified in defect 146657 to make */
                                        /* the line begins with a # sign be  */
                                        /* a separate item in the list       */
            *next_ptr = '\0';           /* terminate this entry              */
            break;
        }
    } /* while */
    if ((*tmp_ptr == '\0') &&
        (*buf_ptr == '\0') ) {          /* ignore trailing empty string      */
        index --;                       /* undo overshoot                    */
        break;
    }
    tmp_ptr = buf_ptr;
    while (*tmp_ptr != '\0') {          /* scan thru current item ...        */
        str_length = strcspn(tmp_ptr, "\n");
                                        /* if list item line too long ...    */
/*      if (str_length > ASL_DEFAULT_LIST_TEXT_SIZE) {                       */
/*          tmp_ptr += ASL_DEFAULT_LIST_TEXT_SIZE;                           */
/*          delta = str_length - ASL_DEFAULT_LIST_TEXT_SIZE;                 */
/*          tmp_ptr_2 = tmp_ptr - 1;                                         */
/*          do {                           truncate by shifting line         */
/*              tmp_ptr_2 ++;                                                */
/*              *tmp_ptr_2 = *(tmp_ptr_2 + delta);                           */
/*          } while (*tmp_ptr_2 != '\0');                                    */
/*           str_length = ASL_DEFAULT_LIST_TEXT_SIZE;
             tmp_ptr[str_length] = '\0';
             while (str_length > 0 &&
                    mbswidth(tmp_ptr,str_length) == -1)
                {
                        str_length--;
                        tmp_ptr[str_length] =  '\0';
                }

        } else {
*/
            tmp_ptr += str_length;
/*        }*/
        if (*tmp_ptr == '\n') {         /* skip newline, if any              */
            tmp_ptr ++;
        }
    }
                                        /* log next list item                */
    asl_note (ASL_NO_MSG, ASL_TRACE, MSGSTR(MSG_047, M_MSG_047), buf_ptr);
    buf_ptr = next_ptr + 1;             /* skip over terminator              */
} /* while */
index ++;                               /* allocate instruction entry        */
*scr_info = (ASL_SCR_INFO *)
        asl_realloct ( *scr_info, (index + 1 ) * sizeof( ASL_SCR_INFO ) );
                                        /* point to last string terminator   */
(*scr_info)[index].text = next_ptr;
scr_type->max_index = index;
ASL_TRACER("< asl_get_list");
return ASL_OK;

} /* end asl_get_list */


/*---------------------------------------------------------------------------*/
/*      ASL_LIST_WADDSTR                                                     */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Extracts and prints trailing HFT ESC sequences for color lists.
 *      Initial (normal) part of line is passed to asl_waddstr().
 *
 * RETURNS:
 *      OK -- always returned.
 */

static int asl_list_waddstr (s, item_num, text)
    ASL_SCREEN  *s;                     /* main screen data structure        */
    int         item_num;               /* item_num of text item             */
    char        *text;                  /* text to be displayed              */
{
char    *esc_stuff;                     /* start of HFT ESC stuff in text    */

esc_stuff = strchr(text, (char) ESC);
if (esc_stuff != NULL) {
    if ((esc_stuff - text) < 2) {       /* room for >= 1 char. hilite bar    */
                                        /* and a char. to replace by '\0'    */
        asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                 MSGSTR(ERR_1820_036, M_ERR_1820_036));
    } else {
        *(esc_stuff - 1) = '\0';
        WMOVE(s-> frame_win, 5 + item_num, 4);
        asl_waddstr (s-> text_subwin, text);
        WMOVE(stdscr, s-> y_pop_offset + 3 + item_num,
             s-> x_pop_offset + 4 + esc_stuff - text);
        WADDCH(stdscr, ' ');
        WREFRESH(stdscr);
        printf("%s\r\n", esc_stuff);    /* output color bar                  */
        fflush(stdout);
        WMOVE(stdscr, 0, 0);
        WREFRESH(stdscr);
    }
}
return OK;

} /* end asl_list_waddstr */
/*---------------------------------------------------------------------------*/
/*      ASL_WCHAR_WPRINTW                                                    */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Like asl_wprintw, but checks for the number of code points in the text
 *
 * RETURNS:
 *      ASL_OK
 *      ASL_EXIT        -- error occured & user requested exit.
 */

static ASL_RC asl_wchar_wprintw(win, format, length,text)
    WINDOW      *win;           /* window to write to                        */
    char        *format;        /* printf() style format                   */
    int         length;         /* format length                             */
    char        *text;          /* text to print                             */
{
ASL_RC  ret = ASL_OK;           /* our return code                           */
char    buf[ASL_WPRINTW_BUF_SIZE + 1];  /* vsprintf buffer                   */
int number_of_code_points;     /* # of characters, not bytes, in the string  */
char char_to_save;           /* Save the char since we need to NULL terminate */
int  original_length;        /* Number of bytes in text */

if ((original_length = strlen(text)) > length)
  {
     char_to_save = text[length];
     text[length] = '\0';
   }

/*number_of_code_points = mbswidth (text, strlen(text));*/
/*number_of_code_points = NLstrdlen(text);*/

buf[0] = '\0';
                        /* will be overwritten by any                        */
                        /* non-null output char                              */
                        /* if the "printed" string is too long               */
                        /* (assuming printf output is indeed printable)    */
buf[ASL_WPRINTW_BUF_SIZE] = '\0';

sprintf(buf, format, length, length,text);

if ( original_length > length )
    text[length] = char_to_save;        /* Put the original char back        */

                                /* formatting done here                      */
if (buf[ASL_WPRINTW_BUF_SIZE] != '\0') {
    buf[200] = '\0';            /* print first few lines,                    */
                                /* in case its not all junk                  */
    ret = asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                   MSGSTR(ERR_1820_022, M_ERR_1820_022), ASL_WPRINTW_BUF_SIZE);
    if (ret == ASL_ENTER || ret == ASL_CANCEL) {
        ret = ASL_OK;
    }
}
WADDSTR(win, buf);
return ret;
} /* end asl_wprintw */



/*---------------------------------------------------------------------------*/
/*      ASL_WADDSTR                                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Like WADDSTR(), but prevents extra unwanted blank line due to
 *      spurious wrap that is generated when a line that is a exactly as
 *      wide as the window is displayed.  (This is due to the automatic
 *      position advance "feature" which moves first and looks second.)
 *
 * RETURNS:
 *      OK -- always returned.
 */

static int asl_waddstr (win, text, y_cur, x_text)
    WINDOW      *win;                   /* window to be updated              */
    char        *text;                  /* text to be displayed              */
    int		y_cur;
    int		x_text;
{
char    *text_2;                        /* text to be displayed              */
int     str_length;                     /* length of text to be displayed    */
int     str_length_2;                   /* length of text to be displayed    */
int     i = ASL_WPRINTW_BUF_SIZE / 2;   /* avoid wprintw() buffer overflow   */

while(*text != '\0') 
{
    str_length = strcspn(text, "\n"); /* length to next '\n' or '\0'       */
                                        /* is this the "extra" '\n'          */
                                        /* wrap condition?                   */
    if (str_length == (win->_maxx - win->_curx))
    {
                                        /* note!: _maxx is 1 + max allowed   */
                                        /* value for _curx                   */
                                        /* if exact fit, exclude '\n'        */
        asl_wchar_wprintw(win, "%*.*s", str_length, text);
                                        /* ignore if exact 2x, 3x fit, etc.  */
    }
    else
    {
                                        /* let curses generate wrap          */
        str_length_2 = str_length + 1;  /* for super-long lines case         */
        text_2   = text;                /* for super-long lines case         */
        while (i < str_length_2)
        {      /* do incremental writes             */
            asl_wchar_wprintw(win, "%-*.*s", i, text_2);
            str_length_2 -= i;
            text_2       += i;
        }
        asl_wchar_wprintw(win, "%-*.*s", str_length_2, text_2);
    }
    text = text + str_length;
    if (*text == '\n')
    {
        text ++;                        /* skip over '\n'                    */
	/* Defect 173383: if line wrapped, increment 1 plus */
	/* number of complete wrap-arounds.		    */
	if (str_length > win->_maxx)
	     y_cur += 1 + (str_length / win->_maxx);
	else
             y_cur ++;
        WMOVE(win, y_cur, x_text);
    }
}
return OK;

} /* end asl_waddstr */


/*--------------------------------------------------------------------------*/
/*      ASL_REDUCE_SBRKS                                                    */
/*--------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Attempts to reduce number of sbrk()s: periodically (every n-th call)
 *      malloc()'s and then immediately free()'s
 *      a moderate sized chunk of storage
 *      that subsequent malloc()'s can proceed to use up instead of
 *      having to request more storage from the operating system.
 *
 * RETURNS:
 *      None.
 */

static void asl_reduce_sbrks()
{
static  malloc_count = MALLOC_RESET;    /* count how many times              */
                                        /* we get more memory                */
char    *p;                             /* pointer to temp. storage block    */
if (malloc_count >= MALLOC_RESET) {     /* is this the "n-th" call to us?    */
    ASL_TRACER(": asl_reduce_sbrks");
    if ( (p = (char *) malloc(MALLOC_BLOCK))  ==  NULL ) {
        asl_note (ASL_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_028, M_ERR_1820_028));
        kill(0, SIGQUIT);               /* give (application program)        */
                                        /* a chance to clean up              */
        pause();                        /* in case interrupts turned off     */
        exit(-1);                       /* shouldn't get this far without    */
                                        /* using debugger                    */
    } else {
#ifdef DEBUG
        asl_note(ASL_NO_MSG, ASL_DEBUG, MSGSTR(MSG_048, M_MSG_048), p);
#endif
        ASL_FREE (p);                   /* make (new) memory available to    */
                                        /* future malloc()'s                 */
    }
    malloc_count = 0;
} else {
    malloc_count ++;                    /* just passing through in order     */
                                        /* to be counted                     */
}

} /* end asl_reduce_sbrks */


/*---------------------------------------------------------------------------*/
/*      ASL_STRCPY_MAX                                                       */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Works like mbsncpy(), but third parameter specifies the maximum
 *      destination string length (including '\0' terminator),
 *      and the destination string is always zero byte terminated,
 *      even if result is truncated.
 *
 * RETURNS:
 *      ASL_OK   -- no errors occured.
 *      ASL_FAIL -- error occured.
 *      ASL_EXIT -- error occured & user wants to exit.
 */

static ASL_RC asl_strcpy_max (s1, s2, max_size)
    char        *s1;            /* destination string                        */
    char        *s2;            /* source string                             */
    int         max_size;       /* maximum size of string                    */
                                /* (including '\0')                          */
{
ASL_RC          ret;            /* return code from asl_note()               */

if (strlen(s2) + 1 > max_size) {
    ret = asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                    MSGSTR(ERR_1820_029, M_ERR_1820_029), s2, max_size - 1);
    if (ret != ASL_ENTER && ret != ASL_CANCEL)
        return(ret);            /* should be ASL_EXIT                        */
    if (max_size <= 0) {
        s1[0] = '\0';           /* assumes empty string always               */
                                /* has room for EOS                          */
        return(ASL_FAIL);
    }
    mbsncpy(s1, s2, max_size - 1);
    s1[max_size - 1] = '\0';
    return(ASL_FAIL);
} else {
    strcpy(s1, s2);
    return(ASL_OK);
}

} /* end asl_strcpy_max */


/*---------------------------------------------------------------------------*/
/*      ASL_STRCAT_MAX                                                       */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Works like strcat(), but checks that maximum size of s1
 *      (including the '\0' terminator) is not exceeded.
 *      Returns TRUE if no errors occured, FALSE otherwise.
 *
 * RETURNS:
 *      ASL_OK   -- no errors occured.
 *      ASL_FAIL -- error occured.
 *      ASL_EXIT -- error occured & user wants to exit.
 */

static ASL_RC asl_strcat_max (s1, s2, max_size)
    char        *s1;                    /* destination string                */
    char        *s2;                    /* source string                     */
    int         max_size;               /* maximum size of string            */
                                        /* (including '\0')                  */
{
int     str_length;                     /* string length                     */
int     copy_size;                      /* how many characters to copy       */
ASL_RC  ret;                            /* return code from asl_note()       */

if (strlen(s1) + strlen(s2) + 1 > max_size) {
    ret = asl_note (ASL_SYS_ERR_MSG, ASL_LOG,
                    MSGSTR(ERR_1820_030, M_ERR_1820_030),
                    s1, s2, max_size - 1);
    if (ret != ASL_ENTER && ret != ASL_CANCEL)
        return(ret);                    /* should be ASL_EXIT                */
    str_length = strlen(s1);
    copy_size = max_size - str_length - 1;
    if (copy_size > 0)
        mbsncpy(&s1[str_length], s2, copy_size);
    if (max_size <= 0) {
        s1[0] = '\0';                   /* assumes empty string always has   */
                                        /* room for EOS                      */
        return(ASL_FAIL);
    }
    s1[max_size - 1] = '\0';
    return(ASL_FAIL);
} else {
    strcat(s1, s2);
    return(ASL_OK);
}

} /* end asl_strcat_max */


/*---------------------------------------------------------------------------*/
/*      ASL_EDIT                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Called when edit key on dialogue is pressed.
 *      Creates new set of scr_type/scr_info parameters and
 *      then passses them to asl_screen() for generating the edit popup.
 *      If an edited value is to be saved, it is copied back into
 *      the original scr_info[x].data_value field.
 *
 * RETURNS:
 *      ASL_OK          -- normal "exit" from edit popup.
 *      ASL_EXIT        -- user requested (system) exit from edit popup.
 */

static ASL_RC asl_edit(s, scr_type, scr_info)
    ASL_SCREEN     *s;          /* current ASL screen descriptor             */
    ASL_SCR_TYPE   *scr_type;   /* overall screen-related info.              */
    ASL_SCR_INFO   scr_info[];  /* line or entry-specific                    */
                                /* screen information                        */
{
ASL_RC          ret;            /* return code                               */
ASL_SCR_TYPE    edit_type;      /* overall screen-related                    */
                                /* information for edit popup                */
ASL_SCR_INFO    edit_info[3];   /* line or entry-specific screen             */
                                /* information for edit popup                */
                                /* edit value buffer                         */
char            edit_data_value[ASL_MAX_INPUT_BUF];
char            edit_instructions[ASL_MSG_MAX_STRLEN + 1];
char            *tmp_ptr;

ASL_TRACER("> asl_edit");
if (s-> screen_code == ASL_EDIT_SC) {
    asl_beep();
    return ASL_OK;
}
                                /* initialize aparameters for asl_screen()   */
edit_type.cur_win_index = 0;
edit_type.cur_win_offset= 0;
edit_type.cur_index     = 1;
edit_type.max_index     = 2;
edit_type.screen_code   = ASL_EDIT_SC;
                                        /* copy value related stuff          */
bcopy( &scr_info[SCR_CUR], &edit_info[1], sizeof(ASL_SCR_INFO) );
                                        /* copy current value                */
strcpy(edit_data_value, scr_info[SCR_CUR].data_value);
edit_info[1].data_value = edit_data_value;
                                        /* use entry name as title           */
edit_info[0].text       = edit_info[1].text;
                                        /* build edit instructions           */
if     (edit_info[1].entry_type == ASL_NO_ENTRY) {
    if (edit_info[1].op_type    == '\0') {
        tmp_ptr = MSGSTR(MSG_EDIT_NO_CHANGE, M_MSG_EDIT_NO_CHANGE);
    } else {
        tmp_ptr = MSGSTR(MSG_EDIT_NO_TYPE,   M_MSG_EDIT_NO_TYPE);
    }
} else {
    tmp_ptr     = MSGSTR(MSG_EDIT_TYPE,      M_MSG_EDIT_TYPE);
}
edit_instructions[0] = '\0';
asl_strcpy_max(edit_instructions, tmp_ptr, ASL_MSG_MAX_STRLEN);
if        (edit_info[1].op_type == ASL_RING_ENTRY) {
    tmp_ptr     = MSGSTR(MSG_EDIT_TAB_LIST,  M_MSG_EDIT_TAB_LIST);
} else if (edit_info[1].op_type == ASL_LIST_ENTRY) {
    tmp_ptr     = MSGSTR(MSG_EDIT_LIST_ONLY, M_MSG_EDIT_LIST_ONLY);
} else {
    tmp_ptr     = NULL;
}
if (tmp_ptr != NULL) {
    asl_strcat_max(edit_instructions, "\n",    ASL_MSG_MAX_STRLEN);
    asl_strcat_max(edit_instructions, tmp_ptr, ASL_MSG_MAX_STRLEN);
}
edit_info[2].text = edit_instructions;

ret = asl_screen(&edit_type, edit_info);
if (ret == ASL_ENTER) {
                                         /* save the edited value            */
    scr_info[SCR_CUR].changed         = edit_info[1].changed;
    scr_info[SCR_CUR].cur_value_index = edit_info[1].cur_value_index;
                                        /* update with new value             */
    strcpy(scr_info[SCR_CUR].data_value, edit_info[1].data_value);
    s-> x_cur_col    = 0;               /* reset lateral scrolling           */
    s-> x_cur_character = 0;
                                        /* "viewport" for value field        */
    s-> x_cur_offset = 0;
    s-> x_character_offset = 0;
    if ( strlen (scr_info[SCR_CUR].data_value)  ==  0
                                        /* wasn't a normal typein            */
            && scr_info[SCR_CUR].cur_value_index >= 0) {
        if ( scr_info[SCR_CUR].op_type == ASL_RING_ENTRY ) {
            asl_ring (scr_info[SCR_CUR].disp_values,
                      scr_info[SCR_CUR].cur_value_index,
                      scr_info[SCR_CUR].data_value,
                      scr_info[SCR_CUR].entry_size);
        } else {
            strcpy (scr_info[SCR_CUR].data_value,
                      scr_info[SCR_CUR].disp_values);
        }
    }
                                        /* display changed value             */
    asl_display_value (s, SCR_CUR, scr_type, scr_info);
    asl_copy_view_line(s);
    WMOVE(s-> main_win, (s-> y_scroll + s-> y_view_cur), s-> x_value);
    TOUCHWIN(s-> main_win);         /* park the hardware cursor here     */
    WREFRESH(s-> main_win);         /* park the hardware cursor here     */
    ret = ASL_OK;
} else if (ret == ASL_CANCEL) {
    WMOVE(s-> main_win, (s-> y_scroll + s-> y_view_cur),
           s-> x_value + s-> x_cur_col - s-> x_cur_offset);
    s-> main_win->_csbp = NORMAL;
    TOUCHWIN(s-> main_win);         /* park the hardware cursor here     */
    WREFRESH(s-> main_win);         /* park the hardware cursor here     */
    ret = ASL_OK;                       /* don't update real value           */
}
ASL_TRACER("< asl_edit");
return ret;

} /* end asl_edit */


/*---------------------------------------------------------------------------*/
/*      ASL_GETCH                                                            */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      (Currently) same as getch(), with some optional debug stuff
 *      that has been useful for running down libcur/terminfo problems.
 *
 * RETURNS:
 *      Returns a character.
 */

static wchar_t asl_getch(type)
int *type;
{
int     c = 0;
int     last_c;
int     done=FALSE;
char    mb[MB_LEN_MAX];
int     mb_index = 0;
wchar_t ret;


do
{
    do {
        last_c = c;
        c = getch();
        asl_note(ASL_NO_MSG, ASL_DEBUG, MSGSTR(MSG_049, M_MSG_049), c, c);
    } while (c == 0xffffffff);  /* a mystery side effect of job control      */
    if (c == CTRL_L) {          /* re-draw screen                            */
        asl_redraw();
    } else if (last_c == ESC) { /* ESC + <key> sequences                     */
        switch (c) {
            case CTRL_G:        /* Exit                                      */
                { (*type) = KEY_F(10); return(KEY_F(10)); }
            case 'v':           /* Page Up                                   */
            case 'V':           /* Page Up                                   */
                { (*type) = KEY_PPAGE; return(KEY_PPAGE); }
            case '<':           /* Home                                      */
                { (*type) = KEY_HOME; return(KEY_HOME); }
            case '>':           /* End                                       */
                { (*type) = KEY_END; return(KEY_END); }
            case 'g':           /* Exit main menu                            */
            case 'G':           /* Exit main menu                            */
                asl_beep();     /* ---------- unimplemented                  */
                break;
            default:
                asl_beep();
                break;
        } /* switch */
    } else {
        switch (c) {
	    case '/':
                { (*type) = KEY_SLASH; return(KEY_SLASH); }
            case ESC:           /* get another key                           */
                break;
            case KEY_LOCESC:
                { (*type) = KEY_LOCESC; return(KEY_LOCESC); }
            case KEY_TAB:       /* begining of line key                      */
                { (*type) = KEY_TAB; return(KEY_TAB); }
            case KEY_BTAB:
                { (*type) = KEY_BTAB; return(KEY_BTAB); }
            case CTRL_A:        /* begining of line key                      */
                { (*type) = CTRL_A; return(CTRL_A); }
            case CTRL_F:
                { (*type) = KEY_RIGHT; return(KEY_RIGHT); }
            case CTRL_B:
                { (*type) = KEY_LEFT; return(KEY_LEFT); }
            case CTRL_CARET:    /* Page Up                                   */
            case ESC_V:         /* Page Up                                   */
            case KEY_PPAGE:
                { (*type) = KEY_PPAGE; return(KEY_PPAGE); }
            case CTRL_V:        /* Page Down                                 */
            case KEY_NPAGE:
                { (*type) = KEY_NPAGE; return(KEY_NPAGE); }
            case ESC_LT:        /* Home                                      */
            case KEY_HOME:
                { (*type) = KEY_HOME; return(KEY_HOME); }
            case ESC_GT:        /* End                                       */
            case KEY_END:
                { (*type) = KEY_END; return(KEY_END); }
            case CTRL_E:        /* end of line key pressed                   */
            case KEY_EOL:
                { (*type) = CTRL_E; return(CTRL_E); }
            case ESC_G:         /* Exit main menu                            */
                asl_beep();     /* ---------- unimplemented                  */
                break;
            case CTRL_J:        /* Enter                                     */
            case CTRL_M:        /* Enter                                     */
            case KEY_ENTER:
            case KEY_NEWL:
                { (*type) = KEY_NEWL; return(KEY_NEWL); }
            case CTRL_P:        /* previous line                             */
            case KEY_UP:
                { (*type) = KEY_UP; return(KEY_UP); }
            case CTRL_N:        /* next line                                 */
            case KEY_DOWN:
                { (*type) = KEY_DOWN; return(KEY_DOWN); }
            case KEY_LEFT:
                { (*type) = KEY_LEFT; return(KEY_LEFT); }
            case KEY_RIGHT:
                { (*type) = KEY_RIGHT; return(KEY_RIGHT); }
            case KEY_F(1):      /* Help key                                  */
            case ESC1:
                { (*type) = KEY_F(1); return(KEY_F(1)); }
            case KEY_F(2):      /* refresh key                               */
            case ESC2:
                { (*type) = KEY_F(2); return(KEY_F(2)); }
            case KEY_F(3):
            case ESC3:
            case CTRL_G:        /* Exit current menu (cancel)                */
                { (*type) = KEY_F(3); return(KEY_F(3)); }
            case KEY_F(4):      /* list key                                  */
            case ESC4:
                { (*type) = KEY_F(4); return(KEY_F(4)); }
            case KEY_F(5):      /* undo key                                  */
            case ESC5:
                { (*type) = KEY_F(5); return(KEY_F(5)); }
            case KEY_F(6):      /* command key                               */
            case ESC6:
                { (*type) = KEY_F(6); return(KEY_F(6)); }
            case KEY_F(7):      /* edit key                                  */
            case ESC7:
                { (*type) = KEY_F(7); return(KEY_F(7)); }
            case KEY_F(8):      /* image key                                 */
            case ESC8:
                { (*type) = KEY_F(8); return(KEY_F(8)); }
            case KEY_F(9):      /* shell key                                 */
            case ESC9:
                { (*type) = KEY_F(9); return(KEY_F(9)); }
            case KEY_F(10):      /* exit key                                 */
            case ESC0:
                { (*type) = KEY_F(10); return(KEY_F(10)); }
            case ESC_M:
                { (*type) = ESC_M; return(ESC_M); }
            case ESC_F:
                { (*type) = ESC_F; return(ESC_F); }
            case ESC_C:
                { (*type) = ESC_C; return(ESC_C); }
            case ESC_S:
                { (*type) = ESC_S; return(ESC_S); }
     /*        case ESC_R:
                { (*type) = ESC_R; return(ESC_R); }        */
            case KEY_NOKEY:
                { (*type) = KEY_NOKEY; return(KEY_NOKEY); }
            case KEY_BACKSPACE:
                { (*type) = KEY_BACKSPACE; return(KEY_BACKSPACE); }
            case KEY_DC:         /* DELETE char                              */
            case CTRL_X:
                { (*type) = KEY_DC; return(KEY_DC); }
            case CTRL_K:         /* kill current field                       */
                { (*type) = CTRL_K; return(CTRL_K); }
            case CTRL_R:         /* INSERT toggle                            */
            case INSERT:
                { (*type) = CTRL_R; return(CTRL_R); }
            default:
                {
                        mb[mb_index++] = c;
                        if ((mblen (mb, mb_index)) != -1)
                        {
                                done = TRUE;
                                mbtowc (&ret, mb, MB_CUR_MAX);
                        }
                }
        } /* switch */
    }
} while (!done);

(*type) = KEY_NOKEY;
return((wchar_t) ret);
} /* end asl_getch() */


/*---------------------------------------------------------------------------*/
/*      ASL_FLUSH_INPUT                                                      */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Throw away any pending buffered input, unless filter_flag set.
 *
 * RETURNS:
 *
 */

static void asl_flush_input()
{
int type;                               /* dummy variable for asl_getch     */
wchar_t tmp;
ASL_TRACER("+ asl_flush_input");
if (! asl_filter_flag) {                /* caller not invoked as AIX filter  */
    nodelay(TRUE);                      /* makes getch() return              */
                                        /* KEY_NOKEY if no data available    */
    asl_getch(&type);
    while (type != KEY_NOKEY)   /* throw out what's available        */
        asl_getch(&type);
    nodelay(FALSE);                     /* wait for "new" user               */
                                        /* input on next read                */
}
} /* end asl_flush_input */


/*---------------------------------------------------------------------------*/
/*      ASL_CHECK_FILE_ENTRIES                                               */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Checks for the presence of files entered by the user
 *      in dialogue entry fields.
 *
 * RETURNS:
 *      ASL_OK          -- file successfully accessed.
 *      ASL_FAIL        -- error occured.
 *      ASL_EXIT        -- error occured & user wants to exit.
 */

ASL_RC asl_check_file_entries(scr_type, scr_info)
    ASL_SCR_TYPE    *scr_type;  /* overall screen-related information        */
    ASL_SCR_INFO    scr_info[]; /* line or entry-specific screen information */
{
ASL_RC  ret = ASL_OK;           /* rtn. code, ASL_OK means no err. detected  */
int     index;                  /* index into scr_info entries               */
char    value_buf[ASL_MAX_VALUE_SIZE + 1];
char    *value_start;           /* first non-whitespace char in value_buf    */
char    *value_end;             /* last non-whitespace char in value_buf     */
char    *value;
char    ptr[ASL_MAX_VALUE_SIZE + 1];

ptr[0] = '\0';
for (index = 1 ; index < SCR_MAX ; index ++) {
    if (ret == ASL_FAIL)
        break;
    /*----------------------------------------------------------*/
    /* took out the check if the file exists due to Defect 5109 */
    /* I kept the procedure because it also checks to see if    */
    /* its a required field.  We just won't check if the file   */
    /* exists.                                              */
    /*----------------------------------------------------------*/
    /* if (scr_info[index].entry_type == ASL_FILE_ENTRY) { */
    if (0) {
                                /* if file entry is non-empty, check it out  */
        if (strlen(scr_info[index].data_value) > 0)
        {
                                /* ignore leading and trailing white space   */
            ret = asl_strcpy_max(value_buf, scr_info[index].data_value,
                                 ASL_MAX_VALUE_SIZE);
            if (ret == ASL_EXIT)
                return ret;
            else
                ret = ASL_OK;
            value_start = value_buf + strspn(value_buf, " \t");
            if (  *value_start == '\0'
                                /* do nothing if empty or whitespace string  */
                   && scr_info[index].required == ASL_YES_NON_EMPTY)
            {
                                /* unless required to be non empty           */
                ret = asl_note(ASL_MSG, ASL_VERBOSE,
                               MSGSTR(ERR_1820_032, M_ERR_1820_032),
                               scr_info[index].text);
                if (ret == ASL_ENTER || ret == ASL_CANCEL)
                    ret = ASL_FAIL;
                asl_cursor(TRUE);
                continue;
            }
            value_end = value_start;
            value     = value_start;
            while (*value)
            {
                if (value != ' ' && value != '\t')
                    value_end = value;          /* no whitespace here        */
                value += mblen(value, MB_CUR_MAX);
            }
            value_end ++;
            *value_end = '\0';          /* delete trailing whitespace        */
            if (access(value_start, F_OK) != 0)
            {
                ret = asl_note(ASL_MSG, ASL_VERBOSE,
                               MSGSTR(ERR_1820_023, M_ERR_1820_023),
                               scr_info[index].data_value,
                               scr_info[index].text);
                if (ret == ASL_ENTER || ret == ASL_CANCEL)
                    ret = ASL_FAIL;
                asl_cursor(TRUE);
            }
        }
        else
        {
            ret = asl_note(ASL_MSG, ASL_VERBOSE,
                           MSGSTR(ERR_1820_023, M_ERR_1820_023),
                           scr_info[index].data_value,
                           scr_info[index].text);
            if (ret == ASL_ENTER || ret == ASL_CANCEL)
                ret = ASL_FAIL;
            asl_cursor(TRUE);
        }
    }
    else if ( scr_info[index].required == ASL_YES_NON_EMPTY)
    {
         if(DSMIT && strcmp(scr_info[1].data_value, "yes") == 0 &&
            strncmp(scr_info[1].text, "Use first value entered", 23) == 0)
         {
             if(index == SCR_MAX || index == 1)
                continue;

             if(ptr[0] == '\0')
             {
                if(strcmp(scr_info[index].data_value, NULL) != 0 &&
                   strcmp(scr_info[index].data_value, " ? ") != 0)
                   asl_strcpy_max(ptr, scr_info[index].data_value,
                                  ASL_MAX_VALUE_SIZE);
             }
             else
             {
                if(strcmp(scr_info[index].data_value, NULL) != 0)
                {
                   if(strcmp(scr_info[index].data_value, ptr) != 0)
                   {
                       ret = asl_note(ASL_MSG, ASL_LOG,
                             MSGSTR(MSG_DEFAULT, M_MSG_DEFAULT));
                       if (ret == ASL_ENTER || ret == ASL_CANCEL)
                       {
                             ret = ASL_FAIL;
                             return ret;
                       }
                   }
                }
                else
                  asl_strcpy_max(scr_info[index].data_value, ptr,
                             ASL_MAX_VALUE_SIZE);
             }
         }
         else
         {                       /* empty or just white space       */
             if(strlen(scr_info[index].data_value)
                        == strspn(scr_info[index].data_value, " \t"))
             {
                ret = asl_note(ASL_MSG, ASL_VERBOSE,
                       MSGSTR(ERR_1820_033, M_ERR_1820_033),
                       scr_info[index].text);
             }
        }
        if (ret == ASL_ENTER || ret == ASL_CANCEL)
            ret = ASL_FAIL;
        asl_cursor(TRUE);
    }
} /* for each index ... */
if(DSMIT && strcmp(scr_info[1].data_value, "yes") == 0 &&
   strncmp(scr_info[1].text, "Use first value entered", 23) == 0)
{
    if(ptr[0] == '\0')
    {
       ret = asl_note
            (ASL_MSG, ASL_LOG, MSGSTR(MSG_AT_LEAST_ONE, M_MSG_AT_LEAST_ONE));
       if (ret == ASL_ENTER || ret == ASL_CANCEL)
             ret = ASL_FAIL;

    }
    else
    {
       for(index = 2; index < SCR_MAX; index++)
       {      /* fill in default value for all empty fields */
          if(strcmp(scr_info[index].data_value, NULL) == 0)
              asl_strcpy_max(scr_info[index].data_value, ptr,
                         ASL_MAX_VALUE_SIZE);
       }
       scr_info[SCR_MAX].data_value = NULL;
    }
}
return ret;
} /* end asl_check_file_entries */


/*---------------------------------------------------------------------------*/
/*      ASL_DISPLAY_INITIAL_VALUE                                            */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Generates display for the initial value of a dialogue entry field
 *      or edit entry field; also invoked by the DEFAULT key.
 *
 * RETURNS:
 *      ASL_OK
 */

static ASL_RC asl_display_initial_value(s, item, scr_type, scr_info)
    ASL_SCREEN      *s;         /* main screen data structure                */
    int             item;       /* scr_info item (index) to be used          */
    ASL_SCR_TYPE    *scr_type;  /* overall screen-related info.              */
    ASL_SCR_INFO    scr_info[]; /* line or entry-specific                    */
                                /* screen information                        */
{
if ( strlen (scr_info[item].data_value) ==  0
        && scr_info[item].cur_value_index >= 0)
{
    if ( scr_info[item].op_type == ASL_RING_ENTRY )
        asl_ring (scr_info[item].disp_values, scr_info[item].cur_value_index,
                  scr_info[item].data_value, scr_info[item].entry_size);
    else {
        strncpy (scr_info[item].data_value, scr_info[item].disp_values, ASL_MAX_INPUT_BUF);
        scr_info[item].data_value[ASL_MAX_INPUT_BUF - 1] = '\0';
    }
}
s-> x_cur_col  = 0;
s-> x_cur_offset = 0;
s-> x_cur_character = 0;
s-> x_character_offset = 0;
asl_display_value (s, item, scr_type, scr_info);
return ASL_OK;

} /* end asl_display_initial_value */


/*---------------------------------------------------------------------------*/
/*      ASL_SAVE_TTY                                                         */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Saves current tty state and sets tty to pre-initscr() state.
 *
 * RETURNS:
 *      ASL_OK
 */

ASL_RC asl_save_tty()
{
ASL_TRACER("> asl_save_tty");
CSAVETTY(FALSE);
return ASL_OK;
} /* end asl_save_tty */


/*---------------------------------------------------------------------------*/
/*      ASL_RESTORE_TTY                                                      */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Restores part of tty state that was saved by asl_save_tty().
 *
 * RETURNS:
 *      ASL_OK
 */

ASL_RC asl_restore_tty()
{
ASL_TRACER("> asl_restore_tty");
RESETTY(TRUE);
return ASL_OK;
} /* end asl_restore_tty */


/*---------------------------------------------------------------------------*/
/*      ASL_RESTORE_TTY_COLORS                                               */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Restores tty state that was saved by asl_save_tty(), including colors.
 *
 * RETURNS:
 *      ASL_OK
 */

ASL_RC asl_restore_tty_colors()
{
ASL_TRACER("> asl_restore_tty_colors");
RESETTY(TRUE);
CRESETTY(TRUE);
return ASL_OK;
} /* end asl_restore_tty_colors */


/*---------------------------------------------------------------------------*/
/*      ASL_WPRINTW                                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Like wprintw, but with larger buffer that is checked for overflow.
 *
 * RETURNS:
 *      ASL_OK
 *      ASL_EXIT        -- error occured & user requested exit.
 */

/*VARARGS2*/
static ASL_RC asl_wprintw(win, format, ap)
    WINDOW      *win;           /* window to write to                        */
    char        *format;        /* printf() style format                   */
    va_list     ap;             /* printf() style variable argument list   */
{
ASL_RC  ret = ASL_OK;           /* our return code                           */
char    buf[ASL_WPRINTW_BUF_SIZE + 1];  /* vsprintf buffer                   */

buf[0] = '\0';
                        /* will be overwritten by any                        */
                        /* non-null output char                              */
                        /* if the "printed" string is too long               */
                        /* (assuming printf output is indeed printable)    */
buf[ASL_WPRINTW_BUF_SIZE] = '\0';
vsprintf(buf, format, &ap);   /* all %-initiated printf-style            */
                                /* formatting done here                      */
if (buf[ASL_WPRINTW_BUF_SIZE] != '\0') {
    buf[200] = '\0';            /* print first few lines,                    */
                                /* in case its not all junk                  */
    ret = asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
                   MSGSTR(ERR_1820_022, M_ERR_1820_022), ASL_WPRINTW_BUF_SIZE);
    if (ret == ASL_ENTER || ret == ASL_CANCEL) {
        ret = ASL_OK;
    }
}
WADDSTR(win, buf);
return ret;
} /* end asl_wprintw */



/*---------------------------------------------------------------------------*/
/*      ASL_CURSOR                                                           */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Sets cursor to specified HFT cursor type.
 *
 * RETURNS:
 *      None.
 */

static void asl_cursor(cursor_state)
    int         cursor_state;   /* TRUE means turn on cursor                 */
{
int     cursor_type;

                                /* cursor already on                         */
if (     ( cursor_state && asl_cursor_on )
                                /* cursor already off                        */
    || ((! cursor_state) && (! asl_cursor_on)) )
    return;                     /* don't try to change it                    */

#ifndef _ASL41
if (cursor_state)
    cursor_type = HFFULLBLOB;
else
    cursor_type = HFNONE;
#endif

asl_cursor_on = cursor_state;
/* ######### set cursor to cursor type here ######### */
} /* end asl_cursor */


/*---------------------------------------------------------------------------*/
/*      ASL_BEEP                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Rings the bell.
 *
 * RETURNS:
 *      None.
 */

void asl_beep()
{
ASL_TRACER("> asl_beep");
beep();
} /* end asl_beep */


/*---------------------------------------------------------------------------*/
/*      ASL_FREE                                                             */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Does free().  Also provides optional tracing information.
 *
 * RETURNS:
 *      None.
 */

static void asl_free(addr, line_num)
        char    *addr;
{
#ifdef DEBUG
asl_note(ASL_NO_MSG, ASL_DEBUG, "+ asl_free    ptr = 0x%x, line = %d",
         addr, line_num);
#endif
free(addr);
} /* end asl_free */


/*---------------------------------------------------------------------------*/
/*      ASL_REALLOCT                                                         */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Does realloc().  Also provides optional tracing information,
 *      checks for malloc() failure, and attempts to reduce sbrk() calls.
 *
 * RETURNS:
 *      Returns pointer to reallocated memory block.
 */


static char *asl_realloct (ptr, block_size)
    char        *ptr;           /* old storage block pointer                 */
    int         block_size;     /* desired quantity of storage               */
{

asl_reduce_sbrks();
ptr = (char *) realloc (ptr, block_size + 1);
#ifdef DEBUG
    asl_note(ASL_NO_MSG, ASL_DEBUG, "+ asl_relloct ptr = 0x%x", ptr);
#endif
if (ptr == NULL) {
    asl_note (ASL_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_026, M_ERR_1820_026));
    kill(0, SIGQUIT);           /* give application a chance to clean up     */
    pause();
    exit(-1);                   /* if all else fails                         */
}
return ptr;
} /* end asl_realloct */


/*---------------------------------------------------------------------------*/
/*      ASL_MALLOCT                                                          */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Does malloc().  Also provides optional tracing information,
 *      checks for malloc() failure, and attempts to reduce sbrk() calls.
 *
 * RETURNS:
 *      Returns pointer to allocated memory block.
 */

static char *asl_malloct (block_size, line_num)
    int         block_size;     /* allocate this much storage                */
{
char *ptr;                      /* pointer to new storage block              */

block_size ++;
asl_reduce_sbrks();
ptr = (char *) malloc (block_size);
#ifdef DEBUG
    asl_note(ASL_NO_MSG, ASL_DEBUG, "+ asl_malloct ptr = 0x%x, line = %d",
             ptr, line_num);
#endif
if (ptr == NULL) {
    asl_note (ASL_ERR_MSG, ASL_LOG, MSGSTR(ERR_1820_027, M_ERR_1820_027));
    kill(0, SIGQUIT);           /* give application a chance to clean up     */
    pause();
    exit(-1);                   /* if all else fails, ....                   */
}
#if defined(DEBUG)
                                 /* fill to make problems surface sooner ... */
    memset(ptr, (char) '\xab', block_size);     /* ab for ASL, cd for SMIT   */
#endif

return ptr;

} /* end asl_malloct */

/*------------------------------------------------------------------------*/
/*      ASL_CURSES_RC
 */
/*------------------------------------------------------------------------*/
/*
 * FUNCTION:
 *      Checks curses return codes.
 *
 * RETURNS:
 *      None.
 */

static void asl_curses_rc (parameter, line_num)
    char        *parameter;     /* return code of preceeding curses call or
 */
                                /* address being passed to next curses call
 */
    int         line_num;       /* __LINE__ at call site
 */
{
if (parameter == NULL) {        /* NULL == ERR (curses returne code)
 */
    asl_note(ASL_SYS_ERR_MSG, ASL_LOG,
             MSGSTR(ERR_1820_038, M_ERR_1820_038), line_num);
}
#ifdef DEBUG
    asl_note(ASL_NO_MSG, ASL_DEBUG,
             "+ asl_curses_rc(parameter: 0x%x, line_num: %d)",
             parameter, line_num);
#endif
} /* end asl_curses_rc */





/*---------------------------------------------------------------------------*/
/*      ASL_SIGNAL                                                           */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Handles screen refresh for job control.
 *
 * RETURNS:
 *      None.
 */

static void asl_signal ()
{
#if ! defined(_IBM5A)
signal(SIGCONT, asl_signal);            /* do refresh() as needed    */
ASL_TRACER("+ asl_signal");
TOUCHWIN(curscr);
WREFRESH(curscr);
asl_flush_input();
#endif
} /* end asl_signal */


/*---------------------------------------------------------------------------*/
/*      ASL_REDRAW                                                           */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *      Does screen refresh (like CTRL-L in vi)
 *
 * RETURNS:
 *      None.
 */

void asl_redraw(s)
ASL_SCREEN *s;
{

 TOUCHWIN(curscr);
 WREFRESH(curscr);
}


/*---------------------------------------------------------------------------*/
/*       MBS_WIDTH                                                           */
/*---------------------------------------------------------------------------*/

/*
 * FUNCTION:
 *     This routine functions like mbswidth(), except with the addition of
 *     error handling. In 4.1 mbswidth will return -1 if any of the characters
 *     passed in are not printable characters. (i.e. newline, tab....)
 *     if mbswidth returns -1, this routine will use wcwidth to check each
 *     character to see if there is any non-printable character. If it
 *     finds any of these chacters, it will replace its length with 1.
 */

int mbs_width(str, n)
char *str;
int n;
{
   int len = 0;
   int count = 0;

   if((count = mbswidth(str, n)) == -1)
   {
      count = 0;
      while(*str != '\0')
      {
         count += (((len = wcwidth(*str)) == -1) ? 1 : len);
         str ++;
      }
   }
   return count;
}

