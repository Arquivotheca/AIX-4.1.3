/* @(#)67	1.14	src/bos/usr/bin/odme/odme.h, cmdodm, bos411, 9428A410j 4/5/93 13:00:44 */
/*
 * COMPONENT_NAME:  (ODME) ODME.H - defines for ODME
 *
 * FUNCTIONS: 
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
/**************************************************************************/
/*                                                                        */
/*  FILENAME    : odme.h                                                  */
/*                                                                        */
/*  DESCRIPTION : This file contains all includes, data constants, and    */
/*                structures used with the Object Data Manager Editor.    */
/*                                                                        */
/*  HISTORY     : Created 12/13/88                                        */
/*                                                                        */
/**************************************************************************/

#include <cur00.h>
#include <cur01.h>
#include <cur02.h>
#include <signal.h>
#include <ctype.h>
#include <varargs.h>
#include <memory.h>
#include <sys/types.h>
#include <nl_types.h>
#include <limits.h>
#include "odmecatdefs.h"
#include "odmold.h"
#include "trace.h"
#include "routine.h"
#include "odmi.h"

#include <fcntl.h>
#include <sys/inode.h>
#include "newodmdefs.h"
#include <string.h>

/*----------------------------------------------------------*/
/* Headings for Object Data Manager Editor screens          */
/*----------------------------------------------------------*/
#define DF_EDITOR_NAME   "Object Data Manager Editor"
#define DF_CREATE_NAME   "Create Object Class"
#define DF_CRITERIA_NAME "Search Criteria"
#define DF_DISPLAY_NAME  "Object Display"
#define DF_GRAPH_NAME    "Display Relational Graphs"
#define DF_DISP_MY_DESC_NAME "Display Object Class Descriptors"
#define DF_CONT_NAME      "OBJECT CLASS MANAGEMENT"
#define DF_NEW_NAME	"NEW OBJECT CLASS" 

/*----------------------------------------------------------*/
/* Error codes for function prerr () to print               */
/*----------------------------------------------------------*/

#define DF_OBJECT_EXISTS	  	"ERROR - object class exists. Cannot create"
#define DF_DELETE_FAILED		"Delete Failed"
#define DF_CT_OPEN_FAILED		"Open of OBJREPOS failed"	
#define DF_CT_OPENED_RO		"Opened Read_Only. Cannot delete"
#define DF_CR_DMALLOC_FAILED	"Fatal Error - Malloc failed"
#define DF_CR_CREATE_FAILED	"CREATE failed! error = "		
#define DF_PRINT_SCREEN_ERROR	"Print Screen Error\n"
#define DF_CR_INPUT_ERROR		"Error - More Data Required"
#define DF_SC_SEARCH_STRING	"Error - search string too long"
#define DF_NW_DMALLOC_FAILED	"Fatal Error - Malloc failed"
#define DF_NW_CREATE_FAILED	"CREATE failed! error = "
#define DF_NW_ADD_FAILED		"Error - Add failed! error = "
#define DF_OD_NO_OBJECTS		"No objects returned"
#define DF_OD_MALLOC_FAILED	"Fatal Error! Delete malloc failed"
#define DF_OD_BAD_RETURN		"Error - ODM get! error = "
#define DF_OD_NOT_A_LINK		"Error - not a link descriptor"
#define DF_OD_EXPAND_OPEN		"Error - Open failed! error = "
#define DF_OD_CLOSE_FAILED		"Error - Close failed! error = "
#define DF_OD_DELETE_FAILED	"Error - Delete failed! error = "
#define DF_OD_ADD_FAILED		"Error - Add failed! error = "
#define DF_OD_CHANGE_FAILED	"Error - Change failed! error = "
#define DF_OD_READ_ONLY		"Readonly mode"
#define DF_OD_DELETE_MALLOC	"Fatal Error! malloc failed"      
#define DF_OS_NO_DESC		"Error! No descriptors"          
#define DF_OS_UNKNOWN_DESC		"Error! unknown descriptor type"
#define DF_OS_CRITERIA		"Error! Bad search criteria"
#define DF_OS_SYNTAX		"Error! Bad criteria syntax"
#define DF_OS_ODM_ERROR		"Error! Bad ODM return"
#define DF_OS_STORAGE_MALLOC	"Fatal Error! Data malloc failed"
#define DF_OS_DESC_MALLOC		"Fatal Error! Descriptor malloc failed"
#define DF_OS_INVALID_TYPE		"Error! Invalid clas type"
#define DF_OS_OPEN_FAILED		"Error - Open failed! error = "
#define DF_OS_DELETE_FAILED	"Delete Failed! error = "
#define DF_RG_MALLOC_FAILED	"Fatal Error! Malloc failed"
#define DF_RG_PRINT_SCREEN_ERROR	"Print Screen Error\n"
#define DF_WD_MALLOC_FAILED	"Error! Malloc failed"

/*------------------------------*/
/* Version number               */
/* level-yearmonth-BLbuildlevel */
/*------------------------------*/
#define VERS "01-9001-BL05" 
/* #define THIS_IS_5A */
/*------------------------------------------------------------------------*/
/* characters which are non printable will be displayed as this character */
/*------------------------------------------------------------------------*/
#define NON_PRINTABLE_CHAR   '*'

/*----------------------------------------------------------------------------*/
/*  CURSES PARAMETERS : These defines represent coordinates used during       */
/*                      curses windowing.                                     */
/*----------------------------------------------------------------------------*/
#define MAX_CRITERIA_ITEMS  (LINES - 5)       /* max items on criteria screen */
#define MAX_MAIN_ITEMS            7           /* max items on main screen     */
#define MAX_CREATE_ITEMS    (LINES - 5)       /* max items on create screen   */
#define MAX_GRAPH_ITEMS     (LINES - 5)       /* max rows on graph screen     */
#define MAX_DISPLAY_ITEMS   (LINES - 9)       /* max items on display screen  */
#define MAX_HEADER_LENGTH         15          /* max descriptor header space  */
#define MAX_DISPLAY_COLS  (COLS - (2*GUTTER)) /* max columns in display space */
#define GUTTER                    2           /* 2 column window spacing      */
#define CENTER              (COLS / 2)        /* center of the window         */


/*----------------------------------------------------------------------------*/
/* Data constants dealing with objects                                        */
/*----------------------------------------------------------------------------*/
#define MAX_DESC_NAME            25    /* max portion of descriptor name to   */
                                       /* display on screen                   */
#define MAX_INPUT_LENGTH         50    /* max input string length             */
#define MAXARGS                  30    /* max variable # of arguments in help */
#define FETCH_OBJECTS            130   /* no. of objects to fetch at a time   */
#define DELETE_OBJECTS           30    /* object id delete storage length     */
#define FLOATWIDTH               10    /* floating point field width          */
#define LONGWIDTH                10    /* long integers field width           */
#define SHORTWIDTH               6     /* short intergers field width         */
#define CONTINUE                 1     /* Return from function and continue   */
#define EXIT                     3     /* Return from function and exit       */
#define MAIN_OBJ_ROW             3     /* start row for main box in rel_graph */
#define MAIN_OBJ_COL             5     /* start col for main box in rel_graph */
#define BOX_HEIGHTH		 3     /* box heighth for main box.rel_graph  */
#define BOX_WIDTH		 25    /* same as MAX_DESC_NAME               */
#define MAX_LINKS		 50    /* default number of links handled     */
#define MAX_LINK_POS             21    /* max value for offset in rel_graph   */
#define LINK_WND_START           3     /* start row for link objects display  */
#define DOWN			 -1    /* passed to disp_links for scrn clear */
#define UP			 1     /* passed to disp_links for scrn clear */
#define DONT			 0     /* passed to disp_links for no scrnclr */
#define Desc_Count		50    /* number of descriptors limit. Create */

#define DISALLOWED              "Config_Rules product sm_cmd_opt sm_name_hdr CuVPD sm_cmd_hdr sm_menu_opt"


/*----------------------------------------------------------------------------*/
/*  Define operations that can be performed on objects                        */
/*----------------------------------------------------------------------------*/
#define NOCHG                    0     /* no operation; default setting       */
#define ADD                      1     /* add object to object class          */
#define CHANGE                   2     /* change object                       */


/*----------------------------------------------------------------------------*/
/*  Bottom function key display strings                                       */
/*----------------------------------------------------------------------------*/
#define DF_CRITERIA_KEYS " <Esc>1=Help   <Esc>2=Search   <Esc>3=EXIT   <Esc>7=PgUp   <Esc>8=PgDown"
#define DF_DISPLAY_KEY_1 " <Esc>1=Help   <Esc>2=Search   <Esc>3=EXIT     <Esc>4=Add    <Esc>5=Delete"
#define DF_DISPLAY_KEY_2 " <Esc>6=Copy   <Esc>7=PgUp     <Esc>8=PgDown   <Esc>9=Left   <Esc>0=Right "
#define DF_CREATE_KEYS   " <Esc>1=Help   <Esc>2=Edit   <Esc>3=EXIT/Create   <Esc>4=Add   <Esc>5=Delete "
#define DF_PANEL_KEYS    "                   <Esc>1=Continue         <Esc>3=Discontinue "
#define DF_MAIN_KEYS     "                      <Esc>1=Help           <Esc>3=QUIT "

#define DF_GRAPH_KEY1 " <Esc>1=Help   <Esc>2=Select Object Class   <Esc>3=EXIT/Return                "
#define DF_GRAPH_KEY2 " <Esc>4=Retrieve/Edit objects  <Esc>5=Display Descriptors "
#define DF_DISP_KEY "                        <Esc>1=Help   <Esc>3=EXIT/Return "
#define DF_STD_KEYS "           <Esc>1=Help  <Esc>2=Functions  <Esc>3=Exit/Return "  

/*------------------------------------------*/
/* Escape key character combinations        */
/*------------------------------------------*/
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


/*-----------------------------------------------------*/
/* US 101-key Translate table; EMACS simple motion     */
/*-----------------------------------------------------*/
#define CTRL_A    0x01     /* beginning of field       */
#define CTRL_B    0x02     /* backward char            */
#define CTRL_D    0x04     /* delete char              */
#define CTRL_E    0x05     /* end of field             */
#define CTRL_F    0x06     /* forward char             */
#define CTRL_K    0x0B     /* kill field               */
#define CTRL_N    0x0E     /* next line                */
#define CTRL_O    0x0F     /* open line                */
#define CTRL_P    0x10     /* previous line            */
#define INSERT    0x10B    /* insert key pressed       */
#define KEY_NWL   0xA	   /* new line alternate       */



/*------------------------------------------------------------------------*/
/*  Object class pointer and description structure                        */
/*------------------------------------------------------------------------*/
struct objtoken
 {
    struct Class *objptr;              /* the pointer to the object class    */
    int number_desc;                   /* number of descriptors returned     */
    char objname[MAX_OBJ_NAME];        /* the object name                    */
    char objrep_path[MAX_OBJ_PATH];    /* the path where the object is       */
    char node[MAX_OBJ_NODE];           /* node the object resides on         */
    int  lock;                         /* how this object is to be opened    */
 };


/*------------------------------------------------------------------------*/
/*  struct descriptor_data is used for holding descriptors returned from  */
/*  odm, their relative screen attributes, and text heading               */
/*------------------------------------------------------------------------*/
struct descriptor_data
 {
   struct Class *classp;              /* ptr to odmi structure              */
   struct descriptor_data *next,    /* used when descriptors are link     */
                          *prev;    /* listed during object create        */
   int start_column,                /* column to start object field on    */
       header_width,                /* width of descriptor header         */
       data_width;                  /* width of descriptor field          */
   char header[MAX_HEADER_LENGTH];  /* descriptor type for header display */
 };



struct descrip_info  {
                     wchar_t descrip_name[MAX_DESCRIP_NAME];
                                          /* descriptor name */
                     int ODM_type;
                                         /* descriptor's ODM type */
                     char class_name[MAX_CLASS_NAME];
                                        /* object class that this descriptor */
                                        /* came from.                        */
                     char link_class[MAX_CLASS_NAME];
                                      /* the object class this descriptor    */
                                      /* links to (if any)                   */
                     char link_descrip[MAX_DESCRIP_NAME];
                                      /* descriptor which is linked to       */
                     short size;
                                      /* length (in bytes) of the descriptor */
                     short iterator;
                                      /* fixed number of times to repeat this*/
                                      /* descriptor.                         */
                     short key;
                                      /* KEY or NOKEY depending if there is  */
                                      /* an index for this descriptor or not */
                     int descrip_number;
                                      /* the unique id for this descriptor. */
                                      /* This value is determined at run-   */
                                      /* time and may not always be the same */
                                      /* value.                              */
                     int linktome;
                                      /* TRUE if this column is linked to */
                                      /* FALSE otherwise                  */
                     int indicator;
                                      /* The trigger mode(s) for ODM_METHOD */

                     char user_area[MAX_USER_AREA];
                                      /* Can hold anything that the user   */
                                      /* wants it to.  This is like a      */
                                      /* comment field for the descriptor  */
                    };


struct descriptor_data_old
 {
   struct descrip_info descriptor;              /* ptr to odmi structure              */
   struct descriptor_data_old *next,    /* used when descriptors are link     */
                          *prev;    /* listed during object create        */
   int start_column,                /* column to start object field on    */
       header_width,                /* width of descriptor header         */
       data_width;                  /* width of descriptor field          */
   char header[MAX_HEADER_LENGTH];  /* descriptor type for header display */
 };
/*------------------------------------------------------------------------*/
/*   struct object_data is used for the storage of objects during display */
/*   and update                                                           */
/*------------------------------------------------------------------------*/
struct object_data
 {
   char **field_array;        /* field array for object data storage      */
   int  modify_type,          /* NOCHG, CHANGE, or ADD modify spec.       */
        update,               /* does the field array need to be updated? */
        object_id;            /* unique object id of this object.         */
   struct object_data *next,  /* pointer to next object                   */
                      *prev;  /* pointer to previous object               */
 };



/*---------------------------------------------------------*/
/*  General purpose field description structure usually    */
/*  used during panel display.                             */
/*---------------------------------------------------------*/
struct field_data
 {
   int start_column,           /* where the field starts   */
       data_width;             /* the width of the field   */
 };

struct elemdata
	{
		char elname[256];
		char linkhold[256];
		char colhold[256];
	};
	

/*--------------------------------------------------------------*/
/*  Global parameters used throughout ODME                      */
/*--------------------------------------------------------------*/
int READONLY;     /* Was the object class opened as read only ? */
int INSERTON;     /* Is insert on or off ?                      */
int TRM;          /* terminal type set in odmewindow init_curses */
char ODME_MASTER_PATH[MAX_CLASS_PATH];
char mbstring[MB_LEN_MAX + 1];

int white;        /* curses colors  white                       */
int rwhite;       /*                reverse white               */
int cyan;         /*                cyan                        */
int rcyan;        /*                reverse cyan                */
int magenta;      /*                magenta                     */
int rmagenta;     /*                reverse magenta             */
int red;          /*                red                         */
int green;        /*                green                       */
FILE *RD;
int TRAC;
int REPLAY;
int DLY;
