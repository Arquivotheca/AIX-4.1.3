static char sccsid[] = "@(#)84	1.13  src/bos/usr/bin/csh/file.c, cmdcsh, bos411, 9428A410j 5/4/94 14:18:57";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: setup_tty, back_to_col_1, pushback, catn, copyn, filetype, 
 *            print_by_column, tilde, retype, beep, print_recognized_stuff, 
 *            extract_dir_and_name, getentry, free_items, search, recognize, 
 *	      is_prefix, is_suffix, ignored, sortscmp, tenex_search, tenex
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 *
 *
 *
 * Tenex style file name recognition, .. and more.
 * History:
 *	Author: Ken Greer, Sept. 1975, CMU.
 *	Finally got around to adding to the Cshell., Ken Greer, Dec. 1981.
 */

#include "sh.h"
#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include <sys/ioctl.h>

#define ON	TRUE	
#define OFF	FALSE

#define ESC	'\033'

#define FREE_ITEMS(items) {					\
	int omask;						\
								\
	omask = sigblock(sigmask(SIGINT));			\
	free_items(items);					\
	items = NULL;						\
	(void)sigsetmask(omask);				\
}


/* Local static function prototypes.  */
static void		setup_tty(int);
static void		back_to_col_1();
static void		pushback(uchar_t *);
static void		catn(uchar_t *, uchar_t *, int);
static void		copyn(uchar_t *, uchar_t *, int);
static uchar_t		filetype(uchar_t *, uchar_t *); 
static void		print_by_column(uchar_t *, uchar_t *[], int);
static uchar_t *	tilde(uchar_t *, uchar_t *);
static void		retype();
static void		beep();
static void		print_recognized_stuff(uchar_t *);
static void		extract_dir_and_name(uchar_t *, uchar_t *, uchar_t *);
static uchar_t *	getentry(DIR *, int);
static void		free_items(uchar_t **);
static int		recognize(uchar_t *, uchar_t *, int, int);
static int		is_prefix(uchar_t *, uchar_t *);
static int		is_suffix(uchar_t *, uchar_t *);
static int		ignored(uchar_t *);


static void
setup_tty(int on)
{

#ifdef BSD_LINE_DISC
        struct sgttyb sgtty;
        static struct tchars tchars;    /* INT, QUIT, XON, XOFF, EOF, BRK */
#else
        struct termios ntermio;         /* POSIXLD */
#endif

	if (on) {
#ifdef BSD_LINE_DISC
		IOCTL(SHIN, TIOCGETC, (char *)&tchars, "2");
		tchars.t_brkc = ESC;
		IOCTL(SHIN, TIOCSETC, (char *)&tchars, "3");
		/*
		 * This must be done after every command: if
		 * the tty gets into raw or cbreak mode the user
		 * can't even type 'reset'.
		 */
		IOCTL(SHIN, TIOCGETP, (char *)&sgtty, "4");
		if (sgtty.sg_flags & (RAW|CBREAK)) {
			sgtty.sg_flags &= ~(RAW|CBREAK);
			IOCTL(SHIN, TIOCSETP, (char *)&sgtty, "5");
		}
#else
		IOCTL(SHIN, TCGETS, &ntermio, "2");		/* POSIXLD */
		ntermio.c_cc[VEOL] = ESC;			/* POSIXLD */
		ntermio.c_lflag |= (ICANON | ISIG | ECHOCTL);	/* POSIXLD */
		IOCTL(SHIN, TCSETSW, &ntermio, "5");		/* POSIXLD */
#endif

	}
	else {

#ifdef BSD_LINE_DISC
		tchars.t_brkc = -1;
		IOCTL(SHIN, TIOCSETC, (char *)&tchars, "6");
#else
		IOCTL(SHIN, TCGETS, &ntermio, "3");	/* POSIXLD */
		ntermio.c_cc[VEOL] = 0;			/* POSIXLD */
		IOCTL(SHIN, TCSETSW, &ntermio, "6");	/* POSIXLD */
#endif

	}
}

/* Move back to beginning of current line */
static void
back_to_col_1()
{

#ifdef BSD_LINE_DISC
	struct sgttyb tty, tty_normal;
#else
	struct termios otermio, ntermio;	/* POSIXLD */
#endif
	int omask;

	omask = sigblock(sigmask(SIGINT));

#ifdef BSD_LINE_DISC
	IOCTL(SHIN, TIOCGETP, (char *)&tty, "7");
	tty_normal = tty;
	tty.sg_flags &= ~CRMOD;
	IOCTL(SHIN, TIOCSETN, (char *)&tty, "8");
#else
	IOCTL(SHIN, TCGETS, &otermio, "7");	/* POSIXLD */
	ntermio = otermio;			/* POSIXLD */
	ntermio.c_oflag |= OCRNL;		/* POSIXLD */
	IOCTL(SHIN, TCSETSW, &ntermio, "8");	/* POSIXLD */
#endif

	(void) write(SHOUT, "\r", 1);

#ifdef BSD_LINE_DISC
	IOCTL(SHIN, TIOCSETN, (char *)&tty_normal, "9");
#else
	IOCTL(SHIN, TCSETSW, &otermio, "9");	/* POSIXLD */
#endif

	(void) sigsetmask(omask);
}

/* Push string contents back into tty queue */
static void
pushback(uchar_t *string)
{

#ifdef BSD_LINE_DISC
	struct sgttyb tty, tty_normal;
#else
	struct termios otermio, ntermio;	/* POSIXLD */
#endif

        int omask;
        register uchar_t *p;

	omask = sigblock(sigmask(SIGINT));
#ifdef BSD_LINE_DISC
	IOCTL(SHOUT, TIOCGETP, (char *)&tty, "10");
	tty_normal = tty;
	tty.sg_flags &= ~ECHO;
	IOCTL(SHOUT, TIOCSETN, (char *)&tty, "11");
#else
	IOCTL(SHOUT, TCGETS, &ntermio, "10");	/* POSIXLD */
	otermio = ntermio;			/* POSIXLD */
	ntermio.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ECHOCTL|ECHOPRT|ECHOKE);
	IOCTL(SHOUT, TCSETSW, &ntermio, "11");	/* POSIXLD */
#endif


        for (p = string; *p; p++) {
		IOCTL(SHOUT, TIOCSTI, p, "12");
	}

#ifdef BSD_LINE_DISC
	IOCTL(SHOUT, TIOCSETN, (char *)&tty_normal, "13");
#else
	IOCTL(SHOUT, TCSETSW, &otermio, "13");	/* POSIXLD */
#endif

	(void) sigsetmask(omask);	
}



/*
 * Concatenate src onto tail of des.
 * Des is a string whose maximum length is count.
 * Always null terminate.
 */
static void
catn(uchar_t *des, uchar_t *src, int count)
{

	while (--count >= 0 && *des)
		des++;
	while (--count >= 0)
		if ((*des++ = *src++) == 0)
			 return;
	*des = '\0';
}

/*
 * Like strncpy but always leave room for trailing \0
 * and always null terminate.
 */
static void
copyn(uchar_t *des, uchar_t *src, int count)
{

	while (--count >= 0)
		if ((*des++ = *src++) == 0)
			return;
	*des = '\0';
}

static uchar_t
filetype(uchar_t *dir, uchar_t *file)
{
	uchar_t path[PATH_MAX+1];
	struct stat statb;

	catn((uchar_t *)strcpy((char *)path, (char *)dir), file, sizeof path);
	if (lstat((char *)path, &statb) == 0) {
		switch(statb.st_mode & S_IFMT) {
		    case S_IFDIR:
			return ('/');

		    case S_IFLNK:
			if (stat((char *)path, &statb) == 0 && /* follow it */
			   (statb.st_mode & S_IFMT) == S_IFDIR)
				return ('>');
			else
				return ('@');

		    case S_IFSOCK:
			return ('=');

		    default:
			if (statb.st_mode & 0111)
				return ('*');
		}
	}
	return (' ');
}

static struct winsize win;

/*
 * Print sorted down columns
 */
static void
print_by_column(uchar_t *dir, uchar_t *items[], int count)
{
	register int i, rows, r, c, maxwidth = 0, columns;

	 if (ioctl(SHOUT, TIOCGWINSZ, (char *)&win) < 0 || win.ws_col == 0)
		win.ws_col = 80;
	for (i = 0; i < count; i++)
		maxwidth = maxwidth > (r = strlen((char *)items[i])) ? maxwidth : r;
	maxwidth += 2;			/* for the file tag and space */
	columns = win.ws_col / maxwidth;
	if (columns == 0)
		columns = 1;
	rows = (count + (columns - 1)) / columns;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < columns; c++) {
			i = c * rows + r;
			if (i < count) {
				register int w;

				printf("%s", items[i]);
				display_char(dir ? filetype(dir, items[i]) : ' ');
				if (c < columns - 1) {	/* last column? */
					w = strlen((char *)items[i]) + 1;
					for (; w < maxwidth; w++)
						display_char(' ');
				}
			}
		}
		display_char('\n');
	}
}

/*
 * Expand file name with possible tilde usage
 *	~person/mumble
 * expands to
 *	home_directory_of_person/mumble
 */
static uchar_t *
tilde(uchar_t *new, uchar_t *old)
{
	register uchar_t *o, *p;
	register struct passwd *pw;
	static uchar_t person[MAX_LOG_NAMLEN];

	if (old[0] != '~')
		return ((uchar_t *)strcpy((char *)new, (char *)old));

	for (p = person, o = &old[1]; *o && *o != '/'; *p++ = *o++)
		;
	*p = '\0';
	if (person[0] == '\0')
		(void) strcpy((char *)new, (char *)value("home"));
	else {
		pw = getpwnam((char *)person);
		if (pw == NULL)
			return (NULL);
		(void) strcpy((char *)new, (char *)pw->pw_dir);
	}
	(void) strcat((char *)new, (char *)o);
	return (new);
}

/*
 * Cause pending line to be printed
 */
static void
retype()
{

#ifdef BSD_LINE_DISC
	int pending_input = LPENDIN;

	IOCTL(SHOUT, TIOCLBIS, (char *)&pending_input, "14");
#else
	struct termios new_termio;			/* POSIXLD */

	IOCTL(SHOUT, TCGETS, &new_termio, "4");		/* POSIXLD */
	new_termio.c_lflag |= PENDIN;			/* POSIXLD */
	IOCTL(SHOUT, TCSETSW, &new_termio, "14");	/* POSIXLD */
#endif

}

static void
beep()
{

	if (adrof("nobeep") == 0)
		(void) write(SHOUT, "\007", 1);
}

/*
 * Erase that silly ^[ and
 * print the recognized part of the string
 */
static void
print_recognized_stuff(uchar_t *recognized_part)
{

	/* An optimized erasing of that silly ^[ */
	switch (strlen((char *)recognized_part)) {

	case 0:				/* erase two character: ^[ */
		printf("\b\b  \b\b");
		break;

	case 1:				/* overstrike the ^, erase the [ */
		printf("\b\b%s \b", recognized_part);
		break;

	default:			/* overstrike both character ^[ */
		printf("\b\b%s", recognized_part);
		break;
	}
	flush();
}

/*
 * Parse full path in file into 2 parts: directory and file names
 * Should leave final slash (/) at end of dir.
 */
static void
extract_dir_and_name(uchar_t *path, uchar_t *dir, uchar_t *name)
{
	register uchar_t  *p;

	p = (uchar_t *)rindex((char *)path, '/');
	if (p == NULL) {
		copyn(name, path, (PATH_MAX));
		dir[0] = '\0';
	} else {
		copyn(name, ++p, (PATH_MAX));
		copyn(dir, path, p - path);
	}
}

static uchar_t *
getentry(DIR *dir_fd, int looking_for_lognames)
{
	register struct passwd *pw;
	register struct dirent *dirp;

	if (looking_for_lognames) {
		if ((pw = getpwent()) == NULL)
			return (NULL);
		return ((uchar_t *)pw->pw_name);
	}
	if (dirp = readdir(dir_fd))
		return ((uchar_t *)dirp->d_name);
	return (NULL);
}

static void
free_items(uchar_t **items)
{
	register int i;

	for (i = 0; items[i]; i++)
		free(items[i]);
	free((char *)items);
}


/*
 * Object: extend what user typed up to an ambiguity.
 * Algorithm:
 * On first match, copy full entry (assume it'll be the only match) 
 * On subsequent matches, shorten extended_name to the first
 * character mismatch between extended_name and entry.
 * If we shorten it back to the prefix length, stop searching.
 */
static int
recognize(uchar_t *extended_name, uchar_t *entry, int name_length, int numitems)
{

	if (numitems == 1)			/* 1st match */
		copyn(extended_name, entry, PATH_MAX);
	else {					/* 2nd & subsequent matches */
		register uchar_t *x, *ent;
		register int len = 0;

		x = extended_name;
		for (ent = entry; *x && *x == *ent++; x++, len++)
			;
		*x = '\0';			/* Shorten at 1st uchar_t diff */
		if (len == name_length)		/* Ambiguous to prefix? */
			return (-1);		/* So stop now and save time */
	}
	return (0);
}

/*
 * Return true if check matches initial characters in template.
 * This differs from PWB imatch in that if check is null
 * it matches anything.
 */
static int
is_prefix(register uchar_t *check, register uchar_t *template)
{

	do
		if (*check == 0)
			return (TRUE);
	while (*check++ == *template++);
	return (FALSE);
}

/*
 *  Return true if the characters in template appear at the
 *  end of check, I.e., are it's suffix.
 */
static int
is_suffix(uchar_t *check, uchar_t *template)
{
	register uchar_t *c, *t;

	for (c = check; *c++;)
		;
	for (t = template; *t++;)
		;
	for (;;) {
		if (t == template)
			return 1;
		if (c == check || *--t != *--c)
			return 0;
	}
}

static int
ignored(register uchar_t *entry)
{
	struct varent *vp;
	register uchar_t **cp;

	if ((vp = adrof("fignore")) == NULL || 
		(cp = vp->vec) == NULL)
		return (FALSE);
	for (; *cp != NULL; cp++)
		if (is_suffix(entry, *cp))
			return (TRUE);
	return (FALSE);
}

/*
 * String compare for qsort.
 */
int
sortscmp(const uchar_t **a1, const uchar_t **a2)
{
	return (strcoll((char *)*a1, (char *)*a2));
}

/*
 * Perform a RECOGNIZE or LIST command on string "word".
 */
int
tenex_search(uchar_t *word, COMMAND command, int max_word_length)
{
#define MAXITEMS 1024
	static uchar_t **items = NULL;
	register DIR *dir_fd;
	register int numitems = 0, ignoring = TRUE, nignored = 0;
	register name_length, looking_for_lognames;
	uchar_t tilded_dir[PATH_MAX + 1], dir[PATH_MAX + 1];
	uchar_t name[PATH_MAX + 1], extended_name[PATH_MAX+1];
	uchar_t *entry;
	uchar_t *org_word;

	if (items != NULL)
		FREE_ITEMS(items);
        org_word = word;
        if (index ((char *)word,'$') != NULL) {
                uchar_t    *p;

                if ((p = (uchar_t *)rindex((char *)word, '/')) == NULL || 
		     index((char *)p, '$') != NULL) {
                        return (0);     /* leave if searching only $foo */
		}
                word = Dfix1(word);
        }

	looking_for_lognames = 
			((*word == '~') && (index((char *)word, '/') == NULL));
	if (looking_for_lognames) {
		(void) setpwent();
		copyn(name, &word[1], PATH_MAX);	/* name sans ~ */
	} else {
		extract_dir_and_name(word, dir, name);
		if (tilde(tilded_dir, dir) == 0)
			return (0);
		dir_fd = opendir(*tilded_dir ? (char *)tilded_dir : ".");
		if (dir_fd == NULL)
			return (0);
	}

again:	/* search for matches */
	name_length = strlen((char *)name);
	for (numitems = 0; entry = getentry(dir_fd, looking_for_lognames); ) {
		if (!is_prefix(name, entry))
			continue;
		/* Don't match . files on null prefix match */
		if (name_length == 0 && entry[0] == '.' &&
		    !looking_for_lognames)
			continue;
		if (command == LIST) {
			if (numitems >= MAXITEMS) {
				if(looking_for_lognames)
					printf (MSGSTR(YIKES1,"\nYikes!! Too many names in password file!!\n"));
				else
					printf (MSGSTR(YIKES2,"\nYikes!! Too many files!!\n"));
				break;
			}
			if (items == NULL)
				items = (uchar_t **)calloc(sizeof (items[1]),
				    MAXITEMS+1);
			items[numitems] = calloc(1,(unsigned)strlen((char *)entry)+1);
			copyn(items[numitems], entry, PATH_MAX);
			numitems++;
		} else {			/* RECOGNIZE command */
			if (ignoring && ignored(entry))
				nignored++;
			else if (recognize(extended_name,
			    entry, name_length, ++numitems))
				break;
		}
	}
	if (ignoring && numitems == 0 && nignored > 0) {
		ignoring = FALSE;
		nignored = 0;
		if (looking_for_lognames)
			(void) setpwent();
		else
			rewinddir(dir_fd);
		goto again;
	}

	if (looking_for_lognames)
		(void) endpwent();
	else
		closedir(dir_fd);
	if (numitems == 0)
		return (0);
	if (command == RECOGNIZE) {
		/* add extended name */
                catn(org_word, extended_name+name_length, max_word_length);
		return (numitems);
	}
	else { 				/* LIST */
		qsort(items, numitems, sizeof(items[1]), 
			(int(*)(const void *, const void *))sortscmp);
		print_by_column(looking_for_lognames ? NULL : tilded_dir,
		    items, numitems);
		if (items != NULL)
			FREE_ITEMS(items);
	}
	return (0);
}

int
tenex(uchar_t *inputline, int inputline_size)
{
	register int numitems, num_read;
	setup_tty(ON);
	while ((num_read = read(SHIN, inputline, inputline_size)) > 0) {
		const static uchar_t delims[] = {  /* WORD DELIMITERS */
			' ',		/* space 		*/
			'\'',		/* apostrophe		*/
			'"',		/* quote		*/
			'\t',		/* tab			*/
			';',		/* semicolon		*/
			'&',		/* ampersand		*/
			'<',		/* left angle		*/
			'>',		/* right angle		*/
			'(',		/* left paren		*/
			')',		/* right paren		*/
			'|',		/* vertical bar		*/
			'^',		/* circumflex		*/
			'%',		/* percent		*/
			'\0'
		};
		register uchar_t *str_end, *word_start, *last_char, should_retype;
		register int space_left;
		COMMAND command;

		last_char = inputline + num_read - 1;
		if (*last_char == '\n' || num_read == inputline_size)
			break;
		command = (*last_char == ESC) ? RECOGNIZE : LIST;
		if (command == LIST)
			display_char('\n');
		str_end = last_char + 1;
		if (*last_char == ESC)
			--str_end;	/* wipeout trailing cmd uchar_t */
		*str_end = '\0';
		/*
		 * Find LAST occurence of a delimiter in the inputline.
		 * The word start is one character past it.
		 */
		for (word_start = last_char = inputline; last_char < str_end;) {
			register int n;
			wchar_t nlc;
			n = mbtowc(&nlc, (char *)last_char, mb_cur_max);
			if (n < 1)
				n = 1;
			else if (any(nlc,(uchar_t *)delims) || 
					iswblank((wint_t)nlc))
				word_start = last_char + n;
			last_char += n;
		}
		space_left = inputline_size - (word_start - inputline) - 1;
		numitems = tenex_search(word_start, command, space_left);

		if (command == RECOGNIZE) {
			/* print from str_end on */
			print_recognized_stuff(str_end);
			if (numitems != 1)	/* Beep = No match/ambiguous */
				beep();
		}

		/*
		 * Tabs in the input line cause trouble after a pushback.
		 * tty driver won't backspace over them because column
		 * positions are now incorrect. This is solved by retyping
		 * over current line.
		 */
		should_retype = FALSE;
		if (strchr((char *)inputline, '\t')) {  /* tab in input line */
			back_to_col_1();
			should_retype = TRUE;
		}
		if (command == LIST)	/* Always retype after a LIST */
			should_retype = TRUE;
		if (should_retype)
			printprompt();
		pushback(inputline);
		if (should_retype)
			retype();
	}
	setup_tty(OFF);
	return (num_read);
}
