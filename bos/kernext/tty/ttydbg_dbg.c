#ifndef lint
static char sccsid[] = "@(#)25 1.3 src/bos/kernext/tty/ttydbg_dbg.c, sysxtty, bos411, 9428A410j 6/17/94 00:55:54";
#endif
/*
 *
 * COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
 *
 * FUNCTIONS:	tty_read_mem, tty_read_word,
 *		tty_db_printf,
 *		tty_print,
 *		tty_db_usage, tty_db_parse_line, tty_db_do_command,
 *		tty_db_disp_tty,
 *		tty_db_kdb_is_avail.
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/lockl.h>
#include "ttydbg.h"

/*
 * Functions declared in that file.
 */
/* Externals */
int		tty_read_mem();
unsigned int	tty_read_word();
int		tty_db_printf();

/* Pseudo Externals */
void		tty_print();

/* Internals */
static void	tty_db_usage();
static int	tty_db_parse_line();
static int	tty_db_do_command();
static int	tty_db_disp_tty();

static int	tty_db_kdb_is_avail();

/*
 * lldb tty subcomand
 */
#define	VIRT	1

#define	VALC(c)		((c)>=0&&(c)<=256)
#define ISDIGIT(c)	(((c>='0') && (c<='9')))
#define ISALPHA(c)	(((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')))
#define ISXDIGIT(c)	((((c>='0') && (c<='9')) || ((c>='a') && (c<='f')) || \
			 ((c>='A') && (c<='F'))))
#define TOLOWER(c)	((((c>='A') && (c<='Z')) ? c + ('a' - 'A') : c))
#define	isdigit(c)	(VALC(c) ? ISDIGIT(c) : 0)
#define	isalpha(c)	(VALC(c) ? ISALPHA(c) : 0)

#define	TTY_SUBCOM	"tty"
#define	TTY_DB_SCREEN_SZ	22
#define TTY_DB_SCREEN_DELTA	3
#define TTY_DB_SCREEN_UTIL	(TTY_DB_SCREEN_SZ - TTY_DB_SCREEN_DELTA)
int tty_db_screen_sz;
#define TTYDBG_DBG_NOMORE()					\
	if (!(tty_db_screen_sz++%(TTY_DB_SCREEN_UTIL))) {	\
		if(debpg() == FALSE)				\
			return(TTYDBG_NOMORE);			\
	}

#define TTYDBG_PAGE_BREAK()				\
	if(debpg() == FALSE) {				\
		return(TTYDBG_NOMORE);			\
	}

#ifdef	_KDB
char *name;
int maj;
int min;
int chan;

int vflag;
int oflag;
int dflag;
int lflag;
int eflag;
#else	/* _KDB */
static char *name;
static int maj;
static int min;
static int chan;

static int vflag;
static int oflag;
static int dflag;
static int lflag;
static int eflag;
#endif	/* _KDB */

/*
 * Kdb extended I/F used by kernext display modules
 * For db_printf, 1st parameter is the print format
 */
#define DB_READ_MEM  1
#define DB_READ_WORD 2
#define DB_READ_BYTE 3

#define db_printf                  brkpoint
#define db_read_mem(vad, lad, siz) brkpoint(DB_READ_MEM, vad, lad, siz)
#define db_read_word(vad)          brkpoint(DB_READ_WORD, vad)
#define db_read_byte(vad)          brkpoint(DB_READ_BYTE, vad)

#define STATIC_BREAK_TRAP	0x7C810808

/************************************************************************/
/************************************************************************/
/*	Routines called by external modules.                            */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_read_mem()
 * 
 * PARAMETERS:	-addr is the address that the caller wants wants to read from
 *		-buffer is the user buffer used to collect the informations
 *		-size is the size to read in memory
 *
 * FUNCTION:	read in memory from addr to buffer size bytes
 *
 * RETURNS:	0  in case of success
 *		-1 if the read or copy failed
 */

int
tty_read_mem(void *addr, void *buffer, unsigned int size)
{
	int kdb_return = 0;

#ifdef DEBUG_TTYDBG
	if (tty_db_kdb_is_avail()) {
		kdb_return = db_read_mem(addr, buffer, size);
		if (kdb_return != size) {
			printf("tty_read_mem : Unable to read at address %08x\n",addr);
			return(-1);
		} else {
			return(0);
		}
	}
	else {
		if (get_from_memory(addr, VIRT, buffer, size)) {
			return(0);
		}
		else {
			printf("tty_read_mem : Unable to read at address %08x\n",addr);
		return(-1);
		}
	}
#else	/* DEBUG_TTYDBG */
#ifdef _KDB
	kdb_return = db_read_mem(addr, buffer, size);
	if (kdb_return != size) {
		printf("tty_read_mem : Unable to read at address %08x\n",addr);
		return(-1);
	} else {
		return(0);
	}
#else	/* _KDB */
	if (get_from_memory(addr, VIRT, buffer, size)) {
		return(0);
	}
	else {
		printf("tty_read_mem : Unable to read at address %08x\n",addr);
		return(-1);
	}
#endif	/* _KDB */
#endif	/* DEBUG_TTYDBG */
}


/*
 * NAME:	tty_read_word()
 * 
 * PARAMETERS:	-addr is the address to read from
 *
 * FUNCTION:	read 4 bytes from addr
 *
 * RETURNS:	4 byes readden
 */

unsigned int
tty_read_word(void *addr)
{
	int intermed;

#ifdef DEBUG_TTYDBG
	if (tty_db_kdb_is_avail()) {
		return(db_read_word(addr));
	}
	else {
		if (get_from_memory(addr, VIRT, &intermed, sizeof(intermed))) {
			return(intermed);
		}
		else {
			printf("tty_read_word : Unable to read at address %08x\n",addr);
			return(-1);
		}
	}
#else	/* DEBUG_TTYDBG */
#ifdef	_KDB
	return(db_read_word(addr));
#else	/* _KDB */
	if (get_from_memory(addr, VIRT, &intermed, sizeof(intermed))) {
		return(intermed);
	}
	else {
		printf("tty_read_word : Unable to read at address %08x\n",addr);
		return(-1);
	}
#endif	/* _KDB */
#endif	/* DEBUG_TTYDBG */
}

/*
 * NAME:	tty_db_printf()
 * 
 * PARAMETERS:	-fmt	string to format.
 *		-a1-a7	arguments to format
 *
 * FUNCTION:	Pages the output for the tty print routines.
 *		It is assumed that there is at most one '\n' in the string.
 *
 * RETURNS:	0  in case of success
 *		-2 if no more display (TTYDBG_NOMORE)
 */

int
tty_db_printf(char *fmt, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
	char	*p;
	int i;

	printf(fmt, a1, a2, a3, a4, a5, a6, a7);

	p = fmt;
	i = strlen(fmt);
	do {
		if (*p != '\n')
			continue;
		if (!(tty_db_screen_sz++%(TTY_DB_SCREEN_UTIL))) {
			if (fmt[i-1] != '\n')	/* push debpg mess next line */
				printf("\n");
			if(debpg() == FALSE)
				return(TTYDBG_NOMORE);
		}
	} while (*p++);

	return(0);
}

/************************************************************************/
/************************************************************************/
/*	Internal Routines but                                           */
/*	called by external modules through a function pointer           */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_print()
 *
 * PARAMETERS:	the buffer from the command.
 *
 * FUNCTION:	lldb tty sub-command.
 *
 * RETURNS: 	none
 */
void
tty_print(char *argv)
{
	if (!tty_db_parse_line((argv + strlen(TTY_SUBCOM))))
		return;

	if (tty_db_do_command())
		return;
}

/************************************************************************/
/************************************************************************/
/*	Internal Routines.                                              */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_db_usage()
 *
 * PARAMETERS:	none
 *
 * FUNCTION:	print the tty command usage.
 *
 * RETURNS: 	none
 */
static void
tty_db_usage()
{
	printf("Usage: tty  [-d | -l | -e] [-o] [-v] [name | [maj [min]]]\n");
	printf("       Displays tty data structure.\n");
	printf("       If no parameter are specified a short listing of all terminals\n");
	printf("       is displayed\n");
	printf("       flags : -d displays the driver information\n");
	printf("               -l displays the line discipline information\n");
	printf("               -e displays information for every module and driver\n");
	printf("                  present in the stream for the selected lines\n");
	printf("               -o displays information about opened lines only\n");
	printf("               -v verbose, displays a long listing\n");
	printf("       selected terminals :\n");
	printf("               name, the terminal specified by this device name is\n");
	printf("               listed\n");
	printf("               major, all the terminals with the specified major are\n");
	printf("               listed\n");
	printf("               major and minor, the terminal with both the specified major\n");
	printf("               and minor is listed\n");
}

/*
 * NAME:	tty_db_parse_line()
 *
 * PARAMETERS:	the buffer from the command.
 *
 * FUNCTION:	tty command analysis.
 *
 * RETURNS: 1 on success, 0 otherwise
 */
static int
tty_db_parse_line(char *s)
{
	int parsed = 0;
	int len;

	/*
	 *	Global data must be cleaned on each tty command.
	 */
	name = NULL;
	maj = -1;
	min = -1;
	chan = -1;

	vflag = 0;
	oflag = 0;
	dflag = 0;
	lflag = 0;
	eflag = 0;

	while (*s && *s == ' ') ++s;
  
	if ((len = strlen(s)) && s[len-1] == '\n')
		s[len-1] = '\0';
  
	/* check for new user args -d -l -e -v -o */
	while (*s && *s == '-') {
		s++;
		switch (*s) {
		case 'd':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			dflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'l':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			lflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'e':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			eflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'v':
			vflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'o':
			oflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		default:
			tty_db_usage();
			return(0);
		}
	}

	/* if (*s == 'o') { */
	/* necessary because crash and pstat still call tty with 'o' arg as
	 * default.  it also indicates that there are no other args provided.
	 */
	/* this is not possible with lldb but keep in comments: crash comparison
		oflag = 1;
		eflag = 1;  
		do ++s;  while (*s && *s == ' ');
		return(1);
	}
	*/
	/* tty name passed as arg */
	if (*s && isalpha(*s)) {
		name = s;
		/* no option given; set eflag */
		if (!lflag && !eflag && !dflag)
			eflag = 1;
		return(1);
	}
	/* maj/min numbers passed as args */
	name = 0;
	if (isdigit(*s)) {
		maj = 0;
		do maj = maj * 10 + *s++ - '0'; while (isdigit(*s));
		do ++s;  while (*s && *s == ' ');
		/* no option given; set eflag */
		if (!lflag && !eflag && !dflag)
			eflag = 1;
		parsed = 1;
	}
	if (isdigit(*s)) {
		min = 0;
		do min = min * 10 + *s++ - '0'; while (isdigit(*s));
		do ++s;  while (*s && *s == ' ');
		parsed = 1;
	}

	if (!parsed) {
		/* equivalent to the crash 'o' arg as default */
		/* tty_db_usage();  /* not sure this is possible */
		oflag = 1;
		eflag = 1;  
		do ++s;  while (*s && *s == ' ');
		return(1);
	}
	return(parsed);
}

/*
 * NAME:	tty_db_do_command()
 * 
 * PARAMETERS:	none, but use data set by tty_db_parse_line().
 *
 * FUNCTION:	perform the tty command.
 *		"1st level" = look through the general tty's table.
 *		Treats the -o option.
 *
 * RETURNS:	0   in case of success
 *		!=0 otherwise (return code of tty_db_disp_tty()).
 */
static int
tty_db_do_command()
{
	struct all_tty_s *refp;
	struct per_tty *per_ttyp;
	int ii, res = 0, once = 0;

	tty_db_screen_sz = 0;
	refp = tty_list;
	while (refp) {  
		for (ii = 0; ii < STR_TTY_CNT; ii++) {
			per_ttyp = &(refp->lists[ii]);	/* easier to handle */

			/* if the -o option was given - we only want to look at
			 * open ttys.  we know it's open by the presence of the 
			 * streamhead tty (strh_tty) in the push_modules array.
			 * the streamhead tty will always be at position
			 * MODULEMAX-1 in this array if it is there at all. to
			 * see if it is there check if mod_ptr == NULL. For
			 * safety, should also check for the module being of
			 * type 's', this will require reading the struct though
			 */

			if (oflag &&
			    per_ttyp->mod_info[MODULEMAX-1].mod_ptr == NULL)
				continue;

			/* there will always be something in the ttyname field
			 * if this per_tty is being used - at this point we
			 * have, i think, a vadr in the field, but i should be
			 * able to check for NULL before going off to do that
			 */
			if (per_ttyp->ttyname[0] != '\0') {
				/* the problem with this is that it continues to
				 * search for possible candidates to print even
				 * after it has printed the one it's looking
				 * for - in the case of name and maj/min 
				 */
				if ((name ?
				     once = !strcmp(name, per_ttyp->ttyname) :
				     (maj == -1 ||
				     (maj == major(per_ttyp->dev) &&
				     (min == -1 ||
				     (min == minor(per_ttyp->dev)))))) &&
				     (res = tty_db_disp_tty(per_ttyp))) {
					return(res);
				}
			}
			if (once)
				return(0);
		}

		refp = refp->next;
	}
	return(res);
}

/*
 * NAME:	tty_db_disp_tty()
 * 
 * PARAMETERS:	per_ttyp points to a per_tty structure.
 *
 * FUNCTION:	perform the tty command.
 *		"2nd level" = look through the per_tty structure.
 *		Treats the -d, -l -e options.
 *		The verbose flag -v , will be passed through to the module
 *		print routine.
 *
 * RETURNS:	0   in case of success
 *		!=0 otherwise (return code of the module print routine).
 */
static int
tty_db_disp_tty (struct per_tty *per_ttyp)
{
	struct str_module_conf *modp;
	void *priv;
	int ii, arg, found = 0, res = 0;

	arg = (vflag ? TTYDBG_ARG_V : 0);

	if (tty_db_screen_sz) {		/* ask for page break after each tty */
		TTYDBG_PAGE_BREAK();
	}
	printf("\n");
	printf("/dev/%s (%d,%d)\n", per_ttyp->ttyname,
		major(per_ttyp->dev), minor(per_ttyp->dev));
	tty_db_screen_sz = 3;

	for (ii = MODULEMAX - 1; ii >= 0; ii--) {
		modp = per_ttyp->mod_info[ii].mod_ptr;	/* easier to handle */
		priv = per_ttyp->mod_info[ii].private_data;
		if (modp) {
			if (dflag) {
				if (modp->type == 'd') {
							/* driver only */
					printf("Module name: %s\n", modp->name);
					tty_db_screen_sz++;
					res = modp->print_funcptr(priv, arg);
					break;
				}
			}
			else if (lflag) {
				if (modp->type == 'l') {
							/* line discipline */
					printf("Module name: %s\n", modp->name);
					tty_db_screen_sz++;
					res = modp->print_funcptr(priv, arg);
					break;
				}
			}
			else {				/* all modules/driver */
				if (modp->name[0] == '\0') {
					continue;
				}
				printf("Module name: %s\n", modp->name);
				tty_db_screen_sz++;
				res = modp->print_funcptr(priv, arg);
				if (res)
					break;
			}
		}
	}
	return(res);
}

/*
 * NAME:	tty_db_kdb_is_avail()
 * 
 * PARAMETERS:	none
 *
 * FUNCTION:	test if kdb is present.
 *
 * RETURNS:	0   kdb is not present
 *		!=0 kdb is present
 */
static int
tty_db_kdb_is_avail()
{
	extern int brkpoint();
	int *ba_addr;

	/* Get the address of brkpoint.
	 * This is contained in the first word of the
	 * function descriptor
	 */
	ba_addr = (int *)*((int *)brkpoint);

	/* Check the TRAP code
	 */
	return (*ba_addr != STATIC_BREAK_TRAP);
}
