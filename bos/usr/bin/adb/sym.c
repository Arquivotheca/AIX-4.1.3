static char sccsid[] = "@(#)17	1.16  src/bos/usr/bin/adb/sym.c, cmdadb, bos411, 9428A410j 4/22/94 10:41:10";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: flexstr, findsym, getfilendx, localsym, nextsym, psymoff, 
 *	      skip_aux, symset, symget, valpr, XXsym, XXsyment, 
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
/*
 *  Symbol table and file handling service routines
 */

#include "defs.h"

extern SYMINFO   * SymTbl;
extern FILEMAP   * FileMap, *SaveMap;
extern int coredump;

LOCAL BOOL    symrqd = TRUE;
LOCAL SYMTAB  symbuf[1];     /* Room for 1 symbol at a time (alignment) */

#ifdef _NO_PROTO
LOCAL BOOL nextsym();
LOCAL BOOL symread();
#else
LOCAL BOOL nextsym(int);
LOCAL BOOL symread(int);
#endif

#ifdef CROSSDB
LOCAL void XXsym();
LOCAL void XXsyment();
#endif

int      filndx; /*  findsym set's filndx to the proper value */

void valpr(v, idsp)
long v;
int idsp;
{
    unsigned long d = findsym(v, idsp);

    if (d < maxoff && filndx != -1 ) {
	adbpr("%s", flexstr(&symbol, filndx));
	if (d)
	    adbpr(OFFMODE, d);
    }
}

BOOL localsym(cframe, pc)
long cframe, pc;
{
    int index = -1;
    if ( (index = getfilendx((unsigned long)pc) ) == -1 ) {
	return FALSE;
    }
	
    while (nextsym(index) && strcmp(symbol.n_name, ".ef") != 0) {
	switch (symbol.n_sclass) {
	case C_AUTO:
	case C_LSYM:
	case C_PSYM:
	case C_ARG:
	    localval = cframe + symbol.n_value;
	    return (TRUE);
	case C_STSYM:
	case C_STAT:
	    if ( pid || coredump )
	    localval = ( symbol.n_value - FileMap[index].begin_data) 
				+ loader_info[index].dataorg;
	    else
	    	localval = ( symbol.n_value - FileMap[index].begin_data) 
				 + FileMap[index].data_seekadr;
	    return (TRUE);
	}
    }
    return (FALSE);
}

/* Prints symbol, offset, and suffix */
void psymoff(v, symtype, s)
long  v;
int   symtype;
char  *s;
{
    unsigned long w;
/* commented out to fix d#143743
    if (v != 0 && (w = findsym(v, symtype)) < maxoff && filndx != -1) {
*/
    w = findsym(v, symtype);
    if (v != 0 && filndx != -1) {
	adbpr("%s", flexstr(&symbol, filndx));
	if (w != 0)
	    adbpr(OFFMODE, w);
    } else {
	adbpr(LPRMODE, v);
    }
    adbpr(s);
}

/*
 *  Linear search through slave array; finds the symbol closest
 *  above value
 */
unsigned findsym(value, symtype)
long value;
int symtype;
{
    long      diff;
    long      symval;
    long      offset;
    int      slvtype;
    SYMSLAVE *symptr;
    SYMSLAVE *symsav;

    if ( (filndx = getfilendx((unsigned long)value)) == -1 ) {
	return NOTFND;
    }

    diff = NOTFND; /* A large number */
    symsav = 0;
    if (symtype != NSYM && (symptr = SymTbl[filndx].symvec)) {
	while (diff && (slvtype = symptr->typslave) != ENDSYMS) {
	    if (((slvtype == symtype) || ((symtype == IDSYM) &&
		((slvtype == DSYM) || (slvtype == ISYM) ))) &&
		(slvtype != EXTRASYM)) {
		symval = symptr->valslave;
		if (value - symval < diff && value >= symval) {
		    diff = value - symval;
		    symsav = symptr;
		}
	    }
	    symptr++;
	}
	if (symsav) {      /* Best entry */
	    offset = symsav - SymTbl[filndx].symvec;
	    SymTbl[filndx].symcnt = SymTbl[filndx].symnum - offset;
	    if (-1L == lseek(FileMap[filndx].fd, 
		(long) (FileMap[filndx].st_seekadr + offset * SYMBOLSIZE), 0))
		perror( "findsym" );
	    if (-1 == read(FileMap[filndx].fd, (char *) &symbol, SYMBOLSIZE))
		perror( "findsym" );
#ifdef CROSSDB
	    XXsyment(&symbol);
#endif
	}
    }
    return (diff);
}

LOCAL BOOL nextsym(i)
int i;
{
    if (--(SymTbl[i].symcnt) < 0 || -1 == lseek(FileMap[i].fd,
	    (long)(FileMap[i].st_seekadr + 
		(SymTbl[i].symnum - SymTbl[i].symcnt) * SYMBOLSIZE), 0))
	return (FALSE);
    return (SYMBOLSIZE == read(FileMap[i].fd, (char *) &symbol, SYMBOLSIZE));
}

/* sequential search through file */
void symset(i)
int i;
{
    SymTbl[i].symcnt = SymTbl[i].symnum;
    SymTbl[i].symnxt = symbuf;
    SymTbl[i].symend = symbuf;

    if (symrqd) {
	if (-1L == lseek(FileMap[i].fd, (long) FileMap[i].st_seekadr, 0))
	    perror( "symset" );
	/* (void)symread(i); */
	symrqd = FALSE;
    } else
    /* changed for single-element accessing */
	if (-1L == lseek(FileMap[i].fd, 
			(long) (FileMap[i].st_seekadr+SYMBOLSIZE), 0))
	    perror( "symset" );
}

SYMPTR symget(i)
int i;
{
    BOOL rc;
    int sym_section = -1;

    if (SymTbl[i].symnxt >= SymTbl[i].symend) {
	rc = symread(i);
	if ((sym_section = symbuf[0].n_scnum) == SymTbl[i].text_scn_num) {
		if ( pid || coredump )
	        symbuf[0].n_value += 
			loader_info[i].textorg + 
			FileMap[i].txt_scnptr ;
	}
	else if ((sym_section == SymTbl[i].data_scn_num) || 
				(sym_section == SymTbl[i].bss_scn_num)) {
		if ( pid || coredump )
	    		symbuf[0].n_value = 
			(symbuf[0].n_value - SaveMap[i].begin_data) 
				  + loader_info[i].dataorg;
		else {
	    		symbuf[0].n_value = 
				(symbuf[0].n_value - SaveMap[i].begin_data) 
				  + SaveMap[i].data_seekadr;
		}
	     }
	symrqd = TRUE;
    } else 
	rc = TRUE;
    if (--(SymTbl[i].symcnt) > 0 && !rc) {
	errflg = catgets(scmc_catd,MS_extern,E_MSG_84,BADFIL);
    }
    if  (SymTbl[i].symcnt >= 0 && rc) {
	  SymTbl[i].symend = SymTbl[i].symnxt;
	  return (SymTbl[i].symnxt++) ;
    }
    else
  	return ( (SYMPTR) NULL);
	  

  /* return ((SymTbl[i].symcnt >= 0 && rc) ? 
					SymTbl[i].symnxt++ : (SYMPTR)NULL);
*/
}

/* The original algorithm fails for a single element buffer */
void skip_aux(p,i)
SYMPTR p;
int i;
{
    register int numaux = p->n_numaux;
    for (;numaux > 0;numaux--) {
	(void)symread(i);
        SymTbl[i].symcnt--; 
    	SymTbl[i].symnxt++;
    }
    symrqd = TRUE;
}

LOCAL BOOL symread(i)
int i;
{
    int symlen;

/* Have to read one at a time for now */
    if (-1 == (symlen = read(FileMap[i].fd, (char *) symbuf, SYMBOLSIZE)))
	perror( "symread" );
    if (symlen >= SYMBOLSIZE) {  /* Has at least one entry been read? */
	SymTbl[i].symnxt = symbuf;
	SymTbl[i].symend = symbuf + 1;
#ifdef CROSSDB
	XXsym();
#endif
	return (TRUE);
    } else
	return (FALSE);
}

#ifdef CROSSDB
LOCAL void XXsym()
{
    register struct syment *s = symbuf;
    register struct syment *se = symend;

    for (; s < se; s++)
	XXsyment(s);
}

/* Byte-swap a symbol entry */
LOCAL void XXsyment(sym)
register struct syment *sym;
{
    sym->n_value = XL(sym->n_value);
    sym->n_type = (unsigned short)XH(sym->n_type);
    if (sym->n_zeroes==0) sym->n_offset = XL(sym->n_offset);
}
#endif

extern char *strchr();
#define MAXFLEXNM 256
LOCAL char flexbuf[MAXFLEXNM+1];
char *flexstr(symp,tabndx)
register SYMPTR symp;
int tabndx;
{
    LOCAL char *colonndx = 0, *stabaddr = 0;
    LOCAL char *dbgtab = 0, *strtab = 0;
    LOCAL int len = MAXFLEXNM;

    if (symp->n_zeroes) {
	char * dbgtab;
        if (symp->n_sclass & DBXMASK) {
		int len;
		dbgtab = symp->n_name;
		colonndx = strchr(stabaddr = dbgtab,':');
		if (colonndx)
			len = colonndx - stabaddr;
		(void)strncpy(flexbuf, stabaddr, len);
		flexbuf[len] = '\0';
         } else {
	    	(void)strncpy(flexbuf, symp->n_name, 8);
		flexbuf[8] = 0;
         }
    } else if (symp->n_sclass & DBXMASK) {
	dbgtab = SymTbl[tabndx].dbgtab;
	colonndx = strchr(stabaddr = &dbgtab[symp->n_offset],':');
	if (colonndx)
		len = colonndx - stabaddr;
	(void)strncpy(flexbuf, stabaddr, len);
	flexbuf[len] = '\0';
      } else {
	strtab = SymTbl[tabndx].strtab;
	if ( strtab )
		(void)strncpy(flexbuf, &strtab[symp->n_offset - 4], MAXFLEXNM);
	else {
		(void)strncpy(flexbuf, "", 8);
		flexbuf[8] = 0;
	}
    }
    return (flexbuf);
}

/*  getfilendx finds a particular file associated with the address
    of any symbol. A value of -1 is returned if a file is not found.
 */
extern int loadcnt;
getfilendx(addr)
unsigned long addr;
{
	int i;
	unsigned long tmp_addr = addr;;

	for(i = 0; i < loadcnt; i++) {
	   if  ( ( tmp_addr >= FileMap[i].begin_txt) &&
	          ( tmp_addr <  FileMap[i].end_text) ) 
		   return i;
	   else {
	    if ( !pid )
           	tmp_addr = addr - SaveMap[i].data_seekadr + 
					 SaveMap[i].begin_data;

	        if ( ( tmp_addr >= FileMap[i].begin_data ) &&
	          ( tmp_addr <  FileMap[i].end_data ) ) 
		return i;
	   }
	}
	return -1;
}
