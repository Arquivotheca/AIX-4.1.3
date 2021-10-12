static char sccsid[] = "@(#)07	1.5  src/bos/usr/bin/ate/hangup.c, cmdate, bos411, 9428A410j 4/18/91 10:56:29";
/* 
 * COMPONENT_NAME: BOS hangup.c
 * 
 * FUNCTIONS: hangup 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/****************************************************************************/
/*                                                                          */
/* Module name:      hangup.c                                               */
/* Description:      end a connection                                       */
/* Functions:        hangup - end connection                                */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c (user selects terminate or quit)      */
/*                   sigrout in signal.c (user interrupt, software          */
/*                     termination or hangup signal)                        */
/*                   conn in connect.c (establish & maintain a connection)  */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          initmod in setups.c (initialize modem)                 */
/*                   retopt in setups.c (restore original terminal settings)*/
/*                   message in message.c (display user messages)           */
/*   Modifies:       connect, lpath, menu, retcode, txti                    */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

/* char * sbuf;                      screen buffer ptr - modem.h  */

/* -------------------------------------------------------------------------
   hangup - terminate a connection
   ------------------------------------------------------------------------- */
hangup ()
{
 int rc,status;

#ifdef DEBUG
kk = sprintf(ss,"entering hangup\n");
write(fe,ss,kk);
#endif DEBUG

 if(lpath>=0)				/* port is open */
   {
   if (txti != 0)                       /* there is captured data */
     {
     if (write(fk,bufr,txti) == ERROR)
       message(26);                     /* can't write to capture file */
     txti = 0;
     }
   (void) free(sbuf);                   /* free allocated memory */
   if (vt100) vtclose();		/* close the vt100 screen */
   initmod(lpath,OFF,ON,0,0,0,0);	/* set port parameters to hangup */
   close(lpath);			/* close the port */
   lpath=ERROR;
   if(locfile) {
     ttyunlock(port);			/* get rid of lock file */
     locfile=OFF;                        /* reset lockfile indicator */
   }
 }
 connect=OFF;				/* no connection */
 menu=0;				/* no menu is being displayed */
 if (retcode != QUIT)
   retcode=EXIT;			/* set retcode to leave cmd() */
 retopt();				/* restore original terminal options */
 
#ifdef DEBUG
kk = sprintf(ss,"leaving hangup\n");
write(fe,ss,kk);
#endif DEBUG

return;

}
