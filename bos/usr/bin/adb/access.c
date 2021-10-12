static char sccsid[] = "@(#)%M  1.14  src/bos/usr/bin/adb/access.c, cmdadb, bos411, 9428A410j  1/30/92  17:41:03";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: bptrace, cget, chkmap, cput, get, getsome, put, putsome. sget,
 *            sput, within
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

#include "defs.h"
/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#ifdef _NO_PROTO
LOCAL BOOL chkmap(); 
LOCAL BOOL within();
#else
LOCAL BOOL chkmap(long * adr, int sptype); 
LOCAL BOOL within(unsigned long adr, unsigned long lbd, unsigned long ubd);
#endif  /* _NO_PROTO */

#ifdef RBLOCK
#ifdef _NO_PROTO
LOCAL int bptrace();
#else
LOCAL int bptrace(register int pmode,register int tpid, 
		  register long addr, register long value);
#endif  /* _NO_PROTO */

#else
#define bptrace ptrace
#endif

#define WMASK  ~03

extern MAP stkmask;
extern int coredump;
extern int kmem_core;
extern int debugflag;
extern BOOL rdwr_core;
extern struct core_dump corehdr;
extern LDFILE  ** ldentry;
extern FILEMAP * FileMap;
extern int datfile, txtfile; /* datfile : address is in data section, 
				txtfile : address is in text section */
int text_object = 0;
LOCAL long oldadr = -1;

LOCAL unsigned long getsome(adr, sptype, cnt)
long adr;
int sptype;
int cnt;
{
    long   w ;
    unsigned long  rc1, offset, newaddr;

#if DEBUG
    if (debugflag)
	printf("->getsome:adr = %#x\n", adr); /*dbg*/
#endif
    if (sptype == NSP)
	return (dot);

    if (pid != 0) {
	errno = 0;
	newaddr = adr & WMASK;
	offset = adr - newaddr;
#if DEBUG
#if 0
if (debugflag)
     printf("newaddr=%#x, adr=%#x, offset=%#x\n",newaddr, adr, offset);/*dbg*/
	if ( 0 ) {
		rc1 = bptrace(RDUSER, pid, newaddr,0);
		if (debugflag)
		printf("bptrace returns in rc1= %#x\n", rc1); /*dbg*/
		if ( ( rc1 == -1 ) && ( errno != 0) ) 
			printf(" bptrace failed at %#x\n", newaddr);
		errno = 0;
		if (debugflag)
		printf("bptrace(RDUSER,pid,%#x,0)\n",newaddr+4);/*dbg*/
		w = bptrace(RDUSER, pid, newaddr+4, 0);
		if (debugflag)
		printf("bptrace returns in w= %#x\n", w); /*dbg*/
		if ( ( w == -1 ) && ( errno != 0) ) 
			printf(" bptrace failed at %#x\n", newaddr+4);
		w = ( rc1 << (offset * 8) ) | ( w >> (4 - offset) * 8);
		if (debugflag)
		printf("w = %#x\n",w);/*dbg*/
	}
	else
#endif
#endif
		w = bptrace(RDUSER, pid, adr, 0); /* RIUSER equivalent to RDUSER */
	/* If the addresses are not word aligned, ignore the errno */
	if ( (offset == 0) && (w == -1 ) && (errno != 0)) {
	    	errflg = (sptype & DSP
			   ? catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT)
			   : catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT));
#if DEBUG
		if (debugflag)
			printf("getsome11: errflg = %#x, errno = %#x\n", errflg,errno ) ; /*dbg*/

#endif
	}
	return (cnt == 1 ? w >> 24 : cnt == 2 ? w >> 16 : w);
    }
    if (!chkmap(&adr, sptype))
	return (0);

    if (sptype & DSP) {
#if KADB
	extern int rdfd, rderr;
	extern long rdwget();
	if (rdfd) {
	    w = rdwget(adr, cnt);
	    if (rderr)
		errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	    return (w);
	}
#endif
	if ( rdwr_core || (coredump || kmem_core) ) {
	     if (lseek(slshmap.ufd, (long) adr, 0) == -1L ||
	    	read(slshmap.ufd,(char *)&w, (unsigned)cnt) != cnt)
	    	errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	}
	else {
	if (lseek(FileMap[text_object].fd, (long) adr, 0) == -1L ||
	    read(FileMap[text_object].fd,(char *)&w, (unsigned)cnt) != cnt)
	    errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	}
	return (cnt == 1 ? *(char *)&w : cnt == 2 ? XH(*(short *)&w) : XL(w));
    }
    if (text_object == -1) {
	errflg = catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT);
#if DEBUG
	if (debugflag)
		printf("getsome2:txtstream is null\n" ); /*dbg*/
#endif
	return (-1);
    }
    if (oldadr != adr) {
#if DEBUG
	if (debugflag)
		printf("Fseeking at the address: %#x in the file %s\n",adr, fd_info[text_object].pathname);/*dbg*/
#endif
	FSEEK(ldentry[text_object], adr, 0);
	oldadr = adr;
    }
    if (cnt == 1) {
	w = FGETC(ldentry[text_object]);
    } else if (cnt == 2) {
	short temp;
	((char *)&temp)[0] = FGETC(ldentry[text_object]);
	((char *)&temp)[1] = FGETC(ldentry[text_object]);
	w = XH(temp);
    } else {
	long temp;
	((char *)&temp)[0] = FGETC(ldentry[text_object]);
	((char *)&temp)[1] = FGETC(ldentry[text_object]);
	((char *)&temp)[2] = FGETC(ldentry[text_object]);
	((char *)&temp)[3] = FGETC(ldentry[text_object]);
	w = XL(temp);
    }
    oldadr += cnt;
    if (FEOF(ldentry[text_object]) || FERROR(ldentry[text_object])) {
#if DEBUG
	if (debugflag)
	printf("getsome; 3 either  eof or textstream is null\n "); /*dbg*/
#endif
	errflg = catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT);
    }
    return (w);
}

LOCAL void putsome(adr, sptype, value, cnt)
long adr;
int sptype;
long value;
int cnt;
{
    register char *pvalue;
    short svalue;
    char  cvalue;

    if (pid) {      /* Does a process exist? */
	if (cnt == 2) {
	    svalue = value;
	    pvalue = (char *)&svalue;
	} else if (cnt == 4) {
	    pvalue = (char *)&value;
	} else {
	    cvalue = value;
	    pvalue = &cvalue;
	}
/*! This seems not to be working !*/
	errno = 0;
	(void)ptrace(WBLOCK, pid, adr, cnt, pvalue);
	if (errno != 0) {
	    errflg = (sptype & DSP 
			? catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT) 
			: catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT));
#if DEBUG
	    if (debugflag)
	    	printf("putsome:1 errflg = %d\n", errflg ); /*dbg*/
#endif
	}
	return;
    }
    if (wtflag == O_RDONLY)
	error( catgets(scmc_catd, MS_access, E_MSG_14, "not in write mode") );

    if (!chkmap(&adr, sptype))
	return;

    if (cnt == 2) {
	svalue = value;
	svalue = XH(svalue);
	pvalue = (char *)&svalue;
    } else if (cnt == 4) {
	value = XL(value);
	pvalue = (char *)&value;
    } else {
	cvalue = value;
	pvalue = &cvalue;
    }
    if (sptype & DSP) {
#if KADB
	extern int rdfd, rderr;
	extern long rdwget();
	if (rdfd) {
	    kwput(adr, cnt == 2 ? svalue : value, cnt);
	    if (rderr)
		errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	    return;
	}
#endif
 	if ( rdwr_core && coredump ) {
	     if (lseek(slshmap.ufd, (long) adr, 0) == -1L ||
	    	write(slshmap.ufd,pvalue, (unsigned)cnt) != -1)
	    	errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	     rdwr_core = FALSE;
  	}
	else
	if (lseek(FileMap[text_object].fd, (long) adr, 0) == -1L ||
		write(FileMap[text_object].fd,  pvalue, (unsigned)cnt) == -1)
	    errflg = catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT);
	return;
    }
    if (text_object == -1) {
	errflg = catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT);
#if DEBUG
	if (debugflag)
		printf("putsome:2 errflg = %d\n", errflg ); /*dbg*/
#endif
	return;
    }
    if (oldadr != adr) {
	FSEEK(ldentry[text_object], adr, 0);
	oldadr = adr;
    }
    FWRITE(pvalue, 1, cnt, ldentry[text_object]);
    oldadr += cnt;
}

/* visible routines for file handling */

void put(adr, sptype, value)
long adr;
int sptype;
long value;
{
    putsome(adr, sptype, value, 4);
}

void sput(adr, sptype, value)
long adr;
int sptype;
long value;
{
    putsome(adr, sptype, value, 2);
}

void cput(adr, sptype, value)
long adr;
int sptype;
long value;
{
    putsome(adr, sptype, value, 1);
}

unsigned long get(adr, sptype)
long adr;
int sptype;
{
    return (getsome(adr, sptype, 4));
}

unsigned char cget(adr, sptype)
long adr;
int sptype;
{
    return ((unsigned char)getsome(adr, sptype, 1));
}

unsigned short sget(adr, sptype)
long adr;
int sptype;
{
    return ((unsigned short)getsome(adr, sptype, 2));
}

extern MAP stkmap;

LOCAL BOOL chkmap(adr, sptype)
long *adr;
int sptype;
{
    text_object = -1;
    if ( (text_object = textobj((unsigned int) *adr)) != -1 ) {
	if ( txtfile != -1 ) {  /* Address is in text section */
	      *adr += FileMap[txtfile].txt_seekadr - FileMap[txtfile].begin_txt
		     	- loader_info[txtfile].textorg;
      	      return (TRUE);
	}
	else if ( datfile != -1 ) {  /* address is in the data section */
	      if ( rdwr_core || coredump ) {
		*adr = (long)( *adr - DATAORG + 
			corehdr.c_stack + corehdr.c_size);
	      }
	      return (TRUE);
       }
    } else if ( kmem_core || coredump ) {  
	if (within((unsigned long)*adr, stkmap.b1, stkmap.e1)){
	    	/* address is in the stack range */
		*adr += (stkmap.f1) - stkmap.b1;
		return (TRUE);
        }  else  if ( within((unsigned long) *adr, slshmap.b1, slshmap.e1) ) 
		return (TRUE);
    } else {
	errflg = (sptype&DSP 
                        ? catgets(scmc_catd,MS_extern,E_MSG_82,BADDAT)
                        : catgets(scmc_catd,MS_extern,E_MSG_93,BADTXT));
#if DEBUG
  	if (debugflag)
		printf("chkmap:1 errflg = %d\n", errflg ); /*dbg*/
#endif
	return (FALSE);
    }
}

LOCAL BOOL within(adr, lbd, ubd)
unsigned long adr;
unsigned long lbd;
unsigned long ubd;
{
    return (adr >= lbd && adr < ubd);
}

#ifdef RBLOCK
LOCAL int bptrace(pmode, tpid, adr, value)
register int pmode, tpid;
register long adr, value;
{
    register int i;
    static char dbuf[1024];
    static long dstart = -1;
    union { char c[4]; long w; } dat;

#if DEBUG
if (debugflag)
printf("bptrace(tpid = %#x, adr=%#x, value = %#x)\n", tpid, adr, value); /*dbg*/
#endif
    switch (pmode) {
    case RIUSER:
    case RDUSER:
	if (dstart == -1 || adr<dstart || adr>=dstart+1020) {
	    dstart = adr & ~3;
	}
	if (debugflag) {
		printf("calling ptrace from bptrace\n");
		printf("dstart = %#x\n",dstart);
	}
	if (ptrace(RBLOCK,tpid,dstart,1024,dbuf)<0) goto retry;
	    /* will fail if it crosses boundary of segment */
	adr -= dstart;
	for (i=0;i<4;i++) dat.c[i] = dbuf[adr+i];
	return (dat.w);
    default:
    retry:
	errno = 0;
	dstart = -1;
	if (debugflag) {
		printf("calling ptrace from bptrace after label retry:\n");
		printf("ptrace(pmode=%#x,tpid=%#x,adr=%#x,value=%#x)\n",pmode,tpid,adr, value);/*dbg*/
	}
	return (ptrace(pmode, tpid, adr, value)); 
    }
}
#endif
extern int coredump;
