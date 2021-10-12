#ifndef lint
static char sccsid[] = "@(#)86 1.1 src/bos/usr/bin/vfu/vfu_scrollbar.c, cmdpios, bos411, 9428A410j 4/28/94 08:16:31";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS)
 *
 * FUNCTIONS: vfu_scrollbar.c
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*                      
*
*			    SCROLLBAR MANAGER MODULE
*			    (    vfu_scrollbar.c   )
*
*	The aim is to control scroll bar ; that offers PC-like facilities to 
*	user. There are less functionnal options with regard to those in 
*	in-line vfu-page and therefore screen clarity , easy use .
*	This module is self_explanatory because	of dense commentaries .
*	
*
*	WARNING : DO NOT ERASE ALL MESSAGES COMMENTED , THAT EASES MAINTENANCE 
*	
*	Author : BUI Q. M.
*	BULL S.A 
*	VERSION : 1.0 June 1990
*
*/

#include <curses.h>
#include <term.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "vfu.h" 


#define GET_DEBUG()		{ debug = get_debug(); \
				  mydebug = get_mydebug(); }

static jmp_buf 		env;
static char 		pack_init_ok = 0;
static W_MENU 		*stack[50]; /* environment stack  for recursivity */
static W_MENU 		OP1,OP2,OP3, /* Option windows */
			OP4,OP5,OP6;
static W_MENU 		*vfu_fname_t = MENU_NULL; /* array of files ".vfu" */
static W_MENU 		*on_way = MENU_NULL; /* tracker for 'FILES' option */
static int 		dive_level = 0; /* level in tree */
static int		st_depth ; /* tracker level in tree */
static WINDOW 		*stealth; /* stealth window */
static int 		stealth_y ; /* stealth window for making clean files display, when using scroll bar mode */
static int 		debug,mydebug;
static int 		there_are_files; /* file existence flag */
static short 		dialogue; /* country dialogue */
static char 		**msgpt; /* message table */
static int 		vfu_printer; /* printer type */
static int 		vfu_lpi; /* printer lpi */


/* Compare two elements 
   Used in qsort function below */
static int 
vfu_compare(tab1,tab2)
W_MENU *tab1,*tab2;
{
	return(strcmp(tab1->info,tab2->info));
}

/* Window field initialization */
static void 
vfu_init_fname(rank)
int 	rank;
{
	vfu_fname_t[rank].row = 0;
	vfu_fname_t[rank].column = 0;
	vfu_fname_t[rank].info = NULL;  
	vfu_fname_t[rank].type = T_VAL;
	vfu_fname_t[rank].weight = 0;
	vfu_fname_t[rank].submenu = MENU_NULL;
	vfu_fname_t[rank].ufield = (WINDOW *)0;
	vfu_fname_t[rank].wleft = MENU_NULL;
	vfu_fname_t[rank].wright = MENU_NULL;
	vfu_fname_t[rank].wup = MENU_NULL;
	vfu_fname_t[rank].wdown = MENU_NULL;
	vfu_fname_t[rank].wnext = MENU_NULL;

}

/* List all filename with '.vfu' extension */
int 
vfu_list_fname()
{
	struct direct 	dirbuf; /* directory structure */
	struct stat 	stbuf;
	char 		nep[AIX_DIRSIZ+1],
			*check_nep,
			*wdir,
			mywdir[BUFSIZE+1],
			*envt,**get_adr_map();
	int 		i,row,count,fd,checkcmp,nbc;
	WINDOW 		*wmsg,*vfu_get_adr_window();
	int 		x_shift,y_shift;
	short		display_file;
	void 		qsort();

	y_shift = ((envt = getenv("LINES"))!= NULL )?atoi(envt):0;
	x_shift = ((envt = getenv("COLUMNS"))!= NULL )?atoi(envt):0;

	there_are_files = 0;
	display_file = 0;

	wmsg = vfu_get_adr_window(W_MSG);
	dialogue = get_country_dialogue();
	msgpt = get_adr_map();

	GET_DEBUG();
	DEBUG(mydebug) {
		werase(wmsg);
		wprintw(wmsg,"In vfu_list_fname\b");
		wrefresh(wmsg);
	}

	/* Catch absolute path of working directory */
	if ((wdir = getcwd(mywdir,BUFSIZE)) == NULL)
		warning(wmsg,msgpt[35]);
		/* warning(wmsg,"Working directory search fails -- Bye\n"); */
	if ((fd=open(mywdir,O_RDONLY)) < 0) 
		syserr(wmsg,msgpt[47]);
		/* syserr(wmsg,"Cannot open working directory -- Bye\n"); */
	i = 0;
	if ((vfu_fname_t = (W_MENU *) malloc((i+1)*sizeof(W_MENU))) == MENU_NULL)
		syserr(wmsg,msgpt[44]);
	vfu_init_fname(i);
	if ((vfu_fname_t[i].info = (char *) malloc((FNAME+1)*sizeof(char))) == NULL)
		syserr(wmsg,msgpt[44]);

	/* Read files in current directory */
	while (read(fd,(char *)&dirbuf,sizeof(dirbuf)) > 0 ) {
		if (dirbuf.d_ino == 0)
			continue;
		if  (strcmp(dirbuf.d_name,".") == 0 || strcmp(dirbuf.d_name,"..") == 0) 
			continue;

		strcpy(nep,dirbuf.d_name);

		/* skip files which don't have extension or extension <> '.vfu' */ 
		if  ((check_nep = strchr(nep,'.')) == NULL || (checkcmp = strcmp(check_nep,".vfu")) != 0)
			continue;
		else {
			strcpy(vfu_fname_t[i].info,strtok(nep,"."));	
			/* set size field for this filename window */
			sprintf(nep,"%s%s\0","./",dirbuf.d_name);
			stat(nep,&stbuf);
			if (stbuf.st_size % 2 == 0) 
				vfu_fname_t[i].weight = (int)((stbuf.st_size-2)/2 - 1);
			else 
				vfu_fname_t[i].weight = (int)((stbuf.st_size-3)/2 );
			there_are_files = 1;

			/* keep track of number of files with '.vfu' extension */
			++i;
			if ((vfu_fname_t = (W_MENU *) realloc(vfu_fname_t,(i+1)*sizeof(W_MENU))) == MENU_NULL)
				syserr(wmsg,msgpt[44]);
			vfu_init_fname(i);
			if ((vfu_fname_t[i].info = (char *) malloc((FNAME+1)*sizeof(char))) == NULL)
				syserr(wmsg,msgpt[44]);
		}

	} /* while */

	close(fd);

	if (!there_are_files) 
		return(0);

	count = i;
	/* sort filenames */
	qsort(vfu_fname_t,(unsigned)count,sizeof(vfu_fname_t[0]),vfu_compare); 

	/* Link windows */
	row = 1;

	/* First item set */
	vfu_fname_t[0].column = XFILES;
	vfu_fname_t[0].row = YFILES;
	vfu_fname_t[0].wleft = MENU_EXIT;

	i = 1;
	while(i < count) {
		/* set member coordinates */
		/* horizontal link */
		/* cut down x_coordinate and increment y_coordinate */
		if ((i%6) == 0) {
			/* increment only when greater than 11-th item */
			if (i > 11 && i%6 == 0) 
				++row;
			vfu_fname_t[i].column = XFILES;
			vfu_fname_t[i].row = vfu_fname_t[i-6].row +1 ;
		}
		else {
			vfu_fname_t[i].column = vfu_fname_t[i-1].column + FNAME ;
			vfu_fname_t[i].row = vfu_fname_t[i-1].row ;
		}
		vfu_fname_t[i].wleft = &vfu_fname_t[i-1];
		vfu_fname_t[i-1].wright = vfu_fname_t[i-1].wnext = &vfu_fname_t[i];
		
		/* vertical link */
		if ( i >= row*6 )  {
			vfu_fname_t[i-6].wdown = &vfu_fname_t[i];
			vfu_fname_t[i].wup = &vfu_fname_t[i-6];
		}

		++i;
	}

	/* vertical and horizontal links of the last */
	vfu_fname_t[i-1].wnext = vfu_fname_t[i-1].wright = &vfu_fname_t[0];
	/*To solve the problem using key_up with a number of files <= 6*/
	/* 		MR: bsx32920524				       */	
	if( i > 6 ){
		vfu_fname_t[i-1].wup = &vfu_fname_t[i-7];
		vfu_fname_t[i-7].wdown = &vfu_fname_t[i-1];
	}

	stealth_y = row;

	DEBUG(mydebug) {
		werase(wmsg);
		wprintw(wmsg,"Out vfu_list_fname\n");
		wrefresh(wmsg);
	}
	return(1);

}

/* Signal management */
static void 
vfu_deal_signal(numsig)
{
	longjmp(env, numsig);
}

int 
vfu_packed_menu(tb)
char 	**tb;
{
	int 	ret;

	if (!pack_init_ok) {
		initscr();
		pack_init_ok = 1;
	}

	refresh();
	noecho(); 
	cbreak();
	ret = vfu_menu(stdscr, tb, -1, -1);
	echo(); 
	nocbreak();
	erase();
	refresh();
	endwin();
	return(ret);
}

/* Master function */
int 
vfu_menu(win, tb, row, column)
WINDOW 	*win;
char 	*tb;
int 	row, column;
{
	int retval;
	W_MENU *start, *ret;

	start = vfu_make_sublevel_menu(win, tb, row, column, 0, 1);
	ret = vfu_toplevel_menu(win, start, A_REVERSE, A_NORMAL);
	if (ret == MENU_NULL) 
		retval = -1;
	else 
		retval = ret->wval;
	free(start);
	return(retval);
}

/* Virtually refresh all windows in stack */
static void 
vfu_noutrefresh_menu()
{
	W_MENU 	*m;
	short 	i;
	WINDOW 	*vfu_get_adr_window();

	/* Preserve initial windows */
	touchwin(vfu_get_adr_window(W_NAME));
	touchwin(vfu_get_adr_window(W_FORM));
	touchwin(vfu_get_adr_window(W_NUM));
	touchwin(vfu_get_adr_window(W_VFU));
	touchwin(vfu_get_adr_window(W_MSG));
	touchwin(stdscr);

	/* then touch all member windows of same parent */
	for (i=0; i<dive_level; i++) {
		m = stack[i];
		do {
			touchwin(m->ufield);
			wnoutrefresh(m->ufield);
			m = m->wnext;
		} while (m != stack[i]);
	}
	return;
}

/* Start the options in top level menu and dive into corresponding submenus tree ;
   exploration looks like depth first search */
static W_MENU 
*vfu_toplevel_menu(win, M, on, off)
WINDOW 	*win;
W_MENU 	*M;
chtype 	on, off;	/* video attribute */
{
	W_MENU 	*start,*old,*fed_up,*professor;
	int 	nbl, nbc, visib;
	int 	i, ii, j;
	chtype 	ch,c;
	int 	numsig;
	void 	(*oldsig)() = SIG_DFL;
	jmp_buf oldenv;
	char 	onlytitle = 1;
	short 	msgpresent;
/* Uncomment when using tabulations in option array definition 
	int x_off,y_off;
	int x_max,y_max;
*/

	WINDOW *wm,*vfu_get_adr_window();
	char *get_adr_tmpstr2();
	void set_tmpstr1(),set_tmpstr2();
	fed_up = (W_MENU *)0;

	/* Set attributes */
	if (!on) 
		on = A_REVERSE;
	if (!off) 
		off = A_NORMAL;

/* Uncomment when using tabulations in option array definition 
	getbegyx(win, y_off, x_off);
	getmaxyx(win, y_max, x_max);
*/
	GET_DEBUG();

	wm = vfu_get_adr_window(W_MSG);

	DEBUG(mydebug) {
		werase(wm);
		wprintw(wm,"In vfu_top_level_menu\n");
		wrefresh(wm);
	}

	visib=1;
	curs_set(0);
	start = M;

	stack[dive_level] = M; 
	/* Signal management */
	oldsig = signal(SIGINT, vfu_deal_signal);
	if (oldsig == SIG_IGN) 
		signal(SIGINT, SIG_IGN);
	/* Environment salvage for recursivity */
	for (i=0; i<_JBLEN; i++) 
		oldenv[i] = env[i];
	/* Delete all windows created upon receipt signal */
	if (numsig = setjmp(env)) {
		curs_set(visib);
		while (start->ufield) {
			werase(start->ufield);
			wnoutrefresh(start->ufield);
			delwin(start->ufield);
			start->ufield = (WINDOW *)0;
			start = start->wnext;
		}
		doupdate();
		errno = EINTR;
		signal(SIGINT, oldsig);
		/* case of packed_menu */
		if (pack_init_ok) { 
			echo(); 
			nocbreak(); 
			endwin(); 
		}
		for (i=0; i<_JBLEN; i++) 
			env[i] = oldenv[i];
		kill(getpid(), numsig);
		return(MENU_NULL);
	}


	/* Menus creation and display at the level where user is i.e when user is on item of T_MENU 
	   type and dives in , creation and display are performed at this node/level */

	do {
		int l, c;

		if (M->type != T_TITLE) 
			onlytitle = 0;
		/* Uncomment when arrays are hard-coded with tabulations 
		 Here is preformatting window 
		ii = strlen(M->info);
		for (i=j=nbl=nbc=0; i<ii; i++, j++) {
			if (j>nbc) nbc=j;
			if (M->info[i] == '\n') {
				nbl++;
				j=0;
			}
		}
		nbl++;
		nbc++;
		l = M->row + y_off;
		c = M->column + x_off;
		if (l < y_off) 
			l = y_off;
		if (c < x_off) 
			c = x_off;
		if (l + nbl > y_off + y_max) 
			l = y_off + y_max - nbl;
		if (c + nbc > x_off + x_max) 
			c = x_off + x_max - nbc;
		M->ufield = newwin(nbl, nbc, l, c);
		wattrset(M->ufield, off);
		werase(M->ufield);
		mvwaddstr(M->ufield, 0, 0, M->info);
		wnoutrefresh(M->ufield);
		*/

		/* Create all windows linked to this item until wrapping around */
		M->ufield = newwin(1, strlen(M->info)+1,M->row,M->column);
		if (M->ufield)
			keypad(M->ufield, TRUE);
		else
			break;
		wattrset(M->ufield, off);
		werase(M->ufield);
		mvwaddstr(M->ufield, 0, 0, M->info);
		wnoutrefresh(M->ufield);
		M = M->wnext;
	} while (M != start);
		
	if (onlytitle || !M->ufield) 
		goto ausgang;

	while (M->type == T_TITLE) 
		M = M->wnext;

	/* Get non-TITLED window */
	start = on_way = professor = M;
	doupdate();
	msgpresent = 0;

	st_depth = dive_level;
	old = MENU_NULL;
	while (1) {
		if (old) {
			/* Set bright attribute off*/
			wattrset(old->ufield, off);
			mvwaddstr(old->ufield, 0, 0, old->info);
			wnoutrefresh(old->ufield);
			old = MENU_NULL;
		}
		/* Set bright attribute on */
		wattrset(M->ufield, on);
		mvwaddstr(M->ufield, 0, 0, M->info);
		wnoutrefresh(M->ufield);
		doupdate(); 

		if (professor == &OP1) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[56]);
			wrefresh(wm);
			msgpresent = 1;
		}
		if (professor == &OP2) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[57]);
			wrefresh(wm);
			msgpresent = 1;
		}
		if (professor == &OP3) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[58]);
			wrefresh(wm);
			msgpresent = 1;
		}
		if (professor == &OP4) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[59]);
			wrefresh(wm);
			msgpresent = 1;
		}
		if (professor == &OP5) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[60]);
			wrefresh(wm);
			msgpresent = 1;
		}
		if (professor == &OP6) {
			if (msgpresent)
				werase(wm);
			wprintw(wm,"%s",msgpt[61]);
			wrefresh(wm);
			msgpresent = 1;
		}

		/* Get user keyboard strike */
		/* switch (ch = wgetch(M->ufield)) { */
		switch (ch = wgetch(M->ufield)) {

			case '\n': 
			case '\r': 
			case KEY_ENTER :
				ch = KEY_ENTER;
				break;

			case '\t' : 
			case 'd' : 
			case KEY_DOWN :
				ch = KEY_DOWN;
				break;

			case 'u' : 
/*
#ifndef BULL_PRG
*/
			case KEY_BTAB : 
/*
#endif
*/
			case KEY_UP :
				ch = KEY_UP;
				break;

			case 'l' : 
			case '\b' : 
			case KEY_BACKSPACE : 
			case KEY_LEFT :
				ch = KEY_LEFT;
				break;

			case ' ' : 
			case 'r' : 
			case KEY_RIGHT :
				ch = KEY_RIGHT;
				break;
		}

		switch (ch) {
			case KEY_ENTER :
				/* Uncomment when deal with shell 
				if (M->type == T_COMM) {
					endwin();
					if (isdigit(M->command[0])) {
						char *p;
						p = M->command;
						while (isdigit(*p)) 
							p++;
						system(p);
						goto ausgang;
					}
					else {
						system(M->command);
						doupdate();
						continue;
					}
				}
				*/

				if ( there_are_files == 0) 
					if (on_way->type == T_MENU ) 
						goto ausgang;
			
				if (there_are_files > 0) {
					/* make files's show clean */
					stealth = newwin(stealth_y,XNBFN*FNAME,YFILES,XFILES);
					werase(stealth);
					wrefresh(stealth);
				}
				/* dive into tree */
				if (M->type == T_MENU && M->submenu && M->submenu != MENU_EXIT && there_are_files > 0) {
					W_MENU *M1;

					/* track level for further use */
					++dive_level;
					/* actual level becomes top level */
					M1 = vfu_toplevel_menu(win, M->submenu, on, off);
					
					if (M1 && M1->ukexit == KEY_ENTER)  
						M = M1;
					else  
						continue;
				}
				goto ausgang;

			/* move track */
			case KEY_DOWN : 
			case KEY_UP : 
			case KEY_LEFT : 
			case KEY_RIGHT :
				{
					W_MENU *wnext;
				jump:
					if (ch == KEY_DOWN ) 
						wnext = M->wdown;
					else if (ch == KEY_UP ) 
						wnext = M->wup;
					else if (ch == KEY_LEFT ) 
						wnext = M->wleft;
					else if (ch == KEY_RIGHT )  
						wnext = M->wright;

					on_way = wnext;
					professor = on_way;

					if (wnext == MENU_EXIT)  {
						fed_up = MENU_EXIT;
						goto ausgang;
					}
					else if (wnext == MENU_NULL) 
						beep();
					else if (wnext == SUBMENU) {
						if (M->type == T_MENU && M->submenu != MENU_NULL && M->submenu != MENU_EXIT) {
							W_MENU *M1; 

							/* actual level becomes top level */
							M1 = vfu_toplevel_menu(win, M->submenu, on, off);
							if (M1 && (M1->ukexit == KEY_ENTER )) {
								M = M1;
								ch = KEY_ENTER;
								st_depth = dive_level;
								goto ausgang;
							}
							else 
								continue;
						}
						else beep();
					}
					else {
						if (M->type != T_TITLE) 
							old = M;
						M = wnext;
						if (M->type == T_TITLE) 
							goto jump;
					}
				}
				break;

			case 'h' :
			case KEY_HOME :
				old = M;
				M = start;
				break;

			/* deal with initial keyboard strike */
			default :
				/* Prohibit non alphanumeric character */
				if (!isalnum((char)ch)) 
					beep();
				else {
					/* Selection by initial */ 
					char c, initiale;
					W_MENU *m = M->wnext;
					int getit = 0;

					c = tolower((char)ch);
					while (m != M) {
						int i = 0;
						if (m->type == T_TITLE) {
							m = m->wnext;
							continue;
						}
						while (isspace(m->info[i])) 
							i++;
						initiale = tolower(m->info[i]);
						if (initiale == c) {
							old = M;
							M = m;
							getit = 1;
							professor = M;
						}
						else 
							m = m->wnext;
					}
					if (!getit) 
						beep();
				}
				break;
		}
	}
ausgang:
	/* delete all submenus which have been visited */
	curs_set(visib);
	/* broadcast information of this file */
	if (dive_level > 0 && fed_up != MENU_EXIT && there_are_files > 0) {
		char myfile[30];

		set_tmpstr1(M->info);
		set_tmpstr2(M->weight);
		set_folength(M->weight);
		strcpy(myfile,"./");
		strcat(myfile,M->info);
		strcat(myfile,SAME);
		vfu_check_intruder(wm,myfile);
		vfu_trap_tprinter();
	}

	vfu_noutrefresh_menu();
	while (start->ufield) {
		if (dive_level > 0) {
			werase(start->ufield);
			wnoutrefresh(start->ufield);
		}
		delwin(start->ufield);
		start->ufield = (WINDOW *)0;
		start = start->wnext;
	}

	dive_level--; 
	if (dive_level < 0)
		dive_level = 0; 
        if (fed_up == MENU_EXIT)
		st_depth = dive_level;
        if (there_are_files == 0)
		st_depth = dive_level;
		
	/* restitution display */
	touchwin(win);
	wnoutrefresh(win);
	wnoutrefresh(vfu_get_adr_window(W_NAME));
	wnoutrefresh(vfu_get_adr_window(W_FORM));
	wnoutrefresh(vfu_get_adr_window(W_NUM));
	wnoutrefresh(vfu_get_adr_window(W_VFU));
	wnoutrefresh(vfu_get_adr_window(W_MSG));
	wnoutrefresh(stdscr);
	doupdate();

	signal(SIGINT, oldsig);
	M->ukexit = ch;
	for (i=0; i<_JBLEN; i++) 
		env[i] = oldenv[i];

	DEBUG(mydebug) {
		werase(wm);
		wprintw(wm,"Out vfu_top_level_menu\n");
		wrefresh(wm);
	}
        if (there_are_files == 0)
		return(on_way);
	return(M);
}

/* Edition formatting when using hard-coded arrays with tabulations */
static W_MENU
*vfu_make_sublevel_menu(win, tb, row, column, flag, offset)
WINDOW 	*win;
char 	**tb;
int 	row, column;
int 	flag, offset;
{
	int 	size, i, retval;
	W_MENU 	*start, *ret;
	int 	x_max, y_max;
	int 	first = 1;

	getmaxyx(win, y_max, x_max);
	for (size = 0; tb[size]; size++);
	if (row < 0) 
		row = y_max / 2 - (size / 2);
	start = (W_MENU *)malloc(size * sizeof(W_MENU));
	for (i=0; i<size; i++) {
		start[i].row = row++;
		if (column < 0) 
			start[i].column = x_max/2 - (strlen(tb[i]) / 2);
		else 
			start[i].column = column;
		start[i].info = tb[i];
		start[i].type = T_VAL;
		start[i].wval = i + offset;
		start[i].ufield = (WINDOW *)0;
		if (i<size-1) 
			start[i].wnext = &start[i+1];
		else 
			start[i].wnext = &start[0];
		start[i].wright = MENU_NULL;
		if ((flag & UPEXIT) == UPEXIT) 
			start[i].wleft = MENU_EXIT;
		else 
			start[i].wleft = MENU_NULL;
		if (i<size-1) 
			start[i].wdown = &start[i+1];
		else 
			start[i].wdown = &start[0];
		if (i>0) 
			start[i].wup = &start[i-1];
		else 
			start[i].wup = &start[size-1];
	}
	for (i=0; i<size; i++) {
		/* Skip blank rows */
		if (start[i].info[0] == '\0') {
			start[i].wup->wdown = start[i].wdown;
			start[i].wdown->wup = start[i].wup;
		}
		else if (first) {
			if ((flag & LEFTEXIT) == LEFTEXIT) 
				start[i].wup = MENU_EXIT;
			first = 0;
		}
	}
	return(start);
}

int 
vfu_command()
{
	W_MENU 	*ret1, *m20;
	WINDOW 	*wm,*vfu_get_adr_window();
	chtype 	c;
	int 	x_shift,y_shift,look;
	char 	*envt;

	y_shift = ((envt = getenv("LINES"))!= NULL )?((atoi(envt) > 25)?((atoi(envt)-25)/2-1):0):0;
	x_shift = ((envt = getenv("COLUMNS"))!= NULL )?((atoi(envt) > 80)?((atoi(envt)-80)/2-1):0):0;

	wm = vfu_get_adr_window(W_MSG);
	vfu_printer = get_printer_type();
	vfu_lpi = get_printer_lpi();


	GET_DEBUG();
	DEBUG(mydebug) {
		werase(wm);
		wprintw(wm,"In vfu_command\n");
		wrefresh(wm);
	}
	refresh();

	if (!dialogue) {
		OP1.info = "SAVE";
		OP2.info = "FILES";
		OP3.info = "PGUP";
		OP4.info = "PGDOWN";
		OP5.info = "CLEAR";
		OP6.info = "QUIT";
	}
	else {
		OP1.info = "SAUVER";
		OP2.info = "FICHIER";
		OP3.info = "PGHAUT";
		OP4.info = "PGBAS";
		OP5.info = "EFFACER";
		OP6.info = "QUITTER";
	}

	OP1.type = T_VAL;
	OP2.type = T_MENU;
	OP3.type = T_VAL;
	OP4.type = T_VAL;
	OP5.type = T_VAL;
	OP6.type = T_VAL;

	OP1.column = ((dialogue)?(XOF1+1):XOF1)+x_shift;
	OP2.column = OP1.column + strlen(OP1.info) + ((dialogue)?(XOF2-2):XOF2);
	OP3.column = OP2.column + strlen(OP2.info) + ((dialogue)?(XOF3-3):XOF3);
	OP4.column = OP3.column + strlen(OP3.info) + ((dialogue)?(XOF4-4):XOF4);
	OP5.column = OP4.column + strlen(OP4.info) + ((dialogue)?(XOF5-2):XOF5);
	OP6.column = OP5.column + strlen(OP5.info) + ((dialogue)?(XOF6-1):XOF6);
	OP1.row = OP3.row = OP2.row = OP4.row = OP5.row = OP6.row = YBEGMSGW - 2 + y_shift;

	OP1.wval = OP2.wval = OP3.wval = OP4.wval = OP5.wval = OP6.wval = 0;
	OP1.weight = OP2.weight = OP3.weight = OP4.weight = OP5.weight = OP6.weight = 0;
	OP1.ufield = OP2.ufield = OP3.ufield = OP4.ufield = OP5.ufield = OP6.ufield = (WINDOW *) 0;

	OP1.wup = MENU_NULL; 
	OP1.wdown = MENU_NULL;
	OP2.wup = MENU_NULL; 
	OP2.wdown = MENU_NULL;
	OP3.wup = MENU_NULL; 
	OP3.wdown = MENU_NULL;
	OP4.wup = MENU_NULL; 
	OP4.wdown = MENU_NULL;
	OP5.wup = MENU_NULL; 
	OP5.wdown = MENU_NULL;
	OP6.wup = MENU_NULL; 
	OP6.wdown = MENU_NULL;

	OP1.wleft = &OP6; 
	OP1.wright = OP1.wnext = &OP2;
	OP2.wleft = &OP1; 
	OP2.wright = OP2.wnext = &OP3;
	OP3.wleft = &OP2; 
	OP3.wright = OP3.wnext = &OP4;
	OP4.wleft = &OP3; 
	OP4.wright = OP4.wnext = &OP5;
	OP5.wleft = &OP4; 
	OP5.wright = OP5.wnext = &OP6;
	OP6.wleft = &OP5; 
	OP6.wright = OP6.wnext = &OP1;

	/* FILES option */
	OP2.submenu = vfu_fname_t;

	ret1 = vfu_TOP(&OP1, A_REVERSE, A_NORMAL);

	if (st_depth > 0)  {
		werase(stealth);
		wnoutrefresh(stealth);
		doupdate();
		delwin(stealth);
		free(vfu_fname_t);
		return(99);
	}
	
	if (st_depth == 0) { 

		if (ret1 == &OP1) {
			refresh();
			vfu_master_call_save();
		}
		else if (ret1 == &OP2) {
			refresh();
			vfu_master_no_operation();
		}
		else if (ret1 == &OP3) {
			refresh();
			vfu_master_call_boostup();
		}
		else if (ret1 == &OP4) {
			refresh();
			vfu_master_call_boostdown();
		}
		else if (ret1 == &OP5) {
			refresh();
			vfu_master_call_clear_all();
		}
		else if (ret1 == &OP6) {
			refresh();
			vfu_close(0,CLOSE_VFU);
		}
	}

	DEBUG(mydebug) {
		werase(wm);
		wprintw(wm,"Out vfu_command\n");
		wrefresh(wm);
	}
	return(0);
}
curs_set(cursorv)
int cursorv;
{
	if(!cursorv)
	{
		if(cursor_invisible)
		     tputs(cursor_invisible, 0, putchar);
	}
	else
	{
		if(cursor_normal)
		     tputs(cursor_normal, 0 , putchar);
	}
}
