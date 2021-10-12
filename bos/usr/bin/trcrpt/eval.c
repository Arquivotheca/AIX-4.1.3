static char sccsid[] = "@(#)16        1.12  src/bos/usr/bin/trcrpt/eval.c, cmdtrace, bos411, 9433B411a 8/17/94 03:28:16";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: rprevent, argtostr, pr_string
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
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <ctype.h>
#include <sys/trchdr.h>					/* GENBUFSIZE */
#include "rpt.h"
#include "td.h"
#include "parse.h"

extern char *fdlookup();				/* fd to filename */
extern char *currfile_lookup();			/* pid to current filename */
extern char *exec_lookup();				/* Pid to exec_pathname */
extern char *exec_lookupbypid();		/* pid to exec_pathname */
extern char *rptsym_nmlookup();			/* symbol value to symbol name */
extern char *loaderlookup();			/* loaded symbol value to name */
extern char *bitflagsstr();
extern char *argtostr();

extern union td *fntodesc();

#define ISQUOTED(sp) ((sp)->s_flags & SFLG_QUOTED)
#define ISDELIM(c)   ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '=')
#define ISBIT(buf,n) ( buf[(n)/8] & (1 << (7 - (n) % 8)) )
#define IPUTS(s)     pr_iputs(0,s)		/* default justification */
#define NEXT(p,TYPE) p = (struct TYPE *)p->u_next

/* array copy w/ null termination */
#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)] = '\0';

#define MAXLEVELS 32				/* descriptor nesting */
#define TSLOP     32
#define LARGELOOP GENBUFSIZE		/* detect bad LOOP value */
#define TBUFSIZE  (GENBUFSIZE+TSLOP)/* equal to max. size of generic event */

int Inaddr;				/* Internet address of this machine */
extern Pid;						/* current pid */
extern Nodelimflg;
extern Intr_depth;
extern Aseconds0;					/* time() variable */
extern Aseconds;					/* Absolute seconds */
extern Ananoseconds;
extern Rnanoseconds;
extern jmp_buf pr_jmpbuf;					/* setjmp/longjmp on $ERROR */
extern lastc;		/* last character written, to know when to output ' ' */
extern Baseindex;	/* start of data within Eventbuf[]. Usually 0 */
extern Byteindex;	/* current byte index into Eventbuf[] */
extern Bitindex;	/* current bit  index into Eventbuf[] */
extern pr_lineno0;
extern pr_lineno;
extern Printflg;

static Symbol_value;
static Symbol_range;
static Fnum;					/* current register value */

#define NFUNCLEVELS 3
int    Nregisters[NFUNCLEVELS];			/* number (0 to last) in use */
struct registers {						/* a register (macro) can have */
	int  r_value;						/* a numeric value or 32 byte */
	char r_string[32];					/* character value */
};
/* R holds macros ($xxx) for current template */
static struct registers R[NFUNCLEVELS][NREGISTERS];
static struct registers *numtoreg();	/* convert register number to R[] */
static efunclevel;
#define REG(n) R[efunclevel][n]

union isc {								/* union for converting */
	unsigned int   ui[1];				/* char arrays to unsigned, short, */
	unsigned short us[2];				/* etc */
	unsigned char  uc[4];
	int   i[1];
	short s[2];
	char  c[4];
};
union dfc {								/* union for "converting" */
	double d[1];						/* char. array (Eventbuf[]) to */
	float  f[2];						/* float or double */
	char   c[8];
};

	int tidflg;
	int pidflg;
	int cpuidflg;
	int priflg;

/*
 * This routine is called recursively by several of the _eval()
 * routines to print a list of template tokens.
 */

#ifdef TRCRPT

rprevent(tdp0,level)
union td *tdp0;
{
	register union td *tdp;

	if(level >= MAXLEVELS) {
		Debug("Exceeding descriptor nesting limit. %d levels deep",
			level);
		return;
	}
	pidflg = 0;
	tidflg = 0;
	cpuidflg = 0;
	priflg = 0;
	Debug("rprevent(%x,%d)\n",tdp0,level);
	for(tdp = tdp0; tdp; tdp = tdp->td_next) {
		switch(tdp->td_type) {
		case IBITFLAGS: bitflags_eval(tdp);      continue;
		case IQDESCRP:  qdescrp_eval(tdp,level); continue;
		case IFUNCTION: func_eval(tdp,level);    continue;
		case ILEVEL:    level_eval(tdp);         continue;
		case IMACDEF:   macdef_eval(tdp);        continue;
		case IFORMAT:   format_eval(tdp);        continue;
		case ILOOP:     loop_eval(tdp,level);    continue;
		case ISWITCH:   switch_eval(tdp,level);  continue;
		case ISTRING0:  string_eval(tdp);        continue;
		case ISTRING:   string_eval(tdp);        continue;
		}
		Debug("prevent: unknown type %d\n",tdp->td_type);
		genexit(1);
	}
}

static bitflags_eval(swp)
register struct switchd *swp;
{
	register struct flagsd *flp;
	char *cp;
	unsigned testval;
	unsigned field;
	unsigned mask;

	if(expr_eval(0,swp->s_expr))
		return;
	testval = Fnum;
	Debug("bitflags_eval(%x)\n",testval);
	for(flp = (struct flagsd *)swp->s_case; flp; NEXT(flp,flagsd)) {
		Debug("BITFLAG: %s\n",bitflagsstr(flp));
		field = flp->fl_field->s_value;
		if(flp->u_type == IBITFLAGS) {
			mask = field;
			Debug("%x %x\n",testval,mask);
			cp = (testval & mask) == field ?
				argtostr(0,flp->fl_true) : argtostr(0,flp->fl_false);
		} else {
			mask = flp->fl_mask->s_value;
			cp = (testval & mask) == field ?
				argtostr(0,flp->fl_true) : 0;
		}
		if(cp && *cp) {
			if(!ISDELIM(lastc) && !ISDELIM(*cp) && !Nodelimflg)
				IPUTS(" ");
			IPUTS(cp);
		}
	}
	/*lastc = 'x';*/		/* this will force a ' ' before next character */
}

static switch_eval(swp,level)
register struct switchd *swp;
{
	register struct cased *cp;
	register struct stringd *sp;
	int stringflg;
	int testval;
	char buf[TBUFSIZE];

	stringflg = expr_eval(buf,swp->s_expr);	/* This will set Fnum */
	testval = Fnum;
	if(stringflg)
		Debug("SWITCH: compare to '%s'\n",buf);
	else
		Debug("SWITCH: compare to %x\n",testval);
	for(cp = swp->s_case; cp; NEXT(cp,cased)) {
		sp = cp->c_value_label;
		Debug("CASE: '%s'|%x 0x%x %x\n",
			sp->s_string,sp->s_value,testval,sp->s_flags);
		if( sp->u_type == IMATCH ||
		   !stringflg && sp->s_value == testval ||
		    stringflg && STREQ(buf,sp->s_string)) {
			Debug("match\n");
			string_eval(sp->u_next);
			if(cp->c_desc)
				rprevent(cp->c_desc,level+1);
			break;
		}
	}
	/*lastc = 'x';*/		/* this will force a ' ' before next character */
	Debug("return from SWITCH\n");
}

static format_eval(fp)
struct formatd *fp;
{
	char buf[TBUFSIZE];

	sprformat(buf,fp);
	switch(fp->f_fmtcode) {
	case 'R':
	case 'O':
	case 'G':
		return;
	}
	if(!ISDELIM(lastc))
		IPUTS(" ");
	IPUTS(buf);
}

#endif


/*
 * If the string does not have a left delimiter (' '),
 *   put one there if not already.
 * If the string is quoted and did not have a right delimiter,
 *   set lastc to ' ' to avoid a delimiter the next time.
 */
static string_eval(sp)
register struct stringd *sp;
{

	if(sp == 0)
		return;
	if(sp->u_type == ISTRING0 && *sp->s_string == '@')
		return;
	Debug("string_eval(%s) %x lastc=%c\n",sp->s_string,sp->s_flags,lastc);
	if(!(sp->s_flags & SFLG_QUOTED && !(sp->s_flags & SFLG_LDELIM)))
		if(!ISDELIM(lastc) && !ISDELIM(*sp->s_string))
			IPUTS(" ");
	IPUTS(sp->s_string);
	if( (sp->s_flags & SFLG_QUOTED && !(sp->s_flags & SFLG_RDELIM)))
		lastc = ' ';		/* this will suppress next IPUTS(" ") */
}


#ifdef TRCRPT

static qdescrp_eval(tdp,level)
union td *tdp;
{

	if(tdp->td_next2 == 0)
		return;
	rprevent(tdp->td_next2,level+1);
}

static loop_eval(fp,level)
register struct formatd *fp;
{
	int n;
	char buf[TBUFSIZE];

	sprformat(0,fp);
	n = Fnum;
	if(n > LARGELOOP) {
		Debug("Large loop value 0x%x",n);
		longjmp(pr_jmpbuf,2);
	}
	if(fp->f_desc->td_type == IFORMAT && fp->f_desc->td_next == NULL) {
		Debug("simple LOOP\n");
		if(!ISDELIM(lastc))
			IPUTS(" ");
		fp = (struct formatd *)fp->f_desc;
		switch(fp->f_fmtcode) {
		case 'R':
		case 'O':
		case 'G':
			while(--n >= 0)
				sprformat(buf,fp);
			return;
		}
		while(--n >= 0) {
			sprformat(buf,fp);
			Debug("buf '%s'\n",buf);
			IPUTS(buf);
			if(n && fp->f_fld1)	/* X0 does not add spaces */
				IPUTS(" ");
		}
		return;
	}
	while(--n >= 0)
		rprevent(fp->f_desc,level+1);
}


/*                 
 * manage a new macro $TID
 * now the search of the current structure u is made thru the thread id
 * and not thru the process id
 */
static macdef_eval(mp)
register struct macdefd *mp;
{
	char buf[256];
	struct registers *rp;
	int stringflg;

	if(mp->m_expr == 0)
		return;
	switch(mp->m_code) {
	case '$':
		rp = numtoreg(mp->m_name);
		if(expr_eval(buf,mp->m_expr)) {	/* This will set Fnum */
			rp->r_value = 0;			/* to the value of the m_expr */
			VCPY(buf,rp->r_string);
		} else {
			rp->r_value = Fnum;
			rp->r_string[0] = '\0';
		}
		Debug("MACDEF: REG#%d val=%d '%s'\n",
			mp->m_name,rp->r_value,rp->r_string);
		return;
	}
	switch(mp->m_name) {
	case RREG_EXECPATH:
		break;
	default:
		stringflg = expr_eval(buf,mp->m_expr);
		break;
	}
	switch(mp->m_name) {
	case RREG_EXECPATH:
		exec_eval(mp->m_expr);
		goto ret;
	case RREG_SVC:
		svc_install(Fnum);
		if(Fnum && Opts.opt_svcflg)		/* "retrofit" new svc */
			pr_svc();
		goto ret;
	case RREG_SYMBOL_VALUE:
		Symbol_value = Fnum;
		goto ret;
	case RREG_SYMBOL_RANGE:
		Symbol_range = Fnum;
		goto ret;
	case RREG_SYMBOL_NAME:
		symbol_eval(mp->m_expr);
		goto ret;
	case RREG_INADDR:
		Inaddr = Fnum;
		goto ret;
	case RREG_PID:
		Pid = Fnum;
		if (!Threadflg) {
			setpid();
			Tid = Pid;
		}
		else {
			if (tidflg) {
				pid_install();
			}
			else {
				pidflg++;
				Debug("pidflg = %d \n",pidflg);
			}
		}
                if(Opts.opt_pidflg)
                        pr_pid();
		goto ret;
	case RREG_CPUID:
		Cpuid = Fnum;
		cpuid_remove(Cpuid);
		if (Threadflg) {
			if (tidflg) {
				cpuid_install();
			}
			else {
				cpuidflg++;
				Debug("cpuidflg = %d \n",cpuidflg);
			}
		}
                if(Opts.opt_cpuidflg)
                        pr_cpuid();
		goto ret;
	case RREG_PRI:
		Pri = Fnum;
		if (Threadflg) {
			if (tidflg) {
				pri_install();
			}
			else {
				priflg++;
				Debug("pridflg = %d \n",priflg);
			}
		}
		goto ret;
	case RREG_TID:
		if (Threadflg) {
			Tid = Fnum;
			settid(Tid,-1);
			if (pidflg) {
				pid_install();
				pidflg = 0;
			}
			if (cpuidflg) {
				cpuid_install();
				cpuidflg = 0;
			}
			if (priflg) {
				pri_install();
				priflg = 0;
			}
			tidflg++;
		}
		if(Opts.opt_tidflg)
			pr_tid();
		goto ret;
	case RREG_BASEPOINTER:
		Debug("set base to %d\n",Fnum);
		if(Fnum < 0)
			Baseindex = 0;
		else if(Fnum > Eventsize)
			Baseindex = Eventsize;
		else
			Baseindex = Fnum;
		goto ret;
	case RREG_DATAPOINTER:
		Debug("set dp to %d\n",Fnum);
		if(Fnum < 0)
			Byteindex = 0;
		else if(Fnum > Eventsize)
			Byteindex = Eventsize;
		else
			Byteindex = Fnum;
		goto ret;
	}
ret:
	Debug("MACDEF: number=%d value=%08x\n",mp->m_name,Fnum);
}

/*
 * {{ $EXECPATH = A14 }}
 * will cause execeval to remember A14 as the pathname for process id Pid.
 */
static exec_eval(ep)
struct exprd *ep;
{
	union td *tdp;
	char buf[128];
	int n;

	if((tdp = ep->e_left) && tdp->td_type == IFORMAT) {
		sprformat(buf,tdp);
		exec_install(buf);
		if(Opts.opt_execflg)
			pr_exec();
	}
}

static symbol_eval(ep)
struct exprd *ep;
{
	union td *tdp;
	char buf[128];

	if((tdp = ep->e_left) && tdp->td_type == IFORMAT) {
		sprformat(buf,tdp);
		rptsym_install(buf,Symbol_value);
	}
}

#define NFUNCARGS 6
#define FUNC_PUSH(VALUE,STRINGFLG) \
{ \
	Debug("     push(%08x)\n",VALUE); \
	if(r_idx < NFUNCARGS) { \
		if(!internalflg && STRINGFLG) { \
			memcpy(r[r_idx].r_string,(VALUE),sizeof(r[0].r_string)); \
			r[r_idx].r_string[sizeof(r[0].r_string)-1] = '\0'; \
		} else { \
			r[r_idx].r_string[0] = '\0'; \
		} \
		r[r_idx].r_value = (int)(VALUE); \
		r_idx++; \
	} \
}

static func_eval(fnp,level)
struct functiond *fnp;
{
	union td *tdp;			/* list of args */
	int r_idx;				/* index into a[] */
	char buf[1024];			/* expand character args in this buffer */
	char *bufp;				/* pointer into buf[] */
	int base;
	int n;
	int rv;
	struct formatd *fp;
	struct stringd *sp;
	struct registers *rp;
	struct builtins *blp;
	struct registers r[6];
	int internalflg;
	int i;

	Debug("func_eval(%d)\n",fnp->fn_number);
	if((blp = fntobuiltin(fnp->fn_number)) == 0)
		return;
	internalflg = blp->bl_func ? 1 : 0;
	r_idx = 0;
	bufp = buf;
	for(tdp = fnp->fn_arglist; tdp; tdp = tdp->td_next) {
		switch(tdp->td_type) {
		case IFORMAT:
			fp = (struct formatd *)tdp;
			switch(fp->f_fmtcode) {
			case 'A':
			case 'S':
rstring:
				sprformat(bufp,fp);
				FUNC_PUSH(bufp,1);
				bufp += strlen(bufp);
				break;
			case 'r':
				if(ischartype(fp) || fp->f_xfmtcode == 'S')
					goto rstring;
				sprformat(0,fp);
				FUNC_PUSH(Fnum,0);
				break;
			case '$':
				rp = numtoreg(fp->f_val);
				if(rp->r_string[0]) {
					FUNC_PUSH(rp->r_string,1);
				} else {
					FUNC_PUSH(rp->r_value,0);
				}
				break;
			default:
				sprformat(0,fp);
				FUNC_PUSH(Fnum,0);
			}
			continue;
		case ISTRING:
			sp = (struct stringd *)tdp;
 			if(!ISQUOTED(sp) && (base = numchk(sp->s_string,10)) > 0) {
				n = strtol(sp->s_string,0,base);
				FUNC_PUSH(n,0);
			} else {
				FUNC_PUSH(sp->s_string,1);
			}
			continue;
		default:
			Debug("func_eval: type=%d\n",tdp->td_type);
			continue;
		}
	}
	if(blp->bl_func) {
		(*blp->bl_func)(r[0].r_value,r[1].r_value,r[2].r_value,
				r[3].r_value,r[4].r_value,r[5].r_value);
		return;
	}
	efunclevel++;
	for(i = 0; i < r_idx; i++) {
		Debug("func_eval[%d] %x %s\n",
			i,REG(i).r_value,REG(i).r_string[0] ? REG(i).r_string : "-");
		REG(i) = r[i];
	}
	for(; i < blp->bl_regcount; i++) {
		REG(i).r_string[0] = '\0';
		REG(i).r_value     = 0;
	}
	Nregisters[efunclevel] = blp->bl_regcount;
	rprevent(fntodesc(fnp->fn_number),level+1);
	efunclevel--;
}

static ischartype(fp)
struct formatd *fp;
{
	struct registers *rp;

	switch(fp->f_fmtcode) {
	case 'S':
	case 'A':
		return(1);
	case '$':
		rp = numtoreg(fp->f_val);
		return(rp->r_string[0] ? 1 : 0);
	case 'X':
		return(fp->f_fld1 > 4 ? 1 : 0);
	case 'r':
		if(fp->f_xfmtcode == 'S')
			return(1);
		switch(fp->f_val) {
		case RREG_EXECPATH:
		case RREG_CURRFILE:
			return(1);
		}
		return(0);
	}
	return(0);
}

/*
 * Evaluate the expression ep and return the numeric value.
 * This is not used very much.
 * Return non-zero if the expression looks like a string
 */
static expr_eval(buf,ep)
char *buf;
struct exprd *ep;
{
	int al,ar;
	union td *tdp;

	Debug("expr_eval(%s)\n",exprstr(ep));
	if(buf)
		buf[0] = '\0';
	switch(ep->e_op) {
	case 0:			/* '=' */
	case '=':
		if(ep->e_right)
			break;
		tdp = ep->e_left;
		switch(tdp->td_type) {
		case IFORMAT:
			if(ischartype(tdp)) {
				sprformat(buf,tdp);
				return(1);
			}
			sprformat(0,tdp);
			return(0);
		case ISTRING:
		  {
			struct stringd *sp;
			int base;

			sp = (struct stringd *)tdp;
 			if(!ISQUOTED(sp) && (base = numchk(sp->s_string,10)) > 0) {
				Fnum = strtol(sp->s_string,0,base);
				return(0);
			} else {
				if(buf)
					strcpy(buf,sp->s_string);
				return(1);
			}
		  }
		}
		return(0);
	}
	al = ep->e_left  ? expr_arg_eval(ep->e_left,0) : 0;
	ar = ep->e_right ? expr_arg_eval(ep->e_right,formatbase(ep->e_left)) : 0;
	Debug("al=%x ar=%x op=%c\n",al,ar,ep->e_op);
	switch(ep->e_op) {
	case '&': Fnum = al & ar; break;
	case '+': Fnum = al + ar; break;
	case '-': Fnum = al - ar; break;
	case '*': Fnum = al * ar; break;
	case '/': Fnum = ar == 0 ? 0x7FFFFFFF : al / ar; break;
	}
	return(0);
}

static formatbase(fp)
struct formatd *fp;
{

	if(fp->u_type != IFORMAT)
		return(0);
	switch(fp->f_fmtcode) {
	case 'X':
		return(16);
	case 'D':
	case 'U':
		return(10);
	case 'o':
		return(8);
	case '$':
	case 'r':
		switch(fp->f_xfmtcode) {
		case 'X':
			return(16);
		case 'D':
		case 'U':
			return(10);
		case 'o':
			return(8);
		default:
			return(0);
		}
	default:
		return(0);
	}
}

/*
 * Return the numeric value of the argument 'ap'
 * It can be a FORMAT, a REG, or STRING (constant)
 */
static expr_arg_eval(tdp,base)
union td *tdp;
{

	switch(tdp->td_type) {
	case IFORMAT:
		Debug("expr_arg_eval(%s)\n",formatstr(tdp));
		sprformat(0,tdp);
		return(Fnum);
	case ISTRING:
		return(strtol(((struct stringd *)tdp)->s_string,0,base));
	}
	return(0);
}

/*
 * Verify that Register 'n' is valid
 * The registers are zero-ed as they are used.
 * Any unused registers below a declared registers (n+1 > Nregisters)
 *   are zeroed also.
 */

static struct registers *numtoreg(n)
unsigned n;
{
	int i;

	if(n >= NREGISTERS) {
		Debug("prevent: Bad register number %d",n);
		return(&REG(0));
	}
	if(n+1 > Nregisters[efunclevel]) {		/* zero out unaccessed registers */
		for(i = Nregisters[efunclevel]; i <= n; i++) {
			REG(i).r_value     = 0;
			REG(i).r_string[0] = '\0';
		}
		Nregisters[efunclevel] = n+1;
	}
	return(&REG(n));
}

/*
 * The W format code actually modifies the Fnum
 */
static wmask(fp)
struct formatd *fp;
{
	int i;
	int endbit;
	int startbit;
	unsigned mask;

	endbit   = fp->f_fld2;
	startbit = fp->f_fld1;
	mask = 1;
	if(startbit > 0)
		mask <<= startbit;
	for(i = startbit; i < endbit; i++)
		mask |= (mask << 1);
	Fnum &= mask;
	Fnum = (unsigned)Fnum >> startbit;
}

/*
 * NAME:     sprformat
 * FUNCTION: sprintf the FORMAT into a buffer.
 * INPUTS:   buf     buffer to print into
 *           fp      formatd structure for this format code
 * RETURNS:  none
 *           The numeric value (if any) is placed in 'Fnum'
 *           The Bitindex and Byteindex are updated according to the length
 *             in the format code.
 *
 * If the format code wants data (such as X4), make sure that the Bitindex
 *   is 0. (byte aligned)
 * Then look at the f_fmtcode:
 * i:   (internal)
 *      Flow control: $STOP, $BREAK, $ERROR, $DEFAULT
 *      Values:       $LOGIDX, $TRACEID, etc
 * r:   (reserved registers)
 *      $BASEPOINTER, $DATAPOINTER, etc. (These work the same way that
 *          the internal ones do, except that the can be altered.
 * $:   This is a register, with optional formatting qualifier like %D2
 * H    HT    = hooktype
 *      HD,HB = hookdata
 * s:   space n ' '
 * S,A  ascii strings
 * F:   floating point
 * X,D,B,U
 * O,R,G
 * P:   process name of supplied process id
 * T:   ctime timestring
 * E:   perror error string
 */
/*                 
 * now manage a new macro $TID
 */
static sprformat(buf,fp)
register char *buf;
register struct formatd *fp;
{
	register n,m;
	register i;
	int Mn;
	int nbits,startbit,lastbit;
	union isc u;
	union dfc dfc;
	register count;
	char *cp;
	int c;
	int eosflg;
	struct registers *rp;

	Debug("sprformat: %s   %d.%d.%d\n",
		formatstr(fp),Baseindex,Byteindex,Bitindex);
	Fnum = 0;
	switch(fp->f_fmtcode) {
	case 'O':
	case 'G':
	case 'B':
	case 'i':
	case 'r':
	case '$':
		break;
	case 'R':
		Bitindex = 0;
		break;
	default:
		if(Bitindex != 0) {
			Bitindex = 0;
			getec();
		}
		break;
	}
	switch(fp->f_fmtcode) {
	case 'i':
		switch(fp->f_fld1) {
		case RES_STOP:
			longjmp(pr_jmpbuf,3);
		case RES_BREAK:
			longjmp(pr_jmpbuf,1);
		case RES_SKIP:
			longjmp(pr_jmpbuf,4);
		case RES_NOPRINT:
			Printflg = 0;
			break;
		case RES_ERROR:
			pr_error();
			break;
		case RES_LOGFILE:
			if(buf)
				sprintf(buf,"%s",Logfile);
			break;
		case RES_INDEX0:
			if(buf)
				sprintf(buf,"0x%x",Logidx0);
			Fnum = Logidx0;
			break;
		case RES_INDEX:
			if(buf)
				sprintf(buf,"0x%x",Logidx);
			Fnum = Logidx;
			break;
		case RES_TRACEID:
			if(buf)
				sprintf(buf,"%03X",Logtraceid);
			Fnum = Logtraceid;
			break;
		case RES_DEFAULT:
			n = fp->f_val;
			Debug("DEFAULT: call traceid(%03x) calling subroutine\n",n);
			if(Tindexes[n].t_tdp) {
				Debug("calling subroutine\n");
				if(!ISDELIM(lastc))
					IPUTS(" ");
				rprevent(Tindexes[n].t_tdp,1);
				Debug("return from subroutine\n");
			}
			if(buf)
				buf[0] = '\0';
			break;
		default:
			Debug("sprformat: Unknown internal format %d",fp->f_fld1);
			genexit(1);
		}
		break;
	case 'r':		/* reserved registers */
		switch(fp->f_val) {
		case RREG_RELLINENO:
			Fnum = pr_lineno-pr_lineno0+1;
			if(buf)
				sprreg(buf,fp);
			break;
		case RREG_LINENO:
			Fnum = pr_lineno;
			if(buf)
				sprreg(buf,fp);
			break;
		case RREG_HTYPE:
		case RREG_HDATAL: 
		case RREG_HDATAU: 
		case RREG_HDATA: 
		case RREG_DATA1: 
		case RREG_DATA2: 
		case RREG_DATA3: 
		case RREG_DATA4: 
		case RREG_DATA5: 
			Debug("sprformat : case RREG_DATA5 \n");
			Fnum = getvnum_d(fp->f_val);
			if(fp->f_xfmtcode == 'S') {		/* print strings: length in reg */
				if(buf == 0)
					break;
				m = n = Fnum;
				goto rstring;
			}
			if(fp->f_xfmtcode == 'W')
				wmask(fp);
			if(buf)
				sprreg(buf,fp);
			break;
		case RREG_BASEPOINTER:
			if(buf)
				sprintf(buf,"%d",Baseindex);
			Fnum = Baseindex;
			Debug("get base: %d\n",Fnum);
			break;
		case RREG_DATAPOINTER:
			if(buf)
				sprintf(buf,"%d",Byteindex);
			Fnum = Byteindex;
			Debug("get dp: %d\n",Fnum);
			break;
		case RREG_SVC:
			if(buf) {
				n = svc_lookup();
				if(n == 0)
					sprintf(buf,"system call");
				else
					sprsym(buf,n);
			}
			break;
		case RREG_SYMBOL_RANGE:
			if(buf)
				sprintf(buf,"%08X",Symbol_range);
			Fnum = Symbol_range;
			break;
		case RREG_SYMBOL_VALUE:
			if(buf)
				sprintf(buf,"%08X",Symbol_value);
			Fnum = Symbol_value;
			break;
		case RREG_SYMBOL_NAME:
			if(buf)
				sprsym(buf,Symbol_value);
			break;
		case RREG_CURRFILE:
			if(buf)
				sprintf(buf,"%s",currfile_lookup());
			break;
		case RREG_EXECPATH:
			if(buf)
				sprintf(buf,"%s",exec_lookup());
			break;
		case RREG_TIME:
			Fnum = Aseconds + Aseconds0;
			if(buf) {
				struct tm *tm;

				tm = localtime(&Fnum);
				sprintf(buf,"%02d%02d%02d%02d%02d",
					tm->tm_mon+1,
					tm->tm_mday,
					tm->tm_hour,
					tm->tm_min,
					tm->tm_year);
			}
			break;
		case RREG_INADDR:
			if(buf)
				sprintf(buf,"%08X",Inaddr);
			Fnum = Inaddr;
			break;
		case RREG_PID:
			if(buf)
				sprintf(buf,"%08X",Pid);
			Fnum = Pid;
			break;
		case RREG_TID:
			if(buf)
				sprintf(buf,"%08X",Tid);
			Fnum = Tid;
			break;
		case RREG_CPUID:
			if(buf)
				sprintf(buf,"%02X",Cpuid);
			Fnum = Cpuid;
			break;
		case RREG_PRI:
			Fnum = Pri;
			break;
		default:
			Debug("sprformat: Unknown reserved register %d",fp->f_val);
			genexit(1);
		}
		break;
	case '$':
		rp = numtoreg(fp->f_val);
		Fnum = rp->r_value;
		if(fp->f_xfmtcode == 'S') {		/* print strings with length in reg */
			if(buf == 0)
				break;
			m = n = Fnum;
			goto rstring;
		}
		if(fp->f_xfmtcode == 'W')
			wmask(fp);
		if(buf)
			sprreg(buf,fp);
		break;
	case 'H':
		switch(fp->f_fld1) {
		case 'T':
			Fnum = Eventbuf[1] & 0x0F;
			if(buf)
				sprintf(buf,"%02X",Fnum);
			break;
		case 'D':
		case 'B':
			u.uc[2] = Eventbuf[2];
			u.uc[3] = Eventbuf[3];
			Fnum = u.us[1];
			if(buf)
				sprintf(buf,"%04X",Fnum);
			break;
		}
		break;
	case 's':
		if(buf == NULL)
			break;
		n = fp->f_fld1;
		for(i = 0; i < n; i++)
			buf[i] = ' ';
		buf[i] = '\0';
		break;
	case 'S':
	case 'A':
		if(buf == NULL)
			break;
		if(fp->f_fmtcode == 'S') {
			Debug("sprformat : case A \n");
			n = getvnum(fp->f_fld1);
		}
		else
			n = fp->f_fld1 ? fp->f_fld1 : 1;
		m = fp->f_fld2;
rstring:
		if(n > TBUFSIZE-TSLOP) {
			Debug("Large string length 0x%x",n);
			longjmp(pr_jmpbuf,2);
		}
		count = 0;
		eosflg = 0;
		cp = buf;
		for(i = 0; i < n; i++) {
			c = getec();
			if(eosflg || c == '\0') {
				eosflg = 1;
				continue;
			}
			if(m && count >= m)
				continue;
			count++;
			*cp = c;
			cp++;
		}
		for(i = count; i < m; i++)	/* fill out to width m */
			*cp++ = ' ';
		*cp = '\0';
		break;
	case 'X':
		count = fp->f_fld1 ? fp->f_fld1 : 1;
		Fnum = 0;
		for(i = 0; i < count; i++) {
			n = getec();
			if(buf)
				sprintf(buf+2*i,"%02X",n);
			Fnum = Fnum * 256 + n;
		}
		if(buf)
			buf[2*i] = '\0';
		break;
	case 'R':
		if((Byteindex -= fp->f_fld1) < 0)
			Byteindex = 0;
		break;
	case 'G':
		nbits = fp->f_fld1 * 8 + fp->f_fld2;
		lastbit  = MIN(nbits,(Eventsize-Baseindex)*8);
		Byteindex = lastbit / 8;
		Bitindex  = lastbit % 8;
		Debug("G: Byte=%x Bit=%x\n",Byteindex,Bitindex);
		break;
	case 'O':
		nbits = fp->f_fld1 * 8 + fp->f_fld2;
		startbit = Byteindex * 8 + Bitindex;
		lastbit  = MIN(startbit+nbits,(Eventsize-Baseindex)*8);
		Byteindex = lastbit / 8;
		Bitindex  = lastbit % 8;
		break;
	case 'B':
		nbits = fp->f_fld1 * 8 + fp->f_fld2;
		if(buf == NULL)
			nbits = MIN(nbits,32);
		startbit = (Baseindex+Byteindex) * 8 + Bitindex;
		lastbit  = MIN(startbit+nbits,Eventsize*8);
		Fnum = 0;
		if(buf) {
			for(i = 0; i < nbits; i++) {
				if(startbit+i >= lastbit) {
					buf[i] = '0';
					continue;
				}
				buf[i] = ISBIT(Eventbuf,startbit+i) ? '1' : '0';
			}
			buf[nbits] = '\0';
		}
		for(i = 0; i < nbits; i++) {
			if(startbit+i >= lastbit)
				continue;
			Fnum <<= 1;
			if(ISBIT(Eventbuf,startbit+i))
				Fnum |= 1;
		}
		startbit = Byteindex * 8 + Bitindex;
		lastbit  = MIN(startbit+nbits,(Eventsize-Baseindex)*8);
		Debug("Byteindex=%d Bitindex=%d nbits=%d\n",
			Byteindex, Bitindex, nbits);
		Byteindex = lastbit / 8;
		Bitindex  = lastbit % 8;
		break;
	case 'P':
		if(buf == NULL)
			break;
		Debug("sprformat : case P \n");
		n = getvnum(4);
		sprintf(buf,"%s",exec_lookupbypid(n));
		break;
	case 'F':
		if(buf == NULL)
			break;
		for(i = 0; i < fp->f_fld1; i++)
			dfc.c[8 - fp->f_fld1 + i] = getec();
		if(fp->f_fld1 == 4)
			sprintf(buf,"%0.4E",dfc.f[1]);
		else
			sprintf(buf,"%0.4E",dfc.d[0]);
		break;
	case 'T':
		if(buf == NULL)
			break;
		Debug("sprformat : case T \n");
		n = getvnum(4);
		Mn = n;
		/*sprintf(buf,"%.24s",asctime(localtime(&Mn)));*/
		sprintf(buf,"%.24s",ctime(&Mn));
		break;
	case 'E':
		if(buf == NULL)
			break;
		Debug("sprformat : case E \n");
		n = getvnum(fp->f_fld1);
		if(n < 0 || Nerrorstr <= n)
			sprintf(buf,"%d",n);
		else
			sprintf(buf,"%s",Errorstr[n]);
		break;
	case 'o':
		count = fp->f_fld1 ? fp->f_fld1 : 1;
		Debug("sprformat : case o \n");
		u.i[0] = getvnum(count);
		switch(count) {
		case 1: Fnum = u.c[3]; break;	/* sign extend */
		case 2: Fnum = u.s[1]; break;
		case 4: Fnum = u.i[0]; break;
		}
		if(buf)
			sprintf(buf,"0%o",Fnum);
		break;
	case 'D':
		count = fp->f_fld1 ? fp->f_fld1 : 1;
		Debug("sprformat : case D \n");
		u.i[0] = getvnum(count);
		switch(count) {
		case 1: Fnum = u.c[3]; break;	/* sign extend */
		case 2: Fnum = u.s[1]; break;
		case 4: Fnum = u.i[0]; break;
		}
		if(buf)
			sprintf(buf,"%d",Fnum);
		break;
	case 'U':
		count = fp->f_fld1 ? fp->f_fld1 : 1;
		Debug("sprformat : case U \n");
		Fnum = getvnum(count);
		if(buf)
			sprintf(buf,"%u",Fnum);
		break;
	default:
		Debug("sprformat: unknown code %c\n",fp->f_fmtcode);
		genexit(1);
	}
}

static sprsym(buf,value)
char *buf;
{	
	char *cp;

	if(value == 0) {
		buf[0] = '\0';
		return;
	}
	if((cp = rptsym_nmlookup(value,0)) == 0)
		sprintf(buf,"%08X",value);
	else
		sprintf(buf,"%s",cp);
}

static sprreg(buf,fp)
char *buf;
register struct formatd *fp;
{
	char fmt[8];
	int nbits;
	int i;
	union isc u;

	Debug("sprreg code=%c\n",fp->f_xfmtcode ? fp->f_xfmtcode : '-');
	if(ischartype(fp)) {
		strcpy(buf,numtoreg(fp->f_val)->r_string);
		return;
	}
	switch(fp->f_xfmtcode) {
	case 'X':
		if(fp->f_fld1 == 0)
			sprintf(fmt,"%%X");
		else
			sprintf(fmt,"%%0%dX",2*fp->f_fld1);
		sprintf(buf,fmt,Fnum);
		break;
	case 'D':
		sprintf(buf,"%d",Fnum);
		break;
	case 'o':
		sprintf(buf,"0%o",Fnum);
		break;
	case 'U':
		sprintf(buf,"%u",Fnum);
		break;
	case 'P':
		sprintf(buf,"%s",exec_lookupbypid(Fnum));
		break;
	case 'B':		/* this is backwards */
		nbits = fp->f_fld1 * 8 + fp->f_fld2;
		if(nbits > 32)
			nbits = 32;
		u.i[0] = Fnum;
		for(i = 0; i < nbits; i++)
			buf[i] = ISBIT(u.c,i) ? '1' : '0';
		buf[nbits] = '\0';
		break;
	case 'E':
		if(0 < Fnum && Fnum < Nerrorstr)
			sprintf(buf,"%s",Errorstr[Fnum]);
		else
			sprintf(buf,"%d",Fnum);
		break;
	default:
		sprintf(buf,"%04X",Fnum);
		break;
	}
}

static getvnum_d(n)
{

	switch(n) {
	case RREG_HTYPE:
		return(*((char *)&Eventbuf[1]) & 0x0F);
	case RREG_HDATAL:
		return(*((char *)&Eventbuf[3]));
	case RREG_HDATAU:
		return(*((char *)&Eventbuf[2]));
	case RREG_HDATA:
		return(*((short *)&Eventbuf[2]));
	case RREG_DATA1: n = 1; break;
	case RREG_DATA2: n = 2; break;
	case RREG_DATA3: n = 3; break;
	case RREG_DATA4: n = 4; break;
	case RREG_DATA5: n = 5; break;
	default:         n = 0;
	}
	if(4 * n > Eventsize)
		return(0);
	return(*((int *)&Eventbuf[n * 4]));
}

/*
 * Get variable length (1, 2, or 4) number from Eventbuf and return
 * the numeric value.
 */
static getvnum(len)
{
	union isc u;
	int i;

	u.i[0] = 0;
	for(i = 0; i < len; i++) {
		u.c[4 - len + i] = getec();
		Debug("i=%d idx=%d char=%02x\n",i, 4 - len + i, u.c[4 - len + i]);
	}
	Debug("getvnum(%d) returns %x\n",len,u.i[0]);
	return(u.i[0]);
}

/*
 * Read the next byte from Eventbuf[].
 * Overrun returns 0.
 */
static getec()
{

	Bitindex = 0;
	if(Baseindex + Byteindex < Eventsize)
		return(Eventbuf[Baseindex + Byteindex++]);
	return(0);
}

#endif


pr_string(str)
char *str;
{
	struct stringd s;

	s.u_type   = ISTRING;
	s.s_string = str;
	s.s_flags  = SFLG_LDELIM|SFLG_RDELIM;
	string_eval(&s);
}


#ifdef TRCRPT

char *argtostr(buf,tdp)
char *buf;
union td *tdp;
{
	char *bufp;
	static char ibuf[128];

	bufp = buf ? buf : ibuf;
	Debug("argtostr(%x,%x) %x\n",buf,tdp,bufp);
	switch(tdp->td_type) {
	case ISTRING:
		strcpy(bufp,((struct stringd *)tdp)->s_string);
		break;
	case IFORMAT:
		sprformat(bufp,tdp);
		break;
	default:
		bufp[0] = '\0';
		break;
	}
	return(bufp);
}

#endif


Tprargs(length)
{
	char buf[TBUFSIZE];
	int c;
	register char *tcp,*fcp;

	tcp = buf;
	fcp = &Eventbuf[Byteindex];
	while(--length >= 0) {
		if((c = *fcp++) == '\0')
			c = ' ';
		*tcp++ = c;
	}
	*tcp = '\0';
	IPUTS(buf);
}

