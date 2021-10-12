static char sccsid[] = "@(#)44    1.25.2.22  src/bos/usr/ccs/lib/libdbx/examine.c, libdbx, bos411, 9434B411a 8/22/94 18:59:06";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dostep, endprogram, findformat, printdata, printerror,
 *	      printformat, printinst, printndata, printninst, printsig,
 *	      printvalue, setbp, unsetbp
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Machine independent "machine" facilities, such as examining addresses.
 */

#include "defs.h"
#include "symbols.h"
#include "envdefs.h"
#include "examine.h"
#include "object.h"
#include "mappings.h"
#include "process.h"
#include "cma_thread.h"
#ifdef K_THREADS
#include "k_thread.h"
#endif /* K_THREADS */

#include "disassembly.h"
#include "ops.h"
#include <signal.h>
#include <sys/errno.h>

#include <sys/types.h>
#include <limits.h>

skiptype	stepignore = skipfunc;
#ifdef K_THREADS
extern boolean traceexec;               /* trace execution */
#endif /* K_THREADS */

/*
 * Decode and print the instructions within the given address range.
 */

public printinst (lowaddr, highaddr)
Address lowaddr;
Address highaddr;
{
    register Address addr, lastaddr;

    lastaddr = 0;
    for (addr = lowaddr; ((addr <= highaddr) && (lastaddr <= addr)); ) {
	lastaddr = addr;
	addr = printop(addr,highaddr-addr);
    }
    prtaddr = addr;
}

/*
 * Another approach:  print n instructions starting at the given address.
 */

public printninst (count, addr)
int count;
Address addr;
{
    register Integer i;
    register Address newaddr,lastaddr;
    integer rangelimit;

    if (count <= 0) {
	error( catgets(scmc_catd, MS_examine, MSG_104,
					     "non-positive repetition count"));
    } 
    else 
    {
	rangelimit = count*4;
	lastaddr = newaddr = addr;
	for (i = 0; i < count; i++) {
	    newaddr = printop(newaddr,rangelimit);
	    rangelimit -= (newaddr - lastaddr);
	    lastaddr = newaddr;
	}
	prtaddr = newaddr;
    }
}

/*
 * Print the contents of the addresses within the given range
 * according to the given format.
 */

typedef struct {
    String name;
    String printfstring;
    int length;
} Format;

private Format fmt[] = {
    { "X", " %08x", sizeof(long) },
    { "c", " '%c'", sizeof(char) },
    { "s", "%c", sizeof(char) },
    { "x", " %04x", sizeof(short) },
    { "d", " %d", sizeof(short) },
    { "b", " \\%o", sizeof(char) },
    { "D", " %ld", sizeof(long) },
    { "o", " %o", sizeof(short) },
    { "O", " %lo", sizeof(long) },
    { "h", " %02x", sizeof(char) },
    { "f", " %.20g", sizeof(float) },
    { "g", " %.20g", sizeof(double) },
    { "q", "", 2*sizeof(double) },
    { "lld", " %lld", sizeofLongLong },
    { "llu", " %llu", sizeofLongLong },
    { "llx", " %llx", sizeofLongLong },
    { "llo", " %llo", sizeofLongLong },
    { nil, nil, 0 }
};

private Format *findformat (s)
String s;
{
    register Format *f;

    f = &fmt[0];
    while (f->name != nil and not streq(f->name, s)) {
	++f;
    }
    if (f->name == nil) {
	error( catgets(scmc_catd, MS_examine, MSG_112, 
						"bad print format \"%s\""), s);
    }
    return f;
}

/*
 * Retrieve and print out the appropriate data in the given format.
 * Floats have to be handled specially to allow the compiler to
 * convert them to doubles when passing to printf.
 *
 * For ILS support, printform returns size of character if it is 
 * printing in character form other wise it returns 1.
 */

/*  STRINGSIZE needs to be long enough to hold the largest
      octal + 1 byte for the null terminator  */
#define STRINGSIZE 24
private int printformat (fmt, addr)
Format *fmt;
Address addr;
{
    unsigned char c, mbc[MB_LEN_MAX + 1];
    unsigned short s;
    int i, mbl;
    float f;
    double d;
    quadf q;
    LongLong ll;
    char num_string[STRINGSIZE];
    char *res_string;
    extern void printquad();

    switch (fmt->name[0]) {
	case 'c':
            dread(mbc, addr, MB_LEN_MAX); /* get 1 char posibly multibyte */
            mbl = mblen(mbc, MB_LEN_MAX);
	    mbl = (mbl > 0) ? mbl : 1; /* mblen may return 0 or -1 */
            mbc[mbl + 1] = (char) 0; 
            if ( mbl > 1) /* output multibyte char as string */
             (*rpt_output)( stdout, " '%s'", mbc );
            else if (mbc[0] == 0)
		(*rpt_output)(stdout, " '\\0'");
	    else if (mbc[0] == '\b')
		(*rpt_output)(stdout, " '\\b'");
	    else if (mbc[0] == '\f')
		(*rpt_output)(stdout, " '\\f'");
	    else if (mbc[0] == '\t')
		(*rpt_output)(stdout, " '\\t'");
	    else if (mbc[0] == '\n')
		(*rpt_output)(stdout, " '\\n'");
	    else if (mbc[0] != 0 and mbc[0] < ' ')
		(*rpt_output)(stdout, " '^%c'",mbc[0] - 1 + 'A');
	    else if (mbc[0] >= ' ' && mbc[0] <= '~')
		(*rpt_output)(stdout, " '%c'",mbc[0]);
	    else
	        (*rpt_output)(stdout, fmt->printfstring, mbc[0]);
            return (mbl);
	case 'b':
	case 'h':
	case 's':
	    dread(&c, addr, sizeof(c));
	    (*rpt_output)(stdout, fmt->printfstring, c);
	    break;
	case 'd':
	case 'o':
	case 'x':
	    dread(&s, addr, sizeof(s));
	    (*rpt_output)(stdout, fmt->printfstring, s);
	    break;
	case 'D':
	case 'O':
	case 'X':
	    dread(&i, addr, sizeof(i));
	    (*rpt_output)(stdout, fmt->printfstring, i);
	    break;
	case 'f':
	    dread(&f, addr, sizeof(f));
	    (*rpt_output)(stdout, fmt->printfstring, f);
	    break;
	case 'g':
	    dread(&d, addr, sizeof(d));
	    (*rpt_output)(stdout, fmt->printfstring, d);
	    break;
	case 'q':
	    dread(&q, addr, sizeof(q));
	    printquad(rpt_output, stdout, q);
            break;
	case 'l':
            dread (&ll, addr, sizeof(ll));
            (*rpt_output)(stdout, fmt->printfstring, ll);
            break;
	      
	default:
	    badcaseval(fmt->name);
    }
    return (1);
}

public Address printdata (lowaddr, highaddr, format)
Address lowaddr;
Address highaddr;
String format;
{
    int m, n;
    register Address addr;
    Format *f;

    if (lowaddr > highaddr) {
	error( catgets(scmc_catd, MS_examine, MSG_151,
					  "first address larger than second"));
    }
    f = findformat(format);
    n = 0;
    for (addr = lowaddr; addr <= highaddr; addr += f->length) {
	if (n == 0) {
	    (*rpt_output)(stdout, "%08x: ", addr);
        }
        m = printformat(f, addr);
	n += m;
        addr += m - 1; /* increment address if multibyte char was printed. */
	if (n >= (16 / f->length)) {
	    (*rpt_output)(stdout, "\n");
	    n = 0;
	}
    }
    if (n != 0) {
	(*rpt_output)(stdout, "\n");
    }
    prtaddr = addr;
    return addr;
}

/*
 * The other approach is to print n items starting with a given address.
 */

public printndata (count, startaddr, format)
int count;
Address startaddr;
String format;
{
    int i, m, n;
    Address addr;
    Format *f;
    Boolean isstring;
    char c;
    unsigned char mbc[MB_LEN_MAX + 1];
    int j, mbl;

    if (count <= 0) {
	error( catgets(scmc_catd, MS_examine, MSG_104,
					     "non-positive repetition count"));
    }
    f = findformat(format);
    isstring = (Boolean) streq(f->name, "s");
    n = 0;
    addr = startaddr;
    for (i = 0; i < count; i++) {
	if (n == 0) {
	    (*rpt_output)(stdout, "%08x: ", addr);
	}
	if (isstring) {
	    errno = 0;
	    dread(mbc, addr, MB_LEN_MAX);
	    if ((mbc[0] == '\377') && (errno == EIO)) {
	        (*rpt_error)(stdout, "Invalid string address\n");
		i = count;
	    } else {
                /*  print out beginning quote  */
	        (*rpt_output)(stdout, "\"");
 
                /*  while the null-terminator has not been reached  */
	        while (mbc[0] != '\0') {
                  
                    /*  get the length of the character  */
                    mbl = mblen (mbc, MB_LEN_MAX);

                    /*  if mblen cannot determine the character length,
                          it will return -1 - this is probably a partial
                          character.  Print each byte of it as a single
                          character.  */

                    mbl = (mbl > 0) ? mbl : 1;

                    /*  if the character is multi-byte  */
                    if (mbl > 1)
                    {

                      /*  print directly by looping through all the bytes  */

                      for (j = 0; j < mbl; j++)
                        (*rpt_output) (stdout, "%c", mbc[j]);
                    }
                    else
                    {
                      /*  call printchar to make sure non-printable
                            character are handled correctly  */
		      printchar(mbc[0]);
                    }
                    /*  increment addr to location of next character  */
		    addr += mbl;

                    /*  read the next character  */
		    dread(mbc, addr, MB_LEN_MAX);
	        }

                /*  print ending quote and newline character  */
	        (*rpt_output)(stdout, "\"\n");
	        n = 0;
	        addr++;
	    }
	} else {
            m = printformat(f, addr);
	    n += m;
            addr += m - 1; /* increment addr if multibyte char was printed. */
	    if (n >= (16 / f->length)) {
		(*rpt_output)(stdout, "\n");
		n = 0;
	    }
	    addr += f->length;
	}
    }
    if (n != 0) {
	(*rpt_output)(stdout, "\n");
    }
    prtaddr = addr;
}

/*
 * Print out a value according to the given format.
 */

public printvalue (v, format)
long v;
String format;
{
    Format *f;
    char *p, *q;

    f = findformat(format);
    if (streq(f->name, "s")) {
	(*rpt_output)( stdout, "\"");
	p = (char *) &v;
	q = p + sizeof(v);
	while (p < q) {
	    printchar(*p);
	    ++p;
	}
	(*rpt_output)( stdout, "\"");
    } else {
	(*rpt_output)(stdout, f->printfstring, v);
    }
    (*rpt_output)( stdout, "\n");
}

/*
 * sys_siglist is provided through libc.a; It contains a message for each signal
 */
extern char	*sys_siglist[];

/*
 * Print out an execution time error.
 * Assumes the source position of the error has been calculated.
 *
 * Have to check if the -r option was specified; if so then
 * the object file information hasn't been read in yet.
 */

public printerror ()
{
    integer err;
    int	rc;
    Address	instruction;
    inst	disassembly;
    Boolean	valid = false;
    char	valid_sets[20] = "";

    if (isfinished(process)) {
	err = exitcode(process);
	if (err == 0) {
	    (*rpt_output)(stdout,  catgets(scmc_catd, MS_examine, MSG_165,
				     "\"%s\" terminated normally\n"), objname);
	} else {
	    (*rpt_output)(stdout,  catgets(scmc_catd, MS_examine, MSG_166,
			"\"%s\" terminated abnormally (exit code %d)\n"),
			objname, err);
	}
	erecover();
    }
    err = errnum(process);
    (*rpt_output)( stdout, "\n");
    printsig(err);
    (*rpt_output)( stdout, " ");
    printloc();
    (*rpt_output)( stdout, "\n");
    if( err == SIGILL ) {
	/*
	 * Output message indicating which instruction set instruction is valid
	 */
	iread(&instruction, pc, sizeof(instruction));
	rc = decode_instruction( instruction, &disassembly, pc,
				 current_hardware, mnemonic_set );
	if( rc == 0 && disassembly.target_addr & ANY ) {
#if 0
	    if( disassembly.target_addr & PWR ) {
		strcpy( valid_sets, "pwr" );
		valid = true;
	    }
	    if( disassembly.target_addr & PWRX ) {
		if( valid == true ) {
		    strcat( valid_sets, ", pwrx" );
		} else {
		    strcpy( valid_sets, "pwrx" );
		    valid = true;
		}
	    } 
	    if( disassembly.target_addr & PPC ) {
		if( valid == true ) {
		    strcat( valid_sets, ", ppc" );
		} else {
		    strcpy( valid_sets, "ppc" );
		    valid = true;
		}
	    }
	    if( disassembly.target_addr & SET_601 ) {
		if( valid == true ) {
		    strcat( valid_sets, ", 601" );
		} else {
		    strcpy( valid_sets, "601" );
		    valid = true;
		}
	    }
	    if( disassembly.target_addr & SET_603 ) {
		if( valid == true ) {
		    strcat( valid_sets, ", 603" );
		} else {
		    strcpy( valid_sets, "603" );
		    valid = true;
		}
	    }
	    if( disassembly.target_addr & SET_604 ) {
		if( valid == true ) {
		    strcat( valid_sets, ", 604" );
		} else {
		    strcpy( valid_sets, "604" );
		    valid = true;
		}
	    }
#else
	    if (disassembly.target_addr & PWR)
		strcat(valid_sets, "pwr, ");
	    if (disassembly.target_addr & PWRX)
               strcat(valid_sets, "pwrx, ");
	    if (disassembly.target_addr & PPC)
               strcat(valid_sets, "ppc, ");
	    if (disassembly.target_addr & SET_601)
               strcat(valid_sets, "601, ");
	    if (disassembly.target_addr & SET_603)
               strcat(valid_sets, "603, ");
	    if (disassembly.target_addr & SET_604)
               strcat(valid_sets, "604, ");

            /*  remove the last ", "  */
            if (strlen(valid_sets) > 1)
              valid_sets[strlen(valid_sets) - 2] = '\0';
#endif
	    (*rpt_output)( stdout, catgets( scmc_catd, MS_examine, MSG_178,
				"Instruction is valid on: %s\n"), valid_sets );
	}
    }
    if (curline > 0) 
    {
	  printlines(curline, curline);
    } 
    else 
    {
	  printinst(pc, pc);
    }
    erecover();
}

/*
 * Print out a signal.
 */

private String illinames[] = {
    "reserved addressing fault",
    "priviliged instruction fault",
    "reserved operand fault"
};

private String fpenames[] = {
    nil,
    "integer overflow trap",
    "integer divide by zero trap",
    "floating overflow trap",
    "floating/decimal divide by zero trap",
    "floating underflow trap",
    "decimal overflow trap",
    "subscript out of range trap",
    "floating overflow fault",
    "floating divide by zero fault",
    "floating undeflow fault"
};

public printsig (signo)
integer signo;
{
    integer code;
    extern boolean forkpending, execpending, loadpending, printExcept();
    extern boolean just_attached;

    if (signo < 0 or signo >= NSIG) {
	(*rpt_output)(stdout, "[signal %d]", signo);
    } else {
	if  (just_attached) {
		 (*rpt_output)(stdout,"attached");
		 return;
	}
	if (signo == SIGTRAP) {
	    if (forkpending) {
		 (*rpt_output)(stdout,
			"stopped due to fork with multiprocessing enabled");
		 forkpending = false;
	    } else if (execpending) {
		 (*rpt_output)(stdout,
			"stopped due to exec with multiprocessing enabled");
		 execpending = false;
	    } else if (loadpending) {
		 (*rpt_output)(stdout, "stopped due to load or unload");
		 loadpending = false;
	    } else if (!(printExcept())) {	/* returns true if successful */
		 (*rpt_output)(stdout, "%s", sys_siglist[signo]);
	    }
	} else {
	    (*rpt_output)(stdout, "%s", sys_siglist[signo]);
	}
    }
    if (signo == SIGILL) {
        code = errcode(process);
	if (code >= 0 and code < sizeof(illinames) / sizeof(illinames[0])) {
	    (*rpt_output)(stdout, " (%s)", illinames[code]);
	}
    } else if (signo == SIGFPE) {
        code = errcode(process);
	if (code > 0 and code < sizeof(fpenames) / sizeof(fpenames[0])) {
	    (*rpt_output)(stdout, " (%s)", fpenames[code]);
	}
    }
}

/*
 * Note the termination of the program.  We do this so as to avoid
 * having the process exit, which would make the values of variables
 * inaccessible.  We do want to flush all output buffers here,
 * otherwise it'll never get done.
 */

public endprogram ()
{
    int exitcode;

    printnews(false);
    exitcode = argn(1, nil);
    if (exitcode != 0) {
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_examine, MSG_176,
			  "\nexecution completed (exit code %d)\n"), exitcode);
    } else {
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_examine, MSG_177,
						   "\nexecution completed\n"));
    }
    getsrcpos();
    action_mask |= EXECUTION_COMPLETED;
    erecover();
}


/*
 * Single step the machine a source line (or instruction if "inst_tracing"
 * is true).  If "isnext" is true, skip over procedure calls.
 */
typedef List Bplist;
extern Bplist bplist;                  /* List of active breakpoints */
extern boolean catchbp;

public dostep (isnext)
Boolean isnext;
{
    register Address oldaddr, addr;
    register Lineno line;
    register Breakpoint bp;
    Address orgin;
    Symbol   func;

#ifdef KDBX
    Address oldpc;
#endif /* KDBX */

    String filename;
#ifdef K_THREADS
    extern next_subcommand;
    extern  Boolean handler_signal;
#endif /* K_THREADS */

#ifdef KDBX
    oldpc = pc;
#endif /* KDBX */

#ifdef K_THREADS
    if (! next_subcommand)
        /* call by stepover() */
        thread_k(th_hold_other, nil);
    /* in case of kernel threads and $hold_next is set don't resume */
    /* the others threads : nextaddr can call stepto() so resumes   */
    /* all the threads                                              */
    /* If $hold_next is set, we want to prevent thread switching */
    /* by suspending all other non-running thread when the       */
    /* running thread is executing.                              */
    if ((lib_type == KERNEL_THREAD) && running_thread && varIsSet("$hold_next"))
        threads(th_hold_other, nil);
#endif /* K_THREADS */
    addr = nextaddr(pc, isnext);
    if (not inst_tracing and nlines_total != 0) {
	line = linelookup(addr);
	while (line == 0 && (oldaddr != addr)) {
            oldaddr = addr;
	    /*
	     * If we are skipping functions with no source, check to
	     * see if the current func has source. 
	     * If we are skipping modules, check to see if the module
	     * of the current address has source.
	     * If either is true, go back to the return address rather
	     * than stepping through the code, provided that the routine
	     * we are in is not linkage code, and is not marked don't
	     * skip.  Also, do not try this when we are in a prolog as
	     * the traceback info is not necessarily correct.
	     */
	    if (((stepignore == skipfunc
		  && (func=whatblock(addr))
		  && nosource(func))
		 || (stepignore == skipmodule
		     && nlhdr[addrtoobj(addr)].nlines == 0
		     && (func=whatblock(addr))))
		&& !func->symvalue.funcv.dontskip
		&& !func->symvalue.funcv.islinkage
		&& !isprolog(addr, nil)
		)
	    {
		if ((addr = return_addr()) != 0)
		    addr = dojump(addr, isnext);
		else
		    addr = nextaddr(oldaddr, isnext);
	    }
	    else
	    	addr = nextaddr(addr, isnext);
	    line = linelookup(addr);
	}
	curline = line;
    } else {
	curline = 0;
    }
#ifdef K_THREADS
    if (! next_subcommand)
        /* call by stepover() */
        thread_k(th_unhold_other, nil);
#endif /* K_THREADS */

#if defined (CMA_THREAD) ||  defined (K_THREADS)
    /* If $hold_next is set, we want to prevent thread switching */
    /* by suspending all other non-running thread when the       */
    /* running thread is executing.                              */ 
    if (running_thread && varIsSet("$hold_next"))
        threads(th_hold_other, nil);
#endif /* CMA_THREAD || K_THREADS */
#ifdef K_THREADS
     /* if kernel threads and stepi : only the running_thread  */
     /* is resumed                                             */
     /* thread-k(th_hold_other)  only set the field ti_hold    */
     /* in the pthread structures                              */
     if (lib_type == KERNEL_THREAD && running_thread &&
         inst_tracing && not isnext)
        thread_k(th_hold_other, nil);
#endif /* K_THREADS */


    /* If $catchbp is set, we want to catch breakpoints when    */
    /* we are doing a next command. To do this we turn on our   */
    /* breakpoints EXCEPT for a breakpoint (if one exists) that */
    /* is set at our current location (orgin) since we cannot    */
    /* step through breakpoints.                                */
    if (catchbp and isnext) {
      orgin = reg(SYSREGNO(PROGCTR));
      foreach (Breakpoint, bp, bplist)
          if (bp->bpaddr != orgin)
              setbp(bp->bpaddr);
      endfor
    }
#ifdef KDBX
    if (not inst_tracing and nlines_total != 0)
	stepto(addr);
    else if (isnext)
	pnext(process, DEFSIG);
    else
    {
      if (oldpc == pc)
        /* we must actually step */
        pstep(process, DEFSIG);
        /* else : the step has been already done by nextaddr */
        /* in case of branch instruction */
    }
#else /* KDBX */
    /* test local break-point in stepto */
#ifdef K_THREADS
    /* if there is a signal handler the stepi instruction is not necessary */
    /* and in case of this signal handler is a thread we have to resume it */
    if (not handler_signal or  next_subcommand)
#endif /* K_THREADS */
	    stepto(addr);
#endif /* KDBX */

    /* Unset all breakpoints set for $catchbp and isnext */
    if (catchbp and isnext)
    {
        foreach (Breakpoint, bp, bplist)
            if (bp->bpaddr != orgin)
                unsetbp(bp->bpaddr);
        endfor
    }
    filename = srcfilename(addr);
    setsource(filename);

#if defined (CMA_THREAD) ||  defined (K_THREADS)
    /* resume all non-running thread if they were suspended */
    if (running_thread && varIsSet("$hold_next"))
        threads(th_unhold_other, nil);
#endif /* CMA_THREAD || K_THREADS */
#ifdef K_THREADS
     /* if kernel threads and stepi : only the running_thread  */
     if (lib_type == KERNEL_THREAD && running_thread &&
         inst_tracing && not isnext)
        thread_k(th_unhold_other, nil);
#endif /* K_THREADS */
}

/*
 * Setting a breakpoint at a location consists of saving
 * the word at the location and poking a BP_OP there.
 *
 * We save the locations and words on a list for use in unsetting.
 */

typedef struct Savelist *Savelist;

struct Savelist {
    Address location;
    Bpinst save;
    short refcount;
    Savelist link;
};

private Savelist savelist;


/*
 * Breakpoints need to be refreshed after a load() since the text is
 * remapped (a new segment w/o the breakpoints is being created and 
 * used after the load() ).
 */
public refreshbp()
{
   Bpinst w = BP_OP;
   register Savelist s;
   for (s = savelist; s != nil; s = s->link) {
       if (s->refcount > 0) {
          iwrite(&w, s->location, sizeof(w));
       }
   }
}

/*
 * Set a breakpoint at the given address.  Only save the word there
 * if it's not already a breakpoint.
 */

public setbp (addr)
Address addr;
{
    Bpinst w, save;
    register Savelist newsave, s;

    for (s = savelist; s != nil; s = s->link) {
	if (s->location == addr) {
	    s->refcount++;
	    return;
	}
    }
    iread(&save, addr, sizeof(save));
    newsave = new(Savelist);
    newsave->location = addr;
    newsave->save = save;
    newsave->refcount = 1;
    newsave->link = savelist;
    savelist = newsave;
    w = BP_OP;
    iwrite(&w, addr, sizeof(w));
}

/*
 * Unset a breakpoint; unfortunately we have to search the SAVELIST
 * to find the saved value.  The assumption is that the SAVELIST will
 * usually be quite small.
 */

public unsetbp (addr)
Address addr;
{
    register Savelist s, prev;

    prev = nil;
    for (s = savelist; s != nil; s = s->link) {
	if (s->location == addr) {
	    iwrite(&s->save, addr, sizeof(s->save));
	    s->refcount--;
	    if (s->refcount == 0) {
		if (prev == nil) {
		    savelist = s->link;
		} else {
		    prev->link = s->link;
		}
		dispose(s);
	    }
	    return;
	}
	prev = s;
    }
    panic("unsetbp: couldn't find address %d", addr);
}

/*
 * Determine if a breakpoint is caused by dbx.
 */

public Boolean isdbxbp(stepbp)
Address stepbp;
{
    register Savelist s;

    if (pc == stepbp) {
	return true;
    } else {
        for (s = savelist; s != nil; s = s->link) {
	    if (s->location == pc) {
	        return true;
	    }
        }
    }
    return false;
}

/*  called by ffork when multproc == child to delete
      all breakpoints - unsetallbps will not work because
      of next and step  */

void delete_all_bps()
{
  register Savelist s;

  for (s = savelist; s != NULL; s = s->link)
  {
    iwrite(&s->save, s->location, sizeof(s->save));
  }
}
