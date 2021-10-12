/* @(#)62	1.4  src/bos/usr/bin/uucp/uucpadm/defs.h, cmduucp, bos411, 9428A410j 8/3/93 16:14:20 */
/* 
 * COMPONENT_NAME: CMDUUCP defs.h
 * 
 * FUNCTIONS: CALL 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  defs.h - constants and other defs for the uucpadm program.
 */

#define MAXENT		8	/* max menu entries */
#define MAXFILE		50000	/* max file size in bytes */
#define MAXLINE		60	/* max size input line */
/*#define MAXLINE		50	/% max size input line */
#define MAXNAME		128	/* max size file name */
#define MAXEDIT		256	/* max edit line */
#define MEMSIZE 	50
#define REQ		75	/* min columns required for error messages */
#define LREQ		24	/* min lines required for menus */
#define SPECMAXENT	18	/* max specify file menu entries */
#define FILEMAX		 5	/* max file entries displayed in spec_*.c */

/*  Configuration files.   */
#define FILES		7	/* number of configuration files */
#define Devices		0	/* position in File array */
#define Systems		1	/* position in File array */
#define Permissions	2	/* position in File array */
#define Poll		3	/* position in File array */
#define Dialers		4	/* position in File array */
#define Dialcode	5	/* position in File array */
#define Sysfiles	6	/* position in File array */

/*  Help file info */
#define  HELPFILE	"/usr/sbin/uucp/uucpadm.hf"
#define  HELPSIZE	29300
#define  INDEXSIZE	100

/* Miscellaneous uucp defines for config files and their access */
#define ACCESS_SYSTEMS  1
#define ACCESS_DEVICES  2
#define ACCESS_DIALERS  3
#define EACCESS_SYSTEMS 4
#define EACCESS_DEVICES 5
#define EACCESS_DIALERS 6
#define SYSFILES	"/etc/uucp/Sysfiles"
#define SYSFILE		"/etc/uucp/Systems"
#define DEVFILE		"/etc/uucp/Devices"
#define DIALERFILE	"/etc/uucp/Dialers"
#define DIALCODES	"/etc/uucp/Dialcodes"
#define PERMISSIONS	"/etc/uucp/Permissions"
#define POLL		"/etc/uucp/Poll"
#define SYSDIR		"/etc/uucp"

/* Subroutine failure return code */
#define FAIL -1

/* String comparisons return same */
#define SAME 0

/* Panel messages. */
#define PANEMSG1	"CNTR D = exit to main menu, CNTR U = add/change, CNTR X = delete"
#define PANEMSG2	"? = help, ~ = EDITOR"
#define PANEMSG3	"CNTR D = exit to main menu, ? = help"

/*
 *  Return code in data from input.
 */
#define CTLD		0x04	/* control D for EOF */
#define CTLU		0x15	/* control U for update */
#define CTLX		0x18	/* control X for delete */
#define ERASE		sg.c_cc[VERASE] /* terminal erase character */
#define VIEW 		'~'	/* Tilde input to invoke vi editor */

/*
 *  Return codes.
 */
#define EX_OK		0	/* normal exit */
#define EX_ACCESS	1	/* unable to access file */
#define EX_IOERR	2	/* i/o error reading/writing file */
#define EX_USAGE	3	/* user usage error */
#define EX_SOFTWARE	4	/* internal software error */
#define EX_SC		5	/* i/o error reading/writing screen */

/*
 *  Easy call and check return.
 */
#define CALL(S)		if ((err = S) != EX_OK) return (err);

/*
 *  Control structures.
 */
struct menu_ent			/* menu entry */
{
    char  *name;		/* name of menu entry */
    int  (*proc) ();		/* name of function to process it */
    int    parm1;		/* parms for that function */
    int    parm2;
    int    parm3;
    int    parm4;
};

struct menu 			/* menu */
{
    char  *tok;			/* help file token */
    char  *title;		/* title of menu */
/*    char  *ent0;		/% zero'th entry string */
    int    count;		/* count of ent ptrs used in this structure */
    struct menu_ent *ent[MAXENT]; /* ptrs to menu entries */
};


struct menu_shell 			/* menu shell */
{
    char  *toks;		/* help file token */
    char  *header;		/* title of menu */
    int   counts;		/* # of menu entries */
    char  *entry[MAXENT];	/* menu entries */
};

struct spec_shell 			/* specify optional filename shell */
{
    char  *toks;		/* help file token */
    char  *header;		/* title of menu */
    int   counts;		/* # of menu entries */
    char  *entry[SPECMAXENT];	/* menu entries */
};

struct files 			/* file structure */
{
    int  fd;			/* file descriptor */
    int  length;		/* file length */
    int  spot;			/* file position in File array */
    char  *name;		/* file name */
    char  *delimit;             /* file delimit chars */
    char  *comment;             /* file comment chars */
    char  *cont;                /* file continue chars */
};
