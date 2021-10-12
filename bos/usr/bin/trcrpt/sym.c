static char sccsid[] = "@(#)32        1.24  src/bos/usr/bin/trcrpt/sym.c, cmdtrace, bos412, 9445C412a 11/3/94 02:33:44";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: reslookup, ressyminit, lookup, install, symclear, symclearl
 *
 * ORIGINS: 27, 83
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
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 5, 5 Years Bull Confidential Information
 *  
 */

/*
 * symbol table routines.
 *
 * NAME:     reslookup
 * FUNCTION: lookup 'name' in the 'reserved[]' symbol table.
 * INPUT:    name    symbol to match with reserved[].s_name
 * RETURNS:  pointer to the symbol. 0 if not found.
 *
 * This lookup is through a simple hash search. The first character
 * is the index into a hash table, which is the head of a linked list
 * of symbol table entries. The routine ressyminit is called by main
 * to initialize this table from the the reserved[] array.
 * Note: see streq_n() for a description of the name matching criteria.
 *
 * NAME:     resinstall
 * FUNCTION: install a symbol as a reserved word
 * INPUT:    ressp   symbol to install (from reserved array)
 * RETURNS:  none
 *
 * This routine is called successively be ressyminit.
 *
 *
 * NAME:     install
 * FUNCTION: install a register (macro)
 * INPUT:    name    name of the register to install
 * RETURNS:  pointer to installed register.
 *
 * This routine is called when the construct:
 * {{ $rrr = X4 }}
 * in encountered in the template.
 *
 *
 * NAME:     lookup
 * FUNCTION: install a named register
 * INPUT:    name    name of the register to lookup
 * RETURNS:  pointer to register.
 *           (The value is not really used except to determine if it
 *           should be installed).
 */

#include <stdio.h>
#include "rpt.h"
#include "sym.h"
#include "parse.h"

extern char *stracpy();

/*
 * reserved words
 * snext (set to 0)
 * name of format string as it appears in the template file
 * type (token) as assigned by parse.y (usually IFORMAT)
 * internal code for IFORMATs
 * secondary internal code for IFORMATs, for fixed length and 'H'
 * (max) number of width fields
 */

#define S(NAME,type,fmtcode,fld1,xcode) \
	{ NULL, type, "NAME", fmtcode, fld1, xcode }

static struct symbol reserved[] = {
	S(A@,           IFORMAT,  'A',  0,                2 ),
	S(B@,           IFORMAT,  'B',  0,                2 ),
	S(W@,           IFORMAT,  'W',  0,                2 ),
	S(G@,           IFORMAT,  'G',  0,                2 ),
	S(O@,           IFORMAT,  'O',  0,                2 ),
	S(R@,           IFORMAT,  'R',  0,                1 ),
	S(X@,           IFORMAT,  'X',  0,                1 ),
	S(o1,           IFORMAT,  'o',  1,                1 ),
	S(o2,           IFORMAT,  'o',  2,                1 ),
	S(o4,           IFORMAT,  'o',  4,                1 ),
	S(S4,           IFORMAT,  'S',  4,                1 ),
	S(S2,           IFORMAT,  'S',  2,                1 ),
	S(S1,           IFORMAT,  'S',  1,                1 ),
	S(D4,           IFORMAT,  'D',  4,                1 ),
	S(D2,           IFORMAT,  'D',  2,                1 ),
	S(D1,           IFORMAT,  'D',  1,                1 ),
	S(E4,           IFORMAT,  'E',  4,                1 ),
	S(E2,           IFORMAT,  'E',  2,                1 ),
	S(E1,           IFORMAT,  'E',  1,                1 ),
	S(F4,           IFORMAT,  'F',  4,                1 ),
	S(F8,           IFORMAT,  'F',  8,                1 ),
	S(P4,           IFORMAT,  'P',  0,                1 ),
	S(T4,           IFORMAT,  'T',  0,                1 ),
	S(U4,           IFORMAT,  'U',  4,                1 ),
	S(U2,           IFORMAT,  'U',  2,                1 ),
	S(U1,           IFORMAT,  'U',  1,                1 ),
	S(HT,           IFORMAT,  'H', 'T',               0 ),
	S(HD,           IFORMAT,  'H', 'W',               0 ),
	S(HB,           IFORMAT,  'H', 'B',               0 ),
	S(BITFLAGS,     IBITFLAGS, 0 ,  0 ,               0 ),
	S(LOOP,         ILOOP,     0 ,  0 ,               0 ),
	S(L=APPL,       ILEVEL,   'L',  IND_APPL,         0 ),
	S(L=KERN,       ILEVEL,   'L',  IND_KERN,         0 ),
	S(L=SVC,        ILEVEL,   'L',  IND_SVC,          0 ),
	S(L=INT,        ILEVEL,   'L',  IND_INT,          0 ),
	S(L=0,          ILEVEL,   'L',  IND_0 ,           0 ),
	S(L=NOPRINT,    ILEVEL,   'L',  IND_NOPRINT,      0 ),
	S($ERROR,       IFORMAT,  'i',  RES_ERROR,        0 ),
	S($LOGIDX0,     IFORMAT,  'i',  RES_INDEX0,       0 ),
	S($LOGIDX,      IFORMAT,  'i',  RES_INDEX,        0 ),
	S($LOGFILE,     IFORMAT,  'i',  RES_LOGFILE,      0 ),
	S($TRACEID,     IFORMAT,  'i',  RES_TRACEID,      0 ),
	S($DEFAULT,     IFORMAT,  'i',  RES_DEFAULT,      0 ),
	S($STOP,        IFORMAT,  'i',  RES_STOP,         0 ),
	S($BREAK,       IFORMAT,  'i',  RES_BREAK,        0 ),
	S($SKIP,        IFORMAT,  'i',  RES_SKIP,         0 ),
	S($NOPRINT,     IFORMAT,  'i',  RES_NOPRINT,      0 ),
	S($BASEPOINTER, IREG,     'r',  RREG_BASEPOINTER, 0 ),
	S($DATAPOINTER, IREG,     'r',  RREG_DATAPOINTER, 0 ),
	S($EXECPATH,    IREG,     'r',  RREG_EXECPATH,    0 ),
	S($IPADDR,      IREG,     'r',  RREG_INADDR,      0 ),
	S($PID,         IREG,     'r',  RREG_PID,         0 ),
	S($TID,         IREG,     'r',  RREG_TID,         0 ),
	S($CPUID,       IREG,     'r',  RREG_CPUID,       0 ),
	S($PRI,         IREG,     'r',  RREG_PRI,         0 ),
	S($TIME,        IREG,     'r',  RREG_TIME,        0 ),
	S($SYMBOLRANGE, IREG,     'r',  RREG_SYMBOL_RANGE,0 ),
	S($SYMBOLVALUE, IREG,     'r',  RREG_SYMBOL_VALUE,0 ),
	S($SYMBOLNAME,  IREG,     'r',  RREG_SYMBOL_NAME, 0 ),
	S($HT,          IREG,     'r',  RREG_HTYPE,       0 ),
	S($HDU,         IREG,     'r',  RREG_HDATAU,      0 ),
	S($HDL,         IREG,     'r',  RREG_HDATAL,      0 ),
	S($HD,          IREG,     'r',  RREG_HDATA,       0 ),
	S($D1,          IREG,     'r',  RREG_DATA1,       1 ),
	S($D2,          IREG,     'r',  RREG_DATA2,       2 ),
	S($D3,          IREG,     'r',  RREG_DATA3,       3 ),
	S($D4,          IREG,     'r',  RREG_DATA4,       4 ),
	S($D5,          IREG,     'r',  RREG_DATA5,       5 ),
	S($SVC,         IREG,     'r',  RREG_SVC,         0 ),
	S($RELLINENO,   IREG,     'r',  RREG_RELLINENO,   0 ),
	S($LINENO,      IREG,     'r',  RREG_LINENO,      0 ),
	S($CURRFILE,    IREG,     'r',  RREG_CURRFILE,    0 ),
};
#define NRESERVED sizeof(reserved)/sizeof(reserved[0])

#define NHASH 32
#define HASHIDX(name) (name[0] % NHASH)

static symbol Ressymtab[NRESERVED];
static int Nreserved;
static symbol *reshashtab[NHASH];

extern Funclevel;
#define NFUNCLEVELS 3
static symbol Symtab[NFUNCLEVELS][NREGISTERS];
static int    Nsyms[NFUNCLEVELS];

/*
 * return the symbol that matches:
 * s_name   == 'name',
 */
symbol *reslookup(name)
char *name;
{
	symbol *sp;
	int i;
	int idx;

	idx = HASHIDX(name);
	Debug2("reslookup(%s) hashidx=%d\n",name,idx);
	sp = reshashtab[idx];
	while(sp) {
		if(streq_n(sp->s_name,name))
			return(sp);
		sp = sp->s_next;
	}
	return(NULL);
}

static symbol *resinstall(ressp)
symbol *ressp;
{
	symbol *sp;
	int idx;

	sp = &Ressymtab[Nreserved++];
	*sp = *ressp;
	idx = HASHIDX(sp->s_name);
	sp->s_next = reshashtab[idx];
	reshashtab[idx] = sp;
	Debug2("resinstall(%s,%d) hashidx=%d\n",sp->s_name,sp->s_type,idx);
	return(sp);
}

/*
 * NAME:     streq_n
 * FUNCTION: determine whether the name matches the installed symbol.
 * INPUTS:   instsym    installed reserved word to compare against
 *           s2         name to lookup
 * RETURNS:  0     not equal
 *           non-0 otherwise
 *           Digits matching '@' (see below) are placed in
 *           Numbuf1[] and Numbuf2[] for use by yylex to fill in the
 *           widths of format codes.
 *
 * If the 2 names are the same, they match.
 * If the instsym contains a '@', then this matches
 *   digit                or
 *   digit '.' digit
 *
 * For example:
 * B4   will match B@
 * B4.2 will match B@
 * B4.  will not match B@
 *
 * Note:
 *  This routine does not have special NLS code.
 *  However, it works with NLS input because the 'instsym' is all ascii
 *    and only equality to 'instsym' and digits and '.' is tested.
 */

static streq_n(instsym,s2)
register char *instsym,*s2;
{
	register char c,ic;
	register state;
	int idx;

	state = 0;
	Numbuf1[0] = '\0';
	Numbuf2[0] = '\0';
	for(;;) {
		switch(state) {
		case 0:				/* look for '@' */
			ic = *instsym++;
			c = *s2++;
			if(ic == '\0' && c == '\0')
				return(1);
			if(ic == '@') {
				idx = 0;
				state = 1;
				s2--;
				continue;
			}
			if(ic != c)
				return(0);
			continue;
		case 1:				/* look for digits */
			c = *s2++;
			if('0' <= c && c <= '9') {
				s2--;
				state = 2;
				continue;
			}
			if(c == 0) {
				Numbuf1[idx] = '\0';
				return(idx ? 1 : 0);
			}
			return(0);
		case 2:				/* look for '.' */
			c = *s2++;
			if('0' <= c && c <= '9') {
				Numbuf1[idx++] = c;
				continue;
			}
			if(c == '.') {
				Numbuf1[idx] = '\0';
				idx = 0;
				state = 3;
				continue;
			}
			if(c == 0) {
				Numbuf1[idx] = '\0';
				return(1);
			}
			return(0);
		case 3:				/* look for end of string or non-digit */
			c = *s2++;
			if('0' <= c && c <= '9') {
				Numbuf2[idx++] = c;
				continue;
			}
			if(c == 0) {
				Numbuf2[idx] = '\0';
				return(1);
			}
			return(0);
		}
	}
}

ressyminit()
{
	int i;

	for(i = 0; i < NRESERVED; i++)
		resinstall(&reserved[i]);
	builtin_init();
}

/*
 * return the symbol that has s_name == 'name'.
 */
symbol *lookup(name)
char *name;
{
	symbol *sp;
	int n;
	int i;

	Debug2("lookup(%s)\n",name);
	n = Nsyms[Funclevel];
	for(i = 0, sp = Symtab[Funclevel]; i < n; i++, sp++)
		if(STREQ(sp->s_name,name))
			return(sp);
	return(0);
}

symbol *install(name)
char *name;
{
	symbol *sp;

	if(Funclevel > NFUNCLEVELS) {
		Debug("Too many nested functions: %d",Funclevel);
		genexit(1);
	}
	if(Nsyms[Funclevel] == NREGISTERS)
		cat_fatal(CAT_TPT_SYMBOLS,
"The register %s exceeds the maximum count of %d.\n",
			name,NFUNCLEVELS);
	sp = &Symtab[Funclevel][Nsyms[Funclevel]];
	sp->s_type = IREG;
	sp->s_fmtcode  = '$';
	sp->s_fld1 = Nsyms[Funclevel];
	sp->s_name = stracpy(name);
	Nsyms[Funclevel]++;
	Debug2("install(%s,%d) number=%d\n",
		sp->s_name,sp->s_type,sp->s_fld1);
	return(sp);
}

symclear()
{

	memset(Nsyms,0,sizeof(Nsyms));
}

symclearl(level)
{

	Nsyms[level] = 0;
}

