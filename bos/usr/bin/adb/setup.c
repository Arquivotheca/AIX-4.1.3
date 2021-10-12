static char sccsid[] = "@(#)16	1.33  src/bos/usr/bin/adb/setup.c, cmdadb, bos411, 9428A410j 6/23/94 13:21:40";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  setsym, setcor, updatemaps, coredump_reading, copyregs,
 * 	       create, getfile, typesym, loadstr, loaddbg, textobj,
 *	       readobj
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

#include       "defs.h"
#ifdef _THREADS
#include <sys/uthread.h>
#endif /* _THREADS */
#include       <core.h>
#undef ADDR_U 
#include       <sys/pseg.h>
#include       <sys/resource.h>

#define TXTHDRSIZ ((int) sizeof(txthdr))
#define TXTRNDSIZ 512
#define  false   0
#define  true    1

#ifdef _NO_PROTO
int  getfile();
LOCAL int  typesym();
LOCAL void loadstr();
LOCAL void loaddbg();
#else
int  getfile(STRING, int);
LOCAL int  typesym(register SYMPTR, int);
LOCAL void loadstr(register FILHDR *, int);
LOCAL void loaddbg(register FILHDR *, int);
#endif

#ifdef _THREADS
int regioncnt, mstcnt;
typedef struct vm_info vm_info_st;
typedef struct mstsave mstsave_st;
vm_info_st *region_ptr;
mstsave_st *mstsave_ptr;
mstsave_st *mstsave_current;
#endif /* _THREADS */

extern void updatemaps();

extern int debugflag ; /*dbg purposes only*/
extern int coredump;
extern int loadcnt ;
extern int rdwr_core;
int inmagic;
unsigned short core_modules = 0;
unsigned int oldtextorg = 0;
unsigned int olddataorg = 0;

SCNHDR      data_scn_hdr, text_scn_hdr, bss_scn_hdr, dbg_scn_hdr;
FILHDR	    txthdr;
AOUTHDR	    aouthdr;
FILEMAP     * FileMap, *SaveMap;
SYMINFO     * SymTbl;
LDFILE      ** ldentry;

void setsym()
{
    SYMSLAVE    *symptr;
    SYMPTR      symp;
    char        *p;
    int         i,j;
    LDFILE *ldptr = 0;
    FILEMAP *filptr = 0;
    struct rlimit resource;

    FileMap = (FILEMAP *) calloc(loadcnt, sizeof (FILEMAP));
    SaveMap = (FILEMAP *) calloc(loadcnt, sizeof (FILEMAP));
    SymTbl  = (SYMINFO *) calloc(loadcnt, sizeof (SYMINFO));

    /* Initialize E1 value */
    getrlimit(RLIMIT_FSIZE, &resource);
    FileMap[0].end_text =  resource.rlim_max;
    /* Get the  symbols from all the files in ldentry */
    for (i = 0; i < loadcnt; i++) {
 	/* If the file doesn't exist then break the for loop */
	if ( !(txtstream = ldentry[i]) ) {
 		FileMap[0].end_text = 0;
		break;
	}
 	symfil = fd_info[i].pathname;
 	FileMap[i].fd = getfile(symfil, 1);

    /* Following strange sequence is because ldopen only opens for read. */
    if (wtflag == O_RDWR) {
	fclose(IOPTR(txtstream));
	IOPTR(txtstream) = fopen(symfil,"r+");
    }
    if ( ldfhread(txtstream, &txthdr) && 
	 ldnshread(txtstream, _TEXT, &text_scn_hdr) )
    {
	if ((magic = kheader(&txthdr)) == 0)
		;
	else {
	    if (txthdr.f_opthdr) {
	        ldohseek(txtstream);
	        FREAD(&aouthdr, 1, sizeof(AOUTHDR), txtstream);
	    }
	    SymTbl[i].symnum = txthdr.f_nsyms;

            /* txtsiz = text_scn_hdr.s_size; */         /* size of text */
            SymTbl[i].txtsize = text_scn_hdr.s_size;    /* size of text */
	    for (j = 1; j <= txthdr.f_nscns; j++) {
		SCNHDR temp_scn_hdr;

		ldshread(txtstream, (unsigned short)j, &temp_scn_hdr);
		if (!strcmp(temp_scn_hdr.s_name, _TEXT))
	   	   SymTbl[i].text_scn_num = j;
		else if (!strcmp(temp_scn_hdr.s_name, _DATA)) {
	   	   SymTbl[i].data_scn_num = j;
	   	   memcpy(&data_scn_hdr, &temp_scn_hdr, SCNHSZ);
	           SymTbl[i].datsize = data_scn_hdr.s_size;
		} else if (!strcmp(temp_scn_hdr.s_name, _DEBUG)) {
	   	   dbg_scn_num = j;
	   	   memcpy(&dbg_scn_hdr, &temp_scn_hdr, SCNHSZ);
		} else if (!strcmp(temp_scn_hdr.s_name, _BSS))
	   	   SymTbl[i].bss_scn_num = j;
		if (SymTbl[i].text_scn_num && 
		   SymTbl[i].data_scn_num && 
		   SymTbl[i].bss_scn_num && SymTbl[i].dbg_scn_num)
	   	   break;
	    }
	    if (!SymTbl[i].text_scn_num)  SymTbl[i].text_scn_num = -3;
	    if (!SymTbl[i].data_scn_num)  SymTbl[i].data_scn_num = -3;
	    if (!SymTbl[i].dbg_scn_num )  SymTbl[i].dbg_scn_num = -3;
	    if (!SymTbl[i].bss_scn_num )  SymTbl[i].bss_scn_num = -3;

	    if (txthdr.f_flags & F_EXEC ) {
		/* Get the begin, end and seek addresses of the a.out */

		magic = aouthdr.magic;
		FileMap[i].begin_txt = aouthdr.text_start;
		FileMap[i].txt_scnptr = text_scn_hdr.s_scnptr - 
			aouthdr.text_start;
	    	FileMap[i].end_text   = aouthdr.text_start + aouthdr.tsize;
		txtsiz = aouthdr.tsize;
	    	ldnsseek(txtstream, _TEXT);
	    	FileMap[i].txt_seekadr   = text_scn_hdr.s_scnptr;
	
	    	FileMap[i].begin_data = aouthdr.o_data_start;
		datsiz   =  aouthdr.o_dsize;
	    	ldnsseek(txtstream, _DATA);
	    	FileMap[i].data_seekadr = FTELL(txtstream);
	    	FileMap[i].end_data = FileMap[i].data_seekadr + aouthdr.o_dsize;
	
	    	FileMap[i].entrypoint = aouthdr.o_entry;
	    	entrypt = aouthdr.o_entry;
	    	ldtbseek(txtstream);
	    	FileMap[i].st_seekadr = FTELL(txtstream);
	    	inmagic = txthdr.f_flags;
	     } else {
		/* Get the begin, end,  and  seek addresses of the various 
		   sections of an object file. */

	    	FileMap[i].begin_txt =  text_scn_hdr.s_paddr;
	    	FileMap[i].txt_scnptr = text_scn_hdr.s_scnptr;
	    	FileMap[i].end_text  =  text_scn_hdr.s_paddr + 
					text_scn_hdr.s_size;
	    	ldnsseek(txtstream, _TEXT);
	    	FileMap[i].txt_seekadr = FTELL(txtstream);
	
	    	ldnsseek(txtstream, _DATA);
	    	FileMap[i].begin_data = data_scn_hdr.s_paddr;
	    	FileMap[i].end_data   = data_scn_hdr.s_paddr + 
					data_scn_hdr.s_size;
	    	FileMap[i].data_seekadr = FTELL(txtstream);
	
	    	FileMap[i].entrypoint = text_scn_hdr.s_paddr;
	    	ldtbseek(txtstream);
	    	FileMap[i].st_seekadr = FTELL(txtstream);
	    	inmagic = txthdr.f_flags;
	    }
	    SaveMap[i].begin_txt = FileMap[i].begin_txt;
	    SaveMap[i].txt_scnptr = FileMap[i].txt_scnptr;
	    SaveMap[i].end_text   = FileMap[i].end_text;
	    SaveMap[i].begin_data = FileMap[i].begin_data;
	    SaveMap[i].end_data   = FileMap[i].end_data;
	    SaveMap[i].txt_seekadr = FileMap[i].txt_seekadr;
	    SaveMap[i].data_seekadr = FileMap[i].data_seekadr;
	    SaveMap[i].st_seekadr  = FileMap[i].st_seekadr;

	    if ( (p = sbrk((1+SymTbl[i].symnum) * (int)sizeof(SYMSLAVE))) == 
		      (char *)-1 ) {
		static SYMSLAVE dummy;
		adbpr("%s\n", catgets(scmc_catd,MS_extern,E_MSG_89,BADNAM));
		symptr = &dummy;
	    } else {
		SymTbl[i].symvec = (SYMSLAVE *) p;
		symptr = SymTbl[i].symvec;
		symset(i);
		while ((symp = symget(i)) && errflg == NULL) {
		    register int numaux;
		    symptr->valslave = symp->n_value;
		    symptr->typslave = typesym(symp, i);
		    symptr++;
		    for (numaux = symp->n_numaux; --numaux >= 0;) {
			symptr->valslave = 0;
			symptr->typslave = EXTRASYM;
			symptr++;
		    }
		    skip_aux(symp,i);
		}
	    }
	    symptr->typslave = ENDSYMS;
	    loadstr(&txthdr,i);
	    if (dbg_scn_num != -3)
	        loaddbg(&txthdr, i);
	}
    }
    if (magic == 0)
	qstmap.e1 = maxfile;
  } /* end of the first for */
  if ( coredump )
	updatemaps();
}

void updatemaps()
{
    SYMSLAVE    *symptr;
    SYMPTR      symp;
    int         symtype;
    int i;
    unsigned int txtreloc, datareloc;

    for(i = 0; i < loadcnt; i++) {
        txtreloc  = loader_info[i].textorg;
	datareloc = loader_info[i].dataorg;

    	FileMap[i].begin_txt  += txtreloc + FileMap[i].txt_scnptr;
    	FileMap[i].end_text   =  txtreloc + loader_info[i].textsize;
    	FileMap[i].txt_seekadr   +=  txtreloc;
    	FileMap[i].begin_data = datareloc;
    	FileMap[i].end_data   = datareloc + loader_info[i].datasize;
    	FileMap[i].entrypoint += txtreloc;
    }
}

struct  core_dump  corehdr;

#define streq(s1, s2) (strcmp(s1, s2) == 0)
#define MAXFREG 64
#define NGREG 32
FILE * corefile = 0;
int kmem_core = 0;

void setcor()
{
     STRING  base_objname, base_progname, objname;
     struct ld_info  ctab;
     struct stat  core_times, obj_times;
     double fpregs[MAXFREG];

     slshmap.b1 = 0;
     slshmap.e1 = (unsigned long)0xffffffff;
     slshmap.f1 = 0;
     slshmap.b2 = 0;
     slshmap.e2 = (unsigned long)0xffffffff;
     slshmap.f2 = 0;

     kmem_core = ( (!strcmp(corfil,"/dev/mem") ) || (!strcmp(corfil,"mem")));
     if ( wtflag == O_RDWR )
        corefile = fopen(corfil,"r+");
     else
        corefile = fopen(corfil,"r");
     if ( kmem_core ) 
	return;
     coredump = true;
     fread((char *) &corehdr, 1, CHDRSIZE, corefile); 

     /* Check to see if the core is a valid core. */
     if ( (corehdr.c_flag & LE_VALID) != LE_VALID ) {
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_43,
		"bad corefile\n"));
	coredump = false;
       	return;
      }
     if ( (corehdr.c_flag & FULL_CORE) != FULL_CORE )
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_44,
		"data section is missing from the corefile\n"));
     if ( (corehdr.c_flag & USTACK_VALID) != USTACK_VALID ){
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_45,
		"user stack is missing from the corefile\n"));
	coredump = false;
      }
     if ( (corehdr.c_flag & UBLOCK_VALID) != UBLOCK_VALID ){
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_46,
		"ublock is missing from the corefile\n"));
	coredump = false;
      }
     if ( (corehdr.c_flag & CORE_TRUNC) == CORE_TRUNC )
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_47,
		"core is truncated\n"));
     if (coredump == false) return;
     rewind(corefile);
     core_modules = (unsigned short) corehdr.c_entries;
     getldrinfo(1);

     /* Strip path to object file if it exists. */
     base_objname = (STRING) strrchr(symfil,'/');
     if (!base_objname)
           base_objname = symfil;
     else
           base_objname++;
     /* Strip path to exec name from core file if it exists. */
     base_progname = (STRING) strrchr(fd_info[0].pathname,'/');
     if (!base_progname)
           base_progname = fd_info[0].pathname;
     else
           base_progname++;
     if (!strcmp(base_progname,base_objname))  /* verify No. 1 */
     {
          stat(corfil, &core_times);
          stat(base_objname,  &obj_times);
          if (obj_times.st_mtime < core_times.st_mtime){  /* No. 2 */
	       coredump_reading(adbreg, fpregs, &signo);
	  }
          else {
             coredump = false;
             perror( catgets(scmc_catd, MS_setup, M_MSG_38, 
		    "Core file is outdated or doesn't match current program") );
	     return ;
          }
      }
      else if (streq(corfil,"/dev/mem") || streq(corfil,"/dev/kmem")) {
	       coredump_reading(adbreg, fpregs, &signo);
      }
      else {
           coredump = false;
           perror(  catgets(scmc_catd, MS_setup, M_MSG_38, 
		    "Core file is outdated or doesn't match current program") );
           return ;
       }
}

LDFILE *objfile = 0;
extern int loadcnt;

LOCAL copyregs(savereg, reg, savefps, fpregs)
int savereg[], reg[];
double savefps[], fpregs[];
{
	int i;
	for(i=0; i< NGREG; i++) {
		reg[i] = savereg[i];
	}
	for(i=0; i< MAXFREG; i++) {
		fpregs[i] = savefps[i];
	}
#ifdef _POWER
#ifdef _THREADS
     reg[SYSREGNO(IAR)] = corehdr.c_mst.iar;
     reg[SYSREGNO(MSR)] = corehdr.c_mst.msr;
     reg[SYSREGNO(CR)] = corehdr.c_mst.cr;
     reg[SYSREGNO(LR)] = corehdr.c_mst.lr;
     reg[SYSREGNO(CTR)] = corehdr.c_mst.ctr;
     reg[SYSREGNO(XER)] = corehdr.c_mst.xer;
     reg[SYSREGNO(MQ)] = corehdr.c_mst.mq;
     reg[SYSREGNO(TID)] = corehdr.c_mst.tid;
     reg[SYSREGNO(FPSCR)] = corehdr.c_mst.fpscr;
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
#endif /* _POWER */
}

coredump_reading(reg, freg, signo)
int reg[];
double freg[];
short *signo;
{
	*signo = (short)corehdr.c_signo;

	stkmap.b1 = USRSTACK - (stksiz = corehdr.c_size);
	stkmap.e1   = USRSTACK;
	stkmap.f1 = (unsigned int) corehdr.c_stack;

        readobj(0,1); 

#ifdef _THREADS
      regioncnt = corehdr.c_vmregions;
      mstcnt  = corehdr.c_nmsts;
      region_ptr =  (vm_info_st *) malloc(regioncnt*(sizeof(vm_info_st)));
      mstsave_ptr = (mstsave_st *) malloc(mstcnt*(sizeof(mstsave_st)));
      mstsave_current = &corehdr.c_mst;
#endif /* _THREADS */

	/* Get the registers from the core file */
#ifdef _THREADS
                  copyregs(mstsave_current->gpr, reg,
                                                 mstsave_current->fpr,freg);
                  if ( (corehdr.c_flag & FULL_CORE) == FULL_CORE) {
                     /* if FULLCORE: initialize region_ptr and mst_ptr  */
                     /* region_ptr : vm_info structures                  */
                     /* mstsave_ptr : mstsave structures                 */
                     fseek(corefile, (long int) corehdr.c_vmm, 0);
                     fread((void *) region_ptr, (size_t) 1,
                          (size_t) regioncnt*sizeof(vm_info_st), corefile);
 
                     fseek(corefile, (long int) corehdr.c_msts, 0);
                     fread((void *) mstsave_ptr, (size_t) 1,
                             (size_t) mstcnt*sizeof(mstsave_st), corefile);
                  }

#else
	copyregs(corehdr.c_u.u_save.gpr, reg, corehdr.c_u.u_save.fpr, freg); 
#endif /* _THREADS */
}


LOCAL int create(f)
STRING f;
{
    int fd;

    if ((fd = creat(f, 0666)) != -1) {
	if (-1 == close(fd))
	    perror( "create: close()" );
	return (open(f, wtflag));
    } else
	return (-1);
}

getfile(filnam, cnt)
STRING filnam;
{
    int symfid;

    if (strcmp("-", filnam) != 0) {
	symfid = open(filnam, wtflag);
	if (symfid == -1 && argcount > cnt) {
	    if (wtflag == O_RDWR)
		symfid = create(filnam);
	    if (symfid == -1)
		perror(filnam);
	}
    } else
	symfid = -1;
#if KADB
    if (isatty(symfid)) {
	close(symfid);
	rdfd = symfid = open(filnam, 2);
	rdterm(rdfd);
    }
#endif
    return (symfid);
}

/*
 * Given a symbol, returns the space to look in.
 * I think we want to allow only external or static?
 * Is there any difference between NSYM and EXTRASYM?
 */
LOCAL int typesym(symp, i)
register SYMPTR symp;
int i;
{
    if ((symp->n_sclass != C_EXT) && (symp->n_sclass != C_STAT) &&
	(symp->n_sclass != C_AUTO) )
	return (EXTRASYM);
    if (symp->n_scnum == SymTbl[i].text_scn_num)
	return(ISYM);
    else if ((symp->n_scnum == SymTbl[i].data_scn_num) ||
             (symp->n_scnum == SymTbl[i].bss_scn_num) ||
             (symp->n_sclass == C_AUTO) )
	return(DSYM);
    else
	return(NSYM);
}

LOCAL void loadstr(hdr,i)
register FILHDR *hdr;
int i;
{
    long ssiz;
    int  issiz;
    int nbytesread = 0;

    FSEEK(txtstream, hdr->f_symptr + (txthdr.f_nsyms * SYMESZ), 0);
    nbytesread = FREAD((char *)(&ssiz), 1, sizeof(ssiz), txtstream);
    if (nbytesread <= 0) /* EOF? */
	    return;
    if (nbytesread != sizeof(ssiz)) {
	    adbpr(catgets(scmc_catd, MS_setup, M_MSG_48,
		"Error reading header of string table.\n"));
	return;
    }
    issiz = XL(ssiz)-4;
    strtab = sbrk(issiz);
    SymTbl[i].strtab = strtab;
    if (strtab == (char *)-1) {
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_49,
		"Not enough space for string table.\n"));
	return;
    }

    nbytesread = FREAD((char *)strtab, 1, (unsigned)issiz, txtstream);
    if (nbytesread != issiz) {
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_50,
	     "Warning: string table is missing or the object is stripped. \n"));
	return;
    }
}

LOCAL void loaddbg(hdr,i)
register FILHDR *hdr;
int i;
{
    char * dbgtab;

    ldsseek(txtstream, dbg_scn_num);
    dbgtab = sbrk(dbg_scn_hdr.s_size);
    SymTbl[i].dbgtab = dbgtab;
    if (dbgtab == (char *)-1) {
	adbpr(catgets(scmc_catd, MS_setup, M_MSG_51,
		"Not enough space for .debug section.\n"));
	return;
    }
    if (FREAD((char *)dbgtab, 1, dbg_scn_hdr.s_size, txtstream)
	!= dbg_scn_hdr.s_size) {
	    adbpr(catgets(scmc_catd, MS_setup, M_MSG_52,
		"can't read .debug section.\n"));
	    return;
    }
}

/* textobj - finds the object file that contains the address "adr" */
int txtfile, datfile;

int textobj(adr)
unsigned int adr;
{
	int text_object;
	txtfile = -1;
	datfile = -1;

#if DEBUG
        if (debugflag)
		adbpr("->textobj: adr = %#x, loadcnt = %#x\n", adr, loadcnt);/*dbg*/
#endif
	/*if ( rdwr_core ) return -1; */
	for(text_object = 0; text_object < loadcnt; text_object++) {
		if ( (adr >= FileMap[text_object].begin_txt) &&
		     (adr < FileMap[text_object].end_text))
			return (txtfile = text_object);
		else  {
		  if ( coredump ) {
			if ( adr >= loader_info[text_object].dataorg &&
			     adr < ( loader_info[text_object].dataorg +
				   loader_info[text_object].datasize ) )
			   return ( datfile = text_object);
		  }
		  else if ( (adr >= FileMap[text_object].data_seekadr) &&
		            (adr < (FileMap[text_object].end_data -  
				    FileMap[text_object].begin_data + 
				    FileMap[text_object].data_seekadr)))
		  	   return (datfile = text_object);
	       }
	}
	return -1;
}

readobj(file, iscoreimage)
char * file;
int iscoreimage;
{
 	ARCHDR  arhdr;
	unsigned int ldcnt = 0;
	unsigned int i;
	struct stat  filestat;
	LDFILE *ld_ptr;
	
	ldentry = (LDFILE **) calloc( loadcnt, sizeof(LDFILE *));
	for ( i = 0; i < loadcnt; i++) {
	   if ( (i != 0) || ( file == 0) ) 
		file = fd_info[i].pathname;
	   if ( ! ( ldentry[i] = ldopen(file, ldentry[i])) ){
	        adbpr( catgets(scmc_catd, MS_setup, M_MSG_41, 
						"can't open %s\n") , file);
		return;
	}
       ld_ptr = ldentry[i];
       if (fd_info[i].membername != 0 && fd_info[i].membername[0] != '\0') {
	   do {
	       if (ldahread(ld_ptr,&arhdr) == FAILURE)
	           adbpr( catgets(scmc_catd, MS_setup, M_MSG_42, 
            			"can't open member %s in file %s\n") , 
				 fd_info[i].membername, file);
	       if (streq(arhdr.ar_name,fd_info[i].membername))
		   break;
	       if (ldclose(ld_ptr) == SUCCESS)
	           adbpr( catgets(scmc_catd, MS_setup, M_MSG_42, 
				"can't open member %s in file %s\n") , 
				 fd_info[i].membername, file);
	   } while (1);
       }
    } /* end of for */
}

set_kmem_map() {
    loadcnt = 1;
    FileMap = (FILEMAP *) calloc(loadcnt, sizeof (FILEMAP));
    SaveMap = (FILEMAP *) calloc(loadcnt, sizeof (FILEMAP));
    FileMap[0].begin_txt = 0;
    FileMap[0].end_text = 0xffffffff;
    FileMap[0].txt_seekadr = 0;
    FileMap[0].begin_data = 0;
    FileMap[0].end_data = 0xffffffff;
    FileMap[0].data_seekadr = 0;
    FileMap[0].st_seekadr = 0;
    FileMap[0].fd = slshmap.ufd;
    SaveMap[0].begin_txt = FileMap[0].begin_txt;
    SaveMap[0].end_text   = FileMap[0].end_text;
    SaveMap[0].begin_data = FileMap[0].begin_data;
    SaveMap[0].end_data   = FileMap[0].end_data;
    SaveMap[0].txt_seekadr = FileMap[0].txt_seekadr;
    SaveMap[0].data_seekadr = FileMap[0].data_seekadr;
    SaveMap[0].st_seekadr  = FileMap[0].st_seekadr;
}
    
