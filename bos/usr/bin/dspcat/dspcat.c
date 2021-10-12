static char sccsid[] = "@(#)78	1.20  src/bos/usr/bin/dspcat/dspcat.c, cmdmsg, bos411, 9428A410j 11/24/93 11:59:50";
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: Main, make_msg, unpack
 *
 * ORIGINS: 18, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 * static char rcsid[] = "RCSfile: dspcat.c,v Revision: 1.5 (OSF) Date: 90/10/07 16:45:15 ";
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "catio.h"
#include "msgfac_msg.h" 

void	make_msg(), 	/*---- prints ._msg format messages ----*/
	unpack();	/*--- reformats string into ._msg (gencat) format ----*/
/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 	standard library functions
 */


/*
 * NAME: main
 *                                                                    
 * FUNCTION: Decides what it has been asked to do (from the input arguments)
 *           and does it. (It either prints out a single message, a set, or the
 * 	     whole catalog.)
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS:   exit(0) if successful
 *
 */  



main(int argc, char *argv[]) 

{
	int 	set = ERR,	/*---- set to be printed ----*/
		msg = ERR,	/*---- msg to be printed ----*/
		msgout = FALSE;	/*---- TRUE for ._msg style output format ----*/
	int 	i,
		j;
	nl_catd catd;
	nl_catd catderr;	/* catalog descriptor for error messages */

	setlocale (LC_ALL,"");
	catderr = catopen(MF_MSGFAC, NL_CAT_LOCALE);
	if (argc >= 2) {
		if (!memcmp(argv[1],"-g",2))  {
			msgout = TRUE;
			argc--;
			argv++;
		}
	}

	if (argc == 2)
		;
	else if (argc == 3) 
		sscanf(argv[2],"%d",&set);
	else if (argc == 4) {
		if (msgout)
			die( catgets(catderr, MS_DSPCAT, M_NO_ID, "No message id allowed with -g option.") );
		sscanf(argv[2],"%d",&set);
		sscanf(argv[3],"%d",&msg);
	}
	else {	/*---- Too many or too few arguments ----*/
		die( catgets(catderr, MS_DSPCAT, M_USAGE, "Usage:  dspcat [-g] catname [set#] [msg#]") );
	}

	/*---- Force message catalogue to be opened by getting any message ----*/
	catd = catopen(argv[1], NL_CAT_LOCALE);
	catgets(catd, 0, 0, "");
	if (catd == CATD_ERR || catd->_fd == (FILE *)-1) {
                fprintf(stderr, catgets(catderr, MS_DSPCAT, M_CAT_NO_OPEN,
                        "Unable to open specified catalog (%s)\n") ,argv[1]);
                exit(1);
	}

	if (argc >= 3 && (set < 1 || set > catd->_hd->_setmax 
                                            || catd->_set[set]._n_msgs == 0))
		die( catgets(catderr, MS_DSPCAT, M_BADSET,
                                  "dspcat:  Specify a valid set number."));

	if (argc == 4 && (msg < 1 || msg > catd->_set[set]._n_msgs
                          || (catd->_set[set]._mp[msg]._offset == 0
                              && catd->_set[set]._mp[msg]._msgno == 0 ) )) 
		die( catgets(catderr, MS_DSPCAT, M_BADMSG, 
                                 "dspcat:  Specify a valid message number."));

	if (msgout) {	/*---- ._msg style output format ----*/
		make_msg(catd,set);
		exit(0);
	}

/*______________________________________________________________________
	Standard dspcat style output
  ______________________________________________________________________*/

	if (argc == 4) {	/*---- both set and message specified ----*/
		fputs(catgets(catd,set,msg,""),stdout);
	}
	else if (argc == 3) {	/*---- just the set ----*/
		for (i = 1 ; i <= catd->_set[set]._n_msgs ; i++) {
			if (catd->_set[set]._mp[i]._offset)
				printf("%s\n",catgets(catd,set,i,""));
		}
	}
	else if (argc == 2) {	/*---- print the whole catalog ----*/
		for (j = 0 ; j <= catd->_setmax ; j++) {
			for (i = 1 ; i <= catd->_set[j]._n_msgs ; i++) {
				if (catd->_set[j]._mp[i]._offset)
					printf("%d : %d %s\n",j,i,
                                               catgets(catd,j,i,""));
			}
		}
	}
	exit(0);
	return(1);
}




/*
 * NAME: make_msg
 *
 * FUNCTION: 	Makes message and prints its output with a ._msg style format 
 *		(i.e. suitable for input to gencat).
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS: 	void
 */

void make_msg(catd, set) 
nl_catd catd;
int set;

	/*---- catd: catalog descriptor ----*/
	/*---- set: set number (ERR for all sets) ----*/

{
	int 	i,	/*---- Misc counter(s) used for loops ----*/
		j,	/*---- Misc counter(s) used for loops ----*/
		setmin,	/*---- minimum set to be printed ----*/
		setmax; /*---- maximum set to be printed ----*/

	char 	buffer[NL_TEXTMAX * 2];
  			/*---- buffer to store unpacked message in  ----*/

	if (set == ERR) {
		setmin = 1;
		setmax = catd->_setmax;
	}
	else 
		setmin = setmax = set;

	for (j = setmin ; j <= setmax ; j++) {

		printf("\n$delset %d\n",j);	/* header info for each set */
		printf("$set %d\n",j);
		printf("$quote \"\n\n");

		for (i = 1; i <= catd->_set[j]._n_msgs; i++) {
			if (catd->_set[j]._mp[i]._offset) {
				unpack(buffer,catgets(catd,j,i,""));	
				printf("%d\t\"%s\"\n",i,buffer);
			}
		}
	}
} 



/*
 * NAME: unpack
 *
 * FUNCTION: unpack a text string into a format suitable for gencat.   
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS: void
 */

void unpack (t, s) 
char *t; char *s;

	/*---- t: Target buffer ----*/
	/*---- s: Source buffer ----*/

{
	int	len;		/* # bytes in next character */
	int	mb_cur_max;	/* temp MB_CUR_MAX */
	wchar_t	wc;		/* process code of next character */

	mb_cur_max = MB_CUR_MAX;
	while (*s) {	/*---- Until end of string ----*/
		len = mbtowc(&wc, s, mb_cur_max);
		if (len < 1) {
			len = 1;
			wc = *s & 0xff;
		}
		switch(wc) {
		case '\b':
			*t++ = '\\';
			*t++ = 'b';
			break;
		case '\t':
			*t++ = '\\';
			*t++ = 't';
			break;
		case '\n':
			*t++ = '\\';
			*t++ = 'n';
			if (s[1] != '\0') {
				*t++ = '\\';
				*t++ = '\n';
			}
			break;
		case '\v':
			*t++ = '\\';
			*t++ = 'v';
			break;
		case '\f':
			*t++ = '\\';
			*t++ = 'f';
			break;
		case '\r':
			*t++ = '\\';
			*t++ = 'r';
			break;
		case '"':
			*t++ = '\\';
			*t++ = '"';
			break;
		case '\\':
			*t++ = '\\';
			*t++ = '\\';
			break;
		default:
			if (wc < 0x20)
				t += sprintf(t,"\\%3.3o", (int)wc);
			else
				do
					*t++ = *s++;
				while (--len > 0);
		}
		s += len;
	}
	*t = '\0';
}
