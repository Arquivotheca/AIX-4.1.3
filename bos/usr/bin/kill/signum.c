static char sccsid[] = "@(#)13  1.8  src/bos/usr/bin/kill/signum.c, cmdcntl, bos411, 9428A410j 5/31/91 09:29:42";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: signum,sigprt
 *
 * ORIGINS: 27, 18
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: signum.c,v Revision: 1.5  (OSF) Date: 90/10/07 16:38:58 ";
*/

#include <ctype.h>
#include <sys/signal.h>
#include <stdio.h>
#include <sys/ioctl.h>


/*
 * Table of signal names associated with signal codes.
 */

static char *SGcodes[NSIG] = {
	"NULL",		"HUP",		"INT",		"QUIT",
	"ILL",		"TRAP",		"IOT",		"EMT",	
	"FPE",		"KILL",		"BUS",		"SEGV",
	"SYS",		"PIPE",		"ALRM",		"TERM",
	"URG",		"STOP",		"TSTP",		"CONT",
	"CHLD",		"TTIN",		"TTOU",		"IO",
	"XCPU",		"XFSZ",		(char *)0,	"MSG",
	"WINCH",	"PWR",		"USR1",		"USR2"
#ifdef _AIX
       ,"PROF",		"DANGER",	"VTALRM",	"MIGRATE",
	"PRE",		(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	"GRANT",	"RETRACT",	"SOUND",	"SAK"
#endif
};


/*
 * NAME:  signum
 *
 * FUNCTION: signum will convert a signal name to a signal number.
 *    if signal is a number that signum will convert it to an integer.
 *
 * RETURNS: -1 - failure or signal number
 *
 */
int signum(char *signal)
{
	int i;
	char *s;

	if (signal == NULL)
		return (-1);

	if (isdigit(signal[0])) {
		i = atoi(signal);
		if ((i >= 0) && (i < NSIG))
			return(i);
		return(-1);
	}

	for (i=0; i < strlen(signal); ++i) 
		signal[i]=toupper((int)signal[i]);
	/* strip SIG off of string */
	if (signal[0] == 'S' && signal[1] == 'I' && signal [2] == 'G')
		s = signal+3;
	else
		s = signal;
	/* compare string to list of signals in SGcodes */
	for (i=0; i < NSIG; i++)
		if (!strcmp(s,SGcodes[i])) {
			return(i);		
		}
	return(-1);
}

/*
 * NAME: sigprt
 *
 * FUNCTION: sigprt will print out a list of signal names without the prefix SIG
 *      if called with an argument of -1.  If called with a signal number, only
 *      one signal name will be printed out.
 */
void sigprt(int signo)
{
        int i, dlen;
	struct	winsize win;
	int 	col;

        if (signo == -1) {
		if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) 
			col = win.ws_col-1;
		else 	col = 80 ;
          	for (i = 0 , dlen = 0 ; i < NSIG - 1 ; i++) {
            		if(*SGcodes[i] == '\0') continue;
            		if (( dlen = dlen + strlen(SGcodes[i]) + 1 ) > col ){
	      			printf("\n");
	      			printf("%s ", SGcodes[i]);
	      			dlen = strlen(SGcodes[i]) + 1 ;
	    		}
            		else 
	      			printf("%s ", SGcodes[i]);
	  		}	
        	printf("%s\n", SGcodes[NSIG-1]);
        } else
            if(*SGcodes[signo] != '\0') printf("%s\n", SGcodes[signo]);
}
