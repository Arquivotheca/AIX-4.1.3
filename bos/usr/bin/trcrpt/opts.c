static char sccsid[] = "@(#)15        1.12  src/bos/usr/bin/trcrpt/opts.c, cmdtrace, bos411, 9431A411a 7/26/94 07:45:46";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: opts, setopts
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
 * NAME:     opts
 * FUNCTION: fill in the Opts display options structure
 * INPUTS:   name    Case-independent name of the option
 *           value   character string containing value for the option.
 * RETURNS:  none
 *
 * The display options can be either numeric (decimal) or boolean.
 *
 *
 * NAME:     setopts
 * FUNCTION: break command line argument of the form:
 *           name=value into a name and value and call opts();
 * INPUTS:   list    A character string of the form:
 *             "name=value, name=value, ..."
 * RETURNS:  none
 */

#include <string.h>
#include <ctype.h>
#include "rpt.h"

struct displayopts Opts = {
	0,   /* opt_timestamp */
	0,   /* opt_pagesize */
	0,   /* opt_microsecflg */
	1,   /* opt_idsflg */
	0,   /* opt_execflg */
	0,   /* opt_pidflg */
	0,   /* opt_svcflg */
	0,   /* opt_compactflg */
	0,   /* opt_fileflg */
	0,   /* opt_2lineflg */
	0,   /* opt_starttime */
	0,   /* opt_endtime */
	0,   /* opt_histflg */
	0,   /* opt_tidflg */
	0,   /* opt_cpuidflg */
};


#ifdef TRCRPT

#define P(member) ( &Opts.member )

static char hp_timestamp[] =
	"0=elapsed+delta, 1=short_elapsed, 2=microseconds, 3=none";

static char hp_pagesize[] =
	"0-500  0=no page breaks,  n=lines between column headers";

static char hp_2lineflg[] =
	"on=use two lines per event, off=use one line";

static char hp_microsec[] =
	"on=delta seconds in microseconds, off=milliseconds";

static char hp_ids[] =
	"on=print trace id, off=none";

static char hp_exec[] =
	"on=print exec pathnames, off=none";

static char hp_pid[] =
	"on=print process id, off=none";

static char hp_svc[] =
	"on=print current svc, off=none";

static char hp_file[] =
	"on=print current file, off=none";

static char hp_compact[] =
	"on=filter repeated sequences of events, off=none";

static char hp_start[] =
	"starting time in seconds";

static char hp_end[] =
	"ending time in seconds";

static char hp_histflg[] =
	"on=generate a histogram of hook";

static char hp_tid[] =
	"on=print thread id, off=none";

static char hp_cpuid[] =
	"on=print cpu id, off=none";

#define L_BOOLEAN 1
#define L_NUMERIC 2
#define L_NTIME   3

static struct l {
	char *l_name;
	int   l_fieldcode;
	int   l_dflt;
	int  *l_member;
	char *l_help;
} l[] = {
	{ "timestamp", L_NUMERIC, 0, P(opt_timestamp),       hp_timestamp },
	{ "pagesize",  L_NUMERIC, 0, P(opt_pagesize),        hp_pagesize },
	{ "ids",       L_BOOLEAN, 1, P(opt_idflg),           hp_ids },
	{ "exec",      L_BOOLEAN, 0, P(opt_execflg),         hp_exec },
	{ "pid",       L_BOOLEAN, 0, P(opt_pidflg),          hp_pid },
	{ "svc",       L_BOOLEAN, 0, P(opt_svcflg),          hp_svc },
	{ "2line",     L_BOOLEAN, 0, P(opt_2lineflg),        hp_2lineflg },
	{ "starttime", L_NTIME,   0, (int *)&Opts.opt_startp,hp_start },
	{ "endtime",   L_NTIME,   0, (int *)&Opts.opt_endp,  hp_end },
	{ "hist",      L_BOOLEAN, 0, P(opt_histflg),         hp_histflg },
	{ "tid",       L_BOOLEAN, 0, P(opt_tidflg),          hp_tid },
	{ "cpuid",     L_BOOLEAN, 0, P(opt_cpuidflg),        hp_cpuid },
	{ 0, }
};

/* { "compact",   L_BOOLEAN, 0, P(opt_compactflg),      hp_compact }, */

static char *dflt_str();
static char *help_str();

opts(name,value)
char *name;
char *value;
{
	struct l *lp;

	if(name == 0)
		return;
	Debug("opts(%s,%s)\n",name,value);
	for(lp = l; lp->l_name; lp++)
		if(streq_cn(name,lp->l_name))
			break;
	if(lp->l_name == 0)
		opts_usage();
	switch(lp->l_fieldcode) {
	case L_NTIME:  		/* numeric value */
	  {
		char *secs,*nsecs;
		int *ip;
		int n;

		secs = nsecs = 0;
		if(value[0] == '.') {
			secs  = "0";
			nsecs = strtok(value,".");
		} else {
			secs  = strtok(value,".");
			nsecs = strtok(0,".");
		}
		if(secs == 0 && nsecs == 0)
			return;
		if(secs == 0)
			secs = "0";
		if(nsecs == 0)
			nsecs = "0";
		ip = (int *)jalloc(8);
		*lp->l_member = (int)ip;
		ip[0] = strtol(secs,0,10);
		n = strlen(nsecs);
		ip[1] = strtol(nsecs,0,10) * pow(10,9-n);
		Debug("NTIME: %d.%d\n",ip[0],ip[1]);
		break;
	  }
	case L_NUMERIC:		/* numeric value */
		*lp->l_member = strtol(value,0,0);
		break;
	case L_BOOLEAN:		/* boolean */
		*lp->l_member = (streq_cn(value,"off") || streq_cn(value,"no") || streq(value,"0")) ? 0 : 1;
		break;
	}
}

static char *dflt_str(lp)
struct l *lp;
{
	static char buf[16];

	switch(lp->l_fieldcode) {
	case L_NUMERIC:		/* numeric value */
		sprintf(buf,"%d",lp->l_dflt);
		return(buf);
	case L_BOOLEAN:		/* boolean */
		sprintf(buf,"%s",lp->l_dflt ? "on" : "off");
		return(buf);
	default:
		return("-");
	}
}

static char *help_str(lp)
struct l *lp;
{

	return(lp->l_help ? lp->l_help : "");
}

setopts(line)
char *line;
{
	char *cp;
	char *name,*value;
	char *local_line;

	Debug("setopts : line %s \n", line);
	local_line = malloc(strlen(line) + 1);
	if (local_line == 0) {
                perror("malloc");
                genexit(1);
        }
        else {
                strcpy(local_line, line);
        }
	Debug("setopts : local_line %s \n", local_line);


	if(strncmp(local_line,"help",4) == 0)
		opts_usage();
	while(cp = strtok(local_line,", \t\n")) {
		local_line = cp + strlen(cp) + 1; /* avoid interaction with strtok in opts (pb when NL_TIMES) */
		Debug("setopts : cp %s \n", cp);
		name = cp;
		while(*cp && *cp != '=')
			cp++;
		Debug("setopts : cp2 %s \n", cp);
		if(*cp != '=')
			continue;
		*cp = '\0';
		value = cp+1;
		Debug("setopts : value %s \n", value);
		opts(name,value);
	}
}

static opts_usage()
{
	cat_eprint(CAT_TPT_USAGE4,"\
timestamp   0    0=elapsed+delta, 1=short_elapsed, 2=microseconds, 3=none\n\
pagesize    0    0-500  0=no page breaks,  n=lines between column headers\n\
ids         on   on=print trace id, off=none\n\
exec        off  on=print exec pathnames, off=none\n\
pid         off  on=print process id, off=none\n\
svc         off  on=print current svc, off=none\n\
2line       off  on=use two lines per event, off=use one line\n\
starttime   -    starting time in seconds\n\
endtime     -    ending time in seconds\n\
hist        off  on=generate a histogram of hook\n\
tid         off  on=print thread id, off=none\n\
cpuid       off  on=print cpu id, off=none\n");

	genexit(1);
}

static pow(n,exp)
{
	int m;
	int i;

	m = 1;
	if(exp <= 1)
		return(1);
	if(exp > 9)
		exp = 9;
	for(i = 0; i < exp; i++)
		m *= n;
	return(m);
}


#endif

