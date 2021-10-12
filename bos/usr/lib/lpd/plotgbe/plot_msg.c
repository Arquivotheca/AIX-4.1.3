static char sccsid[] = "@(#)83	1.3  src/bos/usr/lib/lpd/plotgbe/plot_msg.c, cmdpsla, bos411, 9428A410j 6/15/90 17:40:03";
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOT_MSG.                                           */
/*                                                                      */
/*  ORIGIN: IBM                                                         */
/*                                                                      */
/*. (C) Copyright International Business Machines Corp. 1989, 1990      */
/*. All Rights Reserved                                                 */
/*. Licensed Materials - Property of IBM                                */
/*.                                                                     */
/*. US Government Users Restricted Rights - Use, Duplication or         */
/*. Disclosure Restricted By GSA ADP Schedule Contract With IBM CORP.   */
/*                                                                      */
/* PURPOSE:                                                             */
/*          Plotter support common message module for PSLA.             */
/*                                                                      */
/* MODIFICATIONS:                                                       */
/* 02/13/90 MJ First argument, terminal pointer was removed.	        */
/*             All instances of 'fprintf' and 'log_message' were        */
/*             changed to 'telluser()'.				        */
/*             's_mailonly ' field of the external structure 'stfile'   */
/*             indicates whether the message is sent to local mail box  */
/*	       only or may be displayed on the plot requesting          */
/*	       terminal also.					        */
/* 03/24/90 bb Add NLS support.                                         */
/* 05/19/90 bb Remove all references to 'stfile'.                       */
/************************************************************************/                                                           
                                                                                                                                    
#include <errno.h>                                                                                                                  
#include <stdio.h>                                                                                                                  
#include <fcntl.h>                                                                                                                  
#include "plot_def.h"                                                                                                               
#include <sys/types.h>                                                                                                              

#include <locale.h>
#include <nl_types.h>
#include <limits.h>
#include "plotgbe_msg.h"                /* generated from plotgbe.msg   */

extern nl_catd catd;                    /* for NLS message catalog      */

plot_msg(msgnum,list,var1)                                                                                                  
int msgnum;                                                                                                                         
char *list;                                                                                                                         
char *var1;                                                                                                                         
{                                                                                                                                   
        char msgbuf[BUFSIZ];

        switch(msgnum)                                                                                                              
        {                                                                                                                           
	    case(VIEW):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NOTREADY,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_VIEW,PSLA_DFLT_MSG));
                break;                                                                                                              

	    case(NOPAPER):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NOTREADY,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_NOPAPER,PSLA_DFLT_MSG));
                break;                                                                                                              
                                                                                                                                    
	    case(OPEN_PORT):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_OPENPORT,PSLA_DFLT_MSG),
			list,errno);
                telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(BAD_OPTION):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_BADOPTION,PSLA_DFLT_MSG),
			var1);
                telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(BAD_FR):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_BAD_FR,PSLA_DFLT_MSG));
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(ORG_ERR):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_ORG_ERR,PSLA_DFLT_MSG),
			list,errno);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(OPEN_FILE):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_OPEN_FILE,PSLA_DFLT_MSG),
			list,errno);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(WR_PORT):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_WR_PORT,PSLA_DFLT_MSG),
			list,errno);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(RD_FILE):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_RD_FILE,PSLA_DFLT_MSG),
			list,errno);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(REMOVE_PAPER):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_RMVPAPER,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(RS232_ERROR):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_RS232_ERR,PSLA_DFLT_MSG),
                        var1,list);
                telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(CLOSE):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NOTREADY,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_CLOSEPEN,PSLA_DFLT_MSG));
                break;                                                                                                              
                                                                                                                                    
	    case(CLOSE_VIEW):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NOTREADY,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_CLOSEVIEW,PSLA_DFLT_MSG));
                break;                                                                                                              
                                                                                                                                    
	    case(NOPAPER_CLOSE):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NOTREADY,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_NOPAPERCLOSE,PSLA_DFLT_MSG));
                break;                                                                                                              
                                                                                                                                    
	    case(TIME_OUT):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_TIME_OUT,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(NO_RESPONSE):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_NORESPONSE,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_READY,PSLA_DFLT_MSG));
                break;                                                                                                              
                                                                                                                                    
	    case(CANCEL):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_CANCEL,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(ENA_QERR):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_ENA_QERR,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(BAD_SPEED):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_BAD_SPEED,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    case(CONTINUE):
		telluser(catgets(catd,PSLA_MISC_MSG,PSLA_CONTINUE,PSLA_DFLT_MSG));
                break;                                                                                                             

	    case(STAT_ERR):
		sprintf(msgbuf,
			catgets(catd,PSLA_MISC_MSG,PSLA_STAT_ERR,PSLA_DFLT_MSG),
			list);
		telluser(msgbuf);
                break;                                                                                                              
                                                                                                                                    
	    default:
                break;                                                                                                              
        } 
        return;
}                                                                                                                                   
