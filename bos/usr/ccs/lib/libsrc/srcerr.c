static char sccsid[] = "@(#)21	1.9  src/bos/usr/ccs/lib/libsrc/srcerr.c, libsrc, bos411, 9428A410j 2/26/91 14:54:41";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcerr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


/*
** IDENTIFICATION:
**    Name:	srcerr
**    Title:	Display SRC message or subsystem defined message
** PURPOSE:
**	To display SRC messages from the SRC message catalog or
**	display the subsystem defined error message.
** 
** SYNTAX:
**    srcerr(err_type, msg_id, type, msgtxt1, msgtxt2, msgtxt3, defmsg)
**    Parameters:
**	i int err_type - error set, SRC or other
**	i int msg_id - message number to be printed
**	i int type - shell command message or daemon logging of message
**	i char *msgtxt1 - first text to be inserted in cataloged message
**	i char *msgtxt2 - second text to be inserted in cataloged message
**	i char *msgtxt3 - third text to be inserted in cataloged message
**	i char *defmsg - default message to be printed logged in the
**		event that no message exists in the catalog
**		normaly will be a subsystem defined message
**
** INPUT/OUTPUT SECTION:
**	Message will be displayed through stdout/stderr for a shell command
**	Message will be logged with error loggin daemon for daemon processes
**
** PROCESSING:
**	Get a message from the SRC catlog of fine user messages.
**	Display the message or log the message.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**
** RETURNS:
**
**/
#include "src.h"
#include <sys/syslog.h>
#include <nl_types.h>
#include <limits.h>

void srcerr(err_type, msg_id, type, msgtxt1, msgtxt2, msgtxt3, defmsg)
int err_type;	/* whose error SRC/ODM other */
int msg_id;	/* error number */
int type;	/* error from where shell or deamon */
char *msgtxt1;	/* message token 1 */
char *msgtxt2;	/* message token 2 */
char *msgtxt3;	/* message token 3 */
char *defmsg;	/* default message */
{
	int errset;
	int errindex;
	char *text;
	char odmnum[20];
	int odm_err_msg();
	int src_err_msg();
	int rc;


	if(err_type == SRC_BASE)
	{
		if(src_err_msg(msg_id,&text) == -1)
		{
			printf("SRC Message or Error code: %d\n",msg_id);
			return;
		}
	}
	else if(err_type == ODM_BASE)
	{
		if(odm_err_msg((long)msg_id,&text) == -1)
		{
			if(src_err_msg(SRC_ODMERR,&text) == -1)
			{
				printf("ODM Error code: %d\n",msg_id);
				return;
			}
			sprintf(odmnum,"%d",msg_id);
			msgtxt1=odmnum;
		}

	}
	else 
	{
		if(msg_id == SRC_SUBMSG)
			text=defmsg;
		else if(src_err_msg(msg_id,&text) == -1)
			text=defmsg;

		if(text==0 || *text==0)
		{
			printf("Unknown message or error code: %d\n",msg_id);
			return;
		}
	}


	if(msgtxt1 == 0)
		msgtxt1="";
	if(msgtxt2 == 0)
		msgtxt2="";
	if(msgtxt3 == 0)
		msgtxt3="";

	if(type == SSHELL)
		printf(text,msgtxt1,msgtxt2,msgtxt3);
	else
		syslog(LOG_DAEMON|LOG_WARNING,text,msgtxt1,msgtxt2,msgtxt3);

}
