static char sccsid[] = "@(#)63        1.4  src/bos/usr/ccs/bin/structure/1.line.c, cmdprog, bos411, 9428A410j 3/9/94 13:19:57";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: addchar, flush, getline, input1, unput1
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#
#include "def.h"
#define bufsize 1601
char buffer[bufsize];
int bufcount;
extern int errflag;
long stchars;			/* counts number of chars at most recent \n read */
#ifndef unix
long ostchars;
extern long ftell(FILE *stream);
#endif
int newline;			/* counts number of lines read so far in file */
extern int rdfree(), comfree(),labfree(), contfree();
extern int rdstand(), comstand(), labstand(), contstand();
extern int (*rline[])();
extern int (*comment[])();
extern int (*getlabel[])();
extern int (*chkcont[])();



flush()
	{bufcount = 0; }

addchar(c)
	{
	buffer[bufcount++] = c;
	}

getline(lastline,lastchar,linecom,charcom)
int *lastline, *linecom;
long *lastchar, *charcom;
				/* set *lastline to number of last line of statement,
				set *lastchar to number of last char of statement,
				set *linecom to number of last line of comment preceding statement */
	{

	int i;
	flush();
	while ( unput1(input1()) != EOF)
		{
		while ( (*comment[inputform])(0)  || blankline() )
			{
			(*rline[inputform])(addchar);
			flush();
			}
		*linecom = newline;
			/* set charcom to number of last char of comment, starting at 0
					if at start of file and no comment, will be -1  */
		*charcom = stchars - 1;
		if (unput1(input1()) == EOF)  break;
		(*getlabel[inputform])(addchar);
		(*rline[inputform])(addchar);
	
		while ( blankline() || ( !(*comment[inputform])(0) &&  (*chkcont[inputform])() ))
			(*rline[inputform])(addchar);
	
		addchar('\0');
		*lastline = newline;
		*lastchar = stchars - 1;
if (debug == 40)
fprintf(stderr,MSGSTR(LINEBUFCT, "line %d; bufcount: %d\n"),newline,bufcount); /*MSG*/
	
		for (i = 5; i < bufcount; ++i)
			if (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n')
				buffer[i] = '~';
		return(bufcount);
		}
	return(-1);
	}


int linechars;			/* counts number of chars read so far in current line */
long newchar;			/* counts number of chars read so far in file */


input1()
	{
	static int c;
	if (c == '\n') linechars = 0;
	c = inchar();
	++linechars;
	++newchar;
	if (c == '\n')
		{
		++newline;
#ifdef unix
 		stchars = newchar; 
#else
		ostchars=stchars; stchars=ftell(infd);
#endif
		}
	return(c);
	}

unput1(c)
	{
	--linechars;
	--newchar;
	unchar(c);
	if (c == '\n')
		{
#ifdef unix
 		stchars = newchar; 
#else
		stchars=ostchars;
#endif
		--newline;
		}
	return(c);
	}




