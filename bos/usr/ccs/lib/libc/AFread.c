static char sccsid[] = "@(#)42	1.11  src/bos/usr/ccs/lib/libc/AFread.c, libcgen, bos411, 9428A410j 1/12/94 17:11:15";
/*
 * COMPONENT_NAME: LIBCGEN
 *
 * FUNCTIONS: AFread
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION: Read the next record from an Attribute File.  Parse and
 *	     fill in the current attribute table and current buffer.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>
#include <stdlib.h>
#include <sys/limits.h>

#define SP 0    /* space */
#define LF 1    /* line feed */
#define CO 2    /* ':' */
#define EQ 3    /* '=' */
#define QO 4    /* '"' */
#define SL 5    /* '\' */
#define CM 6    /* ',' */
#define OD 7    /* octal digit */
#define ID 8    /* other alphanumeric */
#define NTYPES 9
#define BOR 0           /* skip to beginning of record */
#define SKR (NTYPES)    /* skip to key */
#define KEY (NTYPES*2)  /* accumulate key */
#define SKK (NTYPES*3)  /* skip to next key */
#define SKC (NTYPES*4)  /* skip to colon */
#define SKA (NTYPES*5)  /* skip to attribute name */
#define ATR (NTYPES*6)  /* accumulate name */
#define SKE (NTYPES*7)  /* skip to equals */
#define SKV (NTYPES*8)  /* skip to value */
#define VAL (NTYPES*9)  /* accumulate value */
#define QUO (NTYPES*10) /* accumulate quoted value */
#define SLS (NTYPES*11) /* last char was backslash */
#define OD1 (NTYPES*12) /* accumulate octal digits */
#define OD2 (NTYPES*13) /* accumulate octal digits */
#define OD3 (NTYPES*14) /* accumulate octal digits */
#define SKL (NTYPES*15) /* skip to next value */
#define NSTATE 16
#define FIN (NTYPES*16)

#define E  0100000  /* error */
#define O  0040000  /* octal digit */
#define D  0020000  /* end octal spec */
#define B  0010000  /* check for backslashed control character */
#define N  0004000  /* terminate name */
#define V  0002000  /* terminate value */
#define L  0001000  /* terminate list */
#define S  0000400  /* store character */
#define SMASK 0377  /* next state */

static char typetab[128] =
{       SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,LF,LF,LF,SP,SP,SP,
	SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,SP,
	SP,ID,QO,ID,ID,ID,ID,ID,ID,ID,ID,ID,CM,ID,ID,ID,
	OD,OD,OD,OD,OD,OD,OD,OD,ID,ID,CO,ID,ID,EQ,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,SL,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,
	ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,ID,SP
};
static short action[NSTATE*NTYPES] = {
/*SP(0)     LF(1)   CO(2)   EQ(3)   QO(4)   SL(5)   CM(6)   OD(7)   ID(8) */
  E|SKR,    0|BOR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  S|KEY,  S|KEY,/*BOR*/
  0|SKR,    0|BOR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,  0|SKR,/*SKR*/
  V|SKC,    E|SKR,V|L|SKA,  E|SKR,  E|SKR,  E|SKR,  V|SKK,  S|KEY,  S|KEY,/*KEY*/
  0|SKK,    L|FIN,  L|SKA,  E|SKR,  E|SKR,  E|SKR,  0|SKK,  S|KEY,  S|KEY,/*SKK*/
  0|SKC,    E|SKR,  L|SKA,  E|SKR,  E|SKR,  E|SKR,  0|SKK,  E|SKR,  E|SKR,/*SKC*/
  0|SKA,    0|FIN,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  S|ATR,  S|ATR,/*SKA*/
  N|SKE,N|V|L|FIN,  E|SKR,  N|SKV,  E|SKR,  E|SKR,  E|SKR,  S|ATR,  S|ATR,/*ATR*/
  0|SKE,  V|L|FIN,  E|SKR,  0|SKV,  E|SKR,  E|SKR,  E|SKR,V|L|SKA,V|L|SKA,/*SKE*/
  0|SKV,    V|SKR,  E|SKR,  E|SKR,  0|QUO,  E|SKR,  E|SKV,  S|VAL,  S|VAL,/*SKV*/
  V|SKL,  V|L|FIN,  S|VAL,  S|VAL,  E|SKR,  S|VAL,  V|SKV, S|VAL,  S|VAL,/*VAL*/
  S|QUO,    E|SKR,  S|QUO,  S|QUO,  V|SKL,  0|SLS,  S|QUO,  S|QUO,  S|QUO,/*QUO*/
  S|QUO,    E|SKR,  S|QUO,  S|QUO,  S|QUO,  S|QUO,  S|QUO,  D|OD1,B|S|QUO,/*SLS*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,  D|OD2,O|S|QUO,/*OD1*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,  D|OD3,O|S|QUO,/*OD2*/
O|S|QUO,    E|SKR,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,O|S|QUO,/*OD3*/
  0|SKL,    L|FIN,  E|SKR,  E|SKR,  E|SKR,  E|SKR,  0|SKV,L|S|ATR,L|S|ATR /*SKL*/
};

#ifdef DEBUG
int AFread_debug = 0;
#endif

/*
 * Read next record from Attribute File.
 * Parse and fill in current attribute table and current buffer.
 */

char *
AFread(af)
AFILE_t af;
{       register char *p;
	register ATTR_t a;
	register int t;
	register int c;
	int tmpch;
	int peekc, num, state, mb_cur_max, mbleng,
	    cattrBottom = &(af->AF_catr[ af->AF_natr -1 ]),
	    cbufBottom  = &(af->AF_cbuf[ af->AF_rsiz ]);
	char mb_buf[MB_LEN_MAX+2];
	char* mbp = mb_buf;
	
	mb_cur_max = MB_CUR_MAX;
	a = af->AF_catr;
	p = af->AF_cbuf;
	a->AT_name = "";
	a->AT_value = p;
	for (peekc = '\n', num = 0, state = BOR; state != FIN; state = t&SMASK)
	{   if ((c = peekc) == EOF)
		c = getc(af->AF_iop);
	    peekc = EOF;
	    if (c < 0 || (int)a >= cattrBottom || (int)p >= cbufBottom )
		return(NULL);
	    if (c == '\n')
	    {   while ((c = getc(af->AF_iop)) == '*') /* strip comments */
		    while ((c = getc(af->AF_iop)) != '\n' && c != EOF);
		if (state == SKV || (c != ' ' && c != '\t'))
		{   peekc = c;
		    c = '\n';
		}
	    }
	    t = (!(c & 0200)) ? typetab[c & 0177] : ID;
		
#ifdef TRACE
fprintf(stderr,"peekc=`%c',c=`%c',type=%d,state=%d,action=0%o\n",
	peekc,c,t,state/NTYPES,action[t + state]);
#endif
	    t = action[t + state];
	    if (t&E)     /* error */
	    {
	    }

	    if (t&D)     /* continue octal specification */
	    {   num = (num << 3) + (c - '0');
	    }
	    if (t&O)     /* end octal specification */
	    {   peekc = c;
		c = num;
		num = 0;
	    }
	    if (t&B)     /* map special backslash */
	    {   switch (c)
		{ case 'b':
		    c = '\b';  break;
		  case 'f':
		    c = '\f';  break;
		  case 'n':
		    c = '\n';  break;
		  case 'r':
		    c = '\r';  break;
		  case 't':
		    c = '\t';  break;
		}
	    }
	    if (t&N)     /* terminate attribute name */
	    {   *p++ = '\0';
#ifdef DEBUG
if (AFread_debug)
	fprintf(stderr,"Term Name `%s'\n",a->AT_name);
#endif
		a->AT_value = p;
	    }
	    if (t&V)     /* terminate attribute value */
	    {   *p++ = '\0';
#ifdef DEBUG
if (AFread_debug)
	fprintf(stderr,"Term Value `%s'\n",a->AT_value);
#endif
	    }
	    if (t&L)     /* terminate attribute list */
	    {   *p++ = '\0';
#ifdef DEBUG
if (AFread_debug)
	fprintf(stderr,"Term List\n");
#endif
		a++;
		a->AT_name = p;
	    }
	    if (t&S)     /* store char */
	    {   	
		/*
		 * Only do multi-byte processing if we are not storing an
		 * octal character (e.g. "\012").
		 */
		if ((mb_cur_max > 1) && (c & 0200) && !(t&O)) {
		    mb_buf[0] = (char) c;
		    mb_buf[1] = '\0';
		    /*
		     * if this is a multi-byte character, note the following
		     * bytes for later processing. Getting the following bytes
		     * now elminates the possibility of confusing the following
		     * byte for a special character like backslash.
		     */
		   for(mbleng=1; mblen(mb_buf, mbleng)<0 && mbleng <= mb_cur_max; mbleng++) {
			if ((tmpch = getc(af->AF_iop)) == EOF) {
			    mb_buf[mbleng] = '\0';
			    mbleng = mb_cur_max + 1; /* mbleng>max == err */
			    break;		     /* so make sure.     */
			}
			mb_buf[mbleng] = (char) tmpch; 
		    }
		    mb_buf[mbleng] = '\0';
		    for (mbp = mb_buf; *mbp; )
			*p++ = *mbp++;
		} else {
		    *p++ = c;
		}
	    }
	}
#ifdef DEBUG
if (AFread_debug)
	fprintf(stderr,"Term Record\n");
#endif
	a->AT_name = NULL;
	*p = '\0';          /* terminate record */
	if (peekc != EOF)
	    ungetc(peekc,af->AF_iop);
	return(p);
}
