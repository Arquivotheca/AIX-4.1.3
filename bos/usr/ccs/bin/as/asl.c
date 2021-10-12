static char sccsid[] = "@(#)77	1.22  src/bos/usr/ccs/bin/as/asl.c, cmdas, bos411, 9428A410j 3/22/94 09:35:44";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: combine, get2, input, put2, unget, yylex,
 *            space_alloc, push_exp
 *
 * ORIGINS:  3, 27
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

/****************************************************************/
/* Assembler lexer and auxiliary routines			*/	
/****************************************************************/

# include "as.h"
# include "as0.h"
# include <stdlib.h>

char * yytext = 0;		/* lexical analyzer input buffer*/
size_t yytext_size = 1023;	        /* size  buffer area    */
					/* 1k without \n        */

int fflag = 0;				/* floating point flag	*/
static int unget_flag=0;
int linebufcounter = 0;			/* check on linebuf size*/

/****************************************************************/
/* Tables for combination of operands.				*/
/****************************************************************/
/* table for + */

/* table for other operators */
char othtab[7][7] = {
/*	 UNK	 ABS	 REL	 EXT	 TREL	 TOCOF  REXT*/
/*UNK*/  E_UNK,	 E_UNK,	 E_UNK,  E_UNK,  E_UNK,  ERR,	E_UNK,
/*ABS*/	 E_UNK,  E_ABS,	 ERR,	 ERR,	 ERR,	 ERR,	ERR,
/*REL*/	 E_UNK,  ERR,	 ERR,	 ERR,    ERR,	 ERR,	ERR,
/*EXT*/	 E_UNK,  ERR,	 ERR, 	 ERR,    ERR,	 ERR,	ERR,
/*TREL*/ E_UNK,  ERR, 	 ERR,	 ERR,	 ERR, 	 ERR,	ERR,
/*TOCOF*/ERR,	 ERR,	 ERR,	 ERR,	 ERR,	 ERR,	ERR,
/*REXT*/ E_UNK,	 ERR, 	 ERR,	 ERR,	 ERR,	 ERR,	ERR,
};


/****************************************************************/
/* Function:	combine						*/
/* Purpose:	combine expressions with an operator into 	*/
/*		one expresion					*/
/* Input:	op - the operator 				*/
/*		r1 - expression one				*/
/*		r2 - expression two				*/
/* Output:	the combined expresion goes into r1		*/
/* Returns:	a pointer to the combined expression (r1)	*/
/****************************************************************/
struct exp *
combine( int op, 
        register struct exp *r1, 
        register struct exp *r2)
{
	register t1, t2, type;
        int   tmp_op;

	t1 = r1->xtype;		/* set the operand types	*/
	t2 = r2->xtype;
        if ( t1 == (char) ERR || t2 == (char)ERR ||
                       t1 == E_TOCOF || t2 == E_TOCOF || 
                    ( t1 == E_REXT && t2 != E_ABS ) ||
                           ( t2 == E_REXT && t1 != E_ABS ) ) {
           r1->xtype = ERR;
           if ( t1 != (char)ERR && t2 != (char)ERR )
             /* when t1 or t2 is ERR, error 43 has been reported before */
              yyerror(43);
           return(r1);
        }
           
        if (t1 == E_REL)
           r1->xvalue += r1->xloc->start;
        if (t2 == E_REL)
           r2->xvalue += r2->xloc->start;
        if ( t1 == E_TREL )
            r1->xvalue += tocname->csect->start;
        if ( t2 == E_TREL )
            r2->xvalue += tocname->csect->start;
   

	switch (op) {

	case PLUS:
        case MINUS:
                if ( op == PLUS )
                  r1->xvalue += r2->xvalue;
                else 
                  r1->xvalue -= r2->xvalue;

                if ( t1 == E_UNK || t2 == E_UNK ) {
                    type = E_UNK;
                    r1->x_rrtype = RLD_NOT_SET;
                }
                else if ( t1 == t2 && t1 == E_ABS ) {
                   type = E_ABS;
                   r1->x_rrtype = RLD_NOT_SET;
                } 
                else if ( t1 == E_ABS && t2 == E_REXT ) {
                    r1->xloc = r2->xloc;
                    r1->xname = r2->xname;
                    r1->st_inx = r2->st_inx;
                    if ( op == MINUS ) {
                       if ( r2->x_rrtype == R_POS )
                          r1->x_rrtype = R_NEG;
                       else
                          r1->x_rrtype = R_POS;
                       if ( r2->rldinfo.rrtype == R_POS )
                           r1->rldinfo.rrtype = R_NEG;
                       else
                           r1->rldinfo.rrtype = R_POS;
                    } else {
                       r1->x_rrtype = r2->x_rrtype;
                       r1->rldinfo.rrtype = r2->rldinfo.rrtype ;
                    }
                    r1->rldinfo.rname = r2->rldinfo.rname;
                    type = E_REXT;
                } else if (t2 == E_ABS && t1 == E_REXT ) 
                    type = E_REXT;
                else {
                   if ( t1 != E_TMP && t1 != E_ABS){
                         /* t1 is E_REL, E_EXT, or E_TREL */
                     if ( r1->x_rrtype == RLD_NOT_SET ||
                                 r1->x_rrtype == (char)R_POS )
                           push_exp(PLUS, r1);
                     else 
                           push_exp(MINUS, r1);
                   }
                   if ( t2 != E_ABS && t2 != E_TMP) {
                        /* () gets the highest precedence. For an exp. */
                        /* having (), then t2 may be E_TMP            */
                         push_exp(op, r2);
                   }
                   type = E_TMP;
                   if ( r1->st_inx == -1)
                      r1->st_inx = r2->st_inx;
                   else if ( t1 != E_TMP && t2 == E_TMP )
                    /* when t2 is E_TMP and t1 is not, some terms have  */
                    /* been push into exp. stack before t1              */
                      r1->st_inx = r2->st_inx;
                }
                break;

	case IOR:
		r1->xvalue |= r2->xvalue;
		goto comm;

	case XOR:
		r1->xvalue ^= r2->xvalue;
		goto comm;

	case AND:
		r1->xvalue &= r2->xvalue;
		goto comm;

	case LSH:
		r1->xvalue <<= r2->xvalue;
		goto comm;

	case RSH:
		r1->xvalue >>= r2->xvalue;
		goto comm;

	case TILDE:
		r1->xvalue |= ~ r2->xvalue;
		goto comm;

	case MUL:
		r1->xvalue *= r2->xvalue;
		goto comm;

	case DIV:
		if (r2->xvalue == 0)
                                                /* message 041 */
			yyerror( 41 );
		else
			r1->xvalue /= r2->xvalue;
		goto comm;

	comm:
		type = othtab[t1][t2];
                if (r1->xloc==0)
                   r1->xloc = r2->xloc;

		break;

	default:
                                                /* message 042 */
		yyerror( 42 );
	}
	r1->xtype = type;
	if (type==(char)ERR)
                                                /* message 043 */
		yyerror( 43 );
	return(r1);
}

/* this table is used to determine what type of token is being read	*/
/* the table is indexed with a character, and the value in the table	*/
/* is the token value of that character					*/
short type[] = {
	/* diff versions needed for ascii versus ebcidic		*/
	/********* this is the ASCII version **********	*/
	EOF,
	SP,	0,	0,	0,	0,	0,	0,	0,
	0,	SP,	NL,	0,	0,	SP,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
/*      space   !       "       #       $       %       &       '       */
	SP,     0,      DQ,     SH,     LOC,  PCENT,  AND,    SQ,
/*      (       )       *       +       ,       -       .       /       */
	LP,	RP,	MUL,	PLUS,	CM,	MINUS,	ALPH,	DIV,
/*       0       1       2       3       4      5       6       7       */
	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,
/*      8       9       :       ;       <       =       >       ?       */
	DIG,	DIG,	COLON,	SEMI,	LSH,	0,	RSH,	0,
/*      @       A       B       C       D       E       F       G       */
	0,      ALPH,   ALPH,   ALPH,   ALPH,   ALPH,   ALPH,   ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*      X       Y       Z       [       \       ]       ^       _       */
	ALPH,   ALPH,   ALPH,   LBRK,   0,   RBRK,      XOR,    ALPH,
/*      `       a       b       c       d       e       f       g       */
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
/*                                      |               ~               */
	ALPH,	ALPH,	ALPH,	LBRC,	IOR,	RBRC,	TILDE,	0,
};

/****************************************************************/
/* Function:	yylex						*/
/* Purpose:	return tokens from input 			*/
/* Input:	stdin						*/
/* Returns:	the value of the token as recognized by yyparse	*/
/* Comments:	The first pass reads stdin, and writes the token*/
/*		out to a temporary file.   In pass 2, tokens	*/
/*		are read directly from the temp file		*/
/****************************************************************/
yylex()
{
        static short  save_val; /* store the beginning char of the */
                                /* next token which is got for handling */
                                /* branch prediction. since calling */
                                /*  ungetc() twice is not portable, */
                                /* save it here for the future use. */ 
	register short val;	/* used to store the token value*/
	register base;		/* the base of the number being read */
	register char *cp;	/* general character pointer	*/
	register struct instab *op;	/* instr table pointer	*/
	static long intval;	/* an integer value		*/
	static double fltval;	/* a double floating point value*/
	char fltchr[64];	/* string to store flpt number chars*/
	int name_qualified;	/* name qual flag		*/
	char s1[5],*s;		/* temp string for storage class */
	char cc;		/* used to save closing	sc char	*/
	int nxtsym_sc;		/* next symbols storage class	*/
	static int state=0;	/* 0 expecting label or opcode	*/
				/* 1 expecting operand or comment*/
/* 	char *decimal_point;	** decimal_point radix char	*/
        short val1;             /* temp. token value            */
        int br_flag = -1;       /* branch prediction indicator: */
                                /* 0 -- no, 1 -- yes            */

	if(!(yytext))
		yytext = space_alloc(yytext,yytext_size);

	/* if in pass 2, then read tokens from tmp token file	*/
	if (passno!=1) {
		val = get2(tmpfil);
		/* these tokens are pointers to various tables	*/
		if (val==NAME || val==INST || val==QNAME || val==STRING) {
			fread(&yylval, sizeof(yylval), 1, tmpfil);
			   }
		/* integer token, read an integer		*/
		else if (val==INT) {
			fread(&intval, sizeof(intval), 1, tmpfil);
			yylval.itok = &intval;
		/* floating point number token			*/
		} else if (val==FLTNUM) {
			fread(&fltval, sizeof(fltval), 1, tmpfil);
			yylval.dtok = &fltval;
		} else		/* general token		*/
			yylval.tok = get2(tmpfil);
		return(val);
	}
	/* in pass 1						*/
/*	if ((decimal_point= nl_langinfo(RADIXCHAR)) == (char *)0)
		 decimal_point= ".";                         **/
/* In the fix of defect 97257, LC_NUMERIC is set back to "C" */
/* So the decimal_point will be "."                          */
	for(;;) {
           if ( save_val == '+' || save_val == '-' ) {
              yylval.tok = (type+1)[save_val];
              save_val = '\0';
           }
           else
              yylval.tok = (type+1)[val = input()];
	   switch(yylval.tok ) {
		case SP:		/* space		*/
			continue;

		case SH:        /* # comment 			*/
			while ((val = input()) != '\n' && val>0)
				;
			yylval.tok = NL;
		case SEMI:		/* new statement	*/
		case NL:
			if (passno ==1 && listopt) {
				*linebuf_ptr= '\0';
				linebuf_ptr=linebuf;
				linebufcounter=0;
			}
			val = yylval.tok;
			state = 0;
			break;

		case EOF:		/* end of input 	*/
			state = 0;
			val = 0;
			break;

		case LBRK:		/* left bracket		*/
			cp = yytext;
			cc = ']';	/* set expected closing char	*/
			goto namequal;	/* this is a qualified name	*/
		case LBRC:		/* left brace ({)		*/
			cp = yytext;
			cc = '}';	/* expect (}) closing char	*/
			goto namequal;	/* go to qualified name code	*/
		case ALPH:		/* symbol, label, or opcode	*/
			cp = yytext;
			do {
				if (cp >= &yytext[yytext_size])
				{
					cp = yytext_size;
					cp += 
					  (int)(yytext =  space_alloc(yytext,
						(yytext_size<<=1)));
				}
					*cp++ = val;
			} while ((type+1)[val=input()]==ALPH
			  || (type+1)[val]==DIG);
	                

                    	if ((state) && (val == '[' || val == '{')) {
			   /* get storage class from between ({}) or ([]) */
			   cc = (val == '[') ? ']' : '}' ;
       namequal:	   s = s1;
			   while ((type+1)[val=input()]==ALPH 
			           || (type+1)[val]==DIG)
	   		      if (s <= &s1[4])
	      		         *s++ = toupper(val); /* convert to upper case */
			   *s='\0';
			   /* nxtsym has the storage class number	*/
        		   if (val!=cc || (nxtsym_sc = get_sc(s1)) == -1) 
                              /* check for alias names TC0==T0 */
                              if (strcmp(s1,"T0")==0)
                     		nxtsym_sc = XMC_TC0;
                              else {
			      /* set the return token type */
			      val = ERRTOK;
			      /* copy the error code to yylval */
			      yylval.tok = INV_STG_CLASS;
			      break;
			      }
               /* Original Code? 3/18/88                    */
 /*       		   if (val!=cc || (nxtsym_sc = get_sc(s1)) == -1) {
			      val = NOCHAR;
			      break;
			      } */
                           name_qualified = 1;	/* set flag	*/
			   val = input();
		        }
			else {
			   name_qualified = 0;
			   nxtsym_sc = C_LC;	/* local symbol	*/
			}
			*cp = '\0';
/*CHANGE FOR LISTING*/
                        if ( val=='\t' || val==' '){
                           br_flag = 0;  /* there are spaces after the string */
                                         /* it must not be branch prediction  */
			   while (val=='\t' || val==' ')
			      val = input();
                        }
			unget(val);	/* put back start of	*/
			   			/* next token		*/
			if (state || (val == ':')) {
			    /* label or operand field 			*/
			    /* if lookup cannot find symbol, it is added*/
			    yylval.stok = lookup(yytext,nxtsym_sc);
			    /* set the token value			*/
			    if (name_qualified)
			       val = QNAME;
			    else val = NAME;
			    break;
			} else { /* else op opcode (or pseudo) */
				state = 1;

           /* if the next char is '+' or '-', get the following input char, */
           /* if the following input is a space, then it is possible the */
           /* '+' or '-' is part of the mnemonic. So set up the br_flag */
           /* call find_opcode to do string search.                     */

                                if ( br_flag && (val == '+' || val == '-')){
                                   val1 = input(); /* since unget is used   */
                                                   /* above, get it again   */
                                                   /* and skip it           */
                                   if ( (val1 = input()) == ' ' || val1 == '\t'
                                          || val1 == '\n' || val1 == '#' 
                                            || val1 == ';') {
                                      /*  next char is a space or tab   */
                                      /* or # sign or new-line or semi-colon */
                                      /* so see if this is a branch prediction */
                                      *cp++ = val;
                                      *cp = '\0';
                                      br_flag = 1;
                                      op = find_opcode(yytext, &br_flag);
                                      if ( op != NULL &&  br_flag != 88) {
                                         /* it is a branch prediction */
                                               /* unget the space   */
                                         unget(val1);
                                      } else if ( op != 0 && br_flag == 88 ) {
                                                 /* it is not branch    */
                                                 /* prediction, unget chars */
                                            unget(val1);
                                            save_val = val;
                                      } else {
                                          /* string with and without '+' or '-' */
                                          /* was not found in                */
                                          /* the instruction table.    */
                                            unget(val1);
                                            save_val = val;
                                            val = ERRTOK;
                                            yylval.tok = INV_OP;
                                            break;
                                       }
                                   } else { /* no space following the '+'or '-' */
                                            /* it won't be a branch prediction */
                                      unget(val1);
                                      save_val = val;
                                      if(!(op = find_opcode(yytext, &br_flag))) {
                                        /* set the return token type */
                                        val = ERRTOK;
                                        /* copy the error code to yylval */
                                        yylval.tok = INV_OP;
                                        break;
                                      }
                                   }
                                }   
                                else  /* no '+' or '-' follows   */
				  if(!(op = find_opcode(yytext, &br_flag))) {
					/* set the return token type */
					val = ERRTOK;
					/* copy the error code to yylval */
					yylval.tok = INV_OP;
					break;
				  }
				yylval.instok = op;
				/* instruction tag fields are all 0	*/
				/* so instuction token values always turn*/
				/* out to be 256			*/
				val = op->tag+256;
				break;
			}
		case MINUS:
		case PLUS:
		case DIG:
			/* floating point number			*/
			if(fflag) {  /* set by IDOUBLE, IFLOAT in as0.y */
				/* read floats */
				char *p = fltchr;
				double atof();

				if(val!='0' || !strchr("dDfF",val=input()))
					unget(val);
				while (p < &fltchr[63]
				 && ((val=input())== '.'
				 || val=='e' || val=='d'
				 || val=='E' || val=='D'
				 || val=='-' || val=='+'
				 || (type+1)[val]==DIG))
					*p++ = val;
				unget(val);
				*p++ = '\0';
				fltval = atof(fltchr);
				val = FLTNUM;
				break;
			}
			else if ((yylval.tok==MINUS) || (yylval.tok==PLUS)) {
				val = yylval.tok;
				break;
			}
			intval = val-'0';
			if (val=='0') { 	/* hex or octal number	*/
				val = input();
				if (val=='x' || val=='X') {
					base = 16;
				} else {
					unget(val);
					base = 8;
				}
			} else
				base = 10;
			while ((type+1)[val=input()]==DIG
			 || (base==16
			  && (val>='a' && val<='f'||val>='A' && val<='F'))) {
				if (base==8)
					intval <<= 3;
				else if (base==10)
					intval *= 10;
				else
					intval <<= 4;
				if (val>='a' && val<='f')
					val -= 'a' - 10 - '0';
				else if (val>='A' && val<='F')
					val -= 'A' - 10 - '0';
				intval += val-'0';
			}
			unget(val);
			val = INT;
			break;

		case SQ:		/* single quote, means one char	*/
			if ((yylval.tok = input()) == '\n')
				lineno++;
			intval = yylval.tok;
			val = INT;
			break;

		case DQ: 			/* read a string */
			cp = yytext;
			while ((val=input()) && val != '\n' && val != EOF) {

				/* if val is a double qoute (") then 	*/
				/* check if it is end of string (one ") */
				/* or a " in a string (double qoute "")	*/
				if (val == '"' && (val=input()) != '"'){
					unget(val);
					break;  
				}
				if (cp >= &yytext[yytext_size])
				{
					cp = yytext_size;
					cp += 
					  (int)(yytext =  space_alloc(yytext,
						(yytext_size<<=1)));
				}
					*cp++ = val;
			}
			*cp = '\0';
			yylval.strtok = put_string(yytext);
			val = STRING;
			break;

		case 0:
                                                /* message 044 */
			yyerror( 44 ,val);
			val = NOCHAR;
			break;

		default:
			val = yylval.tok;
			break;
		}
		break;
	}
	put2(val, tmpfil, tmpn1);  /* write the token to the temp file */
	/* if it the token value is a pointer, then write the 	*/
	/* pointer the the temp file				*/
	if (val==NAME || val==INST || val == QNAME || val == STRING)
		xfwrite(&yylval, sizeof(yylval), 1, tmpfil, tmpn1);
	else if (val==INT) {		/* integer 		*/
		xfwrite(&intval, sizeof(intval), 1, tmpfil, tmpn1);
		yylval.itok = &intval;
	} else if (val==FLTNUM) {	/* float, write 2 words*/
		xfwrite(&fltval, sizeof(fltval), 1, tmpfil, tmpn1);
		yylval.dtok = &fltval;
	} else				/* just write out tok	*/
		put2(yylval.tok, tmpfil, tmpn1);
	return(val);
}

/****************************************************************/
/* Function:	unget						*/
/* Purpose:	sets unget flag					*/
/* Input:	characters					*/
/* Output:	x						*/
/* Returns:	x						*/
/* Comments:	x						*/
/****************************************************************/
unget(ch)
int ch;
{
	ungetc(ch, stdin);
	unget_flag++;
}

/****************************************************************/
/* Function:	input 						*/
/* Purpose:	will get a character one at the time, check if	*/
/*		any char. has been unget then fills linebuf_ptr */
/*		buffer with characters                          */
/* Input:	 						*/
/* Output:	 						*/
/* Returns:	 						*/
/* Comments:	this routine replaces the getchar, also checks	*/
/*		if EOF then fills line buffer with chars	*/
/****************************************************************/
input()
{
	int ch;
	
	ch=getchar();
	if (unget_flag)
		unget_flag--;
	else if(ch != EOF) {
		if (passno == 1) {
			if (listopt && linebufcounter < MAXLINE) {
				*linebuf_ptr++=ch;
				linebufcounter++;
				}
                          /* If the source line is longer than MAXLINE  */
                          /* put a new-line char and truncate rest of it */
                        else
                              linebuf[MAXLINE-1] = '\n';
		}
	}
	return(ch);
}

/****************************************************************/
/* Function:	get2						*/
/* Purpose:	read a short from the intermediate file 	*/
/* Input:	f - the file descriptor for the int file	*/
/* Returns:	returns the value read				*/
/* Comments:	only called from pass 2, from yylex		*/
/****************************************************************/
get2(f)
register FILE *f;
{
	short r;

	r = getc(f) << 8;
	r |= getc(f);
	return(r);
}

/****************************************************************/
/* Function:	put2						*/
/* Purpose:	write a short to the intermediate file 		*/
/* Input:	f - the file descriptor for the int file	*/
/*              fname - name of int file                        */
/* Output:	the short value to the int file			*/
/* Comments:	called from yylex in pass 1			*/
/****************************************************************/
put2(lv, f, fname)
register FILE *f;
char * fname;
{
        xputc(lv>>8, f, fname);
        xputc(lv, f, fname);
}

/****************************************************************/
/* Function:	space_alloc					*/
/* Purpose:	generic allocation routine             		*/
/* Input:	start - area to allocate			*/
/*       	size  - size of area to allocate		*/
/* Output:	space allocated for start       		*/
/* Comments:	currently only used in yylex			*/
/****************************************************************/

space_alloc(start,size)
void * start;
register size_t size;
{

	if (!(start)) {
		if (!(start=malloc(size))) {
                     yyerror( 73 );
                     return;
                }
	}
	else {
		if (!(start=realloc(start,size))) {
                     yyerror( 73 );
                     return;
                }
	}
	return (start);

}
/************************************************************
  Function:  push_exp
  Purpose:   put the xtype information and its sign into a stack
  Input:     sign -- the sing of this item
             e_ptr -- the expression pointer of this item
  Output:    the stack is updated
************************************************************/
void
push_exp(int sign, struct exp *e_ptr)
{
     e_ptr->st_inx = st_inx;
     st_lp[st_inx].sign = sign;
     st_lp[st_inx].x_type = e_ptr->xtype;
     st_lp[st_inx++].exp_ptr = e_ptr;
}
