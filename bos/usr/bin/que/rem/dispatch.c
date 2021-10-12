static char sccsid[] = "@(#)75	1.35  src/bos/usr/bin/que/rem/dispatch.c, cmdque, bos41J, 9514A_all 3/30/95 10:13:13";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
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
 */


#include	<IN/standard.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<time.h>
#include	<sys/signal.h>
#include	<sys/dir.h>
#include	"commonr.h"
#include 	<IN/q.h>
#include	<usersec.h>
#include	<sys/priv.h>
#include	"lpd.h"
#include	<sys/syslog.h>
#ifdef CSECURITY
#include <stdlib.h>
#endif CSECURITY

#include <ctype.h>

#include "lpd_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_LPD,num,str)
nl_catd	catd;

extern	char	**filter_addr;
extern	int	filter_size;
extern 	char    pqueue[];

extern  char    fhost[MAXHOSTNAMELEN];

#ifdef DEBUG
extern FILE	*outfp;
#endif
struct doit_data
{
	char	*jobname,
		*jobclass,
		*username,
		*hostname,
		*person,
		*jobtitle,
		*filename,
		*to,
		*nc,
		*from,
		*title,
		*opmsg,
		*bursts,
		*out,
		*indent,		/* indent amount	*/
		*width,			/* width amount		*/
		*dfn[MAXLPDFILES],	/* data file list */
		*parms[MAXARGS],	/* single letter parameter list */
		*argvec[MAXARGS];	/* the whole darn thing */
        boolean cflag,                  /* use cifplot filter -o -fc */
                dflag,                  /* use tex filter     -o -fd */
                fflag,                  /* use default filter, no enq flags */
                gflag,                  /* use graph filter   -o -fg */
                nflag,                  /* use tex filter     -o -fn */
                pflag,                  /* use pr(1) to print -o -fp */
                rflag,                  /* use fortran filter -o -ff */
                vflag,                  /* use vplot filter   -o -fv */
                lflag,                  /* use literal output -o -fl */
                tflag,                  /* use troff filter   -o -ft */
                oflag,                  /* print PostScript, no enq flags */
		mailflag;		/* send mail upon job completion */
};

doit(cfn)
char	*cfn;
{
	struct doit_data *ddata;
	FILE		*fp;
	char	buf[200];
	char	*dfile;				/* data file to unlink */
	char	*tmpstr;
	int	i,
		fcount,				/* file string count in array */
		pcount;				/* parameter string count in array */
	int	wstat;				/* wait status for fork */
	int 	pid;				/* pid of child	*/
	short	Bfound=0;			/* -Bxx flag ? */


	pid = fork();

	/*----Fork error */
        if (pid < 0)
        {
		syslog(LOG_ERR, MSGSTR(MSGFORK,"unable to fork process."));
                syserr((int)EXITFATAL,MSGSTR(MSGFORK,"unable to fork process."));
        }

	/*----Parent */
	if (pid > 0)
	{
		(void) privilege(PRIV_LAPSE);
		waitpid(pid,&wstat,0);
		return(0);
	}

	/*----Child, do it all */
#ifdef DEBUG
	if (getenv("DOIT"))
	 	fprintf(outfp,"in doit cfn=%s\n",cfn);
#endif
	/* set our process to run as user "lpd"
	 * so that enq doesn't run as an administrator
	 */
	(void) privilege(PRIV_ACQUIRE);
	if (setpcred("lpd",(char **)NULL) != 0)
	{
		syserr((int)EXITFATAL,MSGSTR(MSGCRED,"could not set process credentials."));
	}
	fcount = 0;
	pcount = 0;
	ddata = (struct doit_data *)Qalloc(sizeof(struct doit_data));
	bzero((char *)ddata,sizeof(struct doit_data));

	fp = fopen(cfn, "r");
	while (fgets(buf, (int)sizeof(buf), fp)) 
	{
		buf[strlen(buf) - 1] = '\0';
#ifdef DEBUG
		if (getenv("DOIT"))
			fprintf(outfp,"doit: bufline = [%s]\n",buf);	
#endif		
		/*----The last one is the operator message, if any */
		if(ddata->opmsg != NULL)
		{
			tmpstr = ddata->opmsg;
                        ddata->opmsg = sconcat(tmpstr,buf,"\n",0);
			free((void *)tmpstr);
			continue;
		}

		switch (buf[0]) 
		{
		case 'J':
			ddata->jobname = scopy(&buf[1]);
			break;
		case 'C':
			ddata->jobclass = scopy(&buf[1]);
			break;
		case 'L':
			ddata->username = scopy(&buf[1]);
			break;
		case 'H':
			ddata->hostname = scopy(&buf[1]); 
			break;
		case 'P':
			ddata->person = scopy(&buf[1]);
			break;
		case 'M':
			ddata->mailflag = TRUE;
			break;
		case 'T':
			ddata->jobtitle = scopy(&buf[1]);
			break;
		case 'I':
			ddata->indent = scopy(&buf[1]);
			break;
		case 'W':
			ddata->width = scopy(&buf[1]);
			break;
		case 'N':
			ddata->filename = scopy(&buf[1]);
			break;
		case 'U':
			/* already unlinking */
			break;
                case 'l':
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        ddata->lflag=TRUE;
                        break;
                case 'f':
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        ddata->fflag=TRUE;
                        break;
                case 't':
                        ddata->tflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'n':
                        ddata->nflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'd':
                        ddata->dflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'g':
                        ddata->gflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'v':
                        ddata->vflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'c':
                        ddata->cflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'r':
                        ddata->rflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
                        break;
                case 'p':
                        ddata->pflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
			break;
                case 'o':
			/* this case does not need special handling, but 
			 * we need to recognize this command as valid for a
			 * PostScript file
			 */
                        ddata->oflag=TRUE;
			ddata->dfn[fcount++] = sconcat(LPD_DIRECTORY,"/",&buf[1],0);
			break;
		case '-':
			switch (buf[1])
			{
			case 'f':
				/*----Ignore -from from RT ver 2 */
				break;
			case 't':
				/*----Ignore -to & -tl from RT ver 2 */
				if((!strncmp(buf,"-to",3) || !strncmp(buf,"-tl",3)) &&
				   (isspace(buf[3]) || ('\0' == buf[3])))
					break;
				ddata->to = scopy(&buf[2]);
				break;
			case 'N':
				ddata->nc = scopy(&buf[2]);
				break;
			case 'Z':
				ddata->from = scopy(&buf[2]);
				break;
			case 'T':
				ddata->title = scopy(&buf[2]);
				break;
			case 'm':
				/*----May be multiple line operator messages */
				ddata->opmsg = sconcat(buf,"\n",0);
				break;
			case 'n':
				/*----Ignore -nc from RT ver 2 */
				if ('c' == buf[2])
					break;
			default:	
				/*----Just copy the parm */
				ddata->parms[pcount++] = scopy(buf);
			} /* little switch */
			break;
		default:
			break;
		} /* big switch */
	} /* while */
	fclose(fp);


#ifdef DEBUG
	if (getenv("DOIT"))
 		fprintf(outfp,"doit: after while loop\n");
#endif		

/* What??
	if (strcmp(pqueue,"lp") == 0)
		strcpy(pqueue, "");
*/
	if (ddata->from == NULL)
		/* ddata->from = sconcat(ddata->username,"@",ddata->hostname,0); *** next line is D37536 fix */
		ddata->from = sconcat(ddata->person,"@",fhost,0);
	if (fcount == 0)
		syserr((int)EXITFATAL,MSGSTR(MSGRQUE,
		"no filename, request from %s not printed."), 
		ddata->from);
	if ((ddata->jobname == NULL) || (*(ddata->jobname) == '\0'))
		ddata->jobname = ddata->filename;
	if ((ddata->jobclass == NULL) || (*(ddata->jobclass) == '\0'))
		ddata->jobclass = ddata->hostname;
	if (ddata->to == NULL)
		/* ddata->to = sconcat(ddata->username,"@",ddata->hostname,0);  *** next line is D37536 fix */
		ddata->to = sconcat(ddata->person,"@",ddata->hostname,0);
	if (ddata->nc == NULL)
		ddata->nc = scopy("1");
	if (ddata->title == NULL)
		ddata->title = ddata->jobname;
        if(ddata->lflag)
                ddata->parms[pcount++] = scopy("-o-fl");
        if(ddata->tflag)
                ddata->parms[pcount++] = scopy("-o-ft");
        if(ddata->nflag)
                ddata->parms[pcount++] = scopy("-o-fn");
        if(ddata->dflag)
                ddata->parms[pcount++] = scopy("-o-fd");
        if(ddata->gflag)
                ddata->parms[pcount++] = scopy("-o-fg");
        if(ddata->vflag)
                ddata->parms[pcount++] = scopy("-o-fv");
        if(ddata->cflag)
                ddata->parms[pcount++] = scopy("-o-fc");
        if(ddata->rflag)
                ddata->parms[pcount++] = scopy("-o-ff");
	if(ddata->pflag) 
	{
                ddata->parms[pcount++] = scopy("-o-fp");
		if ( (ddata->jobtitle == NULL) || !strlen(ddata->jobtitle) )
			ddata->jobtitle = ddata->filename;
		ddata->parms[pcount++] = sconcat("-o-h",ddata->jobtitle,0);
	}
	if (ddata->width != NULL)
       	        ddata->parms[pcount++] = sconcat("-o-w",ddata->width,0);
	if (ddata->indent != NULL)
		ddata->parms[pcount++] = sconcat("-o-i",ddata->indent,0);
	if (ddata->username != NULL)
		ddata->parms[pcount++] = sconcat("-o-H",ddata->jobclass,0);
	ddata->parms[pcount] = NULL;
	ddata->dfn[fcount] = NULL;
	
	/*----Construct the enq command */
	i = 0;
	ddata->argvec[i++] = scopy("/usr/bin/enq");
	ddata->argvec[i++] = sconcat("-P",pqueue,0);
	ddata->argvec[i++] = scopy("-r");		/* remove file after printing */
	ddata->argvec[i++] = sconcat("-t",ddata->to,0);
	ddata->argvec[i++] = sconcat("-Z",ddata->from,0);
	ddata->argvec[i++] = sconcat("-N",ddata->nc,0);
        if (ddata->title && *ddata->title)
		ddata->argvec[i++] = sconcat("-T",ddata->title,0);
	if ( ddata->mailflag ) {
		ddata->argvec[i++] = scopy("-n");
		ddata->argvec[i++] = scopy("-C");
	}
	if(ddata->opmsg != NULL)
		ddata->argvec[i++] = ddata->opmsg;
	for(pcount = 0; ddata->parms[pcount] != NULL;) {
		if ( strncmp(ddata->parms[pcount], "-B",2) == 0 )
			Bfound++;
		ddata->argvec[i++] = ddata->parms[pcount++];
	}
	/*
	 * if the username is specified - L from cf - and  no -Bxx flag was 
	 * specified at the command line, add -Ban to the list of flags
	 */
	if ((ddata->username != NULL) && !Bfound) {
		ddata->argvec[i++] = scopy("-Ban");
	}
	for(fcount = 0; ddata->dfn[fcount] != NULL;)
		ddata->argvec[i++] = ddata->dfn[fcount++];
	ddata->argvec[i] = NULL;
#ifdef DEBUG
       	if(getenv("ALLPARMS"))
       	{
               	fprintf(stderr,"doit to enq: ");
               	for(i = 0; ddata->argvec[i] != NULL; i++)
                       	fprintf(stderr,"[%s]",ddata->argvec[i]);
               	fprintf(stderr,"\n");
       	}
#endif
	/*----Exec it */
	setpenv(NULL,PENV_INIT|PENV_ARGV, (char **)NULL, (char *)ddata->argvec);
	syserr((int)EXITFATAL,MSGSTR(MSGEXEC,"could not exec /bin/enq."));
}
