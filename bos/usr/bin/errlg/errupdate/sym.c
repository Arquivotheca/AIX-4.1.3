static char sccsid[] = "@(#)51	1.11  src/bos/usr/bin/errlg/errupdate/sym.c, cmderrlg, bos411, 9428A410j 3/31/94 17:08:00";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: syminit, ressyminit, getsym, strlcpy, reslookup, symtokstr
 *            writemajor1, writemajor2 readmajor1, readmajor2
 *            readmajor1_init, readmajor2_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * General purpose symbol manipulation routines for lex.c
 */

#include <stdio.h>
#include <errupdate.h>
#include <parse.h>

#define S(NAME,type) \
	{ 0, type, "NAME" }

static struct symbol reserved[] = {
	S(Comment,     ICOMMENT),
	S(Class,       IERRCLASS),
	S(Report,      IREPORT),
	S(Log,         ILOG),
	S(Alert,       IALERT),
	S(Err_Type,    IERRTYPE),
	S(Err_Desc,    IERRDESC),
	S(Prob_Causes, IPROBCAUS),
	S(User_Causes, IUSERCAUS),
	S(User_Actions,IUSERACTN),
	S(Inst_Causes, IINSTCAUS),
	S(Inst_Actions,IINSTACTN),
	S(Fail_Causes, IFAILCAUS),
	S(Fail_Actions,IFAILACTN),
	S(Detail_Data, IDETAILDT),
};
#define NRESERVED sizeof(reserved)/sizeof(reserved[0])

#define NHASH 32
#define HASHIDX(name) (name[0] % NHASH)

static symbol Ressymtab[NRESERVED];
static int Nreserved;
static symbol *reshashtab[NHASH];

#define TBUFSIZE 1000
symbol Major[TBUFSIZE];
static sindex = TBUFSIZE;
static char *stringp = (char *)Major;

syminit()
{

	sindex = TBUFSIZE;
	stringp = (char *)(Major+1);
}

ressyminit()
{
	int i;

	for(i = 0; i < NRESERVED; i++)
		resinstall(&reserved[i]);
}

symbol *getsym()
{

	sindex--;
	memset(&Major[sindex],0,sizeof(Major[0]));
	if((int)stringp >= (int)&Major[sindex])
		cat_fatal(CAT_UPD_OVF,"Symbol table overflow");
	return(&Major[sindex]);
}

char *strlcpy(str)
char *str;
{
	char *cp;

	cp = stringp;
	strcpy(stringp,str);
	stringp += strlen(str)+1;
	if((int)stringp >= (int)&Major[sindex])
		cat_fatal(CAT_UPD_OVF,"Symbol table overflow");
	return(cp);
}

symbol *reslookup(name)
char *name;
{
	symbol *sp;
	int idx;

	idx = HASHIDX(name);
	for(sp = reshashtab[idx]; sp; sp = sp->s_next)
		if(streq_c(sp->s_name,name))	/* case insensitive */
			return(sp);
	return(0);
}

static symbol *resinstall(ressp)
symbol *ressp;
{
	symbol *sp;
	int idx;

	if(Nreserved == NRESERVED) {
		return(&Ressymtab[NRESERVED-1]);
	}
	sp = &Ressymtab[Nreserved++];
	*sp = *ressp;
	idx = HASHIDX(sp->s_name);
	sp->s_next = reshashtab[idx];
	reshashtab[idx] = sp;
	return(sp);
}

char *symtokstr(tok)
{
	int i;

	for(i = 0; i < NRESERVED; i++)
		if(reserved[i].s_type == tok)
			return(reserved[i].s_name);
	return(0);
}

static FILE *jtmpfile(n)
{
		return(tmpfile());
}

static FILE *Wm1fp,*Wm2fp;

struct wmajor {
	int w_magic;	/* number of bytes in strings */
	int w_strlen;	/* number of bytes in strings */
	int w_symlen;	/* number of bytes in strings */
};
#define WMAGIC 0x12345678

writemajor1()
{

	if(Wm1fp == 0 && (Wm1fp = jtmpfile(1)) == 0) {
		perror("writemajor1: tmpfile");
		genexit_nostats(1);
	}
	writemajor(Wm1fp);
}

writemajor2()
{

	if(Wm2fp == 0 && (Wm2fp = jtmpfile(2)) == 0) {
		perror("writemajor2: tmpfile");
		genexit_nostats(1);
	}
	writemajor(Wm2fp);
}

static writemajor(fp)
FILE *fp;
{
	struct wmajor wm;

	wm.w_magic  = WMAGIC;
	wm.w_strlen = (int)stringp - (int)Major;
	wm.w_symlen = (TBUFSIZE - sindex) * sizeof(symbol);
	jwrite(fp,&wm,sizeof(wm));
	jwrite(fp,Major,wm.w_strlen);
	jwrite(fp,&Major[sindex],wm.w_symlen);
}

readmajor1()
{

	return(readmajor(Wm1fp));
}

readmajor2()
{

	return(readmajor(Wm2fp));
}

/*
 * return 1 if a symbol list is read in.
 * return 0 otherwise
 */
static readmajor(fp)
FILE *fp;
{
	int n;
	int rv;
	struct wmajor wm;

	if(fp == 0)
		return(0);
	rv = jread(fp,&wm,sizeof(wm));
	if(rv == 0)
		return(0);
	if(rv != sizeof(wm) || wm.w_magic != WMAGIC)
		cat_fatal(CAT_UPD_WMAG,"Error in read from tmpfile");
	if(jread(fp,Major,wm.w_strlen) != wm.w_strlen)
		cat_fatal(CAT_UPD_WMAG,"Error in read from tmpfile");
	n = TBUFSIZE - wm.w_symlen / sizeof(symbol);
	if(jread(fp,&Major[n],wm.w_symlen) != wm.w_symlen)
		cat_fatal(CAT_UPD_WMAG,"Error in read from tmpfile");
	stringp = (char *)((int)Major + wm.w_strlen);
	sindex  = TBUFSIZE - wm.w_symlen / sizeof(symbol);
	return(1);
}

readmajor1_init()
{

	if(Wm1fp)
		fseek(Wm1fp,0,0);
}

readmajor2_init()
{

	if(Wm2fp)
		fseek(Wm2fp,0,0);
}

static jwrite(fp,buf,count)
FILE *fp;
char *buf;
{

	while(--count >= 0)
		putc(*buf++,fp);
}

extern errno;

static jread(fp,buf,count)
FILE *fp;
char *buf;
{
	int n;
	int c;

	n = 0;
	while(--count >= 0) {
		if((c = getc(fp)) == EOF) {
			if(n && errno)
				perror("getc");
			break;
		}
		*buf++ = c;
		n++;
	}
	return(n);
}

