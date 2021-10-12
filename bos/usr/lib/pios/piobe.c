static char sccsid[] = "@(#)27	1.19.2.9  src/bos/usr/lib/pios/piobe.c, cmdpios, bos411, 9428A410j 5/4/94 16:58:38";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, config_obe, pio_info, get_str, getdocpipe, getpipe,
 *            run_pipeline, out_pipe, chkprohib, piosystem, terminate,
 *            ascterm_prtr_setup
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Note: the piosystem function is based on system.c from System V.2 */

/*
*******************************************************************************
*******************************************************************************
** MODULE NAME:	piobe.c		
**
** DESCRIPTION:	The subroutines contained herein are those required by piobe
**		to properly set up the printing pipe.  Specifically, these 
**		module drives pioburst and piout.			  
**									 
** MODULE								
**  ROUTINES:	main() - initializes, calls and checks for error returns
**
**		config_obe() - finds the ODM file name, constructs and 
**			updates attribute table.
**
**		pio_info() - builds the header and diagnostic information
**			files.
**
*******************************************************************************
*******************************************************************************
*/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/lpio.h>
#include <unistd.h>
#include <sys/param.h>
#include <memory.h>
#include <time.h>
#include <locale.h>
#include "pioformat.h"
#include "pioout.h"

#define DEFBASEDIR_31	"/usr/lpd/pio"

#define MAXFILES  100           /* maximum number of files per invocation */
#define HDRPIPE   1
#define TRLPIPE   2

#define SET_FLAG(flagattr, strptr, strlen)   \
  { attr_tab_p2 = get_attrtab_ent(flagattr); \
    attr_tab_p2->ptypes.sinfo->ptr = strptr; \
    attr_tab_p2->ptypes.sinfo->len = strlen; \
    attr_tab_p2->flags |= OVERRIDE;  }

#define RESET_FLAG(flagattr)                 \
  { attr_tab_p2 = get_attrtab_ent(flagattr); \
    attr_tab_p2->ptypes.sinfo->len = 0;      \
    attr_tab_p2->flags |= OVERRIDE;          \
    attr_tab_p2->flags ^= OVERRIDE;  }


/*
** Option list passed to piogetopt
*/
char optlist[] = "a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:\
A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z:\
0:1:2:3:4:5:6:7:8:9:";

FILE    *prt_fp;                /* file ptr for print file */
FILE    *diag_fp;               /* file ptr for diagfile (reopened stderr) */
FILE    *fp;                    /* work file pointer */
char    diagfile[150];          /* name of file containing diagnostic info. */
char    hdrfile[150];           /* name of file containing header page */
char    trlfile[150];           /* name of file containing trailer page */
char    shrfile[150];           /* name of file containing page count */
char	tpbuff[PATH_MAX];	/* temp buff for file containig a % sign */

/* The following are referenced by subroutines and so must exist */
int     statusfile = 1;         /* we're running under the spooler */
int     objtot = 0;             /* Number of objects in ODM tables */
int     piopgskip = 0;          /* needed for pioputc and pioputchar */
int     piodatasent = 0;        /* needed for pioputc and pioputchar */

int     gen_info = 0;
int	retcode;
int     piomode = PIO_CURR_MODE;/* ODM values overridden by command line */
int     preview = 0;            /* preview level */
int     diagnostic = 0;         /* diagnostic level */
int     header;                 /* NEVER, ALWAYS, or GROUP */
int     trailer;                /* NEVER, ALWAYS, or GROUP */
int     align;                  /* align (i.e., formfeed) first */
int     feed;                   /* number formfeeds to output (separate entry)*/
int     numcopies;              /* number of copies */
int     numfiles;               /* number of print files */
int     sigterm;                /* TRUE if SIGTERM signal has been received */
int     shr_fd;                 /* file desc for file containing page count */
pid_t   processgrp;             /* ID of children's process group */

long    zeroval = 0;            /* zero int to write to file */
long    bytes_total;            /* total # of bytes in all print files */
long    bytes_done;             /* # of bytes that have been printed */
long    filesize[MAXFILES+1];   /* # of bytes in each print file */

char    datastream[MAXFILES+1]; /* data type of each input file (normally the
				   same, but might be some PostScript files) */
char    totalbytesbuf[20];      /* total bytes in ASCII */
char    bytesdonebuf[20];       /* bytes_done in ASCII */
char    formfeedsbuf[20];       /* # form feeds in ASCII */
char    fragname[100];          /* ".queuename:queuedevicename" */
char    copies_string[20];
char    burst_string[3];
char    mail_string[2];
struct str_info *get_str();
char  *str2ascii();
void   terminate();
struct str_info *strinfop;
char  *basedir;
char  *basevardir;
char  *hdrpipe[MAXFILES+1];   /* pointers to header pipelines */
char  *trlpipe[MAXFILES+1];   /* pointers to trailer pipelines */
char  *docpipe[MAXFILES+1];   /* pointers to document pipelines */
int   doc_ddloc[MAXFILES+1];  /* offsets into constructed document pipelines
                                 where pioout pipe should be inserted */
int   hdr_ddloc[MAXFILES+1];  /* offsets into constructed header page pipelines
                                 where pioout pipe should be inserted */
int   trl_ddloc[MAXFILES+1];  /* offsets into constructed trailer pg pipelines
                                 where pioout pipe should be inserted */
struct attr_info *attr_tab_p;
struct attr_info *attr_tab_p2;  /* for SET_FLAG() and RESET_FLAG() macros */
struct stat statbuf;            /* used with stat system call */

char  *env_pioascii;            /* ptr to TPF attribute buffer */
char  env_qdate[50];
char  env_to[100];
char  env_devname[50];
char  env_title[S_TITLE+10];
char  env_qname[100];
char  env_qdname[100];
char  env_from[100];
char  env_mail_only[20];
char  env_ptrtype[50];
char  env_jobnum[50];
char  env_pipetype[20];         /* 0=print file, 1=header, 2=trailer */
char  env_shrfildes[20];        /* file descriptor of shared file */
char  env_interface[20];        /* 0=not serial or parallel
                                   1=parallel printer device driver
                                   2=serial printer device driver
                                   3=tty (serial) */
char  env_basedir[300] = "PIOBASEDIR="; 
                                /* base directory where piobe and the
                                   pio directory reside */
char  env_basevardir[300] = "PIOVARDIR="; 
                                /* base directory where piobe and the
                                   pio directory reside */
char Bflag[5] = "-Bxx";
char *qdate;
char *to;
char *devname;
char *title;
char *qname;
char *qdname;
char *from;
char *enqflags;
int   mail_only;
int   jobnum;

char *flaginfo;         /* ptr to area containing flags info (ASCII) */
char *flagp;            /* current location in flaginfo during construction */
char    *tempdirp;
char odm_fullpath[BUFSIZ];	/* full path name of odm file */
char	*mem_start;		/* start address of odm data memory */
char	*hash_table;		/* address of primary hash table */
char	*odm_table;		/* address of odm data memory */
char    *usedbypiobe[] = {PREVIEWLEVEL, DIAGLEVEL, IN_DATASTREAM, FILTERNAME,
			  COPIES, PRINTQUEUE, BURSTPAGE, MAILONLY,
                          "_H", "_X"};
extern int errno;
extern char piopipeline;            /* set by pioparm() when it finds %ix */
extern int piofilterloc;            /* set by pioparm() when it finds %p */
extern int piodevdriloc;            /* set by pioparm() when it finds %z */
extern short usedbysomeone;
extern unsigned char shellchar[];

extern char *get_qdate();
extern char *get_to();
extern char *get_from();
extern char *get_title();
extern char *get_queue_name();
extern char *get_device_name();
extern char *getenv();
extern char *getmsg();
extern char get_cmd_line();
extern char		*piovalav(const char *attrnm);

extern int optind;         /* argv index for next token */

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);

char *getdocpipe();
char *getpipe();


static struct lpr232 plp232;
static struct lprmod plpmod;
static int *dptr;               /* dummy pointer for piogetvals to store into*/

/* attrparm parameter to pass to piogetvals() */
static struct attrparms attrval[] = {
INTERFACETYPE, VAR_INT, NULL, (union dtypes *) &dptr,
NULL,                0, NULL, NULL
};

/*
*******************************************************************************
*******************************************************************************
** NAME:	main()
**
** DESCRIPTION:	main() initializes the log_file and other "housekeeping"
**		functions and controls the routines to be called.  Return codes
**		from other functions are used to determine pipe status and
**		appropriate action.
**
** ROUTINES
**   CALLED:	config_obe() - sets up attribute parameter table
**
**		piobe() - controls the actual printing of jobs
**
** PARAMETERS:	argc - number of arguments passed, including the name
**
**		argv - pointer to the argument vector
**
*******************************************************************************
*******************************************************************************
*/

main(argc,argv)
unsigned argc;
char **argv;
{
    long	clock;
    struct tm	*tdate;
    char	pdate[50];
    char       *prtfile;
    char       *bufptr;
    char       *msgptr;
    char       *wkptr;
    char       *pipeline;
    char       *cp;  
    int		tlen;
    int         copynum;
    int         filenum;
    int         cnt, val, rc;
    int	        val_Z;    /* integer value of _Z in database */
    int         save_piomode; /* temporary variable */
	void ascterm_prtr_setup();
	


    setlocale(LC_ALL, "");

    /* initialize the status file interface */
    if (!get_backend() || log_init() == -1)
	ERREXIT(0, MSG_PIOBECMD, NULL, NULL, 0);

    qdate     = get_qdate();
    to        = get_to();
    title     = get_title();
    qname     = get_queue_name();
    qdname    = get_device_name();
    from      = get_from();
    mail_only = get_mail_only();
    jobnum    = get_job_number();

    (void) sprintf(env_qdate    , "PIOQDATE=%s",     qdate     );
    (void) sprintf(env_to       , "PIOTO=%s",        to        );
    (void) sprintf(env_title    , "PIOTITLE=%s",     title     );
    (void) sprintf(env_qname    , "PIOQNAME=%s",     qname     );
    (void) sprintf(env_qdname   , "PIOQDNAME=%s",    qdname    );
    (void) sprintf(env_from     , "PIOFROM=%s",      from      );
    (void) sprintf(env_mail_only, "PIOMAILONLY=%d",  mail_only );
    (void) sprintf(env_jobnum   , "PIOJOBNUM=%d",    jobnum    );
    (void) sprintf(env_pipetype,  "PIOPIPETYPE= ");

    (void) putenv("PIOSTATUSFILE=3");
    (void) putenv(env_qdate    );
    (void) putenv(env_to       );
    (void) putenv(env_title    );
    (void) putenv(env_qname    );
    (void) putenv(env_qdname   );
    (void) putenv(env_from     );
    (void) putenv(env_mail_only);
    (void) putenv(env_pipetype );
    (void) putenv(env_jobnum   );

    /* Set Up the Base Directory (the base directory is assumed to be   */
    /* the "pio" subdirectory of the directory from which piobe was run */
    if ((cp = strrchr(argv[0], '/')) != NULL) {
        (void) strncat(env_basedir, argv[0], cp - argv[0]);
        strcat(env_basedir, "/pio");
    } else
        (void) strcat(env_basedir, DEFBASEDIR);   /* safety valve */
    (void) putenv(env_basedir);  
    basedir = env_basedir + sizeof("PIOBASEDIR=") - 1;

    /* Set Up the Base Variable Directory (the base variable directory */
    /* defaults to /var/spool/lpd/pio/@local )   */
    if (strcmp(basedir,DEFBASEDIR) == 0 ||
	strcmp(basedir,DEFBASEDIR_31) == 0)
    					/* piobe is running in default env. */
       {
       (void) strcat(env_basevardir, DEFVARDIR);  /* vardir is /var/spool   */ 
       (void) putenv(env_basevardir);  
       basevardir = env_basevardir + sizeof("PIOVARDIR=") - 1;
       }
    else  /* piobe is not running in default environment */
       {
       (void) strcat(env_basevardir, basedir); /* make PIOVARDIR same as PIOBASEDIR */
       (void) putenv(env_basevardir);  
       basevardir = basedir;
       } 

    /* set up info. in case we have to generate an error message */
    (void) strncpy(errinfo.title, title, sizeof(errinfo.title) - 1);
    (void) strncpy(errinfo.progname, "piobe", sizeof(errinfo.progname) - 1);

    /* Message ipc initialization */
    pmi_initmsgipc();

    /* Get Ready For Signal To Cancel the Print Job */
    (void) signal(SIGTERM, terminate); /* want to catch the terminate signal */

    /* Get Info From Status File */
    header = get_header();      /* NEVER, ALWAYS (each file), or GROUP (job) */

    if (header != NEVER && header != ALWAYS && header != GROUP)
	header = NEVER;
    trailer = get_trailer();    /* NEVER, ALWAYS (each file), or GROUP (job) */
    if (trailer != NEVER && trailer != ALWAYS && trailer != GROUP)
	trailer = NEVER;
    align = get_align() && get_was_idle();  /* TRUE or FALSE */
    feed = get_feed();          /* TRUE or FALSE */
    numcopies = get_copies();   /* number of copies */
#ifndef PIOTEST
    enqflags = get_cmd_line();
#endif

    /* Get the Time */
    clock = time((long *) 0);
    tdate = localtime(&clock);
    tlen =  350;
    (void) strftime(pdate,tlen,"%a %h %d %T %Y",tdate);

    /* Call the Various Functions */
    (void) config_obe(argc, argv);

    /* Now OK to Use get_str() */

	(void) ascterm_prtr_setup();

    strinfop = get_str(DEVICENAME, MSG_ATTRFAIL);  /* data base value */
    devname = strinfop->ptr;
    (void) sprintf(env_devname , "PIODEVNAME=%s",  devname);
    (void) putenv(env_devname );
    strinfop = get_str(PRINTERTYPE, MSG_ATTRFAIL);    /* data base value */
    (void) sprintf(env_ptrtype, "PIOPTRTYPE=%s", strinfop->ptr);
    (void) putenv(env_ptrtype);

    /* More Initialization */
    (void) pio_info(argv);

    /* Page Alignment Processing */
    if (align)
    {
	    if ((cnt = DEF_NUM_FF_STRS) != 1)
	    {
		    (void) strcpy(formfeedsbuf, "1"); /* we only want one */
		    SET_FLAG(NUM_FF_STRS, formfeedsbuf, 1);
	    }
	    fp = freopen("/dev/null", "r", stdin);
	    if (fp != NULL)
	    {
		    pipeline = getdocpipe(0, NULL, 3);
		    (void) run_pipeline(pipeline, MSG_ALIGN, NULL);
	    }
	    RESET_FLAG(NUM_FF_STRS);
    }

    /* If No Files To Print (i.e., "feed" pages only), No Copies Either */
    if (numfiles == 0)
	numcopies = 0;

    /***************************************************************************/
    /* The following block of code was added to support a change in pioout
       that sends a form feed string at the end of a canceled job.  The code
       determines whether the FF_STRING flag passed to pioout should
       be set to null or left alone. 

       If _Z = '1' in the database or on the command line, FF_STRING is left
       alone and is passed to pioout.  If the database value of _Z = '1' 
       and the user overrides the _Z attribute on the command line with a value 
       of '0', the override is ignored.

       If _Z = '0' in the database and is not overriden with _Z = '1' on the
       command line, FF_STRING is set to null.
    /***************************************************************************/
    attr_tab_p = get_attrtab_ent(FF_AT_END);
    if (attr_tab_p != NULL)  /* if its in the data base */
    {
        strinfop = get_str(FF_AT_END, MSG_ATTRFAIL);  /* get value for _Z*/
        val_Z = atoi(strinfop->ptr);   /* convert it to integer 0 or 1 */
        if (val_Z == 0) /* if its '0' then check if database is '0' */
        {
           save_piomode = piomode;   /* save away piomode */
           piomode = PIO_DBASE_MODE; /* set database mode */
           strinfop = get_str(FF_AT_END, MSG_ATTRFAIL);  /* data base value for _Z*/
           val_Z = atoi(strinfop->ptr);   /* convert it to integer 0 or 1 */
           piomode = save_piomode;  /* restore piomode */
        }
    }
    if ((attr_tab_p == NULL) || (val_Z == 0))  /* if _Z isn't in database */
       SET_FLAG(FF_STRING, "", 0);             /* or the database has a   */
                                               /* value of '0' for _Z     */ 


    /* Loop To Print Each Copy */
    for (copynum = 1; copynum <= numcopies; copynum++) {
	for (filenum = 1; filenum <= numfiles; filenum++) {

	    /* Build Header Page (if requested) */
	    if (header == ALWAYS || (header == GROUP && filenum == 1))
	    {   /* Need a Header Page */
		pipeline = getpipe(filenum, argv, 2, HDRPIPE);
		if (copynum == 1 && filenum == 1)
		{   /* Header page for first print file is created in a      */
                    /* temporary file instead of being sent directly to the  */
                    /* printer in case the first print file fails because of */
                    /* a bad flag value.  So, make pioout in pipeline for    */
                    /* first print file prefix the temporary file to the     */
                    /* print file.                                           */
		    SET_FLAG(PREFIXFILE, hdrfile, strlen(hdrfile));
                    /* Force Rebuild of Header Pipeline Next Time         */
                    /* (Rest of Header Pages Will Go Directly to Printer) */
                    RESET_FLAG(OUTFILE);
		    hdrpipe[1] = NULL;
                }
		/* Run Pipeline to Build Header Page and Send the Page
		   to Either a Temporary File or to the Printer */
		*(env_pipetype + sizeof("PIOPIPETYPE=") - 1) = '1';
		rc = run_pipeline(pipeline, MSG_HDRPIPE, NULL);
		if (sigterm || rc != PIOO_GOOD)
		    goto SKIPFILE;
	    }

	    /* Open the Print File */
	    prtfile = argv[optind + filenum - 1];
	    if ((prt_fp = freopen(prtfile, "r", stdin)) == NULL)
		ERREXIT(0, MSG_FOPEN3, prtfile, NULL, errno);

            /* Set Up "bytes already done" for pioout */
	    cnt = sprintf(bytesdonebuf, "%ld", bytes_done);
	    SET_FLAG(BYTESDONE, bytesdonebuf, cnt);

	    /* Run the Document Pipeline to Print the File */
	    pipeline = getdocpipe(filenum, argv, 2);
	    RESET_FLAG(PREFIXFILE);
	    *(env_pipetype + sizeof("PIOPIPETYPE=") - 1) = '0';
	    rc = run_pipeline(pipeline, MSG_DOCPIPE, prtfile);

	SKIPFILE:
            /* Check for Printer Failure */
	    if (rc == PIOO_FAIL || rc == PIOO_USR1QUIT || rc == PIOO_TERMQUIT)
		goto QUIT;

            /* Check for Print Job Cancel or Failure */
	    if (sigterm || rc != PIOO_GOOD)
	    {   /* Print Job Cancel or Failure, But Printer is Okay */
		if (trailer != NEVER)
		{   /* trailer: ALWAYS or GROUP */
		    *(env_pipetype + sizeof("PIOPIPETYPE=") - 1) = '2';
		    pipeline = getpipe(filenum, argv, 2, TRLPIPE);
		    (void) run_pipeline(pipeline, MSG_TRLCLEANUP, NULL);
		}
		goto THATSALL;
	    }

	    /* File Printed Successfully */
	    if (trailer == ALWAYS || (trailer == GROUP && filenum == numfiles))
	    {
		*(env_pipetype + sizeof("PIOPIPETYPE=") - 1) = '2';
		pipeline = getpipe(filenum, argv, 2, TRLPIPE);
		rc = run_pipeline(pipeline, MSG_TRLPIPE, NULL);
		if (rc==PIOO_FAIL || rc==PIOO_USR1QUIT || rc==PIOO_TERMQUIT)
		    goto QUIT;
		if (sigterm || rc != PIOO_GOOD)
		    goto THATSALL;
	    }
	    if (sigterm)
		goto THATSALL;
	    bytes_done += filesize[filenum];
	}
    }

THATSALL:
    /* "feed" Processing When Printer Goes Idle (no files to print) */
    if (feed)
    {

	    /* Check FF Cmd String To See If Different From pioout's Default */
	    attr_tab_p = get_attrtab_ent(DB_FORMFEED);
	    if (attr_tab_p != NULL)  /* if its in the data base */
	    {
		    strinfop = get_str(DB_FORMFEED, MSG_ATTRFAIL); /*DB value*/
		    RESET_FLAG(FF_STRING);
		    if (strinfop->len != DEFAULT_FF_LEN ||
			  memcmp(strinfop->ptr, DEFAULT_FF_STR, DEFAULT_FF_LEN))
		    {
			/* Diff From Default, So Fix Up "@" Attr. For pioout */
			wkptr = str2ascii(strinfop->ptr, strinfop->len);
			SET_FLAG(FF_STRING, wkptr, strlen(wkptr));
		    }
	    }

	    if (feed != DEF_NUM_FF_STRS)
	    {
		    cnt = sprintf(formfeedsbuf, "%d", feed);
		    SET_FLAG(NUM_FF_STRS, formfeedsbuf, cnt);
	    }
	    fp = freopen("/dev/null", "r", stdin);
	    if (fp != NULL)
	    {
		    pipeline = getdocpipe(0, NULL, 3);
		    (void) run_pipeline(pipeline, MSG_FEED, NULL);
	    }
	    RESET_FLAG(NUM_FF_STRS);
    }


QUIT:
    if (diagnostic >= 2) {
	fflush(diag_fp);
	rewind(diag_fp);
	if (fstat(2, &statbuf) < 0)
	    ERREXIT(0, MSG_STAT2, diagfile, NULL, errno);
	MALLOC(bufptr, (int) statbuf.st_size + 6000);
	wkptr = bufptr;
	while ((val = getc(diag_fp)) != EOF)
	    *wkptr++ = (char) val;
	msgptr = getmsg(MF_PIOBE, 1, MSG_FLAGVALSHDR);
	while (*msgptr)
	    *wkptr++ = *msgptr++;
	for (flagp = flaginfo; *flagp;)
	    *wkptr++ = *flagp++;
	*wkptr++ = '\n';
	*wkptr++ = '\n';
	*wkptr = '\0';
	piomsgout(bufptr);
    }

    if (header != NEVER)
        (void) unlink(hdrfile);

    if (sigterm || rc == PIOO_TERMOK || rc == PIOO_TERMQUIT)
	exit(EXITSIGNAL);
    else if (rc == PIOO_FAIL)
	exit(EXITFATAL);
/* Added for A12747 */
    if ((unsigned char)rc == EXITIO)
	exit(EXITIO);
    exit(EXITOK);
    return(0);  /* just to keep lint happy */
}


/*
*******************************************************************************
*******************************************************************************
** NAME:	config_obe()
**
** DESCRIPTION:	This routine is responsible for obtaing ODM information from
**		the specified ODM file and managing the information conversion.
**		Once the specified ODM file has been found and opened, memory
**		is allocated to read the file into.  Information concerning
**		the size and location of the the ODM memory space and an
**		attribute conversion table are passed to piogetvals().  The
**		results of this initial conversion are used by the routine
**		piogetopt().  If everthing has gone well a "good" exit code
**		is returned.  If not a "bad" exit code is returned and the
**		specific error handling is accomplished in main().
**
** CALLING
** SEQUENCE:	config_obe(ac,av) 	ac - number of arguments in list
**					av - pointer to arguments in list
**
** EXTERNAL
** FUNCTIONS:	piogetvals(attr_parms, MAXSTRLEN)
**		piogetopt(ac, av, optlist)
**
** GLOBAL
** VARIABLES:	odm_fullpath - full path name to ODM file
**		mem_start - start of the memory allocated for ODM information
**		odm_table - address of odm table memory
**		hash_table - address of primary hash table
**
*******************************************************************************
*******************************************************************************
*/
int config_obe(ac, av)

unsigned	ac;
char		**av;
{
    char       *wkptr;
    char       *buf_ps;
    char       *pi0, *pi1, *p0, *p1, *p2, *p3;
    char        datastream_db;
    char        attrname[3];
    int         ilen, plen;
    int         fraglen, check_for_ps, rc;
    int		namelen;		/* length of file name found */
    int		fd = 0;			/* file descriptor */
    int         cnt, val;
    DIR		*dirp;
    struct dirent *dp;		/* points to particular directory entry */
    struct str_info *strinfop_ps1;
    struct str_info *strinfop_ps2;

    /*
    ** First must try to find the odm data file for this queue and device.
    ** The name is of the form:
    **			*.*.*.quename:devname
    ** The que and device names can be obtained with qdaemon calls.  Once
    ** accomplished a partial name can be constructed.
    */

    (void) strncpy(errinfo.qname ,  qname, sizeof(errinfo.qname)  - 1);
    (void) strncpy(errinfo.qdname, qdname, sizeof(errinfo.qdname) - 1);

    (void) strcpy(fragname,".");
    (void) strcat(fragname,qname);
    (void) strcat(fragname,":");
    (void) strcat(fragname, qdname);

    fraglen = strlen(fragname);

    (void) strcpy(odm_fullpath,basevardir);
    (void) strcat(odm_fullpath, "/ddi/");

    if((dirp = opendir(odm_fullpath)) == NULL)
	ERREXIT(0, MSG_DIRFAIL, odm_fullpath, NULL, 0);

    /*
    ** Know that the full name containing the partial name created is unique.
    ** Therefore, only the end portion of the full path name must match the
    ** partial name we have created.  So when doing a string compare, can
    ** increment the pointer past the first part of the name and compare the
    ** end portion.
    */

    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	namelen = strlen(dp->d_name);
	if (!strcmp(dp->d_name + (namelen - fraglen), fragname))
	{
	    break;                     /* we found it */
	}
    }

    /* See if we found a data base file */
    if (dp == NULL)
	ERREXIT(0, MSG_DBASENAME2, qname, qdname, 0);

    (void) strcat(odm_fullpath, dp->d_name);
    closedir(dirp);

    /*
    ** Open and read the data base file
    */

    (void) attrtab_init(odm_fullpath);

    /* Set Up @4 Attribute Value (path for base (i.e., "pio") directory) */
    attr_tab_p = get_attrtab_ent(BASE_DIR);
    attr_tab_p->ptypes.sinfo->ptr = basedir;
    attr_tab_p->ptypes.sinfo->len = strlen(basedir);

    /* Set Up @5 Attribute Value (path for var (i.e., "pio") directory) */
    attr_tab_p = get_attrtab_ent(BASE_VARDIR);
    attr_tab_p->ptypes.sinfo->ptr = basevardir;
    attr_tab_p->ptypes.sinfo->len = strlen(basevardir);

    /* Fix it so piogetopt won't turn on USEDBYSOMEONE flag */
    usedbysomeone = 0;

    /*
    ** Process command line flags
    */

    (void) piogetopt(ac, av, optlist, NULL);

    /* piogetopt is done, so put things back like they were */
    usedbysomeone = (short) USEDBYSOMEONE;

    /* determine the type of device interface and save it in     */
    /* attribute @3, which can be referenced by other attributes */
    val = 0;				/* initial assumption: unknown type */
    if (ioctl(1, LPRMODG, &plpmod) == 0)/* if printer device driver */
    {
        if (ioctl(1, LPRGETA, &plp232) < 0) /* if parallel printer dev. dri.*/ 
            val = 1;   	                        /* indicate parallel I/F */
        else                                /* otherwise */
            val = 2;			        /* indicate serial I/F */
    }
    else                                /* otherwise NOT printer dev. driver*/
        if (isatty(1))                       /* if is a tty */
            val = 3;			         /* indicate tty I/F */
    attr_tab_p = get_attrtab_ent(INTERFACETYPE);
    if (attr_tab_p != NULL)		/* safety valve */
    {
      (void) piogetvals(attrval, NULL);     /* set up variable integer attr. */
      for (cnt = 0; cnt < PIO_NUM_MODES; cnt++) /* initialize integer values */
        (attr_tab_p->ptypes.ivals)->value[cnt] = val;
    }
    (void) sprintf(env_interface, "PIOINTERFACE=%d", val);
    (void) putenv(env_interface);       /* so pioformat.c can find it */

    /* Set Up Flag For # of Copies So It Looks Like We Got It From Cmd Line */
    (void) sprintf(copies_string, "%d", numcopies);
    attr_tab_p = get_attrtab_ent(COPIES);
    attr_tab_p->ptypes.sinfo->ptr = copies_string;
    attr_tab_p->ptypes.sinfo->len = strlen(copies_string);
    if (numcopies > 1)
	attr_tab_p->flags |= ONCMDLINE;  /* user must have overridden it */

    /* Set Up Flag For Print Queue and Queue Device Names */
    /* So It Looks Like We Got It From the Command Line   */
    attr_tab_p = get_attrtab_ent(PRINTQUEUE);
    attr_tab_p->ptypes.sinfo->ptr = fragname + 1; /* skip over '.' */
    attr_tab_p->ptypes.sinfo->len = strlen(fragname + 1);

    /* Set Up Flag For Burst Page Options */
    if (header == NEVER)
	burst_string[0] = 'n';
    else if (header == ALWAYS)
	burst_string[0] = 'a';
    else if (header == GROUP)
	burst_string[0] = 'g';
    if (trailer == NEVER)
	burst_string[1] = 'n';
    else if (trailer == ALWAYS)
	burst_string[1] = 'a';
    else if (trailer == GROUP)
	burst_string[1] = 'g';
    attr_tab_p = get_attrtab_ent(BURSTPAGE);
    attr_tab_p->ptypes.sinfo->ptr = burst_string;
    attr_tab_p->ptypes.sinfo->len = 2;
    (void) strncpy(Bflag + 2, burst_string, 2);
    if (strstr(enqflags, Bflag))
        attr_tab_p->flags |= ONCMDLINE;

    /* Set Up Flag For Mail-Only Option */
    if (mail_only)
	mail_string[0] = '+';
    else
	mail_string[0] = '!';
    attr_tab_p = get_attrtab_ent(MAILONLY);
    attr_tab_p->ptypes.sinfo->ptr = mail_string;
    attr_tab_p->ptypes.sinfo->len = 1;
    if (mail_only)
	attr_tab_p->flags |= ONCMDLINE;  /* user must have overridden it */

    /* Check For Command Line Flags Prohibited For All Data Streams */
    (void) chkprohib(PROHIBITEDFLGS);

    /* Check For Command Line Flags Prohibited For The Input Data Stream */
    strinfop = get_str(IN_DATASTREAM, MSG_ATTRFAIL);
    datastream_db = strinfop->ptr[0];
    (void) strcpy(attrname, PROHIBITCHAR);
    attrname[1] = datastream_db;
    (void) chkprohib(attrname);

    /* Check Form Feed Cmd String To See If Different From pioout's Default */
    attr_tab_p = get_attrtab_ent(DB_FORMFEED);
    if (attr_tab_p != NULL)  /* if its in the data base */
    {
	strinfop = get_str(DB_FORMFEED, MSG_ATTRFAIL);  /* data base value */
	if (strinfop->len != DEFAULT_FF_LEN ||
	      memcmp(strinfop->ptr, DEFAULT_FF_STR, DEFAULT_FF_LEN))
	{
	    /* Different From Default, So Fix Up "@" Attr. For pioout */
	    wkptr = str2ascii(strinfop->ptr, strinfop->len);
	    SET_FLAG(FF_STRING, wkptr, strlen(wkptr));
	}
    }

    /* Check Cancel Command String To See If Different From pioout's Default */
    strinfop = get_str(DB_CANCELSTRING, MSG_ATTRFAIL);  /* data base value */
    if (strinfop->len != DEFAULT_CAN_LEN ||
	  memcmp(strinfop->ptr, DEFAULT_CAN_STR, DEFAULT_CAN_LEN))
    {
	/* Different From Default, So Fix Up "@" Attr. For pioout's Flag Arg. */
	wkptr = str2ascii(strinfop->ptr, strinfop->len);
	SET_FLAG(CANCELSTRING, wkptr, strlen(wkptr));
    }

    /* See if # Times To Send Cancel Str. Is Different From pioout's Default*/
    strinfop = get_str(DB_NUM_CAN_STRS, MSG_ATTRFAIL);  /* data base value */
    if (atoi(strinfop->ptr) != DEF_NUM_CAN_STRS)
	/* Different From Default, So Fix Up "@" Attr. For pioout's Flag Arg. */
	SET_FLAG(NUM_CAN_STRS, strinfop->ptr, strinfop->len);

    /* Check For Optional Name of Read Printer Routine */
    strinfop = get_str(DB_READROUTINE, MSG_ATTRFAIL);  /* data base value */
    if (strinfop->len != 0)
	/* Different From Default, So Fix Up "@" Attr. For pioout's Flag Arg. */
	SET_FLAG(READROUTINE, strinfop->ptr, strinfop->len);

    /* Check For Optional Name of User To Send Intervention Req'd Messages To */
    strinfop = get_str(DB_INT_REQ_USER, MSG_ATTRFAIL);  /* data base value */
    if (strinfop->len != 0)
	/* Different From Default, So Fix Up "@" Attr. For pioout's Flag Arg. */
	SET_FLAG(INT_REQ_USER, strinfop->ptr, strinfop->len);

    /* Count Print Files, Get Size of Each File, Get Input Data Stream Type */
    check_for_ps = FALSE;     /* initial assumption */
    attr_tab_p = get_attrtab_ent(PS_STRING);
    if (attr_tab_p != NULL)    /* if it's in the data base */
    {
	strinfop_ps1 = get_str(PS_STRING, MSG_ATTRFAIL); /* PostScript string*/
	strinfop_ps2 = get_str(PS_DS_NAME, MSG_ATTRFAIL); /* pipeline ID */
	attr_tab_p = get_attrtab_ent(IN_DATASTREAM);
	if (strinfop_ps1->len > 0 && strinfop_ps2->len > 0
	      && !(attr_tab_p->flags & ONCMDLINE))
	{
	    check_for_ps = TRUE;      /* check for PostScript string */
	    MALLOC(buf_ps, strinfop_ps1->len);
	}
    }
    datastream[1] = datastream_db;   /* in case there are no files */
    for (cnt = 0; (wkptr = av[optind + cnt]) != NULL; cnt++) 
    {
	if (++numfiles > MAXFILES)
	    ERREXIT(0, MSG_MAXFILES, NULL, NULL, MAXFILES);
	datastream[numfiles] = datastream_db;
	if (check_for_ps) 
        {
	    if ((fd = open(wkptr, O_RDONLY)) < 0)
		ERREXIT(0, MSG_FOPEN3, wkptr, NULL, errno);
	    pi0 = pi1 = strinfop_ps2->ptr;
	    ilen = strinfop_ps2->len;
	    plen=strinfop_ps1->len;
	    for (p0=p1=p2 = strinfop_ps1->ptr; (p2-p0) <= plen; p2++) 
            {
		/* Check For Protected Comma; i.e., "\," */
		if ((p2-p0) < plen && *p2 == ',' && p2>p0 && *(p2-1) =='\\')
		    for (p3=p2-1, plen--; p3 < (p0+plen); p3++)
			*p3 = *(p3 + 1);
		/* If End of Compare String, Compare String With the File */
		if ((p2-p0) == plen || *p2 == ',') 
                {
		    if ((p2-p1) > 0 && *pi1 && *pi1 != ',')
                    {
			if (!strncmp(p1,DEFDSDIRECTIVE,p2-p1) &&
			    pi1-pi0 < ilen)
			{
			   datastream[numfiles] = *pi1;
			   break;
			}
			(void) lseek(fd, (off_t) 0, SEEK_SET);
			if ((rc = read(fd, buf_ps, p2-p1)) < 0)
			    ERREXIT(0, MSG_READ3, wkptr, NULL, errno);
			if (rc == (p2-p1) && memcmp(buf_ps, p1, p2-p1) == 0
			      && (pi1-pi0) < ilen) 
                        {
			    datastream[numfiles] = *pi1;
			    break;
			}
		    }
		    p1 = p2 + 1;
		    pi1 += *pi1?*pi1 == ','?1:2:0;
		}
	    }
	    if (fstat(fd, &statbuf) < 0)
		ERREXIT(0, MSG_STAT, wkptr, NULL, errno);
	    (void) close(fd);
	}
	else
	    if (stat(wkptr, &statbuf) < 0)
		ERREXIT(0, MSG_STAT, wkptr, NULL, errno);
	bytes_total += (long) statbuf.st_size;
	filesize[numfiles] = (long) statbuf.st_size;
    }
    bytes_total *= numcopies;


/*
*/

    return(0);
}


/*
*******************************************************************************
** NAME:	pio_info()
**
** DESCRIPTION: pio_info() will create files to collect preview or diag-
**              nostic information.  Diagnostic and preview requirements
**              are mutually exclusive and preview requests will take
**              precedence.  If there are any problems at all, an exit
**              is performed from here
**
*******************************************************************************
*/

pio_info(av)
char	**av;
{

    char        flagbuf[2000];
    char        attrname[3];
    char	*premsg;
    char        *previewbuf;
    char        *wkptr;
    char        *msgptr;
    char        *flag_val;

    int         flagchar;
    int		szused;
    int		hchcnt;
    int		hdrwidth;
    int		flag_len;
    int		i,j;
    int         cnt;
    int         fildes, fdes;

    extern	struct attr_info *get_attrtab_ent();
    extern	struct str_info *get_str();
    extern	FILE *fopen();


    /*
    ** Want to check for a preview request.  If present then it will
    ** take precedence over any diagnostic requests.  In addition, the file
    ** will not be printed.
    */

    strinfop = get_str(PREVIEWLEVEL, MSG_ATTRFAIL);
    preview = atoi(strinfop->ptr);		/* convert string */

    /*
    ** The preview flag can have only two values 0 and 1, anything else
    ** is an error and we will exit.
    */

    if(preview < 0 || preview > 1)
	ERREXIT(0, MSG_PREVIEW, NULL, NULL, preview);

    /*
    ** Next check the diagnostic level requested.  However, if the preview
    ** level is non-zero then the diagnostic level will be set to zero.
    */

    if (preview == 0)
    {
	strinfop = get_str(DIAGLEVEL, MSG_ATTRFAIL);
	diagnostic = atoi(strinfop->ptr);
        if (diagnostic < 0 || diagnostic > 3)
	    ERREXIT(0, MSG_DIAGLEVEL, NULL, NULL, 0);
    }
    else
    {
	diagnostic =  0;
    }

    /*
    ** Now have enough information to decide what type and how many temporary
    ** files are needed.  Now get the directory name for temporary files.
    */

    strinfop = get_str(TEMPDIR, MSG_ATTRFAIL);
    tempdirp = strinfop->ptr;

    /*
    ** Create a temporary file for each purpose needed
    */

    /* File for pioformat to keep cumulative page count for billing */
    (void) strncpy(shrfile, tempdirp, 94);
    (void) strcat(shrfile, "/shared");
    (void) strncat(shrfile, fragname, 50);

    (void) unlink(shrfile);
    (void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP));
    shr_fd = open(shrfile, O_CREAT + O_RDWR + O_TRUNC, 0660);
    if (shr_fd < 0)
	ERREXIT(0, MSG_FOPEN, shrfile, NULL, errno);
    (void) unlink(shrfile);  /* so it will go away when we're done */
    (void) write(shr_fd, (char *)&zeroval, sizeof(long));/*initial page count*/
    (void) lseek(shr_fd, (off_t) 0, SEEK_SET);
    (void) sprintf(env_shrfildes, "PIOSHRFILDES=%d", shr_fd);
    (void) putenv(env_shrfildes);

    /* File for diagnostic information */
    if (diagnostic > 0)
    {
	(void) strncpy(diagfile, tempdirp, 94);
	(void) strcat(diagfile, "/piodiag");
	(void) strncat(diagfile, fragname, 50);

	(void) unlink(diagfile);
	(void) close(2);
	(void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP));
	fildes = open(diagfile, O_CREAT + O_RDWR + O_TRUNC, 0660);
	if (fildes < 0)
	    ERREXIT(0, MSG_FOPEN, diagfile, NULL, errno);
	diag_fp = fdopen(fildes, "w+");
	(void) unlink(diagfile); /* so it will go away when we're done */

	wkptr = getmsg(MF_PIOBE, 1, MSG_DIAGHDR);
	while (*wkptr != (char *) NULL)
	    pioputc(*wkptr++, diag_fp);
	fflush(diag_fp);
	gen_info = TRUE;
    }

    /* See if need list of flag values  because of header page or preview */
    if (header != NEVER || preview == 1)
        gen_info = TRUE;

    /* Get ready to build flag values */
    if (gen_info)
    {
	MALLOC(flaginfo, 5000);
	(void) strcpy(flaginfo, "PIOFLAGS=");
	flagp = flaginfo + sizeof("PIOFLAGS=") - 1;
        flaginfo = flagp;

	strinfop = get_str(ATTR_WIDTH, MSG_ATTRFAIL);
	hdrwidth = atoi(strinfop->ptr);
	if (hdrwidth < 0)    /* safety valve */
	    hdrwidth = 0;
    }

    /* Set Up pioout Flag for Total # of Bytes To Be Printed */
    cnt = sprintf(totalbytesbuf, "%ld", bytes_total);
    SET_FLAG(TOTALBYTES, totalbytesbuf, cnt);

    /* Construct document pipelines so that any references */
    /* to command line flags will be noted                 */
    for (cnt = 1; cnt <= numfiles; cnt++)
        (void) getdocpipe(cnt, av, 1);

    /* Construct trailer pipeline so any references by the */
    /* pipeline to command line flags will be noted.       */
    if (trailer != NEVER)
        (void) getpipe(1, av, 1, TRLPIPE);

    /* Construct header pipeline so any references by the       */
    /* pipeline to command line flags will be noted.  The first */
    /* header page gets special treatment because if the first  */
    /* print file fails because of bad flag value, the header   */
    /* page should not be printed.  So, the header page will be */
    /* put in a temporary file and pioout will later be told    */
    /* to prefix the temporary file to the first print file.    */
    if (header != NEVER)
    {
        (void) strncpy(hdrfile, tempdirp, 94);
        (void) strcat(hdrfile, "/piohdr");
        (void) strncat(hdrfile, fragname, 50);
        SET_FLAG(OUTFILE, hdrfile, strlen(hdrfile));
        (void) getpipe(1, av, 1, HDRPIPE);
    }

    /*
    ** Update attr_info struct to indicate those arguments that are used by
    ** piobe.  usedbypiobe is defined by:
    **          char *usedbypiobe[] = {"_a","_A"......etc}
    */

    szused = (sizeof(usedbypiobe)/sizeof(char *));
    for (i=0; i < szused; i++)
    {
	if ((attr_tab_p = get_attrtab_ent(usedbypiobe[i])) != NULL)
	    attr_tab_p->flags |= USEDBYPIOBE;
    }

    /*
    ** Check to make sure that there are no unexpected flags
    ** in the command line.  If there are, something is wrong somewhere so
    ** issue error messsage and return.
    */

    hchcnt = 0;    /* start list of flag values at beginning of line */

    (void) strcpy(attrname, FLAGCHAR);
    for (flagchar = 'a'; flagchar != ('9' + 1); flagchar++)
    {
	/* Skip over undefined flag letters */
	if (flagchar == ('z' + 1))
	    flagchar = 'A';
	else if (flagchar == ('Z' + 1))
	    flagchar = '0';

	/* Build attribute name for flag and get pointer to its info. */
	*(attrname + 1) = (char) flagchar;
	if ((attr_tab_p = get_attrtab_ent(attrname)) == NULL)
	    continue;  /* flag not defined in data base, so ignore it */
		       /* if flag is specified on cmd line, piogetopt() */
		       /* will detect it. */

	/* Determine if flag on command line is expected by filter or piobe */
	if (!(attr_tab_p->flags & (USEDBYSOMEONE + USEDBYPIOBE)))
	{
	    if (attr_tab_p->flags & ONCMDLINE)
		ERREXIT(0, MSG_BADFLAG, attrname + 1, NULL, 0)
	    continue;  /* not on command line & nobody is expecting it */
	}

	/* Validate the flag if specified on the command line. */
	if (attr_tab_p->flags & ONCMDLINE)
	   if (msgptr = piovalav((const char *)attrname))
	   {
	      ERREXIT(0,MSG_DUMMY,msgptr,(char *)NULL,0);
	   }

	/* Get flag value (in ASCII) and its length */
	if (attr_tab_p->datatype == CONST_STR)
	{
	    flag_val = attr_tab_p->ptypes.sinfo->ptr;
	    flag_len = attr_tab_p->ptypes.sinfo->len;
	}
	else if (attr_tab_p->datatype == VAR_STR)
	{
	    flag_val = (attr_tab_p->ptypes.sinfo + piomode)->ptr;
	    flag_len = (attr_tab_p->ptypes.sinfo + piomode)->len;
	}
	else  /* data type is VAR_INT */
	{   (void)
	    sprintf(flagbuf,"%d",attr_tab_p->ptypes.ivals->value[piomode]);
	    flag_val = flagbuf;
	    flag_len = strlen(flagbuf);
	}

	/* Resolve % Escape Sequences (if any) */
	if (flag_len > 0 && !(attr_tab_p->flags & ONCMDLINE))
	    for (cnt = 0; cnt < flag_len; cnt++)
		if (*(flag_val + cnt) == '%') {
		    /* contains %, so value is from data base, not cmd line */
		    strinfop = get_str(attrname, MSG_ATTRFAIL);
		    flag_val = strinfop->ptr;
		    flag_len = strinfop->len;
		    break;
		}

	/* Build fixed info. for header page, preview option, and/or diag */
	if (gen_info)
	{
	    if (hdrwidth > 0)  /* if formatting flag values into lines */
		if ((hchcnt + flag_len + 5) > hdrwidth  &&
			    (flag_len + 5) < hdrwidth)
		{   /* not enough room on the line */
		    *flagp++ = ',';
		    *flagp++ = '\n';
		    hchcnt = 0;
		}
	    if (hchcnt > 0)  /* if not at beginning of line */
	    {
		*flagp++ = ',';
		*flagp++ = ' ';
		hchcnt += 2;
	    }

	    *flagp++ = (char)flagchar;
	    *flagp++ = '=';
	    hchcnt += 2;

	    for (j=0; j<flag_len; j++)
		*flagp++ = *(flag_val + j);
	    hchcnt += flag_len;
	} /* end if (gen_info) */

    } /* end for flagchar loop */

    /*
    ** if general header information is requested, then must check to see if
    ** preview level is also requested. if so prepend a message and return
    ** a code indicating process.
    */

    if (gen_info)
    {
	*flagp = '\0';
	(void) putenv(flaginfo - (sizeof("PIOFLAGS=") - 1));

	if (preview == 1)
	{   /* Return info. to user and then exit (don't print) */

	    MALLOC(previewbuf, 3000);
	    wkptr = previewbuf;

	    /* "Below is the preview information requested..." */
	    premsg = getmsg(MF_PIOBE, 1, MSG_PREVIEWHDR);
	    while (*premsg)
		*wkptr++ = *premsg++;

	    /* "PRINTER:" */
	    msgptr = getmsg(MF_PIOBE, 1, MSG_PRINTER_HDR);
	    while (*msgptr)
		*wkptr++ = *msgptr++;

	    /* printer value */
	    attr_tab_p = get_attrtab_ent(PRINTERDESC);
	    if (attr_tab_p != NULL)  /* if it's in the data base */
		strinfop = get_str(PRINTERDESC, MSG_ATTRFAIL);
	    else
		strinfop = get_str(PRINTERTYPE, MSG_ATTRFAIL);
	    msgptr = strinfop->ptr;
	    while (*msgptr)
		*wkptr++ = *msgptr++;
	    attr_tab_p = get_attrtab_ent(DATASTRDESC);
	    if (attr_tab_p != NULL) {  /* if it's in the data base */
		*wkptr++ = ' ';
		*wkptr++ = '(';
		strinfop = get_str(DATASTRDESC, MSG_ATTRFAIL);
		msgptr = strinfop->ptr;
		while (*msgptr)
		    *wkptr++ = *msgptr++;
		*wkptr++ = ')';
	    }
	    *wkptr++ = '\n';
	    *wkptr++ = '\n';

	    /* "FLAG VALUES:" */
	    msgptr = getmsg(MF_PIOBE, 1, MSG_FLAGVALSHDR);
	    while (*msgptr)
		*wkptr++ = *msgptr++;

	    /* values for the flags */
	    for (flagp = flaginfo; *flagp;)
		*wkptr++ = *flagp++;
	    *wkptr++ = '\n';
	    *wkptr++ = '\n';

	    /* "PIPELINE OF FILTERS:" */
	    msgptr = getmsg(MF_PIOBE, 1, MSG_DOCPIPE2);
	    while (*msgptr)
		*wkptr++ = *msgptr++;

	    /* pipeline */
	    wkptr += out_pipe(docpipe[1], (FILE *) NULL, wkptr);

	    *wkptr = '\0';
	    piomsgout(previewbuf);
	    if (sigterm)
		exit(EXITSIGNAL);
	    exit(EXITOK);
	}
    }
    return(0);
}


/*
*******************************************************************************
** NAME:	get_str()
**
** DESCRIPTION:	retrieves a string from the attribute parameter table with all
**		it variables resolved.  All error handling is handled in this
**		routine.  The calling process includes the error message
**		number in the parameters passed.  If anything at all goes
**		wrong then the asked for message is output and the process is
**		exited.  This relieves the calling process of having any direct
**              knowledge of the structure of the attribute table.
**
** CALLING
**  SEQUENCE:	struct str_info *
**		  get_str(attribute_name, message_number);
**		  char	*attribute_name
**		  int	message_number
**
*******************************************************************************
*/

struct str_info *get_str(attrname, msgnum)
char	*attrname;
int	msgnum;

{
    char	*buf;
    char        *wkptr;
    struct str_info *strinfo;

    attr_tab_p = get_attrtab_ent(attrname);
    if(attr_tab_p == NULL)
	ERREXIT(0, msgnum, attrname, NULL, 0);

    /*
    ** If the datatype is a VAR_STR then the variables are already
    ** resolved and can pass the pointer to them back directly.  If not
    ** a VAR_STR then they must be resolved manually.
    */

    if(attr_tab_p->datatype == VAR_STR)
    {
	size_t		avlen;

	wkptr = (attr_tab_p->ptypes.sinfo + piomode)->ptr;
	avlen = (attr_tab_p->ptypes.sinfo + piomode)->len;
	if (avlen)
	{
	if (!strncmp(wkptr, YES_STRING, avlen))
	    *wkptr = '1';
	else if (!strncmp(wkptr, NO_STRING, avlen))
	    *wkptr = '0';
	}
	return(attr_tab_p->ptypes.sinfo + piomode);
    }

    MALLOC(wkptr, sizeof(struct str_info));
    strinfo = (struct str_info *) wkptr;
    MALLOC(buf, 2000);
    strinfo->len = piogetstr(attrname, buf, 2000, NULL);
    if (!strcmp(buf, YES_STRING))
	(void) strcpy(buf, "1");
    else if (!strcmp(buf, NO_STRING))
	(void) strcpy(buf, "0");

    strinfo->ptr = buf;		/* buf contains data, update struct */
    return(strinfo);
}

/*
*******************************************************************************
** NAME:        getdocpipe()
**
** DESCRIPTION: return the pipeline for the specified print file number.  If
**              the pipeline had been previously generated for that print
**              file, a pointer to the pipeline is returned.  Otherwise,
**              the pipeline is generated and a pointer returned to the caller.
**
** CALLING
**  SEQUENCE:   char *getdocpipe(filenum, argv, opt);
**                int filenum;
**                char *argv[];
**                int opt;
*******************************************************************************
*/
char *
getdocpipe(filenum, argv, opt)
int filenum;
char *argv[];
int opt;       /* 1: without pioout;  2: pioout also;  3: pioout only */
{
    static char nullstr[] = "";
    char attrname[3];
    char *wkptr;
    char *docpipe1, *docpipe2;
    char *prtfile = NULL;
    struct str_info *strinfop_filter;
    int tplen,j,i;

    if ( argv != NULL )
	prtfile = argv[optind + filenum - 1];
    if (prtfile != NULL){
        if (strchr(prtfile,'%') != NULL){	/* Search for % in filename */
	    tplen = strlen(prtfile);		/* Get length of filename */
	    for( i=0, j=0; i <= tplen; i++, j++){	/* For length of */
	         tpbuff[j] = prtfile[i];	/* filename, check for % sign */
	         if (tpbuff[j] == '%') {	/* if so, add another % sign */
		    j++;			/* to filename and */
		    tpbuff[j] = '%';
	         }
	    }
	    tpbuff[j] = '\0';			/* null terminate */
            SET_FLAG(PRINTFILE, tpbuff, strlen(tpbuff));
        }
	else {
        SET_FLAG(PRINTFILE, prtfile, strlen(prtfile));
	}
    }
    SET_FLAG(IN_DATASTREAM, &datastream[filenum], 1);

    if (opt == 3)
	docpipe1 = nullstr;
    else
	docpipe1 = docpipe[filenum];
    if (docpipe1 == NULL)
    {
	/* Get (optional) Filter String */
	strinfop = get_str(FILTERNAME, MSG_ATTRFAIL);  /* get _f attr value */
	strinfop_filter = NULL;
	piopipeline = '\0';   /* so can tell if %ix is in filter pipeline */
	if (strinfop->len >= 1)
	{
	    (void) strcpy(attrname, FILTERCHAR);
	    attrname[1] = strinfop->ptr[0];
	    strinfop_filter = get_str(attrname, MSG_FILTER);
	}

	/* Get Main Pipeline */
	piofilterloc = 0;  /* filter string goes at beginning (offset 0)
			      of the main pipeline, unless pioparm() finds
			      %p in the main pipeline */
        piodevdriloc = -1; /* pioout (device driver interface routine) string
                              goes at end of the main pipeline, unless
                              pioparm() finds %z in the main pipeline */
	(void) strcpy(attrname, PIPELINECHAR);    /* "i" */
	if (piopipeline == '\0')   /* if pioparm() didn't find a %ix */
	{
	    attrname[1] = datastream[filenum]; /* datastream character */
	    strinfop = get_str(attrname, MSG_DATASTREAM);
            if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                piodevdriloc = strinfop->len; /* pioout goes at end of string*/
	    docpipe1 = strinfop->ptr;
	}
	else
	{  /* pioparm() found a %ix */
	    if (piopipeline == '!')
            {
		docpipe1 = nullstr;   /* make main pipeline a null string */
                piodevdriloc = 0;
            }
	    else
	    { /* use main pipeline as overridden by the filter string (%ix) */
		attrname[1] = piopipeline;
		strinfop = get_str(attrname, MSG_DATASTREAM);
                if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                    piodevdriloc = strinfop->len; /* pioout at string end */
		docpipe1 = strinfop->ptr;
	    }
	}

	/* Prepend (optional) Filter Pipeline to Main Pipeline */
	/* (leave room for " | " and null terminator)          */
	if (strinfop_filter != NULL && strinfop_filter->len > 0)
	{
	    if (*docpipe1 != '\0')       /* is there a document pipeline? */
	    {
		wkptr = docpipe1;
		MALLOC(docpipe1, strlen(docpipe1) + strinfop_filter->len + 5);
		if (piofilterloc > 0)  /* if filter pipeline not at beginning
		 			  of main pipeline */
		{
		    strncpy(docpipe1, wkptr, piofilterloc);
		    strcat(docpipe1, " ");
                    piodevdriloc += 1;           /* add length of " " */
		    wkptr += piofilterloc;
		}
		else
		    *docpipe1 = '\0';
		(void) strcat(docpipe1, strinfop_filter->ptr);
		(void) strcat(docpipe1, " | ");
                piodevdriloc += 3;               /* add length of " | " */
		(void) strcat(docpipe1, wkptr);
	    }
	    else
		docpipe1 = strinfop_filter->ptr;
            piodevdriloc += strinfop_filter->len;  /* add len of filter str */
	}
	docpipe[filenum] = docpipe1;
        doc_ddloc[filenum] = piodevdriloc;
    }
    if (opt == 1)
	return(docpipe[filenum]);

    /*
    ** Add Device Driver Interface to Document Pipeline
    */

    strinfop = get_str(DDIF_CMDSTRING, MSG_ATTRFAIL);
    if (strinfop->len > 0)
    {
	if (*docpipe1 != '\0')
	{
	    MALLOC(docpipe2, strlen(docpipe1) + strinfop->len + 4);
	    (void) strncpy(docpipe2, docpipe1, doc_ddloc[filenum]);
            *(docpipe2 + doc_ddloc[filenum]) = '\0';
	    (void) strcat(docpipe2," | ");
	    (void) strcat(docpipe2, strinfop->ptr);
            (void) strcat(docpipe2, docpipe1 + doc_ddloc[filenum]);
	}
	else
	    docpipe2 = strinfop->ptr;
    }
    else
	docpipe2 = docpipe1;
    return(docpipe2);
}


/*
*******************************************************************************
** NAME:        getpipe()
**
** DESCRIPTION: return the header or trailer pipeline for the file number.
**              If the pipeline had been previously generated for that print
**              file, a pointer to the pipeline is returned.  Otherwise,
**              the pipeline is generated so a pointer can be returned.
**
** CALLING
**  SEQUENCE:   char *getpipe(filenum, argv, opt, type);
**                int filenum;
**                char *argv[];
**                int opt;
**                int type;
*******************************************************************************
*/
char *
getpipe(filenum, argv, opt, type)
int filenum;
char *argv[];
int opt;       /* 1: without pioout;  2: pioout also   */
int type;      /* HDRPIPE: header     TRLPIPE: trailer */
{
    char *pipe1, *pipe2;
    char *prtfile;

    prtfile = argv[optind + filenum - 1];
    if (prtfile != NULL)
        SET_FLAG(PRINTFILE, prtfile, strlen(prtfile));
    SET_FLAG(IN_DATASTREAM, &datastream[filenum], 1);

    if (type == HDRPIPE)
	pipe1 = hdrpipe[filenum];
    else
	pipe1 = trlpipe[filenum];

    if (pipe1 == NULL)
    {   /* pipeline not constructed yet */

	/* Get Main Pipeline */
        piodevdriloc = -1; /* pioout (device driver interface routine) string
                              goes at end of the main pipeline, unless
                              pioparm() finds %z in the main pipeline */
	if (type == HDRPIPE)
	{
	    strinfop = get_str(HDR_PIPELINE,MSG_ATTRFAIL);
	    pipe1 = hdrpipe[filenum] = strinfop->ptr;
            if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                piodevdriloc = strinfop->len; /* pioout goes at end of string*/
            hdr_ddloc[filenum] = piodevdriloc;
	}
	else
	{
	    strinfop = get_str(TRLR_PIPELINE,MSG_ATTRFAIL);
	    pipe1 = trlpipe[filenum] = strinfop->ptr;
            if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                piodevdriloc = strinfop->len; /* pioout goes at end of string*/
            trl_ddloc[filenum] = piodevdriloc;
	}
    }
    if (opt == 1)
	return(pipe1);

    /* Add Device Driver Interface */
    strinfop = get_str(DDIF_CMDSTRING, MSG_ATTRFAIL);
    if (strinfop->len > 0)
    {
	if (*pipe1 != '\0')
	{
            if (type == HDRPIPE)
                piodevdriloc = hdr_ddloc[filenum];
            else
                piodevdriloc = trl_ddloc[filenum];
	    MALLOC(pipe2, strlen(pipe1) + strinfop->len + 4);
	    (void) strncpy(pipe2, pipe1, piodevdriloc);
            *(pipe2 + piodevdriloc) = '\0';
	    (void) strcat(pipe2," | ");
	    (void) strcat(pipe2, strinfop->ptr);
            (void) strcat(pipe2, pipe1 + piodevdriloc);
	}
	else
	    pipe2 = strinfop->ptr;
    }
    else
	pipe2 = pipe1;
    return(pipe2);
}



/*
*******************************************************************************
** NAME:        run_pipeline()
**
** DESCRIPTION: run the specified pipeline.
**
** CALLING
**  SEQUENCE:   run_pipeline(pipeline, msgnum, filename)
**                pipeline: pipeline to pass to shell
		  msgnum: message number for pipe heading
		  filename: name of print file (only with pipe for print file)
**
** RETURN:      0 - good
**             -1 - time to quit
**
*******************************************************************************
*/
run_pipeline(pipeline, msgnum, filename)
char *pipeline;
int msgnum;
char *filename;
{
    int oldsize, val, sys_rc;
    char *wkptr, *bufptr, *msgptr;

    if (diagnostic > 0) {
	fflush(diag_fp);
	if (fstat(2, &statbuf) < 0)
	    ERREXIT(0, MSG_STAT2, diagfile, NULL, errno);
	oldsize = (int) statbuf.st_size;
    }

    if (diagnostic < 3)
        sys_rc = piosystem(pipeline);
    else
        sys_rc = 0;

    if (diagnostic > 0) {
	fflush(diag_fp);
	if (fstat(2, &statbuf) < 0)
	    ERREXIT(0, MSG_STAT2, diagfile, NULL, errno);
	if ((int) statbuf.st_size > oldsize && !sigterm) {
	    wkptr = getmsg(MF_PIOBE, 1, MSG_STDERR);
	    while (*wkptr != NULL)
		pioputc(*wkptr++, diag_fp);
	    fprintf(diag_fp, getmsg(MF_PIOBE, 1, msgnum), filename);
	    (void) out_pipe(pipeline, diag_fp, (char *) NULL);
	    fflush(diag_fp);
	    rewind(diag_fp);
	    if (fstat(2, &statbuf) < 0)
		ERREXIT(0, MSG_STAT2, diagfile, NULL, errno);
	    MALLOC(bufptr, (int) statbuf.st_size + 5000);
	    wkptr = bufptr;
	    while ((val = getc(diag_fp)) != EOF)
		*wkptr++ = (char) val;
	    msgptr = getmsg(MF_PIOBE, 1, MSG_FLAGVALSHDR);
	    while (*msgptr)
		*wkptr++ = *msgptr++;
	    for (flagp = flaginfo; *flagp;)
		*wkptr++ = *flagp++;
	    *wkptr = '\0';
	    if ((wkptr = getenv("PIOMAILONLY")) != NULL)
		if (*wkptr == '0')
		{
		    errinfo.errtype = 0;
		    piogenmsg(MSG_MAILED_DIAG, TRUE);
					/* tell user we mailed it */
		    *wkptr = '1';   /* force message to be mailed */
		}
	    piomsgout(bufptr);
	    return(-1);
	}
	if (diagnostic >= 2) {
	    fprintf(diag_fp, getmsg(MF_PIOBE, 1, msgnum), filename);
	    (void) out_pipe(pipeline, diag_fp, (char *) NULL);
	}
    }
    return(sys_rc);
}


/*
*******************************************************************************
** NAME:        out_pipe()
**
** DESCRIPTION: formats the specified pipeline and outputs it to the
**              specified stream or buffer.
**
** CALLING
**  SEQUENCE:   out_pipe(pipeline, fileptr, bufptr)
**                char  *pipeline
**                FILE  *fileptr       for output to stream
**                char  *bufptr        for output to buffer
**
** RETURN CODE: length of output string
**
*******************************************************************************
*/

out_pipe(pipeline, fileptr, bufptr)
char *pipeline;
FILE *fileptr;
char *bufptr;
{
char *wkptr, *bufp, *bufp_orig;
char buf[2000];

    wkptr = pipeline;
    if (bufptr)
	bufp = bufptr;
    else
	bufp = buf;
    bufp_orig = bufp;

    while (*wkptr != (char *) NULL)
    {
	if (*wkptr == ' ' && *(wkptr+1) == '-')
	{
	    *bufp++ = '\n';
	    *bufp++ = ' ';
	    *bufp++ = ' ';
	}
	else if ( *wkptr == ';' 
	      || (*wkptr == '|' && *(wkptr+1) != '|') 
              || (*wkptr == '&' && *(wkptr+1) != '&')
              || (*wkptr == '&' && wkptr > pipeline && *(wkptr-1) == '&')
              || (*wkptr == '|' && wkptr > pipeline && *(wkptr-1) == '|'))
	{
	    *bufp++ = *wkptr++;
	    *bufp++ = '\n';
	    while (*wkptr == ' ')
		wkptr++;
	    if (*wkptr == (char *) NULL)
		break;
	}
	*bufp++ = *wkptr++;
    }
    *bufp++ = '\n';
    *bufp++ = '\n';
    *bufp = '\0';
    if (fileptr)
	for (wkptr = bufp_orig; *wkptr; wkptr++)
	    pioputc(*wkptr, fileptr);
    return(bufp - bufp_orig);
}


/*
*******************************************************************************
** NAME:        str2ascii()
**
** DESCRIPTION: converts binary string to ASCII string.  If input byte is not
**              an ASCII character, or the character is special to the shell,
**              octal notation (i.e., \xxx) is used.
**
** CALLING
**  SEQUENCE:   str2ascii(bin_str, str_len);
**              char *bin_str;
**              int str_len;
**
** RETURN:      pointer to constructed ASCII string
**
*******************************************************************************
*/

char *
str2ascii(bin_str, str_len)
char *bin_str;
int str_len;
{
int ch, cnt;
char *bufptr, *wkptr;

MALLOC(bufptr, (4 * str_len) + 1);
for (cnt = 0, wkptr = bufptr; cnt < str_len; cnt++)
{
    ch = (int) *(bin_str + cnt);
    if (ch >= ' ' && ch <= '~' && !shellchar[ch])
    {
	if (ch >= '0' && ch <= '7')
	{
	    if (wkptr >= bufptr + 2 && *(wkptr - 2) == '\\')
	    {
		*wkptr = *(wkptr - 1);
		*(wkptr - 1) = '0';
		wkptr++;
	    }
	    if (wkptr >= bufptr + 3 && *(wkptr - 3) == '\\')
	    {
		*wkptr = *(wkptr - 1);
		*(wkptr - 1) = *(wkptr - 2);
		*(wkptr - 2) = '0';
		wkptr++;
	    }
	}
	*wkptr++ = (char) ch;
    }
    else
	wkptr += sprintf(wkptr, "\\%o", ch);
}
*wkptr = '\0';
return (bufptr);
}


/*
*******************************************************************************
** NAME:        chkprohib
**
** DESCRIPTION: Scans the list of flag letters in the attribute string for
**              the specified attribute name to see if any of the flags are
**              specified on the command line.  If so, error message & quit.
**
** CALLING
**  SEQUENCE:   chkprohib(attrname)
**                char *attrname;
**
*******************************************************************************
*/
chkprohib(attrname)
char *attrname;
{
int cnt;
char *cp;
char flagattr[2];

(void) strncpy(flagattr, FLAGCHAR, 1);
attr_tab_p = get_attrtab_ent(attrname);
if (attr_tab_p != NULL)
{
	strinfop = get_str(attrname, MSG_ATTRFAIL);
	cp = strinfop->ptr;
	for (cnt = 0; cnt < strinfop->len; cnt++)
	{
		*(flagattr + 1) = *(cp + cnt);
		attr_tab_p = get_attrtab_ent(flagattr);
		if (attr_tab_p != NULL)
			if (attr_tab_p->flags & ONCMDLINE)
				ERREXIT(0, MSG_PROHIBFLAG, flagattr+1, NULL, 0);
	}
}

return(0);

}

/*
*******************************************************************************
** NAME:        piosystem()
**
** DESCRIPTION: invokes a shell and passes it the specified pipeline string
**
** CALLING
**  SEQUENCE:   piosystem(pipeline_string);
**                char  *pipeline_string
**
*******************************************************************************
*/

extern int wait();
char    trap_com[100] = "trap \"\" 15;\n";

int
piosystem(s)
char    *s;
{
    int         status, pid, w;
    int         looptrue;
    void (*istat)(), (*qstat)();
    char        sh_com[1000];

    (void) strcpy(sh_com, trap_com);
    (void) strcat(sh_com, s);

    if((pid = fork()) == 0)
    {
	(void) setpgrp();           /* set group process i.d. */
	(void) execl("/usr/bin/ksh", "ksh", "-c", sh_com, 0);
	ERREXIT(0, MSG_EXECFAIL, NULL, NULL, errno);
    }

    if (pid == -1)
    {
	ERREXIT(0, MSG_FORKFAIL, NULL, NULL, errno);
    }

    processgrp = pid;    /* child will make its process ID the group ID */

    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    looptrue = 1;
    while(looptrue)
    {
	while((w = wait(&status)) != pid && w != -1)
	    ;
	if (w == -1 && errno == EINTR)
	{
	    looptrue = 1;
	}
	else
	{
	    looptrue = 0;
	    if (w == -1)
	    {
		status = 0;     /* implies there is no child process to wait
				   for at this time will consider to be
				   acceptable. */
	    }
	}
    }

    if ((status & 0xff) == 0)
	status >>=8;
    else
	status &= 0x007f;

    (void) signal(SIGINT, istat);
    (void) signal(SIGQUIT, qstat);
    return(status);
}


/*
*******************************************************************************
** NAME:        terminate()
**
** DESCRIPTION: invoked by signal from qdaemon if print job is to be cancelled
**
** CALLING
**  SEQUENCE:   terminate(signal);
**                int signal;
**
*******************************************************************************
*/
void terminate(signl)
int signl;

{

    (void) signal(SIGTERM, SIG_IGN);
    sigterm = TRUE;
    (void) killpg(processgrp, SIGTERM);

}

/**************************************************************************
* NAME:     ascterm_prtr_setup
*
* DESCRIPTION: Setup routine for ASCII terminal connected printers.
*              Looks for attribute "y0" in the virtual printer colon file
*              to indicate that the queue was setup for an ASCII terminal
*              printer.  If found, set PIOASCII environment variable to
*              be accessed downpipe by pioout. 
*              Looks for attributes y1..y9, device driver specific attributes
*              set in the colon file.  The attributes are put into the environment
*              by setting PIOASCII_ATTR.
*
* PARAMETERS: NONE
*
* GLOBALS:
*   MODIFIED:
*
* RETURN VALUES:
*
*****************************************************************************
*/
#define MAXATTR	10    /* max number of virtual printer attributes */
void
ascterm_prtr_setup()
{

	int i,j;
	char	attr_id[5];  /* Attribute identifier */
	char *tmp;
	char *attr_str_tab[MAXATTR];
	int	len, total_len;


	/* Get tty device driver attributes, if any */
	i=0;
	total_len=0;
	sprintf(attr_id,"y%d",i);
	bzero(attr_str_tab, sizeof(attr_str_tab));
	while ((i < MAXATTR) && (get_attrtab_ent(attr_id) != NULL)) {
		strinfop = get_str(attr_id, MSG_ATTRFAIL);
		if (*strinfop->ptr == '\0')
				total_len += strlen("<na>:");
		else {
			if ((attr_str_tab[i] = (char *) malloc(len=strlen(strinfop->ptr)+1)) != NULL) {
				strcpy(attr_str_tab[i],strinfop->ptr);
				total_len += len;
			}
		} /* else */
		sprintf(attr_id,"y%d",++i);
	} /* while */

	if (total_len > 0) {
		if ((env_pioascii = (char *) malloc(total_len+strlen("PIOASCII_ATTR=")+1)) != NULL) {
			strcpy(env_pioascii,"PIOASCII_ATTR=");
			for (j=0;j < i;j++) {
				if (attr_str_tab[j] == NULL)
					strcat(env_pioascii,"<na>:");
				else {
					if ((tmp = (char *) malloc(strlen(attr_str_tab[j])+1)) != NULL) {
						sprintf(tmp,"%s:",attr_str_tab[j]);
						strcat(env_pioascii,tmp);
						free(tmp);
						free(attr_str_tab[j]);
					}
				} /* else */
			} /* for */
			putenv(env_pioascii); 
		}
	}
}
