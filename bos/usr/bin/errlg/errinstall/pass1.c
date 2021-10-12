static char sccsid[] = "@(#)59  1.14  src/bos/usr/bin/errlg/errinstall/pass1.c, cmderrlg, bos411, 9428A410j 9/16/93 17:48:30";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pass1, readdesc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:     pass1
 * FUNCTION: First pass scan of .desc input to check for
 *           syntax errors (readdesc) and rule violations.
 *           Rules are:
 *           a. 40  chars max for SET E,D,P
 *           b. 128 chars max for SET U,I,F,R
 *           c. Bad 'SET' designations.
 *           pass1 builds a list of md structures as input to pass2,
 *             one per set.
 *           Each entry contains the codepoint and the text. 
 * INPUTS:   NONE
 * RETURNS:  NONE  (error code is through global variable Errflg)
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/err_rec.h>
#include "errinstall.h"

extern FILE *Infp;

static Lineno0;
static currset;
static setcodeok;

pass1()
{
	int rv;
	int codepoint;
	int len = 0;
	char text[5120];

	fseek(Infp,0,0);
	setcodeok = 0;
	currset = 0;
	Lineno = 0;

	/* Read in data from the input file or from stdin
	   by calling readdesc().  If readdesc() returns a number 
	   between 1 and 7, it's a SET and the value of currset is 
	   set accordingly.  If readdesc() returns 0, the length of the
	   codepoint text is checked.  Then the codepoint and text are
	   added to the list of md structures used by pass2().
	 */

	for(; !((Infp == stdin) && Errflg);) {
		if((rv = readdesc(&codepoint,&text[0])) == EOF)
			return;
		if(currset == 0 && !(1 <= (rv) && (rv) <= NERRSETS))
			continue;

		switch(rv) {
		case 0:
			break;
		default:
			currset = rv;
			continue;
		}
		
		switch(currset) {
			case ERRSET_DESCRIPTION:
			case ERRSET_PROBCAUSES:
			case ERRSET_DETAILDATA:
				len = 40;
				break;
			case ERRSET_USERCAUSES:
			case ERRSET_INSTCAUSES:
			case ERRSET_FAILCAUSES:
			case ERRSET_RECACTIONS:
				len = 128;
				break;
			default:
				currset = 0;
				continue;
		}
		if(strlen(text) > len)
			cat_lwarn(M(CAT_INS_MSGTOOLONG),
				"The length, %d, of message %s is longer than the\n\
permitted length %d.\n", strlen(text), text, len);
		if(m_lookup(currset,codepoint))
			continue;
		m_install(currset,codepoint,text[0] ? text : 0);
	}
}

 /*
  * NAME:     readdesc
  * FUNCTION: Called by pass1() to read a line from Infp and separate it
  *           into a codepoint and text.
  *           Also recognize 'SET' commands and comments.
  * INPUT:    Pointers to the codepoint and text buffer
  * RETURNS:  0   if codepoint and text recognized.
  *           1-7 if a 'SET' command was detected. (no codepoint/text)
  *           Lines with syntax errors are skipped.
  */

static readdesc(codepointp,textp)
int *codepointp;
char *textp;
{
	int  rv;
	char *cp;
	char *c_codepoint;
	char linebuf[5120];
	char *line;
	int n;
	int rc;

	while (rc != EOF) {

		line = linebuf;
		Lineno0 = Lineno;
		if (((Infp == stdin) && Errflg) || ((rc = getline(line)) == EOF)) 
			return(EOF);

		switch(line[0]) {
		case '#':
			continue;
		case '$':             /* comment */
			switch(line[1]) {
			case 's':
				if(line[2] == 'e' && line[3] == 't') {	    /* $set */
				        cp = line + (sizeof("$SET")) - 1;
					if ((*cp == ' ' || *cp == '\t') && (cp = strtok(cp," \t")) &&
						(rv = fcodetoset(cp)) > 0)
						return(rv);
					cat_lerror(M(CAT_INS_BADSET3),"Bad message set identifier: '%s'",cp);
					setcodeok++;
				}	
				continue;
			case ' ':
			case '\t':
			case '\n':
			default:
				continue;
			}
			continue;
		case 'S':                   /* SET */
			if (line[1] == 'E' && line[2] == 'T') {
				cp = line + (sizeof("SET")) - 1;
				if ((*cp == ' ' || *cp == '\t') && (cp = strtok(cp," \t")) &&
					(rv = fcodetoset(cp)) > 0)
					return(rv);
				cat_lerror(M(CAT_INS_BADSET3),"Bad set code '%s'",cp);
				setcodeok++;
			}
			continue;
		}

		if(currset == 0 && !setcodeok) {     /* bad SET code */
			setcodeok++;
			cat_lerror(M(CAT_INS_NOSET),"\
Must specify a specific message set.\n\
Valid entries are:\n\
SET E\n\
SET P\n\
SET U\n\
SET I\n\
SET F\n\
SET R\n\
SET D\n");
			continue;
		}

		/* get codepoint */

 		c_codepoint = strtok(line," \t");
		line = c_codepoint  + strlen(c_codepoint) + 1;

		if(strncmp(c_codepoint,"0x",2) == 0 || strncmp(c_codepoint,"0X",2) == 0)
			c_codepoint += 2;
			
		if(!numchk(c_codepoint))  {             /* codepoint must be hex number */
			cat_lerror(M(CAT_INS_BADFMT),"Bad format for Message ID '%-.30s'",
				c_codepoint);
			continue;
		}
		if(currset == ERRSET_DETAILDATA)  {      /* check length of detail data */
			if(strlen(c_codepoint) != 2 && strlen(c_codepoint) != 4) { 
				cat_lerror(M(CAT_INS_2DIGITS),"\
Message IDs for the Detail Data ID message set must be\n\
either two or four digits long.\n");
				continue;
			}
		}	
		else    {
			if(strlen(c_codepoint) != 4)  {    /* check codepoint length */
				cat_lerror(M(CAT_INS_4DIGITS),"\
Message IDs must be 4 digits long.\n");
				continue;
			}
		}
		*codepointp = strtol(c_codepoint,0,16);
		cp = line;
		while(*cp == ' ' || *cp == '\t')
			cp++;
		n = strlen(cp);
		if(cp[0] != '"' || cp[n-1] != '"')  {   /* description must be in quotes */
			cat_lerror(M(CAT_INS_EOL3),"\
Text must be enclosed in quotes\n");
			continue;
		}
		cp[n-1] = '\0';
		strcpy(textp,cp+1);
		return(0);
	}
}
