static char sccsid[] = "@(#)55	1.8  src/bos/usr/bin/trcrpt/nm.c, cmdtrace, bos411, 9428A410j 4/1/94 05:15:57";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: rptsym_init, rptsym_install, rptsym_lookup, rptsym_nmlookup
 *            rptsym_dump, rptsym_stat, sort_init
 *
 * ORIGINS: 27
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

#define _ILS_MACROS

#include <xcoff.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <ctype.h>
#include <string.h>
#include "rpt.h"

#define UNXFILE "/unix"
#define TY_AOUT  1
#define TY_TRCNM 2
#define TY_NOSYM -1

extern char *malloc();

static char *Unxfile = UNXFILE;		/* pathname of namelist file */
static FILE *Unxfp;
static char *Unxbase;

static char *istracpy();

static rpt_allflg;

struct rptsym {
	char          *r_name;
	unsigned       r_value;
};
typedef struct rptsym rptsym;
rptsym *rptsym_lookup();
rptsym *rptsym_install();

static rptsym *getsp();

#define GETSP_INCR 256
static rptsym *Symtab_base;
static Symtab_size;
static Symtab_count;

static struct filehdr filehdr;
static struct aouthdr aouthdr;
static struct scnhdr  scnhdr;
static struct syment  syment;
static struct ldhdr   ldhdr;
static struct ldsym   ldsym;

static int filehdr_OFF;
static int aouthdr_OFF;
static int scnhdr_OFF;
static int syment_OFF;
static int ldhdr_OFF;

static char *syment_BASE;
static char *ldsym_BASE;
static char *symflx_BASE;				/* flexnames for symbol table */
static char *ldrflx_BASE;				/* flexnames for loader symbols */
static int   symflx_SIZE;
static int   ldrflx_SIZE;

#define filehdr_SIZE FILHSZ                    /* 20 */
#define aouthdr_SIZE (sizeof(struct aouthdr))  /* 48 */
#define scnhdr_SIZE  SCNHSZ                    /* 40 */
#define syment_SIZE  SYMESZ                    /* 18 */
#define ldhdr_SIZE   LDHDRSZ                   /* 32 */
#define ldsym_SIZE   LDSYMSZ                   /* 24 */

#define IOFF(STRUCT,IDX) \
	( (STRUCT/**/_OFF) + STRUCT/**/_SIZE * (IDX) )

#define syment_cpy(i) \
	memcpy(&syment,syment_BASE + i * syment_SIZE, syment_SIZE)

#define ldsym_cpy(i) \
	memcpy(&ldsym,ldsym_BASE + i * ldsym_SIZE, ldsym_SIZE)

rptsym_init(filename,allflg)
char *filename;
{
	struct stat statbuf;

	vprint("Initializing namelist symbol tables\n");
	if(filename)
		Unxfile = filename;
	if((Unxfp = fopen(Unxfile,"r")) == 0) {
		vprint("Cannot open %s. %s\n",Unxfile,errstr());
		return;
	}
	if(stat(Unxfile,&statbuf)) {
		vprint("Cannot stat %s. %s\n",Unxfile,errstr());
		return;
	}
	if((statbuf.st_mode & S_IFMT) != S_IFREG) {
		vprint("%s is not a regular file\n",Unxfile);
		return;
	}

	Unxbase = shmat(fileno(Unxfp),0,SHM_MAP|SHM_RDONLY);
	if((int)Unxbase == -1) {
		vprint("map %s: %s\n",Unxfile,errstr());
		Unxbase = 0;
	}

	/*
	 * check for trcnm format
	 */
	switch(typecheck()) {
	case TY_AOUT:
		rptsym_init_aout(allflg);
		break;
	case TY_TRCNM:
		rptsym_init_trcnm(allflg);
		break;
	default:
		return;
	}
}

static typecheck()
{
	char line[512];
	struct filehdr hdr;
	char *value_c,*name;

	jread(&hdr,0,sizeof(hdr));
	if(0x0100 < hdr.f_magic && hdr.f_magic < 0x0200)
		return(TY_AOUT);
	vprint("%s: bad magic number %04X\n",Unxfile,hdr.f_magic);
	jread(line,0,sizeof(line));
	line[sizeof(line)-1] = '\0';
	name    = strtok(line," \t\n");
	value_c = strtok(0,   " \t\n");
	if(value_c == 0 || name == 0 || numchk(value_c) < 0)
		return(TY_NOSYM);
	return(TY_TRCNM);
}

static numchk(str)
char *str;			/* number */
{
	int c;
	char *cp;

	cp = str;
	if(*cp == '0' && ((c = cp[1]) == 'x' || c == 'X'))
		cp += 2;
	while(c = *cp++)
		if(!isxdigit(c))
			return(-1);
	return(1);
}

static rptsym_init_trcnm(allflg)
{
	int i;
	char *value_c,*name;
	int value;
	char line[512];
	
	fseek(Unxfp,0,0);
	while(fgets(line,sizeof(line),Unxfp)) {
		switch(line[0]) {
		case '*':
		case '#':
		case ' ':
		case '\t':
		case '\n':
		case '\0':
			continue;
		}
		name    = strtok(line," \t");
		value_c = strtok(0,   " \t");
		rptsym_install(name,strtol(value_c,0,16));
	}
}

static rptsym_init_aout(allflg)
{
	int i;

	filehdr_OFF = 0;
	aouthdr_OFF = filehdr_SIZE;
	jread(&filehdr,filehdr_OFF,sizeof(filehdr));
	jread(&aouthdr,aouthdr_OFF,sizeof(aouthdr));
	scnhdr_OFF  = filehdr_SIZE + aouthdr_SIZE;
	syment_OFF  = filehdr.f_symptr;

	if(filehdr.f_nsyms > 0)
		jread(&symflx_SIZE,IOFF(syment,filehdr.f_nsyms),4);

	Debug("nscns=%d\n",filehdr.f_nscns);
	for(i = 0; i < filehdr.f_nscns; i++) {	/* look for loader section */
		jread(&scnhdr,IOFF(scnhdr,i),sizeof(scnhdr));
		Debug("sections=%8.8s\n",scnhdr.s_name);
		if((scnhdr.s_flags & (STYP_LOADER|STYP_TYPCHK)) == STYP_LOADER) {
			ldhdr_OFF  = scnhdr.s_scnptr;
			jread(&ldhdr,ldhdr_OFF,sizeof(ldhdr));
			ldrflx_SIZE = ldhdr.l_stlen;
		}
	}
	rpt_allflg = allflg;
	/*
	 * Allocate symbol table
	 */
	base_init();		/* set _BASE and (conditionally) allocate memory */
	syminit();			/* install symbol table */
	ldrinit();			/* install ldr symbols */
	base_term();		/* free Unxfile allocations or mapped file */
	sort_init();		/* sort the internal symbol table */
	fclose(Unxfp);
}

static syminit()
{
	int i;
	char *name;
	char buf[10];

	Debug("syminit\n");
	for(i = 0; i < filehdr.f_nsyms; i++) {
		syment_cpy(i);
		if(syment.n_zeroes == 0) {
			name = symflx_BASE + syment.n_offset;
		} else {
			strncpy(buf,syment.n_name,8);
			buf[8] = '\0';
			name = buf;
		}
		if(name[0] == '.' || syment.n_sclass != C_EXT)
			continue;
		rptsym_install(name,syment.n_value);
		i += syment.n_numaux;		/* skip auxent */
	}
	Debug("return from syminit\n");
}

static ldrinit()
{
	int i;
	char *name;
	rptsym *sp;
	char buf[10];

	Debug("ldrinit\n");
	if(ldhdr_OFF == 0)
		return;
	for(i = 0; i < ldhdr.l_nsyms; i++) {
		ldsym_cpy(i);
		if(ldsym.l_zeroes == 0) {
			name = ldrflx_BASE + ldsym.l_offset;
		} else {
			strncpy(buf,ldsym.l_name,8);
			buf[8] = '\0';
			name = buf;
		}
		switch(ldsym.l_smclas) {		/* install SVC's */
		default:
			if(!rpt_allflg)
				break;
		case XMC_SV:
			if((sp = rptsym_lookup(ldsym.l_value)) && streq(sp->r_name,name))
				break;
			rptsym_install(name,ldsym.l_value);
			break;
		}
	}
	Debug("return from ldrinit\n");
}

rptsym_stat()
{

	return;
}

rptsym_dump()
{
	int i;
	rptsym *sp;

	for(i = 0; i < Symtab_count; i++) {
		sp = &Symtab_base[i];
		/* printing data */
		if(printf("%-20s %08X\n",sp->r_name,sp->r_value) <= 0){
			perror("printf");
			genexit(1);
		}
	}
}

/*
 * NAME:     rptsym_nmlookup
 * FUNCTION: lookup the name in the symbol table by its symbol value
 * INPUTS:   
 */
char *rptsym_nmlookup(value,approxflg)
{
	rptsym *sp;

	if(sp = rptsym_lookup(value,approxflg))
		return(sp->r_name);
	return(0);
}

rptsym *rptsym_lookup(value,approxflg)
{
	register n,ln,rn;
	register tvalue;

	ln = 0;
	rn = Symtab_count;
	for(;;) {
		n = (ln + rn)/2;
		tvalue = Symtab_base[n].r_value;
		if(value < tvalue) {
			if(rn == n)
				break;
			rn = n;
			continue;
		}
		if(value > tvalue) {
			if(ln == n)
				break;
			ln = n;
			continue;
		}
		Debug("match %d\n",n);
		return(&Symtab_base[n]);
	}
	return(approxflg ? &Symtab_base[n] : 0);
}

rptsym *rptsym_install(name,value)
char *name;
{
	register rptsym *sp;

	sp = getsp();
	sp->r_value = value;
	sp->r_name  = istracpy(name);
	return(sp);
}

#define RSYMSIZE sizeof(rptsym)

static rptsym *getsp()
{
	rptsym *sp;

	if(Symtab_base == 0) {
		Symtab_base = (rptsym *)jalloc(GETSP_INCR * RSYMSIZE);
		Symtab_size = GETSP_INCR;
	} else if(Symtab_count == Symtab_size) {
		Symtab_size += GETSP_INCR;
		Symtab_base = (rptsym *)realloc(Symtab_base,Symtab_size * RSYMSIZE);
	}
	return(&Symtab_base[Symtab_count++]);
}

#define ASTRCPY_INCR 128
static char *istracpy_base;
static istracpy_idx;

static char *istracpy(str)
char *str;
{
	int len;
	char *cp;

	len = strlen(str) + 1;
	if(len > ASTRCPY_INCR) {
		str[ASTRCPY_INCR-1] = 0;
		len = ASTRCPY_INCR;
	}
	if(istracpy_base == 0 || len + istracpy_idx > ASTRCPY_INCR) {
		istracpy_idx  = 0;
		istracpy_base = jalloc(ASTRCPY_INCR);
	}
	cp = &istracpy_base[istracpy_idx];
	istracpy_idx += len;
	strcpy(cp,str);
	return(cp);
}

static jread(buf,offset,count)
char *buf;
{

	Debug("jread(%x,%x,%x)\n",buf,offset,count);
	if(count == 0)
		return;
	if(Unxbase) {
		memcpy(buf,Unxbase + offset,count);
		return;
	}
	fseek(Unxfp,offset,0);
	while(--count >= 0)
		*buf++ = getc(Unxfp);
}

static base_init()
{

	if(Unxbase) {
		syment_BASE = Unxbase + IOFF(syment,0);
		symflx_BASE = Unxbase + IOFF(syment,filehdr.f_nsyms);
		ldsym_BASE  = Unxbase + IOFF(ldhdr,1);
		ldrflx_BASE = Unxbase + ldhdr_OFF + ldhdr.l_stoff;
	} else {
		vprint("reading symbol table\n");
		syment_BASE = jalloc(filehdr.f_nsyms * syment_SIZE);
		jread(syment_BASE,IOFF(syment,0),filehdr.f_nsyms * syment_SIZE);

		vprint("reading flexname strings\n");
		symflx_BASE = jalloc(symflx_SIZE+4);
		jread(symflx_BASE,IOFF(syment,filehdr.f_nsyms),symflx_SIZE);

		/*
		 * Allocate loader symbol table
		 */
		vprint("reading ldr symbol table\n");
		ldsym_BASE = jalloc(ldhdr.l_nsyms * ldsym_SIZE);
		jread(ldsym_BASE,IOFF(ldhdr,1),ldhdr.l_nsyms * ldsym_SIZE);

		vprint("reading ldr flexname strings\n");
		ldrflx_BASE = jalloc(ldhdr.l_stlen + 4);
		jread(ldrflx_BASE,ldhdr_OFF + ldhdr.l_stoff,ldrflx_SIZE);
	}
}

static base_term()
{

	if(Unxbase) {
		shmdt(Unxbase);
		Unxbase = 0;
	} else {
		if(syment_BASE)
			free(syment_BASE);
		if(symflx_BASE)
			free(symflx_BASE);
		if(ldsym_BASE)
			free(ldsym_BASE);
		if(ldrflx_BASE)
			free(ldrflx_BASE);
		syment_BASE = 0;
		symflx_BASE = 0;
		ldsym_BASE  = 0;
		ldrflx_BASE = 0;
	}
}

static symcmp();

sort_init()
{

	qsort(Symtab_base,Symtab_count,sizeof(rptsym),symcmp);
}

static symcmp(sp1,sp2)
rptsym *sp1,*sp2;
{

	if(sp1->r_value < sp2->r_value)
		return(-1);
	if(sp1->r_value > sp2->r_value)
		return(+1);
	return(0);
}

