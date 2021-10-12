static char sccsid[] = "@(#)30	1.8  src/bos/usr/bin/ate/vt100.c, cmdate, bos411, 9428A410j 12/15/93 15:56:15";
/* 
 * COMPONENT_NAME: BOS vt100.c
 * 
 * FUNCTIONS: if, vt1, vt3, vt4, vt5, vt7 
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
/* Module name:      vt100.c                                                */
/* Description:      convert G-keyboard sequences to vt100-equivalent       */
/*                     sequences before they are written to the port.       */
/*                     Remap keypad and PF keys 1-4 on G-keyboard.          */
/* Functions:        vt1 - convert sailboat sequences to vt100 sequences    */
/*                   vt3 - map number pad to shifted state (numbers)        */
/*                   vt4 - map number pad to keypad appl mode (vt100)       */
/*                   vt5 - map number pad to base state (G keyboard)        */
/*                   vt7 - map number pad to keypad appl mode (vt52)        */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      conn in connect.c                                      */
/*                   modify in modify.c                                     */
/*                   main in main.c                                         */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          nothing;                                               */
/*   Modifies:       keyappl, kbdata                                        */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
#ifdef HFT
#include <sys/hft.h>
#endif /* HFT */

#define esc 0x1b
#define del 0x7f

int rc;					/* return code */
int rdmask;                             /* file ready to read */ 

/* -------------------------------------------------------------------------
   This program segment is called when an escape character is typed from
   the sailboat's keyboard and when vt100 emulation is set to ON.  This
   will continue reading the keyboard until an end of sequence character
   is received (hex value greater than 0x40) and then it will write the
   equivalent VT100 sequence, modified or not, to the port (a DEC host?).
   ------------------------------------------------------------------------- */
vt1()
{
 int i=0;
 char s[16],ns[16],c;                       /* sailboat string, vt100 string */
 s[0] =  '\0';                              /* clear string */
 ns[0] = '\0';                              /* clear string */
 
#ifdef DEBUG
kk = sprintf(ss,"\nentering vt1\n");
write(fe,ss,kk);
#endif DEBUG

rdmask=(1<<0) | (1<<lpath);                 /* check stdin, port */
if (rdmask & (1<<0))                        /* if data on standard in */
{
alarm(1); 

#ifdef DEBUG
kk = sprintf(ss,"\nvt100.c:  data on standard in\n");
write(fe,ss,kk);
#endif DEBUG

if ((rc=read(0,&kbdata,1)) > 0)		    /* read 1st char after escape */
{
 alarm(0);  

#ifdef DEBUG
kk = sprintf(ss,"\nvt100.c:  data on keyboard\n");
write(fe,ss,kk);
if (kbdata < 0x20 || kbdata > 0x73)
  kk = sprintf(ss,"vt100.c KEYBOARD:  0%o \n",kbdata);
else kk = sprintf(ss,"vt100.c KEYBOARD: %c \n", kbdata);
write(fe,ss,kk);
#endif DEBUG

 /*---------------------------------------------------------------------*/
 /* sequence may be 15 characters after esc [				*/
 /* 0x20 < kbdata < 0x40 implies kbdata is an integer (0 thru 9) or	*/
 /*         ! " # $ % & ' ( ) * + , - . / : ; < = > ?			*/
 /* hex>40 (a letter) is final character 				*/
 /*---------------------------------------------------------------------*/
 if ( kbdata == 0x5b)                       /* sequence starts with esc [ */
   {
#ifdef DEBUG
kk = sprintf(ss,"vt1 sequence starts with esc-[\n");
write(fe,ss,kk);
#endif DEBUG
       
    while ( (kbdata < 0x40 && kbdata > 0x20 && i < 15) ||
            (kbdata == 0x5b && i == 0) )    /* [ is first and only first char. */
      {
      s[i++] = kbdata;                      /* save character */
      read(0,&kbdata,1);		    /* read next character */
      }

    s[i] = kbdata;                          /* save final character */
    s[++i] = '\0';                          /* terminate the string */

#ifdef DEBUG
kk = sprintf(ss,"old s = %-16s\n",s);
write(fe,ss,kk);
#endif DEBUG

    if (strcmp(s,"[0W") ==0)                /* HTS (horizontal tab set) */
      strcpy(ns,"H");
    if (strcmp(s,"[2W") ==0)                /* TBC (tab clear at curr pos */
      strcpy(ns,"[0g");
    if (strcmp(s,"[5W") ==0)                /* TBC (tab clear-all horiz stops */
      strcpy(ns,"[3g");
    if (strcmp(s,"[1E") ==0)                /* NEL (next line) */
      strcpy(ns,"E");
    if (strcmp(s,"[u")  ==0)                /* DECRC (DEC restore cursor) */
      strcpy(ns,"8");
    if (strcmp(s,"[s")  ==0)                /* DECSC (DEC save cursor) */
      strcpy(ns,"7");
    if (strcmp(s,"[001q")  ==0)		    /* Function key 1 */
      strcpy(ns,"OP");
    if (strcmp(s,"[002q")  ==0)             /* Function key 2 */
      strcpy(ns,"OQ");
    if (strcmp(s,"[003q")  ==0)		    /* Function key 3 */
      strcpy(ns,"OR");
    if (strcmp(s,"[004q")  ==0)		    /* Function key 4 */
      strcpy(ns,"OS");

    if ((keyappl & 0x04)==0x04)             /* iff vt52 mode ON */
      {
#ifdef DEBUG
kk = sprintf(ss,"vt52 mode ON.  Doing compares\n");
write(fe,ss,kk);
#endif DEBUG
      if (strcmp(s,"[A")==0)                /* CUU (cursor up) */
        strcpy(ns,"A");
      if (strcmp(s,"[B")==0)                /* CUD (cursor down) */
        strcpy(ns,"B");
      if (strcmp(s,"[C")==0)                /* CUF (cursor forward */
        strcpy(ns,"C");
      if (strcmp(s,"[D")==0)                /* CUB (cursor backward) */
        strcpy(ns,"D");
      }
    else if ((keyappl & 0x02)==0x02)       /* iff cursor appl. ON, ck cursor */
      {
#ifdef DEBUG
kk = sprintf(ss,"vt100 mode ON; cursor appl on!\n");
write(fe,ss,kk);
#endif DEBUG
      if (strcmp(s,"[A")==0)                /* CUU (cursor up) */
        strcpy(ns,"OA");
      if (strcmp(s,"[B")==0)                /* CUD (cursor down) */
        strcpy(ns,"OB");
      if (strcmp(s,"[C")==0)                /* CUF (cursor forward */
        strcpy(ns,"OC");
      if (strcmp(s,"[D")==0)                /* CUB (cursor backward) */
        strcpy(ns,"OD");
      }
                        
#ifdef DEBUG
if (strcmp(ns,"") == 0) 
  {
  kk = sprintf(ss,"no match found for esc%s\n",s);
  write(fe,ss,kk);
  }
#endif DEBUG

   }                                        /* end of kbdata (0)  == 0x5b */

 /*---------------------------------------------------------------------*/
 /* sequence may be 15 characters after esc  				*/
 /* 0x20 <= kbdata < 0x30 implies kbdata is a space or			*/
 /*     ! " # $ % & ' ( ) * + , - . / : ; < = > ?			*/
 /* hex>30 (a number, a letter, or : ; < = > ? @) is final character    */
 /*---------------------------------------------------------------------*/
 else                                       /* sequence does not start esc [ */
   {
#ifdef DEBUG
kk = sprintf(ss,"vt1 sequence starts with lonely esc\n");
write(fe,ss,kk);
#endif DEBUG
       
/*   while (kbdata < 0x30 && kbdata >= 0x20 && i < 15)
     {
     s[i++] = kbdata;                    
     read(0,&kbdata,1);			
     }
*/
   s[i] = kbdata;                           /* save final character */
   s[++i] = '\0';                           /* terminate the string */
     
#ifdef DEBUG
kk = sprintf(ss,"old s = %-16s\n",s);
write(fe,ss,kk); 
#endif DEBUG

    if ((keyappl & 0x04)==0x04)             /* iff vt52 mode ON */
      {
#ifdef DEBUG
kk = sprintf(ss,"vt52 mode ON.  Doing compares\n");
write(fe,ss,kk);
#endif DEBUG
      if (strcmp(s,"O")==0)                 /* PF1 - PF4 */
        {
        return;				    /* leave so old seq not written */
        }
     }

   if (kbdata=='L' && i==1)                /* RI (reverse index) */
     strcpy(ns,"M");

   if (kbdata=='=' && i==1)                /* enter KEYPAD APPLICATION MODE */
     {
     keyappl = (keyappl | 0x0001);         /* set keypad appl bit ON */
#ifdef HFT
     if (keyappl & 0x04)		   /* vt52 mode is ON */
	vt7();				   /* remap for vt52  keypad appl */
     else vt4();			   /* remap for vt100 keyapd appl */
#endif /* HFT */
     }

   if (kbdata=='>' && i==1)                /* exit KEYPAD APPLICATION MODE */
     {
     keyappl = (keyappl & 0xFFFE);         /* set keypad appl bit OFF */
#ifdef HFT
     vt3();                  		   /* remap keypad to shifted state */
#endif /* HFT */
     }
   }                                       /* end else sequence without [  */
  }                                        /* end if data from keyboard */  
}                                          /* end if data from standard in */
 /*---------------------------------------------------------------------*/
 /* Write vt100 data stream to port.  The ESC was written to port as    */
 /* soon as it was read in connect.c                                    */
 /*---------------------------------------------------------------------*/

 if (strcmp(ns,"") != 0)
   {
#ifdef DEBUG
kk = sprintf(ss,"\nincoming s = %s  new s = %s\n", s,ns);
write(fe,ss,kk);
#endif DEBUG

   i = 0;
   while (ns[i] != (char)NULL)                   /* while not end of string */
     write (lpath,&ns[i++],1);             /* write new string to port */
   }
 else
   {
   if (strcmp (s,"") != 0)                 /* no substitution of characters */
     {
     i =0;
     while (s[i] != (char)NULL)                  /* while not end of string */
       write(lpath,&s[i++],1);             /* write old string to port */
     }
   }
#ifdef DEBUG
kk = sprintf(ss,"leaving vt1\n");
write(fe,ss,kk);
#endif DEBUG

} /* end of pgm */


#ifdef HFT  /* Don't remap keys without hft functionality present.   */
/*------------------------------------------------------------------------*/
/*       VT100's  By Brent M. Herling  latest modification 3/25/85        */
/*
   The following information and map arrays are used for remapping the
   G-keyboard number pad and PF keys 1-4.  The source of this information
   is (a) Virtual Terminal Application Interface CIS by Rudy Chukran,
   and (b) KSR Keyboard Data Stream Specification, World Trade keyboards,
   by Frank Waters.
   
   When the user first toggles the vt100 flag on, the number pad is
   remapped so that numbers are produced in the unshifted (base) state
   rather than the normal graphics characters.  The PF keys are also
   remapped at this time.  This is vt3 using map1.
   
   If an escape sequence is then received (user entered or received on the
   port) to turn on "keypad application mode", the keyboard is remapped
   again so that the number pad produces vt100 function sequences.  The PF 
   keys remain remapped with no further changes.  This is vt4 using map2.
   
   When the user exits the program, the number pad and PF keys are remapped
   back to their original state.  This is vt5 using map3.
 
                       ********************
 
   The code was changed July 14, 1987 to remap the DEL key also.  
                                                                           */
/***************************************************************************
*                                                                          *
* NUMERIC KEYPAD     DEFAULT         REMAPPED        APPLICATION           *
* KEY / POSITION    CODE POINT      CODE POINT       CODE STRING           *
*                                                                          *
* ENTER   108           13              13             esc OM              *
*  -      105           45              45             esc Om    (dash)    *
*  .      104          196              46             esc On    (period)  *
*  /       95           47              47             esc Ol replaces "," *
*  0       99          179              48             esc Op              *
*  1       93          192              49             esc Oq              *
*  2       98          193              50             esc Or              *
*  3      103          217              51             esc Os              *
*  4       92          195              52             esc Ot              *
*  5       97          197              53             esc Ou              *
*  6      102          180              54             esc Ov              *
*  7       91          218              55             esc Ow              *
*  8       96          194              56             esc Ox              *
*  9      101          191              57             esc Oy              *
* PF-1    112        esc [001q        esc OP           esc OP              *
* PF-2    113        esc [002q        esc OQ           esc OQ              *
* PF-3    114        esc [003q        esc OR           esc OR              *
* PF-4    115        esc [004q        esc OS           esc OS              *
* DEL      76          151              151              177               *
*                                                                          * 
*        All of the above are on CODE PAGE 0x3c.  '<' in PCASCII           *
*                                                                          *
*  8/21/87 -- Terminology has changed;  for information on above, see      *
*             Tech Ref, under Special Files, hft.  Code pages P0, P1, & P2 *
*             used to be referred to as '<', '=', and '>'.                 *
****************************************************************************/

/* SHIFTED STATE */
char map1[] = {
   0x00,    /* reserved                             */
   0x13,    /* Remap 19 positions                   */
	    /* ???       KEY POSITION  in decimal              */
	    /* 0x00      SHIFT STATE=base, FLAG = a character  */
	    /* 0x3c      CODE PAGE PCASCII 1  "<"              */
	    /* ????      CODE POINT  in decimal                */
	    /* 1st Key through 14th.                           */
	    /* KEY POSITION,SHIFT STATE,CODE PAGE,CODE POINT   */
   108,0x00,0x3c,13,       /*  "ENTER" */
   105,0x00,0x3c,45,       /*    "-"   */
   104,0x00,0x3c,46,       /*    "."   */
   95,0x00,0x3c,47,        /*    "/"   */
   99,0x00,0x3c,48,        /*    "0"   */
   93,0x00,0x3c,49,        /*    "1"   */
   98,0x00,0x3c,50,        /*    "2"   */
   103,0x00,0x3c,51,       /*    "3"   */
   92,0x00,0x3c,52,        /*    "4"   */
   97,0x00,0x3c,53,        /*    "5"   */
   102,0x00,0x3c,54,       /*    "6"   */
   91,0x00,0x3c,55,        /*    "7"   */
   96,0x00,0x3c,56,        /*    "8"   */
   101,0x00,0x3c,57,       /*    "9"   */
   112,0x04,0x3c,3,esc,'O','P',         /*    PF-1    */
   113,0x04,0x3c,3,esc,'O','Q',         /*    PF-2    */
   114,0x04,0x3c,3,esc,'O','R',         /*    PF-3    */
   115,0x04,0x3c,3,esc,'O','S',         /*    PF-4    */
   76,0x04,0x3c,1,del,                  /*    DEL     */  
   }; /* end of char[] map */

/* VT100 KEYPAD APPLICATION MODE */
char map2[] = {
   0x00,    /* reserved                             */
   0x0f,    /* Remap 15 positions                   */
	    /* ???      KEY POSITION  in decimal                */
	    /* 0x04     SHIFT STATE=base, FLAG = character string  */
	    /* 0x3c     CODE PAGE PCASCII 1  "<"                */
	    /* 3        LENGTH of string in decimal             */
	    /* each character in string  (3)                    */
	    /* 1st Key through 14th.    FOLLOWS                 */
	    /* KEY POSITION,SHIFT STATE,CODE PAGE,CODE POINT    */
   108,0x04,0x3c,3,esc,'O','M',      /* "ENTER"      <-- KEYS   */
   105,0x04,0x3c,3,esc,'O','m',      /*   "-"    */
   104,0x04,0x3c,3,esc,'O','n',      /*   "."    */
   95,0x04,0x3c,3,esc,'O','l',       /*   "/"    */
   99,0x04,0x3c,3,esc,'O','p',       /*   "0"    */
   93,0x04,0x3c,3,esc,'O','q',       /*   "1"    */
   98,0x04,0x3c,3,esc,'O','r',       /*   "2"    */
   103,0x04,0x3c,3,esc,'O','s',      /*   "3"    */
   92,0x04,0x3c,3,esc,'O','t',       /*   "4"    */
   97,0x04,0x3c,3,esc,'O','u',       /*   "5"    */
   102,0x04,0x3c,3,esc,'O','v',      /*   "6"    */
   91,0x04,0x3c,3,esc,'O','w',       /*   "7"    */
   96,0x04,0x3c,3,esc,'O','x',       /*   "8"    */
   101,0x04,0x3c,3,esc,'O','y',      /*   "9"    */
   76,0x04,0x3c,1,del,               /*   DEL    */  
   }; /* end of char[] map */

/* DEFAULT BASE STATE */
char map3[] = {
   0x00,    /* reserved                             */
   0x13,    /* Remap 19 positions                   */
	    /* ???      KEY POSITION  in decimal                */
	    /* 0x00     SHIFT STATE=base, FLAG = a character    */
            /*     or 0x01  SHIFT STATE=base, FLAG=a function   */
	    /* 0x3c     CODE PAGE PCASCII 1  "<"                */
	    /* CODE POINT in decimal                            */
	    /* 1st Key through 14th.    FOLLOWS                 */
	    /* KEY POSITION,SHIFT STATE,CODE PAGE,CODE POINT    */
   108,0x00,0x3c,13,       /* "ENTER"      <-- KEYS             */
   105,0x00,0x3c,45,       /*   "-"    */
   104,0x00,0x3c,196,      /*   "."    */
   95,0x00,0x3c,47,        /*   "/"    */
   99,0x00,0x3c,179,       /*   "0"    */
   93,0x00,0x3c,192,       /*   "1"    */
   98,0x00,0x3c,193,       /*   "2"    */
   103,0x00,0x3c,217,      /*   "3"    */
   92,0x00,0x3c,195,       /*   "4"    */
   97,0x00,0x3c,197,       /*   "5"    */
   102,0x00,0x3c,180,      /*   "6"    */
   91,0x00,0x3c,218,       /*   "7"    */
   96,0x00,0x3c,194,       /*   "8"    */
   101,0x00,0x3c,191,      /*   "9"    */
   76,0x01,0x01,0x51,      /*   del    */
/* Code change 8/21/87:
   The RT PF keys were previously mapped as character strings.

   The RT PF keys must be mapped as functions and not as character strings.
   When mapping the PF keys for the VT100 emulation, the keys are mapped as
   character strings and the VT100 application programs will know how to 
   handle them.  However, when mapping the PF keys back to the RT, they
   must be mapped as functions otherwise the RT will not handle them 
   correctly.                                                                 */

   112,0x01,0x00,0x00,    /*  PF-1: key #112, function, high order bits of    */
                          /*  function code, low order bits of function code  */
   113,0x01,0x00,0x01,    /*  PF-2  */
   114,0x01,0x00,0x02,    /*  PF-3  */
   115,0x01,0x00,0x03,    /*  PF-4  */
   }; /* end of char[] map */

/* VT52 KEYPAD APPLICATION STATE */
char map5[] = {
   0x00,    /* reserved                                            */
   0x0e,    /* Remap 18 positions                                  */
	    /* ???      KEY POSITION  in decimal                   */
	    /* 0x04     SHIFT STATE=base, FLAG = character string  */
	    /* 0x3c     CODE PAGE PCASCII 1  "<"                   */
	    /* 3        LENGTH of string in decimal                */
	    /* each character in string  (3)                       */
	    /* 1st Key through 14th.    FOLLOWS                    */
	    /* KEY POSITION,SHIFT STATE,CODE PAGE,CODE POINT       */
   108,0x04,0x3c,3,esc,'?','M',      /* "ENTER"      <-- KEYS      */
   105,0x04,0x3c,3,esc,'?','m',      /*   "-"    */
   104,0x04,0x3c,3,esc,'?','n',      /*   "."    */
   95,0x04,0x3c,3,esc,'?','l',       /*   "/"    */
   99,0x04,0x3c,3,esc,'?','p',       /*   "0"    */
   93,0x04,0x3c,3,esc,'?','q',       /*   "1"    */
   98,0x04,0x3c,3,esc,'?','r',       /*   "2"    */
   103,0x04,0x3c,3,esc,'?','s',      /*   "3"    */
   92,0x04,0x3c,3,esc,'?','t',       /*   "4"    */
   97,0x04,0x3c,3,esc,'?','u',       /*   "5"    */
   102,0x04,0x3c,3,esc,'?','v',      /*   "6"    */
   91,0x04,0x3c,3,esc,'?','w',       /*   "7"    */
   96,0x04,0x3c,3,esc,'?','x',       /*   "8"    */
   101,0x04,0x3c,3,esc,'?','y',      /*   "9"    */
   }; /* end of char[] map */

   
/* ------------------------------------------------------------------------
   This code will remap the BASE state of the numeric keypad to its shifted
   state and remap the PF keys 1 thru 4 to their VT100 equivalent states
   while also disabling the SHIFT-LOCK for each of these keys. After
   VT100 emulation is complete the original base states must be remapped.
   ------------------------------------------------------------------------ */
vt3()
{
struct hfbuf argz;
argz.hf_bufp = map1;                       /* keyboard mapping structure */
argz.hf_buflen = 92;                       /* length of structure */

#ifdef DEBUG
kk = sprintf(ss,"VT3() mapping to SHIFTED STATE\n");
write(fe,ss,kk);
#endif DEBUG

rc = ioctl(1,HFSKBD,&argz);                     /* hft set keyboard */

#ifdef DEBUG
kk = sprintf(ss," VT3() ending.  rc=%d\n",rc);
write(fe,ss,kk);
#endif DEBUG
}



/* ------------------------------------------------------------------------
   This code will remap the numeric keypad's base state  to the KEYPAD
   APPLICATION MODE.  The SHIFT-LOCK key will have no effect on the numeric
   keypad after this.  When the VT100 mode is de-selected the keys must be
   remapped.  see VT5().  VT100 mode.
   ------------------------------------------------------------------------ */
vt4()
{
struct hfbuf argz;
argz.hf_bufp = map2;                       /* keyboard mapping structure */
argz.hf_buflen = 106;                      /* length of structure */

#ifdef DEBUG
kk = sprintf(ss," VT4() mapping to KEYPAD APPLICATION\n");
write(fe,ss,kk);
#endif DEBUG

rc = ioctl(1,HFSKBD,&argz);                     /* hft set keyboard */

#ifdef DEBUG
kk = sprintf(ss," VT4() ending.  rc=%d\n",rc);
write(fe,ss,kk);
#endif DEBUG
}

   
/* ------------------------------------------------------------------------
   This code will remap the numeric keypad & PF1-4 to their DEFAULT BASE
   STATE.  This will be called at the end of a VT100 emulation session.
   ------------------------------------------------------------------------ */
vt5()
{
struct hfbuf argz;
argz.hf_bufp = map3;                       /* keyboard mapping structure */
argz.hf_buflen = 79;                       /* length of structure */

#ifdef DEBUG
kk = sprintf(ss," VT5() mapping to DEFAULT state\n");
write(fe,ss,kk);
#endif DEBUG

rc = ioctl(1,HFSKBD,&argz);                     /* hft set keyboard */

#ifdef DEBUG
kk = sprintf(ss," VT5() ending.  rc=%d\n",rc);
write(fe,ss,kk);
#endif DEBUG
}



/* ------------------------------------------------------------------------
   This code will remap the numeric keypad's base state to the KEYPAD
   APPLICATION MODE.  The SHIFT-LOCK key will have no effect on the numeric
   keypad after this.  When the VT100 mode is de-selected the keys must be
   remapped.  see VT5().  VT52 mode.
   ------------------------------------------------------------------------ */
vt7()
{
struct hfbuf argz;
argz.hf_bufp = map5;                       /* keyboard mapping structure */
argz.hf_buflen = 101;                      /* length of structure */

#ifdef DEBUG
kk = sprintf(ss," VT7() mapping to KEYPAD APPLICATION\n");
write(fe,ss,kk);
#endif DEBUG

rc = ioctl(1,HFSKBD,&argz);                     /* hft set keyboard */

#ifdef DEBUG
kk = sprintf(ss," VT7() ending.  rc=%d\n",rc);
write(fe,ss,kk);
#endif DEBUG
}
#endif /* HFT */
