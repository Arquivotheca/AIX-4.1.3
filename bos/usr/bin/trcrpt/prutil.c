static char sccsid[] = "@(#)09        1.18  src/bos/usr/bin/trcrpt/prutil.c, cmdtrace, bos411, 9428A410j 8/19/93 08:16:14";

/*
*/
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: tokstr, exprstr, argstr, formatstr, bitflagstr
 *            prtree, prtracelbl, ltrace
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989.1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * FUNCTION: tree listing utility for debugging templates.
 */  

#include <stdio.h>
#include "rpt.h"
#include "td.h"
#include "parse.h"

#define CASE(X) \
	case X: s = "X"; break;

char *tokstr(tok)
{
	char *s;
	static char buf[16];

	switch(tok) {
	CASE(IBITFIELD);
	CASE(IBITFLAGS);
	CASE(IXQUOTE);
	CASE(IREGFMT);
	CASE(ILBRACEM);
	CASE(IRBRACEM);
	CASE(ISTRING0);
	CASE(ILEVEL);
	CASE(IEXPR);
	CASE(IMACDEF);
	CASE(IREG);
	CASE(IEOD);
	CASE(ILBRACE);
	CASE(IRBRACE);
	CASE(IFORMAT);
	CASE(ILOOP);
	CASE(IMATCH);
	CASE(ISTRING);
	CASE(ISWITCH);
	CASE(IFUNCTION);
	CASE(EOF);
	default:
		if(' ' < tok && tok < 0x7F)
			sprintf(buf,"'%c'",tok);
		else
			sprintf(buf,"---%d---",tok);
		s = buf;
	}
	return(s);
}

char *exprstr(ep)
struct exprd *ep;
{
	union td *ap;
	static char buf[40];

	if(ap = ep->e_left)
		sprintf(buf,"%s",argstr(ap));
	sprintf(buf + strlen(buf)," %c ",ep->e_op ? ep->e_op : '\0');
	if(ap = ep->e_right)
		sprintf(buf + strlen(buf),"%s",argstr(ap));
	return(buf);
}

/*
 * Debug. Expand the arg to something meaningful
 */
char *argstr(tdp)
union td *tdp;
{
	static char buf[16];

	switch(tdp->td_type) {
	case IREG:
		sprintf(buf,"REG%d",((struct macdefd *)tdp)->m_name);
		return(buf);
	case IFORMAT:
		return(formatstr(tdp));
	case ISTRING:
		return(((struct stringd *)tdp)->s_string);
	}
	sprintf(buf,"yargstr: %d",tdp->td_type);
	return(buf);
}

char *formatstr(fp)
struct formatd *fp;
{
	static char buf[16];

	switch(fp->f_fmtcode) {
	case '$':
		sprintf(buf,"REG%d%%%c%d.%d",
			fp->f_val,fp->f_xfmtcode ? fp->f_xfmtcode : '#',
			fp->f_fld1,fp->f_fld2);
		break;
	case 'r':
		sprintf(buf,"RREG%d%%%c%d.%d",
			fp->f_val,fp->f_xfmtcode ? fp->f_xfmtcode : '#',
			fp->f_fld1,fp->f_fld2);
		break;
	case 'i':
		sprintf(buf,"iREG%d",fp->f_fld1);
		break;
	default:
		sprintf(buf,"%c%d.%d",
			fp->f_fmtcode ? fp->f_fmtcode : '-',fp->f_fld1,fp->f_fld2);
		break;
	}
	return(buf);
}

prtracelbl(tdp0)
union td *tdp0;
{
	int traceid;
	char *str;

	traceid = gettraceid();
	str = ((struct stringd *)(tdp0->td_next))->s_string;
	/* printing data */
	if(printf("%03X %s\n",traceid,*str == '@' ? str+1 : str) <= 0) {
		perror("printf");
		genexit(1);
	}
}

prtree(tdp0)
{
	int traceid;

	traceid = gettraceid();
	ltrace("%s %d.%d ",
		hexstr(traceid),
		Tindexes[traceid].t_version,Tindexes[traceid].t_release);
	iprtree(tdp0,0);
	ltrace("\n\n");
}


static lprstring(sp)
struct stringd *sp;
{

	while(sp) {
		ltrace("%s ",sp->s_string);
		sp = (struct stringd *)sp->u_next;
	}
}

static icurrcol;
static ileftcol;

#define CASEINDENT 2
#define TAB "--------"

static iprtree(tdp0,level)
union td *tdp0;
{
	union td *tdp;
	struct formatd *fp;
	struct cased   *cp;
	struct switchd *swp;
	struct stringd *sp;
	struct macdefd *mp;
	int leftcolsv;

	for(tdp = tdp0; tdp; tdp = tdp->td_next) {
		if(icurrcol > 78)
			ltrace("\n");
		switch(tdp->td_type) {
		case IMACDEF:
			mp = (struct macdefd *)tdp;
			ltrace("[REG%d = %s] ",mp->m_name,exprstr(mp->m_expr));
			continue;
		case IREG:
			mp = (struct macdefd *)tdp;
			ltrace("????????????[REG%d]???????????? ",mp->m_name);
			continue;
		case ILEVEL:
		case IFORMAT:
			fp = (struct formatd *)tdp;
			ltrace("[%s] ",formatstr(fp));
			continue;
		case ILOOP:
			fp = (struct formatd *)tdp;
			ltrace("LOOP [%s] ( ",formatstr(fp));
			iprtree(fp->f_desc,level+1);
			ltrace(") ");
			continue;
		case ISWITCH:
			swp = (struct switchd *)tdp;
			cp = swp->s_case;
			if(cp->u_next || cp->c_desc) {
				ltrace("SWITCH(%s) {\n",exprstr(swp->s_expr));
				leftcolsv = ileftcol;
				ileftcol += CASEINDENT;
				iprtree(swp->s_case,level+1);
				ileftcol = leftcolsv;
				ltrace("} ");
			} else {
				ltrace(" IF (%s == ",exprstr(swp->s_expr));
				sp = cp->c_value_label;
				if(sp->u_type == IMATCH)
					ltrace("ANY) ");
				else
					ltrace("%s) ",sp->s_string);
				sp = (struct stringd *)sp->u_next;
				lprstring(sp);
			}
			continue;
		case ICASE:
			cp = (struct cased *)tdp;
			sp = cp->c_value_label;
			if(sp->u_type == IMATCH)
				ctrace("DEFAULT: ");
			else
				ctrace("CASE %s: ",sp->s_string);
			if(sp->u_next)
				lprstring(sp->u_next);
			if(cp->c_desc)
				iprtree(cp->c_desc,level+1);
			ctrace("\n");
			continue;
		case ISTRING:
		case ISTRING0:
			sp = (struct stringd *)tdp;
			while(sp) {
				switch(sp->s_string[0]) {
				case '\n': l0trace("\n"); break;
				case '\t': ltrace(TAB); break;
				default:   ltrace("%s ",sp->s_string); break;
				}
 				if(sp->u_next && sp->u_next->td_type != ISTRING)
					break;
				sp = (struct stringd *)sp->u_next;
			}
			tdp = (union td *)sp;
			continue;
		case IFUNCTION:
		  {
			struct functiond *fnp;
			extern char *fntoname();

			fnp = (struct functiond *)tdp;
			ltrace("function %s ( ",fntoname(fnp->fn_number));
			iprtree(fnp->fn_arglist,level+1);
			ltrace(")");
			continue;
		  }
		case IQDESCRP:
			if(tdp->td_next2)
				iprtree(tdp->td_next2,level+1);
			continue;
		default:
			ltrace("\nunknown type %d\n",tdp->td_type);
			continue;
		}
	}
}

static l0trace(s,a,b,c,d,e)
{

	itrace(0,s,a,b,c,d,e);
}

ltrace(s,a,b,c,d,e)
{

	itrace(ileftcol,s,a,b,c,d,e);
}

static ctrace(s,a,b,c,d,e)
{

	itrace(ileftcol-CASEINDENT,s,a,b,c,d,e);
}

static itrace(col,s,a,b,c,d,e)
{
	char *cp,*cpsave;
	char cc;
	char buf[128];

	sprintf(buf,(const char *) s,a,b,c,d,e);
	cp = cpsave = buf;
	while(cc = *cp++) {
		switch(cc) {
		case '\n':
			cp[-1] = 0;
			iitrace("%s\n",cpsave);
			cpsave = cp;
			indent(col);
			icurrcol = col;
			continue;
		}
		icurrcol++;
	}
	iitrace("%s",cpsave);
}

static indent(n)
{
	int len;
	char buf[34];

	len = n;
	if(len < 0)
		len = 0;
	if(len > 32)
		len = 32;
	memset(buf,' ',len);
	buf[len] = '\0';
	iitrace("%s",buf);
}

static iitrace(s,a,b,c,d,e)
{

	List(s,a,b,c,d,e);
}


#ifdef TRCRPT

char *bitflagsstr(flp)
struct flagsd *flp;
{
	static char buf[64];

	if(flp->fl_true->u_type != ISTRING) {
		sprintf(buf,"%08X  (-)",flp->fl_field->s_value);
		return(buf);
	}
	if(flp->u_type == IBITFLAGS)
		sprintf(buf,"%08X  (%.20s,%.20s)",
			flp->fl_field->s_value,
			flp->fl_true->s_string,
			flp->fl_false ? flp->fl_false->s_string : "-");
	else
		sprintf(buf,"%08X & %08X (%.20s)",
			flp->fl_field->s_value,
			flp->fl_mask->s_value,
			flp->fl_true->s_string);
	return(buf);
}


#endif

