static char sccsid[] = "@(#)58	1.12  src/bos/kernel/db/POWER/dbparse.c, sysdb, bos411, 9428A410j 11/16/92 01:22:06";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: parse_line, db_atoi
 *
 * ORIGINS: 27
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

#include <sys/types.h>
#include "parse.h"                      /* parser structure        */
#include "debaddr.h"			/* debugger address struct */

#define	isdigit(c)	(((c>='0') && (c<='9')))
#define isxdigit(c)	((((c>='0') && (c<='9')) || ((c>='a') && (c<='f')) || \
			  ((c>='A') && (c<='F'))))
#define tolower(c)	((((c>='A') && (c<='Z')) ? c + ('a' - 'A') : c))

/*
* NAME: parse_line
*
* FUNCTION: parse an input string into separate tokens when a delimiter
* 	is encountered. Usually the delimiter is a blank but it can also
*	be a string of delimiters, like "+,-,:,>", etc. When a delimiter
*	is encountered a new token structure is created. If no tokens
*	are created the number of tokens is -1. If just the command is
*	entered the number of tokens is 0.
*
* RETURN VALUE:
*/

parse_line(line,ps,delim)
char *line;				/* input string */
struct parse_out *ps;			/* ptr to parser structure */
char *delim;				/* delimiters */
{

	uchar c,new_token;
	int i,j,pos,loc,numtok;
	char *tempptr;
	extern struct debaddr debaddr;

	ps->num_tok = -1;		/* initalize token count */
	new_token = TRUE;		/* new token flag */
	for (pos=0; pos<strlen(line); pos++) { /* parse the whole line */
	    c = line[pos];		/* assign current pos */

	    if (new_token) {		/* starting a new term */
	    	new_token = FALSE;	/* reset flag */
	    	if (ps->num_tok == (MAXTOKENS-1)) /* ck for too many toks */
			{
	      		pos = strlen(line); /* end of line */
			printf(" Too many parameters; excess truncated\n");
			}
	    	if (pos < strlen(line)) { /* then something is left */
			while (c == ' ') { /* remove leading blanks */
				c = line[++pos];	
				if (pos >= strlen(line)) 
		  			break;
	      		}
	    	}
	    	if (pos < strlen(line))  {
			loc = 0; 	/* temp loc in cur string */
	      		ps->num_tok++;	/* another token in struct */
			/* initialize the token structure */
	      		numtok = ps->num_tok;
	      		ps->delim_char[numtok] = ' ';	/* delimiter */
	      		ps->token[numtok].tflags = HEXDEC;
	      		for (i=0;i<MAXTOKENLEN;i++)
	      			ps->token[numtok].sv[i] = 0x00; 
	      		ps->token[numtok].hv = 0;
	      		ps->token[numtok].dv = 0;
	      		ps->token[numtok].debaddr = &debaddr;
	      		if (*delim == ' ')  {	/* initial parse */
			 	/* location of token */
				ps->loc[numtok] = &line[pos];	
				if (c == 0x22) 	/* ck double quote */
		  		    ps->token[numtok].tflags = QUOTED;
				else
		  		    pos--;	/* include 1st char */
	      		}
	      		else
				pos--;	/* set to include 1st char */
	    	  }			/* last token parsed */
	      }
	      else {			/* not 1st char */
		  if (ps->token[numtok].tflags == QUOTED)  {
	      		if (c == 0x22)
				new_token = TRUE; /* end of quoted str */
	      		else { 	/* are we off the end ? */
				if (pos == strlen(line)-1) 	  
					/*reached end no quote*/
		  			ps->token[numtok].tflags = ERR; 
				else 	/* just add char to string */
		  			strncat(ps->token[numtok].sv,
						&line[pos],1);
	      		}
    		}
    		else {		/* not a quoted string */
      		    tempptr = delim;
      		    while ((*tempptr != c) && (*tempptr)) 
			tempptr++;
      			if (*tempptr == c) { 	/* delim found */
			    new_token = TRUE;	/* set new token var */
			    ps->delim_char[numtok] = c;
			    if ((pos == 0) && (c == ' '))
		  		    ps->num_tok--;
			    if (pos == 0)
	  		    	ps->token[numtok].sv[0] = c;
		    	}
      			else {	/* not a terminator */
			    /* append to string */
	  		    ps->token[numtok].sv[loc++] = c;

			    /* if it isn't just a number, try to handle */
			    /* it as an expression.			*/

			    if (!isxdigit(c))
				ps->token[numtok].tflags = 
				(ps->token[numtok].tflags & 
				 ~(HEX_VALID | HEX_OVERFLOW)) | EXP_TYPE;
			    if (!isdigit(c))
				ps->token[numtok].tflags =
				ps->token[numtok].tflags & 
				~(DEC_VALID | DEC_OVERFLOW);

			    /* check for hex numbers */
			    if (isxdigit(c) &&
				(ps->token[numtok].tflags & HEX_VALID)) {
	      			if (isdigit(c))
					j = c - 0x30;
	      		        else {
					j = tolower(c) - 0x57;
					ps->token[numtok].tflags =
					ps->token[numtok].tflags & ~DEC_VALID;
				}
				if(((ps->token[numtok].hv * 16)/16) != 
				   ps->token[numtok].hv) {
					ps->token[numtok].tflags = 
					ps->token[numtok].tflags | HEX_OVERFLOW;
					if (ps->token[numtok].tflags&DEC_VALID)
						ps->token[numtok].tflags = 
						ps->token[numtok].tflags &
						~HEX_VALID;
				}
	      			ps->token[numtok].hv = 
					(16*ps->token[numtok].hv) + j;
	      			if ((ps->token[numtok].tflags | DEC_VALID) && 
				    ((c>='0') && (c<='9'))) {
					if(((ps->token[numtok].dv * 10)/10) != 
					   ps->token[numtok].dv)
						ps->token[numtok].tflags = 
						ps->token[numtok].tflags | 
						DEC_OVERFLOW;
        				ps->token[numtok].dv = (10 *
					  ps->token[numtok].dv)+ (c - '0');
				}
		      		else
					if (loc == 0)
			        	    ps->token[numtok].tflags=HEX_VALID;
			    }
			    else 
		  		ps->token[numtok].tflags = EXP_TYPE;
	      		}
	    	  }
	    }
	}
	j = ps->num_tok+1; 
	while (j < 4) {
		ps->token[j].hv = 0;
		ps->token[j].dv = 0;
		ps->token[j].sv[0] = 0x00;
		j++;
	}
	return;
}

int
db_atoi(p)
register char *p;
{
	register int n, f;

	n = 0;
	f = 0;
	for(;;p++) {
		switch(*p) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
		case '+':
			p++;
		}
		break;
	}
	while(*p >= '0' && *p <= '9')
		n = n*10 + *p++ - '0';
	return(f? -n: n);
}
