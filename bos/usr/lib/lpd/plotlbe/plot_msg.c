static char sccsid[] = "@(#)41	1.1  src/bos/usr/lib/lpd/plotlbe/plot_msg.c, cmdplot, bos411, 9428A410j 4/17/91 08:51:25";
/*
 * COMPONENT_NAME: (CMDPLOT) ported RT plotter code
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***********************************************************************/
/*                                                                     */
/* Module Name : PLOT_MSG                                              */
/*                                                                     */
/* Date: 09/03/85                                                      */
/*                                                                     */
/* (c) Copyright 1985, IBM Corporation                                 */
/*                                                                     */
/* Description:                                                        */
/*          Plotter support common message module                      */
/*                                                                     */
/* 03/17/91 - CM changed all log_message calls and fprintf(term_fp...  */
/*               messages to telluser(), removed first parameter       */
/*               'term_fp'. NLS support w/msg catd not added yet.      */
/*                                                                     */
/***********************************************************************/
 
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include "plot_def.h"
#include <sys/types.h>
plot_msg(msgnum,list,var1)
int msgnum;
char *list;
char *var1;
{
	char msgbuf[BUFSIZ];
 
	switch(msgnum)
	{
		case(VIEW):
		telluser("Device attached to '%s' is not ready\n",list);
		sprintf(msgbuf,"   Press REMOTE or VIEW to ready device\n");
                telluser(msgbuf);
		break;
 
		case(NOPAPER):
		sprintf(msgbuf,"   Load media and ready device\n");
                telluser(msgbuf);
		break;
 
		case(OPEN_PORT):
		sprintf(msgbuf,"Unable to open port '%s' error returned = %d\n",
		        list,errno);
                telluser(msgbuf);
		break;
 
		case(BAD_OPTION):
		sprintf(msgbuf,"Invalid option '%s' on print command\n",var1);
                telluser(msgbuf);
		break;
 
		case(BAD_FR):
		sprintf(msgbuf,"'fr' option invalid for non-roll feed \n");
                telluser(msgbuf);
		break;
 
		case(ORG_ERR):
		sprintf(msgbuf,
		    "Error setting origin of device attached to '%s' error returned = %d\n",
		    list,errno);
                telluser(msgbuf);
		break;
 
		case(OPEN_FILE):
               sprintf(msgbuf,"Error opening file '%s' - error returned = %d\n",
		    list,errno);
                telluser(msgbuf);
		break;
 
		case(WR_PORT):
		sprintf(msgbuf,"Error writing to '%s' - error returned = %d\n",
		        list,errno);
                telluser(msgbuf);
		break;
 
		case(BAD_ACK):
		sprintf(msgbuf,"Unrecognized acknowledgment '%d' from '%s'\n",
		        var1,list);
                telluser(msgbuf);
		break;
 
		case(RD_FILE):
	       sprintf(msgbuf,"Error reading from '%s' - error returned = %d\n",
	               list,errno);
                telluser(msgbuf);
		break;
 
		case(REMOVE_PAPER):
	        sprintf(msgbuf,"Remove media from device attached to '%s' \n",
	                list);
                telluser(msgbuf);
		return;
		break;
 
		case(RS232_ERROR):
		sprintf(msgbuf,"RS232 error status = %s for device '%s'\n",
		        var1,list);
                telluser(msgbuf);
		break;
 
		case(CLOSE):
		telluser("Device attached to '%s' is not ready\n",list);
		sprintf(msgbuf,"   Close pen cover to ready device\n");
                telluser(msgbuf);
		break;
 
		case(CLOSE_VIEW):
		telluser("Device attached to '%s' is not ready\n",list);
		sprintf(msgbuf,
		    "   Close pen cover and press REMOTE or VIEW to ready device\n");
                telluser(msgbuf);
		break;
 
		case(NOPAPER_CLOSE):
		telluser("Device attached to '%s' is not ready\n",list);
		sprintf(msgbuf,"   Load media and close pen cover to ready device\n");
                telluser(msgbuf);
		break;
 
		case(BAD_STATUS):
		telluser("Device attached to '%s' is not ready\n",list);
		sprintf(msgbuf,"   Unrecognized status '%c' from device \n",var1);
                telluser(msgbuf);
		break;
 
		case(RD_PORT):
		sprintf(msgbuf,
		    "Error reading from '%s' - error returned = %d\n",
		    list,errno);
                telluser(msgbuf);
		break;
 
		case(TIME_OUT):
		sprintf(msgbuf,
		    "Device attached to '%s' failed to respond - check cable, power, speed \n",
		    list);
                telluser(msgbuf);
		break;
 
		case(NO_RESPONSE):
		telluser("No response from device attached to '%s'\n",
		          list);
		telluser(
		    "   Ready device to continue or cancel job\n");
		return;
		break;
 
		case(CANCEL):
		sprintf(msgbuf,"Processing of file '%s' cancelled\n",list);
                telluser(msgbuf);
		break;
 
		case(ENA_QERR):
		sprintf(msgbuf,"Error enabling queue for device '%s'\n",list);
                telluser(msgbuf);
		break;
 
		case(BAD_SPEED):
		sprintf(msgbuf,"Speed operand '%s' is not valid\n",list);
                telluser(msgbuf);
		break;
 
		case(CONTINUE):
		sprintf(msgbuf,
		    "   Load new media to continue\n",list);
                telluser(msgbuf);
		return;
 
		case(STAT_ERR):
		sprintf(msgbuf,"Unable to get file status 's'\n",list);
                telluser(msgbuf);
		break;
 
	default:
		break;
	}
 
	return;
}
