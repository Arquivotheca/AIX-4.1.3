static char sccsid[] = "@(#)11	1.3  src/bos/usr/bin/errlg/liberrlg/nls.c, cmderrlg, bos411, 9435B411b 9/1/94 14:49:30";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: locale_init, getline
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NLS interface programs
 *
 * NAME:     getline
 * FUNCTION: Read a line (including continuations) on NLS input from Infp
 *           into a supplied buffer.
 *           Strip leading and trailing whitespace.
 * INPUTS:   'line' pointer to the text buffer. (caller allocates)
 * RETURN:   1    A line was read successfully. The line must be terminated
 *                by '\n'
 *           EOF  EOF
 */

#define _USENLS_INCL

#include <stdio.h>
#include <string.h>
#include <errinstall.h>

extern FILE *Infp;

/*
 * Read a line of text into 'line'.
 * Strip off trailing whitespace.
 * Combine continued lines.
 */

getline(line)
char *line;
{
	int nlc;

	for(;;) {
		nlc = nlc_getwc();
		switch(nlc) {
		case WEOF:
			return(EOF);
		case L'#':		/* comment */
		case L'*':		/* comment */
			while((nlc = nlc_getwc()) != L'\n') {
				if(nlc == WEOF)
					return(EOF);
			}
			Lineno++;
			continue;
		case L'\n':
			Lineno++;
			continue;
		default:
			nlc_ungetwc(nlc);
			break;
		}
		break;
	}
	nlc_putinit(line);
	for(;;) {
		nlc = nlc_getwc();
		switch(nlc) {
		case L'\\':
			nlc = nlc_getwc();
			switch(nlc) {
			case L'\n':
				Lineno++;
			case L'n':
			case L'b':
			case L't':
				nlc_putwc(L'\\');
				nlc_putwc(nlc);
				break;
			case WEOF:
				nlc_putwc(0);			/* null terminate */
				nlc_strip(line);
				return(1);
			default:
				nlc_putwc(nlc);
				break;
			}
			break;
		case L'\n':
			Lineno++;
		case WEOF:
			nlc_putwc(0);			/* null terminate */
			nlc_strip(line);
			return(1);
		default:
			nlc_putwc(nlc);
			break;
		}
	}
}

static last_nlwc = WEOF;
static char *linep;
static last_nlc = EOF;

nlc_putinit(line)
char *line;
{

	linep = line;
}

static nlc_strip(line)
char *line;
{
	char *cp;

	cp = line + strlen(line) - 1;
	while(cp > line) {
		if(!(cp[0] == ' ' || cp[0] == '\t'))
			break;
		cp[0] = '\0';
		cp--;
	}
}

nlc_putwc(nlc)
{
	char mbchars[MB_LEN_MAX];
	int wcsize, i;

	wcsize = wctomb(mbchars,nlc);
	for(i=0; i < wcsize; i++) {
		*linep = mbchars[i];
		linep++;
	}
	*linep = '\0';
}

nlc_ungetwc(c)
{

	last_nlwc = c;
}

nlc_getwc()
{
	int c;

	if(last_nlwc != WEOF) {
		c = last_nlwc;
		last_nlwc = WEOF;
		return(c);
	}
	last_nlwc = WEOF;
	if((c = fgetwc(Infp)) == WEOF)
		return(EOF);
	return(c);
}


nlc_put(nlc)
{
	*linep = nlc;
	linep++;
	*linep = '\0';
}



nlc_ungetc(c)
{

	last_nlc = c;
}


nlc_getc()
{
	int c,c1;

	if(last_nlc != EOF) {
		c = last_nlc;
		last_nlc = EOF;
		return(c);
	}
	last_nlc = EOF;
	if((c = fgetc(Infp)) == EOF)
		return(EOF);
	return(c);
}

