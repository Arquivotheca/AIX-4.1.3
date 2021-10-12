static char sccsid[] = "@(#)33  1.17  src/bos/usr/bin/errlg/errupdate/parseutil.c, cmderrlg, bos411, 9428A410j 3/31/94 17:05:53";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: ystanza, ymajor, yminors, yminor, ynumlist, ydetaillist
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file parallels the parse.y file.
 * Each function corresponds to a yacc action. The body of the actions
 *   has been moved into this utility file in order to make the grammar
 *   of the parse.y file easier to read.
 * The name of each utility routine is the name of the yacc rule (see parse.y)
 *   preceded by a 'y'.
 * The main job of these routines is to allocate and fill in 'symbol' entries
 *   on the yacc stack.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/err_rec.h>
#include <errupdate.h>
#include <parse.h>

static char *v();

ystanza(maj,min)
symbol *maj,*min;
{

	if(Errflg)
		return;
	if(maj != Major) {
		genexit_nostats(1);
		}
	maj->s_next = min;
}

symbol *ymajor(op,maj)
symbol *maj;
{

	maj->s_type = op;
	Major[0] = *maj;
	return(Major);
}

symbol *yminors(min0,min)
symbol *min0,*min;
{

	if(min0)
		append(min0,min);
	else
		min0 = min;
	return(min0);
}

#define S(NAME,VALUE) { "NAME",VALUE }
struct nt {
	char *nt_name;
	int   nt_value;
};
struct nt nt_class[] = {
	S(H,ERRCLASS_HARDWARE),
	S(S,ERRCLASS_SOFTWARE),
	S(O,ERRCLASS_OPERATOR),
	S(U,ERRCLASS_UNKNOWN),
	0
};
struct nt nt_type[] = {
	S(PERM,ERRTYPE_PERM),
	S(TEMP,ERRTYPE_TEMP),
	S(PERF,ERRTYPE_PERF),
	S(PEND,ERRTYPE_PEND),
	S(UNKN,ERRTYPE_UNKN),
	S(INFO,ERRTYPE_INFO),
	0
};
struct nt nt_truefalse[] = {
	S(TRUE, ERR_TRUE),
	S(FALSE,ERR_FALSE),
	0
};
struct nt nt_detail[] = {
	S(HEX,    'H'),
	S(ALPHA,  'A'),
	S(DEC,    'D'),
	S(DECIMAL,'D'),
	0
};

symbol *yminor(keyword,rvalue)
symbol *keyword;
symbol *rvalue;
{
	int tn;
	symbol *sp;
	struct nt *ntp;

	switch(keyword->s_type) {
	case IERRCLASS:
	case IERRTYPE:
		switch(keyword->s_type) {
		case IERRCLASS: ntp = nt_class; break;
		case IERRTYPE:  ntp = nt_type;  break;
		}
		ntmsg(ntp,rvalue->s_string,keyword->s_name);
		if((tn = ntsearch(ntp,rvalue->s_string)) < 0)
		{
			if (keyword->s_type == IERRCLASS)
			{
				cat_eprint (CAT_UPD_POSS_CLASS, "S\nO\nH\nU\n");
				return(0);
			}
			else
			{
				cat_eprint (CAT_UPD_POSS_TYPE, "PERM\nTEMP\nPERF\nPEND\nUNKN\nINFO\n");
				return(0);
			}
		}	
		break;
	case IREPORT:
	case ILOG:
	case IALERT:
		ntmsg(nt_truefalse,rvalue->s_string,keyword->s_name);
		if((tn = ntsearch(nt_truefalse,rvalue->s_string)) < 0)
		{
			cat_eprint(CAT_UPD_POSS_ALERT, "TRUE\nFALSE \n");
			return(0);
		}
		rvalue->s_number = tn;
		break;
	case ICOMMENT:
		break;
	case IERRDESC:
		rangechk(rvalue->s_number);
		break;
	case IPROBCAUS:
	case IUSERCAUS:
	case IUSERACTN:
	case IINSTCAUS:
	case IINSTACTN:
	case IFAILCAUS:
	case IFAILACTN:
	case IDETAILDT:
		rangechk(rvalue->s_number);
		sp = getsym();
		sp->s_symp = rvalue;
		rvalue = sp;
		break;
	default:
		genexit_nostats(1);
	}
	rvalue->s_type = keyword->s_type;
	return(rvalue);
}

symbol *ynumlist(list0,list)
symbol *list0,*list;
{

	if(list0)
		append(list0,list);
	else
		list0 = list;
	return(list0);
}

symbol *ydetaillist(a,b,c)
symbol *a,*b,*c;
{
	int tn;

	detailflg = 0;
	if(numchk(a->s_string,10) == 10)
		a->s_number = atoi(a->s_string);
	a->s_next = b;
	b->s_next = c;
	c->s_next = 0;
	ntmsg(nt_detail,c->s_string,"detail");
	if((tn = ntsearch(nt_detail,c->s_string)) < 0)
		{
		cat_eprint(CAT_UPD_POSS_ENCODE,"ALPHA\nDEC\nHEX\n");
		return(0);
		}
	c->s_number = tn;
	return(a);
}

static append(s0,s)
symbol *s0,*s;
{

	if(s0) {
		while(s0->s_next)
			s0 = s0->s_next;
		s0->s_next = s;
	}
}

static ntsearch(ntp,name)
struct nt *ntp;
char *name;
{
	char buf[32];
	int i;

	for(i = 0; i < sizeof(buf); i++) {
		buf[i] = islower(name[i]) ? _toupper(name[i]) : name[i];
		if(name[i] == '\0')
			break;
	}
	buf[i] = '\0';
	while(ntp->nt_name) {
		if(streq(ntp->nt_name,buf)) {
			return(ntp->nt_value);
		}
		ntp++;
	}
	return(-1);
}

static char *v(sp)
symbol *sp;
{
	static char buf[32];

	switch(sp->s_type) {
	case INUMBER:
		sprintf(buf,"%x",sp->s_number);
		break;
	case ISTRING:
		sprintf(buf,"%-.32s",sp->s_string);
		break;
	default:
		sprintf(buf,"%s",tokstr(sp->s_type));
		break;
	}
	return(buf);
}

static rangechk(value)
{

	if(value < 0 || value > 0xFFFF)
		cat_lerror(CAT_UPD_PP2,
			"Message identifier 0x%04X is out of range (0000 - FFFF).\n", value); 
}

static ntmsg(ntp0,value,keyword)
struct nt *ntp0;
char *value;
char *keyword;
{
	struct nt *ntp;

	if(ntsearch(ntp0,value) >= 0)
		return;
	cat_lerror(CAT_UPD_PP1,"\
Bad value '%s' in the template for %s.\n\
Acceptable values are:\n",
		value,keyword);
}

