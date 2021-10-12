#ifndef lint
static char sccsid[] = "@(#)87 1.1 src/bos/usr/bin/vfu/vfu_tools.c, cmdpios, bos411, 9428A410j 4/28/94 08:16:55";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS)
 *
 * FUNCTIONS: vfu_tools.c
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*                      
*
*			       	     TOOLS MODULE
*				   ( vfu_tools.c )
*
*	There are some elementary tools .
*	
*	Author : BUI Q. M.
*	BULL S.A 
*	VERSION : 1.0 June 1990
*
*/

#include <curses.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "vfu.h" 

static char	*err_flash;

/* The more explicit message on screen 
the better humor we have !!! */

/* Catch dialogue flag and multiplex err_flash */
void 
vfu_dispatch_msg()
{
	err_flash = (get_country_dialogue())?"ERREUR":"ERROR";
}

/* Explicit system error */
void 
syserr(win,msg)
WINDOW *win;
char *msg;
{
	extern int errno,sys_nerr;
	extern char *sys_errlist[];

	if (win) {
		werase(win);
		wprintw(win,"%s: %s\n(%d",err_flash,msg,errno);
		if (errno >0 && errno < sys_nerr)
			wprintw(win,"; %s)\n",sys_errlist[errno]);
		else 
			wprintw(win,"\n");
		wrefresh(win);
		vfu_close(2,CLOSE_VFU);
	}
	else {
		fprintf(stderr,"%s: %s\n(%d",err_flash,msg,errno);
		if (errno >0 && errno < sys_nerr)
			fprintf(stderr,"; %s)\n",sys_errlist[errno]);
		else 
			fprintf(stderr,"\n");
		exit(2);
	}
}
	
/* Only warning error */
void 
warning(win,msg)
WINDOW *win;
char *msg;
{

	if (win) {
		werase(win);
		wprintw(win,"%s: %s\n",err_flash,msg);
		wrefresh(win);
		vfu_close(1,CLOSE_VFU);
	}
	else {
		fprintf(stderr,"%s: %s\n",err_flash,msg);
		exit(1);
	}
}

void 
reverse(s)
char *s;
{

	char *t;
	int c;

	for(t=s+(strlen(s)-1);s<t;s++,t--) {
		c = *s;
		*s = *t;
		*t = c;
	}
}

void 
itoa(n,s)
int n;
char *s;
{

	int signe;
	char *t=s;
	
	if ((signe = n) < 0)
		n = -n;
	do {
		*s++ = n%10 + '0';
	} while((n/=10) > 0);
	*s = '\0';
	reverse(t);
}

/* Convert string in number with spying */
int 
vfu_getn(pstr)
char *pstr;
{

	register int val,c;
	register char *p;
	register int spy;

	p = pstr;
	val = 0;
	spy = 0;
	while ((c = *p++) >= '0' && c <= '9') {
		val = val*10 + c - '0';
		++spy;
	}
	if (spy < strlen(pstr))
		return(-99);
	return(val);
}

/* Get basename of pathname */
void
vfu_basename(result,s1,s2)
char 	*result,*s1,s2;
{

	char *pt;

	if ((pt=strrchr(s1,s2)) != NULL) {
		while (s1 != pt) 
			*result++ = *s1++;
		*result = '\0';
	}
	else 
		return;
}

/* Invert n bits of an unsigned x beginning at p-th bit of x, the remained bits
   are unchanged */ 
unsigned char 
invert(x,p,n)
unsigned char 	x;
unsigned 	p,n; 
{

	return( (n > (p+1)) ?	x^(~(~0 << (p+1))) :
		  		x^(~(~0 << n) << (p+1-n)) );
}
