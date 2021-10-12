static char sccsid[] = "@(#)01	1.5  src/bos/usr/bin/ate/check.c, cmdate, bos411, 9428A410j 4/18/91 10:55:42";
/* 
 * COMPONENT_NAME: BOS check.c
 * 
 * FUNCTIONS: ckkey, cklength, ckpacing, ckparity, ckrate, ckredial, 
 *            ckstop, cktoggle, cktransfer 
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
/* Module name:      check.c                                                */
/* Description:      check program inputs from user, ate.def file or the    */
/*                     dialing directory file                               */
/* Functions:        ckrate - check speed (baud rate)                       */
/*                   cklength - check bits (word size)                      */
/*                   ckstop - check stop (stop bits)                        */
/*                   ckparity - check parity                                */
/*                   ckredial - check wait (seconds between redialing)      */
/*                     and attempts (number of times to redial)             */
/*                   cktransfer - check transfer (file transfer type)       */
/*                   ckpacing - check pacing (pacing character or interval) */
/*                   cktoggle - check xlatcr (linefeed), echo, vt100, write */
/*                     (capture), and xctl (xon/xoff control)               */
/*                   ckkey - check that cmdkey (command key), capkey        */
/*                     (capture key) and user interrupt key are ctrl keys   */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      alter in alter.c, direc in directory.c, getdef in      */
/*                     main.c                                               */
/*   Receives:       pointer to string, sometimes a letter                  */
/*   Returns:        -1 (error), 0                                          */
/*   Abnormal exits: none                                                   */
/*   Calls:          message in message.c (print user errors)               */
/*   Modifies:       argp[25]                                               */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

/* --------------------------------------------------------
   check rate   
   -------------------------------------------------------- */
ckrate(q)
char q[];
{
 int tmpr;

#ifdef DEBUG
kk = sprintf(ss,"entering ckrate\n");
write(fe,ss,kk);
#endif DEBUG

 tmpr = atoi(q);
 switch(tmpr)
   {
   case    50 :
   case    75 :
   case   110 :
   case   134 :
   case   150 :
   case   300 :
   case   600 :
   case  1200 :
   case  1800 :
   case  2400 :
   case  4800 :
   case  9600 :
   case 19200 : break;
   default    : strcpy(argp[25],q);         /* store string for msg print */
                message(43);
                return(ERROR);
   }
 return(0);
}

/* -----------------------------
   check length
   ----------------------------- */
cklength(q)
char q[];
{

#ifdef DEBUG
kk = sprintf(ss,"entering cklength with %s\n",q);
write(fe,ss,kk);
#endif DEBUG

 if (strcmp(q,"7") != 0 && strcmp(q,"8") != 0)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   message(44);
   return(ERROR);
   }
 else return(0);
}


/* ----------------------------
   check stop
   ---------------------------- */
ckstop(q)
char q[];
{

#ifdef DEBUG
kk = sprintf(ss,"entering ckstop\n");
write(fe,ss,kk);
#endif DEBUG

 if (strcmp(q,"1") != 0 && strcmp(q,"2") != 0)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   message(45);
   return(ERROR);
   }
 else return(0);
}

/* -------------------------
   check parity
   ------------------------- */
ckparity(q)
char q[];
 {
 
#ifdef DEBUG
kk = sprintf(ss,"entering ckparity\n");
write(fe,ss,kk);
#endif DEBUG

 if (strcmp(q,"0") != 0 && strcmp(q,"1") != 0 &&
     strcmp(q,"2") != 0 || strcmp(q,"")  == 0)
    {
    strcpy(argp[25],q);                     /* store string for msg print */
    message(46);
    return(ERROR);
    }
  else return(0);
}

/* -----------------------
   check redial
   ----------------------- */
ckredial(q,letter)
char q[],letter;
{
 int tmpr,error=0;
 char *p;

#ifdef DEBUG
kk = sprintf(ss,"entering redial with %c\n",letter);
write(fe,ss,kk);
#endif DEBUG

 p = q;
 while (*p != '\0' && *p != ' ')
   {
   if (isdigit(*p) != 0) p++;
   else
     {
     error = 1;
     break;
     }
   }

 tmpr = atoi(q);
 if ((tmpr < 0) || (tmpr > 999999999) || (strcmp(q,"") == 0))
   error = 1;

 if (error)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   if (letter == 'W') message(50);          /* bad wait value */
   if (letter == 'A') message(42);          /* bad attempts value */
   return(ERROR);
   }
 else return(0);
}

/* ---------------------------
   check transfer
   --------------------------- */
cktransfer(q)
char q[];
{

#ifdef DEBUG
kk = sprintf(ss,"entering cktransfer\n");
write(fe,ss,kk);
#endif DEBUG

  if (*q != 'x' && *q != 'X' && *q != 'p' && *q != 'P' || strlen(q) != 1)
    {
    strcpy(argp[25],q);                     /* store string for msg print */
    message(51);
    return(ERROR);
    }
  else
    {
    return(0);
    }
}

/* ------------------------------
   check pacing character
   ------------------------------ */
ckpacing(q)
char q[];
{
#ifdef DEBUG
kk = sprintf(ss,"entering ckpacing\n");
write(fe,ss,kk);
#endif DEBUG

 if (*q <= (char)0x00 || *q >= (char)0x7F || strlen(q) != 1)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   message(52);
   return(ERROR);
   }
 else return(0);
}

/* -------------------------
   check toggles
   ------------------------- */
cktoggle(q,letter)
char q[],letter;
{

#ifdef DEBUG
kk = sprintf(ss,"entering cktoggle with %c\n",letter);
write(fe,ss,kk);
#endif DEBUG

 if (strcmp(q,"0") != 0 && strcmp(q,"1") != 0)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   if (letter == 'L') message(48);          /* bad linefeed value */
   if (letter == 'E') message(47);          /* bad echo value */
   if (letter == 'V') message(53);          /* bad vt100 value */
   if (letter == 'W') message(55);          /* bad write (capture) value */
   if (letter == 'X') message(58);          /* bad xon/xoff value */
   return(ERROR);
   }
 else return(0);
}

/* ---------------------------
   check key
   --------------------------- */
ckkey(q) 
char q[];
{
int tmpr;
char *p;

#ifdef DEBUG
kk = sprintf(ss,"entering ckkey\n");
write(fe,ss,kk);
#endif DEBUG

p = q;
if (*p != '0')
  sscanf(q,"%d",&tmpr);                     /* decimal number */
else
  {
  p++;
  if (*p != 'x' && *p != 'X')               /* octal number */
    sscanf(q,"%o",&tmpr);
  else
    {
    p++;
    sscanf(p,"%x",&tmpr);                   /* hex number */
    }
  }

 if (tmpr < 000 || tmpr > 040)
   {
   strcpy(argp[25],q);                      /* store string for msg print */
   message(59);
   return(ERROR);
   }
 else return(tmpr);
}
