/* @(#)81 1.1 src/bos/usr/bin/vfu/vfu.h, cmdpios, bos411, 9428A410j 4/28/94 08:12:58 */
/*
 * COMPONENT_NAME: (CMDPIOS)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


		/********************************
		*				*
                *  HEADER FILE FOR VFU COMMAND  *
	        *				*
		*********************************/



#define 	MAXFILENAME	50	/* max number of characters allowed 
				   	   in filename  without '.vfu' extension */	
#define 	MAXCHANNEL	12	/* max number of channels */	
#define 	TOP		1	/* max number of channels */	
#define 	BOTTOM		MAXCHANNEL /* max number of channels */	
#define 	MAXFORMLENGTH	144	/* length max of form */	
#define 	DFLTLENGTH	66 	/* default length */
#define 	MAXDIGIT	3	/* max number of digits */	
#define 	N_MASK		0xFF	/* neutral mask */	
#define 	MASK_1		0x01	/* mask for checking less significant bit 1*/	
#define 	LINE		80	/* max number of character in line */	
#define 	FNAME		10	/* max number of character in 
					   filename iconized */ 
#define 	FULLPG		10	/* depth size of shown page */
#define 	SINGLESCROL	1	/* one line scrolling */
#define 	CLOSE_VFU	1	/* used in vfu_close */
#define 	CLOSE_CURS	0	/* used in vfu_close */
#define 	XFN		3	/* x_offset of filename square */
#define 	YFN		10	/* y_offset of filename square */
#define 	XNBFN		6	/* number of windows by columns */
#define 	YNBFN		12	/* Number of rows of windows */
#define 	NBFILES		XNBFN*YNBFN	/* size of array for storing vfu_format files found in working directory */
#define 	MSGSIZE		25	/* average size of message */

#define 	XNBSOS		80	/* size of SOS window */
#define 	YNBSOS		15	
#define 	IS_ON		1	/* used for all_is_set */
#define 	IS_OFF		!IS_ON	/* used for all_is_set */
#define 	XOF1		1	/* offset for scrollbar options */
#define 	XOF2		9
#define 	XOF3		10
#define 	XOF4		10
#define 	XOF5		10
#define 	XOF6		8

/* Offset for drawing vfu_page */
#define 	XMAXCH		33
#define 	XMAXNB		3

/* Coordinates offset for first window of "FILES" option */
#define 	XFILES		10
#define 	YFILES		3

/* SOS window coordinates */
#define 	YBEGSOSW	0
#define		XBEGSOSW	11
#define 	ROWSOSW		12
#define 	COLSOSW		80
/* Vfu window coordinates */
#define 	YBEGW		8
#define		XBEGW		27
#define 	ROWW		10
#define 	COLW		35
/* Fn window coordinates */
#define 	W_NAME		1
#define 	YBEGFNW		1
#define		XBEGFNW		11
#define 	ROWFNW		1
#define 	COLFNW		MAXFILENAME
/* Fo window coordinates */
#define 	W_FORM		2
#define 	YBEGFOW		2
#define		XBEGFOW		13
#define 	ROWFOW		1
#define 	COLFOW		4
/* Num window coordinates */
#define 	W_NUM		3
#define 	YBEGNUMW	8
#define		XBEGNUMW	20
#define 	ROWNUMW		10
#define 	COLNUMW		4
/* Vfu window coordinates */
#define 	W_VFU		4
#define 	YBEGCMDW	20
#define		XBEGCMDW	1
#define 	ROWCMDW		1
#define 	COLCMDW		80
/* Msg window coordinates */
#define 	W_MSG		5
#define 	YBEGMSGW	22
#define		XBEGMSGW	1
#define 	ROWMSGW		3
#define 	COLMSGW		80

/* Padding byte for PR88 */
#define 	CH88_PAD	0x40
#define 	CH88_1_7	0x41
#define 	CH88_2_8	0x42
#define 	CH88_3_9	0x44
#define 	CH88_4_10	0x48
#define 	CH88_5_11	0x50
#define 	CH88_6_12	0x60

/* Padding byte for PR54 */
#define 	CH54_PAD	0x00
#define 	CH54_1_7	0x01
#define 	CH54_2_8	0x02
#define 	CH54_3_9	0x04
#define 	CH54_4_10	0x08
#define 	CH54_5_11	0x10
#define 	CH54_6_12	0x20

/* Default debug level */
#ifndef DEBUGLEVEL
#define 	DEBUGLEVEL 	3
#endif

#define 	BUFSIZE		256

/* Directory of temporary files */
#define 	PANICDIR	"/usr/tmp"

/* Working directory */
#define 	WORKDIR		"./"

/* File extension */
#define 	SAME		".vfu"

#define 	NEWLINE		"\n"

/* Useful debug mode */
/* #define DEBUG(l,f,s)	if( debug > l ) fprintf(stdout,f,s) */
#define 	DEBUG(l)	if( l < debug)
#define 	HOTDEBUG(l)	if(l)

/* W_menu structure */
struct 	w_menu {
	short row; /* row field start location */
	short column; /* column field start location */
	char *info; /* text information */
	short type; /* type of menu */
	int weight; /* used for file size */
	union  {
		int w_val; /* field value */
		struct w_menu *wsubmenu; /* submenu */
		int (*command)(); /* command function */
	}v;
	union {
		WINDOW *u_field; /* window of this field */
		chtype u_kexit; /* exit key */
	}u;
	struct w_menu *wleft; /* left field */
	struct w_menu *wright; /* right field */
	struct w_menu *wup; /* up field */
	struct w_menu *wdown; /* down field */
	struct w_menu *wnext; /* next field */
	};

typedef struct w_menu W_MENU	;

/* Control_driven information */
struct info {
	short type; /* information type */
	short row; /* start row processing location  */
	short col; /* start column processing location  */
	short lgmax; /* maximum length */
	short lgmin; /* minimum length */
	char prompt; /* prompt character in field */
	char *prohib; /* prohibited characters */
	char *allow; /* allowed characters */
	char *separ; /* separators */ 
	char f_base; /* base type */
	/* values */
	union { 
		int int_dt;
		char *str_dt;
	}data;
	/* default value */
	union {
		int int_def;
		char *str_def;
	}deflt;
	int (*class_help)(); /* help function */
	int (*class_ctrl)(); /* ctrl function */
	struct info *father; /* ancestor info */
	struct info *brother; /* same rank info intree */
	struct info *next; /* next info */
	};
typedef struct info	VFU_INFO;

/* Macros related to W_MENU */
#define 	ufield		u.u_field 
#define 	ukexit		u.u_kexit 
#define 	wval		v.w_val 
#define 	submenu		v.wsubmenu 

#define 	T_VAL		0	/* Value type */
#define 	T_MENU		1	/* Menu type */
#define 	T_COMM		2	/* Command type */
#define 	T_TITLE		3	/* Title type */

#define 	MENU_NULL	(W_MENU *)0 /* Particular value given to fields wdown,wup,wright,wleft
						to exit menu upon functionnal key strike */
#define 	MENU_EXIT	(W_MENU *)1
#define 	SUBMENU		(W_MENU *)2
#define 	UPEXIT		0x01
#define 	LEFTEXIT	0x02

/* Macros related to VFU_INFO */
#define 	V_STR		0x0001 /* string type */
#define 	V_INT		0x0002 /* integer type */
#define 	V_MASK		0x00FF /* mask */

#define 	V_EMPTY		0x8000 /* empty flag */
#define 	V_DFLT		0x4000 /* default flag */
#define 	V_NUP		0x2000 /* no upward chaining flag */
#define 	V_AUTN		0x0100 /* automatic next move flag */
#define 	V_AUTP		0x0200 /* automatic precedent move flag */
#define 	V_AUTE		0x0400 /* automatic escape flag */

#define 	ok_decimal(info)	((info->f_base) == 'D') 
#define 	ok_dflt(info)	((info->type) & V_DFLT == V_DFLT) 
#define 	ok_empty(info)	((info->type) & V_EMPTY == V_EMPTY) 
#define 	ok_vnup(info)	((info->type) & V_NUP == V_NUP) 
#define 	ok_vautn(info)	((info->type) & V_AUTN == V_AUTN) 
#define 	ok_vautp(info)	((info->type) & V_AUTP == V_AUTP) 
#define 	ok_vaute(info)	((info->type) & V_AUTE == V_AUTE) 

#define 	intval	data.int_dt
#define 	strval	data.str_dt
#define 	strdef	deflt.str_def
#define 	intdef	deflt.int_def


extern 		W_MENU *vfu_toplevel_menu();
extern 		W_MENU *vfu_make_sublevel_menu();
extern int 	vfu_menu();
extern int 	packed_menu();

/* Macros */
#define 	wbox(w)	wborder(w, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, \
			   ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
#define 	vfu_TOP(M, on, off)			vfu_toplevel_menu(stdscr, M, on, off)
#define 	vfu_M(tab, y, x)			vfu_menu(stdscr, tab, y, x)
#define 	vfu_make_SUB(tab,y,x,f,o)		vfu_make_sublevel_menu(stdscr, tab, y, x, f, o)

/* Terminfo macros */
#define 	Gaddstr(s)			wGaddstr(stdscr, s)
#define 	GCaddstr(s)			wGCaddstr(stdscr, s)
#define 	mvGaddstr(y, x, s)		(move(y,x)==ERR?ERR:wGaddstr(stdscr, s))
#define 	mvGCaddstr(y, x, s)		(move(y,x)==ERR?ERR:wGCaddstr(stdscr, s))
#define 	mvwGaddstr(win, y, x, s)	(wmove(win, y,x)==ERR?ERR:wGaddstr(win, s))
#define 	mvwGCaddstr(win, y, x, s)	(wmove(win, y,x)==ERR?ERR:wGCaddstr(win,s))

/*
#define 	vfu_page_home_top(win)		wmove(win,0,XMAXCH)
#define 	vfu_page_home_bottom(win)	wmove(win,FULLPG,XMAXCH)
#define 	KEY_ESC		0x1B;

void vfu_boost_up();
void vfu_boost_down();
void vfu_draw_page();
void syserr(msg)
void warning(msg)
void reverse(s)
void itoa(n,s)
int vfu_getn(pstr)
void vfu_init_davfu(target,big)
unsigned short vfu_channel_code(item)
int vfu_compare(tab1,tab2)
void vfu_init_page_t(target,forml,large)
void vfu_make_room_all(custom_size)
void vfu_init_all()
void vfu_hard_code(tab,bound,row,channel)
int vfu_translate_code(val,flag)
void vfu_fill_page_t(tbfrom,dimfrom,tbtarget)
void vfu_read_file(fd,target,name)
void vfu_save_file(fd,target,name)
void vfu_help_me()
void vfu_view_davfu(target,lg)
void vfu_view_page(target,heigth,large)
void vfu_boost_up(nbpanel,pgpanel,target,nbscroll)
void vfu_boost_down(nbpanel,pgpanel,target,nbscroll)
void vfu_draw_page(nbpanel,pgpanel,num_toprow,target)
void vfu_edit_page(nbpanel,pgpanel,target2D,target1D)
void vfu_prime_edit()
void vfu_second_edit(nbpanel,pgpanel,target2D,target1D)
void vfu_command();
void vfu_operate(vfg,ffg,lfg,cfg,fname,lgstr,fd,targetarray,filname)
void vfu_parse_channel(argconf)
*/

#ifdef getbegyx
#undef getbegyx
#endif
#define getbegyx(win,y,x)       ((y) = getbegy(win), (x) = getbegx(win))

#ifdef getmaxyx
#undef getmaxyx
#endif
#define getmaxyx(win,y,x)       ((y) = getmaxy(win), (x) = getmaxx(win))

#ifdef getbegy
#undef getbegy
#endif
#define getbegy(win)            ((win)->_begy)

#ifdef getbegx
#undef getbegx
#endif
#define getbegx(win)            ((win)->_begx)

#ifdef getmaxy
#undef getmaxy
#endif
#define getmaxy(win)            ((win)->_maxy)

#ifdef getmaxx
#undef getmaxx
#endif
#define getmaxx(win)            ((win)->_maxx)

#ifndef KEY_BTAB
#define KEY_BTAB	0541		/* Back tab key */
#endif
#ifndef KEY_END
#define KEY_END		0550		/* end key */
#endif
#ifndef KEY_EXIT
#define KEY_EXIT	0551		/* exit key */
#endif
#ifndef KEY_HELP
#define KEY_HELP	0553		/* help key */
#endif
#ifndef KEY_MARK
#define KEY_MARK	0554		/* mark key */
#endif
#ifndef KEY_SAVE
#define KEY_SAVE	0571		/* save key */
#endif

