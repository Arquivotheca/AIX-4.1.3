static char sccsid[] = "@(#)49	1.20.1.11  src/bos/usr/bin/que/rem/cf.c, cmdque, bos411, 9430C411a 7/26/94 16:37:36";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 	This is the remote backend for the queueing system.
	It communicates with remote hosts using the BSD lpr/lpd 
	protocol.  See commonr.h for protocol description.
*/
#include "commonr.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <stdio.h>
#include "outr.h"
#define STDSTAT 3
#define SHORT_STATFILTER "aixshort"
#include <ctype.h>
#include <string.h>   /* D41813 */

#include "rem_msg.h"
nl_catd catd;
#define MSGSTR(num,str)	catgets(catd,MS_REM,num,str)

extern boolean backend ;	  
extern char *getclass();

#ifdef DEBUG
extern FILE *outfp;
#endif



/* given the real filename, the socket, and the fake filename that we give
	to the remote lpd, send the control file accross the net

 send a controlfile  which means:
	send req string containing fakefn
	get ack
	send the control file (realfn)
	send ack
	get ack 
*/

boolean sendcontrolfile(s,realfn,fakefn)
int 	s;		/* socket file descriptor.   */
char	*realfn ,
	*fakefn;
{

	struct stat 	sbuf;
	char		reqtype = '\2';
	int		filefd;

	fakefn[0]='c';		/* it's a cfname not a dfname*/

	if (stat(realfn,&sbuf) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGSTFL,"Status on %s failed:  Errno = %d."),realfn,errno);

	if (!send_file_req(s,sbuf.st_size,realfn,fakefn,reqtype))
		return(FALSE);
	
	if ((filefd = open(realfn,O_RDONLY,0)) < 0 )
		syserr((int)EXITFATAL,MSGSTR(MSGCOPN,"Cannot open %s."),realfn);

	if (!send_file(s,filefd,sbuf.st_size))
		return(FALSE);

	return(TRUE); /* everything ok */
}


char * makecontrolfile(j,c,argv,firstfile,dfnames)
struct job 	*j;
struct comline 	*c;
char		**argv;		/* argv from exec */
int		firstfile;	/* the first file index into argv */
char		**dfnames;	/* names of all the data files we sent*/
{
	char 		*bsdcmds, *aixcmds; 
	static char	*fname;
	int fd;		/* file descriptor of control file we're creating*/

	getcntrlinfo(j,c,argv,firstfile,dfnames,&bsdcmds,&aixcmds);

	fd=gettmpfile("/tmp/",&fname);
	
	if (write(fd,bsdcmds,strlen(bsdcmds))<0)
		syserr((int)EXITFATAL,MSGSTR(MSGBSDC,"Could not write BSD commands to %s on fd = %d."),fname,fd);

	if (strstr(c->c_Nrem,SHORT_STATFILTER) ||  /* if AIX remote server */
			c->c_Xflg )			    /* D43769, -X to force -o flags thru */
		if (write(fd,aixcmds,strlen(aixcmds))<0)
			syserr((int)EXITFATAL,MSGSTR(MSGAIXC,"Could not write commands to %s."),fname);
	
	return(fname);
}

getcntrlinfo(j,c,argv,firstfile,dfnames,bsdcmds,aixcmds)
struct job 	*j;
struct comline 	*c;
char		**argv;		/* argv from exec */
int		firstfile;	/* the first file index into argv */
char		**dfnames;
char **bsdcmds, **aixcmds;
{
	struct sfil *s;
	struct sfil * getaixinfo();

	s = getaixinfo(c,argv[firstfile],j,aixcmds);		
	getbsdinfo(s,j,c,argv,firstfile,dfnames,bsdcmds);
	return(0);
}


/* return the string which has all of the aixcommands in it.
   They r seperated by \n
*/
struct sfil * getaixinfo(c,title,j,aixcmds)
struct comline 	*c;
char		*title;		/* argv from exec */
struct job 	*j;
char 		**aixcmds;
{
	struct sfil *getstatii();
	struct sfil *get_deflt_stat();
	char * aixstr();
	static struct sfil *s;
	char * str;
	
	if (backend)
		s = getstatii();
	else 
 		s = get_deflt_stat(title);

	if (strstr(c->c_Nrem,SHORT_STATFILTER))   /* if AIX remote server */
	{
		addstat2cmds(s,j);

		if (c->c_mflg)	/* if we should notify*/
			add_aixcmd("-n",j);	/* the enq -n flg*/

		if (c->c_Mflg)	/* operator message THIS HAS TO BE LAST ! b*/
		{
			str = sconcat("-m",c->c_Mrem,0);
			add_aixcmd(str,j);
			free((void *)str);
		}
	}
	*aixcmds = aixstr(j->j_ac);

	return(s);
}

char * get_bsd_general(s,j,c,rf,df)
struct sfil 	*s;
struct job	*j;
struct comline 	*c;
char    	**rf,	/* real filenames*/
		**df;	/* data filenames*/
{
	char *rv;	/* ret value*/
	char *p,*q;	/* tmp pointers*/
	char *cmd_line;	/* get cmdline from status file */

/* Defect 38791 - get user name from status file */
	
	if (!s)
		syserr((int)EXITFATAL,"bug in get_bsd_general bad *s");	/* sanity */

	/* if this is a request from lpd then parse arguments for H and
	 * P off of s_to
	 */

	if ( (s->s_to != NULL) && ( q = strrchr(s->s_to, (int)'@')) ) {
	    char tmp_to[S_TO];

	    strcpy(tmp_to, s->s_to);
	    q = strrchr(tmp_to, (int)'@');
	    *q = '\0';
	    q++;
	    p = sconcat("H", q, "\n", "P", tmp_to, "\n",0);
	    q=NULL;
	}
	else
	    p = sconcat("H",myhost(),"\n",
 		    "P",s->s_to == NULL ? (char *)getusername() : (char *)s->s_to,"\n",0);
	
/* end D38791 */

/* Defect 57049 - Put the J, C, & L flags in the control file if
**         1.  printing to an AiX version2 machine, OR
**	   2.  user requested a burst page with -B on command line
*/
        if (strstr(c->c_Nrem,V2FILT) ||  /* required for 5A system */
	   (user_burst(s->s_cmdline) && (s->s_head != NEVER)) )
        {
	q = sconcat(p,"J",j->j_jobname,   "\n",
		      "C",getclass(c),       "\n",
		      "L",getusername(),  "\n",0);
	free((void *)p);
	} 
	else 
		q = p; 

	if (c->c_iflg)	/* indent */
	{
		p = sconcat(q,"I",c->c_irem,"\n",0);
		free((void *)q);
	}
	else
		p = q;

	if (c->c_mflg) /* use mail for completion notify */
	{
		q = sconcat(p,"M",s->s_from,"\n",0);
		free((void *)p);
	}
	else
		q = p;

	if (c->c_1flg) /* load fonts */
	{
		p = sconcat(q,"1",c->c_1rem,"\n",0);
		free((void *)q);
	}
	else
		p = q;

	if (c->c_2flg)
	{
		q = sconcat(p,"2",c->c_2rem,"\n",0);
		free((void *)p);
	}
	else
		q = p;

	if (c->c_3flg)
	{
		p = sconcat(q,"3",c->c_3rem,"\n",0);
		free((void *)q);
	}
	else
		p = q;

	if (c->c_4flg)
	{
		q = sconcat(p,"4",c->c_4rem,"\n",0);
		free((void *)p);
	}
	else
		q = p;

	if (c->c_wflg) /* width */
	{
		p = sconcat(q,"W",c->c_wrem,"\n",0);
		free((void *)q);
	}
	else
		p = q;

	rv = p;
	return(rv);
}

getbsdinfo(s,j,c,argv,firstfile,dfnames,bsdcmds)
struct sfil 	*s;
struct job	*j;
struct comline 	*c;
char		**argv;		/* argv from exec */
int		firstfile;	/* the first file index into argv */
char		*dfnames[];
char 		**bsdcmds;
{
	char *bsd1;
	char *bsd2;
	char *bsd3;
	char * get_bsd_4file();
	int i=0;

	bsd3 = get_bsd_general(s,j,c,&argv[firstfile],dfnames);
#ifdef DEBUG
	if(getenv("GETBSDIN"))
	{
		fprintf(outfp,"bsdgen = %s\n",bsd3);		/* sanity */
		fflush(outfp);
	}
#endif
	for (i = 0; dfnames[i]; i++)
	{
		if (argv[i] == 0)
			syserr((int)EXITFATAL,MSGSTR(MSGSFIL,"Some files not sent."));	
		if (bsd2 = get_bsd_4file(s, c, j,
				         argv[firstfile+i],
				         dfnames[i]))
		{	
			bsd1 = bsd3;
			bsd3 = sconcat(bsd1,bsd2,0);
			free((void *)bsd1);
			free((void *)bsd2);
		}
		else
			syserr((int)EXITFATAL,"problem in get_bsd_4file");	/* sanity */  
	}
	
	*bsdcmds = bsd3;
#ifdef DEBUG
        if(getenv("GETBSDIN"))
        {
                fprintf(outfp,"bsd3 = %s\n",bsd3);            /* sanity */
                fflush(outfp);
        }
#endif
	return(0);
}
	
char * get_bsd_4file(s,c,j,realfnam,dfname)
struct sfil 	*s;
struct comline 	*c;
struct job	*j;
char		*realfnam;
char		*dfname;
{
	char filter;
	char filtstr[2];
	char *fnames;
	char *fstr, *f1str, *tstr, *ustr ,*nstr, *rv;
	
	int i,nc;		/* number of copies */

	filter = 'f';		/* default filter: file already formatted */
	if (c->c_pflg)		/* use pr filter */
		filter = 'p';
	else
	if (c->c_Oflg)		/* print PostScript */
		filter = 'o';
	else
	if (c->c_fflg)		/* use fortran filter */
		filter = 'r';
	else
	if (c->c_cflg)		/* use cifplot filter */
		filter = 'c';
	else
	if (c->c_lflg)		/* use lfilter filter */
		filter = 'l';
	else
	if (c->c_tflg)		/* use troff filter */
		filter = 't';
	else
	if (c->c_nflg)		/* use ditroff filter */
		filter = 'n';
	else
	if (c->c_dflg)		/* use tex filter */
		filter = 'd';
	else
	if (c->c_gflg)		/* use plot filter */
		filter = 'g';
	else
	if (c->c_vflg)		/* use varian filter */
		filter = 'v';

	if ((filter == 'p') && c->c_Tflg)
		tstr = sconcat("T",c->c_Trem,"\n",0);
	else
		tstr = scopy("");
	filtstr[0] = filter;
	filtstr[1] = '\0';
	if (s->s_copies)
		nc = s->s_copies;
	else		/* sanity*/
		syserr((int)EXITFATAL,"bug in get_bsd_4file()");	/* sanity */
#ifdef DEBUG
        if(getenv("GETBSDIN"))
        {
                fprintf(outfp,"bsd copies = %d\n",nc);
                fflush(outfp);
        }
#endif

	fstr = scopy("");
	f1str = sconcat(filtstr,dfname,"\n",0);
	for (i = 0; nc; nc--,i++)
	{
		fnames = sconcat(fstr,f1str,0);
		free((void *)fstr);
		fstr = fnames;
		/* If AIX client we already have copies info in control file */
		if (strstr(c->c_Nrem,SHORT_STATFILTER))
			break;
	}
	free((void *)f1str);
	ustr = sconcat("U",dfname,"\n",0);
	if (s->s_title == NULL) {
		if ( j->j_jobname == NULL )
		    nstr = sconcat("N",realfnam,"\n",0);
		else
		    nstr = sconcat("N",j->j_jobname,"\n",0);
	}
	else
		nstr = sconcat("N",s->s_title,"\n",0);
#ifdef DEBUG
        if(getenv("GETBSDIN"))
        {
                fprintf(outfp,"bsd strings:\n [%s][%s][%s][%s]\n",tstr,fstr,ustr,nstr);
                fflush(outfp);
        }
#endif


	rv = sconcat(tstr,
		     fstr,	
		     ustr,
		     nstr,
		     0);
	free((void *)tstr);
	free((void *)fstr);
	free((void *)ustr);
	free((void *)nstr);
	return(rv);
}


/* only called if we're not a backend*/
struct sfil *get_deflt_stat(title)
char		*title;		/* argv from exec */
{
	struct sfil *s;
	char 	*x;
	int	offset;
	
	s = (struct sfil *)Qalloc(sizeof( struct sfil ));

	s->s_copies = 1;
	strcpy( s->s_from, x=sconcat(getusername(),"@",myhost(),0));
	s->s_head = ALWAYS;
	s->s_trail = NEVER;
	s->s_mailonly = FALSE;

	/* Defect 10937.
	 * Copy only last S_TITLE-1 chars from title
	 */
	offset = strlen(title) - S_TITLE + 1;
	if (offset < 0)
		offset = 0;
	strcpy(s->s_title, title+offset);

	strcpy( s->s_to , x);
	free((void *)x);
	return(s);
}

/* go get stuff from statusfil and put in sfil struct
   such as AIX control file, jobname, jobclass, jobtitle, etc. */

struct sfil *getstatii()
{
	struct sfil *s;
	
	s = (struct sfil *)Qalloc(sizeof( struct sfil ));

	s->s_copies = get_copies();
	strcpy( s->s_from, get_from());
	s->s_head = get_header();
	s->s_trail = get_trailer();
	s->s_mailonly = get_mail_only();
	strcpy( s->s_title , get_title());
	strcpy( s->s_to , get_to());
	strcpy( s->s_cmdline , get_cmd_line());
	return(s);
}

addstat2cmds(s,j)
struct sfil 	*s;
struct job 	*j;
{
	char * str;
	char  buf[40];
	char bpstr[5];


	/*----Number of copies */
	if (s->s_copies == 0)
		s->s_copies = 1;
	sprintf(buf,"%d",s->s_copies);
	str = sconcat("-N",buf,0);
	add_aixcmd(str,j);
	free((void *)str);
	

	/*----From */
	if (s->s_from==NULL)
		str = sconcat("-Z",getusername(),"@",myhost(),0);
	else {  /* D41813 */
                if ( strrchr( s->s_from, (int)'@' ) )
		   str = sconcat("-Z",s->s_from,0);
                else
                   str = sconcat("-Z",s->s_from,"@",myhost(),0);
             }
	add_aixcmd(str,j);
	free((void *)str);
		

	/*----To */
	if (s->s_to == NULL)
		str = sconcat("-t",getusername(),"@",myhost(),0);
	else {
                if ( strrchr( s->s_to, (int)'@' ) )
	           str = sconcat("-t",s->s_to,0);
                else
	           str = sconcat("-t",s->s_to,"@",myhost(),0);
             }
	add_aixcmd(str,j);
	free((void *)str);
		

	/*----Title */
	if (s->s_title == NULL)
		str = sconcat("-T",j->j_jobname,0);
	else
		str = sconcat("-T",s->s_title,0);
	add_aixcmd(str,j);
	free((void *)str);

	/* Only put burst pages in if user specified -B on cmd line.
	   Otherwise, server should default to values in its own qconfig */
	if ( user_burst(s->s_cmdline) ) {
		/*----Burst options */
		bpstr[0] = '-';
		bpstr[1] = 'B';
		switch (s->s_head) 
		{
		case NEVER:
		case USEDEF:
			bpstr[2] = 'n';
			break;
		case ALWAYS:
			bpstr[2] = 'a';
			break;
		case GROUP:
			bpstr[2] = 'g';
			break;
		default:
			syserr((int)EXITFATAL,MSGSTR(MSGBURS,
			"Invalid burst page header value detected in statusfile, value = %d."), 
				s->s_head);
			break;
		}
		switch (s->s_trail)
		{
		case NEVER:
		case USEDEF:
			bpstr[3] = 'n';
			break;
		case ALWAYS:
			bpstr[3] = 'a';
			break;
		case GROUP:
			bpstr[3] = 'g';
			break;
		default:
			syserr((int)EXITFATAL,MSGSTR(MSGBURS,
			"Invalid burst page header value detected in statusfile, value = %d."), 
				s->s_trail);
			break;
		}
		bpstr[4] = '\0';
		add_aixcmd(bpstr,j);
	} /* if user_burst() */

	return(0);
}
