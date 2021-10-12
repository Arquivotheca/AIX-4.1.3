static char sccsid[] = "@(#)98        1.34  src/bos/usr/bin/trcrpt/prevent.c, cmdtrace, bos41J, 9513A_all 2/21/95 10:02:41";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: prevent_init, prevent, pr_iputs, pr_delim, pr_error, pr_hdr
 *            pr_exec, pr_pid, pr_svc, pr_file, level_eval
 *            lflush, pass2excp
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */
/*
 * PROGRAM NAME:   trcrpt
 * FILE NAME:      prevent.c
 * ORIGIN: IBM 83
 *
 * Interpret the parsed template tree for a (binary) event and print it out.
 *                                                                    
 * NAME:     prevent
 * FUNCTION: print out the hookid, timestamp, and indentation.
 * INPUTS:   tdp0   Head of parsed template tree for the event.
 * RETURNS:  none
 *
 * prevent() is called by pass2().
 * The template has already been parsed and its link-listed parse tree is
 * in memory headed by 'tdp0'. The event has been read into the global buffer
 * 'Eventbuf[]' by getevent.
 *
 * There are a few formatting functions that prevent() performs before
 * it calls the recursive routine rprevent to print the tree.
 * 1. Set the default indentation count 'Nindent_appl' to NINDENT plus
 *    corrections for
 *      Opts.opt_idflg   (print 3 digit hook id)
 *      Opts.opt_execflg (print exec pathnames)
 *      Opts.opt_svcflg  (print svc pathnames)
 * 2. If the HEADER hook id is detected, init the starting time 'Aseconds0'
 * 3. The LEVEL= indentation is in the first entry in the tdp list. Extract
 *    it and set Nindent to its value. The routine codetoindent() converts
 *    the L=INT,SVC etc code into a value.
 * 4. Set flags and indices to 0. (This is an initialization step)
 * 5. Zero the "registers". (macros)
 * 6. Call setjmp in case $BREAK or $STOP is detected.
 * 7. Print the column heading if it has not already been printed.
 * 8. Advance the timestamp if a timestamped event.
 * 9. Call preventinfo() to print the Hookid, datestring, exec pathname,
 *    timestamp, and correct indentation.
 * 10. Then call rprevent to print the event itself.
 *
 * NAME:     rprevent
 * FUNCTION: recursively descend the parse tree for this hook.
 * INPUTS:   tdp0   starting branch of tree
 *           level  for "runaway" detection
 * RETURNS:  none
 *
 * The routine acts according to the td_type of the current element: (td union)
 * ILEVEL:  The element is a formatd and contains a new LEVEL code.
 *          Set Nindent
 * IMACDEF: The element is of the form:
 *               {{ $rrr = expr }}     which is represented by
 *               mp->m_name, mp->m_expr
 *          Note that m_name is only an index into the Registers[] table.
 *          Special registers handled here are:
 *               RREG_SVC          set "svc" of current process to value
 *               RREG_SYMBOL_VALUE set Symbol_value to Fnum
 *               RREG_SYMBOL_RANGE set Symbol_range to Fnum
 *               RREG_EXECPATH     by call to execeval to remember pathname
 *               RREG_PID          by setting Pid (current process id)
 *               RREG_BASEPOINTER  by setting Baseindex 
 *               RREG_DATAPOINTER  by setting Byteindex 
 * IFORMAT: The element is a format code. Call sprformat to expand.
 *          It a printing code, insert a delimiter.
 * ILOOP:   The element is the start of a LOOP.
 *          Call sprformat to get the count.
 *          If the next element is just a FORMAT, this is a simple loop:
 *             lop, printing according to the format code in f_desc.
 *             Note that A0,X0, etc. are like A1, X1, etc. but with no spaces.
 *          Otherwise, call rprevent() with the start of the new decriptor.
 * ISWITCH: Get the match value. If numeric, it will have an s_value.
 *          Otherwise, use s_string.
 *          Scan the switch branch headed by s_case.
 *          If a match and a simple string, print it out.
 *          If a match and a descriptor, call rprevent to print out.
 *          Otherwise, continue the linear search.
 * ISTRING: Print out the string.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/trchkid.h>
#include <sys/systemcfg.h>
#include <ctype.h>		/* 'S' and 'A' format codes */
#include <setjmp.h>
#include <time.h>
#include "rpt.h"
#include "td.h"
#include "parse.h"

extern FILE *Rptfp;						/* output file */
extern Condflg;
extern Logtraceid_real;

extern char *realloc();
extern char *basename();				/* in raslib.a */
extern char *currfile_lookup();			/* current filename */
extern char *exec_lookup();			/* pid to exec_pathname */
extern char *rptsym_nmlookup();		/* symbol value to symbol name */
extern char *settid();

#define OUT(c)       putc(c,Rptfp)
#define ISDELIM(c)   ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '=')
#define ISBIT(buf,n) ( buf[(n)/8] & (1 << (7 - (n) % 8)) )
#define IPUTS(s)     pr_iputs(0,s)		/* default justification */
#define IPUTS_LJ(s)  pr_iputs(1,s)		/* left    justification */
#define NSECLT(a,an,b,bn) ((a) < (b) || (a) == (b) && (an) < (bn))
#define NSECGT(a,an,b,bn) ((a) > (b) || (a) == (b) && (an) > (bn))

#define NINDENT   27			/* default left column for template */
#define TABSIZE   8				/* tab spacing */

#define MILLION     1000000		/* convert nanosecond timestamp */
#define BILLION     1000000000		/* to microsecs/millisecs */

extern Nregisters[];			/* number of registers allocated */
int    Pid;				/* current pid */
int    Intr_depth;
int    Nodelimflg;
int    Baseindex;	/* start of data within Eventbuf[]. Usually 0 */
int    Byteindex;	/* current byte index into Eventbuf[] */
int    Bitindex;	/* current bit  index into Eventbuf[] */
int    lastc;		/* last character written, to know when to output ' ' */
int    pr_lineno0;						/* pr_lineno at start of event */
int    pr_lineno;						/* Current line */
int    Aseconds0;						/* time() value on trcon */
int    Aseconds;						/* Absolute seconds */
int    Ananoseconds;
static Prev_ananoseconds;
static Prev_aseconds;
int    Printflg;						/* if off, output suppressed */

static RTflg = 0;
static Prevtimestamp;			/* previous nanosecond timestamp */
static pr_exec_col;			/* column of execname to overwrite */
static pr_pid_col;			/* column of pidnum to overwrite */
static pr_tid_col;			/* column of tidnum to overwrite */
static pr_cpuid_col;                   /* column of cpuid to overwrite */
static pr_svc_col;			/* column of SVC to overwrite */
static pr_file_col;			/* column of currfile to overwrite */
jmp_buf pr_jmpbuf;			/* setjmp/longjmp on $ERROR */

/*
 * event at a time buffering
 */
#define LINEINCR 1024					/* grow in 1024 increments */
static char *linebuf;					/* event character buffer */
static char *linelim;					/* end of linebuf */
static char *linep;						/* current pointer within linebuf */

static Col;			/* current column, to know where to put tab */
static Nindent;		/* start of column, controlled by L=XXX */
static Nindent_appl;/* start of APPL (first) column */
static char *Colheader;



#ifdef TRCRPT

/*
 * Generate the Colheader and Nindent_appl indentation base.
 * Note that the Colheader must correspond to the widths of the
 *   various data in preventinfo(), such as the timestamp.
 * Initialize the linebuf to a starting size.
 */
prevent_init()
{
	char *cp;
	extern char *getenv();

	if((cp = getenv("_JRT")) && streq(cp,"_JRT"))
		RTflg = 1;
	if(RTflg)
		Debug("RT\n");
	Colheader = jalloc(200);
	if(Opts.opt_idflg)
		hdr_append("ID  ");
	if(Opts.opt_execflg)
		pr_exec_col = hdr_append("PROCESS NAME   ");
	if(Opts.opt_cpuidflg)
                pr_cpuid_col = hdr_append("CPU ");
	if(Opts.opt_pidflg)
		pr_pid_col = hdr_append("PID      ");
	if(Opts.opt_tidflg)
		pr_tid_col = hdr_append("TID      ");
	if(Opts.opt_execflg || Opts.opt_pidflg)
		hdr_append("I ");
	if(Opts.opt_svcflg)
		pr_svc_col = hdr_append("SYSTEM CALL ");
	if(Opts.opt_fileflg)
		pr_file_col = hdr_append("FILENAME    ");
	switch(Opts.opt_timestamp) {
	case 0: hdr_append("   ELAPSED_SEC     DELTA_MSEC   "); break;
	case 1: hdr_append("    ELAPSED   ");                break;
	case 2: hdr_append("   ELAPSED_SEC     DELTA_USEC   "); break;
	}
	Nindent_appl = hdr_append("");
	if(Opts.opt_2lineflg) {
		hdr_append("\n");
		Nindent_appl = 0;
	}
	hdr_append("APPL    SYSCALL KERNEL  INTERRUPT\n\n");
	linebuf = jalloc(LINEINCR);
	linelim = linebuf + LINEINCR;
	linep   = linebuf;
	pr_lineno  = 1;
	setpid();			/* initialize exec.c */
	Debug("Nindent_appl=%d Colheader='%s'\n",Nindent_appl,Colheader);
}

/*
 * Used by prevent_init() to generate the Colheader;
 */
static hdr_append(str)
char *str;
{
	static col;
	int colsave;
	int len;

	colsave = col;
	len = strlen(str);
	strcat(Colheader,str);
	col += len;
	return(colsave);
}

/* 
 * when Threadflg flag is set the current structure u is searched
 * if the structure u does not exist, so we creat it with pid=-1
 */
	static int initconver=0;
prevent(tdp)
union td *tdp;
{
	static hdrflg;

	Printflg = !Condflg || ISHOOKID(Condhookids,Logtraceid_real);

#define SUBHKID_TBL	604
/*
 *	For POWER system, the time is kept in second and nanosection.
 *	But for POWER PC, the time is kept in tic, hence conversion
 *	is required to convert tic to nanosecond.
 *
 *	see main.c for the trace command for the format of this hook.
 */

	if(!initconver) {
		if((Logtraceid == HKWDTOHKID(HKWD_TRACE_UTIL)) && 
		(*(short *)&Eventbuf[2] == SUBHKID_TBL) &&
		(*(int *)&Eventbuf[12] == RTC_POWER_PC)) {
			int xint,xfact;
			initconver++;
			/* Get conversion multiplier (xint) and divisor (xfact)
			 * from the buffer.
			 */
			xint = *(int *)&Eventbuf[4];
			xfact = *(int *)&Eventbuf[8];
			/* Calculate conversion factor (double) */
			cnvtfact = (double)xint/(double)xfact;
			time_wrap = TIMEBASE_WRAP; /* see rpt.h */
		}
	}

	if(Logtraceid == HKWDTOHKID(HKWD_TRACE_HEADER)) {	/* set Aseconds */
		Aseconds0 = *(int *)&Eventbuf[8];		/* second full dataword */
		if(Printflg)
			Printflg = 2;		/* overrid pidlist_chk() */
	}

	Debug("prevent(%s) Printflg=%d\n",hexstr(Logtraceid),Printflg);
	if(ISTIMESTAMPED(Hookword)) {		/* update Aseconds */
		initconver++;
		settimestamp();
	}
	if(Opts.opt_histflg) {
		if(timechk() < 0) {
			histflush();
			genexit(0);
		}
		hist(Hookword);
		return;
	}
	if(rawflag) {
		if(timechk() < 0)
			genexit(0);
		prraw(Eventbuf,Eventsize);
		return;
	}
	if(tdp == 0)
		return;
	Nindent = codetoindent(((struct formatd *)tdp)->f_fld1); /* L=XXX */
	if(Nindent < 0) {			/* L=NOPRINT means don't print */
		Printflg = 0;			/* turn off printing */
		Nindent = 0;			/* Nindent < 0 problem for indent() */
	}
	Debug("Nindent=%d Nindent_appl=%d\n",Nindent,Nindent_appl);
	lastc         = '\n';		/* last character is a newline */
	Baseindex     = 0;			/* clear Baseindex */
	Byteindex     = 0;			/* clear Byteindex */
	Bitindex      = 0;			/* clear Bitindex */
	pr_lineno0    = pr_lineno;	/* lineno at start of event */
	Nregisters[0] = 0;			/* reset R[] register array */
	linep         = linebuf;	/* reset line buffer */
	Col           = 0;			/* reset current column */
	Nodelimflg    = 0;
	/*
	 * Note: The pr_jmpbuf contents are in reality static, so that
	 *       it should be possible to have an initialization step
	 *       to the the setjmp once, so that it would not need to
	 *       be done for each event.
	 */
	switch(setjmp(pr_jmpbuf)) {	/* setup for "exception" control */
	case 3:						/* $STOP */
		lflush();
		genexit(1);
	case 2:						/* $ERROR */
	case 1:						/* $BREAK */
		IPUTS("\n");
		lflush();
	case 4:						/* $SKIP */
		return;
	}
	/*
	 * Print the header if not already printed.
	 */
	if(!hdrflg && !nohdrflag && (Hookword & HKID_MASK) != HKWD_TRACE_HEADER && (Hookword & HKID_MASK) != HKWD_TRACE_UTIL ) {
		hdrflg++;
		pr_colheader();
	}
	if(nohdrflag && (Logtraceid == HKWDTOHKID(HKWD_TRACE_HEADER)))
		return;
	if(timechk() < 0)
		genexit(0);
	Byteindex += 2;					/* skip hook id and type */
	if(HKWDTOTYPE(Hookword) == HKWDTOTYPE(HKTY_V) ||
	   HKWDTOTYPE(Hookword) == HKWDTOTYPE(HKTY_VT))
		Byteindex += 2;		/* skip hook data = event length */
	if (Threadflg) {
		settid(Tid,-1);		/* search the current structure u */
		Pid = pid_lookup();
		Cpuid = cpuid_lookup();
		Pri = pri_lookup();
	}
	ipreventinfo();			/* print id, timestamp, indent, etc */
	if(Opts.opt_2lineflg)
		IPUTS("\n");
	rprevent(tdp->td_next,0);		/* print template */
	IPUTS("\n");				/* terminate with newline */
	lflush();
}

static ipreventinfo()
{
	int n;
	struct tm *tm;
	int timestampflg;
	int rnanoseconds;
	int rseconds;                       /* variable for delta seconds */
	char buf[512];

	if(!Printflg || Nindent == 0)
		return;
	if(Opts.opt_idflg) {
		sprintf(buf,"%03X ",Logtraceid_real);
		IPUTS_LJ(buf);
	}
	Execname_ref = exec_lookup();
	if(Opts.opt_execflg) {
		sprintf(buf,"%-14.14s ",Execname_ref);
		IPUTS_LJ(buf);
	}
	if(Opts.opt_cpuidflg) {
                sprintf(buf,"%-2d  ",Cpuid);
                IPUTS_LJ(buf);
        }
	if(Opts.opt_pidflg) {
		sprintf(buf,"%-8d ",Pid);
		IPUTS_LJ(buf);
	}
	if(Opts.opt_tidflg) {
		sprintf(buf,"%-8d ",Tid);
		IPUTS_LJ(buf);
	}
	if(Opts.opt_execflg || Opts.opt_pidflg) {
		if(Intr_depth) {
			sprintf(buf,"%d ",Intr_depth);
			IPUTS_LJ(buf);
		} else {
			IPUTS_LJ("  ");
		}
	}
	if(Opts.opt_svcflg) {
		char buf2[32];

		svc_lookup_buf(buf2);
		sprintf(buf,"%-12.12s",buf2);
		IPUTS_LJ(buf);
	}
	if(Opts.opt_fileflg) {
		char *cp;

		if((cp = currfile_lookup()) == 0 || STREQ(cp,"-"))
			cp = "";
		else
			cp = basename(cp);
		sprintf(buf,"%-14.14s",cp);
		IPUTS_LJ(buf);
	}
	timestampflg = ISTIMESTAMPED(Hookword);
	rseconds = Aseconds - Prev_aseconds;
	rnanoseconds = Ananoseconds - Prev_ananoseconds;
	if(rnanoseconds < 0){
		rnanoseconds += BILLION;
		rseconds--;
	}
	Debug("Anano=%d Prev_nano=%d rnano=%d\n",
		Ananoseconds,Prev_ananoseconds,rnanoseconds);
	switch(Opts.opt_timestamp) {
	case 0:
		if(timestampflg)
			sprintf(buf,"%4d.%09d  %6d.%06d   ",
				Aseconds,Ananoseconds,
				(rnanoseconds/MILLION + 1000*rseconds)
				,rnanoseconds % MILLION);
		else
			sprintf(buf,"%4d.%09d* %6s %6s   ",
				Aseconds,Ananoseconds, "","");
		break;
	case 1:
		if(timestampflg)
			sprintf(buf,"%4d.%06d   ",Aseconds,Ananoseconds / 1000);
		else
			sprintf(buf,"%4d.%06d*  ",Aseconds,Ananoseconds / 1000);
		break;
	case 2:
		if(timestampflg)
			sprintf(buf,"%4d.%09d  %13d   ",
				Aseconds,Ananoseconds,
				(rnanoseconds/1000 + MILLION*rseconds));
		else
			sprintf(buf,"%4d.%09d* %11s   ",
				Aseconds,Ananoseconds,"");
		break;
	default:
		indent(Nindent-Nindent_appl);
		return;
	}
	IPUTS_LJ(buf);
	indent(Nindent-Nindent_appl);
}

#endif


/*
 * NAME:     iputs
 * FUNCTION: print out the buffer and remember the current column and tabs.
 * INPUTS:   ljflag   true:  NL will left justify
 *                    false: NL will justify to column 'Nindent'
 *           s        beffer to print
 * RETURNS:  none
 *
 * The buffering is done so the SYSCALL field (-x option)
 * can be updated when the new SVC is known.
 * Also, with pid filtering (-p flag), the pid is not known until
 * the end of the event.
 */

pr_iputs(ljflg,s)
register char *s;
{
	register c;

	if(!Printflg)
		return;
	Debug("iputs(%s)\n",s);
	while(c = *s++) {
		if(lastc == '\n' && c != '\n' && !ljflg)
			indent(Nindent);
		switch(c) {
		case '\t':
			indent(TABSIZE - (Col + TABSIZE - Nindent_appl%TABSIZE)%TABSIZE);
			break;
		case '\n':
			*linep++ = c;
			pr_lineno++;
			Col = 0;
			if(Opts.opt_pagesize && (pr_lineno % Opts.opt_pagesize) == 1)
				pr_colheader();
			break;
		default:
			*linep++ = c;
			Col++;
			break;
		}
		if(linep >= linelim)
			linecheck(16);
		lastc = c;
	}
}

/*
 * ensure a place for at least n characters in linebuf.
 */
static linecheck(n)
{
	int newsize,oldsize;

	if(linelim-linep > n)
		return;
	oldsize = linep - linebuf;
	newsize = linelim - linebuf + LINEINCR;
	linebuf = realloc(linebuf,newsize);
	linep   = &linebuf[oldsize];
	linelim = &linebuf[newsize];
}


#ifdef TRCRPT

pr_delim()
{

	if(!ISDELIM(lastc))
		IPUTS(" ");
}

#endif


static pr_colheader()
{
	int printflgsv;

	printflgsv = Printflg;
	Printflg = 2;
	IPUTS_LJ(Colheader);
	lflush();
	Printflg = printflgsv;
}

/*
 * lflush does pid filtering because it is only at the end
 * of an event that the Pid is known for sure.
 */
lflush()
{
	int n;

	n = linep - linebuf;
	linep = linebuf;
	if(n == 0 || Printflg == 0)
		return;
	if(Printflg >= 2 || !(Condflg && !(pidlist_chk() && tidlist_chk()))) {
		Prev_ananoseconds = Ananoseconds;
		Prev_aseconds = Aseconds;
		if(fwrite(linebuf,1,n,Rptfp) <= 0) {
			perror("fwrite");
			genexit(1);
		}
	}
}

/*
 * output n blanks
 */
static indent(n)
{
	char buf[128];

	if(!Printflg || n < 0)
		return;
	linecheck(n);
	Col += n;
	memset(linep,' ',n);
	linep += n;
	*linep = '\0';
	lastc = ' ';
}


#ifdef TRCRPT

/*
 * Indent to column an
 */
static aindent(an)
{

	if(!Printflg)
		return;
	if(an - Col > 0)
		indent(an - Col);
}

/*
 * This is the prevent routine when the -r option is invoked.
 * 
 * -r
 *    length    4 bytes
 *    data      'length' bytes
 * -rr
 *    "print out each event in ascii hex, terminated by a newline.
 * -rrr
 *    data      'length' bytes   (used for extracting events from a trace log)
 */
static prraw(buf,n)
unsigned char *buf;
{
	int i;
	union {
		int i;
		unsigned char c[4];
	} uc;

	if(!Printflg)						/* if conditional-ed out */
		return;
	switch(rawflag) {
	case 0:
		return;							/* not on */
	case 2:
		for(i = 0; i < n; i++) {		/* ascii hex */
			if (printf("%02X",buf[i]) <= 0) {
				perror("printf");
				genexit(1);
			}
		}
		if(printf("\n") <= 0) {			/* terminated by a newline */
			perror("printf");
			genexit(1);
		}
		return;
/*
	case 1:
		uc.i = n;
		OUT(uc.c[0]);
		OUT(uc.c[1]);
		OUT(uc.c[2]);
		OUT(uc.c[3]);
*/
	default:
		for(i = 0; i < n; i++) {		/* raw event data */
			OUT(buf[i]);
			if (ferror(stdout) != 0) {
				perror("putc");
				genexit (1);
			}
		}
	}
}

/*
 * Called by pass2 to print the very top header (filename)
 */
pr_hdr()
{
	static bannerflg;
	char buf[1024];

	if(nohdrflag)
		return;
	if(!bannerflg) {
		bannerflg++;
		IPUTS_LJ("\
\n\
                               TRACE LOG REPORT\n\n");
	}
	sprintf(buf,"File: %s\n",Logfile);
	IPUTS_LJ(buf);
}

/*
 * Convert the L= code to an indentation value
 */
static codetoindent(code)
{

	switch(code) {
	case 0:
		return(Nindent_appl + (IND_KERN-1) * TABSIZE);
	case IND_NOPRINT:
		return(-1);
	case IND_0:
		return(0);
	default:
 		return(Nindent_appl + (code-1) * TABSIZE);
	}
}

/*
 * Add the nanoseconds value in the event timestamp to the
 * accumulated time.
 * Rollover is when the accumulated time is less than the previous value.
 * Then advance the Aseconds counter by 1.
 * The 1/2 second heartbeat in the trace device driver traces
 *   HKWD_TRACE_CLOCK to avoid missing seconds.
 */

/*
 * The period of the RT stopwatch is 838      nanoseconds.
 * The period of the RT clock     is 16666667 nanoseconds.
 */
#define RTSTP_NANOSECS(CURR,PREV) (((CURR - PREV) * 838)      % BILLION)
#define RTCLK_NANOSECS(CURR,PREV) (((CURR - PREV) * 20408000) % BILLION)

static settimestamp()
{
	static initflg;
	double rnano;

	if(!initflg) {
		initflg++;
		Prevtimestamp = Timestamp;
	}
	if((Hookword & HKID_MASK) == HKWD_TRACE_TRCON)
		Prevtimestamp = Timestamp;
	if(RTflg) {
		if(Timestamp & 0x80000000) {
			Timestamp &= ~0x80000000;
			rnano = (double)RTSTP_NANOSECS(Timestamp,Prevtimestamp);
		} else {
			rnano = (double)RTCLK_NANOSECS(Timestamp,Prevtimestamp);
		}
	} else {
/*
 *	For POWER system, the time is kept in second and nanosection.
 *	But for POWER PC, the time is kept in tic, hence conversion
 *	is required to convert tic to nanosecond.
 *      cnvtfact is setup when the subhookid type SUBHKID_TBL is received.
 *	The default cnvtfact is 1 (non power PC).
 */
		rnano = (double)(Timestamp - Prevtimestamp);
		if (rnano < 0)		/* wraparound, see rpt.h */
			rnano += time_wrap;
		rnano *= cnvtfact;	/* Convert to nanoseconds */
	}
	Ananoseconds += (int)rnano;
	if(Ananoseconds >= BILLION) {
		Ananoseconds -= BILLION;
		Aseconds++;
	}
	Prevtimestamp = Timestamp;
}

pass2excp()
{

	Debug("segmentation error. pass 2. id=%03X index=%06X.%02X\n",
		Logtraceid,Logidx0,Logidx-Logidx0);
	genexit(2);
}

/*
 * level_eval is handled here instead of in eval.c because
 * it manipulates the Nindent variable.
 */
level_eval(fp)
struct formatd *fp;
{

	if(fp->f_fld1 == IND_0) {
		Debug("level_eval: IND_0: %d %d\n", pr_lineno, pr_lineno0);
		if(pr_lineno == pr_lineno0) {
			linep = linebuf;
			*linep = '\0';
		}
		Nindent = 0;
		return;
	}
	Nindent = codetoindent(fp->f_fld1);
	aindent(Nindent);
}

pr_error()
{
	char buf[256];

	sprintf(buf,"\n\
***ERROR: TRACEID %03X LOGFILE %s OFFSET %06X***",
		Logtraceid_real,Logfile,Logidx0);
	IPUTS_LJ(buf);
	longjmp(pr_jmpbuf,1);
}

#endif


pr_exec()
{
	char buf[128];
	int n;

	sprintf(buf,"%-14.14s ",exec_lookup());
	n = MIN(strlen(buf),14);
	memset(&linebuf[pr_exec_col],' ',14);
	memcpy(&linebuf[pr_exec_col],buf,n);
}


#ifdef TRCRPT

pr_pid()
{
	char buf[128];
	int n;

	sprintf(buf,"%-8d",Pid);
	memcpy(&linebuf[pr_pid_col],buf,8);
}

void pr_cpuid()
{
        char buf[128];
        int n;
 
        sprintf(buf,"%-2d",Cpuid);
        memcpy(&linebuf[pr_cpuid_col],buf,2);
}


void pr_tid()
{
	char buf[128];
	int n;

	sprintf(buf,"%-8d",Tid);
	memcpy(&linebuf[pr_tid_col],buf,8);
}

pr_svc()
{
	char buf[32];
	int n;

	Debug("pr_svc : buf = %s \n",buf);
	svc_lookup_buf(buf);
	n = MIN(strlen(buf),12);
	memset(&linebuf[pr_svc_col],' ',12);
	memcpy(&linebuf[pr_svc_col],buf,n);
}

#endif


pr_file()
{
	char *cp;
	int n;

	memset(&linebuf[pr_file_col],' ',14);
	if((cp = currfile_lookup()) == 0 || STREQ(cp,"-"))
		return;
	cp = basename(cp);
	n = MIN(strlen(cp),14);
	memcpy(&linebuf[pr_file_col],cp,n);
}


#ifdef TRCRPT

static svc_lookup_buf(buf)
char *buf;
{
	int svc;
	char *svcname;

	svc = svc_lookup();
	if(svc == 0)
		buf[0] = '\0';
	else if(svcname = rptsym_nmlookup(svc,0))
		strcpy(buf,svcname);
	else
		sprintf(buf,"-%08X-",svc);
}

static
timechk()
{

	Debug("timechk(): entry Printflg = %d\n",Printflg);

 	if(Logtraceid == HKWDTOHKID(HKWD_TRACE_HEADER))		/* not subject to time */
		return(0);

	if(Opts.opt_startp &&
	   NSECLT(Aseconds,Ananoseconds,Opts.opt_startp[0],Opts.opt_startp[1]))
		Printflg = 0;
	if(Opts.opt_endp &&
	   NSECLT(Opts.opt_endp[0],Opts.opt_endp[1],Aseconds,Ananoseconds)) {
		IPUTS("\n");
		lflush();
		return(-1);
	}
	Debug("timechk(): before return Printflg = %d\n",Printflg);

	return(0);
}

struct histd {
	unsigned h_id;
	int      h_count;
};
static struct histd h[NHOOKIDS];

static hist(hw)
{

	h[HOOKTOID(hw)].h_count++;
}

static hcmp();

histflush()
{
	int i;
	struct histd *hp;
	union td *tdp;
	char *str;

	if(!Opts.opt_histflg)
		return;
	for(i = 0; i < NHOOKIDS; i++)
		h[i].h_id = i;
	qsort(h,NHOOKIDS,sizeof(h[0]),hcmp);
	for(i = 0; i < NHOOKIDS; i++) {
		hp = &h[i];
		if(hp->h_count == 0)
			continue;
		/* print data */
		if(printf("%03X   %-6d  ",hp->h_id,hp->h_count) <= 0) {
			perror("printf");
			genexit(1);
		}
		if(rawflag) {
			if(printf("\n") <= 0) {
				perror("printf");
				genexit(1);
			}
			continue;
		}
		if((tdp = Tindexes[hp->h_id].t_tdp) == 0) {
			str = "UNDEFINED";
		} else {
			str = ((struct stringd *)(tdp->td_next))->s_string;
			if(*str == '@')
				str++;
		}
		if(printf("%s\n",str) <= 0) {
			perror("printf");
			genexit(1);
		}
	}
}

/*
 * descending order
 */
static hcmp(hp1,hp2)
struct histd *hp1,*hp2;
{

	if(hp1->h_count < hp2->h_count)
		return(+1);
	if(hp1->h_count > hp2->h_count)
		return(-1);
	return(0);
}


#endif

