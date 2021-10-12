/* @(#)55	1.8.1.4  src/bos/usr/ccs/bin/gprof/gprof.h, cmdstat, bos411, 9428A410j 3/7/94 15:09:58 */
/*
* COMPONENT_NAME: (CMDSTAT) gprof
*
* FUNCTIONS: gprof.h
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Copyright (c) 1980 Regents of the University of California.
* All rights reserved.  The Berkeley software License Agreement
* specifies the terms and conditions for redistribution.
*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	gprof.h	5.1 (Berkeley) 6/4/85
 *
 *	ISC/AIX: gprof.h	1.3 - 89/07/19 - 15:40:50
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <xcoff.h>
#include <mon.h>


    /*
     *	who am i, for error messages.
     */
extern char	*whoami;

    /*
     * booleans
     */
typedef int	bool;
#define	FALSE	0
#define	TRUE	1

    /*
     *	ticks per second
     */
long	hz;

/* typedef	short UNIT;	* unit of profiling */
#define UNIT short
char	*a_outname;
#define	A_OUTNAME		"a.out"
#define rawarc goutcnt

char	*gmonname;
#define	GMONNAME		"gmon.out"
#define	GMONSUM			"gmon.sum"
	
    /*
     *	headers on the flat and graph profiles.
     */

#define	FLAT_HEADER	0
#define	CALLG_HEADER	1

    /*
     *	a constructed arc,
     *	    with pointers to the namelist entry of the parent and the child,
     *	    a count of how many times this arc was traversed,
     *	    and pointers to the next parent of this child and
     *		the next child of this parent.
     */
struct arcstruct {
    struct nl		*arc_parentp;	/* pointer to parent's nl entry */
    struct nl		*arc_childp;	/* pointer to child's nl entry */
    long		arc_count;	/* how calls from parent to child */
    double		arc_time;	/* time inherited along arc */
    double		arc_childtime;	/* childtime inherited along arc */
    struct arcstruct	*arc_parentlist; /* parents-of-this-child list */
    struct arcstruct	*arc_childlist;	/* children-of-this-parent list */
};
typedef struct arcstruct	arctype;

    /*
     * The symbol table;
     * for each external in the specified file we gather
     * its address, the number of calls and compute its share of cpu time.
     */
struct nl {
    char		*name;		/* the name */
    unsigned long	value;		/* the pc entry point */
    unsigned long	svalue;		/* entry point aligned to histograms */
    double		time;		/* ticks in this routine */
    double		childtime;	/* cumulative ticks in children */
    long		ncall;		/* how many times called */
    long		selfcalls;	/* how many calls to self */
    double		propfraction;	/* what % of time propagates */
    double		propself;	/* how much self time propagates */
    double		propchild;	/* how much child time propagates */
    bool		printflag;	/* should this be printed? */
    int			index;		/* index in the graph list */
    int			toporder;	/* graph call chain top-sort order */
    int			cycleno;	/* internal number of cycle on */
    struct nl		*cyclehead;	/* pointer to head of cycle */
    struct nl		*cnext;		/* pointer to next member of cycle */
    arctype		*parents;	/* list of caller arcs */
    arctype		*children;	/* list of callee arcs */
};
typedef struct nl	nltype;

nltype	*nl;			/* the whole namelist */
nltype	*npe;			/* the virtual end of the namelist */
int	nname;			/* the number of function names */

    /*
     *	flag which marks a nl entry as topologically ``busy''
     *	flag which marks a nl entry as topologically ``not_numbered''
     */
#define	DFN_BUSY	-1
#define	DFN_NAN		0

    /* 
     *	namelist entries for cycle headers.
     *	the number of discovered cycles.
     */
nltype	*cyclenl;		/* cycle header namelist */
int	ncycle;			/* number of cycles discovered */


int	debug;

    /*
     * Each discretized pc sample has
     * a count of the number of samples in its range
     */
unsigned UNIT	*samples; 

unsigned long	s_lowpc;	/* lowpc from the profile file */
unsigned long	s_highpc;	/* highpc from the profile file */
unsigned sampbytes;		/* number of bytes of samples */
int	nsamples;		/* number of samples */
double	actime;			/* accumulated time thus far for putprofline */
double	totime;			/* total time for all routines */
double	printtime;		/* total of time being printed */
double scale; 			/* values: each sample covers scale bytes */
unsigned char	*textspace;		/* text space of a.out in core */

    /*
     *	option flags, from a to z.
     */
bool	bflag;				/* headers, too */
bool	cflag;				/* discovered call graph, too */
bool	dflag;				/* debugging options */
bool	eflag;				/* specific functions excluded */
bool	Eflag;				/* functions excluded with time */
bool	fflag;				/* specific functions requested */
bool	Fflag;				/* functions requested with time */
bool	sflag;				/* sum multiple gmon.out files */
bool	zflag;				/* zero time/called functions, too */

    /*
     *	structure for various string lists
     */
struct stringlist {
    struct stringlist	*next;
    char		*string;
};
extern struct stringlist	*elist;
extern struct stringlist	*Elist;
extern struct stringlist	*flist;
extern struct stringlist	*Flist;

    /*
     *	function declarations
     */

/*  enum opermodes {
    literal, indexed, reg, regdef, autodec, autoinc, autoincdef, 
    bytedisp, bytedispdef, worddisp, worddispdef, longdisp, longdispdef,
    immediate, absolute, byterel, bytereldef, wordrel, wordreldef,
    longrel, longreldef
};
typedef enum opermodes	operandenum;

This is all commented out because the Austin compiler puked big green gobs
over handling switch statements with the enum elements in them.

*/
#define LITERAL		0
#define INDEXED		1
#define REG		2
#define REGDEF		3
#define AUTODEC		4
#define AUTOINC		5
#define AUTOINCDEF	6
#define BYTEDISP	7
#define BYTEDISPDEF	8
#define WORDDISP	9
#define WORDDISPDEF	10
#define LONGDISP	11
#define LONGDISPDEF	12
#define IMMEDIATE	13
#define ABSOLUTE	14
#define BYTEREL		15
#define BYTERELDEF	16
#define WORDREL		17
#define WORDRELDEF	18
#define LONGREL		19
#define LONGRELDEF	20

typedef int operandenum;

int		addarc();
int		arccmp();
arctype		*arclookup();
int		asgnsamples();
int		printheader();
int		cyclelink();
int		dfn();
bool		dfn_busy();
int		dfn_findcycle();
bool		dfn_numbered();
int		dfn_post_visit();
int		dfn_pre_visit();
int		dfn_self_cycle();
nltype		**doarcs();
int		done();
int		findcalls();
int		flatprofheader();
int		flatprofline();
bool		funcsymbol();
int		getnfile();
int		getpfile();
int		getstrtab();
int		getsymtab();
int		gettextspace();
int		gprofheader();
int		gprofline();
int		main();
unsigned long	max();
int		membercmp();
unsigned long	min();
nltype		*nllookup();
FILE		*openpfile();
long		operandlength();
operandenum	operandmode();
char		*operandname();
int		printchildren();
int		printcycle();
int		printgprof();
int		printmembers();
int		printname();
int		printparents();
int		printprof();
int		readsamples();
unsigned long	reladdr();
int		sortchildren();
int		sortmembers();
int		sortparents();
int		tally();
int		timecmp();
int		topcmp();
int		totalcmp();
int		valcmp();
char 	*dem_conv();

#define INVAL		-1
#define	LESSTHAN	-1
#define	EQUALTO		0
#define	GREATERTHAN	1

#define	DFNDEBUG	1
#define	CYCLEDEBUG	2
#define	CHARGEDEBUG	4
#define	TALLYDEBUG	8
#define	TIMEDEBUG	16
#define	SAMPLEDEBUG	32
#define	AOUTDEBUG	64
#define	CALLSDEBUG	128
#define	LOOKUPDEBUG	256
#define	PROPDEBUG	512
#define	ANYDEBUG	1024
/* lifted from vax.h until i figure out what's need for the RT */
#define OFFSET_OF_CODE	2
#define	UNITS_TO_CODE	(OFFSET_OF_CODE / sizeof(UNIT))
struct modebyte {
	unsigned int 	regfield:4;
	unsigned int	modefield:4;
	};
    /*
     *	opcode of the `calls' instruction
     */
#define	CALLS	0xfb

    /*
     *	register for pc relative addressing
     */
#define	PC	0xf
