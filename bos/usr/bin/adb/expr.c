static char sccsid[] = "@(#)02	1.19  src/bos/usr/bin/adb/expr.c, cmdadb, bos411, 9428A410j 2/1/92 16:45:21";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  chkloc, convdig, eqsubstr, eqsub, expr, getnum, hexdigit, item,
 * 	       lookupsym, readsym, symchar, term, varchk
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include "defs.h"
#define SYMNMLEN        8       /* Length of short symbol */

#define SYMMAX 256
extern SYMINFO *SymTbl;
extern int coredump;

LOCAL char   isymbol[SYMMAX+1];
LOCAL long   savframe;
LOCAL long   savpc;

#ifdef _NO_PROTO
LOCAL void   chkloc();
LOCAL int    convdig();
LOCAL BOOL   eqsubstr();
LOCAL BOOL   eqsym();
LOCAL BOOL   hexdigit();
LOCAL BOOL   item();
LOCAL BOOL   term();
#else
LOCAL void   chkloc(long, long);
LOCAL int    convdig(char);
LOCAL BOOL   eqsubstr(register char *, register char *, register char *);
LOCAL BOOL   eqsym(register SYMPTR, register STRING, char, int);
LOCAL BOOL   hexdigit(char);
LOCAL BOOL   item(int);
LOCAL BOOL   term(int);
#endif  /* _NO_PROTO */
LOCAL BOOL   readsym();
LOCAL BOOL   symchar();

extern int loadcnt;
extern int filndx;
extern FILEMAP * SaveMap;

SYMPTR lookupsym(symstr)
STRING symstr;
{
    SYMPTR symp;
    SYMTAB savesym, save_GL_sym;
    int i;
    int saveglflg = 0, found_auxent = 0;
    AUXENT * auxp = 0;

    for( i = 0; i < loadcnt; i++) {
    	symset(i);
    	while (symp = symget(i)) {
    	   found_auxent = 0;
	   if ( symp->n_scnum == SymTbl[i].data_scn_num ||
	        symp->n_scnum == SymTbl[i].text_scn_num )  {
		
		if ( symp->n_sclass == C_STAT || symp->n_sclass == C_EXT ||
		     symp->n_sclass == C_HIDEXT ) {
		if (eqsym(symp, symstr, '_', i)) {

		/* If there is a process, never return a symbol whose
		   x_smclas is GL. If there is no process, then
		   return the symbol whose x_smclas is GL || PR.
		*/
		   /*      if ( !pid ) return symp; */
			filndx = i;
		/* Now check to see, if the symbol is in the file or 
		   is from a different file. If the symbol is from a different
		   file, then it's class in the auxent will be XMC_GL */
			savesym.n_value = symp->n_value;
			savesym.n_numaux = symp->n_numaux;
			savesym.n_sclass = symp->n_sclass;
			savesym.n_type = symp->n_type;
			savesym.n_scnum = symp->n_scnum;
			
		  	if ( symp->n_sclass == C_EXT ) {
			    int j =  symp->n_numaux;
			    for(; j > 0 ; j--) {
			         auxp = (AUXENT *)symget(i);
				 found_auxent = 1;
		 		 SymTbl[i].symcnt--;
				 SymTbl[i].symnxt++;
			         if ( auxp->x_csect.x_smclas == XMC_PR ||
			              auxp->x_csect.x_smclas == XMC_RO ||
			              auxp->x_csect.x_smclas == XMC_RW ||
			              auxp->x_csect.x_smclas == XMC_BS  ) {
	    		                 return (&savesym);
			         } 
			         else if ( auxp->x_csect.x_smclas == XMC_GL ) {
					save_GL_sym = savesym;
					saveglflg = 1;
			    	}
			    }
			} else if ( symp->n_sclass == C_HIDEXT ) {
			    int j =  symp->n_numaux;
			    for(; j > 0 ; j--) {
			         auxp = (AUXENT *)symget(i);
				 found_auxent = 1;
		 		 SymTbl[i].symcnt--;
				 SymTbl[i].symnxt++;
			         if ( auxp->x_csect.x_smclas == XMC_TC ) {
				      	savesym.n_value = 
						get(savesym.n_value,DSP);
					if ( !pid && !coredump)
					  savesym.n_value = savesym.n_value -
						SaveMap[i].begin_data +
						SaveMap[i].data_seekadr;
	    		        	return (&savesym);
			    	}
			    }
		     }
		     else
				return symp;
		     }
		}
               }
		if ( !found_auxent )
			skip_aux(symp, i);
	    } /* while loop */
    	} /* for loop */
	    if ( saveglflg ) {
		saveglflg = 0;
		return &save_GL_sym;
	    }
    return (0);
}



/* term | term dyadic expr */
BOOL expr(a)
int a;
{
    BOOL   rc;
    long   lhs;

    (void) rdc();
    lp--;
    rc = term(a);

    while (rc) {
	lhs = expv;

	switch (readchar()) {

	case '+':
	    (void) term(a|1);
	    expv += lhs;
	    break;

	case '-':
	    (void) term(a|1);
	    expv = lhs - expv;
	    break;

	case '#':
	    (void) term(a|1);
	    expv = round(lhs, expv);
	    break;

	case '*':
	    (void) term(a|1);
	    expv *= lhs;
	    break;

	case '%':
	    (void) term(a|1);
	    expv = lhs / expv;
	    break;

	case '&':
	    (void) term(a|1);
	    expv &= lhs;
	    break;

	case '|':
	    (void) term(a|1);
	    expv |= lhs;
	    break;

	case ')':
	    if ((a&2) == 0)
		error(catgets(scmc_catd,MS_extern,E_MSG_85,BADKET));

	default:
	    lp--;
	    return (rc);
	}
    }
    return (rc);
}

extern FILEMAP * SaveMap;

/* item | monadic item | (expr) */
LOCAL BOOL term(a)
{
    switch (readchar()) {

    case '*':
	(void) term(a|1);
	expv = get(expv, DSP);
	/* The value obtained from the address expv has to be
	    relocated. If there is a process, then runtime loader takes care 
	    of this relocation.
	*/
	if ( !pid )
		expv = expv - SaveMap[0].begin_data
		   	+ SaveMap[0].data_seekadr;
	chkerr();
	return (TRUE);

    case '@':
	(void) term(a|1);
	expv = get(expv, ISP);
	chkerr();
	return (TRUE);

    case '-':
	(void) term(a|1);
	expv = -expv;
	return (TRUE);

    case '~':
	(void) term(a|1);
	expv = ~expv;
	return (TRUE);

    case '#':
	(void) term(a|1);
	expv = !expv;
	return (TRUE);

    case '(':
	(void) expr(2);
	if (*lp!=')')
	    error(catgets(scmc_catd,MS_extern,E_MSG_92,BADSYN));
	else {
	    lp++;
	    return (TRUE);
	}

    default:
	lp--;
	return (item(a));
    }
}

/* name [ . local ] | number | . | ^ | <var | <REG | 'x */
LOCAL BOOL item(a)
int a;
{
    short  base;
    SYMPTR symp;

    (void) readchar();
    if (
#ifndef aiws
	    symchar() &&
#else
	    (symchar() || lastc == '.') &&
#endif
	    readsym()) {
	if (lastc == '.') {
	    long frame  = ADBREG(_FP);
	    long callpc = ADBREG(SYSREGNO(IAR));
	    long regs[NREGS];
	    int  i;
	    long lastframe;

	    for (i = 0; i < NREGS; ++i) regs[i] = ADBREG(i);
	    while ( callpc && ( findsym(callpc, ISYM) == NOTFND ||
		    !eqsym(&symbol, isymbol, '.', filndx) ) ) {
	    again:
		lastframe = frame;
		switch (stackstatus(&frame, &callpc, regs)) {
		   case NOSTACK:
		        goto again;
		   case ENDSTACK:
		       error(catgets(scmc_catd,MS_extern,E_MSG_101,NOCFN));
		   case BADSTACK:
		       error( catgets(scmc_catd, MS_expr, E_MSG_1, "Cannot unwind stack") );
		}
	    }
	    savpc = callpc;
	    if (coredump)
	    	frame = lastframe;
	    savframe = frame;
	    (void)readchar();
	    /* Check for *, this is to allow to print all the local variables
	       of a function */ 
/*
	    if (symchar() || lastc == '*') {
*/
	    if (symchar() || lastc == '=' ) {
		if ( lastc == '=' )
		   expv = frame;
		else
		   chkloc(frame, callpc);
	    }
	} else if ((symp = lookupsym(isymbol)) == 0)
	    error(catgets(scmc_catd,MS_extern,E_MSG_91,BADSYM));
	else 
	    expv = symp->n_value;
	lp--;

    } else if (getnum(readchar))
	;
    else if (lastc == '.') {
	(void) readchar();
/*
 * .local is supposed to mean sameframe.local, but that part of the code is
 * unreachable since "." is a initial char in a symbol.
 */
	if (symchar()) {
	    if (savpc == 0 || findsym(savpc, ISYM) == NOTFND)
		error( catgets(scmc_catd, MS_expr, E_MSG_2, "no previous frame") );
	    chkloc(savframe, savpc);
	} else
	    expv = dot;
	lp--;
    } else if (lastc == '"')
	expv = ditto;
    else if (lastc == '+')
	expv = inkdot(dotinc);
    else if (lastc == '^')
	expv = inkdot(-dotinc);
    else if (lastc == '<') {
	int regno;
	char savc = rdc();
	if ((regno = getreg(savc)) != NOTREG)
	    expv = ADBREG(regno);
	else if ((base = varchk(savc)) != -1)
	    expv = var[base];
	else
	    error(catgets(scmc_catd,MS_extern,E_MSG_94,BADVAR));
    } else if (lastc == '\'') {
	int d = 4;
	expv = 0;
	while (quotchar() != '\0') {
	    if (--d < 0) error(catgets(scmc_catd,MS_extern,E_MSG_92,BADSYN));
	    expv = expv << 8 | lastc;
	}
    } else if (a)
	error(catgets(scmc_catd,MS_extern,E_MSG_99,NOADR));
    else {
	lp--;
	return (FALSE);
    }
    return (TRUE);
}

/* Service routines for expression reading */
BOOL getnum(rdf)
char (*rdf)();
{
    int  base;
    int  d;
    BOOL hex = FALSE;

    if (isdigit(lastc) || (hex = TRUE, lastc == '#' && hexdigit((*rdf)()))) {
	expv = 0;
	base = (hex ? 16 : radix);
	while ((base > 10 ? hexdigit(lastc) : isdigit(lastc))) {
	    expv *= base;
	    if ((d = convdig(lastc)) >= base)
		error(catgets(scmc_catd,MS_extern,E_MSG_92,BADSYN));
	    expv += d;
	    (*rdf)();
	    if (expv == 0) {
		if ((lastc == 'x' || lastc == 'X')) {
		    hex = TRUE;
		    base = 16;
		    (*rdf)();
		} else if ((lastc == 't' || lastc=='T')) {
		    hex = FALSE;
		    base=10;
		    (*rdf)();
		} else if ((lastc == 'o' || lastc=='O')) {
		    hex = FALSE;
		    base = 8;
		    (*rdf)();
		}
	    }
	}
#if FLOAT
	if (lastc == '.' && (base == 10 || expv == 0) && !hex) {
	    int  frpt;
	    union {
		float r;
		int   i;
	    } real;
	    real.r = expv;
	    frpt = 0;
	    base = 10;
	    while (isdigit((*rdf)())) {
		real.r *= base;
		frpt++;
		real.r += lastc - '0';
	    }
	    while (frpt--)
		real.r /= base;
	    expv = real.i;
	}
#endif
	peekc = lastc;
	/*  lp--; */
	return (TRUE);
    } else
	return (FALSE);
}

LOCAL BOOL readsym()
{
    char *p = isymbol;

    do {
	if (p < &isymbol[SYMMAX])
	    *p++ = lastc;
	(void) readchar();
    } while (isdigit(lastc) || symchar());
    *p++ = 0;
#ifdef aiws
    if (strcmp(isymbol, ".") == 0) {
	peekc = lastc;
	lastc = '.';
	return (FALSE);
    }
#endif
    return (TRUE);
}

#ifdef _NO_PROTO
LOCAL BOOL hexdigit(c)
char c;
#else
LOCAL BOOL hexdigit(char c)
#endif
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
}

#ifndef _NO_PROTO
LOCAL int convdig(char c)
#else
LOCAL int convdig(c)
char c;
#endif
{
    if (isdigit(c))
	return (c - '0');
    else if (hexdigit(c))
	return (c - 'a' + 10);
    else
	return (17);
}

/*
 * TRUE if last character read can begin a normal symbol.  Caller must test
 * '.' (for beginning of internal function-name) and/or digits (continuation
 * of symbol).
 */
LOCAL BOOL symchar()
{
    if (lastc == '\\') {
	(void)readchar();
	return (TRUE);
    }
    return (isalpha(lastc) || lastc == '_' );
}

int varchk(name)
{
    if (isdigit(name))
	return (name - '0');
    if (isalpha(name))
	return ((name & 037) - 1 + 10);
    return (-1);
}

LOCAL void chkloc(frame, pc)
long frame, pc;
{
/*
    if ( lastc == '*') {
	   if (findsym(pc, ISYM) != NOTFND)
	      while (localsym(frame, pc)) {
		  long word = get(localval, DSP);
		  adbpr("%8t%s:%10t", flexstr(&symbol, filndx));
		  if (errflg != NULL) {
		      prints("?\n");
		      errflg = NULL;
		  } else 
		      adbpr("%R\n", word);
	      }
    }
    else {
*/
        (void)readsym();
    	do {
	    if (!localsym(frame,pc))
	    	error(catgets(scmc_catd,MS_extern,E_MSG_86,BADLOC));
        } while (!eqsym(&symbol, isymbol, '_',filndx));
        expv = localval;
  /* } */
}

LOCAL BOOL eqsubstr(s1, se, s2)
register char *s1, *se, *s2;
{
    while (s1 < se && *s1 != '\0') {
	if (*s1++ != *s2++) return (FALSE);
    }
    return (*s2 == '\0');
}

/*
 * Test if s2 is a valid name for symbol in symp.  Third arg specifies an
 * initial character which may be elided.  (Note: unlike standard SysV adb,
 * the first arg is a symbol, not a string.)
 */
extern SYMINFO * SymTbl;

#ifdef _NO_PROTO
LOCAL BOOL eqsym(symp, s2, c, fileindex)
register SYMPTR symp;
register STRING s2;
char c;
int fileindex;
#else
LOCAL BOOL eqsym(register SYMPTR symp, 
		 register STRING s2, 
		 char  c, int fileindex)
#endif
{
    register char *s1;
    LOCAL char * colmndx = 0, * stabaddr = 0;
    if (symp->n_zeroes) {
	char * dbgtab;
        if (symp->n_sclass & DBXMASK) {
		int len;
		dbgtab = symp->n_name;
		colmndx = strchr(stabaddr = dbgtab,':');
		if (colmndx)
			len = colmndx - stabaddr;
	  	s1 = (char *)malloc(len+1);
		(void)strncpy(s1, stabaddr, len);
		s1[len+1] = '\0';
      } else {
	s1 = symp->n_name;
      }
	return (eqsubstr(s1, s1+SYMNMLEN, s2) ||
		*s1 == c && eqsubstr(s1+1, s1+SYMNMLEN, s2));
    } else {
	char * strtab;
	char * dbgtab;

        if (symp->n_sclass & DBXMASK) {
		int len;
		dbgtab = SymTbl[fileindex].dbgtab;
		colmndx = strchr(stabaddr = &dbgtab[symp->n_offset],':');
		if (colmndx)
			len = colmndx - stabaddr;
	  	s1 = (char *)malloc(len+1);
		(void)strncpy(s1, stabaddr, len);
		s1[len+1] = '\0';
      } else {
	strtab = SymTbl[fileindex].strtab;
	s1 = &strtab[symp->n_offset - 4];
      }
	return (strcmp(s1, s2) == 0 ||
		*s1 == c && strcmp(s1+1, s2) == 0);
    }
}
