static char sccsid[] = "@(#)75        1.22  src/bos/usr/bin/que/rem/paramr.c, cmdque, bos411, 9428A410j 5/5/94 14:51:25";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	See commonr.h for BSD lpr/lpd protocol description.*/

#define _ILS_MACROS
#include "commonr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <stdio.h>
#include <time.h>
#include "outr.h"
#include <ctype.h>

#include "rem_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_REM,num,str)
nl_catd	catd;

#ifdef DEBUG
extern FILE *outfp;
#endif

extern int timeout_ack;		/* timeout for ACKs - from paramr.c */

/*
	Remote ReaD OPTionS
	Rdopts reads the command line into the comline struct.
	There is no semantic ambiguity in the command line so getopts
	works ok.
	Filenames must come after all options have been specified.
	Anything not preceded by a '-' or an option which requires an
	argument, is a filename.
*/
rrdopts(argc,argv,c,j,firstfile)
int		argc;
char 		**argv;
struct comline 	*c;		/* the main idea of this routine*/
int 		*firstfile;	/* how far did we read thru the argv*/
struct job	*j;
{
	int i;
	int jobnum = 0;
	char *a;
	char *thisarg;
	extern char * optarg;
	extern int optind;
	char * concat_bsdargs(struct comline *,struct job *,char *,char **,int *);

#ifdef DEBUG
        if(getenv("RRDOPTS"))
        {       
                fprintf(outfp,"rrdopts: ");
                for(i = 0; argv[i] != NULL; i++)
                        fprintf(outfp,"[%s]",argv[i]);
                fprintf(outfp,"\n");
		fflush(outfp);
        }
#endif

	/* Nuke out the c struct. */
	bzero((char *) c, sizeof(struct comline));

	while ((i = getopt(argc,argv,"XRN:qLxo:#:u:P:S:T:")) !=EOF)
	{
		switch (i)
		{
		case 'X':	/* D43769, send -o flags regardless */
			c->c_Xflg++;
			break;
		case 'R': 	/* Restart daemon */
			c->c_Rflg++;
			break;
		case 'N': 	/* qstatus filter */
			c->c_Nflg++;
			c->c_Nrem=scopy(optarg);
			break;
		case 'L': 	/* get long q status */
			c->c_Lflg++;
			break;
		case 'q': 	/* get q status */
			c->c_qflg++;
			break;
		case 'x': 	/* Cancel */
			c->c_xflg++;
			break;
                case '#':      /* job number for status or cancel */
                        c->c_numflg++;
			jobnum = ckjobnum(optarg);
			switch(jobnum)
			{
			case -1:
                        	syswarn(MSGSTR(MSGBJOB,"Bad job number %s."),optarg);
				break;
                        default:
                                add_jobnum(jobnum,j);
			}
                        break;
                case 'u':	/* username for status or cancel */
                        c->c_uflg++;
                        add_username( optarg,j);
                        break;
                case 'P':	/* remote queue name */
                        c->c_Pflg++;
                        c->c_Prem=scopy(optarg);
                        break;
                case 'S':	/* remote server name */
                        c->c_Sflg++;
                        c->c_Srem=scopy(optarg);
                        break;
		case 'T':	/* Timeout wait for ACKs - in minutes */
			c->c_TMflg++;
			/* atoi returns 0 on failure - OK for alarm -,
			   so no need to check return */
			timeout_ack = atoi(optarg) * 60 ;
			break;

		case 'o': 	/* optional backend arguments */
	/* BSD flags, like all non-rembak flags are prefaced w/ -o 
	   The following code implies that BSD flags, such as '-p' must be alone
	   after the -o on the command line.  i.e. 
			rembak -o -p -o -hthisisatitle filenm 
		is legal, but
			rembak -o -phthisisatitle filenm 
		is not,( can't put -p and -h together), even though some higher level 
		command might accept it.

	   In this way, 
			rembak -o -plot filenm 
	   is still acceptable, as it must be if we are to talk to 2.2.1aix or
	   older systems.
	*/
			a = optarg;
			while (*a==' ' || *a=='\t') a++;
			if (*a=='-')
			{
				if(*(a+2)=='\0') {
					thisarg = concat_bsdargs(c,j,a,argv,&optind);
					a = thisarg;
				}
				if (*(a+2)!='\0')
				{
					switch (*(a+1)) { 	/* BSD flags with arguments*/
					case 'i': 	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_irem=scopy(a);
						c->c_iflg++;
						break;
					case 'h': 		/* Title flag - in bsd it is -T */	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_Tflg++;
						c->c_Trem=scopy(a);
						break;
					case 'w': 		
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_wflg++;
						c->c_wrem=scopy(a);
						break;
					case '1': 	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_1flg++;
						c->c_1rem=scopy(a);
						break;
					case '2': 	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_2flg++;
						c->c_2rem=scopy(a);
						break;
					case '3': 	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_3flg++;
						c->c_3rem=scopy(a);
						break;
					case '4': 	
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_4flg++;
						c->c_4rem=scopy(a);
						break;
					case 'M':	/* remote operator message */
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_Mflg++;
						c->c_Mrem=scopy(a);
						break;
					case 'H':
						a=a+2;
						while (*a==' ' || *a=='\t') a++;
						c->c_Hflg++;
						c->c_Hrem = scopy(a);
						break;
					case 'f':	/* remote filter options */
						switch (*(a+2)) {
						case 'f': 	
							c->c_fflg++;
							break;
						case 'c': 	
							c->c_cflg++;
							break;
						case 'l': 	
							c->c_lflg++;
							break;
						case 'p': 	
							c->c_pflg++;
							break;
						case 't': 	
							c->c_tflg++;
							break;
						case 'n': 	
							c->c_nflg++;
							break;
						case 'd': 	
							c->c_dflg++;
							break;
						case 'g': 	
							c->c_gflg++;
							break;
						case 'v': 	
							c->c_vflg++;
							break;
						case 'o': 	
							c->c_Oflg++;
							break;
						default:   /* process all other filter flags */
							/* D43769 send it with -o to the remote, regardless */
							c->c_oflg++;
							add_bakcmd(a,j,c);
						} /* switch (*(a+2)) */
						break;
					default:
						c->c_oflg++;
						add_bakcmd(optarg,j,c);
						break;
					} /* switch(*(a+1)) */
				} /* if(*(a+2)!='\0') */
			} /* if (*a=='-') */
			else
			{
				c->c_oflg++;
				add_bakcmd(optarg,j,c);
			}
			break;
		case '?':
			usage();
			return(NOTOK);
		default:
			syserr((int)EXITFATAL,"bug in paramr.c getopt");	/* sanity */
		} 
	} /* while getopt */
	*firstfile = optind;

	
	return(OK);
}

usage()
{
	sysuse( FALSE,
		MSGSTR(MSGUSE1, "[-qxLRX] [-u<username>] [-#<jobnum>] [-N<filter>] [-P<printer>]"),
		MSGSTR(MSGUSE2, "[-S<server>] [-o [-cdfglmnptv] [-i<indent>]"),
	 	MSGSTR(MSGUSE3, "    [-w<width>] [-h<title>] [1,2,3,4<font>]  ]"),
		(char *)0
	      );
}



/**************************************************
	ADD A user name TO THE DESIRED-user LIST
**************************************************/
add_username(name,j)
char		*name;
struct job	*j;
{
	register struct users *new;

	if (!j)
		syserr((int)EXITFATAL,"bug found in add_username, null job struct.");	/* sanity */

	/*----Allocate the space for new item */
	new = (struct users *)Qalloc(sizeof(struct users));

	/*----Insert new item into list */
	new->u_user = scopy(name);
	new->u_next = j->j_u;
	j->j_u = new;
	return(0);
}

/**************************************************
	ADD AN UNRECOGNIZED OPTIONS TO THE AIXOPTION LIST, Prefaced w/ -o
**************************************************/
add_bakcmd(opt,j,c)
char		*opt;
struct job	*j;
struct comline  *c;
{
	char *s;
	
	if (!j)
		syserr((int)EXITFATAL,"bug found in add_bakcmd, null job struct");	/* sanity */
	
	/* D43769, send -o only if the remote site is aix */
	if (strcmp(c->c_Nrem,S_DEFAULT_STATFILTER) == 0)	
	  s = sconcat("-o",opt,0);
	else		/* non aix machine */
	  s = sconcat("",opt,0);

	add_aixcmd(s,j);
	free((void *)s);
	return(0);
}

/**************************************************
	ADD AN UNRECOGNIZED OPTIONS TO THE AIXOPTION LIST
**************************************************/
add_aixcmd(opt,j)
char		*opt;
struct job	*j;
{
	register struct aixcmds *ac;

	if (!j)
		syserr((int)EXITFATAL,"bug found in add_aixcmd, null job struct");	/* sanity */

	/*----Allocate the space for new item */
	ac = (struct aixcmds *)Qalloc(sizeof(struct aixcmds));

	/*----Insert new item into list */
	ac->ac_opt = scopy(opt);
	ac->ac_next = j->j_ac;
	j->j_ac = ac;
	return(0);
}

/**************************************************
	ADD A JOB # TO THE DESIRED-JOB LIST
**************************************************/
add_jobnum(num,j)
int		num;
struct job	*j;
{
	register struct jobnum *newjobnum;
	
	if (!j)
		syserr((int)EXITFATAL,"bug found in add_jobnum, null job struct");

	/*----Allocate the space for new item */
	newjobnum = (struct jobnum *)Qalloc(sizeof(struct jobnum));

	/*----Insert new item into list */
	newjobnum->jn_num = num;
	newjobnum->jn_next = j->j_jn;
	j->j_jn = newjobnum;
	return(0);
}


int ckjobnum(inpnum)
char	*inpnum;
{
	int num = 0;

	if (!strcmp(REM_ALLNUM_VALUE,inpnum))	/*equal */
		return(ALLNUM);

	num = atoi(inpnum);	/* MINJOB and MAXJOB must be set so that atoi()
				   returns a value outside of (MINJOB,MAXJOB)
			           for invalid number strings. (i.e. 123w45) */

	if ((num < MINJOB) || (num > MAXJOB))
	        return(-1);
	else
		return(num);
}


boolean validate(c,firstfile)
struct comline *c;
char *firstfile;
{
	
/*	while ((i = getopt(argc, argv,"NqLxo:#:u:P:S:T:")) !=EOF)
*/

	
	if (!c->c_Sflg )
		syserr((int)EXITFATAL,MSGSTR(MSGNPRT,"No print server specified."));

	if (!c->c_Pflg )
		syserr((int)EXITFATAL,MSGSTR(MSGNREM,"No remote queue specified."));

	if ((!firstfile) && 
		!(c->c_Rflg || c->c_qflg  || c->c_xflg || c->c_Lflg ))
		syserr((int)EXITFATAL,"No files specified");

	if (c->c_xflg && !(c->c_numflg || c->c_uflg ))
	       syserr((int)EXITFATAL,MSGSTR(MSGCCAN,"Cannot specify cancel job without job number or user."));
 
	if (      	 (c->c_numflg || c->c_uflg ) && 
			!(c->c_qflg  || c->c_xflg || c->c_Lflg )
	   )
		syserr((int)EXITFATAL, MSGSTR(MSGUSJN,
			"Cannot specify users or job numbers except on cancel or status requests.")); 
}



/**************************************************
	Return the string which is the concatanation of all the user names
	with spaces in between.
**************************************************/
char * userstr(u)
struct users	*u;
{

	char * fred;
	char * bill;


	if (!u->u_next) 
		return(u->u_user);
	fred = userstr(u->u_next);
	bill = sconcat(fred," ",u->u_user,0);
	free((void *)fred);
	return(bill);

}



/**************************************************
	Return the string which is the concatanation of all the user names
	with spaces in between.
**************************************************/
char * jobnumstr(jn)
struct jobnum	*jn;
{

	char * fred;
	char * bill;
	char num[10];

	/* case where all jobnumbers are desired */
	if(jn->jn_num == ALLNUM)
	{
		fred=sconcat(REM_ALL_STR,0);
		return(fred);
	}
	/* base case*/
	if (!jn->jn_next) 
	{
		sprintf(num,"%d",jn->jn_num);
		fred=sconcat(num,0);
		return(fred);
	}
	fred = jobnumstr(jn->jn_next);
	if(fred[0] == '-')
		return(fred);
	else
	{
		sprintf(num,"%d",jn->jn_num);
		bill = sconcat(fred," ",num,0);
		free((void *)fred);
	}
	return(bill);
}



/**************************************************
	Return the string which is the concatanation of all the aix commands
	with carriage returns in between.
**************************************************/
char * aixstr(ac)
struct aixcmds *ac;
{

	char * fred;
	char * bill;

	if (!ac)	
		return("");
	if (!ac->ac_next) 
	{
		bill = sconcat(ac->ac_opt,"\n",0);
		return(bill);
	}
	fred = aixstr(ac->ac_next);
	bill = sconcat(fred,ac->ac_opt,"\n",0);
	free((void *)fred);
	return(bill);
}


/**************************************************
	Concatenate optargs of bsd options if they are in separate vectors.
	For example, a user may enter
		enq -o -fp filename              
	or he/she may enter
		enq -o -f -o p filename
	The two should be synonymous.
**************************************************/

char * concat_bsdargs(struct comline *c,struct job *j,char *a,char **argvec,int *argvind)
{
	char *newstr;
	char *nextvec;

	switch (*(a+1)) {
		case 'm':	/* notify when done */
			c->c_mflg++;
			break;
		case 'f':
		case 'i':
		case 'w':
		case '1':
		case '2':
		case '3':
		case '4':
		case 'M':
		case 'h':
		case 'H':
			nextvec = argvec[*argvind];
			if ( nextvec[0] == '-' && nextvec[1] == 'o') {
				nextvec = argvec[(*argvind)+1];
				while (isspace((int)(*nextvec)) )
					nextvec++;
				if ( *nextvec != '-' ) {  	/* BSD flag argument */
					newstr = Qalloc(strlen(a) + strlen(nextvec));
					strcpy(newstr,a);
					strcat(newstr,nextvec);
					(*argvind)+=2;
					return (newstr);
				}
			}
			break;
		default:
			c->c_oflg++;
			add_bakcmd(a,j,c);
			break;
	} /* switch */
	return (a);
}
