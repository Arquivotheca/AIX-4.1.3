static char sccsid[] = "@(#)99  1.19  src/bos/usr/bin/sccs/get.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:20";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: clean_up, cmrinsert, enter, escdodelt, fredck, get,
 *            gen_lfile, getser, idsetup, idsubst, in_pfile,
 *            makgdate, mk_qfile, newsid, prfx, trans, wrtpfile,
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
#include 	<string.h>
#include	<nl_types.h>
#include 	<sys/access.h>
#include 	<errno.h>
#include	"defines.h"
#include	"had.h"
#include 	<sys/limits.h>

#include	"get_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_GET, Num, Str)

static struct stat Statbuf;
static char Null[1];
static char ErrMsg[512];

static int	Debug = 0;
static int	had_pfile;
static struct packet gpkt;
static struct sid sid;
static unsigned	Ser;
static int	num_files;
static char	had[26];
static char	Whatstr[BUFSIZ];
static char	Pfilename[FILESIZE];
static char	*ilist, *elist, *lfile;
char	*sid_ab(), *auxf(), *logname();
char	*sid_ba(), *date_ba();
static long	cutoff = LONG_MAX;	/* max positive long */
static int verbosity;
static char	Gfile[FILESIZE];
static char	*Type;
static int	Did_id;
FILE *fdfopen();


nl_catd catd;

main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char c;
	char pflag='p';
	char rstring[LINE_MAX+1];
	char *tokptr, *strptr = rstring;

	extern int Fcnt;
	extern get();

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
			case 'a':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				Ser = patoi(p);
				break;
			case 'r':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				strcpy(rstring,p);
				while ((tokptr = strtok(strptr,".")) != NULL){
					if (atoi(tokptr) == 0)
						fatal(MSGCO(INVALIDSID, "get: The specified SID is not valid.\n\tUse the sact command to check the p-file for valid SID numbers.  (co8)\n"));
					strptr=NULL;
				}
				chksid(sid_ab(p,&sid),&sid);
				if ((sid.s_rel < MINR) ||
					(sid.s_rel > MAXR))
					fatal(MSGCM(ROUTOFRNG, "get: Supply a value for the -r flag that is greater than 0 and\n\
\tless than 10,000. (cm23)\n"));
				break;
			case 'w':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				if(*p)
					strcpy(Whatstr,p);
				break;
			case 'c':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				if (date_ab(p,&cutoff))
					fatal(MSGCM(BADDTTM, "\n  \
get: The date and time you specified are not in the correct format.\n\
\tThe correct format is: yy[mm[dd[hh[mm[ss]]]]] (cm5)\n"));
				break;
			case 'i':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				ilist = p;
				break;
			case 'x':
				if (!p[0]) {
					argv[i] = 0;
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				elist = p;
				break;
			case 'l':
				lfile = p;
				break;
			case 'L':
				lfile = &pflag ;
				had['l' - 'a']++;
				break;
			case 'b':
			case 'g':
			case 'e':
			case 'p':
			case 'k':
			case 'm':
			case 'n':
			case 's':
			case 't':
				if (had[c - 'a']++) {
					sprintf(ErrMsg,MSGCM(KEYLTRTWC, "get: Use the -%c flag only once on the command line.  (cm2)\n"),c);
					fatal(ErrMsg);
				}
				/*
				 * Since the -bgepkmnst options do not take arguments,
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
				printf(MSGSTR(GET_USAGE, "\
Usage: get [-gmnpst -ccutoff -ilist -rSID -xlist -wstring]... [-l -lp]\
 [file]...\n\
Usage: get -e -k [-bst -ccutoff -ilist -rSID -xlist]... [-l -lp] [file]...\n"));
				sprintf(ErrMsg,MSGCM(UNKKEYLTR, "get: Flag -%c is not valid.  Select a flag recognized\n\
\tby the command. (cm1)\n"),c);
				fatal(ErrMsg);
			}

			if (had[c - 'a']++) {
				sprintf(ErrMsg,MSGCM(KEYLTRTWC, "get: Use the -%c flag only once on the command line.  (cm2)\n"),c);
				fatal(ErrMsg);
			}
		}
		else num_files++;
	} /* end of for loop */
	if(num_files == 0)
		fatal(MSGCM(MISSFLNAM, "get: Specify the file to process.  (cm3)\n"));
	if (HADE && HADM)
		fatal(MSGSTR(ENOTWITHM, "get: Do not use the -e flag with the -m flag on the command line. (ge3)\n"));
	if (HADE)
		HADK = 1;
	if (!HADS)
		verbosity = -1;
	if (HADE && ! logname())
		fatal(MSGCM(USERID, "get: Internal error.  The /etc/passwd file is not accessible.\n\
\tFollow local problem reporting procedures. (cm9)\n"));
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=(argc-num_files); i<argc; i++)
			do_file(argv[i],get);
	exit(Fcnt ? 1 : 0);
}


extern char *Sflags[];

static get(file)
char *file;
{
	register char *p;
	register unsigned ser;
	extern char had_dir, had_standinp;
	struct stats stats;
	struct idel *dodelt();

	char	str[32];
	char *idsubst(), *auxf();

	Fflags |= FTLMSG;
	if (setjmp(Fjmp))
		return;
	if (HADE) {
		had_pfile = 1;
		/*
		call `sinit' to check if file is an SCCS file
		but don't open the SCCS file.
		If it is, then create lock file.
		*/
		sinit(&gpkt,file,0);
		if (lockit(auxf(file,'z'),10,getpid()))
			fatal(MSGCM(LOCKCREAT, "get: Cannot lock the specified file.\n\
\tCheck path name and permissions or\n\
\twait until the file is not in use.  (cm4)\n"));
	}
	/*
	Open SCCS file and initialize packet
	*/
	sinit(&gpkt,file,1);
	gpkt.p_ixuser = (HADI | HADX);
	gpkt.p_reqsid.s_rel = sid.s_rel;
	gpkt.p_reqsid.s_lev = sid.s_lev;
	gpkt.p_reqsid.s_br = sid.s_br;
	gpkt.p_reqsid.s_seq = sid.s_seq;
	gpkt.p_verbose = verbosity;
	gpkt.p_stdout = (HADP ? stderr : stdout);
	gpkt.p_cutoff = cutoff;
	gpkt.p_lfile = lfile;
	copy(auxf(gpkt.p_file,'g'),Gfile);

	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);
	finduser(&gpkt);
	doflags(&gpkt);
	if (!HADA)
		ser = getser(&gpkt);
	else {
		if ((ser = Ser) > maxser(&gpkt))
			fatal(MSGSTR(SERNOTOOLRG, "get: The specified serial number does not exist.\n\
\tCheck the SCCS file for existing serial numbers. (ge19)\n"));
		gpkt.p_gotsid = gpkt.p_idel[ser].i_sid;
		if (HADR && sid.s_rel != gpkt.p_gotsid.s_rel) {
			zero((char *)&gpkt.p_reqsid, sizeof(gpkt.p_reqsid));
			gpkt.p_reqsid.s_rel = sid.s_rel;
		}
		else
			gpkt.p_reqsid = gpkt.p_gotsid;
	}
	doie(&gpkt,ilist,elist,(char *) 0);
	setup(&gpkt,(int) ser);
	if (!(Type = Sflags[TYPEFLAG - 'a']))
		Type = Null;
	if (!(HADP || HADG))
		if ( accessx(Gfile, W_ACC, ACC_ANY) == 0 ) {
			sprintf(ErrMsg,MSGSTR(WRITFILEEXST, "get: A writable version of %s exists.\n\
\tRemove the writable version or remove the file's write permission. (ge4)\n"),Gfile);
			fatal(ErrMsg);
		}
	if (gpkt.p_verbose) {
		sid_ba(&gpkt.p_gotsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}
	if (HADE) {
		if (HADC || gpkt.p_reqsid.s_rel == 0)
			gpkt.p_reqsid = gpkt.p_gotsid;
		newsid(&gpkt,Sflags[BRCHFLAG - 'a'] && HADB);
		permiss(&gpkt);
		if (exists(auxf(gpkt.p_file,'p')))
			mk_qfile(&gpkt);
		else had_pfile = 0;
		wrtpfile(&gpkt,ilist,elist);
	}
	if (!HADG || HADL) {
		int i,status;

		if (gpkt.p_stdout)
			fflush(gpkt.p_stdout);

		if ((i = fork()) < 0)
			fatal(MSGCO(CANTFORK, "get: Cannot create another process at this time.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));
		if (i) {
			wait(&status);
			if (status) {
				Fflags &= ~FTLMSG;
				fatal(Null);
			}
			goto out;
		}

		Fflags |= FTLEXIT;
		Fflags &= ~FTLJMP;
		setuid(getuid());
		setgid(getgid());
		if (HADL)
			gen_lfile(&gpkt);
		if (HADG)
			exit(0);
		flushto(&gpkt,EUSERTXT,1);
		idsetup(&gpkt);
		gpkt.p_chkeof = 1;
		Did_id = 0;
		/*
		call `xcreate' which deletes the old `g-file' and
		creates a new one except if the `p' keyletter is set in
		which case any old `g-file' is not disturbed.
		The mod of the file depends on whether or not the `k'
		keyletter has been set.
		*/

		if (gpkt.p_gout == 0) {
			if (HADP)
				gpkt.p_gout = stdout;
			else
				gpkt.p_gout = xfcreat(Gfile,HADK ? 0644 : 0444);
		}
		while(readmod(&gpkt)) {
			prfx(&gpkt);
			p = idsubst(&gpkt,gpkt.p_line);
			if(fputs(p,gpkt.p_gout)==EOF)
				FAILPUT;
		}
		if (gpkt.p_gout)
			fflush(gpkt.p_gout);
		if (gpkt.p_gout && gpkt.p_gout != stdout)
			if (fclose(gpkt.p_gout)  /* Disk Full? D63372 */
				&& (errno == ENOSPC || errno == EDQUOT) ) {
					perror("");
					unlink(Gfile);
					exit(1);
			}
		if (gpkt.p_verbose)
			fprintf(gpkt.p_stdout,MSGSTR(ULINES,"%u lines\n"),gpkt.p_glnno);

		if (!Did_id && !HADK)
			if (Sflags[IDFLAG - 'a'])
				if(!(*Sflags[IDFLAG - 'a']))
					fatal(MSGCM(NOIDKEYWRDS, "get: The file must contain SCCS identification keywords.\n\
\tInsert one or more SCCS identification keywords into the file. (cm6)\n"));
				else
					fatal(MSGCM(INVIDKYWRDS, "get: The SCCS file requires one or more specific\n\
\tidentification keywords.\n\
\tAdd the keywords to the file. (cm10)\n"));
			else if (gpkt.p_verbose)
				fprintf(stderr,MSGCM(NOIDKEYWRDS7, \
"get: There are no SCCS identification keywords in the file. (cm7)\n"));
		exit(0);
	}
out:    if (gpkt.p_iop)
			if (fclose(gpkt.p_iop)  /* Disk Full? D63372 */
				&& (errno == ENOSPC || errno == EDQUOT) ){
					perror("");
					clean_up(0);
					exit(1);
			}

	/*
	remove 'q'-file because it is no longer needed
	*/
	unlink(auxf(gpkt.p_file,'q'));
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());
}


static newsid(pkt,branch)
register struct packet *pkt;
int branch;
{
	int chkbr;

	chkbr = 0;
	/* if branch value is 0 set newsid level to 1 */
	if (pkt->p_reqsid.s_br == 0) {
		pkt->p_reqsid.s_lev += 1;
		/*
		if the sid requested has been deltaed or the branch
		flag was set or the requested SID exists in the p-file
		then create a branch delta off of the gotten SID
		*/
		if (sidtoser(&pkt->p_reqsid,pkt) ||
			pkt->p_maxr > pkt->p_reqsid.s_rel || branch ||
			in_pfile(&pkt->p_reqsid,pkt)) {
				pkt->p_reqsid.s_rel = pkt->p_gotsid.s_rel;
				pkt->p_reqsid.s_lev = pkt->p_gotsid.s_lev;
				pkt->p_reqsid.s_br = pkt->p_gotsid.s_br + 1;
				pkt->p_reqsid.s_seq = 1;
				chkbr++;
		}
	}
	/*
	if a three component SID was given as the -r argument value
	and the b flag is not set then up the gotten SID sequence
	number by 1
	*/
	else if (pkt->p_reqsid.s_seq == 0 && !branch)
		pkt->p_reqsid.s_seq = pkt->p_gotsid.s_seq + 1;
	else {
		/*
		if sequence number is non-zero then increment the
		requested SID sequence number by 1
		*/
		pkt->p_reqsid.s_seq += 1;
		if (branch || sidtoser(&pkt->p_reqsid,pkt) ||
			in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			pkt->p_reqsid.s_seq = 1;
			chkbr++;
		}
	}
	/*
	keep checking the requested SID until a good SID to be
	made is calculated or all possibilities have been tried
	*/
	while (chkbr) {
		--chkbr;
		while (in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
		while (sidtoser(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
	}
	if (sidtoser(&pkt->p_reqsid,pkt) || in_pfile(&pkt->p_reqsid,pkt))
		fatal(MSGSTR(BADSIDCALC, "get: bad SID calculated in newsid()"));
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
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"%s\n",str);
	ap = &pkt->p_apply[n];
	switch(ap->a_code) {

	case SX_EMPTY:
		if (ch == INCLUDE)
			condset(ap,APPLY,INCLUSER);
		else
			condset(ap,NOAPPLY,EXCLUSER);
		break;
	case APPLY:
		sid_ba(sidp,str);
		sprintf(ErrMsg,MSGSTR(ALRDYINC, "get: %s is already included.\n\
\tSpecify the delta only once with the -i flag. (ge9)\n"),str);
		fatal(ErrMsg);
		break;
	case NOAPPLY:
		sid_ba(sidp,str);
		sprintf(ErrMsg,MSGSTR(ALRDYEXCLD, "get: %s is already excluded.\n\
\tSpecify the delta only once with the -x flag. (ge10)\n"),str);
		fatal(ErrMsg);
		break;
	default:
		fatal(MSGSTR(INTRNERR11, "get: There is an internal software error.\n\
\tUse local problem reporting procedures. (ge11)\n"));
		break;
	}
}


static gen_lfile(pkt)
register struct packet *pkt;
{
	char *n;
	int reason;
	char str[32];
	char line[LINE_MAX+1];
	struct deltab dt;
	FILE *in;
	FILE *out;

	in = xfopen(pkt->p_file,0);
	if (*pkt->p_lfile)
		out = stdout;
	else
		out = xfcreat(auxf(pkt->p_file,'l'),0444);
	fgets(line,LINE_MAX+1,in);
	while (fgets(line,LINE_MAX+1,in) != NULL && line[0] == CTLCHAR && line[1] == STATS) {
		fgets(line,LINE_MAX+1,in);
		del_ab(line,&dt,pkt);
		if (dt.d_type == 'D') {
			reason = pkt->p_apply[dt.d_serial].a_reason;
			if (pkt->p_apply[dt.d_serial].a_code == APPLY) {
				putc(' ',out);
				putc(' ',out);
			}
			else {
				putc('*',out);
				if (reason & IGNR)
					putc(' ',out);
				else
					putc('*',out);
			}
			switch (reason & (INCL | EXCL | CUTOFF)) {
	
			case INCL:
				putc('I',out);
				break;
			case EXCL:
				putc('X',out);
				break;
			case CUTOFF:
				putc('C',out);
				break;
			default:
				putc(' ',out);
				break;
			}
			putc(' ',out);
			sid_ba(&dt.d_sid,str);
			fprintf(out,"%s\t",str);
			date_ba(&dt.d_datetime,str);
			fprintf(out,"%s %s\n",str,dt.d_pgmr);
		}
		while ((n = fgets(line,LINE_MAX+1,in)) != NULL)
			if (line[0] != CTLCHAR)
				break;
			else {
				switch (line[1]) {

				case EDELTAB:
					break;
				default:
					continue;
				case MRNUM:
				case COMMENTS:
					if (dt.d_type == 'D')
						fprintf(out,"\t%s",&line[3]);
					continue;
				}
				break;
			}
		if (n == NULL || line[0] != CTLCHAR)
			break;
		putc('\n',out);
	}
	fclose(in);
	if (out != stdout)
		if (fclose(out) /* Disk Full? D63372 */
			&& (errno == ENOSPC || errno == EDQUOT) ){
				perror("");
				unlink(auxf(pkt->p_file,'l'));
				exit(1);  /* no cleanup; that is 
								parent's responsibility */
		}

}


static char	Curdate[18];
static char	*Curtime;
static char	Gdate[9];
static char	Chgdate[18];
static char	*Chgtime;
static char	Gchgdate[9];
static char	Sid[32];
static char	Mod[FILESIZE];
static char	Olddir[PATH_MAX];
static char	Pname[PATH_MAX];
static char	Dir[PATH_MAX];
static char	*Qsect;

static idsetup(pkt)
register struct packet *pkt;
{
	extern long Timenow;
	register int n;
	register char *p;

	date_ba(&Timenow,Curdate);
	Curtime = &Curdate[9];
	Curdate[8] = 0;
	makgdate(Curdate,Gdate);
	for (n = maxser(pkt); n; n--)
		if (pkt->p_apply[n].a_code == APPLY)
			break;
	if (n)
		date_ba(&pkt->p_idel[n].i_datetime,Chgdate);
	Chgtime = &Chgdate[9];
	Chgdate[8] = 0;
	makgdate(Chgdate,Gchgdate);
	sid_ba(&pkt->p_gotsid,Sid);
	if (p = Sflags[MODFLAG - 'a'])
		copy(p,Mod);
	else
		copy(Gfile,Mod);
	if (!(Qsect = Sflags[QSECTFLAG - 'a']))
		Qsect = Null;
}


static makgdate(old,new)
register char *old, *new;
{
	if ((*new = old[3]) != '0')
		new++;
	*new++ = old[4];
	*new++ = '/';
	if ((*new = old[6]) != '0')
		new++;
	*new++ = old[7];
	*new++ = '/';
	*new++ = old[0];
	*new++ = old[1];
	*new = 0;
}


static char Zkeywd[5] = "@(#)";

static char *
idsubst(pkt,line)
register struct packet *pkt;
char line[];
{
	static char tline[LINE_MAX+1];
	char hold[LINE_MAX+1];
	static char str[32];
	register char *lp, *tp;

	char *trans();
	int recursive = 0;
	extern char *Type;
	extern char *Sflags[];

	if (HADK || !any('%',line))
		return(line);

	tp = tline;
	for(lp=line; *lp != 0; lp++) {
		if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%') {
			if((!Did_id) && (Sflags['i'-'a']) &&
				(!(strncmp(Sflags['i'-'a'],lp,strlen(Sflags['i'-'a'])))))
					++Did_id;
			switch(*++lp) {

			case 'M':
				tp = trans(tp,Mod);
				break;
			case 'Q':
				tp = trans(tp,Qsect);
				break;
			case 'R':
				sprintf(str,"%u",pkt->p_gotsid.s_rel);
				tp = trans(tp,str);
				break;
			case 'L':
				sprintf(str,"%u",pkt->p_gotsid.s_lev);
				tp = trans(tp,str);
				break;
			case 'B':
				sprintf(str,"%u",pkt->p_gotsid.s_br);
				tp = trans(tp,str);
				break;
			case 'S':
				sprintf(str,"%u",pkt->p_gotsid.s_seq);
				tp = trans(tp,str);
				break;
			case 'D':
				tp = trans(tp,Curdate);
				break;
			case 'H':
				tp = trans(tp,Gdate);
				break;
			case 'T':
				tp = trans(tp,Curtime);
				break;
			case 'E':
				tp = trans(tp,Chgdate);
				break;
			case 'G':
				tp = trans(tp,Gchgdate);
				break;
			case 'U':
				tp = trans(tp,Chgtime);
				break;
			case 'Z':
				tp = trans(tp,Zkeywd);
				break;
			case 'Y':
				tp = trans(tp,Type);
				break;
			case 'W':
				if((Whatstr[0] != '\0') && (!recursive)) {
					recursive = 1;
					lp += 2;
					strcpy(hold,Whatstr);
					strcat(hold,lp);
					lp = hold;
					lp--;
					continue;
				}
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Mod);
				*tp++ = '\t';
			case 'I':
				tp = trans(tp,Sid);
				break;
			case 'P':
				copy(pkt->p_file,Dir);
				dname(Dir);
				if(curdir(Olddir) != 0)
					fatal(MSGCM(CURDIRFAIL, "get: Cannot determine the path name of the current directory.\n\
\tCheck permissions on current directory.\n\
\tIf the problem persists, follow local problem reporting procedures. (cm21)\n"));
				if(chdir(Dir) != 0) {
					sprintf(ErrMsg,
					MSGCM(CANTCHDIR, "get: Cannot chdir to %s.\n\
\tCheck path name and permissions\n\
\tor use local problem reporting procedures.  (cm22)\n"),Dir);
					fatal(ErrMsg);
				}
				if(curdir(Pname) != 0)
					fatal(MSGCM(CURDIRFAIL, "get: Cannot determine the path name of the current directory.\n\
\tCheck permissions on current directory.\n\
\tIf the problem persists, follow local problem reporting procedures. (cm21)\n"));
				if(chdir(Olddir) != 0) {
					sprintf(ErrMsg,
					MSGCM(CANTCHDIR, "get: Cannot chdir to %s.\n\
\tCheck path name and permissions\n\
\tor use local problem reporting procedures.  (cm22)\n"),Olddir);
					fatal(ErrMsg);
				}
				tp = trans(tp,Pname);
				*tp++ = '/';
				tp = trans(tp,(sname(pkt->p_file)));
				break;
			case 'F':
				tp = trans(tp,pkt->p_file);
				break;
			case 'C':
				sprintf(str,"%u",pkt->p_glnno);
				tp = trans(tp,str);
				break;
			case 'A':
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Type);
				*tp++ = ' ';
				tp = trans(tp,Mod);
				*tp++ = ' ';
				tp = trans(tp,Sid);
				tp = trans(tp,Zkeywd);
				break;
			default:
				*tp++ = '%';
				*tp++ = *lp;
				continue;
			}
			if (!(Sflags['i'-'a']))
				++Did_id;
			lp++;
		}
		else
			*tp++ = *lp;
	}

	*tp = 0;
	return(tline);
}


static char *
trans(tp,str)
register char *tp, *str;
{
	while(*tp++ = *str++)
		;
	return(tp-1);
}


static prfx(pkt)
register struct packet *pkt;
{
	char str[32];

	if (HADN)
		fprintf(pkt->p_gout,"%s\t",Mod);
	if (HADM) {
		sid_ba(&pkt->p_inssid,str);
		fprintf(pkt->p_gout,"%s\t",str);
	}
}


static clean_up()
{
	/*
	clean_up is only called from fatal() upon bad termination.
	*/
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);
	if (gpkt.p_gout)
		fflush(gpkt.p_gout);
	if (gpkt.p_gout && gpkt.p_gout != stdout) {
		fclose(gpkt.p_gout);
		unlink(Gfile);
	}
	if (HADE) {
		if (! had_pfile) {
			unlink(auxf(gpkt.p_file,'p'));
		}
		else if (exists(auxf(gpkt.p_file,'q')))
			unlink(auxf(gpkt.p_file,'q'));
	}
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());
}


/* WARN_MSG is the default message for the catalog WARNMSG message */

#define WARN_MSG "WARNING:  %s is being edited.\n\
\tThis is an informational message only. (ge18)\n"

static wrtpfile(pkt,inc,exc)
register struct packet *pkt;
char *inc, *exc;
{
	char line[LINE_MAX+1], str1[32], str2[32];
	char *user;
	FILE *in, *out;
	struct pfile pf;
	register char *p;

	int fd;
	extern long Timenow;

	user = logname();
	if (exists(p = auxf(pkt->p_file,'p'))) {
		fd = xopen(p,0);
		in = fdfopen(fd,0);
		while (fgets(line,LINE_MAX+1,in) != NULL) {
			p = line;
			p[length(p) - 1] = 0;
			pf_ab(p,&pf,0);
			if (!(Sflags[JOINTFLAG - 'a'])) {
				if ((pf.pf_gsid.s_rel == pkt->p_gotsid.s_rel &&
     				   pf.pf_gsid.s_lev == pkt->p_gotsid.s_lev &&
				   pf.pf_gsid.s_br == pkt->p_gotsid.s_br &&
				   pf.pf_gsid.s_seq == pkt->p_gotsid.s_seq) ||
				   (pf.pf_nsid.s_rel == pkt->p_reqsid.s_rel &&
				   pf.pf_nsid.s_lev == pkt->p_reqsid.s_lev &&
				   pf.pf_nsid.s_br == pkt->p_reqsid.s_br &&
				   pf.pf_nsid.s_seq == pkt->p_reqsid.s_seq)) {
					fclose(in);
					sprintf(ErrMsg,
					     MSGSTR(BNGEDT, "get: Another user is editing %s.\n\
\tWait until the user completes the delta.  (ge17)"),line);
					fatal(ErrMsg);
				}
				if (!equal(pf.pf_user,user))
					fprintf(stderr,MSGSTR(WARNMSG, WARN_MSG),line);
			}
			else fprintf(stderr,MSGSTR(WARNMSG, WARN_MSG),line);
		}
		fd = xopen(auxf(pkt->p_file,'q'),1);
		out = fdfopen(dup(fd),1);
		fclose(in);
	}
	else
		out = xfcreat(p,0644);
	fseek(out,0L,2);
	sid_ba(&pkt->p_gotsid,str1);
	sid_ba(&pkt->p_reqsid,str2);
	date_ba(&Timenow,line);
	fprintf(out,"%s %s %s %s",str1,str2,user,line);
	if (inc)
		fprintf(out," -i%s",inc);
	if (exc)
		fprintf(out," -x%s",exc);
	fprintf(out,"\n");
	if (fclose(out) /* Disk full? D63372 */
		&& (errno == ENOSPC || errno == EDQUOT) ){
			perror("");
			clean_up(0);
			exit(1);
	}
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,MSGSTR(NEWDELTA, "new delta %s\n"),str2);
	if (exists(auxf(pkt->p_file,'q'))) {
		copy(auxf(pkt->p_file,'p'),Pfilename);
		rename(auxf(pkt->p_file,'q'),Pfilename);
	}
}


static getser(pkt)
register struct packet *pkt;
{
	register struct idel *rdp;
	int n, ser, def;

	char *p;
	extern char *Sflags[];

	def = 0;
	if (pkt->p_reqsid.s_rel == 0) {
		if (p = Sflags[DEFTFLAG - 'a'])
			chksid(sid_ab(p, &pkt->p_reqsid), &pkt->p_reqsid);
		else {
			pkt->p_reqsid.s_rel = MAXR;
			def = 1;
		}
	}
	ser = 0;
	if (pkt->p_reqsid.s_lev == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if ((rdp->i_sid.s_br == 0 || HADT) &&
				pkt->p_reqsid.s_rel >= rdp->i_sid.s_rel &&
				rdp->i_sid.s_rel > pkt->p_gotsid.s_rel) {
					ser = n;
					pkt->p_gotsid.s_rel = rdp->i_sid.s_rel;
			}
		}
	}
	/*
	 * If had '-t' keyletter and R.L SID type, find
	 * the youngest SID
	*/
	else if ((pkt->p_reqsid.s_br == 0) && HADT) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
			    rdp->i_sid.s_lev == pkt->p_reqsid.s_lev )
				break;
		}
		ser = n;
	}
	else if (pkt->p_reqsid.s_br && pkt->p_reqsid.s_seq == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
				rdp->i_sid.s_lev == pkt->p_reqsid.s_lev &&
				rdp->i_sid.s_br == pkt->p_reqsid.s_br)
					break;
		}
		ser = n;
	}
	else {
		ser = sidtoser(&pkt->p_reqsid,pkt);
	}
	if (ser == 0)
		fatal(MSGCM(SIDNOEXIST, "get: The SID you specified does not exist.\n\
\tUse the sact command to check the p-file for existing SID numbers. (cm20)\n"));
	rdp = &pkt->p_idel[ser];
	pkt->p_gotsid = rdp->i_sid;
	if (def || (pkt->p_reqsid.s_lev == 0 && pkt->p_reqsid.s_rel == pkt->p_gotsid.s_rel))
		pkt->p_reqsid = pkt->p_gotsid;
	return(ser);
}


/* Null routine to satisfy external reference from dodelt() */

escdodelt()
{
}

static in_pfile(sp,pkt)
struct	sid	*sp;
struct	packet	*pkt;
{
	struct	pfile	pf;
	char	line[LINE_MAX+1];
	char	*p;
	FILE	*in;

	if (Sflags[JOINTFLAG - 'a']) {
		if (exists(auxf(pkt->p_file,'p'))) {
			in = xfopen(auxf(pkt->p_file,'p'),0);
			while ((p = fgets(line,LINE_MAX+1,in)) != NULL) {
				p[length(p) - 1] = 0;
				pf_ab(p,&pf,0);
				if (pf.pf_nsid.s_rel == sp->s_rel &&
					pf.pf_nsid.s_lev == sp->s_lev &&
					pf.pf_nsid.s_br == sp->s_br &&
					pf.pf_nsid.s_seq == sp->s_seq) {
						fclose(in);
						return(1);
				}
			}
			fclose(in);
		}
	}
	return(0);
}


static mk_qfile(pkt)
register struct	packet *pkt;
{
	FILE	*in, *qout;
	char	line[LINE_MAX+1];

	in = xfopen(auxf(pkt->p_file,'p'),0);
	qout = xfcreat(auxf(pkt->p_file,'q'),0644);

	while ((fgets(line,LINE_MAX+1,in) != NULL))
		if(fputs(line,qout)==EOF)
			FAILPUT;
	fclose(in);
	if (fclose(qout) /* Disk full? D63372 */
		&& (errno == ENOSPC || errno == EDQUOT) ){
			perror("");
			clean_up(0);
			exit(1);
	}
}
