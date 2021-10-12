static char sccsid[] = "@(#)64	1.20.1.10  src/bos/usr/ccs/bin/prof/prof.c, cmdstat, bos41B, 9504A 12/21/94 13:42:21";
/*
 * COMPONENT_NAME: (CMDSTAT) Displays Object File Profile Data
 * 
 * FUNCTIONS: prof
 *
 * ORIGINS: 27, 26, 3
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
 *	Usage:
 *
 *	prof [ -[ntca] ] [ -[ox] ] [-S] [ -g ] [ -z ] [ -s ] [ -v ] [ -m mdata ] 	     [ prog ]
 *
 *	Where "prog" is the program that was profiled; "a.out" by default.
 *	Options are:
 *
 *	-n	Sort by symbol name.
 *	-t	Sort by decreasing time.
 *	-c	Sort by decreasing number of calls.
 *	-a	Sort by increasing symbol address.
 *
 *	The options that determine the type of sorting are mutually exclusive.
 *	Additional options are:
 *
 *	-o	Include symbol addresses in output (in octal).
 *	-x	Include symbol addresses in output (in hexadecimal).
 *	-g	Include non-global T-type symbols in output.
 *	-z	Include all symbols in profiling range, even if zero
 *			number of calls or time.
 *	-h	Suppress table header.
 *	-s	Follow report with additional statistical information.
 *  	-v  Suppress report and generate graphic/plot output of sample data
 *	-m mdata Use file "mdata" instead of MON_OUT for profiling data.
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>			/* Needed for "stat.h". */
#include <sys/stat.h>
#include <sys/param.h>			/* for HZ */
#include <stdlib.h>
#include <a.out.h>
#include <mon.h>
#include <locale.h>
#include <filehdr.h>
#include <ldfcn.h>
#include <ar.h>
#include <syms.h>

#include        <nl_types.h>
#include		<demangle.h>
#include        "prof_msg.h"
static nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_PROF,num,str)  /*MSG*/

#define MON_SUM	"mon.sum"
#define MAXSTRING	2000
#define DBG	1		/* Turn debugging on for now */
#undef DBG			/* Turn debugging OFF for now */
#define DEBUG_IT(exp)	if (debug) exp;

#define	LOC(a) if (debug) fprintf (stderr,"location is: %x\n",ftell(a));

#define	Do_READ(a,b,c,d,e) if (fread((void *) a, (size_t) b, (size_t) c, d) !=c){ \
		eofon(a, a_outname,e); \
}


/* The ISMAGIC macro should be defined in some system (i.e. global)
 * header.  Until it is, it is defined here.  It can also be found
 * in various copies of sgs.h.  This macro requires a.out.h for
 * the magic numbers.
 */
#define ISMAGIC(x)   ((((unsigned short)x)==(unsigned short)U802WRMAGIC) || \
		     (((unsigned short)x)==(unsigned short)U802ROMAGIC) || \
		     (((unsigned short)x)==(unsigned short)U802TOCMAGIC) || \
		     (((unsigned short)x)==(unsigned short)U800WRMAGIC) || \
		     (((unsigned short)x)==(unsigned short)U800ROMAGIC) || \
		     (((unsigned short)x)==(unsigned short)U800TOCMAGIC))

#define PROC				/* Mark procedure names. */


	/* Max positive difference between a fnpc and sl_addr for match */
	/* That's 0x38 bytes for prolog overhead and 4 bytes per extra */
	/* base register (there's no way to know how many of the extra base */
	/* registers were actually used by a routine so I allow for the */
	/* maximum. */

#define CCADIFF	(0x38+(4*5))
	/* Type if n_type field in file symbol table entry. */

#define SEC(ticks) ((double)(ticks)/HZ)		/* Convert clock ticks to seconds. */

	/* Title fragment used if symbol addresses in output ("-o" or "-x"). */
static char atitle[] = " Address ";
	/* Format for addresses in output */
static char aformat[] = "%8o ";
static unsigned short text_section_number;


   /* Used for "unsigned fixed-point fraction with binary scale at the left". */
#define BIAS	((uint)0200000L)	/* Note this is 0x10000 */

static int gflag = 0;			/* mjm: replaces gmatch and gmask */

static int	debug=0;
static int 	multi_monfiles=0;	/* incremented for each mon.out file used */
static int	nmlen=20;		/* What is our widest name      */
static FILE	*sym_iop;		/* For program ("a.out") file.  */
static FILE	*mon_iop;		/* For profile (MON_OUT) file.  */
static FILE	*fpsum;			/* For summusry (MON_SUM) file. */
static char	*a_outname = "";	/* Default program file name.   */
static char	*mon_fn = MON_OUT;	/* Default profile file name.   */
static char	**mon_fnp;		/* Default profile file name.   */
static char	*mon_sum = "mon.sum";	/* file name for summary file.  */
static float 	last_sf=999;		/* last Scale for index into pcounts:
					i(pc) = ((pc - pc_l) * sf)/sf.
				   initialized to 999 for ID           */
static int	scalechanged=0;		/* flag set if scale factor changes    */


static PROC static void charge_ticks_to_routines();
static PROC static void charge_call_counts_to_routines();
static PROC static void getnames ();
extern char *optarg;
extern int errno, optind;

	/* For symbol table entries read from program file. */
static struct syment nl;	/* from syms.h */

	/* Number of chars in a symbol. For subsequent sizes & counts. */
#define N_NAME	(SYMNMLEN*3)	/* we make the size three times the normal
				 * size just incase we have long names..
				 */

/* Local representation of symbols and call/time information. */
struct slist {
	char sl_name[N_NAME+5];	/* Symbol name. */
	char *sl_addr;		/* Address. */
	long sl_count;		/* Count of subroutine calls */
	float sl_time;		/* Count of clock ticks in this routine,
						converted to secs. */
};

/* Compare routines called from qsort(). */
PROC int c_ccaddr(struct poutcnt *p1, struct poutcnt *p2); /* Compare fnpc fields of cnt structures. */
static PROC int c_sladdr(struct slist *p1, struct slist *p2); /* Compare   sl_addr fields of slist structures. */
static int c_time(struct slist *p1, struct slist *p2);	/*	"    sl_time    "    "   "	"      */
static int c_name(struct slist *p1, struct slist *p2);	/*	"    sl_name    "    "   "	"      */
static int c_ncalls(struct slist *p1, struct slist *p2);	/*	"    sl_count   "    "   "	"      */
	/* Other stuff. */

	/* Return size of open file (arg is file descriptor) */
static off_t	fsize();

	/* Memory allocation. Like malloc(), but no return if error. */
static char	*Malloc();

	/* Scan past path part (if any) in the ... */
static char	*basename();

	/* command name, for error messages. */
static char	*cmdname;

	/*  Conversion routine for demangled name.  */
static char 	*dem_conv();
#define 	INVAL	-1

/* Structure of subroutine call counters (cnt) is defined in mon.h. */
/* Structure for header of mon.out (hdr) is defined in mon.h. */

static unsigned long	cur;

static struct outhdr head,oldhead;	/* Profile file (MON_OUT) header. */

static int	(*sort)(void *, void *) = NULL;	/* Compare routine for sorting output symbols.
						Set by "-[acnt]". */

static int	flags;		/* Various flag bits. */
static char	*pc_l;		/* From head.lpc. */
static char	*pc_h;		/*   "  head.hpc. */

static HISTCOUNTER *pcounts;	/* Pointer to allocated area for pcounts: 
			 * PC clock hit counts 
			 */

static int vn_cc, n_cc; 	/* Number of cnt structures in profile
			 * data file (later # ones used). 
			 */

static struct poutcnt *ccounts;	/* Pointer to allocated area for cnt
					structures: subr PC-call counts. */

static int n;
static int n_pc;	/* Number of pcounts in profile data file. */
static struct poutcnt *ccp;	/* For scanning ccounts. */

/********************************/
/*	plot related data	*/
/********************************/

static double		ransca, ranoff;		/* scaling for blowing up plots	*/
#define	sampbytes  (n_pc * sizeof(HISTCOUNTER))/* number of bytes of samples	*/
static double 		maxtime;		/* maximum time of any routine	*/
					/* (for plot) 	 		*/
static double		scale;			/* scale factor	converting	*/
					/* samples to pc values: each	*/
					/* sample covers scale bytes	*/
/*	end of plot stuff	*/

/* Bit macro and flag bit definitions. */

#define FBIT(pos)	(01 << (pos))	/* Returns value with bit pos set. */
#define F_SORT		FBIT(0)		/* Set if "-[acnt]" seen. */
#define F_VERBOSE	FBIT(1)		/* Set if "-S" seen. */
#define F_ZSYMS		FBIT(2)		/* Set if "-z" seen. */
#define F_PADDR		FBIT(3)		/* Set if "-o" or "-x" seen. */
#define F_NHEAD		FBIT(4)		/* Set if "-h" seen. */
#define F_SUM		FBIT(5)		/* Set if "-s" seen. */
#define F_PLOT		FBIT(6)		/* Set if "-v" seen. */

static struct slist *slist;	/* Pointer to allocated slist structures: symbol
					name/address/time/call counts */
static int total_symbols=0;	/* Number of text symbols (of proper type)
				that fill in range of profiling.      */
static int symttl;		/* Total # symbols in program file sym-table  */
static float t,t0,t_tot=0.0;	/* Total time: SEC(sum of all pcounts[i])     */
static struct slist *slp;	/* For scanning slist */
static struct slist *newslp,*newslist;
static int load_modules=0;
static char	*newpath;		/* redirecting load object module   */
static int 	Lflg=0,		/* Different shared object pathname */
	lflg=0;		/* reroute objects as well...       */


PROC
main(argc, argv)
int argc;
char **argv;
{
	char buffer[BUFSIZ];	/* buffer for printf */
	int n_nonzero;		/* Number of (above symbols) actually printed
					because nonzero time or # calls. */
	int i;
	off_t symaddr;		/* Address of symbol table in program file. */

	long s_inv;		/* Inverse: i_inv(i) =
					{pc00, pc00+1, ... pc00+s_inv-1}. */

	unsigned pc_m;		/* Range of PCs profiled: pc_m = pc_h - pc_l */
	register HISTCOUNTER *pcp;	/* For scanning pcounts. */
	float t, t0;
	int n,totstaticfuncs;


	int	lowpct, highpct;
	/*
	 *	Use highpct and lowpct as percentages, temporarily
	 *	for graphing options involving blow-up
	 */
	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_PROF,NL_CAT_LOCALE);

	lowpct	= -1;
	highpct	= -1;

	mon_fnp=0;
	cmdname = basename(*argv);	/* command name. */

	argv++;
	while ( *argv != 0 && **argv == '-' ) {
		int (*fcn)(void *, void *);

		*argv += 1;
		if (**argv == 'g') {
			gflag++;
			goto endopt;
		}
		if ((**argv == 'o') || (**argv == 'x')) {
			aformat[2] = **argv;
			flags |= F_PADDR;
			goto endopt;
		}
		if (**argv == 'L') {	/* different directory */
			Lflg++;
			(*argv)++;
			if (**argv == '\0')
				argv++;
			newpath= *argv;
			goto endopt;
		}
#ifdef DBG
		if (**argv == 'd') {	* turn debugging on *
			debug++;
			goto endopt;
		}
		if (**argv == 'j') {	* turn debugging on *
			debug++;
			goto endopt;
		}
#endif
		if (**argv == 'z') {
			flags |= F_ZSYMS;
			goto endopt;
		}
		if (**argv == 'v') {
			flags |= F_PLOT;
			goto endopt;
		}
		if (**argv >= '0' && **argv <= '9') {
			int i = atoi(*argv);
			if (lowpct == -1)
				lowpct = i;
			    else
				highpct = i;
			goto endopt;
		}
		if (**argv == 'h') {
			flags |= F_NHEAD;
			goto endopt;
		}
		if (**argv == 's') {
			flags |= F_SUM;
			goto endopt;
		}
		if (**argv == 'S') {
			flags |= F_VERBOSE;
			goto endopt;
		}
		else if (**argv == 'n') {
			fcn = c_name;
			goto check;
		}
		else if (**argv == 't') {
			fcn = c_time;
			goto check;
		}
		else if (**argv == 'c') {
			fcn = c_ncalls;
			goto check;
		}
		else if (**argv == 'a') {
			fcn = NULL;
			goto check;
		}
/*	Once you get -m, all after that are mon.out files	*/
		else if (**argv == 'm') {
			argv++;
			mon_fnp = argv;
			goto done;
		} else {
		   	fprintf(stderr,
		     MSGSTR(NOTREC,"%s: %c is not a recognized option\n"), cmdname, **argv);
			exit(1);
		}
		check:
		     if ( sort != NULL && sort != fcn) {
		   	fprintf(stderr,
		     MSGSTR(OVERRIDE,"%s: Warning: %c overrides previous specifications\n"),
			cmdname, **argv);
		     }
		     sort = fcn;
		     flags |= F_SORT;
		endopt:
		argv++;
	}
/*	This can happen ONLY if you have an a.out		*/
	if ( *argv != 0 ) {
		a_outname = *argv;
		lflg++;
		argv++;
	}
/*	This occurs if a.out was specified followed by -m mon.out ...	*/
	if ( *argv != 0 ) {
		if ( **argv == '-') {
		    if ( *(*argv+1) == 'm' ) {
		        *argv++;
		        mon_fnp = argv;
		    } else { 
		        fprintf(stderr,
 MSGSTR(ONLYOPT,"%s: The only option allowed after a.out is '-m mon.out ...'\n"),
		cmdname, **argv);
			exit(1);
		    }
		} else {
		    mon_fnp = argv;
		}
	}
	done:

	if (flags & F_PLOT) {		/* Turn off output related flags if plot */
		flags |= F_NHEAD;
		flags &= ~F_PADDR;
		flags &= ~F_VERBOSE;
	}
	if (lowpct >= 100)
		lowpct = 0;
	if (highpct <= lowpct || highpct > 100)
		highpct = 100;
	ransca = 100. / (highpct - lowpct);
	ranoff = 2040. + 40.8 * (long)pc_l * ransca;

	if (sort == NULL && !(flags & F_SORT))
				/* If have not specified sort mode ... */
		sort = c_time;		/* then sort by decreasing time. */




	if ( mon_fnp == 0 )
	{
		mon_proc();
	} else { 
		do {
		    mon_fn = *mon_fnp;
		    DEBUG_IT(fprintf(stderr,"calling mon_proc with %s\n",mon_fn);)
		    mon_proc();
		    multi_monfiles++;
		    mon_fnp++;
		} while ( *mon_fnp );
	}

	slp=slist;
	if(!gflag) {
        	totstaticfuncs=0;
        	for(n=0;n<total_symbols;n++,slp++) {
                	if(slp[0].sl_count == -1) {
                        	totstaticfuncs++;
                        	continue;
                	}
                	newslp[0]=slp[0];
                	newslp++;
        	}
        	slist=slp=newslist;
        	total_symbols-=totstaticfuncs;
	}



	/*
	Most of the heavy work is done now.  Only minor stuff remains.
	The symbols are currently in address order and must be re-sorted
	if desired in a different order.  Report generating options
	include "-o" or "-x": Include symbol address, which causes another column
	in the output; and "-z": Include symbols in report even if zero
	time and call count.  Symbols not in profiling range are excluded
	in any case.  Following the main body of the report, the "-s"
	option causes certain additional information to be printed.
	*/

	if (sort)	/* If comparison routine given then use it. */
		qsort(slp, total_symbols, sizeof(struct slist), sort);

	if (!(flags & F_NHEAD)) {
		if (flags & F_PADDR)
			printf(atitle);	/* Title for addresses. */
		printf("%-*.*s ",nmlen,nmlen,MSGSTR(TITL,"Name              ")); 
		(void)puts(MSGSTR(TITLE,"%Time     Seconds     Cumsecs  #Calls   msec/call"));
	}
	if (flags & F_PLOT) {
		double		time, lastx, lasty, lastsx;
		register int	i;

		openpl();
		erase();
		space(-2048, -2048, 2048, 2048);
		line(-2040, -2040,  -2040, 2040);
		line(0, 2040, 0, -2040);
		for(i=0; i<11; i++)
			line(-2040, 2040-i*408, 0, 2040-i*408);
		lastx	= 0.;
		lasty	= ranoff;
		scale	= (4080.*ransca)/(sampbytes/sizeof(char));
		lastsx	= 0.0;
		for(i = 0; i < n_pc ; i++)	{
			HISTCOUNTER	ccnt;
			double	tx, ty;
			ccnt = pcounts[i];
			time = ccnt;
			tx = lastsx;
			ty = lasty;
			lastsx -= 2000.*time/t_tot;
			lasty  -= scale;
			if (lasty >= -2040. && ty <= 2040.)	{
				line((int)tx, (int)ty, (int)lastsx, (int)lasty);
				if (ccnt != 0 || lastx != 0.0)	{
					tx = lastx;
					lastx = -time*2000./maxtime;
					ty += scale/2;
					line(0, (int)ty, (int)tx, (int)ty);
				}
			}
		}
		scale = (4080.*ransca)/(pc_h - pc_l);
		lastx = 50.;
		for(n = total_symbols, slp=slist; --n >= 0; slp++)	{
			if (slp->sl_addr < pc_l)
				continue;
			if (slp->sl_addr >= pc_h)
				continue;
			if (flags & !F_ZSYMS && slp->sl_time == 0.0 && slp->sl_count == 0)
				continue;
			time = slp->sl_time / t_tot;
			lasty = ranoff - (slp->sl_addr - pc_l) * scale;
			if (lasty >= -2040. && lasty <= 2040.)	{
				char	bufl[BUFSIZ];
				register	j;
				line(0, (int)lasty, 50, (int)lasty);
				line((int)(lastx-50), (int)lasty, (int)lastx, (int)lasty);
				move((int)(lastx+30), (int)(lasty+10));
				sprintf(bufl, "%s", slp->sl_name + (slp->sl_name[0] == '_'));
				label(bufl);
			}
			lastx += 500.;
			if (lastx > 2000.)
				lastx = 50.;
		}
	point(0, -2040);
	closepl();
	exit(0);
	}

/*	NON PLOT ~(-v) */
	t = 0.0;			/* Init cumulative time. */
	if (t_tot != t)			/* Convert to percent. */
		t_tot = 100.0/t_tot;	/* Prevent divide-by-zero fault */
	n_nonzero = 0;	/* Number of symbols with nonzero time or # calls. */
	for (n = total_symbols, slp = slist; --n >= 0; slp++) {
		long count = slp->sl_count;	/* # Calls. */

		t0 = slp->sl_time;	/* Time (sec). */
		if (t0 == 0.0 && count == 0 && !(flags & F_ZSYMS))
			continue; /* Don't do entries with no action. */
		n_nonzero++;		/* Count # entries printed. */
		if (flags & F_PADDR)	/* Printing address of symbol? */
			printf(aformat, slp->sl_addr);
		t += t0;	/* mjm: move here; compiler bug  !! */
		printf("%-*.*s%6.1f %11.2f %11.2f",	/* Accumulate time. */
		    nmlen,nmlen,slp->sl_name, t0 * t_tot, t0, t);
		if (count) {		/* Any calls recorded? */
		/* Get reasonable number of fractional digits to print. */
			int fdigits = fprecision(count);
			printf("%8ld%#*.*f", count, 8 + fdigits, fdigits,
			    1000.0*t0/count);
		}
		(void)putchar('\n');		/* Wrap up line. */
	}
/*	Write to mon.sum			*/
	if (flags & F_SUM) {		/* Extra info? */
		if ((fpsum = fopen(mon_sum, "w")) == NULL)
			Perror(mon_sum);
		fprintf(fpsum, MSGSTR(CALLCNT,"%5d/%d call counts used\n"), n_cc, total_symbols);
		fprintf(fpsum, MSGSTR(QSYM,"%5d/%d symbols qualified"), total_symbols, symttl);
		if (n_nonzero < total_symbols)
			fprintf(fpsum,
			    MSGSTR(ZEROCNT,", %d had zero time and zero call-counts\n"),
			    total_symbols - n_nonzero);
		else
			(void)putc('\n', fpsum);
		fprintf(fpsum, MSGSTR(FACTOR,"%#x scale factor\n"), (long)last_sf);
		if (scalechanged)
			fprintf(fpsum, MSGSTR(SF_CHANGED,
			      "warning scale factor different per region.\n"));
	}
	if (flags & F_VERBOSE) {		/* Extra info? */
		fprintf(stderr, MSGSTR(CUSED,"%5d/%d call counts used\n"), n_cc, total_symbols);
		fprintf(stderr, MSGSTR(SQ,"%5d/%d symbols qualified"), total_symbols, symttl);
		if (n_nonzero < total_symbols)
			fprintf(stderr,
			    MSGSTR(ZEROT,", %d had zero time and zero call-counts\n"),
			    total_symbols - n_nonzero);
		else
			(void)putc('\n', stderr);
		fprintf(stderr, MSGSTR(SQFAC,"%#x scale factor\n"), (long)last_sf);
		if (scalechanged)
			fprintf(stderr, MSGSTR(SF_CHANGED,
			      "warning scale factor different per region.\n"));
	}

	exit(0);
}

/*
 * NAME: fsize
 *                                                                    
 * FUNCTION: 
 * 	Return size of file associated with file descriptor fd.
 *                                                                    
 * RETURNS: off_t
 */  

static PROC off_t
fsize(fd)
{
	struct stat sbuf;

	if (fstat(fd, &sbuf) < 0)		/* Status of open file. */
		Perror("stat");
	return (sbuf.st_size);			/* This is a long. */
}

/*
 * NAME: Malloc
 *                                                                    
 * FUNCTION: 
 * 	Error-checking memory allocator. 
 *	Guarentees good return (else none at all).
 *                                                                    
 * RETURNS: 
 *	pointer to allocated memory
 */  

static PROC char *
Malloc(item_count, item_size)
{
	register char *p;

	if ((p = calloc((size_t)item_count, (size_t)item_size)) == NULL) {
		Perror("Out of space");
		exit (-1);
	}

	DEBUG_IT(fprintf (stderr,
		"Callocing: cnt: %x size: %x total: %x  addr: %x\n",
		item_count,item_size,item_count*item_size,p);)
	return (p);
}

/*
 * NAME: fprecision
 *                                                                    
 * FUNCTION: 
 *	Given the quotiant Q = N/D, where entier(N) == N and D > 0, an
 *	approximation of the "best" number of fractional digits to use
 *	in printing Q is f = entier(log10(D)), which is crudely produced
 *	by the following routine.
 */  

static PROC int
fprecision(count)
long count;
{
	return (count < 10 ? 0 : count < 100 ? 1 : count < 1000 ? 2 :
	    count < 10000 ? 3 : 4);
}

/*
 * NAME: basename
 *                                                                    
 * FUNTCTION and RETURN value: 
 *
 *	Return pointer to base name (name less path) of string s.
 *	Handles case of superfluous trailing '/'s, and unlikely
 *	case of s == "/".
 */  

static PROC char *
basename(s)
register char *s;
{
	register char *p;

	p = &s[strlen(s)];			/* End (+1) of string. */
	while (p > s && *--p == '/')		/* Trim trailing '/'s. */
		*p = '\0';
	p++;					/* New end (+1) of string. */
	while (p > s && *--p != '/');		/* Break backward on '/'. */
	if (*p == '/')		/* If found '/', point to 1st following. */
		p++;
	if (*p == '\0')
		p = "/";			/* If NULL, must be "/". (?) */
	return (p);
}

/*
 * NAME: eofon
 *                                                                    
 * FUNTCTION: 
 * 	Come here if unexpected read problem. 
 * RETURNS:
 *	exits
 */  

static PROC
eofon(iop, fn,msg)
register FILE *iop;
register char *fn;
char *msg;
{
	if (ferror(iop))		/* Real error? */
		Perror(fn);		/* Yes. */
	fprintf(stderr, MSGSTR(EARLYEND,"%s: %s: %s: Premature EOF\n"), cmdname, fn,msg);
	exit(1);
}

/*
 * NAME: Perror
 *                                                                    
 * FUNTCTION: 
 *	  Version of perror() that prints cmdname first.
 *
 * RETURNS:
 *		exits
 */  


static PROC
Perror(s)
char *s;
{				/* Print system error message & exit. */
	register int err = errno;	/* Save current errno in case */

	fprintf(stderr, "%s: ", cmdname);
	errno = err;			/* Put real error back. */
	perror(s);			/* Print message. */
	exit(1);			/* Exit w/nonzero status. */
}



/*
 *	Various comparison routines for qsort. Uses:
 *
 *	c_ccaddr	- Compare fnpc fields of cnt structs to put
 *				call counters in increasing address order.
 *	c_sladdr	- Sort slist structures on increasing address.
 *	c_time		-  "	 "	  "      " decreasing time.
 *	c_ncalls	-  "	 "	  "      " decreasing # calls.
 *	c_name		-  "	 "	  "      " increasing symbol name
 */

#define CMP2(v1,v2)	((v1) < (v2) ? -1 : (v1) == (v2) ? 0 : 1)
#define CMP1(v)		CMP2(v, 0)

static PROC
int
c_ccaddr(struct poutcnt *p1, struct poutcnt *p2)
{
	return (CMP2(p1->fnpc, p2->fnpc));
}

static PROC
int
c_sladdr(struct slist *p1, struct slist *p2)
{
	return (CMP2(p1->sl_addr, p2->sl_addr));
}

static PROC
int
c_time(register struct slist *p1, register struct slist *p2)
{
	register float dtime = p2->sl_time - p1->sl_time; /* Decreasing time. */

	return (CMP1(dtime));
}

static PROC
int
c_ncalls(register struct slist *p1, register struct slist *p2)
{
	register int diff = p2->sl_count - p1->sl_count; /* Decreasing # calls. */

	return (CMP1(diff));
}

static PROC
int
c_name(register struct slist *p1, register struct slist *p2)
{
	wchar_t *wc1, *wc2;
	size_t n1, n2;
	register int diff;

	n1 = strlen(p1->sl_name) + 1;
	n2 = strlen(p2->sl_name) + 1;
	wc1 = (wchar_t *)malloc(n1 * sizeof(wchar_t));
	wc2 = (wchar_t *)malloc(n2 * sizeof(wchar_t));
	mbstowcs(wc1, p1->sl_name, n1);
	mbstowcs(wc2, p2->sl_name, n2);
	diff = wcsncmp(p1->sl_name, p2->sl_name, N_NAME);
	free(wc1);
	free(wc2);

	return (CMP1(diff));
}


/*
 * NAME: mon_proc
 *                                                                    
 * FUNCTION: 
 *	Subroutine to process each mon.out in a series.
 *                                                                    
 * (DATA STRUCTURES:) global data structures:
 *			ccounts,
 *			pcounts,
 *			n_pc,
 *			n_cc 
 *	are produced from the mon.out files...
 *
 * RETURNS: void
 */  

static PROC
mon_proc()
{
	int	i;
	HISTCOUNTER	sample;
	long	begin_of_loadinfo;
	HISTCOUNTER *pcounts;	/* Pointer to allocated area for pcounts: PC clock hit counts */
	HISTCOUNTER **pc_array;
	struct outrng *range;
	int rng;

		/* Open monitor data file (has counts). */
	if ((mon_iop = fopen(mon_fn, "r")) == NULL)
		Perror(mon_fn);

	/* Get size of file containing profiling data. Read header part. */

	Do_READ(&head, sizeof(struct outhdr), 1, mon_iop,"Header")

	if (multi_monfiles && oldhead.nrngs != head.nrngs 
				&& oldhead.lbs != head.lbs) {
		fprintf (stderr,MSGSTR(BADMON,"mon.out %d file doesn't match\n"),multi_monfiles);
		exit(1);
	}
	oldhead = head;

	begin_of_loadinfo = ftell (mon_iop);
	if (!multi_monfiles)
		total_symbols = count_symbols (begin_of_loadinfo,mon_iop);

	LOC(mon_iop);
	fseek (mon_iop, head.lbs + sizeof (struct outhdr),0);
	LOC(mon_iop)
	range = (struct outrng *) Malloc (head.nrngs, sizeof (struct outrng));

	Do_READ(range, sizeof(struct outrng), head.nrngs, mon_iop,"ranges")

       /* really no need
	* pc_array = (HISTCOUNTER **) Malloc (head.nrngs,sizeof(HISTCOUNTER *));
	*/

	for (rng = 0; rng < head.nrngs; rng++) {
		pc_h = range[rng].hpc;
		pc_l = range[rng].lpc;

		pcounts = (HISTCOUNTER *) Malloc (range[rng].nhcnt, sizeof (HISTCOUNTER));
		/* no need
		 * pc_array[rng] = pcounts;
		 */
		n_pc = range[rng].nhcnt;
		for ( i = 0 ;i<n_pc ; i++ ) {
			Do_READ(&sample, sizeof(HISTCOUNTER), 1, mon_iop,"samples")
			pcounts[i] = sample;
#ifdef	DBG
		if(debug)
			if (pcounts[i] !=0)
				fprintf(stderr,"pcounts[%d] is %x\n",i,pcounts[i]);
#endif	DBG
		}
		charge_ticks_to_routines(pc_h,pc_l,(uint)n_pc,pcounts);
		free (pcounts);

	}

	n = fsize(fileno(mon_iop));
	n_cc = (int) ((n - ftell (mon_iop)) / sizeof (struct poutcnt));

	if (n_cc == 0) {
		fprintf (stderr,MSGSTR(NOCNTS1,"Warning: mon.out file has no call counts.  Program\n"));
		fprintf (stderr,MSGSTR(NOCNTS2,"    possibly not compiled with profiled libraries.\n"));
	}
      else {

	ccounts = (struct poutcnt *)Malloc(n_cc, sizeof(struct poutcnt));

DEBUG_IT(fprintf (stderr,"number of functions: %x\n",n_cc);)

		/* Read the call addr-count pairs. */
	LOC(mon_iop)
	Do_READ(ccounts, sizeof(struct poutcnt), n_cc, mon_iop,"call counts")
	LOC(mon_iop)

#ifdef DBG
	if (debug) 
		for (i=0;i<n_cc;i++)
			if (ccounts[i].fnpc)
			fprintf (stderr,"address: %x count: %x\n",
				ccounts[i].fnpc, ccounts[i].mcnt);
#endif DBG

	ccp = &ccounts[n_cc];	/* Point to last (+1) of call counters ... */
	do {		/* and scan backward until find highest one used. */
		if ((--ccp)->mcnt)
			break;		/* Stop when find nonzero count. */
	} while (--n_cc > 0);		/* Or all are zero. */

#ifdef DBG
DEBUG_IT(fprintf (stderr,
"ccp adr: %x size: %x\n",ccp,n_cc * sizeof(struct poutcnt));)
DEBUG_IT(for (i=0;i<n_cc;i++)
fprintf (stderr,"fnpr: %x  count: %x\n",
ccounts[i].fnpc,ccounts[i].mcnt);)
#endif DBG
	
	charge_call_counts_to_routines();
       }

}


/* from libplot.a */

static move(xi,yi){
	putc('m',stdout);
	putsi(xi);
	putsi(yi);
}

/*
 * NAME: count_symbols
 *                                                                    
 * FUNCTION: 
 * 	Given a mon.out file, we build a slist structure which has:
 *
 *		-the names of the routines in program, 
 *		-the number of seconds spent in each routine
 *		-the address of the routine.
 *
 *	Once this is built, it will be passed the call count data
 *	will be read in and charged to each routine.
 *
 *	For each object loaded, there is an entry in the load table.
 *	This file is opened up, the run-time load address is added 
 *	to each symbol, and it is appended to the slist array structure.
 *	Since the array is dynamically built, we make one pass thru just
 *	counting the symbols.  Then we malloc the space and read thru
 *	one last time, filling the structure.
 *                                                                    
 * RETURNS: 
 *	The total number of symbols returned in the slist structure.
 */  

static PROC int 
count_symbols (ptr,mon_iop)
FILE *mon_iop;
long ptr;
{
int 	finished = 0;			/* set when last load module found  */
int	tot_symbols=0;			/* count total symbols              */
char	tmpbuf[2 * MAXSTRING+1];	/* buffer to read load module names */
char    tmpname[MAXSTRING+1];           /* tmp object file load name        */
char	filename[MAXSTRING+1];		/* object file load name            */
char	member[MAXSTRING+1];		/* if load module is a archive      */
int	loc;				/* where are we in file (debugging) */
uint	next;				/* pointer to next load module      */
void	*textorg;			/* run-time load address of module  */
int 	dups=0;				/* There maybe duplicate entries    */
int	i;
int	once=0;
char *p;

	if (fseek (mon_iop,ptr,0) != 0) {
		perror ("can't seek");
		exit (-1);
	}


		/*
		 * We aren't specifically told how many modules were loaded
		 * so we must go until the next field is NULL
		 */
	while (!finished) {

	   LOC(mon_iop)

	   Do_READ (&next, sizeof(uint),1, mon_iop, "next offset read failed")

	   LOC(mon_iop)

	   Do_READ (&textorg, sizeof(void *), 1, mon_iop,"cant read textorg")

	   LOC(mon_iop)

		if (next == 0) {
			finished++;
			(void) fgets (filename, MAXSTRING, mon_iop);
			 strncpy (member,(strchr(filename,'\0')+1),MAXSTRING);
		}
		else if (next > 2 * MAXSTRING) {
			fprintf (stderr, MSGSTR(STR2BIG,"Filenames/paths too big in loader table\n"));
			exit(-1);
		}
		else {
			Do_READ (tmpbuf, (next - (uint) sizeof(uint) 
				- (uint) sizeof(void *)),
				1 ,mon_iop,"string read failed")

			getnames (filename,member,tmpbuf,MAXSTRING);

		}

		if (lflg && !once) {
			strncpy (filename, a_outname,MAXSTRING);
			once++;
		}
		else if (Lflg) {
			if ((p = strrchr(filename,'/')) != '\0') {
				strcpy (tmpname,p);
				sprintf(filename,"%s/%s",newpath,tmpname);
			}
		}
#ifdef DBG
		if (debug) {
			printf ("filename: %s  ",filename);
			printf ("member:   %s\n",member);
		}
#endif DBG
				/* NULL means not to save it */
		tot_symbols += digest_object (filename,member,NULL,NULL);

#ifdef DBG
	   if (debug)
		printf ("curr local is: %x syms: %x\n",ftell(mon_iop),tot_symbols);
#endif DBG
	}

	/* 
	 * Now we have the size of the array, we can malloc the space.
	 * We could have coded this routine fancily by making a loop or
	 * saving all the load modules, but for the sake of clear logic
	 * flow (and my hung over brain), we'll just dup the code and do
	 * it again.
	 */

	finished =0;	/* doit a second time */
	slist = slp = (struct slist *)Malloc(tot_symbols, sizeof(struct slist));
	newslist=newslp=(struct slist *)Malloc(tot_symbols,sizeof(struct slist));
	symttl = tot_symbols;	    /* save how many total symbols */
	tot_symbols = 0;	    /* now fill and count how many qualify */
	once=0;

	if (fseek (mon_iop,ptr,0) != 0) {	/* reset pointer */
		perror ("can't seek2");
		exit (-1);
	}

					/* Ah! doesn't this look familiar? */
	while (!finished) {

	   Do_READ (&next, sizeof(uint),1, mon_iop,"next offset rd failed")
	   Do_READ (&textorg, sizeof(void *),1, mon_iop, "text org RD failed")

	   if (next == 0) {
		finished++;
		(void) fgets (filename, MAXSTRING, mon_iop);
		 strncpy (member,(strchr(filename,'\0')+1),MAXSTRING);
	   }
	   else if (next > 2 * MAXSTRING) {
		fprintf(stderr, MSGSTR(STR2BIG,"Filenames/paths too big in loader table\n"));
		exit(-1);
	   }
	   else {
		Do_READ (tmpbuf, (next - (uint) sizeof(uint) 
			- (uint) sizeof(void *)),1,mon_iop,"string read failed")

		getnames (filename,member,tmpbuf,MAXSTRING);
	   }

		if (lflg && !once) {
			strncpy (filename, a_outname,MAXSTRING);
			once++;
		}
		else if (Lflg) {
			if ((p = strrchr(filename,'/')) != '\0') {
				strcpy (tmpname,p);
				sprintf(filename,"%s/%s",newpath,tmpname);
			}
		}
		
#ifdef DBG
	   if (debug) 
		printf ("filename: %s  member:%s\n",filename,member);
#endif DBG
	   
		/* 
		 * call routine again, but this time give it a place
		 * to fill symbols
		 */

	   tot_symbols += digest_object (filename,member,&slp,textorg);
	   load_modules++;

#ifdef DBG
	   if (debug) 
		fprintf (stderr,"total_sym: %d  slp: %x\n",tot_symbols,slp);
#endif DBG
	}

	LOC(mon_iop)

		/* NOTE: may have to align the mon_iop */
#ifdef DBG
	if (debug) {
		fprintf(stderr,"before sorted:\n");
		for (i=0;i<tot_symbols;i++)
		fprintf (stderr,"slp:name %s  addr: %x slcount: %x sltime:%x\n",
				slist[i].sl_name,
				slist[i].sl_addr,
				slist[i].sl_count,
				slist[i].sl_time);
		fprintf(stderr,"Done symbols:\n");
	}
#endif DBG
		/* Sort symbols by increasing address. */
	slp=slist;
	qsort((void *)slp, (size_t)tot_symbols, (size_t)sizeof(struct slist), c_sladdr);
#ifdef DBG
	if (debug) {
		fprintf(stderr,"after sorted:\n");
		for (i=0;i<tot_symbols;i++)
		fprintf (stderr,"slp:name %s  addr: %x slcount: %x sltime:%x\n",
				slist[i].sl_name, slist[i].sl_addr,
				slist[i].sl_count, slist[i].sl_time);
		fprintf(stderr,"Done symbols:\n");
	}
#endif DBG
	/*
	 *	Get rid of duplicate addresses.
	 */
	for (n = 1, i = 0; n < tot_symbols; n++) {	

		if (slist[i].sl_addr != slist[n].sl_addr)
			i++;
		else
			dups++;
		if (i < n)
			slist[i] = slist[n];
	}
	tot_symbols -=  dups;
#ifdef DBG
	if (debug) {
		fprintf(stderr,"Sorted symbols %d:\n",tot_symbols);
		for (i=0;i<tot_symbols;i++)
		fprintf (stderr,"slp:name %s  addr: %x slcount: %x sltime:%x\n",
				slist[i].sl_name, slist[i].sl_addr,
				slist[i].sl_count, slist[i].sl_time);
		fprintf(stderr,"Done symbols:\n");
	}
#endif DBG

	return (tot_symbols);
}


/*
 * NAME: digest_object
 *                                                                    
 * FUNCTION: 
 *
 *	Open up the file given (search to its the appropriate member,
 *	if it is an archive), if no storage pointer is given, just
 *	return the number of symbols.  If a storage pointer is given
 *	read each symbol and add it to the slist structure.  The text
 *	run-time load address is also add to the function address since
 *	counts and the histigram deal with runtime addresses.
 *
 *	Note: This routine uses the loader library routines which
 *	      are located in libld.a  Also to get the correct offsets,
 *	      we need to define -DAIXV3AR if it is not the default.
 *
 * RETURNS: 
 *	The number of symbols in this object is returned.
 */  


static PROC
digest_object (filename,member,ptr,textbase)
char 		*filename;
char 		*member;
struct slist	**ptr;
caddr_t		textbase;
{

  FILHDR	filehead;
  archdr	arhead;
  LDFILE	*ldptr;
  SYMENT	symbol;
  SCNHDR	sexpoiter;
  AUXENT	auxsym;
  AOUTHDR	opt_hdr;

  long i,ii;
  struct slist	*slp;
  int		filled,staticfunc;
  int		glu_code;
  char		*sym_name;
  unsigned short sec;
  struct stat stat_buf;

  char		*tptr;

        if (stat (filename,&stat_buf) != 0)
                Perror (filename);

	filled = 0;
	ldptr = NULL;
	do {
		if ((ldptr = ldopen(filename,ldptr)) != NULL) {
		
			if (ldfhread (ldptr,&filehead) == FAILURE) {
				fprintf (stderr,MSGSTR(FILEHEAD,"couldn't read fileheader\n"));
				exit (-1);
			}
#ifdef DBG
			if (debug)
				fprintf (stderr,"num syms is: %d\n",filehead.f_nsyms);
#endif DBG
			if (*member != '\0') {
				if (ldahread (ldptr,&arhead) == FAILURE) {
					fprintf (stderr,MSGSTR(NOACHIVE,"ldahread failed. Not an archive.\n"));
					exit(-1);
				}
				else {
#ifdef DBG
					if (debug)
						fprintf (stderr,"arch object is: %s\n",arhead.ar_name);
#endif DBG
					if (strcmp(member,arhead.ar_name) != 0)
						continue;
				}
			}
			if ( ldohseek (ldptr) == FAILURE) {
				fprintf(stderr,"No optional header\n");
				exit(-1);
			}
			Do_READ (&opt_hdr, sizeof(AOUTHDR), 1, IOPTR(ldptr),"No Optional Header")
			textbase -= opt_hdr.text_start;
DEBUG_IT(fprintf(stderr, "textbase: %x, text_start: %x\n", textbase,opt_hdr.text_start);)
			/* 
			 * If the object that is being process has been 
			 * stripped, then we will attribute all of the time to
			 * this object instead of individual routines inside the
			 * object.
			 * In a future enhancement we could get the symbols of
			 * the external routines from the loader table instead
			 * of the symbol table.
			 */
			if (ldtbseek (ldptr) == FAILURE) {
				if (ptr == NULL) 	/* first passed, */
					return (1); 	/* only count    */
				fprintf (stderr,MSGSTR(STRIPPED,"%s(%s) is stripped.\n"),filename,member);
				slp = *ptr;
				slp->sl_addr = textbase;
				if ((tptr = malloc(strlen(member)+strlen(basename(filename))+strlen("{stripped object}")+5)) != NULL)
					sprintf (tptr,"%s(%s) {stripped object}",basename(filename),member);
				else {
					perror ("malloc");
					exit (-1);
				}
				strncpy (slp->sl_name,tptr,N_NAME);
				slp->sl_time = 0.0;
				slp++;
				*ptr = slp;
				ldaclose(ldptr);
				return(1);	/* only one symbol used */
			}

			if (ptr == NULL) {    /* Just interested in the count */
				ldaclose (ldptr);
				return (filehead.f_nsyms);
			}
			for (sec = (unsigned short) 1; sec <= filehead.f_nscns;sec++) {
				if (ldshread (ldptr,sec,&sexpoiter) == FAILURE) {
					fprintf (stderr,MSGSTR(SCN,"Ldnshread failed\n"));
					exit (-1);
				}
				if (strcoll(_TEXT,sexpoiter.s_name) == 0) {
#ifdef DBG
					if (debug)
						printf ("text section is: %x name: %s\n",sec,sexpoiter.s_name);
#endif DBG
					text_section_number = sec;
					break;
				}
			}

			slp = *ptr;

			for (i= 0; i<filehead.f_nsyms; i++) {
				if ( ldtbread (ldptr,i,&symbol) == FAILURE) {
					fprintf(stderr, MSGSTR(NOSYM,"Couldn't read symbol: %d\n"),i);
					exit(-1);
				}

				symbol.n_value += (unsigned long) textbase;

				sym_name = dem_conv(ldgetname(ldptr, &symbol));
				if (sym_name == INVAL)
					sym_name = ldgetname(ldptr, &symbol);
#ifdef DBG
				if (debug) {
					fprintf (stderr,"Symbol: %d is: %s\t",i,sym_name);
					fprintf (stderr," val: %x  scnum: %x  type: %x  class: %x numaux: %x\n",
						(int) symbol.n_value,
						(int) symbol.n_scnum,
						(int) symbol.n_type,
						(int) symbol.n_sclass,
						(int) symbol.n_numaux);
					}
#endif DBG
				if (isvalidsymbol (&symbol, sym_name)) {
				  	staticfunc=0;
				  	if(!gflag && symbol.n_sclass == C_HIDEXT)
						staticfunc = -1;
					glu_code = 0;
					for (ii=1;ii <= symbol.n_numaux;ii++) {
						if ( ldtbread (ldptr,i+ii,&auxsym) == FAILURE) {
							fprintf (stderr,"couldn't read auxillary entry\n");
							exit(-1);
						}
#ifdef DBG
					if (debug) 
						fprintf (stderr,"Aux Symbol: %d is: %s smclas: %x",i+ii,ldgetname(ldptr,&auxsym), (int) auxsym.x_csect.x_smclas);
#endif DBG
						if (auxsym.x_csect.x_smclas == XMC_GL)
							glu_code++;
						if( (ii == symbol.n_numaux) && 
							(auxsym.x_csect.x_smtyp & 0x07) != XTY_LD )
							staticfunc=0;
					}
 					(void)mbsncpy(slp->sl_name, sym_name, N_NAME);
					slp->sl_name[N_NAME] = '\0';  

					if (glu_code)
						strcat (slp->sl_name,".GL");


					/*  fill in the slist struct */
					slp->sl_addr = (char *)symbol.n_value;

					   /* set other slist fields to zero. */

					slp->sl_time = 0.0;
					slp->sl_count = staticfunc;
					DEBUG_IT(fprintf(stderr,
					"%-10.10s: %x\n", slp->sl_name, slp->sl_addr);)
					slp++;
					filled++;
				}

				i += (long) symbol.n_numaux;
			}
			*ptr = slp;
			ldaclose(ldptr);
			return (filled);
		}
	} while (ldclose(ldptr) == FAILURE);
}


/*
 * NAME: charge_ticks_to_routines
 *                                                                    
 * FUNCTION: 
 *
 *	Time is charged to routines in the slist structure.  Ticks
 *	are found in a histigram buffer pointed to by "pcounts".  The
 *	nearest routine is charged for each tic in the buffer.
 *
 * RETURNS: 
 *	none
 */  


PROC
static void
charge_ticks_to_routines(pc_h,pc_l,n_pc,pcounts)
char	*pc_l;		
char	*pc_h;	
uint	n_pc;
HISTCOUNTER *pcounts;	/* Pointer to allocated area for pcounts: PC clock hit counts */
{
	uint sf;	/* Scale for index into pcounts:
					i(pc) = ((pc - pc_l) * sf)/sf. */

	uint s_inv;	/* Inverse: i_inv(i) =
					{pc00, pc00+1, ... pc00+s_inv-1}. */

	unsigned pc_m;	/* Range of PCs profiled: pc_m = pc_h - pc_l */
	register HISTCOUNTER *pcp;	/* For scanning pcounts. */
	float t, t0;
	int i;

	/*
	Having gotten preliminaries out of the way, get down to business.
	The range pc_m of addresses over which profiling was done is
	computed from the low (pc_l) and high (pc_h) addresses, gotten
	from the MON_OUT header.  From this and the number of clock
	tick counters, n_pc, is computed the so-called "scale", sf, used
	in the mapping of addresses to indices, as follows:

			(pc - pc_l) * sf
		i(pc) = ----------------
			 0200000

	Also, the N-to-one value, s_inv, such that

		i(pc_l + K * s_inv + d) = K, for 0 <= d < s_inv

	Following this, the symbol table is scanned, and those symbols
	that qualify are counted.  These  are T-type symbols, excluding
	local (nonglobal) unless the "-g" option was given. Having thus
	determined the space requirements, space for symbols/times etc.
	is allocated, and the symbol table re-read, this time keeping
	qualified symbols.
	*/


		/* Range of profiled addresses. */

	pc_m = (unsigned int) pc_h - (unsigned int) pc_l;	

DEBUG_IT(fprintf(stderr,
"low pc = %x, high pc = %x, range = %x = %u\n\
call counts: %u, %u used; pc counters: %u\n",
pc_l, pc_h, pc_m, pc_m, total_symbols, n_cc, n_pc);)

	sf = (float) ((float)BIAS *(float) n_pc)/(float)pc_m;  /* The "scale" used to map PCs to indices. */

	if (last_sf != 999 && sf != last_sf)
		scalechanged++;
	else
		last_sf = sf;
	s_inv = pc_m/n_pc;	  /* Range of PCs mapped into one index.     */

DEBUG_IT(fprintf(stderr, "sf = %x, s_inv = %ld\n", (long)sf, s_inv);)

	/*
	The distribution of times to addresses is done on a proportional
	basis as follows: The t counts in pcounts[i] correspond to clock
	ticks for values of pc in the range pc, pc+1, ..., pc+s_inv-1
	(odd addresses excluded for PDP11s).  Without more detailed
	information, it must be assumed that there is no greater probability
	of the clock ticking for any particular pc in this range than for
	any other.  Thus the t counts are considered to be equally distributed
	over the addresses in the range, and that the time for any given
	address in the range is pcounts[i]/s_inv.

	The values of the symbols that qualify, bounded below and above
	by pc_l and pc_h, respectively, partition the profiling range into
	regions to which are assigned the total times associated with the
	addresses they contain in the following way:

	The sum of all pcounts[i] for which the corresponding addresses are
	wholly within the partition are charged to the partition (the
	subroutine whose address is the lower bound of the partition).

	If the range of addresses corresponding to a given t = pcounts[i]
	lies astraddle the boundary of a partition, e.g., for some k such
	that 0 < k < s_inv-1, the addresses pc, pc+1, ..., pc+k-1 are in
	the lower partition, and the addresses pc+k, pc+k+1, ..., pc+s_inv-1
	are in the next partition, then k*pcounts[i]/s_inv time is charged
	to the lower partition, and (s_inv-k) * pcounts[i]/s_inv time to the
	upper.  It is conceivable, in cases of large granularity or small
	subroutines, for a range corresponding to a given pcounts[i] to
	overlap three regions, completely containing the (small) middle one.
	The algorithm is adjusted appropriately in this case.
	*/


	pcp = pcounts;				/* Reset to base. */
  	slp = slist;				/* Ditto. */
	t0 = 0.0;				/* Time accumulator. */
	for (n = 0; n < total_symbols; n++) {		/* Loop on symbols. */
			/* Start addr of region, low addr of overlap. */
		char *pc0, *pc00;
			/* Start addr of next region, low addr of overlap. */
		char *pc1, *pc10;
		 /* First index into pcounts for this region and next region. */
		register int i0, i1;
		long ticks;

			/* Address of symbol (subroutine). */
		pc0 = slp[n].sl_addr;

			/* 
			 * Skipped this symbol if not within the range...
			 * i.e. it's from another object load module.
			 * Or a static function when gflag not specified.
			 */

		if ((pc0 < pc_l) || (pc0 > pc_h) || slp[n].sl_count == -1)
			continue;

			/* Address of next symbol, if any or top of profile
								range, if not */
		if(n < total_symbols - 1) {
			int k=n+1;
			while( k<total_symbols && slp[k].sl_count == -1)
				k++;
			pc1=(k == total_symbols) ? pc_h : slp[k].sl_addr;
		} else {
			pc1=pc_h;
		}

			/* Lower bound of indices into pcounts for this range */

		i0 = ((pc0 - pc_l) * (float)sf)/(float)BIAS;
			/* Upper bound (least or least + 1) of indices. */
		i1 = ((pc1 - pc_l) *(float) sf)/(float)BIAS;

		if ((uint) i1 >= (uint) n_pc)		/* If past top, */
			i1 = n_pc - 1;			/*      adjust. */

			/* Lowest addr for which count maps to pcounts[i0]; */

		pc00 = pc_l + (uint) ((((float)BIAS * i0)+1)/(float)sf);

			/* Lowest addr for which count maps to pcounts[i1]. */

		pc10 = pc_l + (uint)((((float)BIAS * i1)+1)/(float)sf);

DEBUG_IT(fprintf(stderr,
"%-8.8s\ti0 = %4d, pc00 = %#6x, pc0 = %#6x\n\
\t\ti1 = %4d, pc10 = %#6x, pc1 = %#6x\n\t\t",
slp[n].sl_name, i0, pc00, pc0, i1, pc10, pc1);)
		t = 0;			/* Init time for this symbol. */
		if (i0 == i1) {

			/* 
			 * Counter overlaps two areas? (unlikely 
			 * unless large granularity). 
			 */

			ticks = pcp[i0];	/* # Times (clock ticks). */

			    /* Time less that which overlaps adjacent areas */

			t += (pc1 - pc0) * SEC(ticks)/s_inv;

DEBUG_IT(fprintf(stderr, "%ld/%ld", (pc1 - pc0) * ticks, s_inv);)
		} else {
				/* Overlap with previous region? */
			if (pc00 < pc0) {
				ticks = pcp[i0];

				/* Get time of overlapping area and subtract
						proportion for lower region. */
				t += SEC(pcp[i0]) - (pc0 - pc00) * SEC(ticks)/s_inv;

				/* Do not count this time when summing times
						wholly within the region. */
				i0++;
DEBUG_IT(fprintf(stderr, "%ld/%ld + ", (pc0 - pc00) * ticks, s_inv);)
			}

			/* Init sum of counts for PCs not shared w/other
								routines. */
			ticks = 0;

			/* Stop at first count that overlaps following
								routine. */
			for (i = i0; i < i1; i++)
				ticks += pcp[i];

			t += SEC(ticks);  /* Convert to secs & add to total. */
DEBUG_IT(fprintf(stderr, "%ld", ticks);)
			/* Some overlap with low addresses of next routine? */
			if (pc10 > pc1) {
					/* Yes. Get total count ... */
				ticks = pcp[i1];

				/* and accumulate proportion for addresses in
							range of this routine */
				t += (pc1 - pc10) * SEC(ticks)/s_inv;
DEBUG_IT(fprintf(stderr, " + %ld/%ld", (pc1 - pc10) * ticks, s_inv);)
			}
		}		/* End if (i0 == i1) ... else ... */

		slp[n].sl_time += t;	/* Store time for this routine. */
		t0 += t;		/* Accumulate total time. */
DEBUG_IT(fprintf(stderr, " ticks = %.2f msec\n", t);)
	}	/* End for (n = 0; n < total_symbols; n++) */

	/* Final pass to total up time. */

	for (n = n_pc; --n >= 0; )	{
		if (SEC(*pcp) > maxtime)
			maxtime = SEC(*pcp);
		t_tot += SEC(*pcp++);
	}
}


/*
 * NAME: getnames
 *                                                                    
 * FUNCTION: 
 *
 *	Fill in the filename and member name found in the buffer
 *	pointed to by "tmpbuf".  These are two null terminated strings.
 *
 * RETURNS: 
 *	none
 */  


PROC
static void
getnames (filename,member,tmpbuf,maxlen)
char *filename,*member,*tmpbuf;
int maxlen;
{
	char *p1=filename,*p2=tmpbuf;
	int max = maxlen;
	do {
		*p1++ = *p2;
		if (!max--) {
			fprintf (stderr,MSGSTR(STR2LONG,"filename string too long\n"));
			exit(-1);
		}
	} while (*p2++);
	max = maxlen;
	p1 = member;
	do {
		*p1++ = *p2;
		if (!max--) {
			fprintf (stderr,MSGSTR(MEMBER,"member string too long\n"));
			exit(-1);
		}
	} while (*p2++);
}


/*
 * NAME: isvalidsymbol
 *                                                                    
 * FUNCTION: 
 *
 * RETURNS: 
 *	Returnes true if the given symbol is within the appropriate range
 *	and is a text symbol.
 */  

static PROC
isvalidsymbol(nl, name)
  SYMENT	*nl;
  char		*name;
{
	if (nl->n_scnum != text_section_number)
		return 0;
	if (name == NULL)
		return 0;
	if (*name == '\0')
		return 0;
	switch(nl->n_sclass) {
		case C_EXT:
		case C_HIDDEN:
		case C_HIDEXT:
		case C_STAT:
			return 1;
	}
	return 0;
}


/*
 * NAME: charge_call_counts_to_routines
 *                                                                    
 * FUNCTION: 
 *
 *	Match call counts with symbols.  To do this, it
 *	helps to first sort both the symbols and the call address/count
 *	pairs by ascending address, since they are generally not, to
 *	begin with.  The addresses associated with the counts are not,
 *	of course, the subroutine addresses associated with the symbols,
 *	but some address slightly past these. Therefore a given count
 *	address (in the fnpc field) is matched with the closest symbol
 *	address (sl_addr) that is:
 *
 *		(1) less than the fnpc value but,
 *		(2) not more than CCADIFF bytes less than it.
 *			NOTE: for the R2 we don't use this.
 *
 *	The value of CCADIFF is roughly the size of the code between
 *	the subroutine entry and that following the call to the mcount
 *	routine.  In other words, unreasonable matchups are avoided.
 *	Situations such as this could arise when static procedures are
 *	counted but the "-g" option was not given to this program,
 *	causing the symbol to fail to qualify.  Without this limitation,
 *	unmatched counts could be erroneously charged.
 *
 * RETURN: 
 *	None, global slist structure is updated.
 */

PROC static void
charge_call_counts_to_routines()

{
struct poutcnt *ccp;	/* For scanning ccounts. */
struct slist *slp;	/* For scanning slist */
int i;

	ccp = ccounts;			/* Point to first call counter. */
	slp = slist;			/*   "		"   "   symbol. */

		/* Sort call counters and ... */
	qsort((void *)ccp, (size_t)n_cc, (size_t)sizeof(struct poutcnt), c_ccaddr);
	vn_cc = n_cc;			/* save this for verbose option */
#ifdef DBG
	if (debug)
		for (i=0;i<n_cc;i++)
			fprintf (stderr,"fnpr: %x  count: %x\n",
					ccounts[i].fnpc,ccounts[i].mcnt);
#endif DBG

		/* Loop to match up call counts & symbols. */

	for (n = total_symbols; n > 0 && vn_cc > 0; ) {

DEBUG_IT(fprintf(stderr,
"slp->addr: %x	ccp->fnpc: %x\n", slp->sl_addr,ccp->fnpc);)

		if (slp->sl_addr > ccp->fnpc ) {

			if(n == total_symbols){  /* can't happen on the first*/
				fprintf(stderr,"warning: symbol for call count address:%x not found\n",ccp->fnpc);
				ccp++;
				continue;
			}
			slp--;
DEBUG_IT(fprintf(stderr,
"Routine %-*.*s @ %#8x+%-2d matches count address %#8x\n",
nmlen,nmlen,slp->sl_name, slp->sl_addr, ccp->fnpc-slp->sl_addr, ccp->fnpc);)

			if(slp->sl_count == -1) {
				ccp++;
				vn_cc--;
				slp++;
				continue;
			}
			slp->sl_count += ccp->mcnt;	/* Copy count. */
			++ccp;
			--vn_cc;
			if (slp->sl_addr > ccp->fnpc && vn_cc && n) {
/*  This should never happen */
				fprintf(stderr,"warning: one symbol matched to two function call counters\n");
				fprintf(stderr,"symbol: %x :%s  counter: %x counter: %x\n", slp->sl_addr,slp->sl_name, --ccp++->fnpc,ccp->fnpc); 
				fprintf(stderr,"call count number: %x \n",vn_cc); 
			}
			++slp;
		} else {
			++slp;
			--n;
		}
	}
	if (vn_cc == 1) {
		slp--;
		slp->sl_count += ccp->mcnt;	/* Copy count. */
		vn_cc--;
	}
	if (vn_cc)
		fprintf(stderr,"warning: %x routines not found for reported call counts\n",vn_cc);

}
/*
 * NAME: dem_conv
 *
 * FUNTCTION:
 *      Conversion routine for demangled name
 * RETURNS:
 *      demangled name or invalid (-1) 
 */
static char *
dem_conv( char *str )
{
    static char sym_save_size = 0;
    static char *sym_save_area = NULL;
    char	**rest, *sym_name;
    struct	Name	*demname, *demangle();
    int		needDot, len;

    if (*str == '.') {
	needDot = 1;
	str++;
    } else
	needDot = 0;

    demname = demangle (str, &rest, RegularNames);

    /* did we do anything? */
    if (demname == NULL)
	return INVAL;

    /* get the real name */
    sym_name = text (demname);

    /* ensure we have enough room to play with */
    len = strlen(sym_name) + needDot + 1;
    if (len >= sym_save_size) {
	if (sym_save_area)
	    free (sym_save_area);
	/* get some extra, just for fun */
	len *= 2;
	sym_save_area = malloc (len);
	if (sym_save_area)
	    sym_save_size = len;
	else {
	    perror ("malloc");
	    exit (-1);
	}
    }

    /* save the name (with leading dot if necessary) */
    if (needDot) {
	sym_save_area[0] = '.';
	strcpy (&sym_save_area[1], sym_name);
    } else
	strcpy (sym_save_area, sym_name);

    /* return the storage */
#ifndef F_PLOT
    /* demangle erase conflicts with plot erase! */
    erase (demname);
#endif
    return sym_save_area;
}
