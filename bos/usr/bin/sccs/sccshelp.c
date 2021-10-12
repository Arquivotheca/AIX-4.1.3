static char sccsid[] = "@(#)01 1.17 src/bos/usr/bin/sccs/sccshelp.c, cmdsccs, bos41B, 9504A 12/9/94 10:00:06";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: ask, clean_up, findprt, lochelp, main
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include 	<locale.h>
#include	"defines.h"
#include 	"sccshelp_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_SCCSHELP, Num, Str)

/*
     Program to locate helpful info in an ascii file.
     The program accepts a variable number of arguments.
*/

/*
     If MSG is defined then the help message file is assumed to be
     a message catalog.  The alphabetic portion of the argument is
     used as the message catalog name and a catopen is done on the 
     file to open it.  NLSPATH is assumed to be set up to find the
     specified message catalog.  If the catopen fails, meaning that
     the file could not be found then the normal help procssing is 
     done (see below).  If the catopen is successful then the numeric
     portion of the argument is used as a message set identifier (after
     changing 0 to 49, to account for set 0 not supported), and
     all messages in the message set are displayed.  Note that the
     messages in the message set must be contiguous starting at
     the value MSGSTART because the printing of messages from the message
     set terminates on the first failure of catgets.  If the argument
     has no numeric portion then all messages in message set 49 are
     displayed.
*/

/*
     The file to be searched is determined from the argument. If the
     argument does not contain numerics, the search
     will be attempted on '/usr/share/lib/sccshelp/cmds', with the search key
     being the whole argument.
     If the argument begins with non-numerics but contains numerics (e.g. zz32)
     the file /usr/share/lib/sccshelp/helploc will be checked for a
     file corresponding to the non numeric prefix, that file will then
     be searched for the message.  If /usr/share/lib/sccshelp/helploc
     does not exist or the prefix is not found there the search will
     be attempted on '/usr/share/lib/sccshelp/<non-numeric prefix>',
     (e.g,/usr/share/lib/sccshelp/zz), with the search key being
     <remainder of arg>, (e.g. 32).
     If the argument is all numeric, or if the file as
     determined above does not exist, the search will be attempted on
     '/usr/share/lib/sccshelp/default' with the search key being
     the entire argument.
     In no case will more than one search per argument be performed.

     File is formatted as follows:

		* comment
		* comment
		-str1
		text
		-str2
		text
		* comment
		text
		-str3
		text

	The "str?" that matches the key is found and
	the following text lines are printed.
	Comments are ignored.

	If the argument is omitted, the program requests it.
*/
#define HELPLOC "/usr/share/lib/sccshelp/helploc"
static struct stat Statbuf;
static char ErrMsg[512];

static char	dftfile[]   =   "/usr/share/lib/sccshelp/default";
static char	helpdir[]   =   "/usr/share/lib/sccshelp/";
static char	hfile[64];
char	*repl();
static FILE	*iop, *fdfopen();
static char	line [MAXLINE];

#define MSGSTART 1
#define SETSTART 49
#define MSGDEFAULT "default"
static char *longnames[10][2] ={ { "prs_kywds", "prskwd" },
				{ "sccsdiff", "sccsdf" },
				{ "ad", "admin" },
				{ "bd", "bdiff" },
				{ "cb", "comb" },
				{ "de", "delta" },
				{ "he", "" },
				{ "ge", "get" },
				{ "rc", "rmdel" },
				{ "sccshelp", "help" }
			      };
#define NNAMES 10

nl_catd catd;


main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	extern int Fcnt;
	char **resp;
	char **ask();

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);


	/*
	Tell 'fatal' to issue messages and return to its caller.
	*/
	Fflags = FTLMSG | FTLJMP;

	if (argc == 1) {
		resp = ask();
		for(;**resp; *resp++)
			findprt(*resp);
	}
	else
		for (i = 1; i < argc; i++)
			findprt(argv[i]);

	exit(Fcnt ? 1 : 0);
}


static findprt(p)
char *p;
{
	register char *q;
	char key[150];
	char *strcpy();
	int msgset;
	nl_catd msgd;
	int i;
	char *msg;

	if (setjmp(Fjmp))		/* set up to return here from */
		return;			/* 'fatal' and return to 'main' */
	if (size(p) > 50)
		fatal(MSGSTR(ARGTOOLNG, 
		  "\nCommand parameter cannot exceed 50 characters. (he2)\n"));


	strcpy(hfile, "h.");
	q = p;

	while (*q && !NUMERIC((int)(*q)))
		q++;

	if (*q == '\0') {			/* no number; only name */
		for ( i = 0; i < NNAMES; i++) {
			if (!strcmp(p,longnames[i][0])) {
				strcat(hfile,longnames[i][1]);
				break;
			}
		}
		if (i == NNAMES)
			strcat(hfile,p);
		msgset = SETSTART;
	}
	else
		if (q == p) {
			sscanf(q,"%d",&msgset);	/* get the set # */
			if (!msgset) msgset = SETSTART;
			strcat(hfile, MSGDEFAULT);
		}
		else {
			strcat(hfile,p);
			*(hfile + (q-p) + 2) = '\0';
			sscanf(q,"%d",&msgset);
			if (!msgset) msgset = SETSTART;
		}
	strcat(hfile,".cat");
	if ((int)(msgd = catopen(hfile, NL_CAT_LOCALE)) != -1) {
		for (i=MSGSTART; ;i++) {
			msg = catgets(msgd,msgset,i,"");
			if (i == MSGSTART && strcmp(msg, "") == 0)
				goto go_on;	/* try normal mechanism */
			else if (i == MSGSTART)
				printf("\n%s:\n",p);
			else if (strcmp(msg, "") == 0)
				break;
			printf("%s\n",msg);
		}
		catclose(msgd);
		return;
	}
go_on:

	q = p;

	while (*q && !NUMERIC((int)(*q)))
		q++;

	if (*q == '\0') {		/* all alphabetics */
		strcpy(key,p);
		sprintf(hfile,"%s%s",helpdir,"cmds");
		if (!exists(hfile))
			strcpy(hfile,dftfile);
	}
	else
		if (q == p) {		/* first char numeric */
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	else {				/* first char alpha, then numeric */
		strcpy(key,p);		/* key used as temporary */
		*(key + (q - p)) = '\0';
		if(!lochelp(key,hfile))
			sprintf(hfile,"%s%s",helpdir,key);
		else
			cat(hfile,hfile,"/",key,0);
		strcpy(key,q);
		if (!exists(hfile)) {
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	}

	iop = (FILE *) xfopen(hfile,0);
	Fflags |= FTLCLN;    /* now add cleanup to fatal's instructions */

	/*
	Now read file, looking for key.
	*/
	while ((q = fgets(line,MAXLINE,iop)) != NULL) {
		repl(line,'\n','\0');		/* replace newline char */
		if (line[0] == '-' && equal(&line[1],key))
			break;
	}

	if (q == NULL) {	/* endfile? */
		printf("\n");
		sprintf(ErrMsg,MSGSTR(NOTFOUND, "\n%s is not a valid parameter.\n\
\tSpecify a valid command or error code. (he1)\n"),p);
		fatal(ErrMsg);
	}

	printf("\n%s:\n",p);

	while (fgets(line,MAXLINE,iop) != NULL && line[0] == '-')
		;
	do {
		if (line[0] != '*')
			printf("%s",line);
	} while (fgets(line,MAXLINE,iop) != NULL && line[0] != '-');

	fclose(iop);
}


static char **
ask()
{
	static char resp[51];
	static char *resp_token[MAXLINE];
	char *ptr, *q, *cur_ptr;
	int  num_token;

	iop = stdin;

	printf(MSGSTR(MSGNOCMDNM, "Provide message number or command name.\n"));
	fgets(resp,51,iop);
	repl(resp, '\n', '\0');
	q = resp;
	cur_ptr = resp;
	for(num_token = 0; num_token < MAXLINE;){
		if(*q == ' ') {
			ptr = (char *)malloc((q - cur_ptr)+1);
			strncpy(ptr, cur_ptr, (q - cur_ptr));
			*(ptr + ((q - cur_ptr)+1)) = '\0';
			resp_token[num_token] = ptr;
			cur_ptr = ++q;
			num_token++;
		}
		else if(*q == '\0') {
			ptr = (char *)malloc((q - cur_ptr)+1);
			strncpy(ptr, cur_ptr, (q - cur_ptr));
			*(ptr + ((q - cur_ptr)+1)) = '\0';
			resp_token[num_token] = ptr;
			num_token++;
			break;
		}

		else
			q++;
	}
	resp_token[num_token] = NULL;
	return(resp_token);

}

/* lochelp finds the file which cojntains the help messages 
if none found returns 0
*/
static lochelp(ky,fi)
	char *ky,*fi; /*ky is key  fi is found file name */
{
	FILE *fp;
	char locfile[513];
	char *hold;
	extern char *strtok();

	if(!(fp = fopen(HELPLOC,"r")))
	{
		/*no lochelp file*/
		return(0); 
	}
	while(fgets(locfile,MAXLINE,fp)!=NULL)
	{
		hold=strtok(locfile,"\t ");
		if(!(strcmp(ky,hold)))
		{
			hold=strtok((char *)0,"\n");
			strcpy(fi,hold); /* copy file name to fi */
			return(1); /* entry found */
		}
	}
	return(0); /* no entry found */
}


static clean_up()
{
	fclose(iop);
}
