static char sccsid[] = "@(#)76  1.28  src/bos/usr/bin/trcrpt/updt.c, cmdtrace, bos41J, 9520A_all 5/12/95 16:42:43";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: updtmain
 *
 * ORIGINS: 27, 83
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
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

/*
 * trcupdate command line interface and control flow.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "rpt.h"
/* Add include files */
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
/* ----------------- */

#define LASTCHAR(cp) (cp[strlen(cp)-1]) /* look for '\\' continuation char */

extern optind;
extern char *optarg;
extern char *malloc();

extern FILE *Tmpltfp;
extern FILE *Listfp;
extern verboseflg;
extern char *Infile;

static struct tindex Utindexes[NHOOKIDS];       /* same size as Tindexes */
static short Utraceids[MAXTEMPLATES];           /* same size as Traceids */
static char updtmode[NHOOKIDS];
#define M_PASS    1
#define M_ADD     2
#define M_UPDATE  3
#define M_DELETE  4

static FILE *Updtfp;
static FILE *Updttmpfp;
static FILE *Tmplttmpfp;
static FILE *Undofp;
static FILE *Undotmpfp;
static char *Updtfile;
static char *Undofile;
static char *Magic = "* /etc/trcfmt";           /* first line of update file */
static overrideflag;
static xtractflag;
static char *xstring;

#define SEMA_KEY 16893671
extern sid;
char *Listfile;
extern FILE *Listfp;
int Condflg;
char *Tmpltfile = TMPLT_DFLT;

/*
 * Called from main to do trcupdate function.
 * Create an 'additions' file that looks like template file for gettmplt.
 * Compile to check for syntax errors.
 * Copy all the entries that are not to be deleted from the template file
 *  into a 'tempfile', merging in the additions from the additions file.
 * Then truncate the original template file and copy the 'tempfile' onto
 *  the original template file.
 */

/* 
 * at the beginning of updtmain, call to updtsema(), which gets a semaphore.
 * This semaphore is got to protect the compress (or uncompress routine). This
 * semaphore is released in genexit() routine.
 * call to the updtuncompress() routine to uncompress the file /etc/trcfmt
 * if need be. This file is compressed again in genexit() routine.
 */
main(argc,argv)
char *argv[];
{
	(void) setlocale(LC_ALL,"");
        setprogname();
        catinit(MCS_CATALOG);
        Tmpltfile = vset(Tmpltfile);
        if(signal(SIGINT, jsignal) == SIG_IGN)
                signal(SIGINT,SIG_IGN);
        if(signal(SIGQUIT, jsignal) == SIG_IGN)
                signal(SIGQUIT,SIG_IGN);
        signal(SIGTERM,jsignal);
        signal(SIGSEGV,jsignal);
      ressyminit();
      if (geteuid()) {
           cat_eprint(CAT_TRC_ROOTUSAGE," permission denied: must have root a\
uthority.\n");
            genexit(1);
	}

	getcmdline(argc,argv);  /* read command line and fill in values */
	vprint("Program name %s\n",Progname);
	updtsema();                    /* get a semaphore   */
	updtopen();                             /* open template, update, and temp files */
	updtrdupdt();                   /* create update/add template file from Updtfile */
	updtgettmpltinit();             /* fill in Utindexes */
	if(Errflg)
		genexit(1);
	updtpass1();                    /* scan update/adds for syntax errors */
	Infile = Tmpltfile;
	gettmpltinit();                 /* fill in Tindexes */
	if(Errflg)
		genexit(1);
	updtsort();                             /* make list of the traceids in file order */
	updtmerge();                    /* copy trcfmt and updates to tempfile */
	updtrewrite();                  /* copy tempfile onto trcfmt file */
	genexit(0);
}

/*
 * NAME:      updtopen
 * FUNCTION:  open the template file, the .undo file, and the input .trc file.
 *            Also open 3 temp. files:   Updttmpfp, Undotmpfp, and Tmplttmpfp
 * INPUTS:    none
 * RETURNS:   none
 */
static updtopen()
{
	char *cp;

	vprint("Opening template file %s\n",Tmpltfile);
	updtuncompress();
	if((Tmpltfp = fopen(Tmpltfile,"r")) == NULL) {
		if(access(Tmpltfile,0) == 0) {  /* usually some permission problem */
			perror(Tmpltfile);
			genexit(1);
		}
		if((Tmpltfp = fopen(Tmpltfile,"w+")) == NULL) {
			perror(Tmpltfile);
			genexit(1);
		}
	}
	Updttmpfp  = tmpfile();
	Undotmpfp  = tmpfile();
	Tmplttmpfp = tmpfile();
	if(Updttmpfp == NULL || Undotmpfp == NULL || Tmplttmpfp == NULL) {
		perror("trcupdate: cannot open tempfiles");
		genexit(1);
	}
	if(xtractflag)
		return;
	if(streq(Updtfile + strlen(Updtfile) - 5,".undo"))
		Undofile = "/dev/null";  /* no undo's for .undo files */
	else {
#define UPDTSUFF ".undo.trc"
		Undofile = malloc(strlen(Updtfile)+strlen(UPDTSUFF)+1);
		sprintf(Undofile,"%s%s",Updtfile,UPDTSUFF);
	}

	cp = malloc(strlen(Updtfile)+5);
	sprintf(cp,"%s.trc",Updtfile);
	Updtfile = cp;
	vprint("Opening updatefile %s\n",Updtfile);
	if((Updtfp = fopen(Updtfile,"r")) == NULL) {
		perror(Updtfile);
		genexit(1);
	}
	vprint("Undofile is %s\n",Undofile);
}

/*
 * NAME:      updtrdupdt
 * FUNCTION:  create update/add template file from Updtfile
 * INPUTS:    none
 * RETURNS:   none
 *
 * Read in the .trc file.
 * Make sure that the "* /etc/trcfmt" string is the first line.
 * Fill in the updtmode[] array with ADD/DELETE for each template.
 * For each ADD template, copy to the Updttmpfp file with the '+'
 *   stripped so that it can be read by gettmplt()
 */

#define ST_START 3
#define ST_BODY  4
#define ST_SYNC  5

static updtrdupdt()
{
	char *cp;
	int traceid;
	int uoffset;
	int state;
	char line[256];

	if(xtractflag) {
		for(cp = strtok(xstring,", \t\n"); cp; cp = strtok(0,", \t\n")) {
			if((traceid = strtoid(cp)) < 0)
				continue;
			updtmode[traceid] = M_DELETE;
		}
		return;
	}
	Lineno = 1;
	if(fgets(line,256,Updtfp) == NULL)
		genexit(0);
	Lineno++;
	strip(line);                    /* remove trailing whitespace */
	if(!streq(line,Magic))
		cat_lerror(CAT_UPT_MAGIC,"Expecting '%s' on the first line of the update file.\n",Magic);
	state = ST_START;
	for(;;) {
		if(fgets(line,256,Updtfp) == NULL)
			break;
		Lineno++;
		if(Errflg > 10)
			genexit(1);
		if(line[0] == '*' || line[0] == '#')
			continue;
		strip(line);            /* remove trailing whitespace */
		if(line[0] == '\0')
			continue;
		switch(state) {
		case ST_SYNC:           /* sync: look for start of next template */
			if(LASTCHAR(line) == '\\')
				continue;
		case ST_START:          /* look for + or - */
			switch(line[0]) {
			case '+':
				cp = &line[1];
				while(*cp == ' ' || *cp == '\t')
					cp++;
				uoffset = ftell(Updttmpfp);
				fprintf(Updttmpfp,"%s\n",cp);
				if(LASTCHAR(cp) == '\\')
					state = ST_BODY;
				cp = strtok(&line[1]," \t");
				if(cp == NULL) {
					cat_lerror(CAT_UPT_EOL,"Unexpected End of Line in the + or - template.\n");
					continue;
				}
				if((traceid = strtoid(cp)) < 0) {
					state = ST_SYNC;
					continue;
				}
				updtmode[traceid] = M_ADD;
				Debug("ADD: traceid=%s\n",hexstr(traceid));
				continue;
			case '-':
				cp = strtok(&line[1]," \t");
				if(cp == NULL) {
					cat_lerror(CAT_UPT_EOL,"Unexpected End of Line in the + or - template.\n");
					continue;
				}
				if(strtok(0," \t")) {
					cat_lerror(CAT_UPT_FIELDS,
"There are too many fields on the delete template.\n");
					state = ST_SYNC;
					continue;
				}
				if((traceid = strtoid(cp)) < 0) {
					state = ST_SYNC;
					continue;
				}
				updtmode[traceid] = M_DELETE;
				Debug("DELETE: traceid=%s\n",hexstr(traceid));
				state = ST_START;
				continue;
			default:
				cat_lerror(CAT_UPT_EXPPM,
"Expecting + or - at the beginning of the line.\n\
This could mean that the beginning of this template is wrong,\n\
or that the previous template is missing a '\\' continuation\n\
character.\n");
				state = ST_SYNC;
				continue;
			}
		case ST_BODY:           /* keep writing this update entry */
			fprintf(Updttmpfp,"%s\n",line);
			if(LASTCHAR(line) != '\\')
				state = ST_START;
			continue;
		default:
			Debug("updtrdupdt: state=%d",state);
			genexit(1);
		}
	}
}

/*
 * NAME:      updtgettmpltinit
 * FUNCTION:  call gettmpltinit with the add/update templates
 * INPUTS:    none
 * RETURNS:   none
 *
 * Save the resulting Tindexes[] structure array in Utindexes[] and
 * the Traceids[] array or traceids in Utraceids[].
 */
static updtgettmpltinit()
{
	FILE *fpsv;
	char *filesv;

	if(xtractflag)
		return;
	fseek(Updttmpfp,0,0);   /* seek to beginnig of Updttmpfp */
	fpsv   = Tmpltfp;
	filesv = Tmpltfile;
	Tmpltfp   = Updttmpfp;  /* gettmpltinit uses Tmpltfp */
	Tmpltfile = Updtfile;
	Infile    = Tmpltfile;
	gettmpltinit();                 /* initialize gettmplt and fill in Tindexes */
	if(Errflg)
		return;
	memcpy(Utindexes,Tindexes,sizeof(Utindexes));   /* save Tindexes */
	memcpy(Utraceids,Traceids,sizeof(Utraceids));   /* save Traceids */
	Tmpltfp   = fpsv;
	Tmpltfile = filesv;
}

/*
 * NAME:      updtpass1
 * FUNCTION:  call pass1 with the add/update template file so that
 *            it can be syntax-checked.
 * INPUTS:    none
 * RETURNS:   none
 *
 * If errors are detected, exit.
 */

static updtpass1()
{
	FILE *fpsv;
	char *filesv;

	/*
         * scan update/add file for syntax errors
         */
	fseek(Updttmpfp,0,0);   /* seek to beginnig of Updttmpfp */
	fpsv      = Tmpltfp;
	filesv    = Tmpltfile;
	Tmpltfp   = Updttmpfp;  /* gettmpltinit and updtpass1 use Tmpltfp */
	Tmpltfile = Updtfile;
	checkflag = 1;
	pass1();
	Tmpltfp   = fpsv;
	Tmpltfile = filesv;
	if(Errflg)
		cat_fatal(CAT_UPT_SC,
"Errors were detected in the update file %s.\n\
No update to %s made.\n",
		    Updtfile,Tmpltfile);
}

/*
 * NAME:      updtsort
 * FUNCTION:  call pass1 with the add/update template file so that
 *            it can be syntax-checked.
 * INPUTS:    none
 * RETURNS:   none
 *
 * Do version checking.
 * Determine which templates are ADDs and which are UPDATEs.
 * Fill in array Utraceids[] of traceids in the order in which
 *  they are to be written to the new trcfmt file.
 */
static updtsort()
{
	int ui;
	int ti;
	int traceid;
	int tv,uv,tr,ur;

	Debug("updtsort\n");
	for(traceid = 0; traceid < NHOOKIDS; traceid++) {
		if(Tindexes[traceid].t_state & T_DEFINED) {
			switch(updtmode[traceid]) {
			case M_ADD:
				Debug("ADD->UPDATE traceid=%s\n",hexstr(traceid));
				tv = Tindexes[traceid].t_version;
				tr = Tindexes[traceid].t_release;
				uv = Utindexes[traceid].t_version;
				ur = Utindexes[traceid].t_release;
				if(!overrideflag && (tv > uv || tv == uv && tr > ur)) {
					if(!quietflag)
						cat_eprint(CAT_UPT_EARLIER,"\
Cannot update traceid %03X to an earlier version.\n%d.%d vs. %d.%d",
						    traceid,tv,tr,uv,ur);
					updtmode[traceid] = M_PASS;
				} else {
					updtmode[traceid] = M_UPDATE;
				}
				break;
			case 0:
				updtmode[traceid] = M_PASS;
				break;
			case M_DELETE:
				Debug("DELETE traceid=%s\n",hexstr(traceid));
				break;
			default:
				Debug("updtsort: Unknown mode %d",updtmode[traceid]);
				genexit(1);
			}
		} else {
			switch(updtmode[traceid]) {
			case M_ADD:
				Debug("ADD traceid=%s\n",hexstr(traceid));
				break;
			case M_DELETE:
				cat_eprint(CAT_UPT_TRACEID,
				    "Traceid %03X is not in the file %s. Cannot delete.\n",
				    traceid,Tmpltfile);
				updtmode[traceid] = 0;
				break;
			case 0:
				break;
			default:
				Debug("updtsort: unknown mode %d",updtmode[traceid]);
				genexit(1);
			}
		}
	}
	/*
         * This step tries to put new templates in numeric order.
         * For each ADD template in the updtmode[] array, add to the Utraceids[]
         *   array.
         * The Utraceids[] array contains the ids in order of occurrence in the
         *   new trcfmt file.
         */
	ui = 0;
	for(ti = 0; ti < MAXTEMPLATES; ti++) {
		traceid = Traceids[ti];
		if(traceid == 0)
			break;
		switch(updtmode[traceid]) {
		case M_UPDATE:
		case M_PASS:    /* PASS = copy from old to new */
			Utraceids[ui++] = traceid;
			break;
		case M_DELETE:
			break;
		}
	}
	for(traceid = 0; traceid < NHOOKIDS; traceid++)
		if(updtmode[traceid] == M_ADD)
			Utraceids[ui++] = traceid;
}

/*
 * There are two files to combine into one file Tmplttmpfp.
 * Read in order from original template file Tmpltfp,
 *  using Traceids[], the array of traceids
 *  in the order in which they occur in Tmpltfp,
 *  and combine with Updttmpfp, the add/update file.
 * Using Ttraceids keeps from having to re-scan for the traceid.
 *
 * For each entry in Ttraceids[]: (for each template in Tmpltfp)
 * If the traceid is to be deleted, (updtmode[i] == M_DELETE)
 *  do not copy template (from Tmpltfp) to Tmplttmpfp.
 * If the traceid is to be updated, (updtmode[i] == M_UPDATE)
 *  when its traceid appears, copy template (from Updttmpfp) to Tmplttmpfp.
 * If the traceid is to be added, (updtmode[i] == M_ADD)
 *  when its the first slot that is can fit appears,
 *  (Ttraceids[x] < traceid < Ttraceids[x+1])
 *  copy template (from Updttmpfp) to Tmplttmpfp.
 * Otherwise, copy template from Tmpltfp to Tmplttmpfp.
 * Note that updtmerge uses file offsets and character io instead of
 *  line numbers and line io.
 */
static updtmerge()
{
	int ui;
	int traceid;
	int toffset;
	int tcount;
	int uoffset;
	int ucount;
	int cmttoffset;
	int cmtuoffset;

	Debug("updtmerge\n");
	fprintf(Undotmpfp,"%s\n",Magic);
	for(traceid = 0; traceid < NHOOKIDS; traceid++) {
		if(updtmode[traceid] == M_DELETE) {
			toffset = Tindexes[traceid].t_offset;
			tcount  = Tindexes[traceid].t_size;
			fprintf(Undotmpfp,"+ ");
			vprint("Deleting traceid %s\n",hexstr(traceid));
			undocopy(Tmpltfp,toffset,tcount);
		}
	}
	cmttoffset = 0;
	cmtuoffset = 0;
	for(ui = 0; ui < MAXTEMPLATES; ui++) {
		traceid = Utraceids[ui];
		if(traceid == 0)
			break;
		toffset = Tindexes[traceid].t_offset;
		tcount  = Tindexes[traceid].t_size;
		uoffset = Utindexes[traceid].t_offset;
		ucount  = Utindexes[traceid].t_size;
		switch(updtmode[traceid]) {
		case M_PASS:                                    /* Tmpltfp -> Tmplttmpfp */
			cmttcopy(Tmpltfp,cmttoffset,toffset);
			cmttoffset = toffset;
			tcopy(Tmpltfp,toffset,tcount);
			continue;
		case M_ADD:                             /* Updttmpfp -> Tmplttmpfp */
			fprintf(Undotmpfp,"- %s\n",hexstr(traceid));
			tcopy(Updttmpfp,uoffset,ucount);
			vprint("Adding   traceid %s\n",hexstr(traceid));
			continue;
		case M_UPDATE:                           /* Updttmpfp -> Tmplttmpfp */
			if (ui == 0){		/* take care of leading comments, when updating first
								   template in repository */
				cmttcopy(Tmpltfp,0,toffset);
				cmttoffset = toffset+tcount;
			}
			else
				cmttcopy(Updttmpfp,cmtuoffset,uoffset);
			cmtuoffset = uoffset;
			tcopy(Updttmpfp,uoffset,ucount);
			fprintf(Undotmpfp,"+ ");
			undocopy(Tmpltfp,toffset,tcount);
			vprint("Updating traceid %s\n",hexstr(traceid));
			continue;
		default:
			Debug("updtmerge: Unknown mode %d",updtmode[traceid]);
			genexit(1);
		}
	}
	cmttcopy(Tmpltfp,cmttoffset,0x7FFFFFFF);        /* "flush" to EOF */
	Debug("return from updtmerge. ui=%d\n",ui);
}

/*
 * copy the 'count' byte template at offset 'offset' from fp file
 * to the Undo file Undotmpfp.
 */
static undocopy(fp,offset,count)
FILE *fp;
{
	int c;

	Debug("undocopy(%x,%d.,%d.)\n",fp,offset,count);
	fseek(fp,offset,0);
	while(--count >= 0) {
		if((c = fgetc(fp)) == EOF)
			cat_fatal(CAT_UPT_UNXPEOF,
"Unexpected end of file condition was detected during the\n\
read of template file. This could mean that a temporary file\n\
was deleted from the system. Re-run the command.\n");
		putc(c,Undotmpfp);
	}
}

/*
 * Copy comment lines from fp.coffset to fp.toffset.
 */
static cmttcopy(fp,coffset,toffset)
FILE *fp;
{
	int c;
	int bolflg;
	int nlflg;

	Debug("cmttcopy(%x,%d.,%d.)\n",fp,coffset,toffset);
	fseek(fp,coffset,0);
	nlflg = 1;
	while(coffset < toffset) {
		bolflg = nlflg;
		nlflg = 0;
		if((c = fgetc(fp)) == EOF)
			return;
		coffset++;
		switch(c) {
		case '\n':              /* print blank lines */
			nlflg++;
			putc(c,Tmplttmpfp);
			continue;
		case '#':               /* print commented lines */
		case '*':               /* print commented lines */
			if(!bolflg)
				continue;
			putc(c,Tmplttmpfp);
			while((c = fgetc(fp)) != '\n') {
				if(c == EOF)
					return;
				coffset++;
				putc(c,Tmplttmpfp);
			}
			coffset++;
			putc(c,Tmplttmpfp);
			nlflg++;
			continue;
		default:                /* skip the other lines */
			while((c = fgetc(fp)) != '\n') {
				if(c == EOF)
					return;
				coffset++;
			}
			continue;
		}
	}
}

/*
 * copy the 'count' byte template at offset 'offset' from fp file
 * to the template file Tmplttmpfp.
 */
static tcopy(fp,offset,count)
FILE *fp;
{
	int c;

	Debug("tcopy(%x,%d.,%d.)\n",fp,offset,count);
	fseek(fp,offset,0);
	while(--count >= 0) {
		if((c = fgetc(fp)) == EOF)
			cat_fatal(CAT_UPT_UNXPEOF,
"Unexpected end of file condition was detected during the\n\
read of template file. This could mean that a temporary file\n\
was deleted from the system. Re-run the command.\n");
		putc(c,Tmplttmpfp);
	}
}

/*
 * Copy the tempfile Tmplttmpfp on top of the original template file Tmpltfp
 * Make this step as indivible as possible by catching common signals.
 */
static updtrewrite()
{
	int c;

	fseek(Tmplttmpfp,0,0);
	fseek(Undotmpfp,0,0);
	fclose(Tmpltfp);
	if(xtractflag) {
		while((c = fgetc(Undotmpfp)) != EOF)
			fputc(c,stdout);
		return;
	}
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	vprint("Opening template file %s for write\n",Tmpltfile);
	if((Tmpltfp = fopen(Tmpltfile,"w+")) == NULL) {
		perror(Tmpltfile);
		genexit(1);
	}
	vprint("Opening undo file %s for write\n",Undofile);
	if((Undofp = fopen(Undofile,"w+")) == NULL) {
		perror(Undofile);
		genexit(1);
	}
	vprint("Copying to %s\n",Tmpltfile);
	while((c = fgetc(Tmplttmpfp)) != EOF)
		fputc(c,Tmpltfp);
	fclose(Tmpltfp);
	vprint("Copying to %s\n",Undofile);
	while((c = fgetc(Undotmpfp)) != EOF)
		fputc(c,Undofp);
	fclose(Undofp);
}

/*
 * scan command line options using getopt.
 */
static getcmdline(argc,argv)
char *argv[];
{
	int c;
	int n;
	int len;
	int i;
	char *cp;

	while((c = getopt(argc,argv,"qx::ot:vHL")) != EOF) {
		switch(c) {
		case 'x':
			xtractflag++;
			xstring = optarg;
			break;
		case 'o':
			overrideflag++;
			break;
		case 'v':
			verboseflg++;
			break;
		case 'q':
			quietflag++;
			break;
		case 'H':
			usage();
		case 't':
			Tmpltfile = optarg;
			break;
/*
		case 'D':
			debuginit("Btrace");
			break;
*/
		case 'L':               /* list parsed templates */
			Listflg++;
			break;
		case '?':
			usage();
		}
	}
	if(!xtractflag) {
		if(optind == argc) {
			Debug("Missing update filename");
			usage();
		}
		Updtfile = argv[optind];
	} else {
		Updtfile = "XTRACT";
	}
	if(Listflg) {
		vprint("Opening list file %s\n",Listfile);
		if((Listfp = fopen(Listfile,"w+")) == NULL) {
			perror(Listfile);
			genexit(1);
		}
	}
	if(!quietflag && Listfp == NULL)
		Listfp = stderr;
}

static usage()
{

	cat_eprint(CAT_TRC_UPDATE,"\
Usage:  trcupdate -o -t tmpltfile -x idlist file\n\
\n\
Update /etc/trcfmt template file from file.trc\n\
\n\
-o           No version number checking\n\
\n\
-t tmpltfile Use 'tmpltfile' as template file instead of /etc/trcfmt\n\
-x idlist    Extract the templates in 'idlist' from template file to stdout\n");
	genexit(1);
}

/*
 * Name : updtsema
 * Function: Get a semaphore
 * Inputs: none
 * Outputs: none; if error exit.
 */
updtsema()
{
struct sembuf sb;


	if ((sid = semget(SEMA_KEY, 1, 0666)) == -1) {
		if (errno == ENOENT) {  /* the semaphore not yet exist  */ 
			if ((sid = semget(SEMA_KEY, 1, 0666|IPC_CREAT)) == -1) {
				perror ("semget");
				exit(1);
			}
			
		} else {
			perror ("semget");
			exit(1);
		  }
	}
	else {
		/* Grab the semaphore */
		sb.sem_num = 0;
		sb.sem_op = -1;
		sb.sem_flg = 0;
		if (semop(sid, &sb, 1) == -1) {
			perror ("semopget");
			exit(1);
		}
	}
	semaflg++;
}


/*
 * name : updtuncompress
 * Function: uncompress the file /etc/trcfmt if any need (trace package is not
installed)
 *    At first, we verify if the tmpltfile exists, if yes we return
 *    Then, we verify if tmpltfile.Z exists; if so, we uncompess it
 *    otherwise we return to the caller
 * Inputs: none
 * Outputs: none
 */

updtuncompress()
{
struct stat buf;
char cmd[256];
char Tmpltfile_Z[256];


	if(stat(Tmpltfile, &buf) == 0)
		return;

	if (errno != ENOENT) {
		perror(Tmpltfile);
		genexit(1);
	}

	sprintf(Tmpltfile_Z, "%s%s", Tmpltfile,".Z");

	if(stat(Tmpltfile_Z, &buf) != 0) {
		if (errno != ENOENT) {
			perror(Tmpltfile);
			genexit(1);
		} else
			return;
	}

	sprintf(cmd,"%s %s", "/usr/bin/uncompress", Tmpltfile);
 	if (system(cmd) != 0) {
		perror("uncompress");
		genexit(1);
	}

/* We verify again that the Tmpltfile is really created, because in certain
 * case, the uncompress() routine returns 0, but the Tmpltfile is not created .
 * ( for exemple when Tmplfile.Z is not a compressed file )
 */
	if(stat(Tmpltfile, &buf) != 0) {
		perror(Tmpltfile);
		genexit(1);
	}

	compressflg++;
}
