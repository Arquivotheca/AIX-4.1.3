static char sccsid[] = "@(#)87	1.16.1.4  src/bos/usr/bin/trcrpt/parseutil.c, cmdtrace, bos411, 9428A410j 7/4/94 10:19:59";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: ytemplate, ydescriptor, yarg, yloop, yexpr, ymacdef
 *            yswitch, ystringlist, ycaselist, ycase, ymatchvalue
 *            ynewdesc, yoptlvl, yqdescrp, yfunction, yarglist
 *            ybitflags, yflaglist, yflagentry
 *
 * ORIGINS: 27
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
 * tree-management routines called by parse.y
 * This file is an extension of parse.y.
 * The routines are separated from parse.y so that the grammar in
 *   parse.y is easier to see.
 *                                                                    
 */

#include <ctype.h>
#include "rpt.h"
#include "td.h"
#include "parse.h"
extern verboseflg;

/*
 * A template has been assembled.
 * Place the head in Tdp0.
 * pass1() will save it in the Tindexes[] structure.
 */
ytemplate(n,tdp0,sp,tdp)
union td *tdp0;
struct stringd *sp;
union td *tdp;
{

	Debug("---ytemplate(%d,%x,%x,%x)\n",n,tdp0,sp,tdp);
	Tdp0 = tdp0;						/* communicate to gettmplt */
	Tdp0->td_next = (union td *)sp;		/* ISTRING0 */
	sp->u_next = tdp;					/* descriptor */
}

/*
 * A descriptor has been recognized.
 * Append to the previous descriptor and return the head of the descriptor.
 */
union td *ydescriptor(n,tdp0,tdp)
union td *tdp0;
union td *tdp;
{

	Debug("ydescriptor(%d,%x,%x) type=%s\n",n,tdp0,tdp,tokstr(tdp->td_type));
	if(tdp == 0)
		return(tdp0);
	append(tdp0,tdp);
	return(tdp0 ? tdp0 : tdp);
}

/*
 * If a REGFMT,  alter the default formatting.
 */
union td *yarg(n,tdp,xfp)
union td *tdp;
struct formatd *xfp;
{
	struct formatd *fp;
	struct stringd *sp;
	int base;

	Debug("yarg(%d,%x,%x) '%s'\n",n,tdp,xfp,tokstr(tdp->td_type));
	switch(tdp->td_type) {
	case ISTRING:
	case ILEVEL:
	case IFUNCTION:
		break;
	case IFORMAT:		/* handle IREGFMT IFORMAT */
		fp = (struct formatd *)tdp;
		if(xfp && (fp->f_fmtcode == '$' || fp->f_fmtcode == 'r')) {
			fp->f_xfmtcode = xfp->f_fmtcode;
			fp->f_fld1     = xfp->f_fld1;
			fp->f_fld2     = xfp->f_fld2;
			Debug("yarg: register %d. code '%c' xcode '%c' %d.%d\n",
				fp->f_val,fp->f_xfmtcode,fp->f_fmtcode,fp->f_fld1,fp->f_fld2);
		}
		break;
	default:
		Debug("yarg: unknown type %d\n",tdp->td_type);
	}
	return(tdp);
}

/*
 * The LOOP is a special case. It is the only format code that
 *   has a descriptor.
 */
struct formatd *yloop(n,fp,tdp)
struct formatd *fp;
union td *tdp;
{

	Debug("yloop(%d,%x,%x) format=%s\n",n,fp,tdp,formatstr(fp));
	switch(fp->f_fmtcode) {
	case 'H':
	case 'D':
	case 'X':
	case '$':
	case 'r':
		break;
	default:
		cat_lerror(CAT_TPT_FMTCODE,
"The format code %c not allowed for the LOOP statement.\n",fp->f_fmtcode);
	}
	fp->u_type = ILOOP;
	fp->f_desc = tdp;
	return(fp);
}

/*
 * Does not handle chains of expressions.
 */
struct exprd *yexpr(n,leftp,op,rightp)
union td *leftp;
union td *rightp;
{
	struct exprd *ep;

	Debug("yexpr(%d,%x,%c,%x)\n",n,leftp,op ? op : '=',rightp);
	ep = (struct exprd *)gettd();
	ep->u_type  = IEXPR;
	ep->e_op    = op;
	ep->e_left  = leftp;
	ep->e_right = rightp;
	return(ep);
}

/*
 * a macro definition is a register number (fp->f_val) '=' expression.
 */
struct macdefd *ymacdef(n,fp,ep)
struct formatd *fp;
struct exprd *ep;
{
	struct macdefd *mp;

	mp = (struct macdefd *)gettd();
	Debug("ymacdef(%d,%x,%x) format='%s'\n",n,fp,ep,formatstr(fp));
	mp->u_type = IMACDEF;	/* IREG -> IMACDEF */
	mp->m_name = fp->f_val;
	mp->m_code = fp->f_fmtcode;
	mp->m_expr = ep;
	return(mp);
}

/*
 * Start of a SWITCH.
 * Combine the match value (fp) with the chain of CASE's (cp)
 */
struct switchd *yswitch(n,ep,cp)
struct exprd *ep;
struct cased *cp;
{
	struct switchd *swp;
	struct stringd *msp;
	int base;

	Debug("yswitch(%d,%x,%x) format=%s\n",n,ep,cp,exprstr(ep));
	swp = (struct switchd *)gettd();
	swp->u_type = ISWITCH;
	swp->s_expr = ep;
	swp->s_case = cp;
	if(base = formatbase(ep->e_left)) {
		while(cp) {
			msp = cp->c_value_label;
			msp->s_value = strtoul(msp->s_string,0,base);
			Debug("convert '%s' to 0x%x\n",msp->s_string,msp->s_value);
			cp = (struct cased *)cp->u_next;
		}
	}
	return(swp);
}

static formatbase(fp)
struct formatd *fp;
{
	int code;
	int base;

	if(fp->u_type != IFORMAT)
		return(0);
	switch(fp->f_fmtcode) {
	case 'r':
	case '$':
		code = fp->f_xfmtcode;
		break;
	default:
		code = fp->f_fmtcode;
		break;
	}
	base = 0;
	switch(code) {
	case 'B':
		base = 2;
		break;
	case 'D':
	case 'U':
		base = 10;
		break;
	case 'o':
		base = 8;
		break;
	case 'S':
	case 'A':
		break;
	default:
		base = 16;
	}
	return(base);
}

struct stringd *ystringlist(n,sp0,sp)
struct stringd *sp0;
struct stringd *sp;
{

	Debug("ystringlist(%d,%x,%x)\n",n,sp0,sp);
	append(sp0,sp);
	return(sp0 ? sp0 : sp);
}

struct cased *ycaselist(n,cp0,cp)
struct cased *cp0;
struct cased *cp;
{

	Debug("yswitchbody(%d,%x,%x)\n",n,cp0,cp);
	append(cp0,cp);
	return(cp0 ? cp0 : cp);
}

struct cased *ycase(n,msp,sp,tdp)
struct stringd *msp;
struct stringd *sp;
union td *tdp;
{
	struct cased *cp;

	Debug("ycase(%d,%x,%x,%x) mv='%s'\n",n,msp,sp,tdp,msp->s_string);
	cp = (struct cased *)gettd();
	cp->u_type = ICASE;
	msp->u_next = (union td *)sp;	/* matchvalue + matchlabel */
	cp->c_value_label = msp;
	cp->c_desc = tdp;
	return(cp);
}

/*
 * Try tp convert the match value to a numeric quantity
 */
struct stringd *ymatchvalue(n,msp)
struct stringd *msp;
{
	union {
		unsigned u;
		int i;
	} ui;

	Debug("ymatchvalue(%d,%x) '%s'\n",n,msp,msp->s_string);
	return(msp);
}

union td *ynewdesc(n,tdp)
union td *tdp;
{

	Debug("ynewdesc(%d,%x)\n",n,tdp);
	return(tdp);
}

/*
 * Place the td at the END of the branch.
 */
static append(tdp0,tdp)
union td *tdp0;
union td *tdp;
{

	if(tdp0) {
		while(tdp0->td_next)
			tdp0 = tdp0->td_next;
		tdp0->td_next = tdp;
	}
}

/*
 * Indentation level
 */
union td *yoptlvl(n,fp)
struct formatd *fp;
{

	Debug("---yoptlvl(%d,%x)\n",n,fp);
	if(fp == 0) {
		fp = (struct formatd *)gettd();
		fp->u_type = ILEVEL;
	} 
	return((union td *)fp);
}

union td *yqdescrp(n,tdp)
union td *tdp;
{
	union td *tdp0;

	if(tdp == 0)
		return(0);
	tdp0 = gettd();
	tdp0->td_type  = IQDESCRP;
	tdp0->td_next2 = tdp;
	return(tdp0);
}

struct functiond *yfunction(n,sp,arglist)
struct stringd *sp;
union td *arglist;
{
	struct functiond *fnp;
	char *name;

	name = sp->s_string;
	fnp = (struct functiond *)gettd();
	fnp->u_type   = IFUNCTION;
	fnp->fn_number = flookup(name);
/*
	if(fnp->fn_number < 0 && verboseflg)
		Debug("undefined function '%s'\n",name);
*/
	if(fnp->fn_number < 0) {
#ifdef TRCRPT
	if (checkflag)
		cat_lwarn(CAT_TPT_FMTFUNC,
		"The function %s is not defined.\n",name);
#endif
		fnp->fn_number = flookup("doesnothing");
		fnp->fn_arglist = 0;
		return(fnp);
	}
	fnp->fn_arglist = arglist;
	Debug("yfunction(%d,%s) number=%d\n",n,name,fnp->fn_number);
	prarglist(arglist);
	return(fnp);
}

static prarglist(arglist)
union td *arglist;
{
	union td *tdp;
	struct formatd *fp;
	struct stringd *sp;

	if(!Debugflg)
		return;
	tdp = arglist;
	if(tdp == 0)
		return;
	for(; tdp; tdp = tdp->td_next) {
		sp = (struct stringd *)tdp;
		fp = (struct formatd *)tdp;
		switch(tdp->td_type) {
		case ISTRING:
			Debug("   ISTRING:   %x\n",sp->s_string);
			break;
		case IFORMAT:
			Debug("   IFORMAT: %s\n",formatstr(fp));
			break;
		default:
			Debug("   type=%d\n",tdp->td_type);
		}
	}
}

union td *yarglist(n,tdp0,tdp)
union td *tdp0,*tdp;
{

	Debug("yarglist(%d,%x,%x)\n",n,tdp0,tdp);
	append(tdp0,tdp);
	return(tdp0 ? tdp0 : tdp);
}

struct switchd *ybitflags(n,ep,flp)
struct exprd *ep;
struct flagsd *flp;
{
	struct switchd *swp;
	int base;

	Debug("ybitflags(%d,%x,%x) expr=%s\n",n,ep,flp,exprstr(ep));
	swp = (struct switchd *)gettd();
	swp->u_type = IBITFLAGS;
	swp->s_expr = ep;
	swp->s_case = (struct cased *)flp;
	if((base = formatbase(ep->e_left)) == 0)
		base = 16;
	while(flp) {
		flp->fl_field->s_value = strtol(flp->fl_field->s_string,0,base);
		if(flp->u_type == IBITFIELD)
			flp->fl_mask->s_value = strtol(flp->fl_mask->s_string,0,base);
		flp = (struct flagsd *)flp->u_next;
	}
	return(swp);
}

struct flagsd *yflaglist(n,flp0,flp)
struct flagsd *flp0,*flp;
{

	append(flp0,flp);
	return(flp0 ? flp0 : flp);
}

struct flagsd *yflagentry(n,maskp,fieldp,truep,falsep)
struct stringd *maskp,*fieldp,*truep,*falsep;
{
	struct flagsd *fp;

	Debug("yflagentry(%d,%s,%s,%s)\n",
		n,
		fieldp->s_string,
		truep->s_string,
		falsep->s_string ? falsep->s_string : "");
	fp = (struct flagsd *)gettd();
	fp->fl_field = fieldp;
	fp->fl_true  = truep;
	if(maskp) {
		fp->u_type   = IBITFIELD;
		fp->fl_mask  = maskp;
	} else {
		fp->u_type   = IBITFLAGS;
		fp->fl_false = falsep;
	}
	return(fp);
}

