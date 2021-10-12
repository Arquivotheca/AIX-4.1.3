static char sccsid[] = "@(#)88	1.1  src/bos/usr/bin/bterm/ptyio.c, libbidi, bos411, 9428A410j 8/26/93 13:35:19";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: array_to_pty
 *		get_from_pty
 *		open_ptyc
 *		ptyc_end
 *		ptyc_init
 *		string_to_pty
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

/* ptycio.c
** --------
*/


#include <sys/types.h>
#include <fcntl.h>
#include <sys/pty.h>
#include <stdio.h>

#include "global.h"
#include "trace.h"


/*======== Initialize PTY controller side ============================*/

void ptyc_init()
{
				    /* get a clear pty */
  if(open_ptyc(&ptyfildes) !=0) {
    printf("Couldn't open PTY");
    ptystatus = -1;
    }
  else {
    ptystatus = 0;
    }
  return;
}

/*=========== Terminate PTY controller ==================================*/

void ptyc_end()
{
  close(ptyfildes);
  return;
}

/*=====================================================================*/
void array_to_pty(st,num)
unsigned char *st;
int num;
{
  write(ptyfildes, st, num);
}
/*=====================================================================*/
void string_to_pty(st)
unsigned char *st;
{
  array_to_pty(st,strlen(st));
}
/*================ PTY open routine ===================================*/


/* This function opens up a pty controller side. 
 */
int open_ptyc (pty)
int *pty;
{
	*pty = open("/dev/ptc", O_RDWR|O_NDELAY);
	if(*pty < 0) return(1);
	strcpy(ttydev, ttyname(*pty));
	ttypPfildes = open(ttydev, O_RDWR);

	return(0);
}

/*------------ read characters from the pty ----------------------------*/


int get_from_pty(cbuf)
char *cbuf;
{
  static unsigned char pty_R_buf[pty_R_size];
  static int pty_IDX = 0, pty_NCH = 0;

  start:
     pty_R_buf[0]=0;
     pty_NCH = read(ptyfildes, pty_R_buf, pty_R_size);
     if(pty_NCH <= 0) 
       {
	pty_NCH = 0;
	return(-1);
       }
     pty_IDX = 0;
     while (pty_NCH > 0) 
     {
         cbuf[pty_IDX] = pty_R_buf[pty_IDX];
         pty_IDX++;
         pty_NCH--;
     }
     cbuf[pty_IDX] = 0;
     return(pty_IDX);
}
