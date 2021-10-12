static char sccsid[] = "@(#)52	1.8  src/bos/usr/ccs/lib/libsrc/cmdargs.c, libsrc, bos411, 9428A410j 2/26/91 14:54:19";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	cmdargs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


/*
** IDENTIFICATION:
**    Name:	cmdargs
**    Title:	Parse Command Line Arguments
** PURPOSE:
**	To parse command line arguments.
** 
** SYNTAX:
**    cmdargs(argc,argv,host,subsystem,subsyspid,object,objname,env,args,argflags)
**    Parameters:
**	i int argc - number of arguments entered on command line
**	i char *argv[] - arguments entered on command line
**	o char *host - host that command will apply to
**	o short *object - SUBSYSTEM or subserver code point
**	o char *subsystem - name of subsystem request is for
**	o long *subsyspid - subsystem pid the request is for
**	o char *objname - subserver object (could end up being a pid)
**	o char *env - environment to be setup when starting a subsystem
**	o char *args - arguments to give subststem when starting it
**	o char *argflags - valid command line option arguements in a
**		format that getopt understands
**
** INPUT/OUTPUT SECTION:
**	ODM Object:
**		SRCsystem - subsystem object
**		SRCsubsvr - subserver object
**
** PROCESSING:
**	Note: Subsystem, Subsystem Group, All, or Subserver must have been
**	entered on the command line or a parameter error is returned.
**	Note: When one subsystem is requested the subsystem name must 
**	exist in the subsystem object in "subsysname" or "synonym"
**	Note: When one subsystem group is requested the subsystem group name
**	must exist in the subsystem object in "grpname"
**	Note: When one subserver is requested the subserver type
**	must exist in the subserver object in "sub_type"
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	Global Storage:
**	    o short Long - long indicator specified on command line
**	    o short Stopforce - Stop force was specified on command line
**	    o short Stopcancel - Stop force was specified on command line
**	    o short Contact - Subsystem contact type
**	    i short Nolong - long on command line is an illegal option
**	    o short Smitformat - print odm data in smit format
**
** RETURNS:
**	SRC_OK on success
**	Error code on failure
**/
#include <odmi.h>
#include "src.h"
#include "srcodm.h"
#include "srcopt.h"
#include "srcobj.h"

static struct SRCsubsys SRCsubsys;
static struct SRCsubsvr SRCsubsvr;


/* Global fields */
short Justpid=TRUE;	/* just subsys pid is an allowed entry */
short Long=FALSE;       /* long status or trace */
short Stopforce=FALSE;  /* stop force */
short Stopcancel=FALSE; /* stop cancel */
short Contact=FALSE;    /* what type of contact */
short Nolong=TRUE;	/* long not allowed on all or group commands */
short Smitformat=0;

static char subtype[SRCNAMESZ*2];
static short all;
static char grpname[SRCNAMESZ*2];
static long subsvrpid;
static short printsubsys=FALSE;
static short printsubserver=FALSE;
static short printnotify=FALSE;
static short printdefaultsubsys=FALSE;
static char notifyname[SRCNAMESZ*2];

/* define all the posible input arguments */
struct argview argview[]=
{
	{1,(char *)&Stopforce,FLAGSHORT,'f'},
	{1,(char *)&Stopcancel,FLAGSHORT,'c'},
	{1,(char *)&Long,FLAGSHORT,'l'},
	{1,(char *)&subsvrpid,ODM_LONG,'P'},
	{1,(char *)subtype,ODM_CHAR,'t',0,0,0,SRCNAMESZ-1,SRC_SUBTYP2BIG},
	{1,0,ODM_CHAR,'s',0,0,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{1,grpname,ODM_CHAR,'g',0,0,0,SRCNAMESZ-1,SRC_GRPNAM2BIG},
	{1,0,ODM_CHAR,'e',0,0,0,ENVSIZE-2,SRC_ENV2BIG},
	{1,0,ODM_CHAR,'o',0,0,0,SRCNAMESZ-1,SRC_SUBOBJ2BIG},
	{1,0,ODM_LONG,'p'},
	{1,0,ODM_CHAR,'a',0,0,0,PARMSIZE-2,SRC_ARG2BIG},
	{1,(char *)&all,FLAGSHORT,'a'},
	{1,0,ODM_CHAR,'h',0,0,0,HOSTSIZE-1,SRC_HOST2BIG},
	{1,notifyname,ODM_CHAR,'n',0,0,0,SRCNAMESZ-1,SRC_NOTENAME2BIG},
	{1,(char *)&printsubsys,FLAGSHORT,'S'},
	{1,(char *)&printsubserver,FLAGSHORT,'T'},
	{1,(char *)&printnotify,FLAGSHORT,'N'},
	{1,(char *)&printdefaultsubsys,FLAGSHORT,'d'},
	{0}
};
/* view into subsystem class, the subset of fields we will use */
static struct fieldview fvsubsys[]=
{
	{0,SRCsubsys.subsysname,sizeof(SRCsubsys.subsysname)},
	{0,SRCsubsys.grpname,sizeof(SRCsubsys.grpname)},
	{0,(char *)&SRCsubsys.contact,sizeof(SRCsubsys.contact)},
	{0}
};     
static struct objview vsubsys= {(char *)&SRCsubsys,fvsubsys};


/* view into subserver class, the subset of fields we will use */
static struct fieldview fvsubsvr[]=
{
	{0,SRCsubsvr.subsysname,sizeof(SRCsubsvr.subsysname)},
	{0,(char *)&SRCsubsvr.sub_code,sizeof(SRCsubsvr.sub_code)},
	{0}
};     
static struct objview vsubsvr= {(char *)&SRCsubsvr,fvsubsvr};

int cmdargs(argc,argv,host,subsystem,subsyspid,object,objname,env,args,argflags)
int argc;
char *argv[];
char *host;
short *object;
char *subsystem;
long *subsyspid;
char *objname;
char *env;
char *args;
char *argflags;
{
	int     rc;
	int     num_flags;
	char	criteria[256];
	char	criteria2[256];

	/* initalize argument view with pointers to data locations */
	argview[5].bufaddr=(char *)subsystem;
	argview[7].bufaddr=(char *)env;
	argview[8].bufaddr=(char *)objname;
	argview[9].bufaddr=(char *)subsyspid;
	argview[10].bufaddr=(char *)args;
	argview[12].bufaddr=(char *)host;

	/* command arguments and all are mutualy exclusive */
	if(args != (char *)0)
	{
		argview[11].flag=' ';
		argview[10].flag='a';
		*args='\0';
	}
	else 
	{
		argview[11].flag='a';
		argview[10].flag=' ';
	}

	if(env != (char *)0)
		*env='\0';
	*subsyspid=0;
	*objname = '\0';
	*subsystem = '\0';
	*host='\0';
	subsvrpid=0;
	all=0;

	/* parse the input line */
	rc=parseopt(argc,argv,argview,argflags);
	if(rc <= 0)
		return(SRC_PARM);

	/* Do we want to print a subserver record? */
	if(printsubserver)
	{
		/* only -T or -T -t may be entered */
		if(rc==1 || (rc==2 && (int)argview[4].newval))
		{
			strcpy(objname,subtype);
			Smitformat=PRINTSUBSERVER;
			return(SRC_OK);
		}
		return(SRC_PARM);
	}
	/* Do we want to print a subsystem record? */
	if(printsubsys)
	{
		/* only -S or -S -s or -S -d may be entered */
		if(rc==1 || (rc==2 && (int)argview[5].newval))
		{
			Smitformat=PRINTSUBSYSTEM;
			return(SRC_OK);
		}
		if(rc==2 && printdefaultsubsys)
		{
			Smitformat=PRINTDEFAULTSUBSYSTEM;
			return(SRC_OK);
		}

		return(SRC_PARM);
	}
	/* Do we want to print a notify record? */
	if(printnotify)
	{
		/* only -N or -N -n may be entered */
		if(rc==1 || (rc==2 && (int)argview[13].newval))
		{
			Smitformat=PRINTNOTIFY;
			strcpy(objname,notifyname);
			return(SRC_OK);
		}
		return(SRC_PARM);
	}

	/* default object is a subsystem */
	*object=SUBSYSTEM;

	/* force and cancel may not both be entered */
	if(argview[1].newval && argview[0].newval)
		return(SRC_PARM);

	/* get the number of flags that are sort of optionial */
	num_flags= rc - Stopforce - Stopcancel - Long - argview[7].newval - argview[10].newval - argview[12].newval;

	/* subsystem, group, subsyspid, subserver not specified on the command line? */
	if(num_flags < 1)
		return(SRC_PARM);

	/* where all subsystem specified? */
	if(all)
	{
		/* other parameters entered is an error */
		if(num_flags > 1 || (Nolong && Long))
			return(SRC_PARM);
		/* set subsystem name to be all subsystems */
		strcpy(subsystem,SRCALLSUBSYS);
		return(SRC_OK);
	}

	/* we have subserver entries */
	if(argview[4].newval)
	{
		/* may only enter one of
          	**    object_name
          	**    subserverpid
	  	** and one
	  	**    subsystempid
          	** all other flags entered are illegal
          	**/
		if(Stopcancel || (num_flags - (int)argview[8].newval - (int)argview[9].newval - (int)argview[3].newval) != 1)

			return(SRC_PARM);

		/* setup offsets for return of subserver fields */
		fvsubsvr[0].c_addr=(char *)subsystem;
		fvsubsvr[1].c_addr=(char *)object;

		/* key off of subtype */
		sprintf(criteria,"sub_type = '%s'",subtype);
		if((rc=src_odm_init()) < 0 ||
		  (rc=readrec(SRCSUBSVR,&vsubsvr,criteria,TRUE)) <= 0)
		{
			srcerr(odmerrset(rc),odmerrmap(rc,SRC_TYPE),SSHELL,subtype,0,0,0);
			src_odm_terminate(TRUE);
			return(-1);
		}
		src_odm_terminate(TRUE);

		/* subserver pid travels as a char string inside objname */
		if(argview[3].newval)
			sprintf(objname,"%ld",subsvrpid);

		return(SRC_OK);
	}

	/* can only have one flag entered now */
	if(num_flags != 1)
		return(SRC_PARM);

	/* subsyspid entered? */
	if(argview[9].newval)
		if(Justpid)
			return(SRC_OK);
		else
			return(SRC_PARM);

	/* must set offset in object view so we can get our data back from odm */
	fvsubsys[0].c_addr=(char *)subsystem;
	fvsubsys[1].c_addr=(char *)grpname;
	fvsubsys[2].c_addr=(char *)&Contact;

	/* subsytem name entered */
	if(argview[5].newval)
	{
		sprintf(criteria,"subsysname = '%s'",subsystem);
		sprintf(criteria2,"synonym = '%s'",subsystem);

		if((rc=src_odm_init()) < 0 ||
		  ((rc=readrec(SRCSYSTEM,&vsubsys,criteria,TRUE)) <= 0 &&
		   (rc=readrec(SRCSYSTEM,&vsubsys,criteria2,TRUE)) <=0))
		{
			src_odm_terminate(TRUE);
			srcerr(odmerrset(rc),odmerrmap(rc,SRC_SUBSYS),SSHELL,subsystem,0,0,0);
			return(-1);
		}
		src_odm_terminate(TRUE);
		return(SRC_OK);
	}

	/* group name entered */
	if(argview[6].newval)
	{
		/* is the long flag ilegal and was it specified? */
		if(Nolong && Long)
			return(SRC_PARM);
		sprintf(criteria,"grpname = '%s'",grpname);
		if((rc=src_odm_init()) < 0 ||
		  (rc=readrec(SRCSYSTEM,&vsubsys,criteria,TRUE)) <= 0)
		{
			srcerr(odmerrset(rc),odmerrmap(rc,SRC_GROUP),SSHELL,grpname,0,0,0);
			src_odm_terminate(TRUE);
			return(-1);
		}
		src_odm_terminate(TRUE);
		strcpy(subsystem,SRCGROUP);
		strcat(subsystem,grpname);
		return(SRC_OK);
	}

	/* some wierd parameter error */
	return(SRC_PARM);
}
