static char sccsid[] = "@(#)57	1.8  src/bos/usr/ccs/lib/libsrc/parseopt.c, libsrc, bos411, 9428A410j 2/26/91 14:54:28";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	parseopt,cpydata,notnum
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#include <stdio.h>
#include <ctype.h>
#include <odmi.h>
#include "srcopt.h"
#include "src.h"

extern char *optarg;
extern int optind;
static int cpydata();
static int notnum();

/*
** IDENTIFICATION:
**    Name:	parseopt
**    Title:	Parce Command Options
** PURPOSE:
**    To parse input commands and convert input arguments of each flag
**    to the proper data type.
**
**    Note: this functions assumes that each flag has one input argument.
** 
** SYNTAX:
**    parseopt(argc,argv,argview,argflags)
**    Parameters:
**       i char *argv[];
**       i int argc;
**       i char *argflags;
**       u struct argview argview[];
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**    Globals:
**	u char *optarg;
**	u int optind;
**
** RETURNS:
**
**/
int parseopt(argc,argv,argview,argflags)
char *argv[];
int argc;
char *argflags;
struct argview argview[];
{
	int flag;
	int atoi();
	int i;
	int flgcnt=0;
	
	flag=getopt(argc,argv,argflags);

	/* process all the flags and there arguments */
	while(flag != EOF)
	{

		/* for each flag search for it's argview array element */
		for(i=0; argview[i].size != 0;i++)
		{
			/* the element exits so copy data from optarg 
			** into the proper datatype for this argument
			**/
			if(argview[i].flag == flag && argview[i].newval == 0)
			{
				if(!cpydata(&argview[i],optarg))
					return(FALSE);
				flgcnt++;

				/* mark argument as being entered */
				argview[i].newval++;
				break;
			}
		}

		/* passed an illegal argument? or pass same argument twice */
		if(argview[i].size == 0)
			return(FALSE);

		flag=getopt(argc,argv,argflags);
	}
	/* error if we have any extra arguments with out flags */
	if(optind != argc)
		return(FALSE);

	return(flgcnt);
}
static int cpydata(argview,source)
struct argview *argview;
char *source;
{
	long atol();
	int atoi();
	int strsz;

	/* copy argument to usr buff (by data type)*/
	switch(argview->type)
	{
	case FLAGFIELD:	/* flagfields have no arguments */
		break;
	case FLAGLONG:	/* flagfields have no arguments */
		*(long *)argview->bufaddr = (long)TRUE;
		break;
	case FLAGSHORT:	/* flagfields have no arguments */
		*(short *)argview->bufaddr = (short)TRUE;
		break;
	case ODM_LONG:
		if(notnum(source))
			return(FALSE);
		*(long *)argview->bufaddr = atol(source);
		if((argview->max && argview->max < *(long *)argview->bufaddr) ||
		  argview->min > *(long *)argview->bufaddr)
		{
			srcerr(SRC_BASE,(int)argview->errno,SSHELL,0,0,0,0);
			return(FALSE);
		}
		break;
	case ODM_SHORT:
		if(notnum(source))
			return(FALSE);
		*(short *)argview->bufaddr = (short)atoi(source);
		if((argview->max && argview->max < *(short *)argview->bufaddr) ||
		  argview->min > *(short *)argview->bufaddr)
		{
			srcerr(SRC_BASE,(int)argview->errno,SSHELL,0,0,0,0);
			return(FALSE);
		}
		break;
	case ODM_METHOD:
		strsz=strlen(source);
		if(argview->max < strsz || argview->min > strsz)
		{
			srcerr(SRC_BASE,(int)argview->errno,SSHELL,0,0,0,0);
			return(FALSE);
		}
		strcpy(argview->bufaddr,source);
		break;
	default:
		strsz=strlen(source);
		if(argview->max < strsz || argview->min > strsz)
		{
			srcerr(SRC_BASE,(int)argview->errno,SSHELL,0,0,0,0);
			return(FALSE);
		}
		strcpy(argview->bufaddr,source);
		break;
	}
	return(TRUE);
}
static int notnum(source)
char *source;
{
	int i;
	
	/* find the first char that is not a digit */
	for(i=0;source[i]!=0 && isdigit((int)source[i]);i++);

	if(source[i]=='\0')
		return(FALSE);
	else
		return(TRUE);
		
}
