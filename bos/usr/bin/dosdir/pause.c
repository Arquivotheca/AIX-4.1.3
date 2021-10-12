static char sccsid[] = "@(#)15	1.4  src/bos/usr/bin/dosdir/pause.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:39";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _pause 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include   <fcntl.h>
#include   <termio.h>
#include   <signal.h>

static  void    (*oldsig)(int);
static  int    file;
static  struct termio arg;

static
void newsig(n)           /* restore tty state if a break occurs during read */
int	n;
{
	ioctl(file,TCSETA,&arg);
	close(file);
	if (oldsig == SIG_DFL)
		return;
	if(oldsig != SIG_IGN)
		(*oldsig)(n);
				  /* execute previous signal handling code */
}

int _pause(msg)
char *msg;
{
	char buffer = '\177';
	struct termio arg2;

	file = open("/dev/tty",O_RDWR);     /* accept no substitutes!      */
	if (file == -1) return(-1);

	write(file,msg,strlen(msg));        /* write out the request       */

	oldsig = signal(SIGINT,newsig);     /* original signal handler     */

	ioctl(file,TCGETA,&arg);            /* get original tty state      */
	arg2 = arg;                         /* save it                     */
	arg2.c_lflag &= ~(ICANON+ECHO);     /* uncook input                */
	arg2.c_cc[VEOF] = 1;                /* minimum chars to read       */
	arg2.c_cc[VEOL] = 1;                /* minimum wait before read    */
	ioctl(file,TCSETAF,&arg2);          /* flush input first           */

	read(file,&buffer,1);               /* get a keystroke             */

	ioctl(file,TCSETA,&arg);            /* restore tty state           */

	signal(SIGINT,oldsig);              /* restore old signal handler  */
	close(file);                        /* close this tty file         */
	return(buffer);                     /* the character that was read */
}









