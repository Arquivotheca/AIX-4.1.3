static char sccsid[] = "@(#)87  1.3  src/bos/usr/bin/errlg/libras/er.c, cmderrlg, bos411, 9431A411a 8/1/94 11:33:53";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: cat_eprint, cat_warn, cat_fatal, cat_scc
 *            cat_lwarn, cat_error, cat_lerror, cat_lfatal, yyerror
 *            vprint, cat_print, cat_string, catinit, errstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * error message print facility interface.
 *
 * The interface to each of the routines is the same.
 * INPUTS:  'msgnum'  message catalog number
 *           s         format string of no message exists
 *           a,b,...   printf parameters
 * RETURNS:  NONE
 *
 * The message set is encoded in 'msgnum':
 * If 'msgnum' is 0, bypass the message catalog lookup and print
 *   the parameters directly.
 * The message set is always 1.
 *
 * All output is to stderr.
 * Exceptions are:
 * cat_print     output is to stdout
 * cat_string    no output. Place poiter to string in supplied poitner.
 *
 * NAME:     cat_warn
 * FUNCTION: print a "warning" header and the message
 *
 * NAME:     cat_fatal
 * FUNCTION: print a "fatal" header, the message, then exit
 *
 * NAME:     cat_scc
 * FUNCTION: print a "self-consistency" header, the message, and exit.
 *           This routine is used like ASSERT in the kernel
 *
 * NAME:     cat_lwarn
 * FUNCTION: print a warning header with line number and the message
 *
 * NAME:     cat_error
 * FUNCTION: print a "error" header and the message.
 *           Increment Errflg.
 *
 * NAME:     cat_lerror
 * FUNCTION: print a "error" header with line number and the message
 *           Increment Errflg.
 *
 * NAME:     cat_lfatal
 * FUNCTION: print a "fatal" header with line number, the message, then exit
 *
 * NAME:     vprint
 * FUNCTION: print the message under conditional control of 'verboseflg'
 *
 * NAME:     cat_print
 * FUNCTION: print the message to stdout
 *
 * NAME:     cat_eprint
 * FUNCTION: print the message to stderr
 *
 * NAME:     cat_string
 * FUNCTION: return message in supplied string pointer.
 * INPUTS:   'msgnum'   message number
 *           'sp'       put string pointer in the contents of this pointer
 *           's'        alternate string if not in message catalog
 * NOTE:    This routine is used to supply translated words and message,
 *          such as yes, no, etc.
 *
 * NAME:     catinit
 * FUNCTION: open message catalogs through jcatopen and fill in Yes and No.
 *           This routine opens both the program's catalog and the
 *           separate catalog of raslib.a
 * INPUT:    'catalog'  Name of the program's catalog.
 *           jcatopen will try several paths. (See cat.c/jcatopen)           
 * RETURNS:  NONE
 *
 * NAME:     errstr
 * FUNCTION: return sys_errlist string as indexed by errno.
 * INPUT:    NONE
 * RETURNS:  character pointer to error string.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <libras.h>

int verboseflg;
int Lineno;
int Errflg;
char *Progname;
char *Infile;

extern errno;
extern char *sys_errlist[];

static yerrflg;
static char lastc;

static Catslot     = -1;
static Catslot_err = -1;

/* msg_scc is not translated; other vars are in dump, trace, and errlg catalogs */
static char *msg_scc     = "Internal Error";
char *msg_line;
char *msg_warning;
char *Yes;
char *No;

cat_warn(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s: %s:\n",Progname,msg_warning);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
}

cat_fatal(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s:\n",Progname);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
	genexit(1);
}

cat_scc(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s: %s:\n",Progname,msg_scc);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
	genexit(1);
}

cat_lwarn(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s: \"%s\", %s %d: %s:\n",
		Progname,Infile,msg_line,Lineno,msg_warning);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
}

cat_error(msgnum,s,a,b,c,d,e,f,g)
{

	cat_eprint(0,"%s:\n",Progname);
	cat_eprint(msgnum,s,a,b,c,d,e,f,g);
	nl();
	Errflg++;
}

cat_lerror(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s: \"%s\", %s %d:\n",
		Progname,Infile,msg_line,Lineno);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
	Errflg++;
}

cat_lfatal(msgnum,s,a,b,c,d,e)
{

	cat_eprint(0,"%s: \n\"%s\", %d:\n",
		Progname,Infile,msg_line,Lineno);
	cat_eprint(msgnum,s,a,b,c,d,e);
	nl();
	genexit(1);
}

yyerror(s,a,b,c,d,e)
{
char *translated_string;
char *translated_line;
			
	if(streq(s,"syntax error"))
		{
		cat_string(CAT_UPD_YACC_SYNTAX,&translated_string,s);
		cat_string(CAT_UPD_YACC_LINE,&translated_line,"line");
		eprint("%s: \"%s\", %s %d: ",Progname,Infile,translated_line,Lineno);
		eprint(translated_string,a,b,c,d,e);
		nl();
		yerrflg++;
		Errflg++;
		}
	else
		{
		if (streq(s,"syntax error - cannot backup"))	
			cat_string(CAT_UPD_YACC_BACKUP,&translated_string,s);	
		else
			cat_string(CAT_UPD_YACC_OFLW,&translated_string,s);
		eprint("%s: ",Progname);
		eprint(translated_string,a,b,c,d,e);
		nl();
		yerrflg++;
		Errflg++;
		}
}

vprint(s,a,b,c,d,e,f,g,h)
{

	if(s == 0)
		return;
	if(verboseflg)
		eprint(s,a,b,c,d,e,f,g,h);
	else
		Debug(s,a,b,c,d,e,f,g,h);
}

cat_print(msgnum,s,a,b,c,d,e,f,g,h)
char *s;
{
	char *ss;

	cat_string(msgnum,&ss,s);
	printf(ss,a,b,c,d,e,f,g,h);
}

cat_eprint(msgnum,s,a,b,c,d,e,f,g,h)
char *s;
{
	char *ss;

	cat_string(msgnum,&ss,s);
	eprint(ss,a,b,c,d,e,f,g,h);
}

cat_string(msgnum,sp,s)
char **sp;
char *s;
{
	char *cp;

	if(msgnum == 0) {
		*sp = s;
		return;
	}
	if(msgnum && Catslot >= 0) {
		cp = jcatgets(Catslot,msgnum);
		if(cp == 0)
			cp = s;
		*sp = cp;
	} else {
		*sp = s;
	}
}

eprint(s,a,b,c,d,e,f,g,h)
char *s;
{
	char buf[5120];

	if(yerrflg) {
		yerrflg = 0;
		_Debug("\n");
		fprintf(stderr,"\n");
		lastc = '\n';
	}
	if(s) {
		_Debug(s,a,b,c,d,e,f,g,h);
		sprintf(buf,s,a,b,c,d,e,f,g,h);
		lastc = buf[strlen(buf)-1];
		fputs(buf,stderr);
	}
}

static nl()
{

	if(lastc != '\n')
		cat_eprint(0,"\n");
}

/*
 * These defines correspond to messages in a .msg file
 */
#define cat_YES  512
#define cat_NO   513
#define cat_WARN 514
#define cat_LINE 515

catinit(catalog)
char *catalog;
{
	char *cp;

	Catslot  = jcatopen(catalog);		/* open main catalog */

	if(cp = jcatgets(Catslot,cat_YES))		/* fill in special words */
		Yes = stracpy(cp);
        else    Yes = stracpy("YES");
	if(cp = jcatgets(Catslot,cat_NO))
		No = stracpy(cp);
        else    No = stracpy("NO");
	if(cp = jcatgets(Catslot,cat_WARN))
		msg_warning = stracpy(cp);
        else    msg_warning = stracpy("Warning");
	if(cp = jcatgets(Catslot,cat_LINE))
		msg_line = stracpy(cp);
        else    msg_line = stracpy("Line");
	Catslot_err = jcatopen("libc.cat");		/* open errno catalog */
}

char *errstr()
{
	char *cp;

	if(errno < 0)
		return("?");
	if(Catslot_err) {
		cp = jcatgets(Catslot_err,errno);
		if(cp)
			return(cp);
	}
	return(sys_errlist[errno]);
}
