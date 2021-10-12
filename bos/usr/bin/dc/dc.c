static char sccsid[] = "@(#)43	1.23  src/bos/usr/bin/dc/dc.c, cmdcalc, bos41B, 9504A 12/16/94 13:23:06";
/*
 * COMPONENT_NAME: (CMDCALC) calculators
 *
 * FUNCTIONS: dc
 *
 * ORIGINS: 3 26 27
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
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
*
*  NAME: dc [ file ]
*                                                                     
*  FUNCTION: 
*     The dc command is  an arbitrary precision arithmetic cal-
*     culator.  dc takes its input  from file or standard input
*     until it  reads an  end-of-file character.  It  writes to
*     standard output.   It operates  on decimal  integers, but
*     you may specify an input  base, output base, and a number
*     of fractional digits to  be maintained.  dc is structured
*     overall as a stacking, reverse Polish, calculator.
* 
*  OPTIONS:
*	file	Specifies the input file.
*/                                                                    
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include <langinfo.h>
#include "dc_msg.h"

static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_DC,Num,Str)
#include "dc.h"

/* Static variables */
static int	prev_char = -1;		/* Used in OUTC to save
					   a pending output character */
static void	(*outdit)(struct blk *, struct blk *, int);
static int	ibase0;			/* 1 if input base is 0 */

#ifdef DEBUG
static int dc_debug;
#endif
/* Forward declaractions */
static void		init(int, char *[]);
static void		commnds(void);
static void		onintr(void);

static void		print(struct blk *);
static void		oneot(struct blk *, int, char);
static void		tenot(struct blk *, int);
static void		hexot(struct blk *, struct blk *, int);
static void		bigot(struct blk *, struct blk *, int);

static void		unreadc(char);
static void		chsign(struct blk *);
static void		pushp(struct blk *);
static struct blk	*pop(void);
static void		sdump(char *, struct blk *);

static struct blk	*readin(void),*add0(struct blk *, int);
static int		subt(void);

static struct blk	*add(struct blk *, struct blk *);
static struct blk	*mult(struct blk *, struct blk *);
static struct blk	*divn(struct blk *, struct blk *, struct blk **);
static struct blk	*exp(struct blk *, struct blk *);
static struct blk	*sqrt(struct blk *);

static struct blk	*scale(struct blk *, int);
static struct blk	*scalint(struct blk *);
static struct blk	*removc(struct blk *, int);
static struct blk	*removr(struct blk *, int);

static char *		nalloc(char *, unsigned, unsigned);
static struct blk	*salloc(int), *copy(struct blk *, int);
static void		more(struct blk *);
static void		release(struct blk *);
static void		redef(struct blk *);

static struct blk	*getdec(struct blk *, int);
static struct blk	*morehd(void);
static void		garbage(char *);
static void		ospace(char *);
static void		load(void);

static void		seekc(struct blk *, int);

/*
 *  NAME:  main
 *
 *  FUNCTION:  This is the main-line procedure.
 *
 *  RETURN VALUE: none
 *
 */
int
main(int argc,
char *argv[])
{
#ifdef DEBUG
	{
		char *s = getenv("DC_DEBUG");
		if (s)
			dc_debug = 1;
	}
#endif		
	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_DC,NL_CAT_LOCALE);
	init(argc,argv);
	commnds();
	catclose(catd);
	/*exit(0);*/
}


/*
 *  NAME:  commnds
 *
 *  FUNCTION:  Contains a huge SWITCH statement within a WHILE loop which
 *		contains the various commands which dc uses.
 *
 *  RETURN VALUE: none
 *
 */
static void
commnds(void){
	register int c;
	register struct blk *p,*q;
	long l;
	int sign;
	struct blk **ptr,*s,*t, *rem;
	struct sym *sptr;
	struct sym *sp;
	int sk,sk1,sk2;
	int neg;
	int n,d,len=1;


	/*  This while statement takes the place of a goto statement.
	 *  It is being used to retain the integrity of the program. */
	while(1){
		
		switch(c = readc()){
		case ' ':
		case '\n':
		case 0377:
		case EOF:
			continue;

		/*  Prints out various statistics on the status of the stack. */
		case 'Y':
			if(stkptr == &stack[0]) {
				printf(MSGSTR(EMPTSTK,"empty stack\n"));
#ifndef DEBUG
				continue;
#endif
			}
			else
				sdump("stk",*stkptr);
			printf(MSGSTR(HEADMOR,
			    "all %ld rel %ld headmor %ld\n"),all,rel,headmor);
			printf(MSGSTR(NBYTES,"nbytes %ld\n"),nbytes);
#ifdef DEBUG
			printf("in use %d\n", all - rel);
#endif
			continue;

		/* Digits or a decimal point indicate a number */
		case '0': case '1':case '2': case '3':case '4':
		case '5': case '6': case '7':case '8': case '9':
		case 'A': case 'B': case 'C':case 'D': case 'E': case 'F':
		case '.':
			unreadc(c);
			p = readin();
			pushp(p);
			continue;

		/*  The underscore indicates unary minus. */
		case '_':
			p = readin();
			savk = sunputc(p);
			chsign(p);
			sputc(p,savk);
			pushp(p);
			continue;
		/*  subtraction */
		case '-':
			subt();
			continue;
		/*  Addition or subtraction. */
		case '+':
			if(eqk() != 0)continue;
			p = add(arg1,arg2);
			release(arg1);
			release(arg2);
			sputc(p,savk);	/* Set the precision of the result. */
			pushp(p);
			continue;
		/*  multiplication */
		case '*':
			arg1 = pop();
			EMPTY;
			arg2 = pop();
			EMPTYR(arg1);
			sk1 = sunputc(arg1);
			sk2 = sunputc(arg2);
			p = mult(arg1,arg2);
			release(arg1);
			release(arg2);
			savk = sk1+sk2;
			if(savk>k && savk>sk1 && savk>sk2){
				sk = sk1;
				if(sk<sk2)sk = sk2;
				if(sk<k)sk = k;
				p = removc(p,savk-sk);
				savk = sk;
			}
			sputc(p,savk);
			pushp(p);
			continue;
		/*  division */
		case '/':  
			if(dscale() != 0)continue;
			p = divn(arg1,arg2, NULL); /* dscale ensures arg2!=0 */
			release(arg1);
			release(arg2);
			sputc(p, savk);	/* Set the precision of the result. */
			pushp(p);
			if(irem != 0)release(irem);
			continue;
		/*  remainder */
		case '%':
			if(dscale() != 0)continue;
			p = divn(arg1,arg2,&rem); /* arg2 must be non-zero;
						     We want the remainder.*/
			release(arg1);
			release(arg2);
			release(p);	/* Don't need quotient */
			if(irem == 0){
				sputc(rem,skr+k);
				pushp(rem);
				continue;
			}
			p = add0(rem,skd-(skr+k));
			q = add(p,irem);
			release(p);
			release(irem);
			sputc(q,skd);
			pushp(q);
			continue;
		/*  Replaces the top element on the stack by its square root. */
		case 'v':
			p = pop();
			EMPTY;
			savk = sunputc(p);
			if(length(p) == 0){
				sputc(p,savk);
				pushp(p);
				continue;
			}
			if((c = sbackc(p))<0){
				error(
				MSGSTR(EM_IMNUM,"sqrt of neg number\n"));/*MSG*/
			}
			if(k<savk)n = savk;
			else{
				n = k*2-savk;
				savk = k;
			}
			arg1 = add0(p,n);
			arg2 = sqrt(arg1);
			sputc(arg2,savk);
			pushp(arg2);
			continue;
		/*  exponentiation */
		case '^':
			neg = 0;
			arg1 = pop();
			EMPTY;
			if(sunputc(arg1) != 0)error(
				MSGSTR(EM_NOTINT,"exp not an integer\n"));/*MSG*/
			arg2 = pop();
			EMPTYR(arg1);
			if(sfbeg(arg1) == 0 && sbackc(arg1)<0){
				neg++;
				chsign(arg1);
			}
			if(length(arg1)>=3){
				error(
				MSGSTR(EM_2BIG,"exp too big\n"));/*MSG*/
			}
			savk = sunputc(arg2);
			p = exp(arg2,arg1);
			release(arg2);
			rewind(arg1);
			c = sgetc(arg1);
			if(c == EOF)c = 0;
			else if(sfeof(arg1) == 0)
				c = sgetc(arg1)*100 + c;
			d = c*savk;
			release(arg1);
			if(neg == 0){
				if(k>=savk)n = k;
				else n = savk;
				if(n<d){
					q = removc(p,d-n);
					sputc(q,n);
					pushp(q);
				}
				else {
					sputc(p,d);
					pushp(p);
				}
			}
			else {
				sputc(p,d);
				pushp(p);
			}
			if(neg == 0)continue;
			p = pop();
			q = salloc(2);
			sputc(q,1);
			sputc(q,0);
			pushp(q);
			pushp(p);
			if(dscale() != 0)continue;
			p = divn(arg1,arg2, NULL);
			release(arg1);
			release(arg2);
			sputc(p,savk);	/* Set the precision of the result. */
			pushp(p);
			if(irem != 0)release(irem);
			continue;
		/*  Pushes the numb of elements in the stack onto the stack. */
		case 'z':
			p = salloc(2);
			n = stkptr - stkbeg;
			if(n >= 100){
				sputc(p,n/100);
				n %= 100;
			}
			sputc(p,n);
			sputc(p,0);
			pushp(p);
			continue;
		/*  Replaces the top number in the stack with the number of 
		 *  digits in that number. */
		case 'Z':
			p = pop();
			EMPTY;
			n = (length(p)-1)<<1;
			fsfile(p);
			sbackc(p);
			if(sfbeg(p) == 0){
				if((c = sbackc(p))<0){
					n -= 2;
					if(sfbeg(p) == 1)n += 1;
					else {
						if((c = sbackc(p)) == 0)n += 1;
						else if(c > 90)n -= 1;
					}
				}
				else if(c < 10) n -= 1;
			}
			release(p);
			q = salloc(1);
			if(n >= 100){
				sputc(q,n%100);
				n /= 100;
			}
			sputc(q,n);
			sputc(q,0);
			pushp(q);
			continue;
		/*  Pops the top value on the stack and uses that value as the
		 *  number radix for further input. */
		case 'i':
			p = pop();
			EMPTY;
			p = scalint(p);
			release(inbas);
			inbas = p;
			/* See if input base is 0. */
			if (length(inbas) == 0)
				ibase0 = 1;
			else
				ibase0 = 0;
			continue;
		/*  Pushes the input base on the top of the stack. */		
		case 'I':
			p = copy(inbas,length(inbas)+1);
			sputc(p,0);
			pushp(p);
			continue;
		/*  Pops the top value on the stack and uses that value as
		 *  the number radix for further output. */
		case 'o':
			p = pop();
			EMPTY;
			p = scalint(p);
			sign = 0;
			n = length(p);
			q = copy(p,n);
			fsfile(q);
			l = c = sbackc(q);
			if(n != 1){
				if(c<0){
					sign = 1;
					chsign(q);
					n = length(q);
					fsfile(q);
					l = c = sbackc(q);
				}
				if(n != 1){
					while(sfbeg(q) == 0)l = l*100+sbackc(q);
				}
			}
			logo = log2(l);
			obase = l;
			release(basptr);
			if(sign == 1)obase = -l;
			basptr = p;
			outdit = bigot;
			if(n == 1 && sign == 0){
				if(c <= 16){
					outdit = hexot;
					fw = 1;
#ifdef KEEP_DIGIT_GROUPS
					ll = LL;
#endif
					release(q);
					continue;
				}
			}
			n = 0;
			if(sign == 1)n++;
			p = salloc(1);
			sputc(p,-1);
			t = add(p,q);
			n += length(t)*2;
			fsfile(t);
			if((c = sbackc(t))>9)n++;
			release(t);
			release(q);
			release(p);
			fw = n;
#ifdef KEEP_DIGIT_GROUPS
			ll = LL;
			/* If obase is bigger than 10^ll, then each digit
			   will be printed in a group longer than LL.
			   Thus, we increase ll to accomodate the digit.
			   (The bc command doesn't support such a big obase. */
			if(fw>=ll)ll=fw+1;
#endif
			continue;
		/*  Pushes the output base on the top of the stack. */
		case 'O':
			p = copy(basptr,length(basptr)+1);
			sputc(p,0);	/* Add an explicit scale. */
			pushp(p);
			continue;

		/*  Puts the bracketed string onto the top of the stack. */
		case '[':
			n = 0;
			p = salloc(0);
			while(1){
				char mbuf[MB_LEN_MAX];
				char *mcp = mbuf;
				if(MB_CUR_MAX > 1){

					bzero(mbuf,sizeof(mbuf));

					do {
						if(len<0)sputc(p,c);
						*mcp++=c=readc();
					} while((len=mblen(mbuf,MB_CUR_MAX))<1);
				} else c = readc();

				if (len==1 && c == ']'){
						if(n == 0)break;
						n--;
					}
				if(len==1 && c == '[')n++;
				sputc(p,c);
			}
			*p->last = TAG_STRING;
			pushp(p);
			continue;
		/*  Pops the top of the stack and uses that value as a 
		 *  nonnegative scale factor. */
		case 'k':
			p = pop();
			EMPTY;
			p = scalint(p);
			if(length(p)>1){
				error(
				MSGSTR(EM_SC2BIG,"scale too big\n"));/*MSG*/
			}
			rewind(p);
			k = sfeof(p)?0:sgetc(p);
			release(scalptr);
			scalptr = p;
			continue;
		case 'K':
			p = copy(scalptr,length(scalptr)+1);
			sputc(p,0);
			pushp(p);
			continue;
		/*  Replaces the number on the top of the stack with its
		 *  scale factor. */
		case 'X':
			p = pop();
			EMPTY;
			fsfile(p);
			n = sbackc(p);
			release(p);
			p = salloc(2);
			sputc(p,n);
			sputc(p,0);
			pushp(p);
			continue;
		/*  Pops the top value on the stack and the string execution
		 *  level by that value. */
		case 'Q':
			p = pop();
			EMPTY;
			if(length(p)>2){
				error(
				MSGSTR(EM_QWHO,"Q?\n"));/*MSG*/
			}
			rewind(p);
			if((c =  sgetc(p))<0){
				error(
				MSGSTR(EM_NEGQ,"neg Q\n"));/*MSG*/
			}
			release(p);
			while(c-- > 0){
				if(readptr == &readstk[0]){
					error(
					MSGSTR(EM_RS,"readstk?\n"));/*MSG*/
				}
				if(*readptr != 0)release(*readptr);
				readptr--;
			}
			continue;
		/*  Exits the program. */
		case 'q':
			if(readptr <= &readstk[1])exit(0);
			if(*readptr != 0)release(*readptr);
			readptr--;
			if(*readptr != 0)release(*readptr);
			readptr--;
			continue;
		/*  Displays all values on the stack. */
		case 'f':
			if(stkptr == &stack[0])
				printf(MSGSTR(EMPTSTK,"empty stack\n"));
			else {
				for(ptr = stkptr; ptr > &stack[0];){
					print(*ptr--);
				}
			}
			continue;
		/*  Displays the top value on the stack. */
		case 'p':
			if(stkptr == &stack[0])
				printf(MSGSTR(EMPTSTK,"empty stack\n"));
			else{
				print(*stkptr);
			}
			continue;
		/*  Interprets the top of the stack as a string, removes
		 *  it and displays it. */
		case 'P':
			p = pop();
			EMPTY;
			sputc(p,0);
			printf("%s",p->beg);
			release(p);
			continue;
		/*  Duplicates the top of the stack. */
		case 'd':
			if(stkptr == &stack[0]){
				printf(MSGSTR(EMPTSTK,"empty stack\n"));
				continue;
			}
			/* If the top of the stack is an array, its elements
			   are not copied. */
			p = copy(*stkptr,length(*stkptr));
			pushp(p);
			continue;
		/*  Cleans the stack.  Dc pops all values on the stack. */
		case 'c':
			/* If any array values are on the stack, its elements
			   are not freed. */
			while(stkerr == 0){
				p = pop();
				if(stkerr == 0)release(p);
			}
			continue;
		/*  Treats the character following S as a stack(ex: "x").
		 *  It pops the top off the main stack and pushes that 
		 *  value onto stack "x". */
		case 'S':
			if(stkptr == &stack[0]){
				error(
				MSGSTR(EM_SARGS,"save: args\n"));/*MSG*/
			}
			c = readc() & 0377;
			sptr = stable[c];
			sp = stable[c] = sfree;
			sfree = sfree->next;
			if(sfree == NULL) {
				error(MSGSTR(M_STOV,"symbol table overflow\n"));
			}
			sp->next = sptr;
			p = pop();	/* Stack checked above */
			sp->val = p;
			continue;
		/*  Pops the top of the stack and stores it in a register
		 *  named "x"(the character following "s").  */
		case 's':
			if(stkptr == &stack[0]){
				error(
				MSGSTR(EM_SARGS,"save:args\n"));/*MSG*/
			}
			c = readc() & 0377;
			sptr=stable[c];
			if(sptr != NULL){
				/* Free old value in register. */
				p = sptr->val;
				if (*p->last == TAG_ARRAY) {
					/* Value is an array.  Free its
					   elements.  An array element
					   should never be an array itself. */
					rewind(p);
					while((p->wt - p->rd) >= PTRSZ) {
						q = *(struct blk **)(p->rd);
						p->rd += PTRSZ;
						if (q)
							release(q);
					}
				}
				release(p);
			}
			else{
				sptr = stable[c] = sfree;
				sfree = sfree->next;
				if(sfree == 0) {
					error(MSGSTR(M_STOV,"symbol table overflow\n"));
				}
				sptr->next = 0;
			}
			p = pop();
			sptr->val = p;
			continue;
		/*  Pushes the value in register "x"(the character following
		 *  "l") onto the stack. */
		case 'l':
			load();
			continue;
		/*  Treats "x"(the character following "L") as a stack and pops
		 *  its top value onto the main stack. */
		case 'L':
			c = readc() & 0377;

			sptr = stable[c];
			if(sptr == 0){
				error(
				MSGSTR(EM_LWHO,"L?\n"));/*MSG*/
			}
			stable[c] = sptr->next;
			sptr->next = sfree;
			sfree = sptr;
			p = sptr->val;
			pushp(p);
			continue;

		/*  Used for array operations. */
		case ':':
			if ((c = get_subscript()) < 0)
				continue;
			n = readc() & 0377; /* Array name */
			sptr = stable[n];
			if(sptr == NULL){
				sptr = stable[n] = sfree;
				sfree = sfree->next;
				if(sfree == NULL) {
					error(MSGSTR(M_STOV,"symbol table overflow\n"));
				}
				sptr->next = 0;
				p = salloc((c+1)*PTRSZ);
				/* Once stable[n] has been filled in, we
				   must fill in stable[n]->val, even if the
				   the array constructor is being misused
				   and the stack is now empty. */
				sptr->val = p;
				*p->last = TAG_ARRAY;
				zero(p);
				p->wt = p->last;
				seekc(p, c*PTRSZ);
			}
			else {
				int prev_len;

				p = sptr->val;
				if (*p->last != TAG_ARRAY) {
					/* Existing register is not an
					   array.  We must zero the value
					   to avoiding passing garbage
					   values to free() when the
					   array is destroyed. */
					zero(p);
				}
				prev_len = p->last - p->beg;
				/* Make sure p's buffer can hold c+1
				   pointers. */
				seekc(p, (c+1) * PTRSZ);
				*p->last = TAG_ARRAY;
				p->rd -= PTRSZ;	/* Back up to actual slot. */
				/* Be sure any added bytes are zeroed, so
				   we don't try to pass garbage to free() when
				   the array is destroyed. */
				if (prev_len < p->last - p->beg)
					memset(&p->beg[prev_len], 0,
					       (p->last - p->beg) - prev_len);
				p->wt = p->last;
				q = *(struct blk **)(p->rd);
				if (q!=NULL) release(q);
			}
			s = pop();
			EMPTY;
			*(struct blk **)(p->rd) = s;
			continue;
		/*  Used for array operations. */
		case ';':
			if ((c = get_subscript()) < 0)
				continue;
			n = readc() & 0377; /* Array name */
			sptr = stable[n];
			if (sptr != NULL
			    && *(p = sptr->val)->last == TAG_ARRAY
			    && length(p) >= (c+1) * PTRSZ
			    && (s = *(struct blk **)(p->beg + c*PTRSZ))) {
				q = copy(s,length(s));
			}
			else {
				/* No value in register.  Push a value of 0. */
				q = salloc(1);
				sputc(q,0);
			}
			pushp(q);
			continue;

		/*  Treats the top element of the stack as a character string
		 *  and executes it as a string of dc commands. */
		case 'x':
			p = pop();
			EMPTY;
			if((readptr != &readstk[0]) && (*readptr != 0)){
				if((*readptr)->rd == (*readptr)->wt)
					release(*readptr);
				else{
					if(readptr++ == &readstk[RDSKSZ]){
						error(MSGSTR(EM_NSTDPT,
							    "nesting depth\n"));
					}
				}
			}
			else readptr++;
			*readptr = p;
			if(p != 0)rewind(p);
			else if((c = readc()) != '\n')unreadc(c);
			continue;
		/*  Gets and runs a line of input. */
		case '?':
			if(++readptr == &readstk[RDSKSZ]){
				error(
				MSGSTR(EM_NSTDPT,"nesting depth\n"));/*MSG*/
			}
			*readptr = 0;
			{
				FILE	*fsave = curfile;

				curfile = stdin;
				while((c = readc()) == '!')command();
				p = salloc(0);
				sputc(p,c);
				while((c = readc()) != '\n'){
					sputc(p,c);
					if(c == '\\')sputc(p,readc());
				}
				curfile = fsave;
			}
			*readptr = p;
			continue;
		/*  Interprets the rest of the line as an AIX command. */
		case '!':
			if(command() == 1) {
				p = pop();
				EMPTY;
				if((readptr != &readstk[0]) && (*readptr != 0)){
					if((*readptr)->rd == (*readptr)->wt)
						release(*readptr);
					else{
						if(readptr++ == &readstk[RDSKSZ]){
							error(MSGSTR(EM_NSTDPT,"nesting depth\n"));
						}
					}
				}
				else readptr++;
				*readptr = p;
				if(p != 0)rewind(p);
				else if((c = readc()) != '\n')unreadc(c);
			}
			continue;
		/*  Pops the top two elements of the stack and compares them. */
		case '<':
		case '>':
		case '=':
			if(cond(c) == 1) {
				p = pop();
				EMPTY;
				if((readptr != &readstk[0]) && (*readptr != 0)){
					if((*readptr)->rd == (*readptr)->wt)
						release(*readptr);
					else{
						if(readptr++ == &readstk[RDSKSZ]){
							error(MSGSTR(EM_NSTDPT,"nesting depth\n"));
						}
					}
				}
				else readptr++;
				*readptr = p;
				if(p != 0)rewind(p);
				else if((c = readc()) != '\n')unreadc(c);
			}
			continue;
		default:
			printf(MSGSTR(UNIMPL,"%o is unimplemented\n"),c);
		}
	}
} /* commnds */

/*
 *  NAME:  divn
 *
 *  FUNCTION:  Divides 'ddivd' by 'ddivr' and returns the result.  This
 *	function must not be called if 'ddivr' is 0.  The inputs are
 *	treated as integers, so any required scaling must be done before
 *	this function is called.  The input arguments are not released.
 *
 *	The algorithm described below is taken from "The Art of Computer
 *	Programming, Volume 2, Seminumerical Algorithms,"  second edition,
 *	by Donald Knuth.
 *
 *	Input arguments:
 *		ddivd: Dividend.  Scale should not be included.
 *		ddivr: Divisor.   Scale should not be included.
 *		rem_ptr:  Pointer to 'struct blk *' variable is the
 *			remainder is needed.
 *
 *  RETURN VALUE: The quotient.
 *
 */
static struct blk *
divn(struct blk *ddivd,
     struct blk *ddivr,
     struct blk **rem_ptr)
{
	int quotient_sign,remainder_sign,offset;
	int carry, dig,magic,d;
	long c,td,cc;
	register struct blk *quotient,*divd,*divr, *divxyz;

	/* See the algorithm description below. */
	static unsigned char logs[50] = {
		0,			/* 0 not used */
		1,			/* 1 */
		2, 2,			/* 2-3 */
		3, 3, 3, 3,		/* 4-7 */
		4, 4, 4, 4, 4, 4, 4, 4,	/* 8-15 */
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,	/* 16-31 */
		6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6}; /* 32-49 */

	if (length(ddivd) == 0) {
		/* dividend is 0, so quotient and remainder are 0. */
		if (rem_ptr)
			*rem_ptr = salloc(0);
		return salloc(0);
	}

	/* If the divisor or divisor are negative, they are negated and
	   the signs of the quotient and remainder are computed.

	   This table shows are signs are computed
	   (P is positive, N is negative):

	   dividend  P P N N
	   divisor   P N P N
	   =================
	   quotient  P N N P
	   remainder P P N N

	   */

	/* A copy of the divisor is only needed if it is negative. */
	fsfile(ddivr);
	if(sbackc(ddivr) == -1) {
		divr = copy(ddivr,length(ddivr));
		chsign(divr);
		quotient_sign = ~0;
	}
	else {
		divr = ddivr;
		quotient_sign = 0;
	}

	/* A copy of the dividend is always needed. */
	divd = copy(ddivd,length(ddivd));
	fsfile(divd);
	if(sbackc(divd) == -1) {
		chsign(divd);
		quotient_sign = ~quotient_sign;
		remainder_sign = ~0;
	}
	else
		remainder_sign = 0;

	offset = length(divd) - length(divr);

	/* If dividend is smaller than divisor, the quotient is 0 and
	   the remainder is the dividend. */
	if(offset < 0) {
		if (rem_ptr) {
			if(remainder_sign<0)
				chsign(divd);
			*rem_ptr = divd;
		}
		else
			release(divd);

		/* If the divisor was copied, release the copy */
		if(divr != ddivr)release(divr);
		return salloc(0);	/* Return 0. */
	}

	/* We've handled the special cases.  Here's the main algorithm from
	   Knuth. Each step of long division boils down to finding
	   q rapidly in the problem

			   	     q
                         _____________
	   V1 V2 ... Vm ) U0 U1 ... Um
	                  <-- q*V --->
			  ____________
			     <-- R -->

	   where V1, ..., Vm are the base-100 digits of V, the dividend;
	   U0, ..., Um are the digits of U, the divisor; V1 != 0,
	   0 <= R < V, and 0 <= q < 100.

	   If we use (100*U0 + U1) / V1 as a guess q' for q, then we can
	   prove that q' >= q, and so if we use q' in the figure above,
	   R will never be greater than V (although R could be negative).

	   Furthermore, if V1 >= 50, then q' - 2 <= q, so the guess is
	   never off by more than 2.

	   Finally, if we use the guess (10000*U0 + 100*U1 +U2) / (100*V1 + V2)
	   when V1 >= 50, then q' - 1 <= q <= q'.

	   When V1 < 50, we can multiply U and V by a constant k so that
	   the first base-100 digit of kV >= 50.  This won't change the
	   quotient since kU/kV = U/V, although the computed remainder
	   will be too large by a factor of k.

	   Instead of multiplying V and U by k, we can compute the first 2
	   digits of kV by computing V' = (100*100*V0 + 100*V1 + V0)*k/100.
	   (The first 3 digits of kU are computed in a similar fashion).

	   One choice for k is floor(100/(V1 + 1)), but this algorithm
	   uses k = 100/(2^(floor(log2(V1+1)))), which isn't an
	   integer but is easy to compute and still has the desired
	   properties.  We compute floor(log2(V1+1)) by using a
	   table.

	   We use U'/V' as the guess q', which is off by at most 1.

	   We take the guess q', compute q'V and subtract the result from U.
	   If R, the remainder, is negative, then q' = q+1, so we set q=q'-1
	   and add V to R to get a proper remainder.

	   The first step of the algorithm begins as shown here.

			   	     q ...
                          ___________________
	   V1 V2 ... Vm ) 00 U1 ... Um ... Un
	                  <-- q*V --->
			  ____________
			     <-- R -->

	   The original dividend is U1...Un.  U0 = 0 is explicitly added
	   before performing the first step.  We have already ensured that
	   the dividend has fewer digits than the divisor, so n >= m.

	   After completion each step, the next digit of U is appended
	   to R, and this value becomes the new U0...Um for the next
	   step.  In the internal representation used by 'dc',
	   n = length(divisor) and m = length(dividend).  The number
	   of digits in the quotient is n - m + 1.

	   Note that U' must be computed at each step, but V' only needs
	   to be computed once.

	   When computing U', we may look at 4 digits of m, U0, U1, U2, and U3.
	   If m < 3, will will use digits to the right of Um, but this cannot
	   cause q' as computed in the code to be less than the q' defined
	   by the algorithm.  In fact, it isn't hard to show that even if
	   U(m+1) = 99, q' will be unaffected.

	   *****************************************************************
	   */

	/* The maximum number of digits in the quotient is offset + 1.
	   We allocate two extra bytes--one for the scale and an extra
	   byte in case the quotient will be negated. */

	quotient = salloc(offset+3);
	/* Position read (and write) pointer for first digit of quotient. */
	quotient->rd = quotient->wt = quotient->beg + offset + 1;

	/* The maximum number of digits in divxyz, which is used to compute
	   q*divr for each digit q of the quotient, is one more than the
	   number of digits in the divisor.*/

	divxyz = salloc(length(divr)+1);

	/* The remainder is not explicitly allocated, because the final,
	   updated dividend is the remainder. */

	sputc(divd,0);			/* Add a leading 0 to the dividend. */

	/* Compute V' */
	fsfile(divr);			/* Reset to high-order end */
	c = sbackc(divr);		/* Get first 2 digits. */
	cc = c*100 + (sfbeg(divr)?0:sbackc(divr));
	if (c < 50) {
		magic = logs[c];
		cc = (cc*100 +(sfbeg(divr)?0:sbackc(divr))) >> magic;
	}
	else
		magic = 0;

#ifdef DEBUG
	if (dc_debug) printf("DIVN: cc = %d\n", cc);
#endif

	while(offset >= 0){
		fsfile(divd);
		/* Get first 3 digits of dividend */
		td = sbackc(divd);
		td = td*100 + (sfbeg(divd)?0:sbackc(divd));
		td = td*100 + (sfbeg(divd)?0:sbackc(divd));
		if (magic) {
			/* Use 4th digit as well. */
			td = td*100 + (sfbeg(divd)?0:sbackc(divd));
			td >>= magic;
		}
		dig = td/cc;

#ifdef DEBUG
		if (dc_debug) {
			printf("DIVN: td = %d\tNext digit = ", td);
			printf( dig > 99 ? " 99 (was %d)\n" : " %d\n", dig);
		}
#endif

		if (dig > 99)
			dig = 99;

		if (dig) {
			/* Compute divxyz = dig * divr. */
			rewind(divr);
			rewind(divxyz);
			carry = 0;
			while(sfeof(divr) == 0){
				d = sgetc(divr)*dig+carry;
				carry = d / 100;
				salterc(divxyz,d%100);
			}
			salterc(divxyz,carry);
#ifdef DEBUG
			if (dc_debug) {
				char *z = divxyz->rd;
				rewind(divxyz);
				printf("DIVN: Next digit * divisor:\n");
				sdump("", divxyz);
				divxyz->rd = z;
			}
#endif

			/* Subtract divxyz from part of dividend being
			   considered in this step. The number of digits
			   in divxyz is always the same as the number of
			   digits being considered in the divisor, so
			   we don't have to check for overflow of the
			   read pointer. */
			rewind(divxyz);
			seekc(divd,offset);

			carry = 0;
			while(sfeof(divd) == 0){
				d = slookc(divd);
				d = d - *divxyz->rd++ - carry;
				carry = 0;
				if(d < 0){
					d += 100;
					carry = 1;
				}
				salterc(divd,d);
			}
#ifdef DEBUG
			if (dc_debug) {
				char *z = divd->rd;
				seekc(divd, offset);
				printf("DIVN: New dividend:\n");
				sdump("", divd);
				divd->rd = z;
			}
#endif
			if (carry != 0) { /*  dig was too large */
				dig = dig - 1;

				/* Add back the divisor. */
				rewind(divr);
				seekc(divd,offset);
				carry = 0;
				while(sfeof(divd) == 0) {
					d = slookc(divd) + carry
						+ (sfeof(divr)?0:sgetc(divr));
					carry = 0;
					if (d >= 100) {
						d -= 100;
						carry = 1;
					}
					salterc(divd,d);
				}
#ifdef DEBUG
				if (dc_debug) {
					char *z = divd->rd;
					printf("DIVN: Digit too large."
					       " New dividend\n");
					seekc(divd, offset);
					sdump("", divd);
					divd->rd = z;
				}
				if (carry != 1) abort();
#endif
			}
		}

		*--quotient->rd = dig;
		/* Delete high-order digit, which should be 0 at this point. */
		if(--offset >= 0)divd->wt--;
#ifdef DEBUG
		if (dc_debug) {
			printf("DIVN: Current quotient:\n");
			sdump("", quotient);
		}
		if (offset >= 0 && *divd->wt != 0)
			abort();
#endif
	}
	release(divxyz);

	/* Strip off leading zeros of quotient */
	fsfile(quotient);
	while(!sfbeg(quotient) && sbackc(quotient) == 0)
		truncate(quotient);
	if(quotient_sign < 0)
		chsign(quotient);

	/* The modified dividend is now the remainder. */
	if (rem_ptr) {
		/* Strip off leading zeros of remainder */
		fsfile(divd);
		while(!sfbeg(divd) && sbackc(divd) == 0)
			truncate(divd);
		if(remainder_sign<0)
			chsign(divd);
		*rem_ptr = divd;
	}
	else
		release(divd);

	/* If we copied the divisor, free the copy. */
	if(divr != ddivr)release(divr);
	return quotient;
}



/*
 *  NAME:  dscale 
 *
 *  FUNCTION:  This function is used by the appropriate division functions
 *		to determine the scale.
 *
 *  RETURN VALUE:  0)  everything was OK.
 *
 */
static int
dscale(void){
	register struct blk *dd,*dr;
	register struct blk *r;
	int c;

	dr = pop();
	EMPTYS;
	dd = pop();
	EMPTYSR(dr);
	fsfile(dd);
	skd = sunputc(dd);
	fsfile(dr);
	skr = sunputc(dr);
	if(sfbeg(dr) == 1 || (sfbeg(dr) == 0 && sbackc(dr) == 0)){
		sputc(dr,skr);
		pushp(dr);
		errorrt(MSGSTR(DIVZERO,"divide by 0\n"));/*MSG*/
	}
	c = k-skd+skr;
	if(c < 0)r = removr(dd,-c);
	else {
		r = add0(dd,c);
		irem = 0;
	}
	arg1 = r;
	arg2 = dr;
	savk = k;
	return(0);
}



/*
 *  NAME:  removr
 *
 *  FUNCTION: Divide the input value by 10^'n'.  Return the quotient and
 *		set 'irem' to the remainder.  If n is even, this can be
 *		done by copying the n/2 least-significant digits to the
 *		remainder and copying the rest of the digits to the result.
 *		Otherwise, a real division is needed.
 *
 *		The input value is released.
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
removr(struct blk *p,
       int n)
{
	register struct blk *q,*s,*r;

	rewind(p);			/* Reset rd pointer to beginning. */
	q = salloc((n+1)/2);		/* Allocate space for remainder. */
	/* Copy low-order digits to remainder. */
	while(n>1){
		sputc(q,sgetc(p));
		n -= 2;
	}
	r = salloc(2);

	/* Copy remaining (high-order digits to quotient */
	while(sfeof(p) == 0) sputc(r,sgetc(p));
	release(p);
	if(n == 1){
		struct blk *rem;
		s = divn(r,tenptr,&rem); /* Compute the real quotient,
					    and keep the remainder. */
		release(r);
		r = s;
		rewind(rem);
		/* Any non-zero remainder of the division by 10 is between
		   1 and 9 and becomes the leading digit of the remainder
		   computed by this function. */
		if(sfeof(rem) == 0)sputc(q,sgetc(rem));
		release(rem);
	}
	irem = q;
	return(r);
}



/*
 *  NAME:  sqrt
 *
 *  FUNCTION:  Calculates the square root of the top element on the stack.
 *
 *  RETURN VALUE:  The square root.
 *
 */
static struct blk *
sqrt(struct blk *p)
{
	struct blk *t;
	struct blk *r,*q,*s;
	int c,n,nn;

	n = length(p);
	fsfile(p);
	c = sbackc(p);
	if((n&1) != 1)c = c*100+(sfbeg(p)?0:sbackc(p));
	n = (n+1)>>1;
	r = salloc(n);
	zero(r);
	seekc(r,n);
	nn=1;
	while((c -= nn)>=0)nn+=2;
	c=(nn+1)>>1;
	fsfile(r);
	sbackc(r);
	if(c>=100){
		c -= 100;
		salterc(r,c);
		sputc(r,1);
	}
	else salterc(r,c);
	while(1){
		q = divn(p,r,NULL);	/* r can't be 0; no remainder needed */
		s = add(q,r);
		release(q);
		q = divn(s,twoval, NULL); /* Remainder not needed. */
		release(s);
		s = copy(r,length(r));
		chsign(s);
		t = add(s,q);
		release(s);
		fsfile(t);
		nn = sfbeg(t)?0:sbackc(t);
		if(nn>=0)break;
		release(r);
		release(t);
		r = q;
	}
	release(t);
	release(q);
	release(p);
	return(r);
}


/*
 *  NAME:  exp
 *
 *  FUNCTION:  Raises 'base' to the 'ex' power and returns the result.
 *
 *  RETURN VALUE:  The result.
 *
 */
static struct blk *
exp(struct blk *base,
    struct blk *ex)
{
	register struct blk *r,*e,*p;
	struct blk *e1,*t,*cp;
	int temp,c,n;
	r = salloc(1);
	sputc(r,1);
	p = copy(base,length(base));
	e = copy(ex,length(ex));
	fsfile(e);
	if(sfbeg(e) != 0) {
		release(p);
		release(e);
		return(r);
	}
	temp=0;
	c = sbackc(e);
	if(c<0){
		temp++;
		chsign(e);
	}
	while(length(e) != 0){
		struct blk *rem;

		e1=divn(e,twoval, &rem);
		release(e);
		e = e1;
		n = length(rem);
		release(rem);
		if(n != 0){
			e1=mult(p,r);
			release(r);
			r = e1;
		}
		t = copy(p,length(p));
		cp = mult(p,t);
		release(p);
		release(t);
		p = cp;
	}
	if(temp != 0){
		if((c = length(base)) == 0){
			release(p);
			release(e);
			return(r);
		}
		if(c>1)create(r);
		else{
			rewind(base);
			if((c = sgetc(base))<=1){
				create(r);
				sputc(r,c);
			}
			else create(r);
		}
	}
	release(p);
	release(e);
	return(r);
}



/*
 *  NAME:  init
 *
 *  FUNCTION:  This is the first function which is executed.  It sets up
 *		the input file, makes sure it can create a directory,
 *		and initializes some variables.
 *
 *  RETURN VALUE: none
 *
 */
static void
init(int argc,
     char *argv[])
{
	register struct sym *sp;
	struct sym *sptr;
	struct stat statbuf;

	/* When an interrupt signal is found, execute onintr. */
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, (void (*)(int))onintr);
	setbuf(stdout,(char *)NULL);
	svargc = --argc;
	svargv = argv;
	while(svargc>0 && svargv[1][0] == '-'){
		svargc--;
		svargv++;
	}

	if(svargc<=0)curfile = stdin;
	else if((curfile = fopen(svargv[1],"r")) == NULL){
		printf(
		MSGSTR(COF,"can't open file %s\n")/*MSG*/
		,svargv[1]);
		exit(1);
		}
	else {
		stat(svargv[1], &statbuf);
		if((statbuf.st_mode & S_IFMT) == S_IFDIR){
		printf(
		MSGSTR(CBD,"%s cannot be a directory\n")/*MSG*/
		, svargv[1]);
		exit(1);
		}
	}
	dummy = (char *)malloc(1);
	scalptr = salloc(1);
	sputc(scalptr,0);
	basptr = salloc(1);
	sputc(basptr,10);
	log10=log2(10L);
	tenptr = salloc(1);
	sputc(tenptr,10);
	inbas = salloc(1);
	sputc(inbas,10);
	twoval = salloc(1);
	sputc(twoval,2);
	chptr = salloc(0);
	strptr = salloc(0);
	stkptr = stkbeg;
	stkerr = 0;
	readptr = &readstk[0];
	k=0;
	sp = sptr = &symlst[0];
	while(sptr < &symlst[TBLSZ-1]){
		sptr->next = ++sp;
		sptr++;
	}
	sptr->next=0;
	sptr = NULL;
	sfree = &symlst[0];
	return;
}



/*
 *  NAME:  onintr
 *
 *  FUNCTION:  This function is called when an interrupt is encountered
 *		in the init procedure.  It will resolve the interrupt, 
 *		assign stdin as the current file, and call the commands
 *		procedure.
 *
 *  RETURN VALUE: none
 *
 */
static void
onintr(void){
	signal(SIGINT, (void (*)(int))onintr);
	while(readptr != &readstk[0]){
		if(*readptr != 0){release(*readptr);}
		readptr--;
	}
	curfile = stdin;
	commnds();
}

/*
 *  NAME:  pushp
 *
 *  FUNCTION:  Pushes an element onto the stack, if there is room.
 *
 *  RETURN VALUE:  none
 *
 */
static void
pushp(struct blk *p)
{
	if(stkptr == stkend){
		printf(
		MSGSTR(OOSS,"out of stack space\n"));/*MSG*/
		return;
	}
	stkerr=0;
	*++stkptr = p;
}



/*
 *  NAME:  pop
 *
 *  FUNCTION:  Pops an element off the stack.
 *
 *  RETURN VALUE:  NULL, if the stack is empty (stkerr is set to 1 as well)
 *		The popped element, otherwise.
 *
 */
static struct blk *
pop(void){
	if(stkptr == stack){
		stkerr=1;
		return(0);
	}
	return(*stkptr--);
}



/*
 *  NAME:  readin
 *
 *  FUNCTION:  Reads in the data from the input file and does some 	
 *		rudimentary error checking before passing the value 
 *		back to the calling routine. Checks value of input character
 *		to be a valid digit . Doesn't parse more than 99 places right
 *		of the decimal point(radix char). 
 *
 *  RETURN VALUE:  The valid input.
 *
 */
static struct blk *
readin(void){
	register struct blk *p,*q;
	int dp,dpct;
	register int c;

	dp = dpct=0; /*dp checks validity of radix char.*/
	p = salloc(0);/* dpct counts # of chars after the radix*/

	if (ibase0)
		dpct = 100;		/* Silently ignore fractional digits
					   if the input base is 0 (unless more
					   than 99 digits are entered). */
	while(1){
		c = readc();
		if (c == '\\'){ /*ignore backsapce*/
			c = readc();
		}
		if (c == '\n' && ((dpct>99 && !ibase0) || dpct > 199)){
			printf(MSGSTR(MAXRAD,
			"Maximum precision of 99 digits:ignoring last digit(s)\n"));
			dpct=99;
		}
		if (c == '.') {
			c = readc();
			dp = 1;		/* Decimal point seen */
		}
		if(c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else {
			if(c >= '0' && c <= '9') c -= '0';
			else break;
		}
		if(dp) dpct++;		/* Count fractional digits */

		if (dpct<=99){
			create(chptr);
			if(c != 0)sputc(chptr,c);
			q = mult(p,inbas);
			release(p);
			p = add(chptr,q);
			release(q);
		}
	} /*while 1*/


	unreadc(c);  
	if(dp == 0 || ibase0){ 
		sputc(p,0);  
		return(p);  
	}  
	else {  
		q = scale(p,dpct);  
		return(q);  
	}  
}



/*
 *  NAME:  add0
 *
 *  FUNCTION:  Add 'ct' leading base 10 0s to the input value and
 *		return the result.  It 'ct' is odd, multiplication is
 *		needed to add the last 0.
 *
 *		The input value is freed.
 *
 *
 */
static struct blk *
add0(struct blk *p,
     int ct)
{
	/* returns pointer to struct with ct 0's & p */
	register struct blk *q,*t;

	q = salloc(length(p)+(ct+1)/2);
	while(ct>1){
		sputc(q,0);
		ct -= 2;
	}
	rewind(p);
	while(sfeof(p) == 0){
		sputc(q,sgetc(p));
	}
	release(p);
	if(ct == 1){
		t = mult(tenptr,q);
		release(q);
		return(t);
	}
	return(q);
}



/*
 *  NAME:  mult
 *
 *  FUNCTION:  Multiplies 'p' and 'q' and returns the product.
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
mult(struct blk *p,
     struct blk *q)
{
	register struct blk *mp,*mq,*mr;
	int sign,offset,carry;
	int cq,cp,mt,mcr;

	offset = sign = 0;
	fsfile(p);
	mp = p;
	if(sfbeg(p) == 0){
		if(sbackc(p) < 0){
			mp = copy(p,length(p));
			chsign(mp);
			sign = ~sign;
		}
	}
	fsfile(q);
	mq = q;
	if(sfbeg(q) == 0){
		if(sbackc(q) < 0){
			mq = copy(q,length(q));
			chsign(mq);
			sign = ~sign;
		}
	}
	mr = salloc(length(mp)+length(mq));
	zero(mr);
	rewind(mq);
	while(sfeof(mq) == 0){
		cq = sgetc(mq);
		rewind(mp);
		rewind(mr);
		mr->rd += offset;
		carry=0;
		while(sfeof(mp) == 0){
			cp = sgetc(mp);
			mcr = sfeof(mr)?0:slookc(mr);
			mt = cp*cq + carry + mcr;
			carry = mt/100;
			salterc(mr,mt%100);
		}
		offset++;
		if(carry != 0){
			mcr = sfeof(mr)?0:slookc(mr);
			salterc(mr,mcr+carry);
		}
	}
	if(sign < 0){
		chsign(mr);
	}
	if(mp != p)release(mp);
	if(mq != q)release(mq);
	return(mr);
}



/*
 *  NAME:  chsign
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
chsign(struct blk *p)
{
	register int carry;
	register int ct;

	carry=0;
	rewind(p);
	while(sfeof(p) == 0){
		ct=100-slookc(p)-carry;
		carry=1;
		if(ct>=100){
			ct -= 100;
			carry=0;
		}
		salterc(p,ct);
	}
	if(carry != 0){
		sputc(p,-1);
		fsfile(p);
		sbackc(p);
		ct = sbackc(p);
		if(ct == 99){
			truncate(p);
			sputc(p,-1);
		}
	}
	else{
		fsfile(p);
		ct = sbackc(p);
		if(ct == 0)truncate(p);
	}
}


/*
 *  NAME:  readc
 *
 *  FUNCTION:  Reads characters in from either stdin, from a specified 	
		file, or from a string on the read stack
 *
 *  RETURN VALUE:  The character read in.
 *
 */
static int
readc(void){
	int lastchar;

	while(1) {
		if((readptr != &readstk[0]) && (*readptr != 0)){
			if(!sfeof(*readptr))
				return *((*readptr)->rd++);
			release(*readptr);
			readptr--;
		}
		else {
			lastchar = getc(curfile);
			if(lastchar != EOF)return(lastchar);
			if(readptr != &readptr[0]){
				readptr--;
				if(*readptr == 0)curfile = stdin;
			}
			else {
				if(curfile != stdin){
					fclose(curfile);
					curfile = stdin;
				}
				else exit(0);
			}
		}
	}
}



/*
 *  NAME:  unreadc
 *
 *  FUNCTION:  Puts c back into the file or removes it from the stack. 
 *
 *  RETURN VALUE:  none
 *
 */
static void
unreadc(char c)
{
	if((readptr != &readstk[0]) && (*readptr != 0)){
		sungetc(*readptr,c);
	}
	else ungetc((int)c,curfile);
}



#ifdef KEEP_DIGIT_GROUPS
/*
 *  NAME:  check_count
 *
 *  FUNCTION: Updates the number of characters remaining on the current output
 *		line.  If necessary, prints a \ and newline if the next
 *		digit group won't fit on the line.  The parameter 'remaining'
 *		must be greater than fw when more than one digit group remains
 *		to be printed.  This function avoids splitting a line before
 *		the last digit group when the digit group would have fit.
 *
 *  RETURN VALUE:
 *
 */
check_count(int remaining) {
	count -= fw;
	if (count <= fw
	    && remaining > 0
	    && (remaining > fw || count < fw)) {
		printf("\\\n");
		count=ll;
	}
}
#endif

/*
 *  NAME:  OUT_NL
 *
 *  FUNCTION: Print a newline.  Print a preceding pending character
 *		is there is one.
 *
 *  RETURN VALUE:
 *
 */
static OUT_NL(void) {
	if (prev_char != -1) {
		putchar(prev_char);
		prev_char = -1;
	}
	putchar('\n');
}
/*
 *  NAME:  OUTC
 *
 *  FUNCTION: Print a character and update the count of
 *	 	characters remaining in the current output line.  When a
 *		single character remains in the output line, the character is
 *		not printed but is saved in prev_char.  If another call to OUTC
 *		occurs, the line will be ended with a \, the pending character
 *		will be printed, and the new character will be printed.
 *
 *  RETURN VALUE: None
 *
 */
static void
OUTC(int x) {
#ifdef DEBUG
	if ((x < '0' || x > '9')
	    && (x < 'A' || x > 'F')
	    && x != '.'
	    && x != ' '
	    && x != 'd'
	    && x != '-') {
		printf("\nBad character %d\n", x);
		abort();
	}
#endif
	switch(count) {
	      case 0:
		printf("\\\n%c%c", prev_char, x);
		prev_char = -1;
		count = ll-2;
		break;
	      case 1:
		prev_char = x;
		--count;
		break;
	      default:
		--count;
		putchar(x);
		break;
	}
}
#ifdef DEBUG
/*
 *  NAME:  OUTDIGIT
 *
 *  FUNCTION: Ensures that the parameter is a valid decimal digit.
 *		Prints the digit in ASCII by calling OUTC.
 *
 *  RETURN VALUE:
 *
 */
void
OUTDIGIT(int x) {
	if (x < 0 || x > 9) {
		printf("\nBad digit %d\n", x);
		abort();
	}
	OUTC(x+'0');
}
#else
#define OUTDIGIT(x) OUTC(x+'0')
#endif

/*
 *  NAME:  print
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
print(struct blk *hptr)
{
	int sc;
	register struct blk *p,*q,*dec;
	int dig,dout,ct;

	rewind(hptr);			/* Put read pointer at beginning */
	while(sfeof(hptr) == 0){
		/* Check for any digit greater than 99.  If there is one,
		   assume the value is a string. When this happens, there
		   is no splitting of lines. */
		if((int)sgetc(hptr) > 99){
			rewind(hptr);
			while(sfeof(hptr) == 0){
				printf("%c",sgetc(hptr));
			}
			printf("\n");
			return;
		}
	}
	fsfile(hptr);			/* Put read pointer at end. */
	sc = sbackc(hptr);		/* Get number of fraction digits.  */
	if(sfbeg(hptr) != 0){
		printf("0\n");
		return;
	}

	/* Initialize number of characters remaining in output lin
	   does not include the newline character at the end of the
	   line.

	   For obase > 10^69, a single obase digit could case a line
	   length to be longer than 70 characters.  This won't occur
	   unless KEEP_DIGIT_GROUPS is defined.  Even so, since 'bc'
	   doesn't allow lines an output base that large, this doesn't
	   violate the X/Open standard. */
	count = ll;
	p = copy(hptr,length(hptr));	/* Get a working copy of the value */
	sunputc(p);			/* Delete the scale value */
	fsfile(p);
	if((sbackc(p)) < 0){
		chsign(p);
		OUTC('-');
	}
	switch(obase) {
	      case -1:
	      case 0:
		oneot(p,sc,'d');
		break;
	      case 1:
		oneot(p,sc,'1');
		break;
	      case 10:
		tenot(p,sc);
		break;
	      default:
		create(strptr);		/* Use to accumulate output value. */
		dig = log10*sc;
		dout = ((dig/10) + dig) /logo;
		if (sc)
			dec = getdec(p,sc); /* Get the fractional part of p */
		p = removc(p,sc);	/* Get the integral part of p */

		/* Generate the integral representation, accumulating the
		   characters in strptr. */
		while(length(p) != 0){
			struct blk *rem;

			/* basptr isn't 0 or we wouldn't
			   be in this default case. */
			q = divn(p,basptr,&rem);
			release(p);
			p = q;
			(*outdit)(rem, strptr, 0);
		}
		release(p);
		fsfile(strptr);
#ifdef KEEP_DIGIT_GROUPS
		while(sfbeg(strptr) == 0) {
			for (i = 0; i < fw; i++)
				putchar(sbackc(strptr));
			check_count(strptr->rd - strptr->beg + (sc ? fw : 0));
		}
#else
		while(sfbeg(strptr) == 0)OUTC(sbackc(strptr));
#endif
		if(sc) {
			create(strptr);
			sputc(strptr, '.'); /* Add decimal point. */
			ct=0;
			do{
				q = mult(basptr,dec);
				release(dec);
				dec = getdec(q,sc);
				p = removc(q,sc);
				(*outdit)(p, strptr, 1);
			}while(++ct < dout);
			if (fw > 1)
				sunputc(strptr); /* Delete last blank */
			rewind(strptr);
#ifdef KEEP_DIGIT_GROUPS
			while(sfeof(strptr) == 0) {
				check_count(strptr->wt - strptr->rd - fw); 
				for (i = 0; i < fw; i++)
					putchar(sgetc(strptr));
			}
#else
			while(sfeof(strptr) == 0)OUTC(sgetc(strptr));
#endif
			release(dec);
		}
#ifdef KEEP_DIGIT_GROUPS
		putchar('\n');
#else
		OUT_NL();
#endif
		break;
	}
}



/*
 *  NAME:  getdec
 *
 *  FUNCTION: Compute and return the fractional part of the input value as
 *		an integer.
 *		Do not modify the input value.
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
getdec(struct blk *p,
       int sc)
{
	int cc;
	register struct blk *q,*t,*s;

	rewind(p);
	if(length(p)*2 < sc){
		/* Input value is less than 1. */
		q = copy(p,length(p));
		return(q);
	}
	q = salloc(length(p));

	/* Copy base-100 digits that represent one or two fractional
	   base-10 digits. */
	while(sc >= 1){
		sputc(q,sgetc(p));
		sc -= 2;
	}
	if(sc != 0) {
		/* We got 1 too many base-10 digits. */
		t = mult(q,tenptr);
		s = salloc(cc = length(q));
		release(q);
		rewind(t);
		while(cc-- > 0)sputc(s,sgetc(t));
		sputc(s,0);
		release(t);
		t = divn(s,tenptr, NULL); /*  Remainder not needed */
		release(s);
		return(t);
	}
	return(q);
}


/*
 *  NAME:  tenot
 *
 *  FUNCTION: Print the input value to standard output, using a base 10
 *	representation.
 *
 *	NOTE:  If the absolute value of the input value is greater than
 *		0 but less than 1, a leading 0 before the decimal point
 *		will only be printed if there are an odd number of fractional
 *		digits.  This behavior is allowed by the XOPEN description
 *		for 'bc'.
 *
 *  INPUT VALUE:  The input value must be positive and does not have
 *		an explicit scale byte.
 *
 *  RETURN VALUE: None
 *
 */
static void
tenot(struct blk *p,
      int sc)				/* Number of fraction digits
					   (in base 10). */
{
	register int c;

	fsfile(p);			/* Set the read pointer to the end,
					   since we'll be printing the
					   most-significant digit first. */

	if(!sfbeg(p) && (p->rd - p->beg - 1)*2 >= sc) {
		/* Print any most-significant base-100 digits that are
		   completely before the decimal point. */
		c = sbackc(p);
		if (c >= 10)
			OUTDIGIT(c/10);
		OUTDIGIT(c%10);
		if (c != 0) {
			while(!sfbeg(p) && (p->rd - p->beg - 1)*2 >= sc) {
				c = sbackc(p);
				OUTDIGIT(c/10);
				OUTDIGIT(c%10);
			}
		}
	}
	if(sc) {
		/* Print decimal point and fraction */
		if((p->rd - p->beg)*2 > sc){
			/* Decimal point is between the 2 digits of the
			   current base 100 digit. */
			c = sbackc(p);
			OUTDIGIT(c/10);
			OUTC('.');
			OUTDIGIT(c%10);
			sc--;
		}
		else
			OUTC('.');

		/* For a number less than 1, leading 0s in the fraction
		   are not explicitly represented, so these leading 0s
		   must be generated here. */
		while(sc > (p->rd - p->beg)*2) {
			OUTC('0');
			sc--;
		}
		while(sc > 1){
			c = sbackc(p);
			OUTDIGIT(c/10);
			OUTDIGIT(c%10);
			sc -= 2;
		}
		if(sc == 1)
			OUTDIGIT(sbackc(p)/10);
	}
	OUT_NL();
	release(p);
}


/*
 *  NAME:  oneot
 *
 *  FUNCTION: Print a value in base 1.  To 'dc', this means to display
 *		n characters (specified by parameter 'ch') where n
 *		is the integral value of the input 'struct blk'.
 *
 *		NOTE:  Base 1 is not allowed in bc, so this function
 *			is not used by 'bc'
 *
 *  RETURN VALUE:
 *
 */
static void
oneot(struct blk *p,
      int sc,
      char ch)
{
	register struct blk *q;

	q = removc(p,sc);
	create(strptr);
	sputc(strptr,-1);
	while(length(q)>0){
		p = add(strptr,q);
		release(q);
		q = p;
		OUTC(ch);
	}
	release(q);
	OUT_NL();
}



/*
 *  NAME:  hexot
 *
 *  FUNCTION: The input value represents an integer between 0 and 15.
 *		Add the ASCII value of the input value to the pseudo-value
 *		buffer.  (The 'flg' parameter is ignored.)
 *
 *  RETURN VALUE:
 *
 */
static void
hexot(struct blk *p,
      struct blk *buffer,
      int flg)
{
	register int c;
	rewind(p);			/* Reset read pointer to beginning. */
	if(sfeof(p)) {
		sputc(buffer,'0');	/* Value is 0 */
	}
	else {
		c = sgetc(p);
		if(c < 16) {
			sputc(buffer, c<10?c+'0':c-10+'A');
		}
		else
			printf( MSGSTR(HDO16,"hex digit > 16"));/*MSG*/
	}
	release(p);
}



/*
 *  NAME:  bigot
 *
 *  FUNCTION: The input value represents an integer between 0 and the current
 *		output base.  The decimal representation of the input value
 *		is written to the pseudo-value buffer.  The routine always
 *		writes fw-1 decimal digits to represent a single base obase
 *		digit.
 *
 *	If flg is 0, the input value is not a fractional digit, and the
 *		decimal representation of the base obase digit should be
 *		written in reverse order, which is the order the decimal
 *		digits are generated by this function.
 *
 *	If flg is 1, the decimal representation is saved in a temporary
 *	buffer, then copied in the correct order to 'buffer'.
 *
 *
 *  RETURN VALUE: None
 *
 */
static void
bigot(struct blk *p,
      struct blk *buffer,
      int flg)
{
	register struct blk *t,*q;
	struct blk *rem;
	register int l;
	int neg;

	if(flg == 1)t = salloc(fw-1);
	else{
		t = buffer;
		l = length(buffer)+fw-1;
	}
	neg=0;

	if(length(p) != 0){
		fsfile(p);
		if(sbackc(p) < 0){
			neg=1;
			chsign(p);
		}
		while(length(p) != 0){
			q = divn(p,tenptr, &rem);
			release(p);
			p = q;
			rewind(rem);
			sputc(t,sfeof(rem)?'0':sgetc(rem)+'0');
			release(rem);
		}
	}
	release(p);
	if(flg == 1){
		l = fw - 1 - length(t) - neg;
		if(neg)
			sputc(buffer,'-');
		fsfile(t);
		while(l-- > 0)sputc(buffer,'0');
		while(sfbeg(t) == 0)sputc(buffer,sbackc(t));
		release(t);
	}
	else{
		l -= length(buffer) + neg;
		while(l-- > 0)sputc(buffer,'0');
		if(neg)
			sputc(buffer,'-');
	}
	sputc(buffer,' ');
}



/*
 *  NAME:  add
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
add(struct blk *a1,
    struct blk *a2)
{
	register struct blk *p;
	register int carry,n;
	int size;
	int c,n1,n2;

	size = length(a1)>length(a2)?length(a1):length(a2);
	p = salloc(size+1);
	rewind(a1);
	rewind(a2);
	carry=0;
	while(--size >= 0){
		n1 = sfeof(a1)?0:sgetc(a1);
		n2 = sfeof(a2)?0:sgetc(a2);
		n = n1 + n2 + carry;
		if(n>=100){
			carry=1;
			n -= 100;
		}
		else if(n<0){
			carry = -1;
			n += 100;
		}
		else carry = 0;
		sputc(p,n);
	}
	if(carry != 0)sputc(p,carry);
	fsfile(p);
	if(sfbeg(p) == 0){
		while(sfbeg(p) == 0 && (c = sbackc(p)) == 0)
			/* skip */;
		if(c != 0)salterc(p,c);
		truncate(p);
	}
	fsfile(p);
	if(sfbeg(p) == 0 && sbackc(p) == -1){
		while((c = sbackc(p)) == 99){
			if(c == EOF)break;
		}
		sgetc(p);
		salterc(p,-1);
		truncate(p);
	}
	return(p);
}



/*
 *  NAME:  eqk
 *
 *  FUNCTION: Take the top 2 elements off the stack, and force them to have
 *		the same scale by shifting one or the other.
 *
 *  RETURN VALUE: 1 if the stack doesn't have 2 elements.
 *		0, otherwise.
 *
 *  GLOBAL VARIABLES:	skp is set to the larger scale.
 *			arg1 is set to the shifted first value
 *			arg2 is set to the shifted second value
 *
 */
static int
eqk(void){
	register struct blk *p,*q;
	register int skp;
	int skq;

	p = pop();
	EMPTYS;
	q = pop();
	EMPTYSR(p);
	skp = sunputc(p);
	skq = sunputc(q);
	if(skp == skq){
		arg1=p;
		arg2=q;
		savk = skp;
		return(0);
	}
	else if(skp < skq){
		savk = skq;
		p = add0(p,skq-skp);
	}
	else {
		savk = skp;
		q = add0(q,skp-skq);
	}
	arg1=p;
	arg2=q;
	return(0);
}



/*
 *  NAME:  removc
 *
 *  FUNCTION: Remove 'n' base-10 digits from the least-significant end of
 *		the input value and return the result.  If 'n' is odd,
 *		division is required to remove the last digit.
 *	The value pointed to by 'p' is freed.
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
removc(struct blk *p,
       int n)
{
	register struct blk *q,*r;

	rewind(p);			/* Reset rd pointer to beginning. */
	while(n>1){
		sgetc(p);		/* Skip over 1 base-100 digit (or
					   2 base-10 digits) at a time. */
		n -= 2;
	}
	q = salloc(2);
	/* Copy remaining base-100 digits. */
	while(sfeof(p) == 0)sputc(q,sgetc(p));
	if(n == 1){
		r = divn(q,tenptr, NULL); /* Remainder not needed. */
		release(q);
		q = r;
	}
	release(p);
	return(q);
}



/*
 *  NAME:  scalint
 *
 *  FUNCTION: Truncate a value to an integer.
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
scalint(struct blk *p)
{
	register int n;
	n = sunputc(p);			/* Get the scale */
	p = removc(p,n);		/* Remove n least-significant digits. */
	return(p);
}



/*
 *  NAME:  scale
 *
 *  FUNCTION:
 *
 *	Note:  This function shouldn't be called if inbas is 0
 *
 *  RETURN VALUE:
 *
 */
static struct blk *
scale(struct blk *p,
      int n)
{
	register struct blk *q,*s,*t;

	t = add0(p,n);
	q = salloc(1);
	sputc(q,n);
	s = exp(inbas,q);		/* inbas shouldn't be 0,
					   so s shouldn't be 0 */
	release(q);
	q = divn(t,s, NULL);		/* Remainder not needed */
	release(t);
	release(s);
	sputc(q,n);
	return(q);
}



/*
 *  NAME:  subt
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static int
subt(void){
	register struct blk *r;

	r=pop();
	EMPTYS;
	savk = sunputc(r);
	chsign(r);
	sputc(r,savk);
	pushp(r);
	if(eqk() != 0)return(1);
	r = add(arg1,arg2);
	release(arg1);
	release(arg2);
	sputc(r,savk);			/* Set the precision of the result. */
	pushp(r);
	return(0);
}



/*
 *  NAME:  command
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static int
command(void) {
	int c;
	char line[100],*sl;
	register (*savint)(int),pid,rpid;
	int retcode;

	switch(c = readc()){
	case '<':
		return(cond(NL));
	case '>':
		return(cond(NG));
	case '=':
		return(cond(NE));
	default:
		sl = line;
		*sl++ = c;
		while((c = readc()) != '\n')*sl++ = c;
		*sl = 0;
		if((pid = fork()) == 0){
			execl("/bin/sh","sh","-c",line,0);
			exit(0100);
		}
		savint = (int (*)(int))signal(SIGINT, SIG_IGN);
		while((rpid = wait(&retcode)) != pid && rpid != -1)
			/* skip */;
		signal(SIGINT, (void (*)(int))savint);
		printf("!\n");
		return(0);
	}
}



/*
 *  NAME:  cond
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static int
cond(char c)
{
	register struct blk *p;
	register int cc;

	if(subt() != 0)return(1);
	p = pop();
	sunputc(p);
	if(length(p) == 0){
		release(p);
		if(c == '<' || c == '>' || c == NE){
			readc();
			return(0);
		}
		load();
		return(1);
	}
	else {
		if(c == '='){
			release(p);
			readc();
			return(0);
		}
	}
	if(c == NE){
		release(p);
		load();
		return(1);
	}
	fsfile(p);
	cc = sbackc(p);
	release(p);
	if((cc<0 && (c == '<' || c == NG)) ||
		(cc >0) && (c == '>' || c == NL)){
		readc();
		return(0);
	}
	load();
	return(1);
}



/*
 *  NAME:  load
 *
 *  FUNCTION: Push a copy of a named register onto the top of the stack.
 *		(The register name must be the next input character.)
 *		If the register has no value, 0 is pushed.
 *
 *  RETURN VALUE: None
 *
 */
static void
load(void) {
	register int c;
	struct sym *sptr;
	register struct blk *p,*q;
	struct blk *s;
	c = readc() & 0377;

	sptr = stable[c];
	if(sptr != NULL){
		p = sptr->val;
		if(*p->last == TAG_ARRAY) {
			q = salloc(length(p));
			rewind(p);
			while((p->wt - p->rd) >= PTRSZ) {
				s = *(struct blk **)(p->rd);
				p->rd += PTRSZ;
				if(s != NULL)
					s = copy(s, length(s));
				*(struct blk **)(q->wt) = s;
				q->wt += PTRSZ;
			}
		}
		else
			q = copy(p,length(p));
		/* Preserve tag */
		*q->last = *p->last;
	}
	else{
		/* No value in register.  Push a value of 0. */
		q = salloc(1);
		sputc(q,0);
	}
	pushp(q);
}



/*
 *  NAME:  log2
 *
 *  FUNCTION: Compute floor(log (base2) of 'n').
 *
 *  RETURN VALUE: Returns 31 if n < 0;
 *		0 if n == 0;
 *		floor(log (base 2) n) otherwise.
 *
 */
static int
log2(long n)
{
	register int i;

	if(n == 0)return(0);
	i=31;
	if(n<0)return(i);
	while((n= n<<1) >0)i--;
	return(--i);
}



/*
 *  NAME:  salloc
 *
 *  FUNCTION:  Allocates a new 'struct blk' with a buffer
 *		containing 'size' bytes (plus 1 for the tag).
 *
 *  RETURN VALUE:  A pointer to the new structure.
 *
 */
static struct blk *
salloc(int size)
{
	register struct blk *hdr;
	register char *ptr;

	all++;
	/* Allocate a buffer of at least 3 bytes (plus 1 for the tag).
	   Most implementations of malloc() will allocate at least
	   4 bytes regardless of the requested size, and we may avoid
	   a few reallocs() by increasing the length slightly. */
	if (size < 3)
		size = 3;
	nbytes += size + 1;
	ptr = malloc(size + 1);
	if(ptr == NULL){
		garbage("salloc");
		if((ptr = malloc(size + 1)) == NULL)
			ospace("salloc");
 	}
	if((hdr = hfree) == 0)hdr = morehd();
	hfree = (struct blk *)hdr->rd;
	hdr->rd = hdr->wt = hdr->beg = ptr;
	hdr->last = ptr+size;
	*hdr->last = TAG_NUMBER;	/* Default tag. */
	return(hdr);
}



/*
 *  NAME:  morehd
 *
 *  FUNCTION: Allocate a new linked list of 'struct blks'.  
 *
 *  RETURN VALUE: The address of the first blk on the chain.
 *
 */
static struct blk *
morehd(void){
	register struct blk *h,*kk;
	headmor++;
	nbytes += HEADSZ;
	hfree = h = (struct blk *)malloc(HEADSZ);
	if(hfree == 0){
		garbage("morehd");
		if((hfree = h = (struct blk *)malloc(HEADSZ)) == 0)
			ospace("headers");
	}
	kk = h;
	while(h<hfree+(HEADSZ/BLK))(h++)->rd = (char *)++kk;
	(--h)->rd=0;
	return(hfree);
}



/*
 *  NAME:  copy
 *
 *  FUNCTION: Copy a 'struct blk'.  The buffer in the new 'struct blk' will
 *		contain 'size' bytes (plus one for the tag).  See comments
 *		in the code for the behavior if the size of hptr's buffer
 *		is not 'size'.
 *
 *  RETURN VALUE: A pointer to the new, initialized 'struct blk'
 *
 */
static struct blk *
copy(struct blk *hptr,
     int size)
{
	register struct blk *hdr;
	register unsigned sz;
	register char *ptr;
	int	zero_size;

	all++;				/* Count total uses of blk's. */
	nbytes += (size+1);		/* Count tag byte in allocated
					   bytes. */
	sz = length(hptr);
	if (size < sz) {
		sz = size;		/* Not all bytes copied. */
		zero_size = 0;		/* No additional bytes will need
					   to be zeroed. */
	}
	else {
		/* If new buffer is larger than the hptr's buffer, the
		   additional bytes must be zeroed. */
		zero_size = size - sz;
	}

	ptr = nalloc(hptr->beg, size+1, sz);
	if(ptr == NULL) {
		garbage("copy");
		if((ptr = nalloc(hptr->beg, size+1, sz)) == NULL){
			printf(MSGSTR(PCOPYSIZ,"copy size %d\n")/*MSG*/
			       ,size);
			ospace("copy");
		}
	}
	if((hdr = hfree) == NULL) hdr = morehd();
	hfree = (struct blk *)hdr->rd;
	hdr->rd = hdr->beg = ptr;
	hdr->last = ptr+size;
	*hdr->last = TAG_NUMBER;	/* Default tag is number. */
	hdr->wt = ptr+sz;
	if (zero_size)
		memset(hdr->wt, 0, zero_size);
	return(hdr);
}



/*
 *  NAME:  sdump
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
sdump(char *s1,
      struct blk *hptr)
{
	char *p;

	printf(MSGSTR(SDUMP, "%s %o rd %o wt %o beg %o last %o\n"),
	       s1,hptr, hptr->rd, hptr->wt, hptr->beg, hptr->last);
#ifdef DEBUG
	if (hptr->last < hptr->beg)
		printf("Invalid 'last' pointer.\n");
	else {
		if (hptr->wt < hptr->beg || hptr->wt > hptr->last)
			printf("Invalid 'wt' pointer.\n");
		else if (hptr->rd < hptr->beg || hptr->rd > hptr->wt)
			printf("Invalid 'rd' pointer.\n");
		printf("TAG=%d\t=> ", *hptr->last);
	}
#endif

	p = hptr->beg;
#ifdef DEBUG
	if (p == hptr->rd)
		printf(" [ ");
#endif
	while(p < hptr->wt) {
		printf("%d ",*p++);
#ifdef DEBUG
		if (p == hptr->rd)
			printf(" [ ");
		if (p == hptr->wt)
			printf(" ] ");
#endif
	}
	printf("\n");
}



/*
 *  NAME:  seekc
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
seekc(struct blk *hptr,
      int n)
{
	int nn;
	register char *p;
	int tag;

	nn = hptr->last - hptr->beg;
	if (n > nn) {
		nbytes += n - nn;
		tag = *hptr->last;
		p = realloc(hptr->beg, n+1);
		if(p == NULL){
			free(hptr->beg);
			hptr->beg = realloc(hptr->beg,
					    (hptr->last - hptr->beg + 1));
			garbage("seekc");
			if ((p = realloc(hptr->beg, n+1)) == 0)
				ospace("seekc");
		}
		hptr->beg = p;
		hptr->wt = hptr->last = hptr->rd = p+n;
		*hptr->last = tag;
	}
	else {
		hptr->rd = hptr->beg + n;
		if(hptr->rd > hptr->wt)
			hptr->wt = hptr->rd;
	}
}


/*
 *  NAME:  get_subscript
 *
 *  FUNCTION: Pop the stack and use the value of the element as an
 *		array subscript.
 *		Valid subscripts are in the range 0..MAXIND-1.
 *
 *  RETURN VALUE: -1 if errors occurred.
 *		the array subscript, otherwise.
 *
 */
static int
get_subscript(void)
{
	int c;
	struct blk *p, *q;

	p = pop();	/* Subscript */
	if(stkerr != 0){
		printf("stack empty\n");
		return -1;
	}
	q = scalint(p);
	fsfile(q);
	c = 0;
	if (!sfbeg(q) && ((c = sbackc(q)) < 0)) {
		printf(MSGSTR(EM_NEGIND,"neg index\n"));/*MSG*/
		c =  -1;
	}
	else if (length(q) > 2){
		printf(MSGSTR(EM_IND2BIG,"index too big\n"));/*MSG*/
		c = -1;
	}
	else {
		if(!sfbeg(q))
			c = c*100 + sbackc(q);
		if(c >= MAXIND){
			printf(MSGSTR(EM_IND2BIG,"index too big\n"));/*MSG*/
			c = -1;
		}
	}
	release(q);
	return c;
}
/*
 *  NAME:  more
 *
 *  FUNCTION:  Allocates more space.
 *
 *  RETURN VALUE:  none
 *
 */
static void
more(struct blk *hptr)
{
	register size_t size;
	register char *p;
	int tag = *hptr->last;

	size = (hptr->last-hptr->beg+1)*2;
	nbytes += size/2;
	p = realloc(hptr->beg, size);
	if(p == NULL){
		free(hptr->beg);
		hptr->beg = realloc(hptr->beg, hptr->last - hptr->beg + 1);
		garbage(MSGSTR(MORE, "more"));
		if((p = realloc(hptr->beg, size)) == 0)
			ospace(MSGSTR(MORE, "more"));
	}
	hptr->rd = hptr->rd - hptr->beg + p;
	hptr->wt = hptr->wt - hptr->beg + p;
	hptr->beg = p;
	hptr->last = p+size-1;
	*hptr->last = tag;
}



/*
 *  NAME:  ospace
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
ospace(char *s)
{
	printf(MSGSTR(OS1,"out of space: %s\n"),s);/*MSG*/
	printf(MSGSTR(HEADMOR,"all %ld rel %ld headmor %ld\n"),all,rel,headmor);
	printf(MSGSTR(NBYTES,"nbytes %ld\n"),nbytes);
	sdump(MSGSTR(STK, "stk"),*stkptr);
	abort();
}



/*
 *  NAME:  garbage
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
garbage(char *s)
{
	int i;
	struct blk *p, *q;
	struct sym *tmps;
	int ct;

	for(i=0;i<TBLSZ;i++){
		tmps = stable[i];
		if(tmps != NULL){
			p = tmps->val;
			if(*p->last != TAG_ARRAY){
				do {
					if(((int)p->beg & 01)  != 0){
						printf(MSGSTR(STRING,"string %o\n"),i);
						sdump(MSGSTR(ODDBEG, "odd beg"),p);
					}
					redef(p);
					tmps = tmps->next;
				} while(tmps != 0);
				continue;
			}
			else {
				do {
					rewind(p);
					ct = 0;
					while((p->wt - p->rd) >= PTRSZ) {
						q = *(struct blk **)(p->rd);
						p->rd += PTRSZ;
						ct++;
						if(q != NULL){
							if(((int)q->beg & 01) != 0){
								printf(MSGSTR(ARR,"array %o elt %d odd\n"),i,ct);
								sdump("elt",q);
							}
							redef(q);
						}
					}
					tmps = tmps->next;
				} while(tmps != 0);
			}
		}
	}
}



/*
 *  NAME:  redef
 *
 *  FUNCTION:
 *
 *  RETURN VALUE:
 *
 */
static void
redef(struct blk *p)
{
	register offset;
	register char *newp;

	if ((int)p->beg&01) {
		printf(MSGSTR(ODD,"odd ptr %o hdr %o\n"),p->beg,p);
		ospace(MSGSTR(REFD, "redef-bad"));
	}
	free(p->beg);
	free(dummy);
	dummy = malloc((size_t)1);
	if(dummy == NULL)ospace(MSGSTR(DUMMY, "dummy"));
	newp = realloc(p->beg, p->last - p->beg + 1);
	if(newp == NULL)ospace(MSGSTR(RED, "redef"));
	offset = newp - p->beg;
	p->beg = newp;
	p->rd += offset;
	p->wt += offset;
	p->last += offset;
}



/*
 *  NAME:  release
 *
 *  FUNCTION: Free a 'struct blk'.  Free the buffer.  Put the 'struct blk'
 *		on the free list.
 *
 *  RETURN VALUE: None
 *
 */
static void
release(struct blk *p)
{
	rel++;
	nbytes -= (p->last - p->beg + 1);
	p->rd = (char *)hfree;
	hfree = p;
	free(p->beg);
}
/*
 *  NAME:  nalloc
 *
 *  FUNCTION: Allocate a buffer containing 'nbytes' bytes and copy 'nbytes'
 *		from 'p' to the buffer.  Null bytes can be copied.
 *
 *  RETURN VALUE: NULL if a new buffer cannot be allocated.
 *		Pointer to the new buffer, otherwise.
 *
 */
static char *
nalloc(char *p,
       unsigned nbytes,
       unsigned initbytes)
{
	char *r = malloc(nbytes);
	if(r==NULL)
		return(NULL);
	memcpy(r, p, initbytes);
	return r;
}
