static char sccsid[] = " @(#)21        1.5  src/bos/usr/bin/errlg/errdead/main.c, cmderrlg, bos411, 9428A410j 12/2/93 14:13:03";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, usage, gen_exit, raise_dead, init_errdead, getcmdline
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <sys/stat.h>
#include <sys/erec.h>
#include <locale.h>
#include <errlg.h>

static void usage(void);
static int  init_errdead(void);
static int  eread_errdead(struct buf_entry *bp);
static void raise_dead(void);
static void getcmdline(int argc,char *argv[]);

extern optind;
extern char *optarg;


char Logfile[PATH_MAX]=ERRLOG_DFLT;
int Logfd;
struct log_hdr   log_hdr;
struct log_entry log_entry;

int   Threshold;	/* value, in bytes, for the logfile to wrap at */
char *Errorfile;	/* temporary file to hold dump image error buffer */
int   Errorfd;		/* file descriptor for Errorfile */
char *Dumpfile;		/* name of dump image for errdead */

struct errc_io errc_io;  /* errdd buffer info struct */
int tmpltflg;            /* specified template repository not available. */


/*
 * NAME:     main
 * FUNCTION: Set up user environment.  Initialize message catalog.
 *           Process the command line.  Get threshold value from ODM.
 *			 Get errlog dump information from the supplied dump image.
 *           Call raise_dead() to add any error buffer entries to an
 *			 error log file.
 *			 Note:
 *				This command has an external command dependency on:
 *					dumpfmt
 * RETURNS:	 None
 */

main(int argc, char *argv[])
{
	char *str;
	struct	stat	sbuf;

	setlocale(LC_ALL,"");				/* Set the environment  */
	setprogname();						/* Set the Program name */
	catinit(MCS_CATALOG);				/* cmderrlg message catalog */
	getcmdline(argc,argv);				/* process command line */
	get_threshold();		/* get errlog threshold from odm */
	if(init_errdead())					/* read in dump images */
		raise_dead();					/* raise the dead errbuffer entries. */
}

/*
 * NAME:     usage
 * FUNCTION: Display the usage message to stderr.
 * RETURNS:  None, exits
 */

static void
usage(void)
{

	cat_eprint(CAT_USAGE_DEAD,"\
Usage:\n\
errdead -i filename dumpfile\n\
\n\
Extract error log entries from a system dump and insert into an error log.\n\
-i filename   Put error log entries in to the supplied error log file.\n");
	exit(1);
}

#define ISBETWEEN(x,a,b) ( (a) <= (x) && (x) <= (b) )

/*
 * NAME:     init_errdead
 * FUNCTION: Read dump images via a pipe with the dmpfmt command.
 * INPUTS:   NONE
 * RETURNS:  1 - success
 *			 0 - failure
 */

static int
init_errdead(void)
{
	char cmd[512];
	FILE *fp;
	int rv;

	/*
	 * Set up a pipe to a dmpfmt command to read the dump image,
	 * and write it to Dumpfile.
	 */
	sprintf(cmd,"%s -x -C errlg -A errc_io %s",DMPFMT,Dumpfile);

	if((fp = popen(cmd,"r")) == NULL) {
		perror(cmd);
		rv = 0;
	}
		/*
		 * Read in the errc_io structure from the dump image and
		 * validate start, end, in, out, and stale pointers, and 
		 * the count of entries.
		 */
	else if((rv=fread(&errc_io,sizeof(errc_io),1,fp)) != 1)
		rv = 0;
	else if(errc_io.e_err_count == 0) /* no entries */
		rv = 0;
	else if(errc_io.e_err_count < 0) /* corrputed errc_io */
		cat_eprint(CAT_DEAD_ERRC_IO_ERR_COUNT, "Invalid error count encountered in dump image %s.\n",Dumpfile);
	else if(errc_io.e_start == 0 ||
			errc_io.e_end == 0 ||
			(errc_io.e_start+ERRBUFSIZE) != errc_io.e_end ||
			!ISBETWEEN(errc_io.e_inptr,errc_io.e_start,errc_io.e_end) ||
			!ISBETWEEN(errc_io.e_outptr,errc_io.e_start,errc_io.e_end) ||
			!ISBETWEEN(errc_io.e_stale_data_ptr,errc_io.e_start,errc_io.e_end)) {
		cat_eprint(CAT_DEAD_ERRC_IO, "Invalid error buffer pointer(s) encountered in dump image %s.\n",Dumpfile);
		rv = 0;
	}
	else {
		pclose(fp);

		/*
		 * Transfer the error log buffer to a temp. file.
		 * Then seek to the logical start of the buffer, which is not
		 * necessarily the same as the start of the file (wraparound).
		 */
		Errorfile = tmpnam(0);
		sprintf(cmd,"%s -x -C errlg -A log %s > %s",DMPFMT,Dumpfile,Errorfile);
		if(shell(cmd)) {
			rv = 0;
		}
		else if((Errorfd = open(Errorfile,0)) == 0) {
			perror(Errorfile);
			rv = 0;
		}
		else {
			/*
			 * Unlinking will cause the file to disappear.
			 * The file is not visible following an unlink, but exists.
			 * The file will be destroyed on close.
			 */
			unlink(Errorfile);
			/*
			 * Seek to the entry at e_outptr.
			 */
			lseek(Errorfd,errc_io.e_outptr - errc_io.e_start,0);
			rv = 1;
		}
	}
	if(rv == 0) {
		cat_eprint(CAT_DEM_NO_RECORD,
			"No error records available.\n");
	}
	return(rv);
}

/*
 * NAME:     eread_errdead
 * FUNCTION: read a buf_entry structure from a dumpfile
 * RETURN:   0  no more entries
 *           x  number of bytes in the erec structure
 *			-1  error
 *
 *   The errc_io structure contains the start, end, current input, current 
 * output, and stale data pointers to the error log buffer.  The errc_io 
 * structure also contains the number of entries in the buffer and the 
 * number of entries overwritten since the last boot. It is one of the data
 * areas in the ERRLG dump.  The absolute values don't have any real
 * significance, only the relative offsets from e_start.

 *   These pointers are important because the error buffer may fill up and
 * wrap.  The wrap is destructive.  The stale data pointer will denote the
 * end of any possible data, and imply that wrap has occured.  The number
 * of entries is important because if the input and output pointers are
 * equal then the buffer may be full or empty.
 *
 *   The reads always start at errc_io.outptr and proceed to errc_io.inptr
 * with the possibility of wrapping to the beginning of the buffer.
 * The buffer may look like:
 *
 *		start                  out                 in             end
 *		 |----------------------!------------------!---------------|
 *
 * or
 *
 *		start                   in                out             end
 *		 |----------------------!------------------!---------------|
 *
 * or
 *
 *		start                 in/out                              end
 *		 |----------------------!----------------------------------|
 *
 */
  
static int
eread_errdead(struct buf_entry *bp)
{
	int rv;
	int len;

	if(errc_io.e_err_count == 0)	/* empty */
		rv = 0;
	else if((rv=read(Errorfd,bp,sizeof(bp->erec.erec_len))) == sizeof(bp->erec.erec_len)) {
		len = bp->erec.erec_len - sizeof(bp->erec.erec_len);	/* don't reread erec_len */
		if(read(Errorfd,(int)bp+sizeof(bp->erec.erec_len),len) == len) {
			errc_io.e_outptr += bp->erec.erec_len;
			errc_io.e_err_count--;
			if(errc_io.e_outptr >= errc_io.e_stale_data_ptr) { /* buffer wrap */
				errc_io.e_outptr = errc_io.e_start;
				lseek(Errorfd,0,0);
			}
			rv = bp->erec.erec_len;
		}
		else
			rv = -1;
	}
	return(rv);
}

/*
 * NAME:     gen_exit
 * FUNCTION: Close the error log and exit with the provided code.
 * INPUTS:   exit code
 * RETURNS:  None, exits
 */

static void
gen_exit(int exitcode)
{

	close(Errorfd);
	_exit(exitcode);
}

/*
 * NAME:     raise_dead
 * FUNCTION: Process entries from the errdd buffer contained in
 *			 a system dump.
 * RETURNS:  NONE
 */

static void
raise_dead(void)
{
#define SBUF_SIZE	128
	char sbuf[SBUF_SIZE];
	int rv;
	struct buf_entry e;			/* structure to hold error record. */

	udb_init();

	siginit();	/* set up signal handling */
	for(;;) {	/* for each error entry. */
		rv = eread_errdead(&e);
		switch(rv) {
		case 0:				/* out of data */
			gen_exit(0); 
		case -1:			/* error */
			sprintf(sbuf,"errdead: read from error device %s",Errorfile);
			perror(sbuf);
			gen_exit(1);
		default:			/* log the entry */
			(void)log(&e);
		}
	}
}

/*
 * NAME:     getcmdline
 * FUNCTION: Process command line arguments
 * RETURNS:  None.
 */

static void
getcmdline(int argc,char *argv[])
{
	int c,logfileset = 0;
	char *options;

	options = "i:";

	while((c = getopt(argc,argv,options)) != EOF) {
		switch(c) {
		case 'i':
			strcpy(Logfile,argv[optind-1]); 
			logfileset++;
			break;
		default:
			usage();
		}
	}
	/* If the user didn't give a log file name, then get */
	/* the current value from ODM.  This is stored under */
	/* the attribute errlg_size.			     */	
	if (!logfileset)   /* get error log path from ODM */
		getlogpath();

	/* dumpfile must be last on cmd line... */
	if(optind >= argc) {
		cat_eprint(CAT_DEAD_NO_DUMP,"Specify a dump file.\n");
		usage();
	}
	Dumpfile = argv[optind];
	if(access(Dumpfile,004)) {
		perror(Dumpfile);
		usage();
	}
}

