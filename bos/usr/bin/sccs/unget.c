static char sccsid[] = "@(#)96 1.15 src/bos/usr/bin/sccs/unget.c, cmdsccs, bos41B, 9504A 12/9/94 10:00:08";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: catpfile, clean_up, edpfile, unget, main
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 *
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include 	<locale.h>
#include        <nl_types.h>
#include	"defines.h"
#include	"had.h"
#include 	"unget_msg.h"
#include 	<stdlib.h>
#include	<errno.h>
#define MSGSTR(Num, Str) catgets(catd, MS_UNGET, Num, Str)
#define FCLOSE(file) \
	{ if ( fclose(file) ) { \
		cmd ? perror("sact") : perror("unget"); \
		clean_up(1); \
		exit(1); \
	}}

/*
		Program can be invoked as either "unget" or
		"sact".  Sact simply displays the p-file on the
		standard output.  Unget removes a specified entry
		from the p-file.
*/

static struct stat Statbuf;

static int	verbosity;
static int	num_files;
static int	cmd = 0;
static long	Szqfile;
static char	had[26];
static char	Pfilename[FILESIZE];
char	*auxf();
static struct	packet gpkt;
static struct	sid sid;

nl_catd catd;

main(argc,argv)
int argc;
char **argv;
{
	char	*p;
	char	*sid_ab();
	extern	unget();
	extern	int Fcnt;
	int	c;
	

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;

	/*	If envoked as "sact", set flag
		otherwise executed as "unget".
	*/
	if (equal(sname(argv[0]),"sact")) {
		cmd = 1;
		HADS = 0;
	}

	/* distinguish between "sact" and "unget" flags in getopt */

	while ((c = getopt(argc, argv, cmd ? "" : "nsr:")) != EOF)
	{
		switch (c)
		{
			case 'r':
				chksid(sid_ab(optarg,&sid),&sid);
				break;
			case 'n':
			case 's':
				break;
			default :
				if (cmd)
					fprintf(stderr,MSGSTR(USAGESACT, 
					   "Usage: sact File...\n"));
				else
					fprintf(stderr,MSGSTR(USAGEUNGET,
					   "Usage: unget [ -r SID ] [ -s ] [ -n ] File...\n"));
				exit(1);
		}
		if (had[c - 'a']++) {
			fatal(MSGCM(KEYLTRTWC, "\nUse the -%c flag only once on the command line. (cm2)\n"),c);
		}
	} /* while */

	if ((num_files = argc - optind) == 0)
		fatal(MSGCM(MISSFLNAM, "\nSpecify the file to process.  (cm3)\n"));

	if (!HADS)
		verbosity = -1;
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for ( ; optind < argc; optind++)
		if (argv[optind])
			do_file(argv[optind],unget);
	exit(Fcnt ? 1 : 0);
}



static unget(file)
char *file;
{
	extern	char had_dir, had_standinp;
	extern	char *Sflags[];
	int	i, status;
	char	gfilename[FILESIZE];
	char	str[BUFSIZ];
	char	*sid_ba();
	struct	pfile *pp, *edpfile();

	if (setjmp(Fjmp))
		return;

	/*	Initialize packet.  Open the file to get r/w status.
	*/
	sinit(&gpkt,file,1);			/* P46353 */
	if (gpkt.p_iop) fclose(gpkt.p_iop);
	gpkt.p_stdout = stdout;
	gpkt.p_verbose = verbosity;

	copy(auxf(gpkt.p_file,'g'),gfilename);
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	/*	If envoked as "sact", call catpfile() and return.
	*/
	if (cmd) {
		catpfile(&gpkt);
		return;
	}

	if (lockit(auxf(gpkt.p_file,'z'),2,getpid()))
		fatal(MSGCM(LOCKCREAT,"\nCannot create the lock file. (cm4)\n"));
	pp = edpfile(&gpkt,&sid);
	if (gpkt.p_verbose) {
		sid_ba(&pp->pf_nsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}

	/*	If the size of the q-file is greater than zero,
		rename the q-file the p-file and remove the
		old p-file; else remove both the q-file and
		the p-file.
	*/
	if (Szqfile)
		rename(auxf(gpkt.p_file,'q'),Pfilename);
	else {
		xunlink(Pfilename);
		xunlink(auxf(gpkt.p_file,'q'));
	}
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());

	/*	A child is spawned to remove the g-file so that
		the current ID will not be lost.
	*/
	if (!HADN) {
		if ((i = fork()) < 0)
			fatal(MSGCO(CANTFORK, "\nCannot create another process at this time.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));
		if (i == 0) {
			setuid(getuid());
			unlink(gfilename);
			exit(0);
		}
		else {
			wait(&status);
		}
	}
}


static struct pfile *
edpfile(pkt,sp)
struct packet *pkt;
struct sid *sp;
{
	static	struct pfile goodpf;
	char	*user, *logname();
	char	line[LINE_MAX+1];
	struct	pfile pf;
	int	cnt, name;
	FILE	*in, *out, *fdfopen();

	cnt = -1;
	name = 0;
	user = logname();
	zero((char *)&goodpf,sizeof(goodpf));
	in = xfopen(auxf(pkt->p_file,'p'),0);
	out = xfcreat(auxf(pkt->p_file,'q'),0644);
	while (fgets(line,LINE_MAX+1,in) != NULL) {
		pf_ab(line,&pf,1);
		if (equal(pf.pf_user,user)) {
			name++;
			if (sp->s_rel == 0) {
				if (++cnt) {
					FCLOSE(out);
					fclose(in);
					fatal(MSGSTR(SIDSPEC1, "\nThere is more than one outstanding SID in the p-file.\n\
\tSpecify with the -r flag the new SID you want to unget. (un1)\n"));
				}
				goodpf = pf;
				continue;
			}
			else if (sp->s_rel == pf.pf_nsid.s_rel &&
				sp->s_lev == pf.pf_nsid.s_lev &&
				sp->s_br == pf.pf_nsid.s_br &&
				sp->s_seq == pf.pf_nsid.s_seq) {
					goodpf = pf;
					continue;
			}
		}
		fputs(line,out);
	}
	fflush(out);
	fstat(fileno(out),&Statbuf);
	Szqfile = Statbuf.st_size;
	copy(auxf(pkt->p_file,'p'),Pfilename);
	FCLOSE(out);
	fclose(in);
	if (!goodpf.pf_user[0])
		if (!name)
			fatal(MSGSTR(LOGNAME, "\nYour user name is not in the p-file.\n\
\tLog in with the user name shown in the p-file. (un2)\n"));
		else fatal(MSGSTR(SIDSPEC3, "\nThe specified SID does not exist.\n\
\tCheck the p-file for existing SID numbers. (un3)\n"));
	return(&goodpf);
}


/* clean_up() only called from fatal().
*/
static clean_up(n)
{
	/*	Lockfile and q-file only removed if lockfile
		was created by this process.
	*/
	if (gpkt.p_iop) fclose(gpkt.p_iop);
	if (mylock(auxf(gpkt.p_file,'z'),getpid())) {
		unlink(auxf(gpkt.p_file,'q'));
		ffreeall();
		unlockit(auxf(gpkt.p_file,'z'),getpid());
	}
}


static catpfile(pkt)
struct packet *pkt;
{
	int c;
	FILE *in;

	if(!(in = fopen(auxf(pkt->p_file,'p'),"r")))
		fprintf(stderr,MSGSTR(NOOUTDELTA, "There are no outstanding deltas for %s.\n"),pkt->p_file);
	else {
		while ((c = getc(in)) != EOF)
			putc(c,pkt->p_stdout);
		fclose(in);
	}
}
