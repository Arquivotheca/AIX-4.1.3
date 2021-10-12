static char sccsid[] = "@(#)98 1.13 src/bos/usr/bin/sccs/comb.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:48";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: clean_up, comb, enter, escdodelt, fredck, getpred,
 *            main, prtget
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include 	<locale.h>
#include        <nl_types.h>
#include	"defines.h"
#include	"had.h"
#include        "comb_msg.h"
#include 	<stdlib.h>
#include	<errno.h>
#define MSGSTR(Num, Str) catgets(catd, MS_COMB, Num, Str)

static struct stat Statbuf;

static struct packet gpkt;
static struct sid sid;
static int	num_files;
static int	Do_prs;
static char	had[26];
static char	*clist;
static char	*Val_ptr;
static char	Blank[]    =    " ";
static int	*Cvec;
static int	Cnt;
static FILE	*iop;
char    *fmalloc();

nl_catd catd;

main(argc,argv)
int argc;
register char *argv[];
{
	int c;
	char *sid_ab();
	extern comb();
	extern int Fcnt;

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	while ((c = getopt(argc, argv, "c:op:s")) != EOF)
	{
		switch (c) {

		case 'p':
			chksid(sid_ab(optarg,&sid),&sid);
			break;
		case 'c':
			clist = optarg;
			break;
		case 'o':
		case 's':
			break;
		default:
			fprintf(stderr,MSGSTR(COMB_USAGE, 
                       "Usage: comb [-o] [-s] [-p SID] [-c List] File\n"));
			exit(1);
		}

		if (had[c - 'a']++) {
			fprintf(stderr,MSGCM(KEYLTRTWC, "\nUse the -%c flag only once on the command line. (cm2)\n"),c);  /* MSG */
		}
	}

	if((num_files = argc - optind) == 0)
		fatal(MSGCM(MISSFLNAM, "\nSpecify the file to process.  (cm3)\n"			));  /* MSG */
	if (HADP && HADC)
		fatal(MSGSTR(NOPANDC, "\nThe -p flag and the -c flag are mutually exclusive. (cb2)\n"));  /* MSG */
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	iop = stdout;
	for ( ; optind < argc; optind++)
		if (argv[optind])
			do_file(argv[optind],comb);
	if (fclose(iop)) {
		perror("comb");
		exit(1);
	}
	exit(Fcnt ? 1 : 0);
}


static comb(file)
char *file;
{
	register int i, n;
	register struct idel *rdp;
	struct idel *dodelt();
	char *p;
	char *auxf();
	char rarg[32], *sid_ba();
	int succnt;
	struct sid *sp, *prtget();
	extern char had_dir, had_standinp;
	extern char *Sflags[];
	struct stats stats;

	if (setjmp(Fjmp))
		return;
	sinit(&gpkt, file, 1);
	gpkt.p_verbose = -1;
	gpkt.p_stdout = stderr;
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (exists(auxf(gpkt.p_file, 'p')))
		fatal(MSGSTR(PFILEXSTS, "\nCannot use the comb command while there is an outstanding\n\
\tdelta to the file.\n\
\tUse the delta command or the unget command; then use the comb command. (cb1)\n"));  /* MSG */

	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);

	Cvec = (int *) fmalloc(n = ((maxser(&gpkt) + 1) * sizeof(*Cvec)));
	zero((char *)Cvec, n);
	Cnt = 0;

	if (HADP) {
		rdp = gpkt.p_idel;
		if (!(n = sidtoser(&sid, &gpkt)))
			fatal(MSGCM(SIDNOEXIST, "\nThe SID you specified does not exist. \n\
\tUse the sact command to check the p-file for existing SID numbers. (cm20)\n"));  /* MSG */
		while (n <= maxser(&gpkt)) {
			if (rdp[n].i_sid.s_rel == 0 &&
			    rdp[n].i_sid.s_lev == 0 &&
			    rdp[n].i_sid.s_br == 0  &&
			    rdp[n].i_sid.s_seq == 0) {
				n++;
				continue;
			}
			Cvec[Cnt++] = n++;
		}
	}
	else if (HADC) {
		dolist(&gpkt, clist, 0);
	}
	else {
		rdp = gpkt.p_idel;
		for (i = 1; i <= maxser(&gpkt); i++) {
			succnt = 0;
			if (rdp[i].i_sid.s_rel == 0 &&
			    rdp[i].i_sid.s_lev == 0 &&
			    rdp[i].i_sid.s_br == 0  &&
			    rdp[i].i_sid.s_seq == 0)
				continue;
			for (n = i + 1; n <= maxser(&gpkt); n++)
				if (rdp[n].i_pred == i)
					succnt++;
			if (succnt != 1)
				Cvec[Cnt++] = i;
		}
	}
	finduser(&gpkt);
	doflags(&gpkt);
	fclose(gpkt.p_iop);
	gpkt.p_iop = 0;
	if (!Cnt)
		fatal(MSGSTR(NOTHTODO, "\nThe flags specified on the command line would create no changes\n\
\tto the SCCS file. (cb4)\n"));  /* MSG */
	rdp = gpkt.p_idel;
	Do_prs = 0;
	sp = prtget(rdp, Cvec[0], iop, gpkt.p_file);
	sid_ba(sp,rarg);
	if (!(Val_ptr = Sflags[VALFLAG - 'a']))
		Val_ptr = Blank;
	fprintf(iop, "admin -iCOMB$$ -r%s -fv%s -m '-yThis was COMBined' s.COMB$$\n", rarg,Val_ptr);
	Do_prs = 1;
	fprintf(iop, "rm -f COMB$$\n");
	for (i = 1; i < Cnt; i++) {
		n = getpred(rdp, Cvec, i);
		if (HADO)
			fprintf(iop, "get -s -r%d -g -e -t s.COMB$$\n",
				rdp[Cvec[i]].i_sid.s_rel);
		else
			fprintf(iop, "get -s -a%d -r%d -g -e s.COMB$$\n",
				n + 1, rdp[Cvec[i]].i_sid.s_rel);
		prtget(rdp, Cvec[i], iop, gpkt.p_file);
		fprintf(iop, "delta -s -m\"$b\" -y\"$a\" s.COMB$$\n");
	}
	fprintf(iop, "sed -n '/^%c%c$/,/^%c%c$/p' %s >comb$$\n",
		CTLCHAR, BUSERTXT, CTLCHAR, EUSERTXT, gpkt.p_file);
	fprintf(iop, "ed - comb$$ <<\\!\n");
	fprintf(iop, "1d\n");
	fprintf(iop, "$c\n");
	fprintf(iop, MSGSTR(DELTAMSG,"*** DELTA TABLE PRIOR TO COMBINE ***\n"));
	fprintf(iop, ".\n");
	fprintf(iop, "w\n");
	fprintf(iop, "q\n");
	fprintf(iop, "!\n");
	fprintf(iop, "prs -e %s >>comb$$\n", gpkt.p_file);
	fprintf(iop, "admin -tcomb$$ s.COMB$$\\\n");
	for (i = 0; i < NFLAGS; i++)
		if (p = Sflags[i])
			fprintf(iop, " -f%c%s\\\n", i + 'a', p);
	fprintf(iop, "\n");
	fprintf(iop, "sed -n '/^%c%c$/,/^%c%c$/p' %s >comb$$\n",
		CTLCHAR, BUSERNAM, CTLCHAR, EUSERNAM, gpkt.p_file);
	fprintf(iop, "ed - comb$$ <<\\!\n");
	fprintf(iop, "v/^%c/s/.*/ -a& \\\\/\n", CTLCHAR);
	fprintf(iop, "1c\n");
	fprintf(iop, "admin s.COMB$$\\\n");
	fprintf(iop, ".\n");
	fprintf(iop, "$c\n");
	fprintf(iop, "\n");
	fprintf(iop, ".\n");
	fprintf(iop, "w\n");
	fprintf(iop, "q\n");
	fprintf(iop, "!\n");
	fprintf(iop, ". comb$$\n");
	fprintf(iop, "rm comb$$\n");
	if (!HADS) {
		fprintf(iop, "rm -f %s\n", gpkt.p_file);
		fprintf(iop, "mv s.COMB$$ %s\n", gpkt.p_file);
		if (!Sflags[VALFLAG - 'a'])
			fprintf(iop, "admin -dv %s\n", gpkt.p_file);
	}
	else {
		fprintf(iop, "set `ls -st s.COMB$$ %s`\n",gpkt.p_file);
		fprintf(iop, "c=`expr 100 - 100 '*' $1 / $3`\n");
		fprintf(iop, "echo '%s\t' ${c}'%%\t' $1/$3\n", gpkt.p_file);
		fprintf(iop, "rm -f s.COMB$$\n");
	}
}


enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	Cvec[Cnt++] = n;
}


static struct sid *
prtget(idp, ser, fptr, file)
struct idel *idp;
int ser;
FILE *fptr;
char *file;
{
	char buf[32], *sid_ba();
	struct sid *sp;

	sid_ba(sp = &idp[ser].i_sid, buf);
	fprintf(fptr, "get -s -k -r%s -p %s > COMB$$\n", buf, file);
	if (Do_prs) {
		fprintf(fptr, "a=`prs -r%s -d:C: %s`\n",buf,file);
		fprintf(fptr, "b=`prs -r%s -d:MR: %s`\n",buf,file);
	}
	return(sp);
}


static getpred(idp, vec, i)
struct idel *idp;
int *vec;
int i;
{
	int ser, pred, acpred;

	ser = vec[i];
	while (--i) {
		pred = vec[i];
		for (acpred = idp[ser].i_pred; acpred; acpred = idp[acpred].i_pred)
			if (pred == acpred)
				break;
		if (pred == acpred)
			break;
	}
	return(i);
}


static clean_up(n)
{
	ffreeall();
}


escdodelt()	/* dummy for dodelt() */
{
}
