#ifndef lint
static char sccsid[] = "@(#)85 1.1 src/bos/usr/bin/vfu/vfu_screen.c, cmdpios, bos411, 9428A410j 4/28/94 08:15:51";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS)
 *
 * FUNCTIONS: vfu_screen.c
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*                      
*
*			    SCREEN MANAGER MODULE
*		            (    vfu_screen.c   )
*
*	The aim is to control what is going on into vfu page and deal with
*	I/O operations i.e file salvage and pick-up . User can perform 
*	everything directly onto his interactive vfu page . Any functionnal 
*	key use is under control of elementary functions . This module is 
*	self_explanatory because of dense commentaries .
*	
*
*	WARNING : DO NOT ERASE ALL MESSAGES COMMENTED , THAT EASES MAINTENANCE 
*	
*	Author : BUI Q. M.
*	BULL S.A 
*	VERSION : 1.0 June 1990
*
*/

#include 	<curses.h>
#include 	<sys/types.h>
#include 	<unistd.h>
#include 	<memory.h>
#include 	<varargs.h>
#include 	<signal.h>
#include 	<sys/dir.h>
#include 	<sys/stat.h>
#include 	<fcntl.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<ctype.h>
#include 	"vfu.h" 

#define 	vfu_page_home_top(win)		wmove(win,0,XMAXCH)
#define 	vfu_page_home_bottom(win)	wmove(win,FULLPG,XMAXCH)

/* Broadcast main debug level to this module */
#define 	GET_DEBUG()	{ debug = get_debug(); \
				  mydebug = get_mydebug(); }

/* Variables related to curses */
static	WINDOW	 *vfuw = (WINDOW *) 0, /* vfu window */
		 *vfumsgw = (WINDOW *) 0, /* message window */
		 *vfunumw = (WINDOW *) 0, /* number window */
		 *SOSw = (WINDOW *) 0, /* help window */
		 *fow = (WINDOW *) 0, /* formlength window */
		 *fnw = (WINDOW *) 0, /* filename window */
		 *curses_mode = (WINDOW *) 0; /* curses mode */

/* Decoded array from davfu array and used for interactive edit */
static char 	**vfu_page_t; 
/* Coded array for downloading in davfu printer memory */
static unsigned char 	*davfu;

/* Global variables */
static int 	cur_pgrow,cur_pgcol; /* current row and column of shown page */
static int 	cur_pgtop,cur_pgbottom; /* current top and bottom of shown page */
static int 	cur_channel,cur_lnstop; /* current channel and row of davfu array */
static int 	msgpresent = 0; /* message presence */
static int 	helppresent = 0; /* help menu presence */
static int 	is_a_newone = 0; /* newfile flag */
static VFU_INFO info1, info2; /* informations structures */

static int 	folength; /* form length */
static char 	vfu_tmpstr1[MAXFILENAME+5]; /* temporary storage for filename */
static int 	in_default=0; /* global variable for default name flag */
static char 	vfu_tmpstr2[MAXDIGIT+1]; /* temporary storage for formlength */
static char 	fnpanic[BUFSIZE+1]; /* pathname for file */
static char	*vfuenv; /* VFU environment variable */
static char	**msgtb; /* pointer to msgtab */
static int 	fidesc; /* file descriptor */
static short 	all_is_set = IS_OFF; /* overall structures initialization flag */
static short 	advert = 0; /* output message flag */
static short 	master = 0; /* call master function flag */
static short 	dialogue ; /* country dialogue */
static int 	x_shift,y_shift; /* translation coordinates with regard to current screen size*/
static int 	vfu_printer, vfu_lpi; /* printer type, start code */
	
/******************/

/* Tracker arrays created at vfu start up */
/*               Linestop                 */
static char 	*track_same_lset;
/*               Channel                  */
static char 	track_same_cset[13]={
	'd','u','u','u','u','u','u','u','u','u','u','d','\0 '
			}; /* u for not done ; d for done */

/* Interactive debug variables */
static 		int debug,mydebug;

void 		vfu_boost_up();
void 		vfu_boost_down();
void 		vfu_draw_page();
VFU_INFO 	*vfu_catch();
void 		vfu_init_info();
void 		vfu_make_room_all();
void 		vfu_expand_room_all();
void 		vfu_init_all();
void 		vfu_message();
void 		vfu_escape_curses();
void 		vfu_close();
char 		*vfu_get_string();
int 		vfu_get_integer();
void 		vfu_view_page();
void 		vfu_trap_tprinter();
void 		vfu_fill_page_t();
void 		vfu_save_file();
void 		vfu_read_file();
void 		vfu_edit_page();
void 		vfu_operate();

/********************************************************************
 *                    INTERFACE FOR OTHER MODULES                   * 
 ********************************************************************/

char 
*get_adr_lset()
{
	return(track_same_lset);
}

char 
*get_adr_cset()
{
	return(track_same_cset);
}

char 
**get_adr_page_t()
{
	return(vfu_page_t);
}

unsigned char 
*get_adr_davfu()
{
	return(davfu);
}

char 
*get_adr_tmpstr1()
{
	return(vfu_tmpstr1);
}

void 
set_tmpstr1(str)
char 	*str;
{
	strcpy(vfu_tmpstr1,str);
}

char 
*get_adr_tmpstr2()
{
	return(vfu_tmpstr2);
}

void 
set_tmpstr2(val)
int 	val;
{
	folength = val;
	sprintf(vfu_tmpstr2,"%d\0",val);
}

char 
*get_adr_fnpanic()
{
	return(fnpanic);
}

void 
set_fnpanic(str)
char 	*str;
{
	strcpy(fnpanic,str);
}

char 
*get_adr_vfuenv()
{
	return(vfuenv);
}

void 
set_vfuenv(str)
char 	*str;
{
	strcpy(vfuenv,str);
}

int 
get_folength()
{
	return(folength);
}

void 
set_folength(val)
int 	val;
{
	folength = val ;
}

int 
get_all_is_set()
{
	return(all_is_set);
}

void 
set_all_is_set(val)
int 	val;
{
	all_is_set = val ;
}

/* Get curses flag */
WINDOW 
*get_curses_mode()
{
	return(curses_mode);
}

/* Top level call from vfu_scrollbar_manager.c */
void 
vfu_master_call_read()
{
	master = 1;
	vfu_read_file(vfumsgw,fidesc,davfu,fnpanic);
	/* determine printer type via first byte pair of davfu data array */
	if (davfu[0] == 0xEE) {
		vfu_printer = 54;
		if (davfu[1] == 0xEC) 
			vfu_lpi = 6;
		else
			vfu_lpi = 8;
		set_printer_type(vfu_printer);
		set_printer_lpi(vfu_lpi);
		/* reconfigure davfu data */
		vfu_expand_room_all(folength,folength,MAXCHANNEL);
	}
	doupdate();
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
}

void 
vfu_master_call_save()
{

	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	werase(vfumsgw);
	wnoutrefresh(vfumsgw);
	doupdate();

	if ((fidesc = open(fnpanic,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) < 0) {
		werase(vfumsgw);
		wrefresh(vfumsgw);
		syserr(vfumsgw,msgtb[48]); /* exit here */ 
		/* syserr(vfumsgw,"Cannot creat file -- Bye\n"); */
	}
	vfu_save_file(vfumsgw,fidesc,davfu);
	doupdate();
	close(fidesc);
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
}

void 
vfu_master_call_boostup()
{

	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	werase(vfumsgw);
	wnoutrefresh(vfumsgw);
	doupdate();

	vfu_boost_up(vfunumw,vfuw,vfu_page_t,FULLPG-1);
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
}

void 
vfu_master_call_boostdown()
{

	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	werase(vfumsgw);
	wnoutrefresh(vfumsgw);
	doupdate();

	vfu_boost_down(vfunumw,vfuw,vfu_page_t,FULLPG-1);
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
}

void 
vfu_master_call_clear_all()
{

	register int i;

	master = 1;
	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	werase(vfumsgw);
	wnoutrefresh(vfumsgw);
	doupdate();

	vfu_trap_tprinter();
	vfu_init_all(folength);

	/* initialize cursor coordinates into screen and array */
	cur_pgtop = 0;
	cur_pgrow = 0;
	cur_pgcol = XMAXCH;
	cur_lnstop = 0;
	cur_channel = (MAXCHANNEL - 1) - cur_pgcol/3;
	vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
	wmove(vfuw,cur_pgrow,cur_pgcol);
	wrefresh(vfuw);
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
	werase(vfumsgw);
	wrefresh(vfumsgw);
}

void 
vfu_master_no_operation()
{

	master = 1;
	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	werase(vfumsgw);
	werase(vfumsgw);
	wprintw(vfumsgw,"%s\n",msgtb[19]);
	/* wprintw(vfumsgw,"No file with ''vfu'' extension found in current directory\n"); */
	wnoutrefresh(vfumsgw);
	doupdate();

	/* initialize cursor coordinates into screen and array */
	cur_pgtop = 0;
	cur_pgrow = 0;
	cur_pgcol = XMAXCH;
	cur_lnstop = 0;
	cur_channel = (MAXCHANNEL - 1) - cur_pgcol/3;
	vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
	wmove(vfuw,cur_pgrow,cur_pgcol);
	wrefresh(vfuw);
	vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
	werase(vfumsgw);
	wrefresh(vfumsgw);
}

/* Get address of named window */
WINDOW 
*vfu_get_adr_window(val)
int 	val;
{
	switch(val) {
		case W_NAME:
			return(fnw);
		case W_FORM:
			return(fow);
		case W_NUM:
			return(vfunumw);
		case W_VFU:
			return(vfuw);
		case W_MSG:
			return(vfumsgw);
		default:
			if (!vfumsgw)
				vfumsgw = newwin(ROWMSGW,COLMSGW,YBEGMSGW,XBEGMSGW);
			syserr(vfumsgw,msgtb[45]);
			/* syserr(vfumsgw,"Cannot get address corresponding to this value"); */
	}
}

/*********************************************************************
 *                      VFU TOOLS FOR ARRAYS                         *
 *********************************************************************/

/* Init davfu array , tightly printer-dependent 
   Refer to Maintenance Guide for PR88 or PR54 */
void 
vfu_init_davfu(target,big)
unsigned char 	*target; /* binary array */
int 		big; /* size */
{
	register int i;
	chtype c;

	vfu_trap_tprinter();

	switch (vfu_printer) {
		case 88:
			for(i=3;i<big-4;i++) 
				target[i] = CH88_PAD;
			target[0] =  0x1D; /* GS code */
			target[1] =  CH88_1_7; /* TOF = channel 1*/
			target[2] =  CH88_PAD;
			target[big-6] =  CH88_PAD; /* BOF = channel 12 */
			target[big-5] =  CH88_6_12;
			target[big-4] =  CH88_1_7; /* dummy TOF */
			target[big-3] =  CH88_PAD;
			target[big-2] =  0x1E; /* RS code */
			target[big-1] =  '\0';
			break;

		case 54:
			get_printer_lpi();
			for(i=4;i<big-4;i++) 
				target[i] = CH54_PAD;

			/* Start code */
			target[0] =  0xEE;
			/* Start code LPI */
			target[1] =  (vfu_lpi == 6)?0xEC:0xED;
			target[2] =  CH54_1_7; /* TOF = channel 1*/
			target[3] =  CH54_PAD;
			target[big-4] =  CH54_PAD; /* BOF = channel 12 */
			target[big-3] =  CH54_6_12;
			target[big-2] =  0xEF; /* Stop code */
			target[big-1] =  '\0';
	}

}

/* Translate into real davfu code , tightly printer-dependent 
   Refer to DataProducts Maintenance Guide : X (DON'T CARE BIT) = 0 */
unsigned short 
vfu_channel_code(win,item)
WINDOW 	*win; /* message window */
int 	item; /* channel to be decoded */
{

	switch (item) {
		case 1 : 
		case 7 : 
			return((vfu_printer==88)?CH88_1_7:CH54_1_7);
		case 2 : 
		case 8 : 
			return((vfu_printer==88)?CH88_2_8:CH54_2_8);
		case 3 : 
		case 9 : 
			return((vfu_printer==88)?CH88_3_9:CH54_3_9);
		case 4 : 
		case 10 : 
			return((vfu_printer==88)?CH88_4_10:CH54_4_10);
		case 5 : 
		case 11 : 
			return((vfu_printer==88)?CH88_5_11:CH54_5_11);
		case 6 : 
		case 12 : 
			return((vfu_printer==88)?CH88_6_12:CH54_6_12);
		default:
			if (win){
				werase(win);
				wprintw(win,"-> %d\n",item);
				wrefresh(win);
			}
			else
				fprintf(stderr,"-> %d\n",item);

			warning(win,msgtb[41]);
			/* warning(win,"Invalid channel value\n"); */
	}
}

/* Intialize two_dimension array ; it's used for keeping track of any change
on screen */
void 
vfu_init_page_t(target,forml,large)
char 	**target; /* display array */
int 	forml,large; /* y_size and x_size */
{
	register int i,j;
	WINDOW *win= (WINDOW *)0;

	for(i=0;i<forml;i++) 
		if ((target[i]=(char *)malloc((large+1)*sizeof(char))) == NULL)
			syserr(win,msgtb[44]);
	for(i=0;i<forml;i++) {
		for(j=0;j<large;j++) 
			target[i][j] = '.';
		target[i][large] = '\0';
	}
	target[forml-1][large-1] = 'X';
	target[0][0] = 'X';
}

/* Make room all for processing */
void 
vfu_make_room_all(custom_size)
int 	custom_size; /* size */
{

	int 	lgdavfu;

	vfu_printer = get_printer_type();

        /* GS+Dummy TOF pair+RS+\0 for pr88, total=5 
           Start code+start code lpi+stop code+\0 for pr54, total=4 */
	lgdavfu = (vfu_printer==88)?2*custom_size+5:2*custom_size+4;
	/* make enough room and init arrays */
	if ((track_same_lset = (char *)malloc((custom_size+1)*sizeof(char))) == NULL) 
		syserr(vfumsgw,msgtb[44]);
	if ((davfu =(unsigned char *)malloc(lgdavfu*sizeof(unsigned char))) == NULL )
		syserr(vfumsgw,msgtb[44]);
	if ((vfu_page_t =(char **)malloc((custom_size)*sizeof(char *))) == NULL )
		syserr(vfumsgw,msgtb[44]);
}

/* Expand room all 
   It's possible to deal with any size with regard to the old */
void 
vfu_expand_room_all(custom_size,oldsize,large)
int 	custom_size,oldsize,large; /* sizes */
{

	int 	lgdavfu1, lgdavfu2, i;
	char 	*ptr1,**ptr3;
	unsigned char 	*ptr2;
	int	good_size;

	vfu_trap_tprinter();
	
	lgdavfu1 = (vfu_printer==88)?2*oldsize+5:2*oldsize+4;
	lgdavfu2 = (vfu_printer==88)?2*custom_size+5:2*custom_size+4;
	/* Make room for temporary arrays , initialize them */
	if ((ptr1 = (char *)malloc((oldsize+1)*sizeof(char))) == NULL) 
		syserr(vfumsgw,msgtb[44]);
	for(i=0;i<oldsize;i++)
		ptr1[i] = 'u';
	ptr1[oldsize] = '\0';

	if ((ptr2 =(unsigned char *)malloc(lgdavfu1*sizeof(unsigned char))) == NULL )
		syserr(vfumsgw,msgtb[44]);
	vfu_init_davfu(ptr2,2*oldsize+1+2+1+1);

	for(i=0;i<large;i++)
		track_same_cset[i] = 'u';
	track_same_cset[large] = '\0';

	if ((ptr3 =(char **)malloc((oldsize)*sizeof(char *))) == NULL )
		syserr(vfumsgw,msgtb[44]);
	vfu_init_page_t(ptr3,oldsize,large);

	/* temporary salvage */
	ptr1 = memcpy(ptr1,track_same_lset,oldsize);
	ptr2 = (unsigned char *)memcpy(ptr2,davfu,lgdavfu1);

	free(track_same_lset);
	free(davfu);
	free(vfu_page_t);

	/* make enough room with new size and init arrays */
	if ((track_same_lset = (char *)malloc((custom_size+1)*sizeof(char))) == NULL) 
		syserr(vfumsgw,msgtb[44]);
	if ((davfu =(unsigned char *)malloc(lgdavfu2*sizeof(unsigned char))) == NULL )
		syserr(vfumsgw,msgtb[44]);
	if ((vfu_page_t =(char **)malloc((custom_size)*sizeof(char *))) == NULL )
		syserr(vfumsgw,msgtb[44]);
	vfu_init_all(custom_size);

	/* memory copy */
	track_same_lset = memcpy(track_same_lset,ptr1,(custom_size > oldsize)?oldsize:custom_size);
	/* memory copy , except last linestop channel 12 byte pair , dummy BOF bytes pair , RS code and \0 for pr88 */
	if (vfu_printer==88) 
		good_size = (custom_size > oldsize)?lgdavfu1-6:lgdavfu2-6;
	else 
	/* memory copy , except last linestop channel 12 byte pair, Stop code and \0 for pr54 */
		good_size = (custom_size > oldsize)?lgdavfu1-4:lgdavfu2-4;
	/* memory copy , except stop code and \0 for pr88 */
	davfu = (unsigned char *)memcpy(davfu,ptr2,good_size);

	/* mapping */
	vfu_fill_page_t(davfu,lgdavfu2,vfu_page_t);
		
	free(ptr1);
	free(ptr2);
	free(ptr3);
}

/* Init all arrays used in system */
void 
vfu_init_all(size)
int 	size;
{
	register int 	i;
	int 	pr_flg;
	unsigned char	*tab;
	
	vfu_trap_tprinter();
	/* Channels 1 and 12 for TOF and BOF */
	for(i=1;i<size-1;i++)
		track_same_lset[i] = 'u';

	track_same_lset[0] = 'd';
	track_same_lset[size-1] = 'd';
	track_same_lset[size] = '\0';

	for(i=1;i<MAXCHANNEL-1;i++)
		track_same_cset[i] = 'u';

	track_same_cset[0] = 'd';
	track_same_cset[MAXCHANNEL-1] = 'd';
	track_same_cset[MAXCHANNEL] = '\0';

	/* shot structures */
	vfu_init_page_t(vfu_page_t,size,MAXCHANNEL);
	tab = davfu;
	pr_flg = (vfu_printer == 88)?2*size+5:2*size+4;
	vfu_init_davfu(tab,pr_flg);
}
	
/* Clear all arrays */
void 
vfu_clear_all()
{
	if (all_is_set)
		vfu_init_all(folength);
}

/* Code in davfu array */
int 
vfu_hard_code(win,tab,bound,row,channel)
WINDOW 	*win;
unsigned char 	*tab; /* binary array */
int 	bound,row,channel;
{
	vfu_printer = get_printer_type();

	/* bound parameter is passed without dummy TOF bytes pair */
	if (row > bound/2) { 
		if (win) {
			werase(win);
			wprintw(win,"%s\n",msgtb[16]);
			/* wprintw(win,"Limit reached,no change made\n"); */
			wrefresh(win);
			/* vfu_close(1,CLOSE_VFU); */
		}
		else
			fprintf(stderr,"%s\n",msgtb[16]);
			/* fprintf(stderr,"Limit reached,no change made\n"); */
		return(0);
	}

	/* switch code between PR88 and PR54 */
	if (channel <= 6) {
		tab[((vfu_printer == 88)?1:2)+2*(row-1)] |= vfu_channel_code(win,channel);
		tab[((vfu_printer == 88)?1:2)+2*(row-1)+1] &= N_MASK;
	}
	if (channel > 6) {
		tab[((vfu_printer == 88)?1:2)+2*(row-1)] &= N_MASK;
		tab[((vfu_printer == 88)?1:2)+2*(row-1)+1] |= vfu_channel_code(win,channel);
	}

	/*
	tab[((vfu_printer == 88)?1:2)+2*(row-1)] |= (channel > 6)?((vfu_printer==54)?CH54_PAD:CH88_PAD):vfu_channel_code(win,channel);
	tab[((vfu_printer == 88)?1:2)+2*(row-1)+1] |= (channel > 6)?vfu_channel_code(win,channel):((vfu_printer==54)?CH54_PAD:CH88_PAD);
	*/
	return(1);
}

/* Translate code : when read in core memory ,decode channel value in number */
int 
vfu_translate_code(win,val,flag)
WINDOW 	*win;
short 	val;
int 	flag; /* upper channel flag */
{
	int 	f;
	
	vfu_printer = get_printer_type();

	switch (vfu_printer) {
		case 88:
			switch (val) {
				case CH54_PAD : 
				case CH88_PAD : 
					return(-1);
				case CH88_1_7 : 
					return((flag == 0)?1:7);
				case CH88_2_8 : 
					return((flag == 0)?2:8);
				case CH88_3_9 : 
					return((flag == 0)?3:9);
				case CH88_4_10 : 
					return((flag == 0)?4:10);
				case CH88_5_11 : 
					return((flag == 0)?5:11);
				case CH88_6_12 : 
					return((flag == 0)?6:12);
				default:
					break;
			}
			break;

		case 54:
			switch (val) {
				case CH54_1_7 : 
					return((flag == 0)?1:7);
				case CH54_2_8 : 
					return((flag == 0)?2:8);
				case CH54_3_9 : 
					return((flag == 0)?3:9);
				case CH54_4_10 : 
					return((flag == 0)?4:10);
				case CH54_5_11 : 
					return((flag == 0)?5:11);
				case CH54_6_12 : 
					return((flag == 0)?6:12);
				default:
					break;
			}
			break;

		default:
			if (win) {
				werase(win);
				wprintw(win,"--> %02d : %s\n",vfu_printer,msgtb[62]);
				wrefresh(win);
				vfu_close(2,CLOSE_VFU);
			}
			else {
				fprintf(stderr,"--> %02d : %s\n",val,msgtb[62]);
				exit(2);
			}
	}

	if (win) {
		werase(win);
		wprintw(win,"--> %02x : %s\n",val,msgtb[46]);
		wrefresh(win);
		vfu_close(2,CLOSE_VFU);
	}
	else {
		fprintf(stderr,"--> %02x : %s\n",val,msgtb[46]);
		exit(2);
	}
}
	
/* Fill page to show on screen ;tbfrom array is used and 
   there is one-to-one map  in tbtarget 
   First byte pair is reserved for Top Of Form ,therefore 
   scan task start at location 2 */
void 
vfu_fill_page_t(tbfrom,dimfrom,tbtarget)
unsigned char 	*tbfrom;
char 	**tbtarget;
int 	dimfrom;
{
	int first;
	register int i,step;
	int lower_bound,upper_bound;
	unsigned char unused_byte,tmp_byte1,tmp_byte2;

	vfu_printer = get_printer_type();

	lower_bound = (vfu_printer==88)?1:2;
	upper_bound = (vfu_printer==88)?dimfrom-4:dimfrom-2;
	unused_byte = (vfu_printer==88)?CH88_PAD:CH54_PAD;

	/* skip GS code for pr88
           Start code and start code lpi for pr54 */
	i = lower_bound+2;
	/* while (i<dimfrom-2-1-1) { */
	while (i<upper_bound) { 
		first = 0;
		tmp_byte1 = tbfrom[i];
		tmp_byte2 = tbfrom[i+1];
		if ((tbfrom[i] == unused_byte) && (tbfrom[i+1] == unused_byte)) {
			/* Nothing to do if the two are unused_byte */
				i += 2;
				continue;
		}
		
		/* check bit = 0 or 1 by right shifting */
		for (step=0;step<6;step++) {
			if ( (tmp_byte1 & MASK_1) == MASK_1) {
				tbtarget[((vfu_printer ==88)?i:i-1)/2][step] = 'X'; 
				track_same_cset[step] = 'd';
			}
			else
				tbtarget[((vfu_printer ==88)?i:i-1)/2][step] = '.'; 
			if ( (tmp_byte2 & MASK_1) == MASK_1) {
				tbtarget[((vfu_printer ==88)?i:i-1)/2][step+6] = 'X'; 
				track_same_cset[5+step] = 'd';
			}
			else
				tbtarget[((vfu_printer ==88)?i:i-1)/2][step+6] = '.'; 
			tmp_byte1 >>= 1;
			tmp_byte2 >>= 1;

		}	
			
		/* impact tracker's arrays */
		track_same_lset[((vfu_printer == 88)?i:i-1)/2] = 'd'; 
		i += 2;
	}
}

/* Read file in memory core */
void 
vfu_read_file(win,fd,target,name)
WINDOW 	*win;
int 	fd; /* file descriptor */
unsigned char *target;
char 	*name;
{
	int 	nbr;
	register int 	i;
	char dial[20];

	strcpy(dial,(dialogue)?"LECTURE DU FICHIER":"READ FILE");
	if (win) {
		werase(win);
		wprintw(win,"%s %s ...\n",dial,name); 
		wnoutrefresh(win);
	}
	else
		fprintf(stdout,"%s %s ...\n",dial,name); 

	i = 0;
	while ((nbr=read(fd,&target[i],sizeof(target[i]))) > 0)
		++i;
	msgpresent = 1;
}

/* Save file */
void 
vfu_save_file(win,fd,target)
WINDOW 	*win;
int 	fd;
unsigned char *target;
{
	register int 	i;
	int 	nbw;
	char 	*myfic,*mydir,*get_adr_fnpanic();
	char dial1[20],dial2[10],*wheredir;

	strcpy(dial1,(dialogue)?"Ecriture du fichier":"Write file");
	strcpy(dial2,(dialogue)?"sur":"on");
	mydir = get_adr_fnpanic();
	if ((wheredir=(char *)malloc((strlen(mydir)+1)*sizeof(char))) == NULL)
		syserr(win,msgtb[44]);
	for(i=0;i<strlen(mydir)+1;i++)
		wheredir[i] = ' ';

	vfu_basename(wheredir,mydir,'/');
	wheredir[strlen(mydir)] = '\0';
	myfic = get_adr_tmpstr1();

	/* write file on disk */
	if (win != (WINDOW *) 0){
		werase(win);
		wprintw(win,"%s %s%s %s %s \n",dial1,myfic,SAME,dial2,wheredir);
		wnoutrefresh(win);
	}
	else
		fprintf(stdout,"%s %s%s %s %s \n",dial1,myfic,SAME,dial2,wheredir);

	for(i=0;i<2*folength+((get_printer_type()==88)?4:3);i++) {
		if ((nbw = write(fd,&target[i],sizeof(target[i]))) != sizeof(target[i])) {
			close(fd);
			syserr(win,msgtb[49]);
			/* syserr(win,"Trouble on saving file\n"); */
		}
	}

	msgpresent = 1;
	free(wheredir);
	return;
}

/* Delete all windows and exit with 'val' value or just close curses mode 
   to perform some shell task */
void 
vfu_close(val,flag)
int val,flag; /* exit code , vfu-quit-or-stay flag */
{
	delwin(fnw);
	delwin(fow);
	delwin(vfunumw);
	delwin(vfuw);
	delwin(vfumsgw);
	endwin();

	if (!flag) {
		fprintf(stdout,"%s\n",msgtb[17]); 
		/* fprintf(stdout,"Exit CURSES mode\n"); */
		return;
	}
	/* fprintf(stdout,"Exit VFU -- Bye \n"); */
	/* fprintf(stdout,"%s\n",msgtb[18]); */
	exit(val);
}

/* Catch msgtab address ; triggered from master module */
void
vfu_trap_adrmap()
{
	char **get_adr_map();

	msgtb = get_adr_map();
	dialogue = get_country_dialogue();
}

/* Catch vfu_printerand vfu_lpi values ; triggered from master module */
void
vfu_trap_tprinter()
{
	vfu_printer = get_printer_type();
	vfu_lpi = get_printer_lpi();
}

/********************************************************************
 *                 VFU TOOLS FOR DRAWING AND EDITING                *
 ********************************************************************/

/* HELP ME PLEASE , I AM LOST !!! */
static int 
vfu_help_me()
{
	chtype c;

	/* Window is created each time when user needs help */
	SOSw = newwin(YNBSOS,XNBSOS,0,x_shift+1+strlen("Filename :"));	
	if (dialogue){
		mvwaddstr(SOSw,0,0,"Utilisez les touches flechees pour deplacer le curseur ou appuyer sur la touche");
		mvwaddstr(SOSw,1,0,"u ou U ou - pour le deplacer vers le haut d'une ligne");
		mvwaddstr(SOSw,2,0,"d ou D ou + pour le deplacer vers le bas d'une ligne");
		mvwaddstr(SOSw,3,0,"p ou P pour defiler dix lignes vers le haut");
		mvwaddstr(SOSw,4,0,"n ou N pour defiler dix lignes vers le bas");
		mvwaddstr(SOSw,5,0,"l ou L ou < pour le deplacer d'une colonne vers la gauche");
		mvwaddstr(SOSw,6,0,"r ou R ou > pour le deplacer d'une colonne vers la droite");
		mvwaddstr(SOSw,7,0,"z ou Z ou 0 pour remettre a zero le champ");
		mvwaddstr(SOSw,8,0,"x ou X pour marquer le champ");
		mvwaddstr(SOSw,9,0,"e ou E pour basculer en mode menu barre deroulante");
		mvwaddstr(SOSw,10,0,"s ou S pour archiver le fichier");
		mvwaddstr(SOSw,11,0,"q ou Q pour quitter le programme");
		mvwaddstr(SOSw,12,0,"Appuyer sur RetourChariot pour continuer ...");
	}
	else {
		mvwaddstr(SOSw,0,0,"Use functionnal keys to move anywhere in shown page and scrollbar menu or strike key ");
		mvwaddstr(SOSw,1,0,"u or U or - to move up one row");
		mvwaddstr(SOSw,2,0,"d or D or + to move down one row");
		mvwaddstr(SOSw,3,0,"p or P to scroll up ten rows ");
		mvwaddstr(SOSw,4,0,"n or N to scroll down ten rows ");
		mvwaddstr(SOSw,5,0,"l or L or < to move backward one column");
		mvwaddstr(SOSw,6,0,"r or R or > to move forward one column");
		mvwaddstr(SOSw,7,0,"z or Z or 0 to zero array location");
		mvwaddstr(SOSw,8,0,"x or X to impact array");
		mvwaddstr(SOSw,9,0,"e or E to escape and toggle in scrollbar mode ");
		mvwaddstr(SOSw,10,0,"s or S to save file");
		mvwaddstr(SOSw,11,0,"q or Q to quit");
		mvwaddstr(SOSw,12,0,"Hit return to continue ...");
	}
	wrefresh(SOSw);
	helppresent = TRUE;
	c =getch();
	return(1);
}

/* Close curses mode */
void
vfu_escape_curses()
{
	werase(fnw);
	werase(fow);
	werase(vfunumw);
	werase(vfuw);
	werase(vfumsgw);
	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	wnoutrefresh(vfunumw);
	wnoutrefresh(vfuw);
	wnoutrefresh(vfumsgw);
	if (SOSw) {
		werase(SOSw);
		wrefresh(SOSw);
		delwin(SOSw);
	}
	werase(stdscr);
	wnoutrefresh(stdscr);
	doupdate();
	delwin(fnw);
	delwin(fow);
	delwin(vfunumw);
	delwin(vfuw);
	delwin(vfumsgw);
	endwin();
}

/* Make whole screen clean */
int
vfu_sighandler()
{
	extern errno;
	int dialg;

	signal(SIGINT,vfu_sighandler);
	vfu_escape_curses();
	if (dialogue)
		fprintf(stderr,"Signal INTERRUPT recu ... Terminaison du process\n");
	else
		fprintf(stderr,"INTERRUPT signal caught ... Process terminates\n");
	exit(errno);

}

/* Draw frame */
static void 
vfu_draw_frame()
{
	register int i;
	int y_off,x_off;

	getyx(stdscr,y_off,x_off);
	if (!dialogue) {
		mvwaddstr(stdscr,4+y_shift,20+x_shift,"LINE              C H A N N E L");
		mvaddstr(20+y_shift,1+x_shift,"Save         Files          PgUp          PgDown          Clear        Quit");
	}
	else {
		mvwaddstr(stdscr,4+y_shift,20+x_shift,"LIGNE         C   A    N    A    L");
		mvaddstr(20+y_shift,1+x_shift," Sauver       Fichier       PgHaut       PgBas       Effacer       Quitter");
	}
	mvwaddstr(stdscr,5+y_shift,27+x_shift,"12 11 10 9  8  7  6  5  4  3  2  1");
	mvwaddstr(stdscr,6+y_shift,25+x_shift," _____________________________________");
	for(i=0;i<FULLPG+1;i++)
		mvwaddstr(stdscr,i+7+y_shift,25+x_shift,"|                                     |");
	mvwaddstr(stdscr,i+7+y_shift,25+x_shift,"|_____________________________________|");
	mvaddstr(19+y_shift,1+x_shift,"____________________________________________________________________________");
	wmove(stdscr,y_off,x_off);
	refresh();

}
	
/* Draw vfu-page */
static void 
vfu_draw_page(nbpanel,pgpanel,num_toprow,target)
WINDOW *nbpanel,*pgpanel; /* windows where to draw current page of form */
char **target; /* target array */
int num_toprow; /* top number of current page */
{
	register int i,j; 
		
	GET_DEBUG();

	werase(nbpanel);
	werase(pgpanel);

	for(i=0;i<FULLPG;i++) {
		DEBUG(mydebug) {
			werase(vfumsgw);
			wprintw(vfumsgw,"top=%02d\n",cur_pgtop);
			wrefresh(vfumsgw);
			wmove(pgpanel,cur_pgrow,cur_pgcol);
			refresh();
			wrefresh(pgpanel);
		}
		if (cur_pgtop+i > folength -1) 
			break;
		mvwprintw(vfunumw,i,1,"%03.d",num_toprow+i+1); 
		for(j=0;j<MAXCHANNEL;j++)
			mvwaddch(vfuw,i,XMAXCH-j*XMAXNB,(chtype)vfu_page_t[num_toprow+i][j]);
	}
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
}

/* Reinitialize global coordinates */
void
vfu_restart_coordinates()
{

	cur_pgtop = 0;
	cur_pgrow = 0;
	cur_pgcol = XMAXCH;
	cur_lnstop = 0;
	cur_channel = (MAXCHANNEL - 1) - cur_pgcol/3;
}

/* Scroll page up nbscroll lines */
static void 
vfu_boost_up(nbpanel,pgpanel,target,nbscroll)
WINDOW *nbpanel,*pgpanel; /* windows where to draw current page of form */
char **target; /* target array */
int nbscroll;
{

	GET_DEBUG();

	if ((cur_pgtop-nbscroll) < 0) {
		cur_pgtop = 0;
		werase(vfumsgw);
		wprintw(vfumsgw,"%s\n",msgtb[21]);
		/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form\n"); */
		wrefresh(vfumsgw);
		beep();
		msgpresent = 1;
	}
	else 
		cur_pgtop -= nbscroll;
	
	cur_pgrow = 0;
	cur_pgcol = XMAXCH;
	vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
	cur_lnstop = 0;
	cur_channel = (MAXCHANNEL - 1) - cur_pgcol/3;
	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_boost_up :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
	
}

/* Scroll page down nbscroll lines */
static void 
vfu_boost_down(nbpanel,pgpanel,target,nbscroll)
WINDOW *nbpanel,*pgpanel; /* windows where to draw current page of form */
char **target; /* target array */
int nbscroll;
{

	GET_DEBUG();

	if ((cur_pgtop+nbscroll) > folength - 1) {
		wmove(pgpanel,cur_pgrow,cur_pgcol);
		wnoutrefresh(nbpanel);
		wnoutrefresh(pgpanel);
		beep();
		doupdate();
		return;
	}
	else {
		cur_pgtop += nbscroll;
		cur_pgrow = 0;
		cur_pgcol = XMAXCH;
	}
	werase(vfunumw);
	werase(vfuw);
	vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
	cur_lnstop = cur_pgtop + cur_pgrow;
	cur_channel = (MAXCHANNEL-1) - cur_pgcol/3 ;

	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_boost_down :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
	
}

/* Move forward to next item 
   If at end of linestop then stand on first location of next row
   If at end of vfu-page then update current top line , stand at first
   location of next row ; refresh vfu-page
   If at last linestop of file , wrap around */

static void 
vfu_next_col(nbpanel,pgpanel,target)
WINDOW 		*nbpanel,*pgpanel; /* windows where to draw current page of form */
char 		**target; /* target array */
{

	GET_DEBUG();

	if (cur_pgtop <= folength - 1 ) { 
		if (cur_pgcol < XMAXCH )
			cur_pgcol += 3;
		else { 
			if (cur_pgrow < FULLPG - 1 ) {
				/* scroll down if cursor at bottom of vfu page */
				if (cur_lnstop < folength - 1 )
					++cur_pgrow;
				cur_pgcol = 0;
			}
			else {
				/* if current channel is at bottom of form then force to stay where it is  */
				if (cur_lnstop == folength - 1 ) {
					wmove(pgpanel,cur_pgrow,cur_pgcol);
					wnoutrefresh(nbpanel);
					wnoutrefresh(pgpanel);
					beep();
					doupdate();
					return;
				}
				/* if top is less than bottom of form then increment it and redraw vfu page */
				if (cur_pgtop + FULLPG - 1 < folength - 1 ){
					++cur_pgtop ;
					vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
					cur_pgrow = FULLPG - 1;
					cur_pgcol = 0;
					DEBUG(mydebug) {
						werase(vfumsgw);
						mvwaddstr(vfumsgw,0,0,"Onbound\n");
						wrefresh(vfumsgw);
					}
				}
				/* force to stay where it is  */
				else {
					cur_pgtop = folength - 1;
					cur_pgrow = cur_pgcol = 0;
				/*	vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target); */
					DEBUG(mydebug) {
						werase(vfumsgw);
						mvwaddstr(vfumsgw,0,0,"Outbound\n");
						wrefresh(vfumsgw);
					}
				}
			}
		}
	}
	cur_lnstop = cur_pgtop + cur_pgrow;
	cur_channel = (MAXCHANNEL-1) - cur_pgcol/3 ;

	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_next_col :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
}

/* Move backward to previous item 
   If at beginning of linestop then stand on end location of previous row
   If at beginning of vfu-page then update current top line , stand at first
   location of previous row ; refresh vfu-page
   If at first linestop of file , wrap around */
static void 
vfu_back_col(nbpanel,pgpanel,target)
WINDOW 		*nbpanel,*pgpanel; /* windows where to draw current page of form */
char 		**target; /* target array */
{

	GET_DEBUG();

	if (cur_pgtop >= 0 ) { 
		/* decrement when between two boundaries of current row */
		if (cur_pgcol > 0)
			cur_pgcol -= 3;
		else { 
			/* move up one row if at beginning of row whenever current page row is not yet at top of vfu screen */
			if (cur_pgrow > 0)  {
				/* scroll down if cursor at bottom of page */
				--cur_pgrow;
				cur_pgcol = XMAXCH;
			}
			else {
				/* current page row is at top of vfu screen , update top and redraw vfu page */
				if (cur_pgtop  > 0 ){
					--cur_pgtop ;
					vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
					cur_pgrow = 0;
					cur_pgcol = XMAXCH;
					DEBUG(mydebug) {
						werase(vfumsgw);
						mvwaddstr(vfumsgw,0,0,"Onbound\n");
						wrefresh(vfumsgw);
					}
				}
				else {
					/* squeeze cursor to stay where it is */
					beep();
					cur_pgtop = 0;
					cur_pgrow =  0;
					cur_pgcol = XMAXCH;
					wmove(pgpanel,cur_pgrow,cur_pgcol);
					wrefresh(pgpanel);
					werase(vfumsgw);
					wprintw(vfumsgw,"%s\n",msgtb[21]);
					/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form\n"); */
					wrefresh(vfumsgw);
					msgpresent = 1;
					DEBUG(mydebug) {
						werase(vfumsgw);
						mvwaddstr(vfumsgw,0,0,"Outbound\n");
						wrefresh(vfumsgw);
					}
				}
			}
		}
	}

	cur_lnstop = cur_pgtop + cur_pgrow;
	cur_channel = (MAXCHANNEL-1) - cur_pgcol/3 ;

	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_back_col :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
}

/* Move forward to next row .
   If at bottom, stay and flash, otherwise move one step */ 
static void 
vfu_next_row(nbpanel,pgpanel,target)
WINDOW 		*nbpanel,*pgpanel; /* windows where to draw current page of form */
char 		**target; /* target array */
{

	GET_DEBUG();

	if (cur_pgrow < FULLPG - 1) {
		if (cur_lnstop < folength - 1 )
			++cur_pgrow;
	}
	else {
		if (cur_lnstop == folength - 1 ) {
			/* force cursor to stay where it is */
			wmove(pgpanel,cur_pgrow,cur_pgcol);
			wnoutrefresh(nbpanel);
			wnoutrefresh(pgpanel);
			beep();
			doupdate();
			return;
		}
		/* update top and redraw vfu screen */
		if (cur_pgtop + 1 < folength - 1) {
			++cur_pgtop;
			cur_pgrow = FULLPG - 1;
			werase(vfuw);
			werase(vfunumw);
			vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
		}
		else {
			beep();
			doupdate();
		}
	}
	cur_lnstop = cur_pgtop + cur_pgrow;
	cur_channel = (MAXCHANNEL-1) - cur_pgcol/3 ;

	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_next_row :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
}

/* Move backward to previous row.
   As above, but in reverse way */
static void 
vfu_back_row(nbpanel,pgpanel,target)
WINDOW 		*nbpanel,*pgpanel; /* windows where to draw current page of form */
char 		**target; /* target array */
{

	GET_DEBUG();

	if (cur_pgrow > 0) 
		--cur_pgrow;
	else {
		/* update top and redraw vfu screen */
		if (cur_pgtop > 0) {
			--cur_pgtop;
			cur_pgrow = 0;
			vfu_draw_page(nbpanel,pgpanel,cur_pgtop,target);
		}
		else {
			beep();
			werase(vfumsgw);
			wprintw(vfumsgw,"%s\n",msgtb[21]);
			/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form\n");  */
			wrefresh(vfumsgw);
			msgpresent = 1;
		}
	}
	cur_lnstop = cur_pgtop + cur_pgrow;
	cur_channel = (MAXCHANNEL-1) - cur_pgcol/3 ;

	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"vfu_back_row :row,col=%02d,%02d;top,ln,ch=%02d,%02d,%02d\n",cur_pgrow,cur_pgcol,cur_pgtop,cur_lnstop,cur_channel);
		wrefresh(vfumsgw);
	}
	wmove(pgpanel,cur_pgrow,cur_pgcol);
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate();
}


/* Edit vfu-page.
   When back in from scrollbar mode, set master flag */
static void 
vfu_edit_page(nbpanel,pgpanel,target2D,target1D)
WINDOW 		*nbpanel,*pgpanel;
char 		**target2D; /* 2-D store array of shown page and filename */
unsigned char 	*target1D; /* 1-D store array of code */
{
	chtype 	c,ca;
	int 	y_off,x_off;
	int 	davlg;
	int 	i,ln_count,ret_cmd = -2,there_are_files = -1,think;
	int vfu_command();
	unsigned char unused_byte;


	vfu_printer = get_printer_type();
	unused_byte = (vfu_printer==88)?CH88_PAD:CH54_PAD;

	GET_DEBUG();

	DEBUG(mydebug) 
		vfu_message(pgpanel,vfumsgw,"In vfu_edit_page\n");

	if (!master)
		vfu_page_home_top(pgpanel);
	keypad(pgpanel,TRUE);
	refresh();
	wnoutrefresh(nbpanel);
	wnoutrefresh(pgpanel);
	doupdate(); 

	if (!advert && !master ) {
		werase(vfumsgw);
		wprintw(vfumsgw,"%s\n",msgtb[21]);
		/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form \n"); */
		wrefresh(vfumsgw);
		msgpresent = 1;
		++advert;
	}

	while (1) {
		if (ret_cmd == -2 && !master)
			getyx(pgpanel,y_off,x_off);

		if (helppresent ) {
			/* preserve everything but SOS window */
			touchwin(stdscr);
			touchwin(pgpanel);
			touchwin(nbpanel);
			touchwin(fnw);
			touchwin(fow);
			delwin(SOSw);
			helppresent = FALSE;
			refresh();
			wnoutrefresh(fnw);
			wnoutrefresh(fow);
			wmove(pgpanel,cur_pgrow,cur_pgcol);
			wnoutrefresh(nbpanel);
			wnoutrefresh(pgpanel);
			doupdate();
		}
		
		if (ret_cmd == -2 && !master) {
			wmove(vfuw,y_off,x_off);
			wrefresh(pgpanel);
		}

		if (ret_cmd == 99 || master)  {
			master = 0;
			ret_cmd = -2;
			vfu_restart_coordinates();
			wmove(pgpanel,cur_pgrow,cur_pgcol); 
			wrefresh(pgpanel);
		}
		
		if (ret_cmd != -1 && ret_cmd != -2) 
			ret_cmd = -2;
	

		switch(c = wgetch(pgpanel)) {
			/* move to previous row */
			case KEY_UP:
				c = KEY_UP;
				break;
			/* move backward */
			case KEY_BACKSPACE:
			case KEY_BTAB:
			case KEY_LEFT:
				c = KEY_LEFT;
				break;
			/* move forward */
			case KEY_RIGHT:
				c = KEY_RIGHT;
				break;
			/* go home */
			case KEY_HOME:
				c = KEY_HOME;
				break;
			/* boost downward 10 rows */
			case KEY_NPAGE:
				c = KEY_NPAGE;
				break;
			/* boost upward 10 rows */
			case 'p':
			case KEY_PPAGE:
				c = KEY_PPAGE;
				break;
			/* move to next row */
			case KEY_ENTER:
			case KEY_DOWN:
				c = KEY_ENTER;
				break;
			/* escape vfu page */
			case KEY_EXIT :
				c = KEY_EXIT;
				break;
			/* save our soul !!! */ 
			case KEY_HELP:
				c = KEY_HELP;
				break;
			/* zero down item */
			case KEY_CLEAR:
				c = KEY_CLEAR;
				break;
			/* shoot */
			case KEY_MARK:
				c = KEY_MARK;
				break;
			/* salvage */
			case KEY_SAVE:
				c = KEY_SAVE;
				break;
			/* quit */
			case KEY_END :
				c = KEY_END;
				break;
			default :
				if ((char)c == 'u' || (char)c == 'U' || (char)c == '-'){
					c = KEY_UP;
					break;
				}
				if ((char)c == 'l' || (char)c == 'L' || (char)c == 'b' ||(char)c == '\b' || (char)c == '<'){
					c = KEY_LEFT;
					break;
				}
				if ((char)c == ' ' || (char)c == 'r' || (char)c == 'R' ||(char)c == '\t' || (char)c == '>'){
					c = KEY_RIGHT;
					break;
				}
				if ((char)c == 'n' || (char)c == 'N' || (char)c == '-'){
					c = KEY_NPAGE;
					break;
				}
				if ((char)c == 'p' || (char)c == 'P') {
					c = KEY_PPAGE;
					break;
				}
				if ((char)c == 'd' || (char)c == 'D' ||(char)c == '\n' ||(char)c == '\r' || (char)c == '+'){
					c = KEY_ENTER;
					break;
				}
				if ((char)c == 'e' || (char)c == 'E' ||c == (chtype)0x1B ){
					c = KEY_EXIT;
					break;
				}
				if ((char)c == 'h' || (char)c == 'H'){
					c = KEY_HELP;
					break;
				}
				if ((char)c == '0' || (char)c == 'z' ||(char)c == 'Z'){
					c = KEY_CLEAR;
					break;
				}
				if ((char)c == 'x' || (char)c == 'X'){
					c = KEY_MARK;
					break;
				}
				if ((char)c == 's' || (char)c == 'S'){
					c = KEY_SAVE;
					break;
				}
				if ((char)c == 'q' || (char)c == 'Q'){
					c = KEY_END;
					break;
				}
		}

		switch(c) {
			/* move to previous row */
			case KEY_UP:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_back_row(nbpanel,pgpanel,target2D);
				if (cur_lnstop == 0) {
					werase(vfumsgw);
					wprintw(vfumsgw,"%s\n",msgtb[21]);
					/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form (channel 1) \n"); */
					wrefresh(vfumsgw);
					msgpresent = 1;
				}
				break;

			/* move backward */
			case KEY_LEFT:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_back_col(nbpanel,pgpanel,target2D);
				if (cur_lnstop == 0 && cur_channel == 0) {
					werase(vfumsgw);
					wprintw(vfumsgw,"%s\n",msgtb[21]);
					/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form (channel 1) \n"); */
					wrefresh(vfumsgw);
					msgpresent = 1;
				}
				break;

			/* move forward */
			case KEY_RIGHT:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_next_col(nbpanel,pgpanel,target2D);
				if (cur_lnstop == folength-1 && cur_channel == MAXCHANNEL-1) {
					werase(vfumsgw);
					wprintw(vfumsgw,"%d%s\n",folength,msgtb[22]);
					/* wprintw(vfumsgw,"-th is reserved for Top Of Form (channel 12) \n",folength); */
					wrefresh(vfumsgw);
					msgpresent = 1;
				}
				break;

			/* go home */
			case KEY_HOME:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_page_home_top(pgpanel);
				cur_pgrow = cur_pgcol = 0;
				cur_lnstop = cur_pgtop + cur_pgrow;
				cur_channel = (MAXCHANNEL-1) - cur_pgcol/3;
				break;

			/* boost downward 10 rows */
			case KEY_NPAGE:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_boost_down(nbpanel,pgpanel,target2D,FULLPG-1);
				break;

			/* boost upward 10 rows */
			case KEY_PPAGE:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_boost_up(nbpanel,pgpanel,target2D,FULLPG-1);
				break;

			/* move to next row */
			case KEY_ENTER:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
				vfu_next_row(nbpanel,pgpanel,target2D);
				break;

			/* escape vfu page */
			case KEY_EXIT:
				werase(vfumsgw);
				wrefresh(vfumsgw);
				there_are_files = vfu_list_fname() ;
				getyx(vfuw,y_off,x_off); 
				break;

			/* save our soul !!! */ 
			case KEY_HELP:
				touchwin(stdscr);
				touchwin(pgpanel);
				touchwin(nbpanel);
				touchwin(fnw);
				touchwin(fow);
				refresh();
				wnoutrefresh(fnw);
				wnoutrefresh(fow);
				wnoutrefresh(nbpanel);
				wnoutrefresh(pgpanel);
				doupdate();
				vfu_help_me();
				break;

			/* shoot */
			case KEY_MARK:
				if (cur_channel == 0 || cur_channel == MAXCHANNEL-1) {
					if (cur_lnstop ==  0){
						werase(vfumsgw);
						wprintw(vfumsgw,"--> %d : %s\n",cur_lnstop+1,msgtb[23]);
						/* wprintw(vfumsgw,"Linestop 1 is reserved for Top Of Form (channel 1), command ineffective\n",cur_lnstop+1); */
						wrefresh(vfumsgw);
						msgpresent = 1;
						break;
					}
					if (cur_lnstop ==  folength-1){
						werase(vfumsgw);
						wprintw(vfumsgw,"--> %d : %d%s\n",cur_lnstop+1,cur_lnstop+1,msgtb[22]);
						/* wprintw(vfumsgw,"-th linestop is reserved for Bottom Of Form (channel 12), command ineffective\n",cur_lnstop+1); */
						wrefresh(vfumsgw);
						msgpresent = 1;
						break;
					}
				}

				mvwaddch(pgpanel,cur_pgrow,cur_pgcol,'X');
				wmove(pgpanel,cur_pgrow,cur_pgcol);
				/* Impact arrays right now */
				target2D[cur_lnstop][cur_channel] = 'X';
				/* switch code between PR88 and PR54 */
				if ( (cur_channel+1) <= 6) {
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop] |= vfu_channel_code(vfumsgw,cur_channel+1);
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop+1] &= N_MASK;
				}
				if ( (cur_channel+1) > 6) {
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop] &= N_MASK;
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop+1] |= vfu_channel_code(vfumsgw,cur_channel+1);
				}
				track_same_lset[cur_lnstop] = 'd';
				track_same_cset[cur_channel] = 'd';
				wnoutrefresh(nbpanel);
				wnoutrefresh(pgpanel);
				doupdate();
				break;

			/* zero down item */
			case KEY_CLEAR:
				/* check all but first linestop and last linestop*/
				if (cur_lnstop == 0 || cur_lnstop == (folength-1) ) {
					werase(vfumsgw);
					wprintw(vfumsgw,"--> %d : %s\n",cur_lnstop+1,msgtb[67]);
					/* wprintw(vfumsgw,"Linestop reserved, command ineffective\n",cur_lnstop+1); */
					wrefresh(vfumsgw);
					msgpresent = 1;
					break;
				}

				track_same_lset[cur_lnstop] = 'u';

				for(i=1,ln_count=0;i<folength-1;i++) {
					/* count number of lines 
                                           configured which correspond 
                                           to the channel to be cleared
                                           in order to blow it off if 
                                           there is only one */
					if (target2D[ln_count][cur_channel] == 'X')
						++ln_count;
				}

				/* free channel tracker if there is one selection */
				if ((i > 0) && (i < folength - 1) &&  ln_count == 1) 
					track_same_cset[cur_channel] = 'u';

				mvwaddch(pgpanel,cur_pgrow,cur_pgcol,'.');

				/* Impact arrays right now */
				target2D[cur_lnstop][cur_channel] = '.'; 

				wmove(pgpanel,cur_pgrow,cur_pgcol);
				if ((cur_channel + 1) <= 6) {
					unsigned char r;

					r = invert(target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop],cur_channel,1);
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop] = r;
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop+1] &= N_MASK;
				}
				if ((cur_channel + 1) > 6) {
					unsigned char r;

					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop] &= N_MASK;
					r = invert(target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop+1],cur_channel-6,1);
					target1D[((vfu_printer == 88)?1:2)+2*cur_lnstop+1] = r;
					
				}

				wrefresh(nbpanel);
				wrefresh(pgpanel);
				break;

			/* salvage */
			case KEY_SAVE:
				/* fnpanic is not already set */
				if (fnpanic[0] == ' ') {
					if (strrchr(vfu_tmpstr1,'/') == NULL || vfu_tmpstr1[0] != '/') 
						sprintf(fnpanic,"%s%s%s%s\0",vfuenv,"/",vfu_tmpstr1,SAME);
					/* assume absolute pathname */
					else 
						sprintf(fnpanic,"%s%s\0",vfu_tmpstr1,SAME);
				}

				if ((fidesc = open(fnpanic,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) < 0) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					syserr(vfumsgw,msgtb[48]);
					/* syserr(vfumsgw,"Cannot creat file -- Bye\n"); */
				}
				beep();
				refresh();
				/* update address when recursion is 
                                   triggled */
				target1D = davfu;
				vfu_save_file(vfumsgw,fidesc,target1D);
				doupdate();
				close(fidesc);
				getyx(vfuw,y_off,x_off); 
				break;

			/* quit */
			case KEY_END:
				if (msgpresent) {
					werase(vfumsgw);
					wrefresh(vfumsgw);
					msgpresent = 0;
				}
/*
#ifdef BULL_PRG
*/
				refresh();
/*
#endif
*/
				if (get_mydebug() > get_debug()) 
					vfu_close(0,CLOSE_VFU); 
				else 
					vfu_close(0,CLOSE_CURS); 
				break;

			default:
				beep();
				break;

		} /* switch */

		if (c == KEY_END) 
			break;

		if (c == KEY_EXIT) 
			ret_cmd = vfu_command() ; 

		/* file is selected, set printer type, its length, and display                    it*/
		if (ret_cmd == 99) {
			master = 1;
			werase(fnw);
			werase(fow);
			wprintw(fnw,"%s\n",vfu_tmpstr1);
			wprintw(fow,"%s\n",vfu_tmpstr2);
			wrefresh(fnw);
			wrefresh(fow);
			if (vfu_tmpstr1[0] != '/' || strrchr(vfu_tmpstr1,'/') == NULL)
				sprintf(fnpanic,"%s%s%s%s\0",vfuenv,"/",vfu_tmpstr1,SAME);
			else
				sprintf(fnpanic,"%s%s%s\0",vfuenv,vfu_tmpstr1,SAME);
			
			if ((fidesc = open(fnpanic,O_RDWR)) < 0) {
				werase(vfumsgw);
				wrefresh(vfumsgw);
				syserr(vfumsgw,msgtb[47]);
				/* syserr(vfumsgw,"Cannot open file -- Bye\n"); */
			}
			vfu_make_room_all(folength);
			vfu_init_all(folength);
			vfu_read_file(vfumsgw,fidesc,davfu,fnpanic);
			close(fidesc);
			if (davfu[0] == 0xEE) {
				vfu_printer = 54;
				if (davfu[1] == 0xEC) 
					vfu_lpi = 6;
				else
					vfu_lpi = 8;
				set_printer_type(vfu_printer);
				set_printer_lpi(vfu_lpi);
				/* reconfigure davfu data */
				vfu_expand_room_all(folength,folength,MAXCHANNEL);
			}
			doupdate();
			/* update davfu address */
			target1D = davfu;
			davlg = (vfu_printer == 88)?2*folength+5:2*folength+4;
			vfu_fill_page_t(davfu,davlg,vfu_page_t);
			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
		}

	} /* while */
	
	
	DEBUG(mydebug) {
		werase(vfumsgw);
		wprintw(vfumsgw,"Out vfu_edit_page\n");
		wrefresh(vfumsgw);
	}
}

/********************************************************************
 *                VFU TOOLS FOR DRIVING EDITION                     *
 ********************************************************************/
			
/* Deal with verbose option 
   Catch filename , formlength and set up interactively davfu array;
   Checks are done at any level : pathname , filename , formlength checks 
   If file exists , partial reuse of old file if formlength change is needed */ 
static void 
vfu_prime_edit()
{

	int 	myval,size_remake=0,myoldsize;
	struct 	stat bufstatfn; /* file information structure */
	chtype 	answer;
	int	pr_flg;

	GET_DEBUG();

	DEBUG(mydebug)
		mvwaddstr(vfumsgw,0,0,"In of vfu_prime_edit\n");

	werase(fnw);
	werase(fow);
	wmove(fnw,0,0);
	wnoutrefresh(fnw);
	wnoutrefresh(fow);
	doupdate();



mehr :
	strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));

	if (in_default) {
		if (access(PANICDIR,F_OK|W_OK) < 0)  { 
			werase(vfumsgw);
			wrefresh(vfumsgw);
			warning(vfumsgw," --> %s : ",PANICDIR);
			warning(vfumsgw,"%s\n",msgtb[36]);
			/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n"); */
		}
		sprintf(fnpanic,"%s%s%s",PANICDIR,"/",vfu_tmpstr1);
		strcat(fnpanic,SAME);
		werase(vfumsgw);
		wprintw(vfumsgw,"%s %s%s\n",msgtb[25],vfu_tmpstr1,SAME);
		/* wprintw(vfumsgw,"Nonexistent filename,default is %s%s\n",vfu_tmpstr1,SAME); */
		msgpresent = 1;
		wrefresh(vfumsgw);
		
		/* if ((fidesc = open(fnpanic,O_RDONLY)) < 0)  */
		if (access(fnpanic,F_OK) < 0) 
			/* got nothing , then this is the new one */
			is_a_newone = 1;
	}
	
	/* there's a normal name */
	else {
		if (strchr(vfu_tmpstr1,'/') == NULL)
			sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
		else {
			char *tempo,*m1;
			int	ix;

			if ((tempo = (char *)malloc((BUFSIZE+1)*sizeof(char))) == NULL)
				syserr(vfumsgw,msgtb[44]);
			for(ix=0;ix<BUFSIZE+1;ix++)
				*(tempo+ix) = ' ';
			if ((m1 = malloc((MAXFILENAME+4+1)*sizeof(char))) == NULL)
				syserr(vfumsgw,msgtb[44]);
			for(ix=0;ix<MAXFILENAME+5;ix++)
				*(m1+ix) = ' ';

			vfu_basename(m1,vfu_tmpstr1,'/');
			if (vfu_tmpstr1[0] != '/') {
				strcpy(tempo,"./");
				strcat(tempo,m1);
				sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
			}
			/* assume absolute pathname */
			else {
				strcpy(tempo,m1);
				sprintf(fnpanic,"%s\0",vfu_tmpstr1);
			}

			if (access(tempo,F_OK) < 0) { 
				if (msgpresent)
					werase(vfumsgw);
				wprintw(vfumsgw,"--> %s : %s\n",tempo,msgtb[26]);
				/* wprintw(vfumsgw,"Pathname corrupted in  %s or permission denied , try again please ...\n",tempo); */
				wnoutrefresh(vfumsgw);
				msgpresent = 1;
				werase(fnw);
				wnoutrefresh(fnw);
				doupdate();
				goto mehr;
			}

			free(tempo);
			free(m1);

		}

		/* creat full pathname */
		strcat(fnpanic,SAME);

		/* if do not exist then creat */
		if (access(fnpanic,F_OK) < 0) 
			is_a_newone = 1;
		/* if file exists then show its length */
		else {
			if ((fidesc = open(fnpanic,O_RDONLY)) < 0) { 
				if (msgpresent)
					werase(vfumsgw);
				wrefresh(vfumsgw);
				syserr(vfumsgw,msgtb[47]);
				/* syserr(vfumsgw,"Cannot open file -- Bye\n"); */
			}
			beep();
			if (msgpresent)
				werase(vfumsgw);
			wprintw(vfumsgw,"%s\n",msgtb[11]);
			/* wprintw(vfumsgw,"File exists,overwrite ? (yY/nN) ->  "); */
			wrefresh(vfumsgw);
			answer = (chtype)wgetch(vfumsgw);
			/* erase and get filename again */
			if (answer == 'N' || answer == 'n') {
				werase(vfumsgw);
				wrefresh(vfumsgw);
				werase(fnw);
				wrefresh(fnw);
				goto mehr;
			}

			/* get its file size  and display it */
			stat(fnpanic,&bufstatfn);
			/* PR88 davfu length is always even, PR54 odd */
			if (bufstatfn.st_size % 2 == 0)
				folength = (int)((bufstatfn.st_size-2)/2 - 1);
			else
				folength = (int)((bufstatfn.st_size-3)/2);
			sprintf(vfu_tmpstr2,"%d\0",folength);
			wprintw(fow,"%s",vfu_tmpstr2);
			wrefresh(fow);

			/* initialize everything */
			all_is_set = IS_ON;
			vfu_make_room_all(folength);
			vfu_init_all(folength);
			vfu_draw_frame();
			vfu_restart_coordinates();

			/* read file and map into vfu_page_t for displaying */
			vfu_read_file(vfumsgw,fidesc,davfu,fnpanic);
			/* determine printer type via first byte pair of davfu data array */
			if (davfu[0] == 0xEE) {
				vfu_printer = 54;
				if (davfu[1] == 0xEC) 
					vfu_lpi = 6;
				else
					vfu_lpi = 8;
				set_printer_type(vfu_printer);
				set_printer_lpi(vfu_lpi);
				/* reconfigure davfu data */
				vfu_expand_room_all(folength,folength,MAXCHANNEL);
			}
			doupdate();
			close(fidesc);
			pr_flg = (vfu_printer == 88)?2*folength+5:2*folength+4;
			vfu_fill_page_t(davfu,pr_flg,vfu_page_t);

			if (msgpresent)
				werase(vfumsgw);
			wprintw(vfumsgw,"%s\n",msgtb[27]);
			/* wprintw(vfumsgw,"Formlength is to be changed ? (yY/nN) ->  "); */
			wrefresh(vfumsgw);
			beep();
			answer = (chtype)wgetch(vfumsgw);

			/* formlength change */
			if (answer == 'y' || answer == 'Y' ) {
				werase(vfumsgw);
				wrefresh(vfumsgw);
				werase(fow);
				wrefresh(fow);
				size_remake = 1;
				goto more;
			}
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			goto ausg1;

		} /* else */
	} /* else */

more :
	myoldsize = folength;
	folength = vfu_get_integer(fow,vfumsgw,info2,YBEGFOW,XBEGFOW,MAXDIGIT,0);
	sprintf(vfu_tmpstr2,"%d\0",folength);
	if (folength < 3 || folength > MAXFORMLENGTH) {
		werase(fow);
		werase(vfumsgw);
		wnoutrefresh(fow);
		wnoutrefresh(vfumsgw);
		doupdate();
		vfu_message(fow,vfumsgw,"%s\n",msgtb[20]);
		/* vfu_message(fow,vfumsgw,"Value less than 2 or greater than 144 prohibited"); */
		goto more;
	}
	if (folength == DFLTLENGTH) {
		if (msgpresent)
			werase(vfumsgw);
		wrefresh(vfumsgw);
		vfu_message(fow,vfumsgw,"%s ",msgtb[9]);
		/* vfu_message(fow,vfumsgw,"Default length is "); */
		vfu_message(fow,vfumsgw,"%d \n",DFLTLENGTH);
	}
	
	if ( size_remake )  {
		all_is_set = IS_ON;
		/* room expension if same data and bigger size */
		vfu_expand_room_all(folength,myoldsize,MAXCHANNEL);
	}
	else {
		all_is_set = IS_ON;
		vfu_make_room_all(folength);
		vfu_init_all(folength);
	}

	/* redraw what has been changed or not */
	vfu_draw_frame();
	vfu_restart_coordinates();
	vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);

ausg1:
	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_prime_edit\n");
		wrefresh(vfumsgw);
	}

}

/* Operations beeing dealed with flags 
   When cfg flag is raised , assume that is a newfile ;
   there is no matter for ambiguity */
void 
vfu_operate(vfg,ffg,lfg,cfg)
int 	vfg,ffg,lfg,cfg;
{

	int 	ix,val,lgdavfu,overwrite=0;
	int 	myval,size_remake=0,myoldsize,mynewsize;
	chtype 	answer;
	struct 	stat bufstatfn;
	char 	tab[5],*envt;
	char 	*tempo,*m1;
	int 	pr_flg;

	y_shift = ((envt = getenv("LINES"))!= NULL )?((atoi(envt) > 25)?((atoi(envt)-25)/2-1):0):0;
	x_shift = ((envt = getenv("COLUMNS"))!= NULL )?((atoi(envt) > 80)?((atoi(envt)-80)/2-1):0):0;
/*
	y_shift = (y_shift>25)?((y_shift-25)/2-1):0;
	x_shift = (x_shift>80)?((x_shift-80)/2-1):0;
*/

	GET_DEBUG(); 
	signal(SIGINT,vfu_sighandler);

	if (helppresent) {
		/* preserve everything but SOS window */
		touchwin(stdscr);
		touchwin(vfuw);
		touchwin(vfunumw);
		touchwin(fnw);
		touchwin(fow);
		delwin(SOSw);
		helppresent = FALSE;
		refresh();
		wnoutrefresh(fnw);
		wnoutrefresh(fow);
		wmove(vfunumw,cur_pgrow,cur_pgcol);
		wnoutrefresh(vfunumw);
		wnoutrefresh(vfuw);
		doupdate();
	}

	/* Process upon combinations of flag values */
	sprintf(tab,"%d%d%d%d\0",vfg,ffg,lfg,cfg);

	/* Creat new windows before entering interactive job */
	curses_mode = initscr();
	vfuw = newwin(ROWW,COLW,YBEGW+y_shift,XBEGW+x_shift);	
	vfunumw = newwin(ROWNUMW,COLNUMW,YBEGNUMW+y_shift,XBEGNUMW+x_shift);
	vfumsgw = newwin(ROWMSGW,COLMSGW,YBEGMSGW+y_shift,XBEGMSGW+x_shift);
	SOSw = newwin(ROWSOSW,COLSOSW,YBEGSOSW+y_shift,XBEGSOSW+x_shift);
	fnw = newwin(ROWFNW,COLFNW,YBEGFNW+y_shift,XBEGFNW+x_shift);
	fow = newwin(ROWFOW,COLFOW,YBEGFOW+y_shift,XBEGFOW+x_shift);

	noecho();
	cbreak();

	if (!dialogue) {
		mvaddstr(1+y_shift,1+x_shift,"Filename :");
		mvaddstr(2+y_shift,1+x_shift,"Formlength :");
		mvaddstr(10+y_shift,2+x_shift,"Type :");
		mvaddstr(11+y_shift,2+x_shift,"h(H) for HELP");
		mvaddstr(12+y_shift,2+x_shift,"e(E) for ");
		mvaddstr(13+y_shift,2+x_shift,"ESCAPE and use");
		mvaddstr(14+y_shift,2+x_shift,"scrollbar menu\n");
	}
	else {
		mvaddstr(1+y_shift,1+x_shift,"    Nom  :");
		mvaddstr(2+y_shift,1+x_shift,"  Longueur :");
		mvaddstr(10+y_shift,2+x_shift,"Taper :");
		mvaddstr(11+y_shift,2+x_shift,"h(H) pour AIDE");
		mvaddstr(12+y_shift,2+x_shift,"e(E) pour ");
		mvaddstr(13+y_shift,2+x_shift,"   utiliser");
		mvaddstr(14+y_shift,2+x_shift,"   le menu");
		mvaddstr(15+y_shift,2+x_shift,"  deroulant");
	}
	refresh();
	vfu_draw_frame();

	if ((vfuenv=(char *)malloc(BUFSIZE*sizeof(char))) == NULL)
		syserr(vfumsgw,msgtb[44]);

	if (getenv("VFU") != NULL) 
		strcpy(vfuenv,getenv("VFU"));
	else 
		if ((envt=getcwd(vfuenv,BUFSIZE)) == NULL)
			warning(vfumsgw,msgtb[35]);
			/* warning(vfumsgw,"Working directory search fails -- Bye\n"); */

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In of vfu_operate\n");
		wrefresh(vfumsgw);
	}

	val = atoi(tab);

	switch(val) {

		/* short way to use interactive facility */
		case 1000 :
			vfu_prime_edit();
			/* fidesc and fnpanic already set */
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
			break;

		/* edit default filename and formlength, ready to make room and
                   shot arrays !!! */
		/* in this case , it is a new file */
		case 1010 :
			if (access(PANICDIR,F_OK|W_OK) < 0) { 
				werase(vfumsgw);
				wprintw(vfumsgw,"--> %s : ",PANICDIR);
				wrefresh(vfumsgw);
				warning(vfumsgw,msgtb[36]);
				/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n");  */
			}
			sprintf(vfu_tmpstr1,"%s\0",mktemp("vfuXXXXXX"));
			sprintf(fnpanic,"%s%s",PANICDIR,"/");
			strcat(fnpanic,vfu_tmpstr1);
			strcat(fnpanic,SAME);
			werase(fnw);
			werase(fow);
			wprintw(fnw,"%s",vfu_tmpstr1);
			wprintw(fow,"%s",vfu_tmpstr2);
			wnoutrefresh(fnw);
			wnoutrefresh(fow);
			doupdate();
			beep();
			msgpresent = 1;
			is_a_newone = 1;
			folength = atoi(vfu_tmpstr2);
			vfu_message(vfuw,vfumsgw,"%s",msgtb[7]);
			/* vfu_message(vfuw,vfumsgw,"Default filename is given\n"); */
			vfu_message(vfuw,vfumsgw," %s\n",vfu_tmpstr2);
			
			/* initialize all */
			all_is_set = IS_ON;
			vfu_make_room_all(folength);
			vfu_init_all(folength);
			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
			break;

		/* edit filename and catch formlength, ready to make room and 
                   shot arrays !!! */
		case 1100 :
			/* Fill up filename  */
			if (strlen(vfu_tmpstr1) == 0)
				sprintf(vfu_tmpstr1,"%s\0",mktemp("vfuXXXXXX"));
			werase(fnw);
			wprintw(fnw,"%s",vfu_tmpstr1);
			wrefresh(fnw);

			/* filename check in order to set up full pathname */

		noch:
			if (in_default) {
				if (access(PANICDIR,F_OK|W_OK) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wrefresh(vfumsgw);
					wprintw(vfumsgw,"--> %s : ",PANICDIR);
					warning(vfumsgw, msgtb[36]);
					/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n");  */
				}
				sprintf(fnpanic,"%s%s%s",PANICDIR,"/",vfu_tmpstr1);
				goto more1;				
			}
			if (strchr(vfu_tmpstr1,'/') == NULL)
				sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
			else {

				if ((tempo = malloc((BUFSIZE+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);
				for(ix=0;ix<BUFSIZE+1;ix++)
					*(tempo+ix) = ' ';
				if ((m1 = malloc((MAXFILENAME+4+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);

				vfu_basename(m1,vfu_tmpstr1,'/');

				if (vfu_tmpstr1[0] != '/') {
					sprintf(tempo,"%s%s","./",m1);
					sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
				}
				/* assume absolute pathname */
				else {
					sprintf(tempo,"%s",m1);
					sprintf(fnpanic,"%s\0",vfu_tmpstr1);
				}

				/* something wrong in pathname */
				if (access(tempo,F_OK|W_OK) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wprintw(vfumsgw,"--> %s : %s\n",tempo,msgtb[26]);
					/* wprintw(vfumsgw,"Pathname corrupted in  %s or permission denied , try again please ...\n",tempo); */
					wnoutrefresh(vfumsgw);
					msgpresent = 1;
					werase(fnw);
					wnoutrefresh(fnw);
					doupdate();
					/* recompute */
					goto mehr1;
				}

				free(tempo);
				free(m1);

			}

			/* unicity filename check */
			strcat(fnpanic,SAME);
			/* OK now next step ! */
			goto blow0;

		mehr1:
			strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));
			/* check again */
			goto noch;

		blow0 :
			/* existence check */
			if (access(fnpanic,F_OK) < 0) {
				/* got nothing */
				is_a_newone = 1;
				/* catch formlength */
				goto more1;
			}

		side :
			/* if file exists then query to overwrite */
			beep();
			if (msgpresent)
				werase(vfumsgw);
			wprintw(vfumsgw,"%s\n",msgtb[11]);
			/* wprintw(vfumsgw,"File exists,overwrite ? (yY/nN) ->  "); */
			wrefresh(vfumsgw);
			answer = (chtype)wgetch(vfumsgw);
			werase(vfumsgw);
			wrefresh(vfumsgw);

			/* get filename again */
			if (answer == 'N' || answer == 'n') {
				werase(fnw);
				wrefresh(fnw);
				goto mehr1;
			}

			/* get file size  and display it */
			stat(fnpanic,&bufstatfn);
			if (bufstatfn.st_size % 2 == 0)
				folength = (int)((bufstatfn.st_size-2)/2 - 1);
			else
				folength = (int)((bufstatfn.st_size-3)/2);
			sprintf(vfu_tmpstr2,"%d\0",folength);
			werase(fow);
			wprintw(fow,"%s",vfu_tmpstr2);
			wrefresh(fow);

			/* initialize everything */
			all_is_set = IS_ON;
			vfu_make_room_all(folength);
			vfu_init_all(folength);

			if ((fidesc = open(fnpanic,O_RDONLY)) < 0) {
				werase(vfumsgw);
				wrefresh(vfumsgw);
				syserr(vfumsgw,msgtb[47]);
				/* syserr(vfumsgw,"Cannot open file -- Bye\n"); */
			}
			/* read file and map into vfu_page_t for displaying */
			vfu_read_file(vfumsgw,fidesc,davfu,fnpanic);
			/* check printer type via first byte pair of davfu 
                           data array */
			if (davfu[0] == 0xEE) {
				vfu_printer = 54;
				if (davfu[1] == 0xEC) 
					vfu_lpi = 6;
				else
					vfu_lpi = 8;
				set_printer_type(vfu_printer);
				set_printer_lpi(vfu_lpi);
				/* reconfigure davfu data */
				vfu_expand_room_all(folength,folength,MAXCHANNEL);
			}
			doupdate();
			close(fidesc);
			pr_flg = (vfu_printer == 88)?2*folength+5:2*folength+4;
			vfu_fill_page_t(davfu,pr_flg,vfu_page_t);

			werase(vfumsgw);
			wprintw(vfumsgw,"%s\n",msgtb[27]);
			/* wprintw(vfumsgw,"Formlength is to be changed ? (yY/nN) ->  ") */
			wrefresh(vfumsgw);
			beep();
			answer = (chtype)getch();
			werase(vfumsgw);
			wrefresh(vfumsgw);

			/* formlength change */
			if (answer == 'y' || answer == 'Y' ) {
				werase(fow);
				wrefresh(fow);
				size_remake = 1;
				goto more1;
			}
			/* no change , edit */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			goto ausg1;


		more1 :
			if (!is_a_newone)
				myoldsize = folength;

			folength = vfu_get_integer(fow,vfumsgw,info2,YBEGFOW,XBEGFOW,MAXDIGIT,0);
			sprintf(vfu_tmpstr2,"%d\0",folength);
			if (folength < 3 || folength > MAXFORMLENGTH) {
				werase(fow);
				werase(vfumsgw);
				wnoutrefresh(fow);
				wnoutrefresh(vfumsgw);
				doupdate();
				vfu_message(fow,vfumsgw,"%s\n",msgtb[20]);
				/* vfu_message(fow,vfumsgw,"Value less than 2 or greater than 144 prohibited"); */
				goto more1;
			}
			itoa(folength,vfu_tmpstr2);
			if (folength == DFLTLENGTH) {
				sprintf(vfu_tmpstr2,"%d\0",DFLTLENGTH);
				werase(vfumsgw);
				wrefresh(vfumsgw);
				vfu_message(fow,vfumsgw,"%s %d",msgtb[9],DFLTLENGTH);
				/* vfu_message(fow,vfumsgw,"--> %d\n",DFLTLENGTH); */
				/* vfu_message(fow,vfumsgw,"Default value %d is taken into account \n",DFLTLENGTH); */
			}
			
			if ( size_remake ) 
				vfu_expand_room_all(folength,myoldsize,MAXCHANNEL);
			else {
				all_is_set = IS_ON;
				vfu_make_room_all(folength);
				vfu_init_all(folength);
			}

			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);

		ausg1: 
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
			break;

		/* edit default filename and formlength and shot arrays !!! */
		/* in this case , it is a new file and arrays have been 
                   initialized */
		case 1011 :
		case 1001 :
			if (access(PANICDIR,F_OK|W_OK) < 0)  {
				wprintw(vfumsgw,"--> %s : ",PANICDIR);
				warning(vfumsgw, msgtb[36]);
				/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n");  */
			}
			sprintf(vfu_tmpstr1,"%s\0",mktemp("vfuXXXXXX"));
			sprintf(fnpanic,"%s%s",PANICDIR,"/");
			strcat(fnpanic,vfu_tmpstr1);
			strcat(fnpanic,SAME);
			werase(fnw);
			werase(fow);
			wprintw(fnw,"%s",vfu_tmpstr1);
			werase(vfumsgw);
			wprintw(vfumsgw,"%d",folength);
			wnoutrefresh(vfumsgw);
			sprintf(vfu_tmpstr2,"%d\0",folength);
			wprintw(fow,"%s",vfu_tmpstr2);
			wnoutrefresh(fnw);
			wnoutrefresh(fow);
			doupdate();
			is_a_newone = 1;

			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			pr_flg = (vfu_printer == 88)?2*folength+5:2*folength+4;
			vfu_fill_page_t(davfu,pr_flg,vfu_page_t);
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			if (!ffg) 
				vfu_message(vfuw,vfumsgw,"%s\n",msgtb[50]);
				/* vfu_message(vfuw,vfumsgw,"Default filename is given\n"); */
			if (!ffg && !lfg)
				vfu_message(vfuw,vfumsgw,"%s\n",msgtb[51]);
				/* vfu_message(vfuw,vfumsgw,"Default filename and formlength are given\n"); */
		
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
			break;

		/* edit filename and default formlength and shot arrays !!! */
		/* in this case , it is a new file */
		case 1101 :
		begg :
			werase(fnw);
			wprintw(fnw,"%s",vfu_tmpstr1);
			wnoutrefresh(fnw);

			if (in_default) {
				if (access(PANICDIR,F_OK|W_OK) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wrefresh(vfumsgw);
					wprintw(vfumsgw,"--> %s : %s\n",PANICDIR,msgtb[36]);
					/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n");  */
				}
				sprintf(fnpanic,"%s%s%s",PANICDIR,"/",vfu_tmpstr1);
				werase(vfumsgw);
				wprintw(vfumsgw,"%s %s%s\n",msgtb[7],vfu_tmpstr1,SAME);
				/* wprintw(vfumsgw,"Default filename is %s%s\n",vfu_tmpstr1,SAME); */
				msgpresent = 1;
				wrefresh(vfumsgw);
			}
			if (strchr(vfu_tmpstr1,'/') == NULL)
				sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
			else {

				if ((tempo = malloc((BUFSIZE+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);
				for(ix=0;ix<BUFSIZE+1;ix++)
					*(tempo+ix) = ' ';
				if ((m1 = malloc((MAXFILENAME+4+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);

				vfu_basename(m1,vfu_tmpstr1,'/');

				if (vfu_tmpstr1[0] != '/') {
					sprintf(tempo,"%s%s","./",m1);
					sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
				}
				/* assume absolute pathname */
				else {
					sprintf(tempo,"%s",m1);
					sprintf(fnpanic,"%s\0",vfu_tmpstr1);
				}

				/* something wrong in pathname */
				if (access(tempo,F_OK|W_OK) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wprintw(vfumsgw,"--> %s : %s\n",tempo,msgtb[26]);
					/* wprintw(vfumsgw,"Pathname corrupted in  %s  or permission denied  , try again please ...\n",tempo); */
					wnoutrefresh(vfumsgw);
					msgpresent = 1;
					werase(fnw);
					wnoutrefresh(fnw);
					doupdate();
					strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));
					goto begg;
				}

				free(tempo);
				free(m1);

			}

			strcat(fnpanic,SAME);
			werase(fow);
			wprintw(fow,"%s",vfu_tmpstr2);
			wnoutrefresh(fow);
			doupdate();

			/* if do not exist then creat  */
			if (access(fnpanic,F_OK|W_OK) > 0) {
				beep();
				if (msgpresent)
					werase(vfumsgw);
				wprintw(vfumsgw,"%s\n",msgtb[28]);
				/* wprintw(vfumsgw,"Filename exists , choose another one please\n"); */
				wnoutrefresh(vfumsgw);
				msgpresent = 1;
				werase(fnw);
				wnoutrefresh(fnw);
				doupdate();
				strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));
				goto begg;
			}

			is_a_newone = 1;
			beep();
			vfu_message(fnw,vfumsgw,"%s\n",msgtb[50]);
			/* vfu_message(fnw,vfumsgw,"Default formlength is given\n"); */

			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);
			break;

		/* edit filename and formlength and shot arrays 
		   case 1111 : assume that is a new file and create new one 
                               after filename check
		   case 1110 : overwrite job if file exists */
		case 1110 :
		case 1111 :
			/* file check and shot arrays !!! */
		beg1 :
			werase(fnw);
			wprintw(fnw,"%s",vfu_tmpstr1);
			wrefresh(fnw);

			if (in_default) {
				if (access(PANICDIR,F_OK|W_OK ) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wnoutrefresh(vfumsgw);
					wprintw(vfumsgw,"--> %s : ",PANICDIR);
					wnoutrefresh(vfumsgw);
					doupdate();
					warning(vfumsgw, msgtb[36]);
					/* warning(vfumsgw,"Directory /usr/local doesn't exist or permission denied\n");  */
				}
				sprintf(fnpanic,"%s%s%s",PANICDIR,"/",vfu_tmpstr1);
				werase(vfumsgw);
				wprintw(vfumsgw,"%s %s%s\n\n",msgtb[7],vfu_tmpstr1,SAME);
				/* wprintw(vfumsgw,"Default filename is %s%s\n",vfu_tmpstr1,SAME); */
				msgpresent = 1;
				wrefresh(vfumsgw);
				goto more;				
			}
			if (strchr(vfu_tmpstr1,'/') == NULL)
				sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);

			else {

				if ((tempo = malloc((BUFSIZE+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);
				for(ix=0;ix<BUFSIZE+1;ix++)
					*(tempo+ix) = ' ';
				if ((m1 = malloc((MAXFILENAME+4+1)*sizeof(char))) == NULL)
					syserr(vfumsgw,msgtb[44]);

				vfu_basename(m1,vfu_tmpstr1,'/');

				if (vfu_tmpstr1[0] != '/') {
					sprintf(tempo,"%s%s","./",m1);
					sprintf(fnpanic,"%s%s%s\0",vfuenv,"/",vfu_tmpstr1);
				}
				/* assume absolute pathname */
				else {
					sprintf(tempo,"%s",m1);
					sprintf(fnpanic,"%s\0",vfu_tmpstr1);
				}

				if (access(tempo,F_OK|W_OK) < 0) {
					if (msgpresent)
						werase(vfumsgw);
					wprintw(vfumsgw,"--> %s : %s\n",tempo,msgtb[26]);
					/* wprintw(vfumsgw,"Pathname corrupted in  %s or no write permision , try again please ...\n",tempo); */
					wnoutrefresh(vfumsgw);
					msgpresent = 1;
					werase(fnw);
					wnoutrefresh(fnw);
					doupdate();
					strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));
					goto beg1;
				}

				free(tempo);
				free(m1);

			}

			strcat(fnpanic,SAME);

			/* do not exist , creat */
			if (access(fnpanic,F_OK|W_OK) < 0) 
				is_a_newone = 1;
			/* file exists , query user to overwrite it  */
			else {
				beep();
				werase(vfumsgw);
				wprintw(vfumsgw,"%s",msgtb[11]);
				/* wprintw(vfumsgw,"File exists,overwrite ? (yY/nN) ->  "); */
				wrefresh(vfumsgw);
				msgpresent = 1;
				answer = (chtype)wgetch(vfumsgw);
				werase(vfumsgw);
				wrefresh(vfumsgw);
				if (answer == 'N' || answer == 'n') {
					/* try to reseize filename */
					werase(fnw);
					wrefresh(fnw);
					strcpy(vfu_tmpstr1,vfu_get_string(fnw,vfumsgw,info1,YBEGFNW,XBEGFNW,MAXFILENAME,0));
					/* check again */
					goto beg1;
				}
				overwrite = 1;
			}

			goto more;

		again :
			/* get value */
			folength = vfu_get_integer(fow,vfumsgw,info2,YBEGFOW,XBEGFOW,MAXDIGIT,0);
			if (folength < 3 || folength > MAXFORMLENGTH) {
				werase(fow);
				werase(vfumsgw);
				wnoutrefresh(fow);
				wnoutrefresh(vfumsgw);
				doupdate();
				vfu_message(fow,vfumsgw,"%s\n",msgtb[20]);
				/* vfu_message(fow,vfumsgw,"Value less than 3 or greater than 144 prohibited"); */
				goto again;
			}

		more:
			/* discrimination between case 1111 and 1110 */
			if (cfg) {
				folength = atoi(vfu_tmpstr2);
				werase(fow);
				wprintw(fow,"%s",vfu_tmpstr2);
				wrefresh(fow);
				goto isnew;
			}
			else
				mynewsize = atoi(vfu_tmpstr2);

			/* get size only if file exists i.e. flag overwrite = 1*/
			if (overwrite) {
				stat(fnpanic,&bufstatfn);
				/* PR88 davfu length is always even, PR54 odd */
				if (bufstatfn.st_size % 2 == 0)
					myoldsize = (int)((bufstatfn.st_size-2)/2 - 1);
				else
					myoldsize = (int)((bufstatfn.st_size-3)/2);
			}
			/* translate string in integer  */
			sprintf(vfu_tmpstr2,"%d\0",((overwrite)?mynewsize:myoldsize));
			if (is_a_newone)
				sprintf(vfu_tmpstr2,"%d\0",mynewsize);
			/* at that stage , update recently  */
			folength = atoi(vfu_tmpstr2);
			werase(fow);
			wprintw(fow,"%s",vfu_tmpstr2);
			wrefresh(fow);

		isnew:
			/* formlength check */
			if (folength < 3 || folength > MAXFORMLENGTH) {
				werase(fow);
				werase(vfumsgw);
				wnoutrefresh(fow);
				wnoutrefresh(vfumsgw);
				doupdate();
				vfu_message(fow,vfumsgw,"%s\n",msgtb[20]); 
				/* vfu_message(fow,vfumsgw,"Value less than 2 or greater than 144 prohibited");  */
				/* try to catch formlength again */
				goto again;
			}
			if (folength == DFLTLENGTH) {
				if (msgpresent)
					werase(vfumsgw);
				wrefresh(vfumsgw);
				vfu_message(fow,vfumsgw,"%s",msgtb[9]);
				vfu_message(fow,vfumsgw," %d\n",DFLTLENGTH);
				/*
				vfu_message(fow,vfumsgw,"Default value %d is taken into account \n",DFLTLENGTH);
				*/
			}
			
			if (!cfg && is_a_newone) {
				all_is_set = IS_ON;
				vfu_make_room_all(folength);
				vfu_init_all(folength);
			}

			/* overwrite davfu */
			if (!cfg && overwrite ) {
				/* if new and old size are different, make room
                                   with old size and further expand arrays */
				if (mynewsize != myoldsize) {
					vfu_make_room_all(myoldsize);
					vfu_init_all(myoldsize);
				}
				/* if the size is unchanged, just make room and
                                   read file and fill vfu-page */
				else {
					vfu_make_room_all(folength);
					vfu_init_all(folength);
				}
				all_is_set = IS_ON;

				if ((fidesc = open(fnpanic,O_RDONLY)) < 0) {
					werase(vfumsgw);
					wnoutrefresh(vfumsgw);
					wprintw(vfumsgw,"--> %s : ",fnpanic);
					wnoutrefresh(vfumsgw);
					doupdate();
					syserr(vfumsgw,msgtb[47]);
					/* syserr(vfumsgw,"Cannot open file -- Bye\n"); */
				}

				/* read file and map into vfu_page_t for 
                                   displaying */
				vfu_read_file(vfumsgw,fidesc,davfu,fnpanic);
				close(fidesc);
				/* determine printer type via first byte pair 
                                   of davfu data array */
				if (davfu[0] == 0xEE) {
					vfu_printer = 54;
					if (davfu[1] == 0xEC) 
						vfu_lpi = 6;
					else
						vfu_lpi = 8;
					set_printer_type(vfu_printer);
					set_printer_lpi(vfu_lpi);
					/* reconfigure davfu data 
					vfu_expand_room_all(folength,folength,MAXCHANNEL); */
				}
				doupdate();
				if (mynewsize != myoldsize) 
					vfu_expand_room_all(folength,myoldsize,MAXCHANNEL);
				else
					vfu_expand_room_all(folength,folength,MAXCHANNEL);
			}

			pr_flg = (vfu_printer == 88)?2*folength+5:2*folength+4;
			/* fill vfu-page and show it */
			if (overwrite)
				vfu_fill_page_t(davfu,pr_flg,vfu_page_t);

			/* initialize cursor coordinates into screen and array */
			vfu_restart_coordinates();
			vfu_draw_page(vfunumw,vfuw,cur_pgtop,vfu_page_t);
			vfu_edit_page(vfunumw,vfuw,vfu_page_t,davfu);

			break;
	} /* switch */

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_operate\n");
		wrefresh(vfumsgw);
	}

} /* vfu_operate */

/********************************************************************
 *         VFU TOOLS FOR PROCESSING OBJECTS INTO WINDOWS            *
 ********************************************************************/

/* Initialize structure */ 
static void 
vfu_init_info(S)
VFU_INFO 	*S; /* VFU_INFO structure */
{
	S->type = V_EMPTY;
	S->row = 0;
	S->col= 0;
	S->lgmax = 0;
	S->prompt = '.';
	S->prohib = (char *)0;
	S->allow = (char *)0;
	S->separ= (char *)0;
	S->strval = (char *)0;
	S->strdef = (char *)0;
	S->intdef = DFLTLENGTH;
	S->class_ctrl = (int (*)())0;	
	S->class_help = vfu_help_me;
	S->next = (VFU_INFO *)0;
	S->father = (VFU_INFO *)0;
	S->brother = (VFU_INFO *)0;
	return;
}

/* Variable arguments message print */
static void 
vfu_message(ws,wm, fmt, va_alist)
WINDOW 		*ws,*wm; /* window where cursor is back after 
			   showing message and message window */
char 		*fmt;
va_dcl
{
	va_list 	args;
	int 	ret = 0;
	int 	l, c;

	/* get previous location */
	if (ws) 
		getyx(ws, l, c);
	if (!wm) 
		wm = newwin(ROWMSGW,COLMSGW,YBEGMSGW,XBEGMSGW);
	if (msgpresent) 
		werase(wm);
	va_start(args);
	(void)vwprintw(wm, fmt, args);
	wrefresh(wm);
	va_end(args);
	msgpresent = 1;
	/* go back to previous window */
	if (ws) {
		wmove(ws, l, c);
		wrefresh(ws);
	}
	return;
}

/* Simple string compute function */
static char 
*vfu_get_string(win,wm,info,row,col,lgmax,lgmin)
WINDOW 		*win;
WINDOW 		*wm; /* message window */
int 		row, col, lgmax, lgmin;
VFU_INFO 	info;
{


	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_get_string\n");
		wrefresh(vfumsgw);
	}

	vfu_init_info(&info);
	info.type |= V_STR | V_NUP | V_AUTN;
	info.row = row;
	info.col = col;
	info.lgmax = lgmax;
	info.lgmin = lgmin;
	info.strdef = (char *) malloc((MAXFILENAME+1)*sizeof(char));
	strcpy(info.strdef,mktemp("vfuXXXXXX"));
	vfu_catch(win,wm,&info);

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_get_string\n");
		wrefresh(vfumsgw);
	}

	return(info.strval);
}

/* Simple integer compute function */
static int 
vfu_get_integer(win,wm,info,row,col,lgmax,enable)
WINDOW 		*win;
WINDOW 		*wm; /* message window */
int 		row, col, lgmax, enable;
VFU_INFO 	info;
{

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_get_integer\n");
		wrefresh(vfumsgw);
	}

	vfu_init_info(&info);
	/* info.type |= V_INT | V_NUP; */
	info.type |= V_INT | V_DFLT ;
	info.row = row;
	info.col = col;
	info.lgmax = lgmax;
	info.lgmin = 0;
	info.intdef = DFLTLENGTH;
	if (enable) info.prohib = "-";
	vfu_catch(win,wm,&info);

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_get_integer\n");
		wrefresh(vfumsgw);
	}

	return(info.intval);
}

/* Simple answer compute function */
static int 
vfu_answer(win, row, col)
WINDOW 		*win;
int 		row, col;
{
	VFU_INFO 	info;

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_answer\n");
		wrefresh(vfumsgw);
	}

	vfu_init_info(&info);
	info.type |= V_STR | V_NUP | V_AUTN;
	info.row = row;
	info.col= col;
	info.prompt = '?';
	info.lgmax = 1;
	info.allow = "noyNOY";
	vfu_catch(win, &info);

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_answer\n");
		wrefresh(vfumsgw);
	}

	if (info.strval[0] == 'n' || info.strval[0] == 'N')
		return(0);
	else
		return(1);
}

/* Overall dispatcher function */
static VFU_INFO 
*vfu_catch(ws,wm,S)
WINDOW 		*ws; /* window where information is read */
VFU_INFO 	*S; /* information stored in structure S */
WINDOW 		*wm; /* message window */
{
	int 	ret = 0;
	VFU_INFO *debv = S;
	int 	x_off, y_off;

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_catch\n");
		wrefresh(vfumsgw);
	}

	getbegyx(ws, y_off, x_off);
	wmove(ws,y_off,x_off);
	wrefresh(ws);

		keypad(ws, TRUE);

		switch (S->type & V_MASK) {
			case V_INT :
				ret = vfu_readint(ws,wm,S);
				break;
			case V_STR :
				ret = vfu_readstr(ws,wm,S);
				break;
		}

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_catch\n");
		wrefresh(vfumsgw);
	}

}

/* Compute character function */
static chtype 
vfu_readchar(ws,wm,S)
WINDOW 		*ws; /* window where information is read */
WINDOW 		*wm; /* message window */
VFU_INFO 	*S; /* information stored in structure S */
{
	chtype 	LastCh;

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_readchar\n");
		wrefresh(vfumsgw);
	}

	while (1) {
		LastCh = wgetch(ws);
		if (wm && msgpresent) {
			werase(wm);
			wrefresh(wm);
			msgpresent = 0;
		}

		/* deal with functionnal key */
		switch (LastCh) {
			case KEY_DOWN :
			case KEY_ENTER :
				return(LastCh = KEY_ENTER);

			case KEY_BACKSPACE : 
			case KEY_UP :
			case KEY_HELP :
				return(LastCh);

			default :
				if ((char)LastCh == '\n') 
					return(LastCh = KEY_ENTER);
				if ((char)LastCh == '\r') 
					return(LastCh = KEY_ENTER);
				if ((char)LastCh == '\b') 
					return(LastCh = KEY_BACKSPACE);
				if (LastCh == KEY_RIGHT) 
					LastCh = (chtype)' ';
				/* disable any use of prohibited character */
				if (isascii((char)LastCh)) {
					if (S->prohib && strchr(S->prohib, (char)LastCh)) {
						beep();
						vfu_message(ws,wm,msgtb[52]);
						/* vfu_message(ws,wm, "Prohibited character."); */
						break;
					}
					if (S->separ && strchr(S->separ,(char)LastCh))
						return(LastCh = KEY_RIGHT);
					if (S->allow && !strchr(S->allow, (char)LastCh)) {
						beep();
						vfu_message(ws,wm,msgtb[52]);
						/* vfu_message(ws,wm, "Prohibited character."); */
						break;
					}
					return(LastCh);
				}
				else {
					beep();
					vfu_message(ws,wm,msgtb[53]);
					/* vfu_message(ws,wm, "Illegal character."); */
				}
		} /* switch */
	} /* while */

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_readchar\n");
		wrefresh(vfumsgw);
	}
}

/* Read string function */
static int 
vfu_readstr(ws,wm,S)
WINDOW 		*ws; /* window where information is read */
WINDOW 		*wm; /* message window */
VFU_INFO 	*S; /* information stored in structure S */
{
	register chtype ch;
	register int	i;

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_readstr\n");
		wrefresh(vfumsgw);
	}

	/* Make room */
	if (!S->strval) 
		S->strval = malloc(S->lgmax + 1);

tryag:
	if (ok_empty(S)) {
		S->strval[0] = '\0';
		S->type &= ~V_EMPTY;
	}

	/* Fill up prompt character */
	for (i=0; i<S->lgmax; i++)
		wprintw(ws,"%c",(chtype)S->prompt);

	/* show string */
	wmove(ws, 0, 0);
	waddstr(ws, S->strval);
	wrefresh(ws);

	i = strlen(S->strval);

	while(1) {

		switch (ch = vfu_readchar(ws,wm,S)) {

			/* fill up window with prompt character */
			case KEY_BACKSPACE : 
				if (i == 0) {
					if (ok_vautp(S)) 
						return(-1);
					else 
						beep();
				}
				else {
					i--;
					wmove(ws, 0, i);
					waddch(ws, (chtype)S->prompt);
					S->strval[i] = '\0';
					wmove(ws, 0, i);
					wrefresh(ws);
				}
				break;

			/* clear to end of line */
			case KEY_UP :
				if (ok_vnup(S)) 
					beep();
				/* clear to end of line */
				else {
					S->strval[i] = '\0';
					if (i < S->lgmax) {
						wclrtoeol(ws);
						wrefresh(ws);
					}
					return(-1);
				}
				break;

			/* check before catch value */
			case KEY_RIGHT :
			case KEY_ENTER :
				/* number of characters check */
				if (i < S->lgmin) {
					beep();
					if (S->lgmin == S->lgmax) 
						vfu_message(ws,wm,msgtb[53]);
						/* vfu_message(ws,wm,"Entire field to be filled up."); */
					else if (S->lgmin == 1) 
						vfu_message(ws,wm,msgtb[54]);
						/* vfu_message(ws,wm,"Need to be filled up."); */
					else  
						vfu_message(ws,wm,"%d %s", S->lgmin,msgtb[55]);
						/* vfu_message(ws,wm,"%d character(s) at least.", S->lgmin); */
				}
				else 
					goto ausgang;
				break;

			/* show help menu if necessary */
			case KEY_HELP :
				if (S->class_help) 
					(S->class_help)();
				wmove(ws, 0, i);
				wrefresh(ws);
				break;

			default :
				if (i >= S->lgmax) {
					beep();
					wrefresh(ws);
				}
				else {
					S->strval[i++] = (char)ch;
					wechochar(ws, ch);
					if (i >= S->lgmax && ok_vautn(S)) 
						goto ausgang;
				}
				break;
		}
	}

ausgang:
	S->strval[i] = '\0';
	if (i == 0 && ok_dflt(S)) {
		in_default = 1;
		strcpy(S->strval,S->strdef);
		wmove(ws, 0, 0);
		waddstr(ws, S->strval);
		i = strlen(S->strval);
		if (msgpresent)
			werase(wm);
		wprintw(wm,"%s\n",msgtb[50]);
		/* wprintw(wm,"Default name is taken into account"); */
		wrefresh(wm);
	}
	if (i < S->lgmax) {
		wclrtoeol(ws);

		S->strval[i] = '\0';
		wmove(ws, 0, 0);
		waddstr(ws, S->strval);
	}
	if (S->class_ctrl && !(S->class_ctrl)()) 
		goto tryag;
	wrefresh(ws);

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_readstr\n");
		wrefresh(vfumsgw);
	}

	if (ch == KEY_RIGHT) 
		return(1);
	else if (ch == KEY_ENTER) 
		return(!ok_vaute(S));
	else 
		return(1);
}

/* Read integer function */
static int 
vfu_readint(ws,wm,S)
WINDOW 		*ws; /* window where information is read */
WINDOW 		*wm; /* message window */
VFU_INFO 	*S; /* information stored in structure S */
{
	register chtype	ch;
	register int 	i;
	register int 	j;
	char 	format[10];
	char 	c;
	int 	tmp, base, boundary;
	char 	fmt;


	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"In vfu_readint\n");
		wrefresh(vfumsgw);
	}

	fmt = 'd';
	base = 10;
	S->allow = "0123456789-";
	boundary = 0;

	sprintf(format, "%%#%d%c", S->lgmax, fmt);
	if (ok_empty(S)) {
		S->intval = DFLTLENGTH;
		S->type &= ~V_EMPTY; 
	}

tryagn:
	i = boundary;
	tmp = S->intval * 10;
	while ((tmp /= base) != 0) 
		i++;
	if (S->intval < 0)  
		i++;

	/* show integer */
	wmove(ws, 0, 0);
	wprintw(ws, format, S->intval);
	wmove(ws, 0, 0);
	for (j=0; j < S->lgmax - i - (i==0); j++) 
		waddch(ws, (chtype)S->prompt);
	wmove(ws, 0, S->lgmax - 1);
	wrefresh(ws);

	/* deal with functionnal key , commentaries are the same as in string function */
	while(1) {
		switch (ch = vfu_readchar(ws,wm,S)) {

			/* fill up window with prompt character */
			case KEY_BACKSPACE : 
				if (i == boundary) {
					if (ok_vautp(S)) 
						return(-1);
					else 
						beep();
				}
				else {
					i--;
					/* No more minus sign left when zero value value reached */
					if (S->intval < 0 && -S->intval < base) 
						i--;

					S->intval /= base;
					wmove(ws, 0, 0);
					wprintw(ws, format, S->intval);
					wmove(ws, 0, 0);
					for (j=0; j < S->lgmax - i - (i==0); j++)
						waddch(ws, (chtype)S->prompt);
					wmove(ws, 0, S->lgmax - 1);
					wrefresh(ws);
				}
				break;

			case KEY_UP :
				if (ok_vnup(S)) 
					beep();
				else 
					goto ausgan;
				break;

			case KEY_RIGHT :
			case KEY_ENTER :
				if (i < S->lgmin + boundary) {
					beep();
					if (S->lgmin == S->lgmax) 
						vfu_message(ws,wm,msgtb[53]);
						/* vfu_message(ws,wm,"Entire field to be filled up."); */
					else if (S->lgmin == 1) 
						vfu_message(ws,wm,msgtb[54]);
						/* vfu_message(ws,wm,"Need to be filled up."); */
					else  
						vfu_message(ws,wm,"%d %s\n", S->lgmin,msgtb[55]);
						/* vfu_message(ws,wm,"%d character(s) at least.", S->lgmin); */
				}
				else 
					goto ausgan;
				break;

			case KEY_HELP :
				if (S->class_help) 
					(S->class_help)();
				wmove(ws, 0, S->lgmax - 1);
				wrefresh(ws);
				break;

			default :
				/* number between -99 and 999 is allowed 
				   minus sign takes one place */
				if ((i >= S->lgmax) && (S->intval>0 || ch!=(chtype)'-')) {
					beep();
					wrefresh(ws);
				}
				else {
					if (ch == (chtype)'-') {
						if (S->intval < 0) 
							i--;
						if (S->intval > 0) 
							i++;
						S->intval = -S->intval;
					}
					else {
						char c = (char)ch;
						if (c == '0' && S->intval == 0)  
							break;
						S->intval *= base;
						if (c >= 'a') 
							c += ':' - 'a';
						else if (c >= 'A') 
							c += ':' - 'A';
						if (S->intval < 0) 
							tmp = '0' - c;
						else 
							tmp = c - '0';
						S->intval += tmp;
						i++;
					}
					wmove(ws, 0, 0);
					wprintw(ws, format, S->intval);
					wmove(ws, 0, 0);
					for (j=0; j < S->lgmax - i - (i==0); j++)
						waddch(ws, (chtype)S->prompt);
					wmove(ws, 0, S->lgmax - 1);
					wrefresh(ws);
					if ((i >= S->lgmax) && (S->intval > 0 || ch != (chtype)'-')) {
						if (ok_vautn(S)) 
							goto ausgan;
					}
				}
				break;
		}
	}

ausgan:
	wmove(ws, 0, 0);
	for (j=0; j < S->lgmax - i - (i==0); j++) 
		waddch(ws, (chtype)' ');
	/* if (i == boundary && ok_dflt(S)) { */
	/* Default value */
	if (i == boundary) {
		S->intval = S->intdef;
		wmove(ws, 0, 0);
		wprintw(ws, format, S->intval);
		wrefresh(ws);
	}
	if (ch != KEY_UP && S->class_ctrl && !(S->class_ctrl)(ws, S)) 
		goto tryagn;
	else {
		wrefresh(ws);
		if (ch == KEY_UP) 
			return(-1);
		else if (ch == KEY_RIGHT) 
			return(1);
		else if (ch == KEY_ENTER) 
			return(!ok_vaute(S));
		else 
			return(1);
	}

	DEBUG(mydebug) {
		mvwaddstr(vfumsgw,0,0,"Out vfu_readint\n");
		wrefresh(vfumsgw);
	}
}

wechochar(w,c)
WINDOW *w;
chtype c;
{
	waddch(w,c);
	wrefresh(w);
}

vwprintw(w,fmt,arg)
WINDOW *w;
char *fmt;
va_list arg;
{
int n;
unsigned char buf[BUFSIZ];

	n=vsprintf((char *)buf, fmt, arg);
	va_end(arg);
	if(n == ERR)
		return(ERR);
	return(waddstr(w, buf));
}
