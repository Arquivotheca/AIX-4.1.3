static char sccsid[] = "@(#)12	1.7  src/bos/usr/ccs/lib/libsrc/srcmsg.c, libsrc, bos411, 9428A410j 12/2/93 17:29:57";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	src_err_msg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
** IDENTIFICATION:
**    Name:	src_err_msg
**    Title:	Retreive SRC message 
** PURPOSE:
**	To Retreive SRC error message
** 
** SYNTAX:
**    char *src_err_msg(srcerrno)
**    Parameters:
**	i int srcerrno - src error message
** 	u char **msgtxt - pointer to a char pointer for the error text
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**	Get a message from the SRC catlog of fine user messages.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** RETURNS:
**	char * to error message
**
**/
#include "src.h"
#include <macros.h>
#include <nl_types.h>

char *src_get_msg(int set_id,int msg_id,char * def_msg)
{
	nl_catd cat_id;
	char *cat_msg;
	static char *ret_msg=NULL;

	cat_id=catopen(SRCMSGCAT,NL_CAT_LOCALE);
	if(cat_id==(nl_catd)-1)
		return(def_msg);
	cat_msg=catgets(cat_id,set_id,msg_id,def_msg);
	if(ret_msg!=NULL)
	{
		ret_msg=realloc(ret_msg,strlen(cat_msg)+1);
		if(ret_msg==NULL)
			return(cat_msg);
	}
	else
	{
		ret_msg=malloc(strlen(cat_msg)+1);
		if(ret_msg==NULL)
			return(cat_msg);
	}
	strcpy(ret_msg,cat_msg);
	catclose(cat_id);
	return(ret_msg);
}

#define SRCMSGNO(msg_id) (abs(msg_id)-(SRC_BASE))
#define SRCMSG(msg_id) src_def_msg[(abs(msg_id-(FIRST_SRC_ERROR)))]
extern char *src_def_msg[];
int src_err_msg(srcerrno,msgtxt)
int srcerrno;	/* error number */
char **msgtxt;  /* message text */
{
	/* Do we have an illegal message ?
	 * the src error messages are all negative 
	 */
	if(srcerrno > FIRST_SRC_ERROR || srcerrno < LAST_SRC_ERROR)
		return(-1);

	*msgtxt=src_get_msg(SRCERRSET,SRCMSGNO(srcerrno),SRCMSG(srcerrno));
	if(msgtxt==0 || *msgtxt==0)
		return(-1);
	return(0);

}
