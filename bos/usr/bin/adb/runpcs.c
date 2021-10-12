static char sccsid[] = "@(#)15	1.28  src/bos/usr/bin/adb/runpcs.c, cmdadb, bos411, 9428A410j 12/2/93 13:00:45";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: bpwait, delbp, doexec, endpcs, getsig, getldrinfo, readregs,
 *            reinit, relocbkpts, runpcs, scanbkpt, setbp, setup
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
 *  Service routines for subprocess control
 */

#include "defs.h"

extern int debugflag;
#define BPOUT 0
#define BPIN 1
#define LDCHUNK   4096

LOCAL int  bpstate = BPOUT;
LOCAL long userpc = 1;
int execentry = 0;

#ifdef _NOPROTO
extern long alt_loc();
extern void delbp1();
extern void setbp1();
extern void single();
LOCAL void relocbkpts();
LOCAL void setbp();
#else
extern long alt_loc(int, long, long);
extern void delbp1(BKPTR);
extern void setbp1(register BKPTR);
extern void single(unsigned long, int);
LOCAL void relocbkpts(unsigned long);
LOCAL void setbp();
#endif    /* _NO_PROTO */
extern void bpwait();
extern void readregs();
LOCAL void doexec();

unsigned int exec_text_reloc;
unsigned int exec_data_reloc;
extern unsigned int oldtextorg;
extern unsigned int olddataorg;

extern int coredump;
extern int map_changed;
extern FILEMAP * FileMap;
extern  LDFILE ** ldentry;
extern int datfile, txtfile; /* datfile : address is in data section,
                                txtfile : address is in text section */

int getsig(sig)
int sig;
{
    return (expr(0) ? expv : sig);
}

long save_pc = 0;
int runpcs(runmode,execsig)
int runmode;
int execsig;
{
    int   rc;
    BKPTR bkpt;
#if DEBUG
if (debugflag)
	adbpr("runpcs( adrflg = %#x, execsg = %#x)\n", adrflg, execsig );
#endif
    if (adrflg) userpc = dot;
    if (bkpt = scanbkpt(userpc)) bkpt->flag = (adrflg) ? BKPTSET : BKPTEXEC;
    if (runmode != SINGLE) adbpr("%s: running\n", symfil);

    setbp();
    while (--loopcnt >= 0) {

	if ((bkpt = scanbkpt(userpc)) && (bkpt->flag == BKPTEXEC)) {
	    dot = bkpt->loc;
	    delbp();
	    if ( runmode == SINGLE && save_pc == 0 ) {
	    	save_pc = userpc;
	        single(dot, execsig);
	    	userpc = save_pc;
	    }
	    else
		single(dot, execsig);
	    save_pc = userpc;
	    setbp();
	    bkpt->flag = BKPTSET;
	    execsig = 0;
	} else if (runmode == SINGLE) {
	    single(userpc,execsig);
	}

	errno = 0;
	if (runmode != SINGLE) {
	    if (  (-1 == ptrace(runmode, pid, userpc, execsig)) 
				&& ( errno != 0) )
		  perror( "runpcs" );
	    bpwait();
	    chkerr();
	    execsig = 0;
	    delbp();
	    readregs();
	}

	if ((signo == 0) && (bkpt = scanbkpt(userpc))) {
	    bkpt->flag = BKPTEXEC;
	    --bkpt->count;
	    command(bkpt->comm, ':');
	    if ((bkpt->count == 0)) {
		bkpt->count = bkpt->initcnt;
		rc = 1;                 /* means stopped at breakpoint */
	    } else if (runmode == CONTIN) {
		loopcnt++;
	    }
	} else {
	    execsig = signo;
	    rc = 0;
	}
    }
    return (rc);
}

void endpcs()
{
    BKPTR bkptr;

    if (pid) {
	(void)ptrace(EXIT, pid, 0, 0);
	pid = 0;
	userpc = 1;
	for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
	    if (bkptr->flag)
		bkptr->flag = BKPTSET;
    }
    bpstate = BPOUT;
}

void setup()
{
    long du;

    /*if (-1 == close(fsym))
	perror("setup : close(fsym)"); */
    if ((txtstream == NULL) || (EOF == fclose(IOPTR(txtstream))))
	perror( "setup: fclose()" );
    fsym = -1;
    if ((pid = fork()) == 0) {
	(void)ptrace(SETTRC, 0, 0, 0);
	if (SIG_ERR == signal(SIGINT, sigint))
	    perror( "setup: signal(SIGINT, sigint)" );
	if (SIG_ERR == signal(SIGQUIT, sigqit))
	    perror( "setup: signal(SIGQUIT, sigquit)" );
	doexec();
	_exit(127);
    }
    if (pid == -1)
	error(catgets(scmc_catd,MS_extern,E_MSG_103,NOFORK));
    bpwait();
    errno = 0;
    if (errno != 0)
	perror( "setup" );
    readregs();
    reinit();
    relocbkpts(1);
    updatemaps();
    lp[0] = '\n';
    lp[1] = 0;
    symfil = fd_info[0].pathname;
    if (-1 == (fsym = open(symfil, wtflag)))
	perror( "setup: open(symfil, wtflag)" );
    if ( ldaclose(txtstream) == -1 ) 
	perror( "setup: ldaclose(txtstream)" );
    txtstream = ldopen(symfil, 0);
    if (txtstream == NULL)
	perror( "setup: ldopen(symfil, 0)" );
    if (errflg != NULL) {
	adbpr("%s: cannot execute\n", symfil);
	endpcs();
	error((STRING)NULL);
    }
    bpstate=BPOUT;
}

LOCAL void doexec()
{
    STRING argl[MAXARG];
    char   args[LINSIZ];
    STRING p = args;
    STRING *ap = argl;
    STRING filnam;

    symfil = fd_info[0].pathname;
    *ap++ = symfil;
    do {
	if (rdc() == '\n')
	    break;
	*ap = p;
	while (lastc != '\n' && lastc != ' ' && lastc != '\t') {
	    *p++ = lastc;
	    (void)readchar();
	}
	*p++ = 0;
	filnam = *ap + 1;
	if (**ap == '<') {
	    if (-1 == close(0))
		perror( "doexec: close(0)" );
	    if (open(filnam, 0) == -1) {
		perror(filnam);
		_exit(127);
	    }
	} else if (**ap == '>') {
	    if (-1 == close(1))
		perror( "doexec: close(1)" );
	    if (creat(filnam, 0666) == -1) {
		perror(filnam);
		_exit(127);
	    }
	} else
	    ap++;
    } while (lastc != '\n');
    *ap++ = 0;
    if (-1 == execve(symfil, argl, environ))
	adbpr( catgets(scmc_catd, MS_runpcs, M_MSG_32, 
	  "adb: cannot  exec the process, probable cause : bad executable\n") );
}

/*
 * getldrinfo:  get necessary loader information from the process
 *	Note:  the loader information is actually slightly different
 *	       from the ldinfo structure used here.  To see the actual
 *	       loader structure, see <sys/ldr.h>.
 */

extern struct core_dump corehdr;
extern FILE * corefile;
int loadcnt = 0;

getldrinfo (iscoreimage) 
int iscoreimage;
{
	char *ldinfo_ptr;
	BOOL badinfo = 1;
	unsigned int ldinfosz = LDCHUNK;
	unsigned int ldcnt = 0;

	if ( iscoreimage ) {
	   /* Stack is after ldinfop in core file. */
           ldinfosz = (unsigned int) ((int) corehdr.c_stack) - 
						((int) corehdr.c_tab);
           ldinfo_ptr = (char *) malloc(ldinfosz);
	   rewind(corefile);
           fseek(corefile, (long int) corehdr.c_tab, 0);
           fread((void *) ldinfo_ptr, (size_t) 1, 
					(size_t)ldinfosz, corefile);
        } else if (pid != 0) {
	   ldinfo_ptr = (char *) malloc(LDCHUNK);
	   badinfo = (BOOL)ptrace(PT_LDINFO,pid,ldinfo_ptr,ldinfosz);
	   for (; badinfo; ) {
	       ldinfosz += LDCHUNK;
	       ldinfo_ptr = (char *) realloc((char *)ldinfo_ptr,ldinfosz);
	       errno = 0;
	       badinfo = (BOOL)ptrace(PT_LDINFO,pid,ldinfo_ptr,ldinfosz);
	   }
	} else
	   ldinfo_ptr = (char *) malloc(LDCHUNK);
	
	loader_info = (struct ldinfo *) malloc(ldinfosz);
	if ( fd_info ) {
		fd_info = (struct fdinfo *) realloc(fd_info, ldinfosz/3);
	} else {
		fd_info = (struct fdinfo *) calloc(1, ldinfosz/3);
		fd_info[0].pathname = symfil;
		fd_info[0].membername = (char *) 0;
	}

	if  ( (pid != 0) || ( iscoreimage) ) { 
	   do {
	       memcpy(&loader_info[ldcnt],ldinfo_ptr,sizeof(struct ldinfo));
	       if ( ! fd_info[ldcnt].pathname && 
				! fd_info[ldcnt].membername ) {
	          fd_info[ldcnt].pathname = ldinfo_ptr + sizeof(struct ldinfo);
	          fd_info[ldcnt].membername =
		     		fd_info[ldcnt].pathname + 
				strlen(fd_info[ldcnt].pathname) + 1;
	       }
	       ldinfo_ptr += loader_info[ldcnt].ldinfo_next;
	       if (loader_info[ldcnt].textorg == 0x10000000)
		   execentry = ldcnt;
	       if ( iscoreimage ) {
		   if ( loader_info[ldcnt].dataorg < slshmap.b2 )
				  slshmap.b2 = loader_info[ldcnt].dataorg;
		   slshmap.e2 += loader_info[ldcnt].dataorg + 
				  loader_info[ldcnt].datasize;
	       } 
	   } while (loader_info[ldcnt++].ldinfo_next != 0);
	   if ( iscoreimage ) {
		slshmap.e2 += slshmap.b2;
		slshmap.f2 = (unsigned int) (corehdr.c_stack + corehdr.c_size);
	   }
	
	} else {
/*
 	adb should be able to disassemble the nonexecutable object file ".o". 
	fill this section for such object files.
*/
	   loader_info->ldinfo_next = 0;
	   loader_info->ldinfo_fd = getfile(symfil,1);
	   loader_info->textorg = 0x0;
	   loader_info->textsize = 0x0fffffff;
	   loader_info->dataorg = 0x0;
	   loader_info->datasize = 0x0fffffff;
	   fd_info->pathname = symfil;
	   fd_info->membername = (char *) 0;
	   ldcnt = 1;
	}
	loadcnt =  ldcnt; 
	exec_text_reloc = loader_info[execentry].textorg;
	exec_data_reloc = loader_info[execentry].dataorg;
}

LOCAL void relocbkpts(textorg)
unsigned long textorg;
{
    BKPTR bkptr;
    int filndx;

    for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
	if (bkptr->flag) {
		filndx = textobj(bkptr->loc);
		if ( textorg == 0 ) 
		   if ( txtfile != -1 ) /* standard case - bkpt in text seg. */
			bkptr->loc = bkptr->loc - loader_info[filndx].textorg -
					FileMap[filndx].txt_scnptr;
		   else if ( datfile != -1 ) /* bkpt in data segment */
				bkptr->loc = bkptr->loc - 
					loader_info[filndx].dataorg -
					FileMap[filndx].txt_scnptr;
			else /* not within map use base data value */
				bkptr->loc -= DATAORG;
		else 
		   if ( txtfile != -1 )
			bkptr->loc += loader_info[filndx].textorg + 
				FileMap[filndx].txt_scnptr;
		   else if ( datfile != -1 )
				bkptr->loc += loader_info[filndx].dataorg + 
					FileMap[filndx].txt_scnptr;
			else 
				bkptr->loc += DATAORG;
        }
}

/* If you find the breakpoint at the address adr then return the structure 
that contains the breakpoint */
BKPTR scanbkpt(adr)
long adr;
{
    BKPTR bkptr;

    for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) 
	if (bkptr->flag && bkptr->loc == adr)
	    break;
    return (bkptr);
}

void delbp()
{
    BKPTR bkptr;

    if (bpstate != BPOUT) {
	for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) {
	    if (bkptr->flag) delbp1(bkptr);
	}
	bpstate = BPOUT;
    }
}

LOCAL void setbp()
{
    register BKPTR bkptr;

    if (bpstate != BPIN) {
	for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt) {
	    if (bkptr->flag) setbp1(bkptr);
	}
	bpstate = BPIN;
    }
}

#define LOAD 0x7c
extern int save_wtflag;
void bpwait()
{
    int w;
    int stat, s;

    if (SIG_ERR == signal(SIGINT, SIG_IGN))
	perror( "bpwait: signal " );
    while ((w = wait(&stat)) != pid && w != -1 )
	;
    if (SIG_ERR == signal(SIGINT, sigint))
	perror( "bpwait: signal " );
    if (w == -1) {
	pid = 0;
	errflg = catgets(scmc_catd,MS_extern,E_MSG_95,BADWAIT);
    } else if ((stat & 0177) != 0177 )  {
	int i;
	if ( (stat & 0174) == 0174 ) {
		for ( i = 0; i < loadcnt; i++ ) {
			ldaclose(ldentry[i]);
			close(FileMap[i].fd);
		}
		fd_info = 0;
		loader_info = 0;
		reinit();
		updatemaps();
	} else {
	if (signo = stat & 0177)
	    sigprint();
	if (stat & 0200) {
	    prints( " - core dumped" );
	    if (-1 == close(fcor))
		perror( "bpwait: close(fcor)" );
	    setcor();
	}
	flushbuf();
	pid = 0;
	errflg = catgets(scmc_catd,MS_extern,E_MSG_96,ENDPCS);
	coredump = FALSE;
	wtflag = save_wtflag;
	save_wtflag = O_RDONLY;
	for ( i = 0; i < loadcnt; i++ ) {
		ldaclose(ldentry[i]);
		close(FileMap[i].fd);
	}
	reinit();
      }
    } else {
	signo = stat >> 8;
	if (signo != SIGTRAP)
	    sigprint();
	else
	    signo = 0;
	flushbuf();
    }
}

/*  FUNCTION : readregs: It gets the values of all the machine registers
    	       of a child process.
*/
void readregs()
{
    int i;

    for (i = 0; i < NREGS+NKREGS; i++) {
	errno = 0;
	LVADBREG(reglist[i].roffs) =
		ptrace(RREG, pid, REGTOSYS(i), 0);
	if (errno != 0)
	    perror( catgets(scmc_catd, MS_runpcs, E_MSG_37, 
		"error in reading the general purpose registers.") );
    }
    for (i = 0; i < MAXFPREGS; i++) {
	errno = 0;
	ptrace(PT_READ_FPR, pid, &fpregs[i], FREGTOSYS(i), 0);
	if (errno != 0)
	    perror( catgets(scmc_catd, MS_runpcs, E_MSG_38, 
		"error in reading the floating point registers.") );
    }
    userpc = ADBREG(SYSREGNO(IAR));
}

extern int kmem_core;
printloaderinfo()
{
   int i;

   for (i = 0; i < loadcnt; i++) {
       if (( fd_info[i].membername != 0) && 
				( fd_info[i].membername[0] != '\0' ) )
            printf("[%d] : ? map : '%s' in library '%s'\n\n", 
			i, fd_info[i].membername, fd_info[i].pathname);
       else
       	    printf("[%d] : ? map : '%s'\n\n", i, fd_info[i].pathname);

       printf("b1 = %#x,\te1 = %#x,\tf1 = %#x\n",
		FileMap[i].begin_txt, FileMap[i].end_text, 
						FileMap[i].txt_seekadr);
       printf("b2 = %#x,\te2 = %#x,\tf2 = %#x\n",
		FileMap[i].begin_data, FileMap[i].end_data, 
						FileMap[i].data_seekadr);
       printf("\n");
   }
   if ( (! coredump) && (! kmem_core) && ( ! map_changed ) ) {
       	    printf("[-] : / map : '-'\n\n");
       	    printf("b1 = 0x0000000,\te1 = 0x000000000,\tf1 = 0x000000000\n");
       	    printf("b2 = 0x0000000,\te2 = 0x000000000,\tf2 = 0x000000000\n");
   }
   else {
       	    printf("[0] : / map : '%s'\n\n", corfil);
       	    printf("b1 = %#x,\te1 = %#x,\tf1 = %#x\n", slshmap.b1,
			slshmap.e1, slshmap.f1);
       	    printf("b2 = %#x,\te2 = %#x,\tf2 = %#x\n", slshmap.b2,
			slshmap.e2, slshmap.f2);
	}	
}


extern SYMINFO * SymTbl;
reinit()
{
	int i;

	if (fsym > 0) close(fsym);
	if (fcor > 0) close(fcor);
	if (txtstream != NULL) {
		ldaclose(txtstream);
	}
	for ( i = 0; i < loadcnt; i++ ) {
		ldaclose(ldentry[i]);
		close(FileMap[i].fd);
		close(loader_info[i].ldinfo_fd);
	}
	if ( SymTbl ) free( (char *)SymTbl);
	if ( FileMap ) free( (char *)  FileMap );
	if ( ldentry ) free ( (char *) ldentry );
	coredump = FALSE;
	if (! pid)
		relocbkpts(0);
	getldrinfo(0);
	readobj(0,0);
	setsym();
	symfil = fd_info[0].pathname;
}
