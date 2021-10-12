static char sccsid[] = "@(#)74	1.49.2.39  src/bos/usr/ccs/lib/libdbx/process.c, libdbx, bos411, 9438B411a 9/21/94 14:31:51";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: CheckDataAddr, CheckTextAddr, cacheflush, fetch, fpregval,
 *	      getinfo, getldrinfo, getpc, getprog, infrom, noexec_file_map,
 *	      outto, pcont, pio, printloaderinfo, printptraceinfo,
 *	      process_init, psigtrace, pstart, pterm, reg, setinfo, setreg,
 *	      setregs, setsigtrace, sigs_off, sigs_on, store, unsetsigtraces,
 *	      usignal, sync_load_info, debuggee_setpgrp, debugger_setpgrp
 *            gettid, getsigstatus, getheapsize
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
 * Process management.
 *
 * This module contains the routines to manage the execution and
 * tracing of the debuggee process.
 */

#include "defs.h"
#include "envdefs.h"
#include <sys/types.h>
#include "process.h"
#include "execute.h"
#include "ops.h"
#include "eval.h"
#include "object.h"
#include "main.h"
#include "coredump.h"
#include "cma_thread.h"
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/stat.h>
#include <sys/FP.h>
#include <sys/core.h>
#ifndef KDBX
#include <sys/systemcfg.h>
#endif /* KDBX */
#ifdef K_THREADS
#include <sys/uthread.h>
#endif /* K_THREADS  */
#if defined (CMA_THREAD) ||  defined(K_THREADS)
#include "k_thread.h"
#endif /* CMA_THREAD || K_THREADS*/
#include "cma_thread.h"
#ifdef _THREADS
#include <procinfo.h>
#endif /* _THREADS */

#define LDCHUNK		4096

#define TRAPI_OPCODE 3
#define TRAP_OPCODE 31

#ifdef KDBX
#define PT_READ_U    	3
#define U_READ     	PT_READ_U
#endif /* KDBX */

/*  NOTE : the opcode of an assembly instruction in in the
           first 6 bits (bits 0 through 5).  The length of 
           an assembly instruction is 32 bits.  To isolate 
           the opcode, shift the instruction right 26 bits 
           (32 - 6)  */ 

/*  macro to isolate the opcode from an instruction  */
#define get_opcode(inst) (inst >> 26)

/*  macro to determine if an instruction is a trap  */
#define is_trap_instruction(x)   ((get_opcode(x) == TRAPI_OPCODE) \
                               || (get_opcode(x) == TRAP_OPCODE))

public integer loadcnt = 0;

extern Boolean traceback_table;    /* This is defined in frame.c     */ 
extern Boolean just_started;       /* These are defined in execute.c */
extern Boolean noexec,textaddrs;
extern String errfile;
extern Address text_reloc, data_reloc;

extern String corename;
extern Ttyinfo ttyinfo;
extern Ttyinfo ttyinfo_in;

private struct Process pbuf;

struct ldinfo *loader_info;
struct fdinfo *fd_info;
unsigned short current_hardware;

extern struct core_dump corehdr;
extern unsigned coresize;

pid_t debugger_pgrp;		/* process group of dbx */
pid_t debuggee_pgrp;		/* progess group of debuggee */
static boolean pgrps_set = false;
static boolean job_control = true;

#ifdef _THREADS
/* typedef and pointers used by gettid() and getstatus() : access to processe */
/* information and threads information                                        */
typedef struct thrdsinfo  thrdsinfo;
typedef struct mstsave  mstsave;
typedef struct procsinfo  procsinfo;
public int *loader_offset;            /* offset in core file of data region */
public int nb_k_threads = 0;          /* number of kernel threads */
public tid_t tid_running_thread = 0;  /* tid of the running_thread */
public  thrdsinfo *pthrdsinfo = NULL; /* pointer to a buffer where thrdsinfo */
                                      /* structures are stored by getthrds() */
public struct procsinfo *pprocsinfo = NULL; /* pointer to a buffer where     */
                                            /* procsinfo structure is stored */
                                            /* by getprocs()                 */
#endif /* _THREADS */


private Word fetch();
private store();
private infrom();
private outto();

#ifndef _NO_PROTO
void (*save_sig)(int);
#else
void (*save_sig)();
#endif
static int debuggee_fd = 0;     /* When screen(1) is called, debuggee_fd
                                   will be set to the originating tty,
                                   otherwise, it is 0.   */

static boolean called_debuggee_setpgrp = false; /* Used to ensure that      */
                                                /* debugge[er]_setpgrp()    */
                                                /* only get called in pairs */

/*
 * Initialize process information.
 */

public process_init ()
{
    register int i;
    char buf[10];
    struct stat core_times, obj_times, prog_times;
    struct user uarea;
    String base_objname, base_progname;

    process = &pbuf;
    if (!pgrps_set) {		/* only set these once */
      debuggee_pgrp = debugger_pgrp = getpgrp();
      pgrps_set = true;
      /*
       * This test tries to determine if the parent shell supports
       * job control or not. The test passes if the shell is sh (which
       * does not have job control) and fails for ksh and csh (which do have
       * job control). However, if the behavior of the shells change, or if
       * other factors affect the output of these sys calls, or if some
       * other shells are used (tcsh, bash, etc...), this test may not
       * be valid.
       * The whole reason for this test is so that we do not call
       * tcsetpgrp() if there is no job control.
       */
#ifdef KDBXRT
      job_control = false;
#else
      if (getpid() != getpgrp() && getppid() == getpgrp())
	job_control = false;
#endif
    }
    process->status = (coredump || norun || noexec) ? STOPPED : NOTSTARTED;
    setsigtrace();

    process->is_bp = false;

    /*
     * Determine what hardware we are currently running on.  Set external
     * variable current_hardware to tell this
     */
#if defined(_POWER) && !defined(KDBX)
    if( __power_rs1() || __power_rsc() ) {
	current_hardware = PWR;
    } else if( __power_rs2() ) {
	current_hardware = PWRX;
    } else if( __power_601() ) {
	current_hardware = SET_601;
    } else if( __power_603() ) {
	current_hardware = SET_603;
    } else if( __power_604() ) {
	current_hardware = SET_604;
    } else {
	/*
	 * Not one of our recognized hardware types.  Set to ANY
	 */
	current_hardware = ANY;
    }
#else
    current_hardware = ANY;
#endif
    getfptype(&fpregs);
    for (i = 0; i < NGREG; i++) {
        sprintf(buf, "$r%d", i);
        defregname(identname(buf, false), i);
        sprintf(buf, "$R%d", i);
        defregname(identname(buf, false), i);
    }
    for (i = 0; i < fpregs; i++) {
        sprintf(buf, "$fr%d", i);
        deffregname(identname(buf, false), i + NGREG + NSYS);
        sprintf(buf, "$FR%d", i);
        deffregname(identname(buf, false), i + NGREG + NSYS);
    }
    defsysregs();
    /*
     *    Check to make sure we are reading a valid core file.
     *    We do this by (1) making sure the core file was generated
     *    by the current program we are debugging and (2) by making
     *    sure the core file is newer.
     */
    if (coredump) 
    {
	fread((void *) &corehdr, (size_t) 1, (size_t) CHDRSIZE, corefile);
	rewind(corefile);
	stat(corename, &core_times);
	coresize = (unsigned) core_times.st_size;
	getldrinfo(true);
	if (coredump) {
	   /* Strip path to object file if it exists. */
	   base_objname = (String) strrchr(objname,'/');
	   if (!base_objname)
	      base_objname = objname;
	   else
	      base_objname++;
	   /* Strip path to exec name from core file if it exists. */
	   base_progname = (String) strrchr(fd_info[0].pathname,'/');
	   if (!base_progname)
	      base_progname = fd_info[0].pathname;
	   else
	      base_progname++;
	   if (streq(corename,"/dev/mem") || streq(corename,"/dev/kmem")) {
	          coredump_readin(process->reg, process->freg, process->signo);
	   } else {
	          stat(objname,  &obj_times);
	          stat(base_progname, &prog_times);
						/* verify No. 1 */
		  if ((!strcmp(base_progname,base_objname)) || 
					/* determine if linked file */
	             ((prog_times.st_dev == obj_times.st_dev) &&
		      (prog_times.st_ino == obj_times.st_ino))) { 

						/* No. 2 */
	              if (obj_times.st_mtime <= core_times.st_mtime)  
	                 coredump_readin(process->reg, process->freg,
							       process->signo);
	              else {
	                 coredump = false;
		         error( catgets(scmc_catd, MS_process, MSG_267,
		     "Core file is older than current program (core ignored)"));
		       }
#ifdef _THREADS
                       if (!corehdr.c_flag & CORE_VERSION_1) {/* AIX 4 ?*/
                           coredump = false;
                           error( catgets(scmc_catd, MS_process, MSG_538,
                                "%s is not a valid core file (core ignored)"),
                                 corename);
                       }
#endif /* _THREADS  */
	          } else {
	              coredump = false;
	              error( catgets(scmc_catd, MS_process, MSG_537,
	             "Core file program (%s) does not match current program \
(core ignored)"), base_progname );
	          }
	   }
       }
    }
    if (coredump || noexec) {
	for (i = 0; i <= regword(NGREG+NSYS+MAXFREG+1-1); i++) {
	    process->valid[i] = 0xffffffff;
	    process->dirty[i] = 0;
	}
	pc = reg(SYSREGNO(PROGCTR));
    } else {
        process->status = (norun) ? STOPPED : NOTSTARTED;
    }
    arginit();
}
/*
 * getldrinfo:  get necessary loader information from the process
 *	Note:  the loader information is actually slightly different
 *	       from the ldinfo structure used here.  To see the actual
 *	       loader structure, see <sys/ldr.h>.
 */

public getldrinfo (iscoreimage) 
Boolean iscoreimage;
{
char *ldinfo_ptr;
boolean badinfo = true;
unsigned int ldinfosz = LDCHUNK;
unsigned int ldcnt = 0;
unsigned int i;

if (iscoreimage) {
	if (vaddrs) {	/* Create the core file  */
    	    loader_info = (struct ldinfo *) malloc(sizeof(struct ldinfo));
    	    fd_info = (struct fdinfo *) malloc(sizeof(struct fdinfo));
            loader_info->ldinfo_next = 0;
            loader_info->ldinfo_fd = 0;
            loader_info->textorg = 0;
            loader_info->textsize = 0xffffffff;
            loader_info->dataorg = 0;
            loader_info->datasize = 0xffffffff;
            fd_info->pathname = objname;
            fd_info->membername = (char *) 0;
            ldcnt = 1;	/* Allow perusal of the object file only. */
	    loadcnt = ldcnt;
	    return;
	} else {
	   /* Stack is after ldinfop in core file. */
	   if (((unsigned) corehdr.c_stack) > ((unsigned) corehdr.c_tab)) {
#ifdef _THREADS
              if ( corehdr.c_msts)
                 ldinfosz = (unsigned int) ((unsigned) corehdr.c_msts) -
                                                    ((unsigned) corehdr.c_tab);
              else
                 ldinfosz = (unsigned int) ((unsigned) corehdr.c_stack) -
                                                    ((unsigned) corehdr.c_tab);
#else
              ldinfosz = (unsigned int) ((unsigned) corehdr.c_stack) -
                                                    ((unsigned) corehdr.c_tab);
#endif /* _THREADS  */
	      ldinfo_ptr = (char *) malloc(ldinfosz);
#ifdef _THREADS
              if (((Address)corehdr.c_tab) >= CHDRSIZE  &&
                (((Address)corehdr.c_tab) < coresize))
#else
	      if ((((Address)corehdr.c_tab) >= (sizeof(corehdr.c_u) +
		     (Address)(&corehdr.c_u) - (Address)(&corehdr.c_signo))) &&
	          (((Address)corehdr.c_tab) < coresize))
#endif /* _THREADS  */
	      {
	   	   fseek(corefile, (long int) corehdr.c_tab, 0);
	   	   fread((void *) ldinfo_ptr, (size_t) 1,
						  (size_t) ldinfosz, corefile);
	      } else {
	           coredump = false;
		   error( catgets(scmc_catd, MS_process, MSG_538,
		      "%s is not a valid core file (core ignored)"), corename);
		   if (ldinfo_ptr) {
	               free((void *)ldinfo_ptr);
		   }
		   return;
	      }
	   } else {
	           coredump = false;
		   if (corehdr.c_stack)
		   	error( catgets(scmc_catd, MS_process, MSG_538,
		           "%s is not a valid core file (core ignored)"), 
			   corename);
		   else
			error( catgets(scmc_catd, MS_process, MSG_550,
			   "%s does not contain a stack (core ignored)"),
			   corename);
		   return;
	   }
	}
    } else if (!noexec) {
       ldinfo_ptr = (char *) malloc(LDCHUNK);
       badinfo = (boolean) ptrace(PT_LDINFO,process->pid,ldinfo_ptr,
							         ldinfosz,0);
       for (; badinfo; ) {
           ldinfosz += LDCHUNK;
           ldinfo_ptr = (char *) realloc(ldinfo_ptr,ldinfosz);
           badinfo = (boolean) ptrace(PT_LDINFO,process->pid,ldinfo_ptr,
							         ldinfosz,0);
       }
    }
    loader_info = (struct ldinfo *) malloc(ldinfosz);
#ifdef _THREADS
    if (coredump) 
        loader_offset = (int *) malloc(corehdr.c_entries * sizeof(int));
#endif /* _THREADS  */
    fd_info = (struct fdinfo *) malloc(ldinfosz/3);
    if (!noexec) {
       /* Rearrange the information so that it is manageable. */
       do {
           memcpy(&loader_info[ldcnt],ldinfo_ptr,sizeof(struct ldinfo));
#ifdef _THREADS
           /*the field ldinfo_fd contains the offset in the core file of the */
           /* data region associated with the loaded module                  */
           if (coredump) loader_offset[ldcnt] = loader_info[ldcnt].ldinfo_fd;
#endif /* _THREADS  */
           fd_info[ldcnt].pathname = ldinfo_ptr + sizeof(struct ldinfo);
           fd_info[ldcnt].membername =
	       fd_info[ldcnt].pathname + 
				       strlen(fd_info[ldcnt].pathname) + 1;
           ldinfo_ptr += loader_info[ldcnt].ldinfo_next;
       } while (loader_info[ldcnt++].ldinfo_next != 0);
    } else {
       /* This will be fixed up after reading in the object header */
       loader_info->ldinfo_next = 0;
       loader_info->ldinfo_fd = 0;
       loader_info->textorg = 0;
       loader_info->textsize = 0;
       loader_info->dataorg = 0;
       loader_info->datasize = 0;
       fd_info->pathname = objname;
       fd_info->membername = (char *) 0;
       ldcnt = 1;	/* Allow perusal of the object file only. */
    }
    loadcnt = ldcnt;
}

sync_load_info(prev_ldinfo,prev_fdinfo,prev_ldcnt)
int prev_ldcnt;
struct ldinfo *prev_ldinfo;
struct fdinfo *prev_fdinfo;
{
	int oldndx = 1;	/* No need to check execed prog, is always = 0 */
	int ldndx;	/* Indices for previous load info map */
	struct ldinfo *old_ld = prev_ldinfo;
	struct fdinfo *old_fd = prev_fdinfo;
	struct ldinfo *new_ld = loader_info;
	struct fdinfo *new_fd = fd_info;

	int *found_match;	/* Not newly loaded objects. */
	int *retained_prog;	/* Previously loaded objects. */

	found_match = (int *) calloc(loadcnt,sizeof(int));
	retained_prog = (int *) calloc(prev_ldcnt,sizeof(int));

	for (;oldndx <= prev_ldcnt-1; oldndx++) {
	    for (ldndx = 1;ldndx <= loadcnt-1; ldndx++) {
	    /* If address maps and object names the same, they're the same */
		 if (((old_ld[oldndx].textorg == new_ld[ldndx].textorg)) &&
		     ((old_ld[oldndx].textsize == new_ld[ldndx].textsize)) &&
		     ((old_ld[oldndx].dataorg == new_ld[ldndx].dataorg)) &&
		     ((old_ld[oldndx].datasize == new_ld[ldndx].datasize)) &&
		 /* check file name */
		     (streq(old_fd[oldndx].pathname,new_fd[ldndx].pathname)) &&
		 /* if archive, must check member name */
		     ((!(old_fd[oldndx].membername ||
			 			new_fd[ldndx].membername)) ||
		      (((old_fd[oldndx].membername &&
			 			new_fd[ldndx].membername)) &&
		       (streq(old_fd[oldndx].membername,
					        new_fd[ldndx].membername))))){
		 /* swap in old information if same */
		    retained_prog[oldndx] = ldndx;
		    found_match[ldndx] = oldndx;
		    break;
		    
	         }
	    }
	}
	update_loaded_object(prev_ldcnt, loadcnt, retained_prog,
			     found_match, old_ld);
}

/* 
 * noexec_file_map - Set up maps for non-executable debuggees
 */
void noexec_file_map(magic, txtstart, txtlen, datastart, datalen, fd)
short magic;
Address txtstart, txtlen, datastart, datalen;
int fd;
{
   loader_info->ldinfo_fd = 0;
   if (exec_object(magic)) {
       loader_info->textorg = txtstart;
       loader_info->textsize = txtlen;
       loader_info->dataorg = datastart;
       loader_info->datasize = datalen;
   } else {
       loader_info->textorg = 0;
       loader_info->textsize = 0x7fffffff;
       loader_info->dataorg = 0;
       loader_info->datasize = 0x7fffffff;
   }
   loadcnt = 1;
}

/*
 * printloaderinfo - dump the loader table information
 */

printloaderinfo()
{
   int ldcnt;
   Boolean lastentry = false;

   for (ldcnt = 0; !lastentry; ldcnt++) {
       (*rpt_output)(stdout, "Entry %d:\n",ldcnt+1);
       (*rpt_output)(stdout, "   Object name: %s\n", fd_info[ldcnt].pathname);
       if ((fd_info[ldcnt].membername != nil) &&
           (fd_info[ldcnt].membername[0] != '\0'))
           (*rpt_output)(stdout, "   Member name: %s\n",
						    fd_info[ldcnt].membername);
       (*rpt_output)(stdout, "   Text origin:     0x%x\n",
						   loader_info[ldcnt].textorg);
       (*rpt_output)(stdout, "   Text length:     0x%x\n",
						  loader_info[ldcnt].textsize);
       (*rpt_output)(stdout, "   Data origin:     0x%x\n",
						  loader_info[ldcnt].dataorg);
       (*rpt_output)(stdout, "   Data length:     0x%x\n",
						  loader_info[ldcnt].datasize);
       (*rpt_output)(stdout, "   File descriptor: 0x%x\n",
						 loader_info[ldcnt].ldinfo_fd);
	
       if (loader_info[ldcnt].ldinfo_next == 0)
	   lastentry = true;
       (*rpt_output)(stdout, "\n");
   }
}

public getprog (pid,exec)        /* determine prog name from pid */
  int pid;                	 /* attach or exec */
  boolean exec;
{
   int errcond;
   int rc;
   FILE *f;
   char syscall[512];
   char *prognm;
   int status = 0;

   attach = (boolean)(!exec);                 /* set attach to program flag  */
   norun = true;                       /* run and re-run not allowed  */
   coredump = false;		       /* Ignore core dumps in this case */
   if (attach) {
      if (reattach)                    /* if doing reattach instead */
      {
        status = 0x7f;                               /* process stopped */
        errcond = ptrace(REATT, pid, 0, 0, 0);       /* reattach to program   */
      } else
        errcond = ptrace(ATTACH, pid, 0, 0, 0);      /* attach to program   */
      if (errcond < 0)                              /* if error on attach  */
         fatal( catgets(scmc_catd, MS_process, MSG_277,
					 "could not attach to pid %d\n"), pid);
      else {
	 (*rpt_output)(stdout,  catgets(scmc_catd, MS_process, MSG_278,
				"Waiting to attach to process %d ...\n"), pid);
	 ptraced(pid);
	 while (status == 0)
	 	pwait(pid, &status);
      }
   }
   else {
      (*rpt_output)(stdout,  catgets(scmc_catd, MS_process, MSG_279,
				       "Attaching to process from exec...\n"));
   }
   process = &pbuf;
   process->pid = pid;
   getldrinfo(false);
#if defined (CMA_THREAD) ||  defined(K_THREADS)
   threads(th_get_info, nil);    /* get info about any existing threads */
   if(lib_type == KERNEL_THREAD) {
   /* update also $ci $mi and $ai */
      condition_k(th_get_info,nil);
      attribute_k(th_get_info,nil);
      mutex_k(th_get_info,nil);
   }
#endif /* CMA_THREAD || K_THREADS*/
   openfiles(false);
   isstopped = true;
}

/*
 * Routines to get at process information from outside this module.
 */

public Word reg (n)
int n;
{
    register Process p;
    struct user u;

    p = process;
    if ((p->valid[regword(n)] & regbit(n)) == 0) {
       if (n < NGREG) {	/* General purpose register */
	   p->reg[n] = readreg(p, n);
       } else if (n < (NGREG+NSYS)) { /* System register */
	   p->reg[n] = readreg(p, REGTOSYS(n));
#if defined(_IBMRT) && !defined(KDBXRT)
	   if (REGTOSYS(n) == CR) {
	       p->reg[n] = (p->reg[n] >> 16) & 0xffff;
	   } else if (REGTOSYS(n) == ICS) {
	       p->reg[n] &= 0xffff;
	   }
#endif
       } else { /* Floating point register */
	   readflreg(p, FREGTOSYS(n - NGREG - NSYS),p->freg[n - NGREG - NSYS]);
	   return (Word) p->freg[n - NGREG - NSYS];
       }
       p->valid[regword(n)] |= regbit(n);
    }
    return p->reg[n];
}

/*
 * Routines to get at floating point registers.
 */

public double fpregval (n)
int n;
{
    register Process p;
    int regno;

    p = process;
    regno = NGREG+NSYS+n;		/* Overall register number. */
    if ((p->valid[regword(regno)] & regbit(regno)) == 0) {
        readflreg(p, FREGTOSYS(n), p->freg[n]);
        p->valid[regword(regno)] |= regbit(regno);
    }
    return p->freg[n];
}

public setreg (n, w)
int n;
Word w;
{
    register Process p;
    static union { Word fregp[2];
		   double fvalue;
		} fpvalue;
    int x = n;

    p = process;

    if (n < 0) {
	n *= -1;
	fpvalue.fregp[1] = w;
	p->freg[n-NGREG-NSYS] = fpvalue.fvalue;
    }
    else if (n < NGREG) {
		p->reg[n] = w;
    } else if (n < NGREG + NSYS) {
#if defined(_IBMRT) && !defined(KDBXRT)
    	if (n == CR)
		p->reg[n] = w & 0xffff;
	else
#endif
		p->reg[n] = w;
    }
    else {
	fpvalue.fregp[0] = w;
    }
#if defined (CMA_THREAD) || defined (K_THREADS)
    if (running_thread != nil && current_thread != nil &&
        running_thread != current_thread && (lib_type != KERNEL_THREAD)) {
        /* If we have CMA thread and the current thread is not the running   */
        /* thread, we need to write into the stack and not set the dirty bit */
        /* No only do we need to modify reg. values of Process, we need to   */
        /* write the values into the sigcontext structure stored on stack.   */
        setThreadreg(x, w);                      /* x is original value of n */
        process->valid[regword(n)] |= regbit(n);
    } else {
        /* if we have kernel threads (and libpthreads 1:1) the registers are  */
        /* updated by setregs when the application is resume or when the     */
        /* user execute the subcommand : thread current n                    */
#endif /* CMA_THREAD || K_THREADS */
        process->dirty[regword(n)] |= regbit(n);
        process->valid[regword(n)] |= regbit(n);
#if defined (CMA_THREAD) ||  defined (K_THREADS)
    }
#endif /* CMA_THREAD || K_THREADS */
}

/*
 * Start up a new process by forking and exec-ing the
 * given argument list, returning when the process is loaded
 * and ready to execute.  The PROCESS information (pointed to
 * by the first argument) is appropriately filled.
 *
 * If the given PROCESS structure is associated with an already running
 * process, we terminate it.
 */

pstart (p, argv, infile, outfile)
Process p;
String argv[];
String infile;
String outfile;
{
    int status;

    if (norun) {
	getinfo(p, STOPPED);
	ptraced(p->pid);
	return;
    }
    if (p->pid != 0) {
	pterm(p);
	cacheflush(p);
    }
    fflush(stdout);

#ifdef KDBX
    p->pid = kdbx_fork();
#else
    p->pid = fork();
#endif
    if (p->pid == -1) {
	panic( catgets(scmc_catd, MS_process, MSG_280, "cannot fork"));
    }
    if (ischild(p->pid)) {
	nocatcherrs();
	traceme();
	/* Reset stdio if Screen Command */
        if (ScrUsed)
        {
           close(0);
           dup(debuggee_fd);
           close(1);
           dup(debuggee_fd);
           close(2);
           dup(debuggee_fd);
        }
	/* Check for redirection AFTER reset of stdio */
	if (infile != nil) {
	    infrom(infile);
	}
	if ((outfile != nil) || (errfile != nil)) {
	    outto(outfile, errfile);
	}
	execv(argv[0], argv);
	_exit(1);
    }
    pwait(p->pid, &status);
    getinfo(p, status);
    if (p->status != STOPPED) {
	beginerrmsg();
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_process, MSG_281,
				     "warning: cannot execute %s\n"), argv[0]);
	noexec = true;
	text_reloc = data_reloc = 0;
	defvar(identname("$text",true),nil);
	textaddrs = true;
	objfile_readin();
	just_started = true;
    } else {
	just_started = true;
	ptraced(p->pid);
	noexec = false;
    }
}

/*
 * Terminate a ptrace'd process.
 */

public pterm (p)
Process p;
{
    int status;

    if (p != nil and p->pid != 0) {
	ptrace(PKILL, p->pid, 0, 0, 0);
	if (!attach)  /* We are not the parent, so we won't get the signal */
	    pwait(p->pid, &status);
	unptraced(p->pid);
    }
}

/*
 * Continue a stopped process.  The first argument points to a Process
 * structure.  Before the process is restarted it's user area is modified
 * according to the values in the structure.  When this routine finishes,
 * the structure has the new values from the process's user area.
 *
 * Pcont terminates when the process stops with a signal pending that
 * is being traced (via psigtrace), or when the process terminates.
 */

pcont (p, signo)
Process p;
int signo;
{
    int s, status;
    boolean skip_load;
    extern boolean loadpending;
    extern boolean lazy;
    extern int last_module;
    extern refreshbp();

    if (p->pid == 0) {
	error( catgets(scmc_catd, MS_process, MSG_282,
						     "program is not active"));
	return -1;
    }
    s = signo;
    do {
        skip_load = false;
	setinfo(p, s);
	if (traceexec) {
	    (*rpt_output)(stdout, "!! pcont from 0x%x with signal %d (%d)\n",
		reg(SYSREGNO(PROGCTR)), s, p->signo);
	}
	sigs_off();
	/* We don't need to tell the OS what it already knows... 
	if (ptrace(CONT, p->pid, p->reg[SYSREGNO(PROGCTR)], p->signo) < 0) {
	*/
#if defined ( K_THREADS) && !defined (KDBX)
        /* resume a list of threads : PTT_CONTINUE */
        thread_k_cont(p); /* to see if the running_thread is held */
#else
	if (traceexec) {
	    (*rpt_output)(stdout, "!! ptrace(%d,0x%x,%d,%d)\n",
		CONT, p->pid, 1, p->signo, 0);
	}
	if (ptrace(CONT, p->pid, 1, p->signo, 0) < 0) {
	    panic( catgets(scmc_catd, MS_process, MSG_285,
				"error %d trying to continue process"), errno);
	}
#endif /* K_THREADS && !KDBX */
	(*rpt_executing)( );
	pwait(p->pid, &status);
	if (traceexec) {
	    (*rpt_output)(stdout, "!! wait status = 0x%x, errno = %d\n",
					 status, errno);
	}
	action_mask |= EXECUTION;
	action_mask &= ~CONTEXT_CHANGE;
	action_mask &= ~LISTING;
	action_mask &= ~ELISTING;
	sigs_on();
	getinfo(p, status);

        /*  if the user has set "debug 3" (traceexec is true) and
              we are not going to stop for this signal  */
	if (traceexec && (p->status == STOPPED) 
         && !istraced(p) && !(p->is_bp)) {
            /*  print debug message  */
	    (*rpt_output)(stdout, "!! ignored signal %d at 0x%x\n",
		p->signo, reg(SYSREGNO(PROGCTR)));
	    fflush(stdout);
	}
	s = p->signo;
        /* In case user don't care about load() and unload()... */
        if (loadpending and varIsSet("$ignoreload")) {
           s = 0;                       /* reset the cont signal to zero */
           skip_load = true;            /* go thru the do loop once more */
           loadpending = false;         /* reset loadpending             */
           refreshbp();                 /* refresh breakpoints after load()*/
	   if (lazy)			/* need to reread string and debug */
	      last_module = -2;		/* table after load or unload	   */
        }

    /*  while we want to ignore this signal and continue execution  */
    } while ((p->status == STOPPED && !istraced(p) && !(p->is_bp))
              || skip_load);

    if (traceexec) {
	(*rpt_output)(stdout, "!! pcont to 0x%x on signal %d\n",
					  reg(SYSREGNO(PROGCTR)), p->signo);
	fflush(stdout);
    }
}

/*
 * Return from execution when the given signal is pending.
 */

public psigtrace (p, sig, sw)
Process p;
int sig;
Boolean sw;
{
    if (sw) {
	if (sig <= (8*sizeof(Word)))
	    p->sigset[0] |= setrep(sig);
	else
	    p->sigset[1] |= setrep(sig-32);
    } else {
	if (sig <= (8*sizeof(Word)))
	    p->sigset[0] &= ~setrep(sig);
	else
	    p->sigset[1] &= ~setrep(sig-32);
    }
}

/*
 * Set up what signals we want to trace.
 */

setsigtrace ()
{
    register int i;
    register Process p;

    p = process;
    for (i = 1; i <= NSIG; i++) {
	psigtrace(p, i, true);
    }
    psigtrace(p, SIGHUP, false);
    psigtrace(p, SIGKILL, false);
    psigtrace(p, SIGALRM, false);
    psigtrace(p, SIGCLD, false);
#if defined (CMA_THREAD) ||  defined (_THREADS)
    psigtrace(p, SIGVIRT, false);
#endif /* CMA_THREAD */
}

/*
 * Don't catch any signals.
 * Particularly useful when letting a process finish uninhibited.
 */

public unsetsigtraces (p)
Process p;
{
    p->sigset[0] = 0;
    p->sigset[1] = 0;
}

/*
 * This interrupt handler is used if dbx gets interrupt (SIGINT)
 * while it is waiting for the child (debuggee) to come back after
 * a "cont" ,"step", or "next", etc...
 * Normally dbx ignores all signals during this wait, but we need
 * to handle SIGINT specially so that there's a way to interrupt
 * dbx in cases of continue after attaching to a process. In which
 * case, there might not be a tty for the child to accept interrupts
 * (control-c).
 *
 * We do this by interrupting the child debuggee which in turn
 * will stop dbx's wait.
 */
private void intrchild()
{
   signal(SIGINT, intrchild);           /* reset the signal handler   */
   kill(process->pid, SIGINT);          /* stop debuggee using SIGINT */
}

/*
 * Turn off attention to signals not being caught.
 */

private Voidfunc sigfunc[NSIG];

public sigs_off ()
{
    register int i;

    for (i = FIRSTSIG; i < LASTSIG; i++) {
        if (i == SIGINT) {
            sigfunc[i] = signal(i, intrchild);
        } else if (i != SIGKILL) {
	    sigfunc[i] = signal(i, SIG_IGN);
	}
    }
}

/*
 * Turn back on attention to signals.
 */

public sigs_on ()
{
    register int i;

    for (i = FIRSTSIG; i < LASTSIG; i++) {
	if (i != SIGKILL) {
	    signal(i, sigfunc[i]);
	}
    }
}
#ifndef  PT_READ_U
/*
 * NAME: getheapsize
 *
 * FUNCTION: Access to procsinfo to get heap size
 *
 * PARAMETERS:
 *      p      - process id for current process
 *
 * NOTES: Used by dpi_check_heap_and_stacks
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none
 *
 * RETURNS:  end of heap (if successful)
 *           0           (if malloc or getprocs fails)
 */

public uint getheapsize (pid_t pid)
{
    uint heap_size=0;

    if (pprocsinfo == NULL)
    {
        pprocsinfo=(procsinfo *) malloc ((size_t)(sizeof(procsinfo)));
    }
    if (pprocsinfo == NULL)
    {
        panic( catgets(scmc_catd, MS_pthread, MSG_773, "malloc error"));
    }
    else
    {
        if (getprocs(pprocsinfo, sizeof(procsinfo), NULL,
                     sizeof(struct fdsinfo), &pid, 1) < 0)
        {
           panic( catgets(scmc_catd,MS_pthread, MSG_770,
                  "could not access to procsinfo errno %d"), errno);
        }
        else
        {
           heap_size = (uint)pprocsinfo->pi_dsize;
        }
    }
    return(heap_size);
}
#endif

#ifdef _THREADS

/*
 * NAME: getsigstatus
 *
 * FUNCTION: Access to procsinfo to get sigstatus
 *
 * PARAMETERS:
 *      p      - structure Process to read
 *      sig    - signal
 *
 * NOTES: Used by getinfo and usignal
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  return address associated with the signal
 */

public Address getsigstatus (p,sig)
register Process p;
int sig;
{
    pid_t index;

    if (pprocsinfo == NULL)
        pprocsinfo=(procsinfo *) malloc ((size_t)(sizeof(procsinfo)));
    if (pprocsinfo == NULL)
        panic( catgets(scmc_catd, MS_pthread, MSG_773, "malloc error"));

    index = p->pid;
    if (getprocs(pprocsinfo, sizeof(procsinfo), NULL,
                 sizeof(struct fdsinfo), &index, 1) < 0) {
       panic( catgets(scmc_catd,MS_pthread, MSG_770,
              "could not access to procsinfo errno %d"), errno);
    }

    return(pprocsinfo->pi_signal[sig]);
}

/*
 * NAME: gettid
 *
 * FUNCTION: Access to thrdsinfo : access to the running_thread
 *
 * PARAMETERS:
 *      p      - structure Process
 *
 * NOTES: Used by getinfo
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: update reg in Process structure p.
 *                  and sigcode
 *
 * RETURNS: thread_id of running_thread
 */


public tid_t gettid (p)
register Process p;
{
    register int r,n,i;
    int index = 0;
    thrdsinfo  *ptr;
    /* first call to know the number of threads */
    r=getthrds(p->pid,NULL,sizeof(thrdsinfo),&index,1000000);
    if (r < 0) {
       panic( catgets(scmc_catd, MS_pthread, MSG_771,
              "could not access to thrdsinfo errno %d"), errno);
    }
    if ((r > nb_k_threads || pthrdsinfo == NULL) && (r > 0)) {
       if (pthrdsinfo) free(pthrdsinfo);
       pthrdsinfo=(thrdsinfo *) malloc ((size_t)(r * sizeof(thrdsinfo)));
       if ( pthrdsinfo == NULL)
             panic( catgets(scmc_catd, MS_pthread, MSG_773, "malloc error"));
    }
    /* second call to read the threads information */
    nb_k_threads = r;
    index = 0;
    n = getthrds(p->pid,pthrdsinfo,sizeof(thrdsinfo),&index,r);
    if (n < 0) {
       panic( catgets(scmc_catd, MS_pthread, MSG_771,
              "could not access to thrdsinfo errno %d"), errno);
    }

    /* search the thread who has stopped the process */
    /* the thread with TTRCSIG and cursig = signal received on wait() */
    for (i = 0; i < nb_k_threads; i++) {
      ptr=&pthrdsinfo[i];
      if ((ptr->ti_flag & TTRCSIG) && ((ptr->ti_cursig == p->signo) ||
          (p->signo == 0)))
         break;
    }

    /* update registers  */

    if (i != nb_k_threads) {
        p->tid = ptr->ti_tid;
        readgprs(p,&p->reg[0]);
        readsprs(p,&p->reg[NGREG]);
        readfprs(p,&p->freg[0]);

        for (i=0; i < NGREG+NSYS+MAXFREG; i++) {
            p->valid[regword(i)] |= regbit(i);
            p->dirty[regword(i)] = 0;
        }
    }
    else {
       panic( catgets(scmc_catd, MS_pthread, MSG_772,
              "thread with signal not found "));

    }
    p->sigcode = ptr->ti_code;
    return(ptr->ti_tid);
}
#endif /* _THREADS*/


/*
 * Get process information from user area.
 */

public getinfo (p, status)
register Process p;
register int status;
{
    register int i;
    struct user uarea;
    Bpinst instruction;

    debugger_setpgrp();
    p->signo = (status&0177);
    p->exitval = ((status >> 8)&0377);
    if (p->signo != STOPPED) {
	p->status = FINISHED;
	p->pid = 0;
	p->reg[SYSREGNO(PROGCTR)] = 0;
    } else {
	p->status = p->signo;
	p->signo = p->exitval;
	p->exitval = 0;
#if defined ( _THREADS) && !defined (KDBX)
        /* update p->sigcode andf registers   */
        p->tid = gettid(p);
#else
/* p->sigcode = 0 because kernel never inits u_code. It is and
   has always been 0. u_code is not a part of the u-block in 4.1
   it is part of thread. It is still not used.
*/
        p->sigcode = 0;

/*	p->sigcode = ptrace(U_READ, p->pid, U_OFSET(urea,u_code), 0); */
	for (i = 0; i <= regword(NGREG+NSYS+MAXFREG+1-1); i++) {
	    p->valid[i] = 0;
	    p->dirty[i] = 0;
	}
#endif /*_THREADS */
	pc = reg(SYSREGNO(PROGCTR));
	savetty(stdout, &p->ttyinfo);
	savetty(stdin, &p->ttyinfo_in);
#if defined ( _THREADS) && !defined (KDBX)
        p->sigstatus = getsigstatus(p,p->signo);        /* update p->sigstat */
        /* in case of trace subcommand curframe has to be updated */
        /* if the thread changes, the stack changes so  we have to update
           frames */
        if (tid_running_thread && (p->tid != tid_running_thread))
        	setcurfunc(whatblock(pc));
        tid_running_thread = p->tid;
#else
#define	U_SIG U_signal
/*	p->sigstatus = ptrace(U_READ, p->pid, U_OFSET(u,u_signal[p->signo]),0); */
	p->sigstatus = ptrace(U_READ, p->pid, U_OFSET(uarea,U_SIG[p->signo]),0);
#endif /* _THREADS */
	/* make sure we get a valid address from ptrace. */
	if (p->sigstatus == -1)
	   p->sigstatus = 0;

        /*  if we are stopped because of a trap signal (5)  */
        if (p->signo == SIGTRAP)
        {
          /*  if this is an active dbx breakpoint  */
          if (isdbxbp((void *) -1))
          {
            p->is_bp = true;
          }
          else
          {
            /*  read the instruction at this address  */
            iread (&instruction, pc, sizeof (instruction));

            /*  treat SIGTRAP signals that were not generated
                  by trap instructions as dbx breakpoints  */
            if (!is_trap_instruction(instruction))
              p->is_bp = true;
 
#ifdef _THREADS
            /* if there are several threads on the same code, we can  */
            /* receive several SIGTRAP, but the break-point is reset  */
            if (nb_k_threads >1)
              p->is_bp = true;
#endif
          }
        }
    }
#if defined (CMA_THREAD) || defined (K_THREADS)
    /* get info about existing threads if any... */
    threads(th_get_info, nil);
    if(lib_type == KERNEL_THREAD) {
    /* update also  $ci $mi and $ai */
       condition_k(th_get_info,nil);
       attribute_k(th_get_info,nil);
       mutex_k(th_get_info,nil);
    }
#endif /* CMA_THREAD || K_THREADS */
    restoretty(stdout, &ttyinfo);
    restoretty(stdin, &ttyinfo_in);
}

/* 
 * Get the current address
 */

getpc(iar)
Word *iar;
{
    *iar = reg(SYSREGNO(IAR));
}

/*
 * Set process's user area information from given process structure.
 */

public setinfo (p, signo)
register Process p;
int signo;
{
    if (signo == DEFSIG) {
        /*  if we stopped because of a traced signal or a breakpoint
              and there is no signal handler
              or we are blocking signals  */
        /*  NOTE : if we stopped because of a breakpoint, 
              sigstatus is set to SIG_IGN.  See isbperr */

	if (((istraced(p) || p->is_bp) 
         && (p->sigstatus == SIG_DFL || p->sigstatus == SIG_IGN)) 
         || varIsSet("$sigblock")) {
	    p->signo = 0;
	}
    } else {
	p->signo = signo;
    }
    setregs(p);
    restoretty(stdout, &(p->ttyinfo));
    restoretty(stdin, &(p->ttyinfo_in));
    debuggee_setpgrp();
#if defined (CMA_THREAD) ||  defined (K_THREADS)
   /* reset current_thread to nil before continuing execution */
   current_thread = nil;
#endif /* CMA_THREAD || K_THREADS */
   p->is_bp = false;
}

/*
 * Set process's user area information from given process structure.
 */

public setregs (p)
register Process p;
{
    register int i;
    register unsigned *w, mask;
    Boolean isdirty = false;

    for (i = 0; (i <= NREGBITS) && (!isdirty); i++)
	isdirty = (Boolean) p->dirty[i];
	
    if (!isdirty)  /* Nothing to do here! */
	return;

    w = &p->dirty[0];
    mask = 1;
    for (i = 0; i < NGREG; i++) {
	if ((*w & mask) != 0) {
#ifdef _THREADS
            /* there is no ptrace to write one register */
            writegprs(p, &p->reg[0]);
            break;
#else
	    writereg(p, i, p->reg[i]);
#endif /* _THREADS */
        }
	mask <<= 1;
	if (mask == 0) {
	    *w = 0;
	    ++w;
	    mask = 1;
        }
    }
    for (i = NGREG; i < NGREG+NSYS; i++) {
	if ((*w & mask) != 0) {
#ifdef _THREADS
            /* there is no ptrace to write one register */
            writesprs(p, &p->reg[NGREG]);
            break;
#else
	    writereg(p, REGTOSYS(i), p->reg[i]);
#endif /* _THREADS */
        }
	mask <<= 1;
	if (mask == 0) {
	    *w = 0;
	    ++w;
	    mask = 1;
        }
    }
    for (i = 0; i < fpregs; i++) {
	if ((*w & mask) != 0) {

#ifdef _THREADS
            /* there is no ptrace to write one register */
            writefprs(p, &p->freg[0]);
            break;
#else
	    writeflreg(p, FREGTOSYS(i), p->freg[i]);
#endif /* _THREADS */
        }
	mask <<= 1;
	if (mask == 0) {
	    *w = 0;
	    ++w;
	    mask = 1;
        }
    }
}
/*
 * Return the address associated with the current signal.
 * (Plus two since the address points to the beginning of a procedure).
 */

public Address usignal (p)
Process p;
{
    Address r;
    extern int contsig;

    /* if sigblock is on, no signal will go to the program. */
    /* So it should not jump to the signal handler.         */
    if (varIsSet("$sigblock")) {
	r = 0;
    } else {
        r = p->sigstatus;
    }
    /* If user continued program with a signal, we need to        */
    /* locate signal handler (if any) of signal provided by user. */
    /* This needs to be done no matter if $sigblock is set or not.*/
    if (contsig != DEFSIG) {
#ifdef _THREADS
       r = getsigstatus(p, contsig);
#else
       struct user uarea;
       r = ptrace(U_READ, p->pid, U_OFSET(uarea,u_signal[contsig]),0);
#endif /* _THREADS */
    }
    if (r != 0 and r != 1) {
	r += FUNCOFFSET;
    }
    return r;
}

/*
 * Structure for reading and writing by words, but dealing with bytes.
 */

typedef union {
    Word pword;
    Byte pbyte[sizeof(Word)];
} Pword;

/*
 * Read (write) from (to) the process' address space.
 * We must deal with ptrace's inability to look anywhere other
 * than at a word boundary.
 */

pio (p, op, seg, buff, addr, nbytes)
Process p;
PioOp op;
PioSeg seg;
char *buff;
Address addr;
int nbytes;
{
    register int i;
    register Address newaddr;
    register char *cp;
    char *bufend;
    Pword w;
    Address wordaddr;
    int byteoff;

    if (p->status != STOPPED) {
	error( catgets(scmc_catd, MS_process, MSG_282,
						     "program is not active"));
    }
    cp = buff;
    newaddr = addr;
    wordaddr = (newaddr&WMASK);
    if (wordaddr != newaddr) {
	w.pword = fetch(p, seg, wordaddr);
	for (i = newaddr - wordaddr; i < sizeof(Word) and nbytes > 0; i++) {
	    if (op == PREAD) {
		*cp++ = w.pbyte[i];
	    } else {
		w.pbyte[i] = *cp++;
	    }
	    nbytes--;
	}
	if (op == PWRITE) {
	    store(p, seg, wordaddr, w.pword);
	}
	newaddr = wordaddr + sizeof(Word);
    }
    byteoff = (nbytes&(~WMASK));
    nbytes -= byteoff;
    bufend = cp + nbytes;
    while (cp < bufend) {
	if (op == PREAD) {
	    w.pword = fetch(p, seg, newaddr);
	    for (i = 0; i < sizeof(Word); i++) {
		*cp++ = w.pbyte[i];
	    }
	} else {
	    for (i = 0; i < sizeof(Word); i++) {
		w.pbyte[i] = *cp++;
	    }
	    store(p, seg, newaddr, w.pword);
	}
	newaddr += sizeof(Word);
    }
    if (byteoff > 0) {
	w.pword = fetch(p, seg, newaddr);
	for (i = 0; i < byteoff; i++) {
	    if (op == PREAD) {
		*cp++ = w.pbyte[i];
	    } else {
		w.pbyte[i] = *cp++;
	    }
	}
	if (op == PWRITE) {
	    store(p, seg, newaddr, w.pword);
	}
    }
}

/*
 * Get a word from a process at the given address.
 * The address is assumed to be on a word boundary.
 *
 * A simple cache scheme is used to avoid redundant ptrace calls
 * to the instruction space since it is assumed to be pure.
 *
 * It is necessary to use a write-through scheme so that
 * breakpoints right next to each other don't interfere.
 */

private int nfetchs = 0, nreads = 0, nwrites = 0;

#define CheckTextAddr(addr) \
{ \
    if (addr >= 0x20000000) { \
	warning( "bad text addr 0x%x", addr); \
	return -1; \
    } \
}

#define CheckDataAddr(addr) \
{ \
    if (addr >= 0x20000000) { \
	warning( "bad data addr 0x%x", addr); \
	return -1; \
    } \
}

private Word fetch (p, seg, addr)
Process p;
PioSeg seg;
register int addr;
{
    register CacheWord *wp;
    register Word w;

    switch (seg) {
	case TEXTSEGM:
	    ++nfetchs;
	    wp = &p->word[cachehash(addr)];
	    if (addr == 0 or wp->addr != addr) {
		/* CheckTextAddr(addr); */
		++nreads;
		errno = 0;
		w = ptrace(I_READ, p->pid, addr, 0, 0);
		if ((w == (Address) -1) && (errno == EIO)) {
		if (traceback_table)	
		     warning( catgets(scmc_catd, MS_process, MSG_301,
	                     "Invalid trace table.  Unreadable instruction \
at address 0x%x"), addr);
		else
		     error( catgets(scmc_catd, MS_process, MSG_291,
			   "Unreadable instruction at address 0x%x"), addr);
		}
		wp->addr = addr;
		wp->val = w;
	    } else {
		w = wp->val;
	    }
	    break;

	case DATASEGM:
	default:
	    /* CheckDataAddr(addr); */
	    w = ptrace(D_READ, p->pid, addr, 0, 0);
/*
 *	Let the user decide if he wants bad address fetches to abort reading.
	    if ((w == -1) && (errno == EIO))
		error( "Unreadable data at 0x%x", addr);
*/
	    break;
    }
    return w;
}

/*
 * Put a word into the process' address space at the given address.
 * The address is assumed to be on a word boundary.
 */

private store (p, seg, addr, data)
Process p;
PioSeg seg;
int addr;
Word data;
{
    register CacheWord *wp;
    int w;

    switch (seg) {
	case TEXTSEGM:
	    /* CheckTextAddr(addr); */
	    ++nwrites;
	    wp = &p->word[cachehash(addr)];
	    wp->addr = addr;
	    wp->val = data;
	    errno = 0;
	    w = ptrace(I_WRITE, p->pid, addr, data, 0);
	    if ((w == -1) && (errno == EIO))
		error( catgets(scmc_catd, MS_process, MSG_294,
			  "store: could not write instruction at 0x%x"), addr);
	    break;

	case DATASEGM:
	default:
	    /* CheckDataAddr(addr); */

            /* User can use assign (dwrite) for assignment to TEXT address */
            /* Update cache if we have a value in cache for that address   */
            /* Don't want to cache it all the time for all addresses.      */
            wp = &p->word[cachehash(addr)];
            if (wp->addr == addr) {
               wp->val = data;
            }
	    errno = 0;
	    w = ptrace(D_WRITE, p->pid, addr, data, 0);
	    if ((w == -1) && (errno == EIO))
		error( catgets(scmc_catd, MS_process, MSG_295,
				 "store: could not write data at 0x%x"), addr);
	    break;
/*
	The impossible.
	default:
	    panic( "store: bad seg %d", seg);
*/
    }
}

/*
 * Flush the instruction cache associated with a process.
 */

public cacheflush (p)
Process p;
{
    bset0(p->word, sizeof(p->word));
}

public printptraceinfo ()
{
    (*rpt_output)(stdout, "%d fetchs, %d reads, %d writes\n",
						nfetchs, nreads, nwrites);
}

/*
 * Redirect input.
 * Assuming this is called from a child, we should be careful to avoid
 * (possibly) shared standard I/O buffers.
 */

private infrom (filename)
String filename;
{
    Fileid in;

    in = open(filename, 0);
    if (in == -1) {
	write(2, "cannot read ", 11);
	write(2, filename, strlen(filename));
	write(2, "\n", 1);
	_exit(1);
    }
    fswap(0, in);
}

/*
 * Redirect standard output.
 * Same assumptions as for "infrom" above.
 */

extern enum redirect { CREATE, APPEND, OVERWRITE } stdout_mode;
extern enum errordirect { STANDARD, SAME_AS_OUT, CREAT, APPND } stderr_mode;
private outto (outfilename, errfilename)
String outfilename, errfilename;
{
    Fileid out, err;
    int oflag, eflag;

    if (!strcmp(objname,outfilename) || !strcmp(errfilename,objname)) {
	write(2, "Can't write over running object file!\n", 38);
        erecover();
    }
    if (outfilename != nil) {
       if (stdout_mode == APPEND)
          oflag = O_RDWR | O_CREAT | O_APPEND;
       else
          oflag = O_RDWR | O_CREAT | O_TRUNC;
       out = open(outfilename, oflag, 0644);
       if (out == -1) {
	   write(2, "cannot write ", 12);
	   write(2, outfilename, strlen(outfilename));
	   write(2, "\n", 1);
	   _exit(1);
       }
       fswap(1, out);
       if (stderr_mode == SAME_AS_OUT)
       {
          close(2);
          dup(1);
       }
    } 
    if (errfilename != nil && stderr_mode != SAME_AS_OUT) {
       if (stderr_mode == APPND)
          eflag = O_RDWR | O_CREAT | O_APPEND;
       else
          eflag = O_RDWR | O_CREAT | O_TRUNC;
       err = open(errfilename, eflag, 0644);
       if (err == -1) {
	      write(2, "cannot write ", 12);
	      write(2, errfilename, strlen(errfilename));
	      write(2, "\n", 1);
	      _exit(1);
       }
       fswap(2, err);
    }
}

reset_pgrp_info()
{
  called_debuggee_setpgrp = false;
  
  if (!job_control)
    return;			/* no job control, no tcsetpgrp! */

  save_sig = signal(SIGTTOU, SIG_IGN);		/* set up child dbx's pgrp */
  tcsetpgrp(0, debugger_pgrp);			/* set to debugger's pgrp */
  signal(SIGTTOU, save_sig);
}

reset_debuggee_fd(fd)
     int fd;
{
  if (debuggee_fd == 0)		/* only reset this once */
    debuggee_fd = fd;		/* debugged-parent's STDIN */
}

/*
 * debugger_setpgrp() - Set the terminal's process group (pgrp) to the
 *	debugger's pgrp after saving the debuggee's pgrp.
 */
debugger_setpgrp()
{
  pid_t tcpgrp;

  if (!job_control)
    return;			/* no job control, no tcsetpgrp! */

  if (called_debuggee_setpgrp)		/* call in pairs only */
    called_debuggee_setpgrp = false;
  else
    return;

  save_sig = signal(SIGTTOU, SIG_IGN);
  tcpgrp = tcgetpgrp(debuggee_fd); /* find out current owner */

  /* don't set if in background */
  if ((tcpgrp != getppid()) || (multproc != off)
	/* don't set if in background */
	&& tcpgrp != -1) {
    debuggee_pgrp = tcpgrp;	/* save debuggee's pgrp */
    if (debuggee_pgrp != debugger_pgrp) /* don't set it to what it is now */
      tcsetpgrp(debuggee_fd, debugger_pgrp); /* set to debugger's pgrp */
  }
  signal(SIGTTOU, save_sig);
}

/*
 * debuggee_setpgrp() - Set the terminal's process group (pgrp) to the
 *	debuggee's pgrp.
 */
debuggee_setpgrp()
{
  if (!job_control)
    return;			/* no job control, no tcsetpgrp! */

  if (called_debuggee_setpgrp)		/* don't call it twice */
    return;
  else
    called_debuggee_setpgrp = true;

  if (debuggee_pgrp > 0) {
    save_sig = signal(SIGTTOU, SIG_IGN);
    if (debugger_pgrp != debuggee_pgrp)
      tcsetpgrp(debuggee_fd, debuggee_pgrp);	/* set to debuggee's pgrp */
    signal(SIGTTOU, save_sig);
  }
}

