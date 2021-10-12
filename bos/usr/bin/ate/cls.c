static char sccsid[] = "@(#)02	1.4  src/bos/usr/bin/ate/cls.c, cmdate, bos411, 9428A410j 4/18/91 10:55:49";
/* 
 * COMPONENT_NAME: BOS cls.c
 * 
 * FUNCTIONS: PUT_CH, cls 
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
/* Module name:      cls.c                                                  */
/* Description:      clear screen using curses                              */
/* Functions:        cls() - clear screen                                   */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */ 
/*                   curses library                                         */
/*                   <term.h> - termrc                                      */ 
/*   Called by:      direc in directory.c; help in help.c; mainmenu,        */
/*                     altermenu, modifymenu, prompt in menu.c; message     */
/*                     in message.c; redraw, message in portrw.c, sigrout   */
/*                     in signal.c, cmd in command.c, and vtclose in        */
/*                     portvt.c                                             */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          nothing                                                */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/
#include "modem.h"
#define HZ HZ1
#include <cur00.h>
#include <term.h>

void PUT_CH(c)  char c;  { write (1, &c, 1); }

extern int termrc;

/*----------------------------------------------------------------------*/
/* Clear screen using terminfo						*/
/*----------------------------------------------------------------------*/
cls ()
{
  int rc;			/* return code */
  int i;			/* loop variable */
  char buf[24];			/* char buffer to hold clear string */

#ifdef DEBUG
kk = sprintf(ss,"entering cls\n");
write(fe,ss,kk);
#endif DEBUG

  if (termrc != 1)		/* TERM undefined; do something crude */
    {
    for (i = 25; i > 0; i--)  PUT_CH('\n');
    }
  else 
    {
    tputs(clear_screen, 1, PUT_CH);
    }

#ifdef DEBUG
kk = sprintf(ss,"leaving cls\n");
write(fe,ss,kk);
#endif DEBUG
}
