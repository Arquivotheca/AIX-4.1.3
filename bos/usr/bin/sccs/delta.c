static char sccsid[] = "@(#)94  1.20  src/bos/usr/bin/sccs/delta.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:17";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: after, before, clean_up, delete, delta, dodiff, enter,
 *            escdodelt, fgetchk, fredck, getdiff, insert, linerange,
 *            mkdelt, mkixg, putcmrs, putmrs, rddiff, rdpfile, skipline,
 *            main
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
#include 	<nl_types.h>
#include 	<sys/access.h>
#include 	<errno.h>
#include	"defines.h"
#include	"had.h"

#include 	"delta_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_DELTA, Num, Str)

static struct stat Statbuf;
static char Null[1];
static char ErrMsg[512];

static char	Diffpgm[]   =   "/usr/bin/diff";
static FILE	*Diffin;
static FILE	*Gin;
static int	Debug = 0;
static struct packet gpkt;
static struct sid sid;
static int	num_files;
static char	had[26];
static char	*ilist, *elist, *glist;
char    *Comments;
char	*Mrs;
char	*auxf(), *logname(), *sid_ba();
static int verbosity;
int	Did_id;
static long	Szqfile;
static char	Pfilename[FILESIZE];
FILE	 *fdfopen();
extern FILE	*Xiop;
extern int	Xcreate;

nl_catd catd;

/* _AMBIG is the default message string for the AMBIG catalog message */
#define _AMBIG "delta: 1255-095  There is more than one outstanding delta.\n\
\tSpecify the SID number that will be created. (de15)\n"

/*ILL_DATA is the default message string for the ILLDAT catalog message */
#define ILL_DATA "delta: 1255-097  The SOH character is in the first position of line %1$d of file %2$s.\n\
\tRemove this character or precede it with a \\ (backslash).(de14)"
main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char c;
	char *sid_ab();

	extern delta();
	extern int Fcnt;

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && (c=argv[i][1])) {
			if (!strcmp(argv[i], "--")) {  /* got -- argument */
				num_files+=argc-(i+1);
				p = argv[i+1];
				break;
			}
			p = &argv[i][2];
nextopt:
			switch (c) {
			case 'r':
				if (!p[0]) {
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				chksid(sid_ab(p,&sid),&sid);
				break;
			case 'g':
				if (!p[0]) {
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				glist = p;
				break;
			case 'y':
				savecmt(p);
				break;
			case 'm':
				if (!p[0]) {
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				Mrs = p;
				repl(Mrs,'\n',' ');
				break;
			case 'p':
			case 'n':
			case 's':
				if (had[c - 'a']++) {
					sprintf(ErrMsg,MSGCM(KEYLTRTWC, 
						"delta: Use the -%c flag only once on the command line. (cm2)\n"),c);
					fatal(ErrMsg);
				}
				/*
				 * Since the -p, -n and -s options do not take arguments,
				 * they must allow for other options to be grouped behind them.
				 */
				if (*p) {
					c = *p;
					p++;
					goto nextopt;
				} else {		/* nothing follows option, so  */
					continue;	/* continue with next argument */
				}
			default:
				printf(MSGSTR(DELTA_USAGE, "Usage: delta \
[-rSID -s -n -glist -p] [-m[mrlist]] [-y[comment]] files\n"));
				sprintf(ErrMsg,MSGCM(UNKKEYLTR, "delta: Flag -%c is not valid.  Select a flag recognized\n\
\tby the command. (cm1)\n"),c);  /* MSG */
				fatal(ErrMsg);
			}

			if (had[c - 'a']++) {
				sprintf(ErrMsg,MSGCM(KEYLTRTWC, "delta: Use the -%c flag only once on the command line. (cm2)\n"),c);  /* MSG */
				fatal(ErrMsg);
			}
		}
		else num_files++;
	} /* end of for loop */
	if(num_files == 0)
		fatal(MSGCM(MISSFLNAM, "delta: Specify the file to process.  (cm3)\n"));  /* MSG */
	if (!HADS)
		verbosity = -1;
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=(argc-num_files); i<argc; i++)
			do_file(argv[i],delta);
	exit(Fcnt ? 1 : 0);
}


static delta(file)
char *file;
{
	int n, linenum;
	char type;
	register int ser;
	extern char had_dir, had_standinp;
	extern char *Sflags[];
	char dfilename[FILESIZE];
	char gfilename[FILESIZE];
	char line[LINE_MAX+1];
	char *getline();
	FILE  *dodiff();
	struct stats stats;
	struct pfile *pp, *rdpfile();
	struct stat sbuf;
	struct idel *dodelt();
	int inserted, deleted, orig;
	int newser;
	int status;
	int diffloop;

	extern char	*acl_get();
	char 		*sptr;

	if (setjmp(Fjmp))
		return;
	sinit(&gpkt,file,1);
	if (lockit(auxf(gpkt.p_file,'z'),2,getpid()))
		fatal(MSGCM(LOCKCREAT, "delta: Cannot lock the specified file.\n\
\tCheck path name and permissions or \n\
\twait until the file is not in use. (cm4)\n"));  /* MSG */
	gpkt.p_reopen = 1;
	gpkt.p_stdout = stdout;
	copy(auxf(gpkt.p_file,'g'),gfilename);

	fgetchk(gfilename, &gpkt);

	Gin = xfopen(gfilename,0);
	pp = rdpfile(&gpkt,&sid);
	gpkt.p_cutoff = pp->pf_date;
	ilist = pp->pf_ilist;
	elist = pp->pf_elist;

	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);
	if ((ser = sidtoser(&pp->pf_gsid,&gpkt)) == 0 ||
		sidtoser(&pp->pf_nsid,&gpkt))
			fatal(MSGCO(BADPFILE, "delta: The p-file is damaged.\n\
\tRestore a backup copy. (co17)\n"));  /* MSG */
	doie(&gpkt,ilist,elist,glist);
	setup(&gpkt,ser);
	finduser(&gpkt);
	doflags(&gpkt);
	gpkt.p_reqsid = pp->pf_nsid;
	permiss(&gpkt);
	flushto(&gpkt,EUSERTXT,1);
	gpkt.p_chkeof = 1;
	copy(auxf(gpkt.p_file,'d'),dfilename);
	gpkt.p_gout = xfcreat(dfilename,0444);
	while(readmod(&gpkt)) {
		if(fputs(gpkt.p_line,gpkt.p_gout)==EOF)
			FAILPUT;
	}
	if (fclose(gpkt.p_gout) && /* Disk full? D63372 */
		(errno == ENOSPC || errno == EDQUOT) ){
			perror("");
			clean_up(0);
			exit(1);
	}

	orig = gpkt.p_glnno;
	gpkt.p_glnno = 0;
	gpkt.p_verbose = verbosity;
	Did_id = 0;
	while (fgets(line,LINE_MAX+1,Gin) != NULL &&
			 !chkid(line,Sflags['i'-'a']))
		;
	fclose(Gin);
	dohist();
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (!Did_id)
		if (Sflags[IDFLAG - 'a'])
			if(!(*Sflags[IDFLAG - 'a']))
				fatal(MSGCM(NOIDKEYWRDS, "delta: The file must contain SCCS identification keywords.\n\
\tInsert one or more SCCS identification keywords into the file. (cm6)\n"));  /* MSG */
			else
				fatal(MSGCM(INVIDKYWRDS, "delta: The SCCS file requires one or more specific\n\
\tidentification keywords.\n\
\tAdd the keywords to the file. (cm10)\n"));  /* MSG */
		else if (gpkt.p_verbose)
			fprintf(stderr,MSGCM(NOIDKEYWRDS7, "delta: There are no SCCS identification keywords in the file. (cm7)\n"));  /* MSG */

diffloop = 0;
	inserted = deleted = 0;
	gpkt.p_glnno = 0;
	gpkt.p_upd = 1;
	gpkt.p_wrttn = 1;
	getline(&gpkt);
	gpkt.p_wrttn = 1;
	newser = mkdelt(&gpkt,&pp->pf_nsid,&pp->pf_gsid,
					diffloop,orig);
	diffloop = 1;
	flushto(&gpkt,EUSERTXT,0);
	Diffin = dodiff(auxf(gpkt.p_file,'g'),dfilename);
	while (n = getdiff(&type,&linenum)) {
		if (type == INS) {
			inserted += n;
			insert(&gpkt,linenum,n,newser);
		}
		else {
			deleted += n;
			delete(&gpkt,linenum,n,newser);
		}
	}
	fclose(Diffin);
	if (gpkt.p_iop)
		while (readmod(&gpkt))
			;
	wait(&status);
		/*
		Check top byte (exit code of child).
		*/
		if (((status >> 8) & 0377) == 32) { /* 'execl' failed */
			sprintf(ErrMsg,MSGCO(CANTEXEC, "delta: Cannot execute %s.\n\
\tCheck path name and permissions or\n\
\tuse local problem reporting procedures. (co50)\n"),Diffpgm);  /* MSG */
			fatal(ErrMsg);
		}
	fgetchk(gfilename,&gpkt);
	unlink(dfilename);
	stats.s_ins = inserted;
	stats.s_del = deleted;
	stats.s_unc = orig - deleted;
	if (gpkt.p_verbose) {
		fprintf(gpkt.p_stdout,MSGSTR(INSM,"%u inserted\n"),stats.s_ins);
		fprintf(gpkt.p_stdout,MSGSTR(DELM,"%u deleted\n"),stats.s_del);
		fprintf(gpkt.p_stdout,MSGSTR(UNC,"%u unchanged\n"),stats.s_unc);
	}
	flushline(&gpkt,&stats);
	stat(gpkt.p_file,&sbuf);
	sptr = acl_get(gpkt.p_file);
	rename(auxf(gpkt.p_file,'x'),gpkt.p_file);
	acl_put(gpkt.p_file, sptr, 1);
	chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
	if (Szqfile)
		rename(auxf(gpkt.p_file,'q'),Pfilename);
	else {
		xunlink(Pfilename);
		xunlink(auxf(gpkt.p_file,'q'));
	}
	clean_up(0);
	if (!HADN) {
		int i;

		fflush(gpkt.p_stdout);
		if ((i = fork()) < 0)
			fatal(MSGCO(CANTFORK, "delta: Cannot create another process at this time.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));  /* MSG */
		if (i == 0) {
			setuid(getuid());
			setgid(getgid());
			unlink(gfilename);
			exit(0);
		}
		else {
			wait(&status);
		}
	}
}


static mkdelt(pkt,sp,osp,diffloop,orig_nlines)
struct packet *pkt;
struct sid *sp, *osp;
int diffloop;
int orig_nlines;
{
	extern long Timenow;
	struct deltab dt;
	char str[BUFSIZ];
	char *del_ba(), *strncpy();
	int newser;
	extern char *Sflags[];
	register char *p;
	int ser_inc, opred, nulldel;

	if (!diffloop && pkt->p_verbose) {
		sid_ba(sp,str);
		fprintf(pkt->p_stdout,"%s\n",str);
		fflush(pkt->p_stdout);
	}
	sprintf(str,"%c%c00000\n",CTLCHAR,HEAD);
	putline(pkt,str);
	newstats(pkt,str,"0");
	dt.d_sid = *sp;

	/*
	Check if 'null' deltas should be inserted
	(only if 'null' flag is in file and
	releases are being skipped) and set
	'nulldel' indicator appropriately.
	*/
	if (Sflags[NULLFLAG - 'a'] && (sp->s_rel > osp->s_rel + 1) &&
			!sp->s_br && !sp->s_seq &&
			!osp->s_br && !osp->s_seq)
		nulldel = 1;
	else
		nulldel = 0;
	/*
	Calculate how many serial numbers are needed.
	*/
	if (nulldel)
		ser_inc = sp->s_rel - osp->s_rel;
	else
		ser_inc = 1;
	/*
	Find serial number of the new delta.
	*/
	newser = dt.d_serial = maxser(pkt) + ser_inc;
	/*
	Find old predecessor's serial number.
	*/
	opred = sidtoser(osp,pkt);
	if (nulldel)
		dt.d_pred = newser - 1;	/* set predecessor to 'null' delta */
	else
		dt.d_pred = opred;
	dt.d_datetime = Timenow;
	strncpy(dt.d_pgmr,logname(),LOGSIZE-1);
	dt.d_type = 'D';
	del_ba(&dt,str);
	putline(pkt,str);
	if (ilist)
		mkixg(pkt,INCLUSER,INCLUDE);
	if (elist)
		mkixg(pkt,EXCLUSER,EXCLUDE);
	if (glist)
		mkixg(pkt,IGNRUSER,IGNORE);
	if (Mrs) {
		if (!(p = Sflags[VALFLAG - 'a']))
			fatal(MSGCM(MRNOTALD, "delta: The SCCS file you specified does not allow MR numbers. (cm24)\n"));  /* MSG */
		if (*p && !diffloop && valmrs(pkt,p))
			fatal(MSGCM(INVMRS, "delta: Use a valid MR number or numbers. (cm25)\n"));  /* MSG */
		putmrs(pkt);
	}
	else if (Sflags[VALFLAG - 'a'])
		fatal(MSGCM(MRSREQ, "delta: Specify an MR number or numbers on the command line or as\n\
\tstandard input. (cm26)\n"));  /* MSG */
	putline(pkt,Comments);
	sprintf(str,CTLSTR,CTLCHAR,EDELTAB);
	putline(pkt,str);
	if (nulldel)			/* insert 'null' deltas */
		while (--ser_inc) {
			sprintf(str,"%c%c %s/%s/%05u\n", CTLCHAR, STATS,
				"00000", "00000", orig_nlines);
			putline(pkt,str);
			dt.d_sid.s_rel -= 1;
			dt.d_serial -= 1;
			if (ser_inc != 1)
				dt.d_pred -= 1;
			else
				dt.d_pred = opred;	/* point to old pred */
			del_ba(&dt,str);
			putline(pkt,str);
			sprintf(str,"%c%c ",CTLCHAR,COMMENTS);
			putline(pkt,str);
			putline(pkt,MSGSTR(AUTONDELTA,"AUTO NULL DELTA\n"));
			sprintf(str,CTLSTR,CTLCHAR,EDELTAB);
			putline(pkt,str);
		}
	return(newser);
}


static mkixg(pkt,reason,ch)
struct packet *pkt;
int reason;
char ch;
{
	int n;
	char str[LINE_MAX+1];

	sprintf(str,"%c%c",CTLCHAR,ch);
	putline(pkt,str);
	for (n = maxser(pkt); n; n--) {
		if (pkt->p_apply[n].a_reason == reason) {
			sprintf(str," %u",n);
			putline(pkt,str);
		}
	}
	putline(pkt,"\n");
}


# define	LENMR	60

static putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char str[LENMR+6];
	extern char **Varg;

	for (argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


static struct pfile *
rdpfile(pkt,sp)
register struct packet *pkt;
struct sid *sp;
{
	char *user;
	struct pfile pf;
	static struct pfile goodpf;
	char line[LINE_MAX+1];
	int cnt, uniq;
	FILE *in, *out;

	uniq = cnt = -1;
	user = logname();
	zero((char *)&goodpf,sizeof(goodpf));
	in = xfopen(auxf(pkt->p_file,'p'),0);
	out = xfcreat(auxf(pkt->p_file,'q'),0644);
	while (fgets(line,LINE_MAX+1,in) != NULL) {
		pf_ab(line,&pf,1);
		if (equal(pf.pf_user,user)) {
			if (sp->s_rel == 0) {
				if (++cnt) {
					fclose(out);
					fclose(in);
					fatal(MSGSTR(MISSRKYLTR, "delta: Specify an SID with the -r flag. (de1)\n"));  /* MSG */
				}
				goodpf = pf;
				continue;
			}
			else if ((sp->s_rel == pf.pf_nsid.s_rel &&
				sp->s_lev == pf.pf_nsid.s_lev &&
				sp->s_br == pf.pf_nsid.s_br &&
				sp->s_seq == pf.pf_nsid.s_seq) ||
				(sp->s_rel == pf.pf_gsid.s_rel &&
				sp->s_lev == pf.pf_gsid.s_lev &&
				sp->s_br == pf.pf_gsid.s_br &&
				sp->s_seq == pf.pf_gsid.s_seq)) {
					if (++uniq) {
						fclose(in);
						fclose(out);
						fatal(MSGSTR(AMBIG, _AMBIG));
					}
					goodpf = pf;
					continue;
			}
		}
		if(fputs(line,out)==EOF)
			FAILPUT;
	}
	fflush(out);
	fstat(fileno(out),&Statbuf);
	Szqfile = Statbuf.st_size;
	copy(auxf(pkt->p_file,'p'),Pfilename);
	if (fclose(out) /* Disk Full? D63372 */
		&& (errno == ENOSPC || errno == EDQUOT) ) {
			perror("");
			clean_up(0);
			exit(1);
	}
	fclose(in);
	if (!goodpf.pf_user[0])
		fatal(MSGSTR(NAMORSID, "delta: The specified SID or your user name is not listed in the p-file.\n\
\tUse the correct user name or SID number. (de2)\n"));  /* MSG */
	return(&goodpf);
}


static FILE *
dodiff(newf,oldf)
char *newf, *oldf;
{
	register int i;
	int pfd[2];
	extern char Diffpgm[];

	xpipe(pfd);
	if ((i = fork()) < 0) {
		close(pfd[0]);
		close(pfd[1]);
		fatal(MSGCO(CANTFORK, "delta: Cannot create another process at this time.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));  /* MSG */
	}
	else if (i == 0) {
		close(pfd[0]);
		close(1);
		dup(pfd[1]);
		close(pfd[1]);
		for (i = 5; i < 15; i++)
			close(i);
		execl(Diffpgm,Diffpgm,oldf,newf,0);
		close(1);
		exit(32);	/* tell parent that 'execl' failed */
	}
	close(pfd[1]);
	return fdfopen(pfd[0],0);
}


static getdiff(type,plinenum)
register char *type;
register int *plinenum;
{
	char line[LINE_MAX+1]; 
	register char *p;
	char *rddiff(), *linerange();
	int num_lines = 0;    /* A13759 */
	static int chg_num, chg_ln;
	int lowline, highline;

	if ((p = rddiff(line,LINE_MAX+1)) == NULL)    /* A13759 */
		return(0);

	if (*p == '-') {
		*type = INS;
		*plinenum = chg_ln;
		num_lines = chg_num;
	}
	else {
		p = linerange(p,&lowline,&highline);
		*plinenum = lowline;

		switch(*p++) {
		case 'd':
			num_lines = highline - lowline + 1;
			*type = DEL;
			skipline(line,num_lines);
			break;

		case 'a':
			linerange(p,&lowline,&highline);
			num_lines = highline - lowline + 1;
			*type = INS;
			break;

		case 'c':
			chg_ln = lowline;
			num_lines = highline - lowline + 1;
			linerange(p,&lowline,&highline);
			chg_num = highline - lowline + 1;
			*type = DEL;
			skipline(line,num_lines);
			break;
		}
	}

	return(num_lines);
}


static insert(pkt,linenum,n,ser)
register struct packet *pkt;
register int linenum;
register int n;
int ser;
{
	char str[LINE_MAX+1];
        char *rddiff();

	after(pkt,linenum);
	sprintf(str,"%c%c %u\n",CTLCHAR,INS,ser);
	putline(pkt,str);
	for (++n; --n; ) {
		rddiff(str,LINE_MAX+1);
		putline(pkt,&str[2]);
	}
	sprintf(str,"%c%c %u\n",CTLCHAR,END,ser);
	putline(pkt,str);
}


static delete(pkt,linenum,n,ser)
register struct packet *pkt;
register int linenum;
int n;
register int ser;
{
	char str[LINE_MAX+1];

	before(pkt,linenum);
	sprintf(str,"%c%c %u\n",CTLCHAR,DEL,ser);
	putline(pkt,str);
	after(pkt,linenum + n - 1);
	sprintf(str,"%c%c %u\n",CTLCHAR,END,ser);
	putline(pkt,str);
}


static after(pkt,n)
register struct packet *pkt;
register int n;
{
	before(pkt,n);
	if (pkt->p_glnno == n)
		putline(pkt,(char *) 0);
}


static before(pkt,n)
register struct packet *pkt;
register int n;
{
	while (pkt->p_glnno < n) {
		if (!readmod(pkt))
			break;
	}
}


static char *
linerange(cp,low,high)
register char *cp;
register int *low, *high;
{
	cp = satoi(cp,low);
	if (*cp == ',')
		cp = satoi(++cp,high);
	else
		*high = *low;

	return(cp);
}


static skipline(lp,num)
register char *lp;
register int num;
{
       char *rddiff();
	for (++num;--num;)
		rddiff(lp,LINE_MAX+1);
}


static char *
rddiff(s,n)
register char *s;
register int n;
{
	register char *r;

	if ((r = fgets(s,n,Diffin)) != NULL && HADP)
		if(fputs(s,gpkt.p_stdout)==EOF)
			FAILPUT;
	return(r);
}


enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	char str[32];
	register struct apply *ap;

	sid_ba(sidp,str);
	ap = &pkt->p_apply[n];
	if (pkt->p_cutoff >= pkt->p_idel[n].i_datetime)
		switch(ap->a_code) {
	
		case SX_EMPTY:
			switch (ch) {
			case INCLUDE:
				condset(ap,APPLY,INCLUSER);
				break;
			case EXCLUDE:
				condset(ap,NOAPPLY,EXCLUSER);
				break;
			case IGNORE:
				condset(ap,SX_EMPTY,IGNRUSER);
				break;
			}
			break;
		case APPLY:
			fatal(MSGSTR(INTRNERR5, "delta: There is an internal software error.\n\
\tUse local problem reporting procedures. (de5)\n"));  /* MSG */
			break;
		case NOAPPLY:
			fatal(MSGSTR(INTRNERR6, "delta: There is an internal software error.\n\
\tUse local problem reporting procedures. (de6)\n"));  /* MSG */
			break;
		default:
			fatal(MSGSTR(INTRNERR7, "delta: There is an internal software error.\n\
\tUse local problem reporting procedures. (de7)\n"));  /* MSG */
			break;
		}
}


escdodelt()	/* dummy routine for dodelt() */
{
}

static clean_up()
{
	if (mylock(auxf(gpkt.p_file,'z'),getpid())) {
		if (gpkt.p_iop)
			fclose(gpkt.p_iop);
		if (Xiop) {
			fclose(Xiop);
			unlink(auxf(gpkt.p_file,'x'));
		}
		if(Gin)
			fclose(Gin);
		unlink(auxf(gpkt.p_file,'d'));
		unlink(auxf(gpkt.p_file,'q'));
		xrm(&gpkt);
		ffreeall();
		unlockit(auxf(gpkt.p_file,'z'),getpid());
	}
}


static fgetchk(file,pkt)
register char	*file;
register struct packet *pkt;
{
	FILE	*iptr;
	register int c, k, l;
	int lastc = '\n';

	iptr = xfopen(file,0);
	k = 1;
	l = 0;
	while ( (c=getc(iptr)) != EOF ) {
		/*
		 * Make sure line does not start with a Ctrl-A
		 */
		if ( l == 0 && c == CTLCHAR ) {
			fclose(iptr);
			sprintf(ErrMsg, 	/* MSG */
			  MSGSTR(ILLDAT,ILL_DATA),k, auxf(pkt->p_file,'g'));
			fatal(ErrMsg);
		}


		/*
		 * Make sure file does not have any embedded nulls
		 */
		
		if ( c == '\0' ) {
			fclose(iptr);
			sprintf(ErrMsg, 	/* MSG */
			  MSGSTR(ILLDAT,ILL_DATA),k, auxf(pkt->p_file,'g'));
			fatal(ErrMsg);
		}

		/*
		 * Check line length
		 */

		if (++l >= LINE_MAX) {
			fclose(iptr);
		}

		if( l == LINE_MAX ) {
			sprintf(ErrMsg,MSGCM(EXLINEMAX, "delta: max line length (%d bytes) exceeded\n"),LINE_MAX-2);  /* MSG */
			fatal(ErrMsg);
			}
		
		if ( c == '\n' ) {
			l = 0;
			++k;
		}
		lastc = c;
	}

	fclose(iptr);
	/*
	 * The last character in the file must be a newline
	 */
	if ( lastc != '\n')
		fatal(MSGCO(PRMTREOF,"delta: The end of the file was premature.\n\
\tMake sure that the last line of the file ends with a newline character or\n\
\tuse local problem reporting procedures. (co5)\n"));	/*MSG*/
}

