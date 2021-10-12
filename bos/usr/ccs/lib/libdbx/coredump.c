static char sccsid[] = "@(#)35	1.19.1.13  src/bos/usr/ccs/lib/libdbx/coredump.c, libdbx, bos41B, 9504A 1/5/95 18:02:33";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: copyregs, coredump_close, coredump_getkerinfo, coredump_readdata,
 *	      coredump_readtext, coredump_reloc, coredump_set_maps,
 *	      coredump_xreadin, getpcb, readfromfile, vmap, vreadfromfile
 *            find_region, find_data, find_thread
 *
 * ORIGINS: 26, 27, 83
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
 * Deal with the core dump anachronism.
 */

#include "defs.h"
#include "machine.h"
#include "coredump.h"
#include "object.h"
#include "main.h"
#include "process.h"
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/termio.h> /* This is just a wild guess */
#include <sys/user.h>
/* ldfcn.h and fcntl.h share FREAD and FWRITE */
#undef FREAD
#undef FWRITE
#ifdef _THREADS
#include <sys/uthread.h>
#include "k_thread.h"
#endif /* _THREADS */
#include <sys/core.h>
#include <sys/pseg.h>
#include <sys/reg.h>
#include <a.out.h>
#include "aoutdefs.h"

#include <filehdr.h>
#include <ldfcn.h>
#include <sys/errno.h>


typedef struct {
    Address begin;
    Address end;
    Address seekaddr;
} Map;

extern LDFILE *ld_ptr;

private Map stkmap, datamap, *txtmap;
private LDFILE *objfile;
private FILHDR hdr;

private unsigned long txtsiz, datsiz, stksiz, datbas;
public  unsigned coresize;	/* Length of core file itself */
public Boolean processed_partial_core = false;
extern String corename; /* Name of core file. */
extern int loadcnt;
extern LDFILE **ldentry;
#define UTOP 	TOP(STACKSEG)+1
#ifdef _THREADS
/* the core file contains :                                          */
/*                        core header (with mst of faulting thread   */
/*                        msts of others threads                     */
/*                        ldinfo                                     */
/*                        default user stack                         */
/* if fullcore                                                       */
/*                        vm_info structures                         */
/*                        user data                                  */
/*                        memory mapped regions                      */
/*                                                                   */

int regioncnt;                 /* number of anonymously mapped areas */
int  mstcnt;                   /* number of msts in the core file    */
typedef struct vm_info vm_info_st; /* information of vm regions      */
vm_info_st *region_ptr;            /* pointer on vm_info structures  */
public mstsave_st *mstsave_ptr;    /* pointer on mstsave of threads  */
public mstsave_st *mstsave_current;/* pointer on current mstsave     */
extern int *loader_offset;        /* offset of data region in core  */
#endif /* _THREADS  */

/*
 * Special variables for debugging the kernel.
 */

private integer masterpcbb;
private integer slr;
private struct pte *sbr;
/*private struct pcb pcb;*/
private int mapcount = 0;

public struct core_dump corehdr;
public boolean fullcore = false;
private readfromfile();

private getpcb ()
{
}

public coredump_getkerinfo ()
{
}

public copyregs (savreg, reg, savfps, fpregs)
Word savreg[], reg[];
double savfps[], fpregs[];
{
     int i;
     for  (i = 0; i < NGREG; i++) {
	reg[i] = savreg[i];
     }
     /* System registers */
#ifdef _POWER
#if defined(_THREADS)
     reg[SYSREGNO(IAR)] = mstsave_current->iar;
     reg[SYSREGNO(MSR)] = mstsave_current->msr;
     reg[SYSREGNO(CR)] = mstsave_current->cr;
     reg[SYSREGNO(LR)] = mstsave_current->lr;
     reg[SYSREGNO(CTR)] = mstsave_current->ctr;
     reg[SYSREGNO(XER)] = mstsave_current->xer;
     reg[SYSREGNO(MQ)] = mstsave_current->mq;
     reg[SYSREGNO(TID)] = mstsave_current->tid;
     reg[SYSREGNO(FPSCR)] = mstsave_current->fpscr;
#else  /* _THREADS */
     reg[SYSREGNO(IAR)] = corehdr.c_u.u_save.iar;
     reg[SYSREGNO(MSR)] = corehdr.c_u.u_save.msr;
     reg[SYSREGNO(CR)] = corehdr.c_u.u_save.cr;
     reg[SYSREGNO(LR)] = corehdr.c_u.u_save.lr;
     reg[SYSREGNO(CTR)] = corehdr.c_u.u_save.ctr;
     reg[SYSREGNO(XER)] = corehdr.c_u.u_save.xer;
     reg[SYSREGNO(MQ)] = corehdr.c_u.u_save.mq;
     reg[SYSREGNO(TID)] = corehdr.c_u.u_save.tid;
     reg[SYSREGNO(FPSCR)] = corehdr.c_u.u_save.fpscr;
#endif /* _THREADS */
#else
     reg[SYSREGNO(IAR)] = corehdr.c_u.u_save.iar;
     reg[SYSREGNO(CR)] = corehdr.c_u.u_save.cr;
     reg[SYSREGNO(ICS)] = corehdr.c_u.u_save.ics;
     reg[SYSREGNO(MQ)] = corehdr.c_u.u_save.mq;
#endif
     for  (i = 0; i < MAXFREG; i++) {
	fpregs[i] = savfps[i];
     }
}

/*
 * Read the user area information from the core dump.
 */

public coredump_xreadin(reg, freg, signo)
Word reg[];
double freg[];
short *signo;
{
	register struct user *up;
	struct {
	    struct user u;
	} uarea;

	objfile = ldopen(objname, objfile);
	if (objfile == nil) {
	    fatal( catgets(scmc_catd, MS_execute, MSG_225,
				  	       "cannot read \"%s\""), objname);
	}
	if (vaddrs) {
	   mapcount = 1;
	   datamap.begin = 0;
	   datamap.seekaddr = 0;
	   datamap.end = 0xffffffff;
	   stkmap.begin = 0xffffffff;
	   stkmap.end = 0xffffffff;
	   txtmap = (Map *) malloc(mapcount*(sizeof(Map)));
	   txtmap[0].begin = loader_info[0].textorg;
	   txtmap[0].end = loader_info[0].textorg + loader_info[0].textsize;
	   txtmap[0].seekaddr = 0;
	} else {
	   if(corefile == nil) return;
  	   mapcount = loadcnt; 
	   txtmap = (Map *) malloc(mapcount*(sizeof(Map)));
	   datamap.begin = 0xffffffff;
	   datamap.end = 0;
#ifdef _THREADS
           regioncnt = corehdr.c_vmregions;
           mstcnt = corehdr.c_nmsts;
           if (regioncnt) {
              region_ptr =(vm_info_st *) malloc(regioncnt*(sizeof(vm_info_st)));
              if (region_ptr == NULL)
                panic( catgets(scmc_catd, MS_pthread, MSG_773, "malloc error"));
           }

           if (mstcnt) {
              mstsave_ptr =(mstsave_st *) malloc(mstcnt*(sizeof(mstsave_st)));
              if (mstsave_ptr == NULL)
                panic( catgets(scmc_catd, MS_pthread, MSG_773, "malloc error"));
           }


           mstsave_current = &corehdr.c_mst;
#endif /* _THREADS  */

	   /* Check for valid core file */
#ifdef _THREADS
           if ((((Address)corehdr.c_stack) >=
                (sizeof(struct core_dump))) &&
               (((Address)corehdr.c_stack) < coresize))
#else
	   if ((((Address)corehdr.c_stack) >=
		(sizeof(corehdr.c_u) + (Address)(&corehdr.c_u) -
				    (Address)(&corehdr.c_signo))) &&
	       (((Address)corehdr.c_stack) < coresize))
#endif /* _THREADS */
	   {
#ifdef _THREADS
                   fullcore = (corehdr.c_datasize != 0);
#else
	           fullcore = corehdr.c_flag & FULL_CORE;
#endif /* _THREADS */
	   	   processed_partial_core = (Boolean) (!fullcore);
                   up = &(corehdr.c_u);
		   *signo = (short) corehdr.c_signo;

                   if (corehdr.c_flag & CORE_TRUNC)
                   {
	             warning( catgets(scmc_catd, MS_coredump, MSG_551,
"The core file is truncated.  You may need to increase the ulimit\n\
for file and coredump, or free some space on the filesystem."));

                   }
   
                   /* Set up text, data, & stack ranges */
		   stkmap.begin = USRSTACK - (stksiz = corehdr.c_size);
		   stkmap.end = USRSTACK;
		   stkmap.seekaddr = (Address) corehdr.c_stack;

                   /*
		      initialize the register structure to contain the
                      user registers from the core file 
                   */
#ifdef _THREADS
                   /* take the registers in the default thread */
                   copyregs(mstsave_current->gpr, reg,
                                                  mstsave_current->fpr,freg);
                   /* mstsave_ptr : mstsave  structures                   */

                   if (corehdr.c_msts) {
                      fseek(corefile, (long int) corehdr.c_msts, 0);
                      fread((void *) mstsave_ptr, (size_t) 1,
                            (size_t) mstcnt*sizeof(mstsave_st), corefile);
                   }
                   if ( corehdr.c_vmm ) {
                      /*  initialise region_ptr  */
                      /* region_ptr : vm_info structures                     */
                      fseek(corefile, (long int) corehdr.c_vmm, 0);
                      fread((void *) region_ptr, (size_t) 1,
                            (size_t) regioncnt*sizeof(vm_info_st), corefile);

                  }
#else
                   copyregs(corehdr.c_u.u_save.gpr, reg,
                                                  corehdr.c_u.u_save.fpr,freg);
#endif /* _THREADS */


	   } else {
	 	   error( catgets(scmc_catd, MS_coredump, MSG_535,
			  "%s is not a valid core file (discarded)"),
								     corename);
		   corefile = nil;
	   }
	}
}

/*
 * Get data base address, and seek addresses to text sections.
 */
public coredump_set_maps()
{
    int i;

    for (i = 0; i < loadcnt; i++) {
	if (loader_info[i].dataorg < datamap.begin) {
		datamap.begin = loader_info[i].dataorg;
	}
	/* Data sections are loaded together? */
	datamap.end += loader_info[i].datasize;
	ldnsseek(ldentry[i],_TEXT);
	txtmap[i].begin = loader_info[i].textorg;
	txtmap[i].end = loader_info[i].textorg + loader_info[i].textsize;
	txtmap[i].seekaddr = FTELL(ldentry[i]) - OFFSET(ldentry[i]);
    }
    /* Add in beginning offset to data section. */
    datamap.end += datamap.begin;

    /* reset data section org. address */
    if (corehdr.c_flag & CORE_BIGDATA)
        datamap.begin = BDATAORG;
    else
        datamap.begin = DATAORG;

    if (!vaddrs)
#ifdef _THREADS
        datamap.seekaddr = (Address) corehdr.c_data;
        /* no data :  offset = size of core, to have a warning message  */
        if (datamap.seekaddr == 0)
 	    datamap.seekaddr =  coresize;
#else
        datamap.seekaddr = stkmap.seekaddr + corehdr.c_size;
#endif /* _THREADS */

}

/*
 * Adjust the seek addresses to text and data per program.
 */

public coredump_reloc(prog_no, text_scnptr)
int prog_no;
int text_scnptr;
{
    txtmap[prog_no].begin += text_scnptr;
}

public coredump_close()
{
    ldclose(objfile);
}

public coredump_readtext(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    int text_object;
    if (vaddrs) {
       coredump_readdata(buff, addr, nbytes);
    } else {
	if ((text_object = textobj(addr)) != -1) {
	    FSEEK(ldentry[text_object],(long)(txtmap[text_object].seekaddr
				+ addr - txtmap[text_object].begin),0);
	    FREAD((void *) buff, (size_t) nbytes, (size_t) sizeof(Byte),
							ldentry[text_object]);
	} else if ((addr >= stkmap.begin) || (addr <= stkmap.end)) {
	    readfromfile(corefile, addr, buff, nbytes);
    	} else if ((addr >= datamap.begin) && (addr <= datamap.end)) {
       	    if (vaddrs) {
	  	/*v*/readfromfile(corefile, addr, buff, nbytes);
       	    } else {
	  	readfromfile(corefile, addr, buff, nbytes);
       	    }
        } else {
           memset(buff, -1, 4);
        }
    }
}

/*
 * Map a virtual address to a physical address.
 */

private Address vmap (addr)
Address addr;
{
     return(addr);
}

public coredump_readdata(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Address a;

    a = addr;
    if ((a >= datamap.begin) && (a <= datamap.end))
       if (vaddrs) {
	  /*v*/readfromfile(corefile, a, buff, nbytes);
       } else {
	  readfromfile(corefile, a, buff, nbytes);
       }
    else if ((a >= stkmap.begin) && (a <= stkmap.end)) {
	  readfromfile(corefile, a, buff, nbytes);
    } else if (!vaddrs) {
       coredump_readtext(buff, a, nbytes);
    } else {
       memset(buff, -1, 4);
    }
}

/*
 * Read a block of data from a memory image, mapping virtual addresses.
 * Have to watch out for page boundaries.
 */

private vreadfromfile (corefile, v, buff, nbytes)
File corefile;
Address v;
char *buff;
integer nbytes;
{
    Address a;
    integer i, remainder, pagesize;
    char *bufp;

    a = v;
    /*
    pagesize = (integer) ptob(1);
    */
    remainder = pagesize - (a mod pagesize);
    if (remainder >= nbytes) {
	readfromfile(corefile, vmap(a), buff, nbytes);
    } else {
	readfromfile(corefile, vmap(a), buff, remainder);
	a += remainder;
	i = nbytes - remainder;
	bufp = buff + remainder;
	while (i > pagesize) {
	    readfromfile(corefile, vmap(a), bufp, pagesize);
	    a += pagesize;
	    bufp += pagesize;
	    i -= pagesize;
	}
	readfromfile(corefile, vmap(a), bufp, i);
    }
}
#ifdef _THREADS

/*
 * NAME: find_region
 *
 * FUNCTION: Searchs an address in vm_info table
 *           vm_info provides information on vm regions
 *
 * PARAMETERS:
 *      a      - address
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NULL  if address is not found in vmm_regions
 *          Otherwise the address in core file of region.
 */
private integer find_region(a)
Address a;
{
   vm_info_st *ptr;
   integer i;
   for (i = 0; i < regioncnt; i++){
       ptr = &region_ptr[i];
       if (a > (Address)ptr->vminfo_addr &&
           a < ((int)ptr->vminfo_addr + ptr->vminfo_size))

          return(ptr->vminfo_offset + (int) a - (int) ptr->vminfo_addr);

   }
   return(NULL);
}

/*
 * NAME: find_data
 *
 * FUNCTION: Searchs an address in loader_info table
 *           loader_info (structure ld_info) provides information on
 *           loader module. If there is a data region associated the field
 *           _core_offset(ldinfo_core) indicates the location in core file.
 *
 * PARAMETERS:
 *      a      - address
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NULL  if address is not found in data areas
 *          Otherwise the address in core file of data.
 */

private integer find_data(a)
Address a;
{
   integer i;
   for (i = 0; i < loadcnt; i++){
       if (a > loader_info[i].dataorg && a <
          ((int)loader_info[i].dataorg + loader_info[i].datasize))
          if ( loader_offset[i] )
             return(loader_offset[i] + (int)a - (int)loader_info[i].dataorg);
   }
   return(NULL);
}

/*
 * NAME: find_thread
 *
 * FUNCTION: Searchs a kernel thread in core file : uthread structures
 *
 * PARAMETERS:
 *      th_id      - kernel thread id
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NULL if th_id is not found in uthread structures.
 *          Otherwise the pointer on a uthread structure.
 */

public mstsave_st *find_thread(th_id)
tid_t th_id;
{
   mstsave_st *ptr;
   integer i;
   /* test if it's the default thread (in the core header ) */
   if (corehdr.c_mst.curid == th_id) return &corehdr.c_mst;
   for (i = 0; i < mstcnt; i++){
       ptr = &mstsave_ptr[i];
       if (th_id == (tid_t) ptr->curid) return(ptr);
   }
   return(NULL);
}
#endif /* _THREADS  */


private readfromfile (f, a, buff, nbytes)
File f;
Address a;
char *buff;
integer nbytes;
{
    integer fileaddr;
    extern boolean badIO;

    if ((a >= stkmap.begin) && (a <= stkmap.end)) {
	fileaddr = stkmap.seekaddr + a - stkmap.begin;
    } else {
#ifdef _THREADS

        /*search in vm_regions : stacks of the threads               */
        /*         shared data : regions pointed by ldinfo structure */
        /*                       data region associated with the loaded module*/
        /*         take the default data area                        */

        fileaddr = find_region(a);
        if (!fileaddr)
           fileaddr = find_data(a);
        if (!fileaddr)
#endif /* _THREADS  */
	fileaddr = datamap.seekaddr + a - datamap.begin;
    }
    fseek(f, (long) fileaddr, 0);
    if (fread((void *)buff, (size_t) sizeof(Byte), (size_t) nbytes, f)
								!= nbytes) {
	 badIO = true;
	 warning( catgets(scmc_catd, MS_coredump, MSG_534,
			       "Unable to access address 0x%x from core"), a);
         memset(buff, -1, nbytes);
    }
}
