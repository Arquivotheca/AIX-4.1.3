static char sccsid[] = "@(#)98	1.6  src/bos/usr/bin/sysdumpdev/opts.c, cmddump, bos411, 9428A410j 6/10/91 16:02:25";
/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: opts, setopts
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

#include <string.h>
#include "dmpfmt.h"

#define F_FIELDSIZE   1
#define F_NFIELDS     2
#define F_ALIGN       3
#define F_WRAPWIDTH   4
#define F_ASCII       5
#define F_COMPACT     6

static sethelp();

struct dmp_displayopts Opt = {
	4,    /* opt_fieldsize */
	4,    /* opt_nfields */
	16,   /* opt_align */
	0,    /* opt_wrapwidth */
	0,    /* opt_asciiflg */
	1     /* opt_compactflg */
};

struct dmp_displayopts Opt_dflt = {
	4,    /* opt_fieldsize */
	4,    /* opt_nfields */
	16,   /* opt_align */
	0,    /* opt_wrapwidth */
	0,    /* opt_asciiflg */
	1     /* opt_compactflg */
};

/*
 * NAME:     opts
 * FUNCTION: fill in the Opts display options structure
 * INPUTS:   name    Case-independent name of the option
 *           value   character string containing value for the option.
 * RETURNS:  none
 *
 * The display options can be either numeric (decimal) or boolean.
 */

/* 
 * Note that crash runs on top of this code and as per Elizabeth Francis (5/26/93)
 * there are no plans to internationalize crash.  Therefore prinfs/messages  in
 * this file will not be entered into message catalog and/or translated.
 */

opts(name,value)
char *name;
char *value;
{
	int field;
	int boolflg;
	unsigned int *p;
	unsigned int *dfltp;

	if(name == 0) {
		printf("\
fieldsize  %d\n\
nfields    %d\n\
align      %d\n\
wrapwidth  %d\n\
ascii      %s\n\
compact    %s\n",
			Opt.opt_fieldsize,
			Opt.opt_nfields,
			Opt.opt_align,
			Opt.opt_wrapwidth,
			Opt.opt_asciiflg ? "yes" : "no",
			Opt.opt_compactflg ? "yes" : "no");
		return;
	}
	if(streq(name,"-")) {
		Opt = Opt_dflt;
		return;
	}
	if(streq_cn(name,"fieldsize")) {
		p = &Opt.opt_fieldsize;
		dfltp = &Opt_dflt.opt_fieldsize;
		field = F_FIELDSIZE;
		goto op;
	}
	if(streq_cn(name,"nfields")) {
		p = &Opt.opt_nfields;
		dfltp = &Opt_dflt.opt_nfields;
		field = F_NFIELDS;
		goto op;
	}
	if(streq_cn(name,"align")) {
		p = &Opt.opt_align;
		dfltp = &Opt_dflt.opt_align;
		field = F_ALIGN;
		goto op;
	}
	if(streq_cn(name,"wrapwidth")) {
		p = &Opt.opt_wrapwidth;
		dfltp = &Opt_dflt.opt_wrapwidth;
		field = F_WRAPWIDTH;
		goto op;
	}
	if(streq_cn(name,"ascii")) {
		p = &Opt.opt_asciiflg;
		dfltp = &Opt_dflt.opt_asciiflg;
		field = F_ASCII;
		goto op;
	}
	if(streq_cn(name,"compact")) {
		p = &Opt.opt_compactflg;
		dfltp = &Opt_dflt.opt_compactflg;
		field = F_COMPACT;
		goto op;
	}
	if(streq(name,"?")) {
		sethelp();
		return;
	}
	printf("Bad set command: %s\n",name);
	return;
op:
	boolflg = 0;
	switch(field) {
	case F_ASCII:
	case F_COMPACT:
		boolflg++;
	}
	if(value == 0) {
		if(boolflg)
			printf("%s\n",*p ? "yes" : "no");
		else
			printf("%d\n",*p);
		return;
	}
	if(streq(value,"-")) {
		*p = *dfltp;
		return;
	}
	if(boolflg) {
		if(streq_cn(value,"no") || streq(value,"0"))
			*p = 0;
		else
			*p = 1;
	} else {
		if(numchk(value) && value[0] != '-')
			*p = atoi(value);
		else
			printf("Bad option '%s' for %s\n",value,field);
	}
}

/*
 * NAME:     setopts
 * FUNCTION: break command line argument of the form:
 *           name=value into a name and value and call opts();
 * INPUTS:   list    A character string of the form:
 *             "name=value, name=value, ..."
 * RETURNS:  none
 */
setopts(line)
char *line;
{
	char *cp;
	char *name,*value;

	while(cp = strtok(line,", \t\n")) {
		line = 0;
		name = cp;
		while(*cp && *cp != '=')
			cp++;
		if(*cp != '=')
			continue;
		*cp = '\0';
		value = cp+1;
		opts(name,value);
	}
}

/*
 * NAME:     sethelp
 * FUNCTION: Output help message to use 'set' subcommand.
 * INPUTS:   none
 * RETURNS:  none
 */
static sethelp()
{

	printf("\
set                    show current value of all options\n\
set -                  set all options to their default value\n\
set <option>           show current value of selected option\n\
set <option> -         set selected option its default value\n\
set <option> <value>   set selected option to <value>\n\
set <option> ?         show help message for selected option\n");
}

