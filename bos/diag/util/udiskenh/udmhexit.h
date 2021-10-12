/* @(#)49	1.1  src/bos/diag/util/udiskenh/udmhexit.h, dsaudiskenh, bos411, 9435A411a 8/22/94 15:24:04 */
/*
 * COMPONENT_NAME: DSAUDISKMNT
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * PURPOSE:  This is the common header file included in all the source
 *           code files.  The technique used here is to define all global
 *           variables as "externs" except in the module in which they are
 *           defined, by using #define rather than having two variable
 *           lists.
 *
 *           Non-global static variables are not declared as externs but
 *           are included in this file.
 *
 *           Function prototypes are also included in here and use the same
 *           technique as globals.  This ensures that the compiler will
 *           catch changes in function calling sequences, both in the caller
 *           and in the definition code.
 *
 * HISTORY:
 * nchang     07/13/94   Modified the hexit utility program written by
 *                       Pete Hilton to display hex data and allow users
 *                       to edit it.

 */

#include <asl.h>

#define MAX_BLOCK  2
#define ABUFF  512       /* actual data size passed by buff */
#define BUF_SIZE    624    /* 208*3, ie. 3 pages */
#define O_RDONLY 00000000

 int hex_edit(char *title_str,
             char *subtitle_str,
             char *sector_disk_str,
             char *instruction_str,
             char *offset_str,
             char *keys_str,
             char *hexw_str,
             char *asciiw_str,
             char *buff
            );

#ifdef WINDOWS_C
#  define WINDOWS_D
#else
#  define WINDOWS_D extern
#endif

#ifdef MISC_C
#  define MISC_D
#else
#  define MISC_D extern
#endif

#ifdef HEXIT_C
#  define HEXIT_D
#else
#  define HEXIT_D extern
#endif

#ifdef MSGS_C
#  define MSG(x,y) char x[]={y}
#else
#  define MSG(x,y) extern char x[]
#endif

 /*                       Common includes                            */
 
#include <stdio.h>
#include <stdlib.h>
#include <cur01.h>
#include <cur02.h>
#include <cur00.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/errno.h>

typedef int bool;

 /*                       Common defined Constants                   */
#define NOT_STACKED 0
#define STACKED     1
#define STACK_LEN     208
#define BYTE_OFF 68
#define MSG_LINE 22
#define MESG 2
#define ASCII 1
#define HEX 0
#define RUB_OUT 0xff
#define EDIT_MODE 1
#define COMMAND_MODE 2
#define CTRL_E '\x05'
#define CTRL_F '\x06'
#define CTRL_P '\x10'
#define CTRL_R '\x12'

typedef struct lifo_stack L_STACK;

struct lifo_stack
{
  int pointer;
  int depth;
  int stack[208];
};

/*                            Global Variables                       */

MISC_D int msg_clear;
MISC_D int h_mode, mode;
MISC_D int block_n;
MISC_D int hexoff;
MISC_D int stack[STACK_LEN];
MISC_D char *current_msg;
MISC_D char *default_msg;

/* The following ints provide a record of the cursor position as     */
/* calculated from the nibble value by calc_cursor.                  */

MISC_D int ascii_row, ascii_col, hex_part_row, hex_part_col;

/* The following ints provide a record of the cursor position as     */
/* calculated from the current displayed message.                    */

MISC_D long msg_row, msg_col;


MISC_D L_STACK mode_stack;
MISC_D char ext_msg[100];
MISC_D char cmd_path[208];

#ifdef MISC_C

/*                    declarations for misc.c                        */


int key_strokes = NOT_STACKED;
char *key_queue_pointer, *key_queue_end;
	
char *k_table[] =
{
  NULL,                    /*  0x100    invalid keysym               */
  NULL,                    /*  0x101    break - unreliable           */
  "\x10\x06:d\x12",        /*  0x102    cursor down                  */
  "\x10\x06:u\x12",        /*  0x103    cursor up                    */
  "\x10\x06:l\x12",        /*  0x104    cursor left                  */
  "\x10\x06:r\x12",        /*  0x105    cursor right                 */
  "\x10\x06:h\x12",        /*  0x106    home - top left              */
  "\x08",                  /*  0x107    backspace - unreliable       */
  NULL,                    /*  0x108    delete line                  */
  NULL,                    /*  0x109    insert line                  */
  NULL,                    /*  0x10a    delete character             */
  NULL,                    /*  0x10b    insert character mode start  */
  NULL,                    /*  0x10c    exit insert character mode   */
  NULL,                    /*  0x10d    clear screen                 */
  NULL,                    /*  0x10e    clear to end of screen       */
  NULL,                    /*  0x10f    clear to end of line         */
  "\x10\x06:B\x12",        /*  0x110    goto beginning of file       */
  "\x10\x06:F\x12",        /*  0x111    goto end of file             */
  "\x10\x06:f\x12",        /*  0x112    next page                    */
  "\x10\x06:b\x12",        /*  0x113    previous page                */
  NULL,                    /*  0x114    set tab stop                 */
  NULL,                    /*  0x115    clear tab stop               */
  NULL,                    /*  0x116    clear all tab stops          */
  NULL,                    /*  0x117    enter key - unreliable       */
  NULL,                    /*  0x118    soft reset key - unreliable  */
  NULL,                    /*  0x119    hard reset key - unreliable  */
  NULL,                    /*  0x11a    print or copy                */
  NULL,                    /*  0x11b    lower left (last line)       */
  NULL,                    /*  0x11c    pad upper left               */
  NULL,                    /*  0x11d    pad upper right              */
  NULL,                    /*  0x11e    pad center                   */
  NULL,                    /*  0x11f    pad lower left               */
  NULL,                    /*  0x120    pad lower right              */
  NULL,                    /*  0x121    DO key                       */
  NULL,                    /*  0x122    QUIT key                     */
  NULL,                    /*  0x123    Command key                  */
  NULL,                    /*  0x124    Previous command key         */
  NULL,                    /*  0x125    Next pane key                */
  NULL,                    /*  0x126    previous pane key            */
  NULL,                    /*  0x127    command pane key             */
  "\x10\x06:q\x12",        /*  0x128    end key                      */
  NULL,                    /*  0x129    help key                     */
  NULL,                    /*  0x12a    select key                   */
  NULL,                    /*  0x12b    scroll right key             */
  NULL,                    /*  0x12c    scroll left key              */
  "\x10\x06:t\x12",        /*  0x12d    tab key                      */
  NULL,                    /*  0x12e    back tab key                 */
  "\x0d",                  /*  0x12f    new line key                 */
  NULL,                    /*  0x130    action key                   */
  NULL,                    /*  0x131    reserved for future use      */
  NULL,                    /*  0x132    reserved for future use      */
  NULL,                    /*  0x133    reserved for future use      */
  NULL,                    /*  0x134    reserved for future use      */
  NULL,                    /*  0x135    reserved for future use      */
  NULL,                    /*  0x136    reserved for future use      */
  NULL,                    /*  0x137    reserved for future use      */
  NULL,                    /*  0x138    reserved for future use      */
  NULL,                    /*  0x139    reserved for future use      */
  NULL,                    /*  0x13a    reserved for future use      */
  NULL,                    /*  0x13b    reserved for future use      */
  NULL,                    /*  0x13c    reserved for future use      */
  NULL,                    /*  0x13d    reserved for future use      */
  NULL,                    /*  0x13e    reserved for future use      */
  NULL,                    /*  0x13f    reserved for future use      */
  NULL,                    /*  0x140    reserved for future use      */
  NULL,                    /*  0x141    reserved for future use      */
  NULL,                    /*  0x142    reserved for future use      */
  NULL,                    /*  0x143    reserved for future use      */
  NULL,                    /*  0x144    reserved for future use      */
  NULL,                    /*  0x145    reserved for future use      */
  NULL,                    /*  0x146    reserved for future use      */
  NULL,                    /*  0x147    reserved for future use      */
  NULL,                    /*  0x148    reserved for future use      */
  NULL,                    /*  0x149    reserved for future use      */
  NULL,                    /*  0x14a    reserved for future use      */
  NULL,                    /*  0x14b    reserved for future use      */
  NULL,                    /*  0x14c    reserved for future use      */
  NULL,                    /*  0x14d    reserved for future use      */
  NULL,                    /*  0x14e    reserved for future use      */
  NULL,                    /*  0x14f    reserved for future use      */
  NULL,                    /*  0x150    reserved for future use      */
  NULL,                    /*  0x151    reserved for future use      */
  NULL,                    /*  0x152    reserved for future use      */
  NULL,                    /*  0x153    reserved for future use      */
  NULL,                    /*  0x154    reserved for future use      */
  NULL,                    /*  0x155    reserved for future use      */
  NULL,                    /*  0x156    reserved for future use      */
  NULL,                    /*  0x157    reserved for future use      */
  NULL,                    /*  0x158    reserved for future use      */
  NULL,                    /*  0x159    reserved for future use      */
  NULL,                    /*  0x15a    reserved for future use      */
  NULL,                    /*  0x15b    reserved for future use      */
  NULL,                    /*  0x15c    reserved for future use      */
  NULL,                    /*  0x15d    reserved for future use      */
  NULL,                    /*  0x15e    reserved for future use      */
  NULL,                    /*  0x15f    reserved for future use      */
  NULL,                    /*  0x160    reserved for future use      */
  NULL,                    /*  0x161    reserved for future use      */
  NULL,                    /*  0x162    reserved for future use      */
  NULL,                    /*  0x163    reserved for future use      */
  NULL,                    /*  0x164    reserved for future use      */
  NULL,                    /*  0x165    reserved for future use      */
  NULL,                    /*  0x166    reserved for future use      */
  NULL,                    /*  0x167    reserved for future use      */
  NULL,                    /*  0x168    reserved for future use      */
  NULL,                    /*  0x169    reserved for future use      */
  NULL,                    /*  0x16a    reserved for future use      */
  NULL,                    /*  0x16b    reserved for future use      */
  NULL,                    /*  0x16c    reserved for future use      */
  NULL,                    /*  0x16d    reserved for future use      */
  NULL,                    /*  0x16e    reserved for future use      */
  NULL,                    /*  0x16f    reserved for future use      */
  NULL,                    /*  0x170    reserved for future use      */
  NULL,                    /*  0x171    reserved for future use      */
  NULL,                    /*  0x172    reserved for future use      */
  NULL,                    /*  0x173    reserved for future use      */
  NULL,                    /*  0x174    reserved for future use      */
  NULL,                    /*  0x175    reserved for future use      */
  NULL,                    /*  0x176    reserved for future use      */
  NULL,                    /*  0x177    reserved for future use      */
  NULL,                    /*  0x178    reserved for future use      */
  NULL,                    /*  0x179    reserved for future use      */
  NULL,                    /*  0x17a    reserved for future use      */
  NULL,                    /*  0x17b    reserved for future use      */
  NULL,                    /*  0x17c    reserved for future use      */
  NULL,                    /*  0x17d    reserved for future use      */
  NULL,                    /*  0x17e    reserved for future use      */
  NULL,                    /*  0x17f    reserved for future use      */
  NULL,                    /*  0x180    Fkey  0                      */
  "\x10\x06:h\x12",        /*  0x181    Fkey  1                      */
  NULL,        /*  0x182    Fkey  2                      */
  "\x10\x06:q\x12",        /*  0x183    Fkey  3, CANCEL             */
  NULL,        /*  0x184    Fkey  4, G to NULL                      */
  NULL,        /*  0x185    Fkey  5, U to NULL                      */
  NULL,        /*  0x186    Fkey  6, D to NULL                      */
  "\x10\x06:c\x12",        /*  0x187    Fkey  7, commit, not page backward*/
  NULL,        /*  0x188    Fkey  8, f to NULL                      */
  NULL,        /*  0x189    Fkey  9, m to NULL                      */
  "\x10\x06:e\x12",        /*  0x18a    Fkey  10, EXIT*/
  NULL,        /*  0x18a    Fkey 10, P to NULL                      */
  NULL,        /*  0x18b    Fkey 11, S to NULL                      */
  NULL,                    /*  0x18c    Fkey 12                      */
  NULL,                    /*  0x18d    Fkey  1 shift                */
  NULL,                    /*  0x18e    Fkey  2 shift                */
  NULL,                    /*  0x18f    Fkey  3 shift                */
  "\x10\x06:g\x12",        /*  0x190    Fkey  4 shift                */
  NULL,                    /*  0x191    Fkey  5 shift                */
  NULL,                    /*  0x192    Fkey  6 shift                */
  "\x10\x06:B\x12",        /*  0x193    Fkey  7 shift                */
  "\x10\x06:F\x12",        /*  0x194    Fkey  8 shift                */
  "\x10\x06:M\x12",        /*  0x195    Fkey  9 shift                */
  "\x10\x06:P\x12",        /*  0x196    Fkey 10 shift                */
  "\x10\x06:s\x12",        /*  0x197    Fkey 11 shift                */
  NULL                     /*  0x198    Fkey 12 shift                */
};
#define NT_KEYS   sizeof(k_table)/sizeof(char *)

#endif                                                     /* MISC_C */


#ifdef WINDOWS_C

/*                 declarations for curses windows                   */

WINDOW *ourscr;
WINDOW *hex_part, *ascii;
WINDOW *help_scr;

/* The following ints provide a record of the cursor position as     */
/* CURRENTLY DRAWN by draw_cursor.                                   */

long d_ascii_row, d_ascii_col, d_hex_part_row, d_hex_part_col;
int  d_mode;
long d_msg_row, d_msg_col;

void *actscr, *passcr,*tmpscr;

/*                     help display cosmetics                        */

char *help_head[7];
static char blanks[] = "                                        ";

static char *no_help[]=
{	
"The help text file \"hexit.help.text\"    ",
"cannot be opened.  This is all you get! ",
 NULL
};

static char **no_help_pages[]=
{
  &no_help[0],
   NULL	
};
   
static char ***help_text = NULL;
int help_page, help_page_number; 
int total_help_pages = 5;              

#endif                                                  /* WINDOWS_C */

/*      Message Definitions .. defined in msgs.c                     */

MSG(hexchars,"0123456789ABCDEFabcdef"); 
MSG(blanks,"                                                                              ");
MSG(Hhead,"0 1 2 3  4 5 6 7  8 9 A B  C D E F ");
MSG(Hfoot," Hex  Window ");
MSG(Afoot," ASCII Window ");
MSG(page_throw_string,".p\x0a");
MSG(help_file,"hexit.help.text");


/*  function prototypes for functions in windows.c                   */

WINDOWS_D void reset_reset();
WINDOWS_D int  init_init();
WINDOWS_D void set_byte_off(int number);
WINDOWS_D int  draw_cursor(int mode);
WINDOWS_D void display_msg(char *message);
WINDOWS_D void init_cursor();
WINDOWS_D void init_parentw(char *st1,char *str2,char *str3,char *str4,
                            char *str5);
WINDOWS_D void init_hexw(char *st1);
WINDOWS_D void init_asciiw(char *st1);
WINDOWS_D int  init_display(char *st1, char *str2,char *str3,char *str4,
                            char *str5,char *str6, char *str7,char *str8);
WINDOWS_D void reset_display();
WINDOWS_D void init_keybd();   
WINDOWS_D void init_help_strings();
WINDOWS_D void init_helpw();
WINDOWS_D void fill_help();
WINDOWS_D void display_help();
WINDOWS_D void repaint();
WINDOWS_D void flip_screens();
WINDOWS_D int  fill_with_hex(char *buff);
WINDOWS_D int  fill_with_char(char *buff);
WINDOWS_D void set_windows(char *buff); 
WINDOWS_D void insert_a_char(char q,int nibble);
WINDOWS_D int  get_a_key(); 

/* function prototypes for functions in misc.c                       */

MISC_D void init_global();
MISC_D int make_byte_string(int number,char *string, int hex);
MISC_D void calc_cursor(int nibble);
MISC_D void set_cursor(int nibble,int mode);
MISC_D char get_input(); 

