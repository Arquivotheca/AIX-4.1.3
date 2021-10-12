static char sccsid[] = "@(#)94	1.1  src/bos/usr/bin/bterm/ttyio.c, libbidi, bos411, 9428A410j 8/26/93 13:35:35";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: array_to_hft
 *		char_to_hft
 *		flush_to_hft
 *		get_kb_char
 *		physio_end
 *		physio_init
 *		put_char
 *		put_cursor
 *		string_to_hft
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Documentation:  physio.c
** 		This file contain functions that control the physical
** 		display output.
**
*/

/* ttyio.c
** --------
*/

#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <cur00.h>
#include <memory.h>
#include <string.h>
#include <term.h>

#include "global.h"
#include "trace.h"

#ifndef NULL
#define NULL 0
#endif NULL

static unsigned char to_hft_rec[to_hft_SIZE + 2];
static int to_hft_idx = 0;

static struct termios rawtermios;
/*====================================================================*/
/* Following are the buffering to hft functions                       */


/*------------ flush a record to hft ------------------------------------*/

void flush_to_hft()
{
  if(to_hft_idx > 0) {
     tcdrain(termfildes);
     write(termfildes, to_hft_rec, to_hft_idx);
     }
  to_hft_idx = 0;
  return;
}

/*------------- write a character to hft record -------------------------*/

void char_to_hft(ch)
unsigned char ch;
{
  if(to_hft_idx > to_hft_SIZE) flush_to_hft();
  if (ch) to_hft_rec[to_hft_idx++] = ch;
  else to_hft_rec[to_hft_idx++] = 32;  /* space instead of nulls */
  return;
}

/*------------- write n characters from buf to hft record ----------------*/

void array_to_hft(buf, n)
unsigned char *buf;
int n;
{
  int i;
                             /* check if there is space for the whole array */
  if((to_hft_idx+n) > to_hft_SIZE) 
      flush_to_hft(); 
  for(i = 0; i < n; i++)
    to_hft_rec[to_hft_idx++] = buf[i];
  return;
}

/*----------- write string of chars to controlling hft ---------------- */

void string_to_hft(sss)
unsigned char *sss;
{
  int i;

  i = strlen(sss);
  array_to_hft(sss, i);
  return;
}


/*====================================================================*/

/*-------------- INITIALIZE /dev/tty -------------------------------- */


void physio_init()
{
	void exit();
	pid_t getpid(), pid;

	tcgetattr(0, &rawtermios);

	/* Now, open an "hft" for I/O with NODELAY  */

	termfildes = open("/dev/tty",O_RDWR|O_NDELAY);

	/* we want a RAW mode io on termfildes */
	/* rawsg.sg_flags = RAW;   */
	/* stty(termfildes, &rawsg);   */
	rawtermios.c_lflag     = 0;    /* ~ICANON;  */
	rawtermios.c_cc[VTIME] = 0;
	rawtermios.c_cc[VMIN]  = 0;
	tcsetattr(termfildes, TCSANOW, &rawtermios);

        /* We want the terminal to be in no scrolling mode, to allow us 
           to refresh the last character on the last line. When scrolling is 
           needed we simulate it on our buffer. We also want the force insert
           parameter to be set to LINE, to enable the insert line command
           to work correctly in spite of the scroll mode settings */
        string_to_hft(NoScroll);
        flush_to_hft();

	return;
}


/*-------------- TERMINATE TERMINAL ----------------------------------*/

void physio_end()
{
        /* We want to set the terminal back to its original
           scrolling mode */
        string_to_hft(Scroll);
        flush_to_hft();

        /*    We don't reset the TERM variable as it won't
	/*    affect the parents, only the children.
	*/

	/*      Close our hft terminal                    */
	 close(termfildes);


	/*  Reset /dev/tty to the settings before execution */
	tcsetattr(0, TCSANOW, &orgtermios);

	/*	Just say good bye */

	printf("\n\n                   EXITING  BTERM");
	printf("\n\n");
	sleep(1);
	return;
}

/*------------- GET CHARACTER FROM KEYBOARD -------------------------*/

unsigned char get_kb_char()
{
  unsigned char charkb;
  static unsigned char noinput=0xff;
  int numchars;

  numchars=read(termfildes, &charkb,1);
  if(numchars> 0) 
    {
    if(charkb == 0x0a) charkb = 0x0d; /* line feed to CR */
    return(charkb);
    }
  else return(noinput);
}


/*------------- Position Cursor at (y,x) ------------------------------*/
 

void put_cursor(y,x)
 int y,x;
{
  /* Check valid x,y values. */
  /* Valid x address is from 0 to COLUMNS-1,
     valid y address is from 0 to LINES-1. */
  if (x < 0) x=0;
  if (x > (COLUMNS-1)) x=COLUMNS-1;
  if (y < 0) y=0;
  if (y > (LINES-1)) y=LINES-1;

  string_to_hft(tparm(cursor_address, y, x));

}


/*------------- Write character at position (y,x) ---------------------*/

void put_char ( y, x, chval )
int x, y;
unsigned int chval;

{
   put_cursor(y,x);
   char_to_hft((char) chval); 
   return;
}

