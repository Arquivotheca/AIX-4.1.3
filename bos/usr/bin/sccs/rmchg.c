static char sccsid[] = "@(#)00  1.15  src/bos/usr/bin/sccs/rmchg.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:58";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: ckmrs, clean_up, esccmfdelt, escdodelt, fredck, 
 *            getvalflag, msg, put_delmrs, putmrs, rdpfile, rmchg,
 *            split_mrs, testfred, verif, main
 *            
 * ORIGINS: 3, 10, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */



# include <locale.h>
# include <nl_types.h>
# include "defines.h"
# include "had.h"
# include "rmchg_msg.h"
# include <stdlib.h>

#define MSGSTR(Num, Str) catgets(catd, MS_RMCHG, Num, Str)
char *strncpy();


/*
	Program to remove a specified delta from an SCCS file,
	when invoked as 'rmdel',
	or to change the MRs and/or comments of a specified delta,
	when invoked as 'cdc'.
	(The program has two links to it, one called 'rmdel', the
	other 'cdc'.)

	The delta to be removed (or whose MRs and/or comments
	are to be changed) is specified via the
	r argument, in the form of an SID.

	If the delta is to be removed, it must be the most recent one
	in its branch in the delta tree (a so-called 'leaf' delta).
	For either function, the delta being processed must not
	have any 'delivered' MRs, and the user must have basically
	the same permissions as are required to make deltas.

	If a directory is given as an argument, each SCCS file
	within the directory will be processed as if it had been
	specifically named. If a name of '-' is given, the standard
	input will be read for a list of names of SCCS files to be
	processed. Non SCCS files are ignored.
*/

# define COPY 0
# define NOCOPY 1

static struct stat Statbuf;
static char Null[1];
static char ErrMsg[512];

static struct sid sid;
static int num_files;
static char had[26];
static char D_type;
char	*auxf();
char    *logname();
char	*Mrs;
char	*Comments;
static char	*Darg[NVARGS];
static char    *Earg[NVARGS], **eargv;
static char	*NVarg[NVARGS];
extern char *Sflags[];
static int D_serial;

nl_catd catd;

main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	register char *p;
	char c;
	char *sid_ab();
	wchar_t wc_flag;
	extern rmchg();
	extern int Fcnt;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	/*
	Set flags for 'fatal' to issue message, call clean-up
	routine, and terminate processing.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && (c = argv[i][1])) {
			if (!strcmp(argv[i], "--")) {  /* got -- argument */
				num_files+=argc-(i+1);
				p = argv[i+1];
				break;
			}
			p = &argv[i][2];
			switch (c) {

			case 'r':
				if (!p[0]) {
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				if (!(*p))
					fatal(MSGSTR(RNOSID, "Specify an SID with the -r flag. (rc11)\n"));  /* MSG */
				chksid(sid_ab(p,&sid),&sid);
				break;
			case 'm':	/* MR entry */
				if (!p[0]) {
					i++;
					p = &argv[i][0];
					if (!p[0]) continue;
				}
				Mrs = p;
				repl(Mrs,'\n',' ');
				break;
			case 'y':	/* comment line */
				savecmt(p);
				break;
			default:
				sprintf(ErrMsg,MSGCM(UNKKEYLTR, 
                                  "Flag -%c is not valid. Select a flag recognized\n\tby the command. (cm1)\n"),c);  /*MSG*/
				fatal(ErrMsg);
			}

			if (had[c - 'a']++) {
				sprintf(ErrMsg,MSGCM(KEYLTRTWC,  
                                  "Use the -%c flag only once on the command line. (cm2)\n"),c);/*MSG*/
				fatal(ErrMsg);
			}
		}
		else
			num_files++;
	} /* end of for loop */
	if(num_files == 0)
		fatal(MSGCM(MISSFLNAM, 
                   "Specify the file to process. (cm3)\n"));  /* MSG */
	if (!HADR)
		fatal(MSGSTR(MISSINGR, 
                   "Use the -r flag with this command. (rc1)\n"));  /* MSG */


	if (*(p = sname(argv[0])) == 'n')
		p++;
	if (equal(p,"rmdel"))
		D_type = 'R';		/* invoked as 'rmdel' */
	else if (equal(p,"cdc"))
		D_type = 'D';		/* invoked as 'cdc' */
	else
		fatal(MSGSTR(BADINVOKE,"You cannot invoke rmdel or cdc except by the commands rmdel or cdc. (rc10)\n"));  /* MSG */
	if (! logname())
		fatal(MSGCM(USERID, 
                  "The /etc/passwd file is not accessible.\n\
\tFollow local problem reporting procedures. (cm9)\n")); /*MSG*/

	setsig();

	/*
	Change flags for 'fatal' so that it will return to this
	routine (main) instead of terminating processing.
	*/
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	/*
	Call 'rmchg' routine for each file argument.
	*/
	for (i=(argc-num_files); i<argc; i++)
			do_file(argv[i],rmchg);
	exit(Fcnt ? 1 : 0);
}


/*
	Routine that actually causes processing of the delta.
	Processing on the file takes place on a
	temporary copy of the SCCS file (the x-file).
	The name of the x-file is the same as that of the
	s-file (SCCS file) with the 's.' replaced by 'x.'.
	At end of processing, the s-file is removed
	and the x-file is renamed with the name of the old s-file.

	This routine makes use of the z-file to lock out simultaneous
	updates to the SCCS file by more than one user.
*/

static struct packet gpkt;	/* see file s.h */
static char line[LINE_MAX+1];

static rmchg(file)
char *file;
{
	struct stats stats;	/* see file s.defines.h */
	struct stat sbuf;
	struct idel *dodelt();
	extern char *Sflags[];
	int n;
	char *p, *cp, *getline();
	int keep;
	extern char Pgmr[LOGSIZE];
	int fowner, downer, user;

	extern char	*acl_get();
	char		*sptr;

	if (setjmp(Fjmp))	/* set up to return here from 'fatal' */
		return;		/* and return to caller of rmchg */

	sinit(&gpkt,file,1);	/* initialize packet and open s-file */
	/*
	Lock out any other user who may be trying to process
	the same file.
	*/
	if (lockit(auxf(file,'z'),2,getpid()))
		fatal(MSGCM(LOCKCREAT, 
                   "Cannot lock the specified file.\n\
\tCheck path name and permissions or\n\
\twait until the file is not in use. (cm4)\n"));  /* MSG */

	/*
	Flag for 'putline' routine to tell it to open x-file
	and allow writing on it.
	*/
	gpkt.p_upd = 1;

	/*
	Save requested SID for later checking of
	permissions (by 'permiss').
	*/
	gpkt.p_reqsid = sid;

	/*
	Now read-in delta table. The 'dodelt' routine
	will read the table and change the delta entry of the
	requested SID to be of type 'R' if this is
	being executed as 'rmdel'; otherwise, for 'cdc', only
	the MR and comments sections will be changed 
	(by 'escdodelt', called by 'dodelt').
	*/
	if (dodelt(&gpkt,&stats,&sid,D_type) == 0)
		fmterr(&gpkt);

	/*
	Get serial number of requested SID from
	delta table just processed.
	*/
	D_serial = sidtoser(&gpkt.p_reqsid,&gpkt);

	/*
	If SID has not been zeroed (by 'dodelt'),
	SID was not found in file.
	*/
	if (sid.s_rel != 0)
		fatal(MSGCM(SIDNOEXIST, 
                  "The SID you specified does not exist.\n\
\tUse the sact command to check the p-file for existing SID numbers. (cm20)\n"));  /* MSG */
	/*
	Replace 'sid' with original 'sid'
	requested.
	*/
	sid = gpkt.p_reqsid;

	/*
	Now check permissions.
	*/
	finduser(&gpkt);
	doflags(&gpkt);
	permiss(&gpkt);

	/*
	Check that user is either owner of file or
	directory, or is one who made the delta.
	*/
	fstat(fileno(gpkt.p_iop),&Statbuf);
	fowner = Statbuf.st_uid;
	copy(gpkt.p_file,line);		/* temporary for dname() */
	if (stat(dname(line),&Statbuf))
		downer = -1;
	else
		downer = Statbuf.st_uid;
	user = getuid();
	if (user != fowner && user != downer)
		if (!equal(Pgmr,logname())) {
		   sprintf(ErrMsg,MSGSTR(NOTOWNER, 
                     "You must be the owner of file or %s\n\
\tto use this command. (rc4)\n"),Pgmr);/*MSG*/
		   fatal(ErrMsg);
		}

	/*
	For 'rmdel', check that delta being removed is a
	'leaf' delta, and if ok,
	process the body.
	*/
	if (D_type == 'R') {
		struct idel *ptr;
		for (n = maxser(&gpkt); n > D_serial; n--) {
			ptr = &gpkt.p_idel[n];
			if (ptr->i_pred == D_serial)
			    fatal(MSGSTR(NOTLEAF, "You can remove only the most recently created delta on a branch, \n\
\tor the latest trunk delta if it has no branches. (rc5)\n"));  /* MSG */
		}

		/*
		   For 'rmdel' check that the sid requested is
		   not contained in p-file, should a p-file
		   exist.
		*/

		if (exists(auxf(gpkt.p_file,'p')))
			rdpfile(&gpkt,&sid);

		flushto(&gpkt,EUSERTXT,COPY);

		keep = YES;
		gpkt.p_chkeof = 1;		/* set EOF is ok */
		while ((p = getline(&gpkt)) != NULL) {
			if (*p++ == CTLCHAR) {
				cp = p++;
				NONBLANK(p);
				/*
				Convert serial number to binary.
				*/
				if (*(p = satoi(p,&n)) != '\n')
					fmterr(&gpkt);
				if (n == D_serial) {
					gpkt.p_wrttn = 1;
					if (*cp == INS)
						keep = NO;
					else
						keep = YES;
				}
			}
			else
				if (keep == NO)
					gpkt.p_wrttn = 1;
		}
	}
	else {
		/*
		This is for invocation as 'cdc'.
		Check MRs.
		*/
		if (Mrs && *Mrs) {
			if (!(p = Sflags[VALFLAG - 'a']))
				fatal(MSGCM(MRNOTALD, "The SCCS file you specified does not allow MR numbers. (cm24)\n"));  /* MSG */
			if (*p && valmrs(&gpkt,p))
				fatal(MSGCM(INVMRS, "Use a valid MR number or numbers. (cm25)\n"));  /* MSG */
		}

		/*
		Indicate that EOF at this point is ok, and
		flush rest of s-file to x-file.
		*/
		gpkt.p_chkeof = 1;
		while (getline(&gpkt))
			;
	}

	flushline(&gpkt,(struct stats *) 0);
	
	/*
	Delete old s-file, change x-file name to s-file.
	*/
	stat(gpkt.p_file,&sbuf);
	sptr = acl_get(gpkt.p_file);
	rename(auxf(gpkt.p_file,'x'),gpkt.p_file);
	acl_put(gpkt.p_file, sptr, 1);
	chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
	clean_up();
}


escdodelt(pkt)
struct packet *pkt;
{
	extern int First_esc;
	extern int First_cmt;
	extern int CDid_mrs;
	char	*p;
	char	*date_ba();
	extern char *Sflags[];
	extern long Timenow;

	if (pkt->p_line[1] == MRNUM) {		/* line read is MR line */
		p = pkt->p_line;
		while (*p)
			p++;
		if (*(p - 2) == DELIVER)
		        fatal(MSGSTR(DELTSPEC, "You cannot remove this delta or change its commentary.\n\
\tThe delta contains an MR number which has been marked \"delivered\"\n\
\t(unchangeable).  (rc9)\n"));  /* MSG */
	}
	if (D_type == 'D' && First_esc) {	/* cdc, first time */
		First_esc = 0;
		getvalflag();
		dohist();
		if (Mrs && *Mrs) {
			/*
			 * if adding MRs then put out the new list
			 * from `split_mrs' (if any)
			*/

			split_mrs();
			putmrs(pkt);
			CDid_mrs = 1;	/* indicate that some MRs were read */
			eargv = Earg;
		}
	}

	if (pkt->p_line[1] == MRNUM) {		/* line read is MR line */
		if (!CDid_mrs)	/* if no MRs list from `dohist' then return */
			return;
		else
			/*
			 * check to see if this MR should be removed
			*/

			ckmrs(pkt,pkt->p_line);
	}
	else if (D_type == 'D') {               /* this line is a comment
						 * or the end of the entry
						 */
		if (First_cmt) {		/* first comment encountered */
			First_cmt = 0;
			/*
			 * if comments were read by `dohist' then
			 * put them out.
			*/

			if (*Comments)
				putline(pkt,Comments);

			/*
			 * if any MRs were deleted, print them out
			 * as comments for this invocation of `cdc'
			*/

			put_delmrs(pkt);
			/*
			 * if comments were read by `dohist' and
			 * there were previous comments then
			 * indicate that comments were CHANGED.
			*/

			if (*Comments && pkt->p_line[1] == COMMENTS) {
				sprintf(line,"%c%c ",CTLCHAR,COMMENTS);
				putline(pkt,line);
				putline(pkt,MSGSTR(CHANGED,"*** CHANGED *** "));
				/* get date and time */
				date_ba(&Timenow,line);	
				putline(pkt,line);
				sprintf(line," %s\n",logname());
				putline(pkt,line);
			}
		}
		else return;
	}
}


/* find VALFLAG for dohist */
static getvalflag()
{
	char nline[LINE_MAX+1];
	register char *p;
	FILE *in, *fdfopen();

	Sflags[VALFLAG - 'a'] = NULL;
	in = xfopen(gpkt.p_file,0);
	while ((p = fgets(nline,LINE_MAX+1,in)) != NULL)
		if (nline[0] == CTLCHAR && nline[1] == EUSERNAM)
			break;
	if (p) {
		while (fgets(nline,LINE_MAX+1,in) && nline[1] == FLAG &&
		    nline[0] == CTLCHAR)
			if (nline[3] == VALFLAG) {
				Sflags[VALFLAG - 'a'] = Null;
				break;
			}
	}
	fclose(in);
}


extern char **Varg;

static split_mrs()
{
	register char **argv;
	char	**dargv;
	char	**nargv;
	char	*ap, *stalloc();

	if (Varg == NVarg) return;

	dargv = Darg;
	nargv = &NVarg[VSTART];
	for (argv = &Varg[VSTART]; *argv; argv++)
		if (*argv[0] == '!') {
			*argv += 1;
			ap = *argv;
			copy(ap,*dargv++ = stalloc(size(ap)));
			*dargv = 0;
			continue;
		}
		else {
			copy(*argv,*nargv++ = stalloc(size(*argv)));
			*nargv = 0;
		}
	Varg = NVarg;
}


static putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char	str[64];

	for(argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


static clean_up()
{
	if(gpkt.p_iop)
		fclose(gpkt.p_iop);
	xrm(&gpkt);
	if (exists(auxf(gpkt.p_file,'x')))
		xunlink(auxf(gpkt.p_file,'x'));
	ffreeall();
	if (gpkt.p_file[0])
		unlockit(auxf(gpkt.p_file,'z'),getpid());
}


static rdpfile(pkt,sp)
register struct packet *pkt;
struct sid *sp;
{
	struct pfile pf;
	char rline[LINE_MAX+1];
	FILE *in, *fdfopen();

	in = xfopen(auxf(pkt->p_file,'p'),0);
	while (fgets(rline,LINE_MAX+1,in) != NULL) {
		pf_ab(rline,&pf,1);
		if (sp->s_rel == pf.pf_gsid.s_rel &&
			sp->s_lev == pf.pf_gsid.s_lev &&
			sp->s_br == pf.pf_gsid.s_br &&
			sp->s_seq == pf.pf_gsid.s_seq) {
				fclose(in);
				fatal(MSGSTR(BEINGEDIT, "Another user is working on the delta you specified.\n\
\tWait until the delta is not in use. (rc12)\n"));  /* MSG */
		}
	}
	fclose(in);
	return;
}


static ckmrs(pkt,p)
struct packet *pkt;
char *p;
{
	register char **argv;
	char mr_no[BUFSIZ];
	char *mr_ptr, *fmalloc();

	copy(p,mr_no);
	mr_ptr = mr_no;
	repl(mr_ptr,'\n','\0');
	mr_ptr += 3;
	for (argv = Darg; *argv; argv++)
		if (equal(mr_ptr,*argv)) {
			pkt->p_wrttn = 1;
			copy(mr_ptr,*eargv++ = fmalloc(size(mr_ptr)));
			*eargv = 0;
		}
}


static put_delmrs(pkt)
struct	packet	*pkt;
{

	register char	**argv;
	register int first;
	char	str[64];

	first = 1;
	for(argv = Earg; *argv; argv++) {
		if (first) {
			putline(pkt,"c *** LIST OF DELETED MRS ***\n");
			first = 0;
		}
		sprintf(str,"%c%c %s\n",CTLCHAR,COMMENTS,*argv);
		putline(pkt,str);
	}
}
