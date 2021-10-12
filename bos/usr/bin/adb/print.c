static char sccsid[] = "@(#)%M  1.17.1.5  src/bos/usr/bin/adb/print.c, cmdadb, bos41B, 9504A  12/14/94  18:07:53";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: dollar, getreg, printmap, printreg, printpc, sigprint
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
/*
 *  General printing routines
 */

#include "defs.h"
/*
 * Include files for hardware platform support. 
 */
#include "ops.h"
#include "disassembly.h"
#include <sys/systemcfg.h>

unsigned int instruction_set = DEFAULT; /* Used to select instruction set */
unsigned int mnemonic_set = DEFAULT;    /* Used to select mnemonic set */
 
LOCAL void printflregs();
LOCAL void printregs();
void printmap();
#ifdef _NO_PROTO
LOCAL BOOL Valid_Reg();
#else
LOCAL BOOL Valid_Reg(int);
#endif

extern int loadcnt;
extern int filndx;
extern int kmem_core;
extern int coredump;
extern SYMINFO * SymTbl;
extern double fpregs[];
extern char dir_name[];


#if KADB
extern int rdfd;
#endif
LOCAL STRING signals[NSIG] = {
    "hangup",
    "interrupt",
    "quit",
    "illegal instruction",
    "trace/BPT",
    "IOT",
    "EMT",
    "floating exception",
    "killed",
    "bus error",
    "memory fault",
    "bad system call",
    "broken pipe",
    "alarm call",
    "terminated",
    "first user defined signal",
    "second user defined signal",
    "death of a child",
    "power fail",
};

/* Function dollar: It parses the dollar($) command of the adb. */

void dollar(modif)
int modif;
{
    short  i;
    BKPTR  bkptr;
    STRING comptr;
    SYMPTR symp;
    char   inputstr[8];

    if ( !cntflg )
	cntval = -1;

    switch (modif) {
    case '<':
    case '>':
    case 'I':
	{
	    char userfile[128];
	    char Dirname[128];
	    char *dirname;
	    int  index;

	    if ( !cntval ) 
		break;
	    else
	        var[9] = cntval;
	    index = 0;
	    if (modif == '<' || modif == 'I')
		iclose();
	    else
		oclose();
	    if (rdc() != '\n') {
		do {
		    userfile[index++] = lastc;
		    if (index >= sizeof(userfile))
			error(catgets(scmc_catd,MS_extern,E_MSG_98,LONGFIL));
		} while (readchar() != '\n');
		userfile[index] = 0;
	        if ( modif != 'I' ) {
		      strcpy(Dirname, dir_name);
		      dirname = (char *) strcat(Dirname, "/");
		      dirname = (char *) strcat(Dirname, userfile);
		}
	 	else  if ( chkdir(userfile) )
		       strcpy(dir_name, userfile);
		if ( modif != 'I' ) {
		   if (modif == '<') {
		       infile = open(dirname, 0);
		       if (infile < 0) {
			   infile = 0;
			   error(catgets(scmc_catd,MS_extern,E_MSG_106,NOTOPEN));
		       }
		   } else {
		       outfile = open(dirname, 1);
		       if (outfile < 0 && 
				(outfile = creat(dirname, 0666)) < 0) {
			   outfile = 1;
			   error(catgets(scmc_catd,MS_extern,E_MSG_106,NOTOPEN));
		       }
		       (void)lseek(outfile, 0L, 2);
		   }
		}
	    }
	    lp--;
	}
	break;

    case 'd':
	if (adrflg) {
	    if (adrval < 2 || adrval > 16)
		error(catgets(scmc_catd,MS_extern,E_MSG_90,BADRAD));
	    radix = adrval;
	} else
	    radix = DEFRADIX;
	break;

    case 'o':
	radix = 8;
	break;

    case 'q':
    case 'Q':
    case '%':
	done();

    case 'w':
    case 'W':
	maxpos = (adrflg ? adrval : MAXPOS);
	break;

    case 's':
    case 'S':
	maxoff = (adrflg ? adrval : MAXOFF);
	break;

    case 'v':
    case 'V':
	prints("variables\n");
	for (i = 0; i < VARNO; i++)
	    if (var[i]) {
		printc((i <= 9 ? '0' : 'a' - 10) + i);
		adbpr(" = %R\n",var[i]);
	    }
	break;

    case 'm':
    case 'M':
	printloaderinfo();  
	break;

    case   0:
    case '?':
	if (pid)
	    adbpr("pcs id = %d\n", pid);
	else
	    prints( catgets(scmc_catd, MS_print, E_MSG_22, "no process\n") );
	sigprint();
	flushbuf();
	printregs(-1, -1);
	flushbuf();
	break;

#if KADB
    case 'R':
	if (rdfd)
	    rdrregs((char *)NULL);
    case 'r':
	printregs();
	return;
#else
    case 'r':
    case 'R':
	if ( !adrflg && !cntflg )
	   printregs(-1, -1);
	else if ( adrflg && !cntflg && Valid_Reg(dot) )
	   printregs(dot, -1);
	else if ( !adrflg && cntflg && Valid_Reg(expv) )
	   printregs(expv-1, 0);
	else if ( Valid_Reg(expv) && Valid_Reg(dot) ) {
		  if ( ( expv+dot ) &&  Valid_Reg(expv+dot-1) )
		     printregs(expv+dot-1, dot);
		  else 
		     adbpr("Bad register range: from %d to %d\n", dot, expv);
	}
	
	return;
#endif

    case 'c':
    case 'C':
	stktrace(modif);
	break;

    case 'e':
    case 'E':            /* Print externals */
  	{
	  int i = 0;
	  int init = 0;
	  int final = loadcnt;
	  if ( cntflg ) {
		init = expv;
		final = expv + 1;
	  }

	  for (i=init; i < final; i++) {
	     	SYMSLAVE * symvec = SymTbl[i].symvec;
		int numaux = 0;

		symset(i);
		while (symp = symget(i)) {
	    	chkerr();
	    	if ((symp->n_scnum == SymTbl[i].data_scn_num) ||
		    	(symp->n_scnum == SymTbl[i].bss_scn_num) ) {
		    if ( pid )
			adbpr("%s:%12t%R\n", flexstr(symp, i), 
						get(symp->n_value, DSP));
		    else 
			adbpr("%s:%12t%R\n", flexstr(symp, i), 
						symvec->valslave );
		}	
		symvec++;
		for (numaux = symp->n_numaux; --numaux >= 0;)
			symvec++;
	    	skip_aux(symp, i);
		}
	  }
	}
	break;

    case 'b':           /* print breakpoints */
    case 'B':           /* set default c frame */
#if KADB
	if (rdfd) {
	    rdstops();
	    break;
	}
#endif
	adbpr("breakpoints\ncount%8tbkpt%24tcommand\n");
	for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) {
	    if (bkptr->flag) {
		adbpr("%-8.8d", bkptr->count);
		psymoff(bkptr->loc, ISYM, "%24t");
		comptr = bkptr->comm;
		while (*comptr)
		    printc(*comptr++);
	    }
	}
	break;

#if DEBUG
    case '=':
	traceflag = (adrflg ? adrval : ~(unsigned)0);
	break;
#endif

    case 'p':
	{
	    if ( kmem_core )
		adbpr("function not implemented, use crash(1) instead\n");
	    else
		adbpr("No kernel debugging\n");
	    break;
	}
    case 'P':   /* To change the default adb prompt */
	{
	    register char *p = promptstr;
	    (void)rdc();
	    --lp;
	    while (p < promptstr+MAXPROMPT-1 && readchar() != '\n')
		*p++ = lastc;
	    *p = '\0';
	    --lp;
	}
	break;
    case 'i':   /* To change the default instruction set */
 {
     register char *p = inputstr;
     (void)rdc();
     --lp;
     while (p < inputstr+8 && readchar() != '\n')
  *p++ = lastc;
     *p = '\0';
     --lp;
     if (strlen(inputstr) == 0) { /* no parameter */
  char outputstr[10];
  switch (instruction_set) {
     case PWR: strcpy(outputstr, "pwr");
        break;
     case PWRX: strcpy(outputstr, "pwrx");
        break;
     case PPC: strcpy(outputstr, "ppc");
        break;
     case DEFAULT: strcpy(outputstr, "default");
        break;
     case SET_601: strcpy(outputstr, "601");
        break;
     case SET_603: strcpy(outputstr, "603");
        break;
     case SET_604: strcpy(outputstr, "604");
        break;
     case SET_620: strcpy(outputstr, "620");
        break;
     case COM: strcpy(outputstr, "com");
        break;
     case ANY: strcpy(outputstr, "any");
        break;
     default: strcpy(outputstr, "undefined");
        break;
  }
  adbpr(catgets(scmc_catd,MS_print,E_MSG_25,
     "Current instruction set is %s.\n"),
     outputstr);
     }
     else if (!strcmp(inputstr, "pwr"))
         instruction_set = PWR;
     else if (!strcmp(inputstr, "pwrx"))
         instruction_set = PWRX;
     else if (!strcmp(inputstr, "ppc"))
         instruction_set = PPC;
     else if (!strcmp(inputstr, "default"))
         instruction_set = DEFAULT;
     else if (!strcmp(inputstr, "601"))
         instruction_set = SET_601;
     else if (!strcmp(inputstr, "603"))
         instruction_set = SET_603;
     else if (!strcmp(inputstr, "604"))
         instruction_set = SET_604;
     else if (!strcmp(inputstr, "620"))
         instruction_set = SET_620;
     else if (!strcmp(inputstr, "com"))
         instruction_set = COM;
     else if (!strcmp(inputstr, "any"))
         instruction_set = ANY;
     else
/*  Commenting out because message in catalog is incomplete   
  error(catgets(scmc_catd,MS_print,E_MSG_23,
     "Invalid instruction set."));
 */
  error("Invalid instruction set.\nChoices are pwr, pwrx, ppc, 601, 603, 604, com, any, default.\n");
 }
 break;
    case 'n':   /* To change the default mnemonic set */
 {
     register char *p = inputstr;
     (void)rdc();
     --lp;
     while (p < inputstr+8 && readchar() != '\n')
  *p++ = lastc;
     *p = '\0';
     --lp;
     if (strlen(inputstr) == 0) { /* no parameter */
  char outputstr[10];
  switch (mnemonic_set) {
     case PWR: strcpy(outputstr, "pwr");
        break;
     case PPC: strcpy(outputstr, "ppc");
        break;
     case DEFAULT: strcpy(outputstr, "default");
        break;
     default: strcpy(outputstr, "undefined");
        break;
  }
  adbpr(catgets(scmc_catd,MS_print,E_MSG_26,
     "Current mnemonic selection is %s.\n"),
     outputstr);
     }
     else if (!strcmp(inputstr, "pwr"))
         mnemonic_set = PWR;
     else if (!strcmp(inputstr, "ppc"))
         mnemonic_set = PPC;
     else if (!strcmp(inputstr, "default"))
         mnemonic_set = DEFAULT;
     else
  error(catgets(scmc_catd,MS_print,E_MSG_24,
     "Invalid mnemonic selection.\nChoices are pwr, ppc, default.\n"));
 }
 break;

     case 'f': 
	printflregs();
	break;
    default:
	error(catgets(scmc_catd,MS_extern,E_MSG_88,BADMOD));
    }
}

void printmap(s, amap)
STRING s;
MAP    *amap;
{
    int fid;

    fid = amap->ufd;
    adbpr("%s%12t`%s'\n", s, (fid < 0 ? "-" :
	    (fid == fcor ? corfil : symfil)));
    adbpr("b1 = %-16R", amap->b1);
    adbpr("e1 = %-16R", amap->e1);
    adbpr("f1 = %-16R", amap->f1);
    adbpr("\nb2 = %-16R", amap->b2);
    adbpr("e2 = %-16R", amap->e2);
    adbpr("f2 = %-16R", amap->f2);
    printc('\n');
}
/* Printregs, prints the value of the machine registers. This function takes 
   two arguments - r1 and r2. These arguments tells printregs which registers
   to print. If r1 and r2 equals -1, then all the machine registers are to be 
   printed. If r1 != -1, and r2 == -1, then only the register designated by
   the r1 should be printed. If r1 == -1 and r2 != -1, or ( r1 != -1 and 
   r2 != -1), then the registers r2+r1-1 , r2+r1-2, ...2,1,0 are to be printed.

   Return Value: None.
*/

LOCAL void printregs(r1, r2)
int r1, r2;
{
    REGPTR p = 0;
    long    v;
    int i = 0;

    if ( r1 == -1 && r2 == -1 ) {
	r1 = NREGS+NKREGS;
	r2 = 0;
    }
    else if ( r1 == -1 && r2 != -1 ) {
	r1 = NREGS+NKREGS;
	if ( VALID_SPR(r2) || VALID_FPR(r2) )
		r2 = REGNO(r2);
    }
    else if ( r2 == -1 && r1 != -1 ) {
	if ( VALID_FPR(r1) || VALID_SPR(r1) )
		r1 = REGNO(r1);
	r2 = r1;
    }
    else if ( r1 != -1 && r2 != -1 ) {
	if ( VALID_FPR(r1) || VALID_SPR(r1) )
		r1 = REGNO(r1);
	if ( VALID_FPR(r2) || VALID_SPR(r2) )
		r2 = REGNO(r2);
    }	

    p = &reglist[r1];
    do  {
	v = ADBREG(p->roffs);
	adbpr("%s%6t%R %16t", p->rname, v);
	valpr(v, (p->roffs == SYSREGNO(IAR) || p->roffs == REGNO(LINK) ? 
		  ISYM : DSYM ));
	if (p->roffs == SYSREGNO(CR) )
    		kcsflags((unsigned long)v);
    	printc('\n');
    } while ( --p >= &reglist[r2] );
    printc('\n');
    printpc();
}


/*
 * The loop goes backwards so we don't match string r15 against pattern r1.
 * This really should match the complete string (using while (isalnum(c)) or
 * something) so we wouldn't have to depend on such details.
 */
int getreg(regnam)
int regnam;
{
    REGPTR p;
    STRING regptr;
    char   *olp;

    olp = lp;
    for (p = &reglist[NREGS+NKREGS+NREGSYNONYMS]; --p >= reglist;) {
	regptr = p->rname;
	if (regnam == *regptr++) {
	    do {
		if (*regptr == '\0') return (p->roffs);
	    } while (readchar() == *regptr++);
	    lp = olp;
	}
    }
    return (NOTREG);
}

void printpc()
{
    dot = ADBREG(SYSREGNO(IAR));
    psymoff(dot, ISYM, ":%16t");
    printins();
    printc('\n');
    dot = ADBREG(SYSREGNO(IAR));                  /* printins smashed it */
}

void sigprint()
{
    if (signo > 0 && signo <= NSIG) {
	if (signals[signo-1] != NULL) {
	    adbpr("%s", signals[signo-1]);
	} else {
	    adbpr("signal %d", signo);
	}
    }
}

/* Valid_Reg() function checks to see, if the "reg" is a valid machine
   register or not. The macros VALID_*** are defined in the file sys/reg.h.
   
   Return Value: It returns a Boolean value.
*/
   
LOCAL BOOL Valid_Reg(reg)
int reg;
{
	if (reg >= 0 && (VALID_GPR(reg) || VALID_FPR(reg) || VALID_SPR(reg))) {
		if ( VALID_FPR(reg) ) {
			return FALSE;
		}
		return TRUE;
	}
	else {
		adbpr("%d:\tBad register number\n", reg);
	    	return FALSE;
	}
}

/* printflregs: prints the values of all the floating point 
   registers. */
LOCAL void printflregs() {
    int i;
    int j = 1;
    for( i = 0; i < MAXFPREGS; i++) {
	adbpr("fpr%~2.2d=%~16.16Z", i, fpregs[i]);
	if ( j == 3 ) {
		printc('\n');
		j = 1;
	}
	else {
		adbpr("  ");
		j++;
	}
    }
    printc('\n');
}
