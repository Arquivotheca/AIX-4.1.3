static char sccsid[] = "@(#)29	1.10.1.4  src/bos/usr/lib/pios/piodigest.c, cmdpios, bos411, 9428A410j 3/31/94 11:28:22";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, pio_digest, cpy2odmtab, oct2char, str_alloc, hash_name,
 *            validate_attrvallims, verify_refers_attrvallim,
 *            verify_syntax_attrvallim,
 *            strip_nonprt_attrvallim, find_desc, show_op
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*******************************************************************************
*
* MODULE NAME:            piodigest.c
*
* DESCRIPTION:    Data is retrieved from the input colon file, which contains
*                 all the attributes required to support a particular printer
*                 data stream.  After the data is retrieved, it is transformed
*                 and copied into a table which has been "hashed" for fast
*                 retrieval at a later time.  Each record in the table has
*                 three members:
*
*                 * An offset pointing to the attribute value (the offset is
*                   calculated from the beginning of the memory set aside for
*                   the storage of all tables and strings),
*
*                 * The length of the attribute value. All attribute values
*                   are represented as string (The formatter decides the true
*                   type of data the attribute will assume).
*
*                 * The name of the attribute.  The name is 2 chars long.
*
*                 * The length of the limits string of the attribute.
*
*                 Prior to the actual storage of the data in the table, the
*                 attribute name is "hashed" into an index table.  At most
*                 there will be a primary hash table and a secondary hash
*                 table corresponding to the first and second characters in
*                 the attribute name.  So an attribute record can be retrieved
*                 by using its attribute name as a key and at most only two
*                 table lookups will be performed to accomplish this.
*
*                 After the data has been stored in the tables, the
*                 contents of the memory allocated to hold everything are
*                 written to a file so that a formatter can easily access
*                 and use this information.
*
* INPUT:          Colon file data.  Each record has the format
*                    msgcatalogid:msgcatalogindex:attrname::value[:description]
*
*                 Command line flags:
*
*                     NOTE: values for flags not specified come from the
*                           input colon file.
*
*                     -s (or -d): data stream type.
*                     -n: device name (e.g., "lp0").
*                     -p: path name of directory to contain the
*                         output file (if not specified, the value defined by
*                         the PIOBASEDIR environment is assumed).
*                     -q: print queue name
*                     -t: printer type (e.g., 4201-3).
*                     -d (or -v): virtual printer name (queue device name).
*
*                 Command line operand:
*
*                     colonfilename: file name of the input colon
*                        file (or "-" if stdin).
*
* OUTPUT:         Transformed data, written to a file for access by the
*                 formatter associated with the data.
*
* GLOBALS:        objtot - total number of attributes retrieved.
*
*                 mem_start - Start address of the memory malloced for storage
*                             of the odm table and the hash tables along with
*                             the attribute strings.
*
*                 mem_used - Amount of memory used at any one time by tables
*                            and strings, can also be thought of as an offset
*                            from mem_start where the next string or secondary
*                            hash table to be stored will begin.
*
* EXTERNALS:      SEE piostruct.h
*
* FUNCTIONS:      main();
*                 piodigest();
*                 cpy2odmtab();
*                 oct2char();
*                 str_alloc();
*                 hash_attr();
*                 validate_attrvallimss();
*                 verify_refers_attrvallim();
*                 verify_syntax_attrvallim();
*                 strip_nonprt_attrvallim();
*		  find_desc();
*		  show_op();
*
* RETURNS:        0  success
*                !0  failure
*
*******************************************************************************/

#define _ILS_MACROS
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/lockf.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <locale.h>
#include <nl_types.h>     /*  included for getmsg  */
#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include "pioformat.h"
#include "pioattred.h"
#include "piolimits.h"


#define MAX_LONGCHAR_LENGTH  2000
#define TUPLE_NUM   3           /* Number of fields in an input record */
#define LINEFEED        10      /* value for line feed control in colon file */
#define SETDELIMITCHARS ",]"	/* delimiting chars for a set of attr. names */
#define MAXWORK_ATTRNAMELEN 15	/* max length of an embedded attr name */
#define MAXDESCLEN	1000
#define MAXOCTHEXDIGITS	3	/* maxno of octal or hexadecimal digits denoting
				   a char (using "\ooo" or "\xhh" notation)
				 */
#define MAXOCTDIGITS	3
#define MAXHEXDIGITS	2
#define BACKSLASH	'\\'
#define TMPINTSTRSIZ	15


/* External function declaractions */
extern int		lockf (int fd, int request, off_t size);
						/* should be in sys/lockf.h */
extern char 		*getmsg (char *CatName, int set, int num);
extern char		*basename (char *);
extern int		yyparse (void);

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);


/* Local function declarations */
static int		pio_digest (char *dirname, char *printertype,
				    char *datastream, char *devname,
				    char *qname, char *vpname, FILE *colonfile);
static int		hash_name (mem_ushval_t offset, char *name);
static mem_ushval_t	cpy2odmtab (mem_ushval_t offset, char *string,
				    mem_ushval_t len, int attrvalflag);
static mem_ushval_t	oct2char (mem_ushval_t offset, char *string,
				  int attrvalflag);
static int		str_alloc (mem_ushval_t size);
static int		validate_attrvallims (unsigned int noofobjs);
static int		verify_refers_attrvallim
			   (const char *attrnm, const char *attrvallim,
			     int attrvalflag);
static int		verify_syntax_attrvallim
			   (const char *attrnm, const char *attrvallim,
			     int attrvalflag);
static char		*strip_nonprt_attrvallim
			   (const char *attrval, unsigned int avlen);



typedef enum {
		OCTTYPE = 1,
		HEXTYPE }	octhex_type_t;

int  *mem_start;                /* Start of malloced memory */

unsigned int objtot;		/* Number of objects retreive */
static mem_ushval_t mem_used = 0;
				/* Amount of memory malloc for tables & stuff */
int piopgskip = 0;              /* Because pioputc/pioputchar expect it */
int piodatasent = 0;            /* Because pioputc/pioputchar expect it */
int statusfile = 0;             /* Not running under spooler */
int piomode = PIO_CURR_MODE;    /* ODM values overridden by command line */
static int linecnt;             /* current line number in input file */
static int out_fd = -1;		/* File descriptor for file pointed by "out" */
static unsigned int errtot = 0U; /* Total errors detected during validation */

static char *colonfilename;     /* file name for optional input colon file */
static FILE *colon_file;        /* File pointer for optional input colon file */

static char attrname[3];        /* attribute name */
static char extra_attrname[3] = EXTRA_ATTRNAME; /* prototype attr name for   */
				/* extra attributes defined for use by the */
				/* Print Job Manager with pipelines        */
static char spoolflag_attrname[3] = FLAGCHAR;  /* prototype attr name for */
				/* spooler flags */
char *hash_table;
char *odm_table;
static char *basedir;           /* base (i.e., "pio") directory */
static char defbasedir[] = DEFVARDIR;


static union {                  /* used to convert back and forth between  */
    int  *intptr;               /* (char *) and (int *) and (struct odm *) */
    mem_shval_t *shptr;		/* and (short *) and (unsigned short *) */
    mem_ushval_t *ushptr;
    char *charptr;
    struct odm *odmptr;
} memptr;

SPOOLERFLAGS                    /* spoolerflags array of structures (defined in
				   piostruct.h): table of spooler flags, their
				   default values, and whether or not they
				   have an argument on the command line */


/*******************************************************************************
*                                                                              *
* NAME:           main                                                         *
*                                                                              *
* DESCRIPTION:    Processes input flags                                        *
*                                                                              *
* PARAMETERS:     parameters from command line                                 *
*                                                                              *
* INPUT:          colon file                                                   *
*                                                                              *
* OUTPUT:         Transformed input data, written to a file for access by the  *
*                 formatter associated with the data.                          *
*                                                                              *
* GLOBALS                                                                      *
*     MODIFIED:   objtot - total number of objects in the tables.              *
*                 mem_start - address of the memory image.                     *
*                                                                              *
* RETURN VALUES:  successful:  0                                               *
*                 failure:     (exit)                                          *
*                                                                              *
*******************************************************************************/

int
main (int argc, char **argv)
{
    extern char *optarg;        /* ptr to current argument - set by piogetarg */
    extern int optind;          /* index for next cmd line arg to process */
    extern int opterr;          /* flag to suppress error msgs from getopt() */
    extern int optopt;          /* flag letter */

    char *dirname = NULL;       /* directory for output file                  */
    char *printertype = NULL;   /* printer type                               */
    char *datastream = NULL;    /* date stream type                           */
    char *devname = NULL;       /* name of the device (e.g., "lp0")           */
    char *qname = NULL;         /* print queue name                           */
    char *vpname = NULL;        /* name of virtual printer (queue device)     */

    char flagstr[2];            /* string to store flag letter in for err msg */
    int chval;                  /* int character value */

    setlocale(LC_ALL, "");

    /* Store the program name in errinfo structure for msg display purposes */
    (void) strncpy
       (errinfo.progname, basename (*argv), sizeof (errinfo.progname) - 1);

    {
    /* Convert Old Flag Letters (if present) To New Flag Letters (d->s, v->d) */
    int cnt;
    int oldflags = 0;

    for (cnt = 1; cnt < argc; cnt++)
      if (*argv[cnt] == '-' && *(argv[cnt] + 1) == 'v')
      {
	oldflags = 1;
	break;
      }
    if (oldflags)
      for (cnt = 1; cnt < argc; cnt++)
	if (*argv[cnt] == '-' && *(argv[cnt] + 1) == 'd')
	  *(argv[cnt] + 1) = 's';
	else if (*argv[cnt] == '-' && *(argv[cnt] + 1) == 'v')
	  *(argv[cnt] + 1) = 'd';
    }

    /* Determine Base Directory */
    if ((basedir = getenv("PIOBASEDIR")) == NULL)
        basedir = defbasedir;                   /* default base directory */

    /* check for flags */
    opterr = 0;       /* suppress error messages from getopt() */
    while ((chval = (int) getopt(argc, argv, "d:n:p:q:s:t:")) != EOF)
    {
	switch(chval)
	{
		/* data stream arg */
	    case 's':
		datastream = optarg;
		break;

		/* device name arg */
	    case 'n':
		devname = optarg;
		break;

		/* directory path name arg */
	    case 'p':
		dirname = optarg;
		break;

		/* queue name arg */
	    case 'q':
		qname = optarg;
		break;

		/* printer type arg */
	    case 't':
		printertype = optarg;
		break;

		/* queue device name arg */
	    case 'd':
		vpname = optarg;
		break;

	    case '?':
		flagstr[0] = (char) optopt;
		flagstr[1] = '\0';
		ERREXIT(0, MSG_BADFLAG3, flagstr, NULL, 0);
		break;
	}
    }

    /* make sure that one, and only one, operand (file name) was specified */
    if ((argc - 1) != optind)
	ERREXIT(0, MSG_OPERAND, NULL, NULL, 0);

    /* Message ipc initialization */
    pmi_initmsgipc();

    /* get ready to create new files */
    (void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));

    /* if colon file is not stdin, open the file */
    colonfilename = argv[optind];
    if (!strcmp(colonfilename, "-"))
	colon_file = stdin;
    else
	if ((colon_file = fopen(colonfilename, "r")) == NULL)
	    ERREXIT(0, MSG_FOPEN4, colonfilename, NULL, errno);

    /* Do the work */
    pio_digest(dirname, printertype, datastream, devname, qname, vpname,
	       colon_file);

    return(PIOEXITGOOD);
}



/*******************************************************************************
*									       *
* NAME:           pio_digest                                                   *
*									       *
* DESCRIPTION:    Reads attributes from the input file, builds memory image    *
*                 of tables containing the attribute names, values, and value  *
*                 lengths, and writes the memory image to a file.              *
*									       *
* PARAMETERS:     parameters from command line                                 *
*									       *
* INPUT:          attributes from colon file                                   *
*									       *
* OUTPUT:         Transformed input data, written to a file for access by the  *
*		  formatter associated with the data.			       *
*									       *
* GLOBALS								       *
*     MODIFIED:   objtot - total number of objects in the tables.              *
*                 mem_start - address of the memory image.                     *
*									       *
* RETURN VALUES:  successful:  0                                               *
*                 failure:     (exit)                                          *
*									       *
*******************************************************************************/

static int
pio_digest (char *dirname, char *printertype, char *datastream,
	    char *devname, char *qname, char *vpname, FILE *colonfile)
{
    extern unsigned int objtot;

    char odm_filepath[BUFSIZ];			/* full pathname of odm file */
    char ch;                    /* work char */
    char *strptr;               /* work pointer */
    struct dirent *dp;          /* pointer to directory entry info. */
    DIR *dirp;                  /* value returned by opendir() */
    char *suffixptr;            /* ptr to  ".queue:queuedevice" in output
				   file name */
    size_t suffixlen;           /* length of ".queue:queuedevice" in output
				   file name */
    int cnt;                    /* work counter */
    char workbuf[MAX_LONGCHAR_LENGTH]; /* work buffer for colon file input */
    char workbuf1[MAX_LONGCHAR_LENGTH]; /* work buffer 1 for colon file input
					   used for attribute limits string */
    char ddidir[1000];          /* "ddi" directory path */

    FILE *tempfile;             /* work file for when colon file is stdin */

    int count;                  /* work integer */
    int fildes;                 /* File desc. of odm data file to be made */
    mem_ushval_t offset;        /* offset into odm table */
    mem_ushval_t saved_offset = 0; /* offset value for attr for file name */
    int objcnt;			/* object iterator (used for 'for' loops) */
    mem_ushval_t len;           /* length of strings stored in odm table */
    int comment;                /* is attribute a comment attribute? (0 or 1) */
    int spoolflagattr;          /* is attribute for spooler flag? (0 or 1) */
    int charval;                /* character value from colon file */
    int prevchar;               /* previous character read from input file */
    int skipline;               /* indicates current input line being skipped */
    int real_objtot;            /* actual number of objects */
    int attrname_char;          /* work variable for extra_attrname[1] */
    size_t prefix_len;          /* length of prefix (dir + /) for file name */
    int validateflag = FALSE;	/* flag to indicate whether to validate
				   attrvals; this is set upon finding "zV"
				   attribute set to TRUE */

    struct odm *odm_tab_p;	    /* points to an odm table record */ 
    struct stat statbuf;	    /* file stat structure */
    attrothinfo_t *aoip;	/* attribute other (non-ddi) info. */
    register attrothinfo_t *tp; /* tmp ptr */
    char psctnm[SECTNMLEN+1] = ""; /* previous section name length */
    register int i;		/* tmp counter */
    char msgnostr[SHRTSTRLEN+1]; /* string to store msg no */
    void *vp = NULL; 		/* temp. void pointer */
    register struct attr_info *ap; /* temp. attr_info pointer */
    register struct odm	 *tmpodmp; /* Temporary ODM pointer to an odm record */
    register unsigned int objcntr; /* counter to object number */
    char	qnm[MAXQNMLEN+1];
    char	qdnm[MAXVNMLEN+1];

    /* initialize "ddi" dirctory path name */
    (void) strcpy(ddidir, basedir);
    (void) strcat(ddidir, "/ddi");

    /* count attributes in colon file */
    if (colonfile == stdin)   /* can't do rewind for stdin from pipe */
	if ((tempfile = tmpfile()) == NULL)
	    ERREXIT(0, MSG_TMPFILE, NULL, NULL, errno);

    /* Count the attributes in the input file */
    objtot = 0;   /* attribute count */
    cnt = 0;      /* colon count */
    prevchar = LINEFEED;  /* in case first line is comment line */
    skipline = FALSE;
    strptr = workbuf;
    while((charval = piogetc(colonfile)) != EOF)
    {
	switch (charval)
	{
	case '#':  /* comment line? */
	    if (prevchar == LINEFEED)
		skipline = TRUE;
	    break;
	case ':':  /* field separator */
	    cnt++;
	    if (cnt == 3)                    /* if end of attr name field */
	    {
		/* Determine if the attr is a comment attribute, and if so,
		   skip it. */
		if (strptr - workbuf >= 2    /* attr name is atleast 2B long */
		    &&  (*workbuf == '_' && *(workbuf + 1) == '_'))
					     /* if a comment attribute */
		    skipline = TRUE;         /* ignore the attr */
		else if (strptr - workbuf == 2  /* attr name is 2 char long */
		         && *workbuf == '_')    /* and is a flag attr */
		/* Determine if the attr is for a spooler flag */
		    for (count = 0; count < NUMSPOOLERFLAGS; count++)
		        if (*(workbuf + 1) == spoolerflags[count].flagname)
		        {                 /* if this is attr for spooler flag */
			    skipline = TRUE;  /* ignore the attr */
			    break;
		        }
	    }			/* if cnt == 3 */
	    break;
	case LINEFEED:  /* end of line */
	    if (!skipline)
		objtot++;
	    else
		skipline = FALSE;
	    cnt = 0;   /* reset colon count */
	    strptr = workbuf;    /* get ready for next attr name */
	    break;
	default:
	    if (cnt == 2)
		*strptr++ = (char) charval;  /* attr name char */
	} /* end switch */

	if (colonfile == stdin)
	    pioputc(charval, tempfile);
	prevchar = charval;
    }
    if (!skipline && cnt > 0)
	objtot++;    /* last line is only partially there */
    if (colonfile == stdin)
	colonfile = tempfile;
    rewind(colonfile);

    real_objtot = objtot;          /* save actual # of objects for later */
    objtot += NUM_EXTRA_ATTR;      /* add extra ones for Print Job Mgr.  */
    objtot += NUMSPOOLERFLAGS;     /* add extra ones for spooler flags */

    /* Allocate space to store other attribute (non-ddi) information. */
    MALLOC(aoip,(real_objtot+1)*sizeof(*aoip));
    (void)memset((void *)(aoip+real_objtot),0,sizeof(*aoip));

    /*
     * Allocate space for the odmtable and the primary hash table enough space
     * to hold an integer in which to store the total number of objects in
     * the odm table.
     */
    (void) str_alloc ((mem_ushval_t) (HASH_TAB_SIZ +
		       (objtot * sizeof(struct odm))+sizeof(int)));

    /* copy the total number of odm objects to the first few bytes of memory */
    (void) memcpy((char *) mem_start, (char *) &objtot, sizeof(int));

    /* fill in the odm_table, and "hash" the attribute name */
    linecnt = 1;  /* for error messages */
    for (objcnt = 0, tp = aoip, *(psctnm+sizeof(psctnm)-1) = 0;
	 objcnt < real_objtot; objcnt++, linecnt++)
    {
	offset = (sizeof(int) + (objcnt * sizeof(struct odm)));

	/* ignore comment lines (first character is #) */
	while ((charval = piogetc(colonfile)) == '#')
	{
	    while ((charval = piogetc(colonfile)) != LINEFEED)
		if (charval == EOF)
		    ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    linecnt++;
	}
	(void) ungetc(charval, colonfile);

	/* save the first two fields in the colon file */
	    for (i = 0; (charval = piogetc(colonfile)) != ':'; )
	    {
		if (charval == EOF)
		    ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
		if (charval == LINEFEED)
		    ERREXIT(0, MSG_EOL, colonfilename, NULL, linecnt);
		if (i < sizeof(tp->mcnm)-1)
		   *(tp->mcnm+i++) = charval;
	    }
	    *(tp->mcnm+i) = 0;
	    for (i = 0; (charval = piogetc(colonfile)) != ':'; )
	    {
		if (charval == EOF)
		    ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
		if (charval == LINEFEED)
		    ERREXIT(0, MSG_EOL, colonfilename, NULL, linecnt);
		if (i < sizeof(msgnostr)-1)
		   *(msgnostr+i++) = charval;
	    }
	    *(msgnostr+i) = 0;
	    tp->msgno = strtol(msgnostr,NULL,10);

	/* get the attribute name from the colon file */
	strptr = workbuf;
	while ((charval = piogetc(colonfile)) != ':')
	{
	    if (charval == EOF)
		ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    if (charval == LINEFEED)
		ERREXIT(0, MSG_EOL, colonfilename, NULL, linecnt);
	    *strptr++ = (char) charval;
	}
	*strptr = '\0';
	strptr = workbuf;

	/* Verify that attribute name is valid */
	len = strlen(strptr);
	comment = (*strptr == '_' && *(strptr+1) == '_');
	if (len < 2 || (!comment && len > 2) || (comment && len > 5))
	    ERREXIT(0, MSG_ATTRNAME2, colonfilename, strptr, linecnt);
	for (count = 0; count < 2; count++)
	{
	    ch = *(strptr + count);
	    if (ch < '0' || (ch > '9' && ch < 'A') ||
		  (ch > 'Z' && ch < 'a' && ch != '_') || ch > 'z')
		ERREXIT(0, MSG_ATTRNAME2, colonfilename, strptr, linecnt);
	}

	/* Ignore the attribute, if it is a comment attribute. */
	if (comment)
	{
	    /* skip over the rest of the line */
	    while ((charval = piogetc(colonfile)) != LINEFEED)
	    {
	        if (charval == EOF)
	            ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    }
	    (void)strncpy(psctnm,strptr,sizeof(psctnm)-1);
	    objcnt--;		/* deduct this attribute from the total count */
	    continue;		/* go to fetch next attribute */
	}

	/* Ignore attribute if it is for spooler flag */
	if (strlen(strptr) == 2
	      && *strptr == '_')
	{
	    spoolflagattr = FALSE;
	    for (count = 0; count < NUMSPOOLERFLAGS; count++)
		if (*(strptr + 1) == spoolerflags[count].flagname)
		{                         /* if attr for spooler flag */
		    /* skip over rest of line */
		    while ((charval = piogetc(colonfile)) != LINEFEED)
			if (charval == EOF)
			    ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
		    spoolflagattr = TRUE;  /* it is a spooler flag attr */
		    break;
		}
	    if (spoolflagattr)
	    {
		objcnt--;         /* don't count the attr */
		continue;         /* on to next attr */
	    }
	}

	(void) strncpy(attrname, strptr, 2);

	/* Store other attribute (non-ddi) info. */
	(void)memcpy((void *)tp->anm,(void *)strptr,sizeof(tp->anm));
	(void)memcpy((void *)tp->sctnm,(void *)psctnm,sizeof(tp->sctnm));
	tp++;

	/* Store the attribute name and the odm table entry offset*/
	(void) hash_name(offset, attrname);

	/* Retrive attribute limits string. */
	strptr = workbuf1;
	while ((charval = piogetc(colonfile)) != ':')
	{
	    if (charval == EOF)
		ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    if (charval == LINEFEED)
		ERREXIT(0, MSG_EOL, colonfilename, NULL, linecnt);
	    *strptr++ = (char) charval;
	};
	*strptr = 0;

	/* get the attribute string from the colon file */
	strptr = workbuf;
	while ((charval = piogetc(colonfile)) != LINEFEED
	       && charval != ':')
	{
	    if (charval == EOF)
		ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    *strptr++ = (char) charval;
	}
	*strptr = '\0';
	strptr = workbuf;

	/* skip over the rest of the line (if there is anything there) */
	if (charval == ':') {
	    while ((charval = piogetc(colonfile)) != LINEFEED)
	    {
		if (charval == EOF)
		    ERREXIT(0, MSG_EOF, colonfilename, NULL, linecnt);
	    }
	}

	/* Update data base to reflect command line values */
	/* Also check to see if "zV" attribute exists and is set to true */
	if (!strncmp(attrname, PRINTERTYPE, 2) && printertype != NULL)
	    strptr = printertype;
	else if (!strncmp(attrname, OUT_DATASTREAM, 2) && datastream != NULL)
	    strptr = datastream;
	else if (!strncmp(attrname, DEVICENAME, 2) && devname != NULL)
	    strptr = devname;
	else if (!strncmp(attrname, QUEUENAME, 2))
	    if (qname)
	        strptr = qname;
	    else
		(void)strncpy(qnm,strptr,sizeof(qnm)-1),
		*(qnm+sizeof(qnm)-1) = 0,
		qname = qnm;
	else if (!strncmp(attrname, VIRTUALPTRNAME, 2))
	    if (vpname)
	        strptr = vpname;
	    else
		(void)strncpy(qdnm,strptr,sizeof(qdnm)-1),
		*(qdnm+sizeof(qdnm)-1) = 0,
		vpname = qdnm;
	else if (!strncmp(attrname, DB_FILENAME, 2))
	    saved_offset = offset;            /* save for later */
	else if (!strncmp(attrname, VALIDATE_AV_FLAG, 2))
	{
	    if (*strptr  &&  (!strcmp (strptr, YES_STRING)  ||
			      atoi (strptr) == 1))
	       validateflag = TRUE;
	}

	/* Store the attribute string, and return the length of the string */
	len = oct2char(offset, strptr, TRUE);

	/* Store the attribute string length */
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, length)),
		      (char *)&len, sizeof(len));
	
	/* Store the attribute limits string, and return the length of the
	   limits string. */
	len = oct2char(offset, workbuf1, FALSE);

	/* Store the attribute limits string length */
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, limlength)),
		      (char *)&len, sizeof(len));
    }

    /* Generate @x attributes used by Print Job Manager */
    attrname_char = '0';
    for (; objcnt < (real_objtot + NUM_EXTRA_ATTR); objcnt++)
    {
	extra_attrname[1] = (char) attrname_char;
	if (++attrname_char == ';')
	    attrname_char = 'A';              /* jump over ;<=>?@ */
					      /* use ':' for a dummy attr */
	offset = sizeof(int) + (objcnt * sizeof(struct odm));
	(void) hash_name(offset, extra_attrname);  /* store attr name */
	len = oct2char(offset, "", TRUE);       /* store attr string */
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, length)),
		      (char *)&len, sizeof(len));

	/* Store the attribute limits string, and return the length of the
	   limits string. */
	len = oct2char(offset, "", FALSE);

	/* Store the attribute limits string length */
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, limlength)),
		      (char *)&len, sizeof(len));
    }

    /* Generate attributes for spooler flags (used by Print Job Mgr.) */
    count = 0;
    for (; objcnt < (real_objtot + NUM_EXTRA_ATTR + NUMSPOOLERFLAGS); objcnt++)
    {
	spoolflag_attrname[1] = spoolerflags[count].flagname;
	offset = sizeof(int) + (objcnt * sizeof(struct odm));
	(void) hash_name(offset, spoolflag_attrname); /* store attr name */
	len = oct2char(offset, spoolerflags[count].val, TRUE);
						     /* store attr string*/
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, length)),
		      (char *)&len, sizeof(len));

	/* Store the attribute limits string, and return the length of the
	   limits string. */
	len = oct2char(offset, "", FALSE);

	/* Store the attribute limits string length */
	(void) memcpy((char *) GET_OFFSET(offset +
		      			  offsetof(struct odm, limlength)),
		      (char *)&len, sizeof(len));
	count++;
    }

    odm_table = (char *) (mem_start + 1);
    hash_table = odm_table + (objtot * sizeof(struct odm));

   /* Set up attr_info table so that evaluation of attribute value strings
      and limit strings can be done using piogetstr() to facilitate limit
      validation */
   MALLOC(vp, objtot * sizeof (struct attr_info));
   for (ap = attr_table = *(struct attr_info **) &vp,
	tmpodmp = *(struct odm **) (void **) &odm_table,
	objcntr = objtot;
	objcntr;
	--objcntr, ap++, tmpodmp++)
   {
      MALLOC(vp, sizeof (*ap->ptypes.sinfo));
      ap->ptypes.sinfo = *(struct str_info **) &vp;
      ap->ptypes.sinfo->ptr = (char *) GET_OFFSET (tmpodmp->str);
      ap->ptypes.sinfo->len = tmpodmp->length;
      ap->datatype = CONST_STR;
      ap->flags = 0;
   }

    /* Validate the data base attribute values and limits that are stored in
       the memory. */
    /* Perhaps this should be done after the last update to the memory (which
       is done later after determining the value of DB_FILENAME ["mm"] from the
       command flag args).  But since the last update in this case (the value
       of DB_FILENAME attribute) does not contain any escape command sequences,
       it should be okay to perform the validation procedure here. */
    /* However, perform the validation only when "zV" attribute is defined and
       set to true.  If validation fails, abort without any further error
       messages, as sufficient number of error messages would have been
       displayed by then. */
    if (validateflag  &&  !validate_attrvallims ((unsigned int) objtot))
       ERREXIT (0, MSG_DUMMY, "", (char *) NULL, 0);

    /* Rebuild smit ODM objects for 'print a file', 'printer setup', 'default
       job attributes' and 'pre-processing filters' dialogs. */
    piorebuild(qname,vpname,colonfilename,(unsigned int)real_objtot,aoip);

   /* Free up the memory allocated earlier */
   for (ap = attr_table; ap < attr_table + objtot; ap++)
      free((void *) ap->ptypes.sinfo);
   free((void *) attr_table), attr_table = NULL;

    /* get the path to the directory to hold the output file */
    (void) memset(odm_filepath, 0, sizeof (odm_filepath));  /* safety measure */
    if (dirname != NULL)   /* if overridden from command line */
	(void) memcpy(odm_filepath, dirname, strlen(dirname));
        /*(memcpy instead of strcpy because of temporary compiler problem)*/
    else
	(void) strcpy(odm_filepath, ddidir);
    (void) strcat(odm_filepath, "/");
    prefix_len = strlen(odm_filepath);  /* for later, remember how long it is */


/*
Construct a file name for the output file.  The name is created by
concatenating the strings for five parameters:
(printer type).(datastream type).(device name).(queue name):(queue device name)
*/

    /* printer type */
    if (printertype != NULL)   /* if overridden from command line */
	(void) strcat(odm_filepath, printertype);
    else
	if ((odm_tab_p = get_odmtab_ent(PRINTERTYPE)) != NULL)
	    (void) strncat(odm_filepath, (char *) GET_OFFSET(odm_tab_p->str),
			   (size_t) odm_tab_p->length);
    (void) strcat(odm_filepath, ".");

    /* printer data stream type */
    if (datastream != NULL)   /* if overridden from command line */
	(void) strcat(odm_filepath, datastream);
    else
	if ((odm_tab_p = get_odmtab_ent(OUT_DATASTREAM)) != NULL)
	    (void) strncat(odm_filepath, (char *) GET_OFFSET(odm_tab_p->str),
			   (size_t) odm_tab_p->length);
    (void) strcat(odm_filepath, ".");

    /* device name */
    if (devname != NULL)   /* if overridden from command line */
	(void) strcat(odm_filepath, devname);
    else
	if ((odm_tab_p = get_odmtab_ent(DEVICENAME)) != NULL)
	    (void) strncat(odm_filepath, (char *) GET_OFFSET(odm_tab_p->str),
			   (size_t) odm_tab_p->length);
    suffixptr = odm_filepath + strlen(odm_filepath);    /* save for later */
    (void) strcat(odm_filepath, ".");

    /* queue name */
    if (qname != NULL)   /* if overridden from command line */
	(void) strcat(odm_filepath, qname);
    else
	if ((odm_tab_p = get_odmtab_ent(QUEUENAME)) != NULL)
	    (void) strncat(odm_filepath, (char *) GET_OFFSET(odm_tab_p->str),
			   (size_t) odm_tab_p->length);
    (void) strcat(odm_filepath, ":");

    /* queue device name */
    if (vpname != NULL)    /* if overridden from command line */
	(void) strcat(odm_filepath, vpname);
    else
	if ((odm_tab_p = get_odmtab_ent(VIRTUALPTRNAME)) != NULL)
	    (void) strncat(odm_filepath, (char *) GET_OFFSET(odm_tab_p->str),
			   (size_t) odm_tab_p->length);

    /* save file name in digested data base */
    if (saved_offset != 0)     /* if data base has attribute for file name */
    {
	(void) strcpy(attrname, DB_FILENAME); /* for oct2char error msg */
	len = oct2char(saved_offset, odm_filepath + prefix_len, TRUE);
	(void) memcpy((char *) GET_OFFSET(saved_offset +
		      			  offsetof(struct odm, length)),
		      (char *)&len, sizeof(len));
    }

    /* get rid of the old file (if any) and any conflicting       */
    /* (i.e., end with ".queue:queuedevice") file names, (if any) */
    suffixlen = strlen(suffixptr);
    if ((dirp = opendir(ddidir)) == NULL)
	ERREXIT(0, MSG_OPENDIR, ddidir, NULL, errno);
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if (!strcmp(dp->d_name + (dp->d_namlen - suffixlen), suffixptr))
	{
	    strcpy(workbuf, ddidir);
	    strcat(workbuf, "/");
	    strcat(workbuf, dp->d_name);
	    if (unlink (workbuf) < 0)
		ERREXIT(0, MSG_UNLINK, odm_filepath, NULL, errno);
	}
    }

    /* open and stat the new file */
    if ((fildes = open(odm_filepath, O_CREAT | O_RDWR | O_TRUNC, 0664)) < 0
	  || (fstat(fildes, &statbuf)) < 0)
	ERREXIT(0, MSG_FOPEN5, odm_filepath, NULL, errno);

    /* lockf to prevent contention */
    if (lockf(fildes, F_LOCK, (off_t) mem_used) < 0)
	ERREXIT(0, MSG_LOCKF, odm_filepath, NULL, errno);

    /* write the odm table, hash tables and strings to the odm file */
    /* set the version no of the odm file to be "00001" (instead of "00000")
       to denote the new format of the ddi files (compressed memory model) */
    if ((write(fildes, ODMFILE_HEADER_TEXT, sizeof (ODMFILE_HEADER_TEXT) - 1)
	 != sizeof (ODMFILE_HEADER_TEXT) - 1) ||
	(write(fildes, (char *) mem_start, (size_t) mem_used)
	 != (int) mem_used))
	ERREXIT(0, MSG_WRITE2, odm_filepath, NULL, errno);

    /* all done */
    (void) close(fildes);
    return(0);
}



/*******************************************************************************
*
* NAME:           copy2odmtab
*
* DESCRIPTION:    copy a data base string to a location in memory, then
*                 copy the offset of the location of the string to the
*                 odm table.
*
* PARAMETERS:     offset - offset into an odm table record.
*                 string - string to copy into memory.
*                 length - length of the string to copy.
*		  attrvalflag - flag to denote whether attribute value or      *
*		  		limit string is passed; this also serves as    *
*				a flag to denote whether to copy the offset    *
*				of value string at the given offset in ODM     *
*
* GLOBALS:
*     MODIFIED:   mem_used - Total amount of memory used by the odm table and
*                            by the stored strings.
*
*     REFERENCED: objtot - total number of attributes being stored in the
*                          share memory segment.
*
* RETURN VALUES: success:    length of string
*                failure:    subroutine str_alloc will issue error msg & exit
*
*******************************************************************************/

static mem_ushval_t
cpy2odmtab(mem_ushval_t offset, char *string, mem_ushval_t len,
	   int attrvalflag)
{
    char 		*str_loc; /* ptr to where string will be stored */

    /* allocate space for string */
    (void) str_alloc(len);

    /* get address of where string will be stored */
    str_loc = (char *) GET_OFFSET(mem_used);

    /* copy the string to the address stored in str_loc */
    (void) memcpy(str_loc, string, (size_t) len);

    /* copy the offset from mem_start (where the string will be stored)
     * to the correct record in the odm_table.
     * Copy it only when the flag is TRUE.  This is to provide option for
     * copying the given offset in ODM (typically, yes for attribute value
     * string offsets and no for attribute limits string offsets.
     */
    if (attrvalflag)
       (void) memcpy((char *) GET_OFFSET(offset), (char *) &mem_used,
		     sizeof(mem_used));

    mem_used += len;

    return(len);
}



/*******************************************************************************
*									       *
* NAME: 	  oct2char						       *
*									       *
* DESCRIPTION:	  Converts octal/hexadecimal substrings embedded in odm        *
*		  attribute values into 1 byte characters.  For example a      *
*		  substring "\77" or "\077" will be converted to a character   *
*		  '?'.	In the same token, a substring "\xc" or "\x0c" will be *
*		  converted to a character '^L' (form feed - \f).  This allows *
*		  the emdedding of NULLs ('\0') in strings, which is important *
*		  because some printer commands have NULL values imbedded in   *
*		  them but the odm does not support NULLs.  Plus its just kind *
*		  of cool.						       *
*									       *
* PARAMETERS:	  offset - offset of attribute record entry in odm table.      *
*		  string - string to be massaged, the results of which to      *
*			   be stored in memory area.			       *
*		  attrvalflag - flag to denote whether attribute value or      *
*		  		limit string is passed; this also serves as    *
*				a flag to denote whether to copy the offset    *
*				of value string at the given offset in ODM     *
*									       *
* RETURN VALUES:  new size of string	  success			       *
*		  -1			  failure			       *
*									       *
*******************************************************************************/

static mem_ushval_t
oct2char (mem_ushval_t offset, char *string, int attrvalflag)
{
   char 			newstring[BUFSIZ + 1];
				/* pointer to beginning of transformed*/
   register char 		*newstring_p 	= newstring;
				/* pointer to transformed string */
   register char		*string_p	= string;
				/* pointer to original odm attribute value */
   register char		*endstring_p;
				/* pointer to end of hex/octal digits for
				   a char */
   char 			*backslashp;          
				/* pointer to backslash denoting octal/hex no */
   register int 		backslash	= FALSE;
				/* '\' flag, 1 if one is found */
   register int 		octhexcnt	= 0;
				/* number of digits following a '\' */
   register int 		octhexindex	= 0;
				/* index into octhexarray */
   unsigned int			octhexvalue	= 0;
				/* value of octal string */
   mem_ushval_t			newstrsiz = 0;
				/* size of the transformed string */
   register octhex_type_t	octhex_flag;

   /* Parse the original odm attr value string and convert octal/hexadecimal
      notations to decimal values */
   for ( ; newstring_p - newstring < sizeof (newstring) - 1  &&  *string_p;
	string_p++)
   {
      if (*string_p == BACKSLASH)		/* if '\\' */
      {
	 backslashp = string_p;
	 if (backslash = !backslash)	/* if previous char is not a '\\' */
	    continue;			/* skip this */
      }
      else if (backslash)			/* parse the octal/hex digits */
      {
	 backslash = FALSE;

	 if (*string_p == 'x')			/* hex digits */
	 {
	    /* Parse the no of proper hex digits */
	    octhex_flag = HEXTYPE;
	    for (endstring_p = ++string_p, octhexcnt = 0;
		 octhexcnt < MAXHEXDIGITS  &&
		 (*endstring_p >= '0' && *endstring_p <= '9'  ||
		  *endstring_p >= 'a' && *endstring_p <= 'f');
		 endstring_p++, octhexcnt++)
	       ;
	    endstring_p--;
	 }
	 else					/* octal digits, supposedly */
	 {
	    /* Parse the no of proper octal digits */
	    octhex_flag = OCTTYPE;
	    for (endstring_p = string_p, octhexcnt = 0;
		 octhexcnt < MAXOCTDIGITS  &&
		 (*endstring_p >= '0' && *endstring_p <= '7');
		 endstring_p++, octhexcnt++)
	       ;
	    endstring_p--;
	 }		/* end - if *string_p = 'x' */

	 /* If no proper octal/hex digits found, display error and exit */
	 /* Else, convert the notation (octal/hex) to decimal (ASCII) value */
	 if (!octhexcnt)
	 {
	    ERREXIT (0, attrvalflag ? MSG_OCT_UNDER : MSG_LMT_OCT_UNDER,
		     colonfilename, attrname, backslashp - string);
	 }
	 else
	 {
	    /* Calculate the decimal value */
	    octhexvalue = 0;
	    if (octhex_flag == HEXTYPE)		/* if hex sequence */
	    {
	       unsigned int		tmpc;

	       for (octhexindex = 0; octhexindex < octhexcnt; octhexindex++)
	       {
		  tmpc = *(endstring_p - octhexindex);
		  tmpc = (tmpc >= 'a' && tmpc <= 'f') ?
			 tmpc - 'a' + 10 : tmpc - '0';
		  octhexvalue += ((unsigned int) 1 << 4 * octhexindex) * tmpc;
	       }
	    }
	    else				/* if oct sequence */
	    {
	       for (octhexindex = 0; octhexindex < octhexcnt; octhexindex++)
		  octhexvalue += ((unsigned int) 1 << 3 * octhexindex) *
				 (*(endstring_p - octhexindex) - '0');
	    }
	    if (octhexvalue > 255)
	    {
	       ERREXIT (0, attrvalflag ? MSG_OCT_OVER : MSG_LMT_OCT_OVER,
			colonfilename, attrname, backslashp - string);
	    }

	    /* Copy the value to the new string */
	    *newstring_p++ = (char) octhexvalue;
	    if (octhexvalue == ESCCHAR)
	       *newstring_p++ = ESCCHAR;
	    string_p = endstring_p;
	 }		/* end - if !octhexcnt */

	 continue;
      }			/* end - if backslash */
      *newstring_p++ = *string_p;
   }			/* end - for *string_p */
   *newstring_p = 0;

   /* store the new string in the odm table memory area */
   (void) cpy2odmtab (offset, newstring,
		      newstrsiz = newstring_p - newstring, attrvalflag);

   return newstrsiz;
}		/* function - oct2char */



/*******************************************************************************
*									       *
* NAME: 	  str_alloc						       *
*									       *
* DESCRIPTION:								       *
*		  If no memory has been allocated then malloc the memory       *
*		  otherwise if memory has already been allocated and	       *
*		  no more space exists in the currently allocated space        *
*		  realloc BUFSIZE more space, do this until the current        *
*		  size requirements are met.  If there is enough space	       *
*		  do not do anything.					       *
*									       *
* PARAMETERS:	  size - additional amount of storage required for a table     *
*			 or string.					       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   mem_start - start of table & str. memory, initialized here.  *
*									       *
*     REFERENCED: mem_used - Total amount of memory used by the odm table and  *
*		  by the stored strings.				       *
*									       *
* RETURN VALUES:  success:     number of byte requested and allocated          *
*                 failure:     (error message and exit by MALLOC or REALLOC)   *
*									       *
*******************************************************************************/

static int
str_alloc (mem_ushval_t size)
{
    static mem_ushval_t mem_size;       /* total size of memory malloced  */

    /* If memory to be allocated (totaled, so far) exceeds the max value the
       representation can support, exit */
    if (mem_used + size > SHRT_MAX)
    {
	char			tmpsizestr[TMPINTSTRSIZ + 1];

	(void) sprintf (tmpsizestr, "%d", SHRT_MAX);
	ERREXIT(0, MSG_DDISIZ_EXCEED, tmpsizestr, (char *) NULL, 0);
    }

    if ((char *) mem_start == NULL)
    {
	MALLOC(memptr.charptr, (size_t) size);      /* malloc initial memory */
	mem_start = memptr.intptr;
	(void) memset((char *) mem_start, '\0', (size_t) size);
						    /* clear memory */
	mem_size = mem_used = size;
    }
    else	/* malloc memory if needed, malloc in chunks of BUFSIZ */
	while ((size + mem_used) > mem_size)
	{
	    memptr.intptr = mem_start;
	    REALLOC(memptr.charptr, memptr.charptr,
		    (size_t) (mem_size + BUFSIZ));
	    mem_start = memptr.intptr;
	    (void) memset((char *) GET_OFFSET(mem_size), '\0', BUFSIZ);
	    mem_size += BUFSIZ;
	}
    return(size);
}



/*******************************************************************************
*									       *
* NAME: 	  hash_name						       *
*									       *
* DESCRIPTION:	  hash_name "hashes" an attribute name into a primary and in   *
*		  most cases, a secondary index table.	Since the first two    *
*		  characters of an attribute name, when combined, must be      *
*		  unique, they are used as keys into the index tables.	For    *
*		  example the primary index for attribute name "fl" would      *
*		  be at location "f" -"0" and would contain an offset to the   *
*		  beginning of the secondary hash table, in this instance      *
*		  the "f" table.  The index "l" - "0" in the "f" table	       *
*		  would contain the offset of the odm table record	       *
*		  associated with the name we just "hashed" into the tables.   *
*									       *
* PARAMETERS:	  offset - location of attribute record in odm table.	       *
*									       *
*		  name	 - name associated with attribute record.	       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   mem_used - current size of memory allocated for tables       *
*		  and strs.						       *
*									       *
*     REFERENCED:							       *
*									       *
* EXTERNALS:								       *
*		  GET_OFFSET() - calculate offset from mem_start.	       *
*		  GET_INDEX()  - calculate hash index of a character in an     *
*				 attribute name.			       *
* RETURN VALUES:  success:      hash table entry                               *
*                 comment only: 1                                              *
*                 failure:      (error message and exit by ERREXIT)            *
*                                                                              *
*******************************************************************************/

static int
hash_name (mem_ushval_t offset, char *name)
{
    char *name_p = name; /* name pointer */

    int align_int;	/* Used to align hash tables on an integer boundry*/
    mem_shval_t *hash_tab_p;	/* Pointer to primary hash table */
    mem_ushval_t *hash_tab_p2;	/* Pointer to secondary hash table */
    mem_ushval_t index_ht;	/* index into hash table */
    mem_ushval_t tmp_offset;	/* index into odm table */

    struct odm *odm_tab_p;	/* odm table pointer */
    void *tmp_vp;		/* temp. generic pointer */

    index_ht = GET_INDEX(*name_p++);
    memptr.charptr =
	(char *) (mem_start + 1) + (objtot * sizeof(struct odm)) + index_ht;
    hash_tab_p = memptr.shptr;      /* convert (char *) to (short *) */

    /*  The name to be "hashed" is the first of its kind so we stash the
	odm table offset value (offset) in the primary lookup table.  In
	other words the first character of the name is the first occurance
	of that character to date. Since we dont want to create a table for
	only one occurance of a particular type of name we store it in the
	primary table. We make the value negative so that it can be
	distinguished from a value that references a secondary table.
    */
    if (*hash_tab_p == 0)
	*hash_tab_p = -offset;
    else
    {
	/* This is where the fun starts.  The first character of the name
	   to be "hashed" has occured once before, so now we must retrieve 
	   the odm table index value that was stored in reverse sign format
	   in the primary table.  To get the name associated with the value
	   we are moving we must first calculate the odm table offset 
	   represented by odm_tab_p.  Next we calculate the index into the
	   secondary table.  Since we are are going to create a table of 
	   integers we must be sure that the secondary table starts on an
	   integer bounty (thus the modulus operation).  Now we create a 
	   secondary table to hold the present and future ocurrances of such
	   names with the same first character; Then we move the value
	   currently residing in the primary table to its proper slot in the
	   secondary table.
	*/
	if (*hash_tab_p < 0)
	{
	    /* Is the name a duplicate?? */
	    memptr.intptr = mem_start;           /* start of memory */
	    memptr.charptr += (-(*hash_tab_p));  /* addr of odm tab entry */
	    odm_tab_p = memptr.odmptr;  /* convert to (struct odm *) */
	    if (!strncmp(name, odm_tab_p->name, ATTRNAMELEN))
		ERREXIT(0, MSG_DUP_ATTR, name, colonfilename, 0);

	    tmp_offset = -(*hash_tab_p);	/* retrieve odm table index */
	    index_ht = GET_INDEX(odm_tab_p->name[1]);
	    align_int = (sizeof(int)) - mem_used % sizeof(int);
	    *hash_tab_p = mem_used + align_int;

	    (void) str_alloc((mem_ushval_t) (HASH_TAB_SIZ + align_int));

	    /* refresh hash_tab_p in case str_alloc() did realloc() */
	    memptr.charptr = (char *) (mem_start + 1)
		+ (objtot * sizeof(struct odm))
		+ GET_INDEX(name[0]);
	    hash_tab_p = memptr.shptr;  /* convert (char *) to (short *) */

	    mem_used += (HASH_TAB_SIZ + align_int);
            tmp_vp = GET_OFFSET(*hash_tab_p + index_ht);
            hash_tab_p2  = *(mem_ushval_t **) &tmp_vp;
            *hash_tab_p2 = tmp_offset;
	}
	/* Here we "hash" the second character of the name and store the
	   offset to the odm table entry that contains the information
	   associated with the name we just "hashed" in the resulting
	   location.
	*/
	index_ht = GET_INDEX(*name_p);
	tmp_vp  = GET_OFFSET(*hash_tab_p + index_ht);
	hash_tab_p2  = *(mem_ushval_t **) &tmp_vp;

	/* Is the name a duplicate ?? */
	if (*hash_tab_p2 != 0)
	    ERREXIT(0, MSG_DUP_ATTR, name, colonfilename, 0);

	*hash_tab_p2 =  offset;
    }
    /* If the name is valid then store the name in the odm table attribute
       record */
    if (*hash_tab_p)
    {
	tmp_vp = GET_OFFSET(offset);
	odm_tab_p = *(struct odm **) &tmp_vp;
	odm_tab_p->name[0] = name[0]; odm_tab_p->name[1] = name[1];
    }
    return((int) *hash_tab_p);
}



/*******************************************************************************
*									       *
* NAME: 	  validate_attrvallims					       *
*									       *
* DESCRIPTION:	  "validate_attrvallims" function validates values of data     *
*                 base attribute value strings and limits that are stored in   *
*                 the memory along with odm table, primary and seconday hash   *
*                 index tables.                                                *
*                 Validation includes reference-checking (of referred          *
*                 attributes) and syntax verification.                         *
*                 It also includes attribute value validation against its      *
*                 limit string.                                                *
*									       *
* PARAMETERS:	  noofobjs - no of odm objects to be validated.		       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*									       *
*     REFERENCED: 							       *
*                 mem_start - address of the memory image.                     *
*                 odm_table - address of the odm table.                        *
*									       *
* EXTERNALS:								       *
*		  GET_OFFSET() - calculate offset from mem_start.	       *
*									       *
* RETURN VALUES:  success:      TRUE		                               *
*                 failure:      FALSE (warning message[s])	               *
*                                                                              *
*******************************************************************************/

static int
validate_attrvallims (register unsigned int noofobjs)
{
   register struct odm		*tmpodmp =
				   *(struct odm **) (void **) &odm_table;
				   /* Temporary ODM pointer to an odm record */
   register char		*tmpstrp;
				   /* Temporary string ptr to an attr value */
   register int			noerrorsflag 	= TRUE;
				   /* Flag for errors not found */
   int				verifysyntaxflag = TRUE;
				   /* Flag for performing syntax checks */
   const int			doonce_flag = FALSE;
				   /* Flag for 'do' loop control for once */
   char				attrerrflag;
				   /* flag to track errors for a single attr */

   /* Open a temp. file and if failure, set a flag such that verification of
      syntax is not performed on the attribute values */
   if (!(out = tmpfile ()))
      verifysyntaxflag = FALSE;
   else
   {
      out_fd = fileno (out);
      (void) setbuf (out, (char *) NULL);

      /* Initialize a few variables that may be used in "yyparse" function
	 subsequently.
       */
      *attributes = 0, cat_name_mD = (char *) NULL, indent = ATTR_INDENT;
   }

   /* Perform verification and validation on each attribute value string
      and limit string stored in the memory */
   for (tmpodmp = *(struct odm **) (void **) &odm_table;
	noofobjs;
	noofobjs--, tmpodmp++)
   {
      attrerrflag = FALSE;

      /***********************************************************************/
      /*		Attribute value string validation		     */
      /***********************************************************************/

      do
      {
         tmpstrp = (char *) GET_OFFSET (tmpodmp->str);

         /* Strip off any non-printable characters (including NUL chars) before
	    further validation */
         tmpstrp = strip_nonprt_attrvallim ((const char *) tmpstrp,
					    (unsigned int) tmpodmp->length);

         /* Look for a '%' character in the value string.  If there is none,
	    skip to attribute limit, since there is no necessity of
	    validation in that case. */
         if (!strchr (tmpstrp, ESCCHAR))		/* '%' not found */
	    break;				/* validation neednt be done */

         /* Verify the syntax of the attribute value string, in case flag for
	    checking syntax is true. If there are any errors, display the errors
	    and skip any further validation and skip to attr limit.
	    Else, continue to validate the value string by checking for any
	    referenced attribute names. */
         if (verifysyntaxflag  &&
	     (verify_syntax_attrvallim ((const char *) tmpodmp->name,
				        (const char *) tmpstrp, TRUE) ?
	      FALSE :			/* do not skip next validation */
	      (attrerrflag = !(noerrorsflag = FALSE), TRUE)))
					/* skip next validation */
	    break;

         /* Verify references embedded in the attribute value string.  If there
	    are any errors, display them. */
         if (!verify_refers_attrvallim ((const char *) tmpodmp->name,
				        (const char *) tmpstrp, TRUE))
	    attrerrflag = !(noerrorsflag = FALSE);
      } while (doonce_flag);		/* just one iteration - for control breaks */

      if (attrerrflag)
	 continue;


      /***********************************************************************/
      /*		Attribute limit string validation		     */
      /***********************************************************************/

      do
      {
         /* Locate the attribute limit string.  Note that attribute limit string
	    is stored right next to the attribute value string. */
         tmpstrp = GET_ATTR_LIMSTR (tmpodmp);

         /* Strip off any non-printable characters (including NUL chars) before
	    further validation */
         tmpstrp = strip_nonprt_attrvallim ((const char *) tmpstrp,
				            (unsigned int) tmpodmp->limlength);

         /* Look for a '%' character in the value string.  If there is none,
	    skip to limit validation, since there is no necessity of this
	    validation in that case. */
         if (!strchr (tmpstrp, ESCCHAR))		/* '%' not found */
	    break;				/* validation neednt be done */

         /* Verify the syntax of the attribute limit string, in case flag for
	    checking syntax is true.  If there are any errors, display the
	    errors and skip any further validation and continue to limit
	    validation.  Else, continue to validate the limit string by
	    checking for any referenced attribute names. */
         if (verifysyntaxflag  &&
	     (verify_syntax_attrvallim ((const char *) tmpodmp->name,
				        (const char *) tmpstrp, FALSE) ?
	      FALSE :			/* do not skip next validation */
	      (attrerrflag = !(noerrorsflag = FALSE), TRUE)))
					/* skip next validation */
	    break;

         /* Verify references embedded in the attribute value string.  If there
	    are any errors, display them. */
         if (!verify_refers_attrvallim ((const char *) tmpodmp->name,
				        (const char *) tmpstrp, FALSE))
	    attrerrflag = !(noerrorsflag = FALSE);
      } while (doonce_flag);	/* just one iteration - for control breaks */

      if (attrerrflag)
	 continue;


      /***********************************************************************/
      /*		Attribute limit vs value validation		     */
      /***********************************************************************/

      /* Perform limit validation by verifying that the attribute value string
	 is valid as per the limits specified in the attribute limit string */
      if (tmpstrp = piovalav ((const char *) tmpodmp->name))
      {
	 ERRDISP (0, MSG_DUMMY, tmpstrp, (char *) NULL, 0);
	 noerrorsflag = FALSE;
	 errtot++;
      }
   }			/* end - for */

   /* If errors found (in either syntax verification or reference validation),
      display a generic problem-fix exhorting message */
   if (!noerrorsflag)
      ERRDISP (0, MSG_ERR_URGEFIX, (char *) NULL, (char *) NULL, (int) errtot);

   return noerrorsflag;
}		/* function - validate_attrvallims */




/*******************************************************************************
*									       *
* NAME: 	  verify_refers_attrvallim				       *
*									       *
* DESCRIPTION:	  "verify_refers_attrvallim" function verifies references      *
*                 embedded in passed strings (attribute values or limits).     *
*                 The strings are parsed to see if any references are          *
*		  embedded that point to other attribute values.  This is      *
*		  done by looking for the escape character '%' and the         *
*		  referencing command character 			       *
*		  ('I', 'G', 'S', 'L', 'D', '`', 'i', 'C', 'F', 'f', '#')      *
*		  followed by the attribute name(s) that are referred to.      *
*		  ( The escape sequences considered are:		       *
*		  		%Ixx					       *
*				%I[xx,yy,zz,...]			       *
*				%Gxx					       *
*				%Sxx					       *
*				%Lxx					       *
*				%Dxx					       *
*				%`xx					       *
*				%ix					       *
*				%i!					       *
*				%Cy					       *
*				%Fxy					       *
*				%F[abc...]				       *
*				%fxy					       *
*				%f[abc...] )				       *
*				%#xx"..."                                      *
*		  Validation is then performed on the referred attribute       *
*		  names by checking to see if they are already stored in the   *
*		  memory (that is, defined in the input colon file).  In cases *
*		  of unsuccessful queries, warning messages are displayed      *
*		  indicating that the input colon file has incorrect or/and    *
*		  incomplete entries.					       *
* PARAMETERS:	  attrnm - name of the attribute to be verified		       *
*		  attrvallim - attribute value/limit string to be verified.    *
*                 attrvalflag - flag to denote whether attribute value is      *
*                               to be verified or not                          *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*									       *
*     REFERENCED: 							       *
*									       *
* EXTERNALS:								       *
*									       *
* RETURN VALUES:  success:      TRUE		                               *
*                 failure:      FALSE (reference error message[s])             *
*                                                                              *
*******************************************************************************/

static int
verify_refers_attrvallim (const char *attname, const char *attrvallim,
		          int attrvalflag)
{
#define IF_INCLEN_DISPERR_BLOCK \
	    { \
	       if (strlen (attrnm) != ATTRNAMELEN) \
					/* if could not get an attrname */ \
	       { \
	          ERRDISP (0, attrvalflag ? MSG_ATTR_INCATTRLEN : \
			   MSG_ATTRLMT_INCATTRLEN, attname, attrnm, 0); \
	          noerrorsflag = cparseflag = FALSE, errtot++; \
		  continue;		/* continue to parse loop */ \
	       } \
	    }

/* Possible C Compiler ERROR here !!!???
   The error shows up with the conditional operator ?: with the third
   operand (if it involves pointer arithmetic).  Surpisingly, it shows
   up only in certain circumstances.
   Anyway, to circumvent the compiler error, a workaround is incorporated
   down below.
#define IF_ODMNOTFOUND_DISPERR_BLOCK \
	    { \
	       if (!get_odmtab_ent (attrnm))		|* odm not found *| \
	       { \
		  tmplen = sizeof (attropstr) - 1 < tmpstrp - tmpattropstrp ? \
			   sizeof (attropstr) - 1 : tmpstrp - tmpattropstrp; \
		  (void) strncpy (attropstr, tmpattropstrp, tmplen), \
		  attropstr[tmplen] = 0; \
                  ERRDISP (0, attrvalflag ? MSG_ATTR_ATTRNOTFOUND : \
			   MSG_ATTRLMT_ATTRNOTFOUND, attname, attropstr, 0); \
		  noerrorsflag = FALSE, errtot++; \
	       } \
	    }
 */
#define IF_ODMNOTFOUND_DISPERR_BLOCK \
	    { \
	       if (!get_odmtab_ent (attrnm))		/* odm not found */ \
	       { \
		  size_t		tslen = tmpstrp - tmpattropstrp; \
		  \
		  tmplen = sizeof (attropstr) - 1 < tslen ? \
			   sizeof (attropstr) - 1 : tslen; \
		  (void) strncpy (attropstr, tmpattropstrp, tmplen), \
		  attropstr[tmplen] = 0; \
                  ERRDISP (0, attrvalflag ? MSG_ATTR_ATTRNOTFOUND : \
			   MSG_ATTRLMT_ATTRNOTFOUND, attname, attropstr, 0); \
		  noerrorsflag = FALSE, errtot++; \
	       } \
	    }

   register const char		*tmpstrp 	= attrvallim;
				   /* Temporary string ptr to an attr value */
   register int			noerrorsflag 	= TRUE;
				   /* Flag for errors not found */
   char				attrnm[ATTRNAMELEN + 1];
				   /* Local charray for each parsed attrname */
   register int			cparseflag;
				   /* Flag for whether to coninue parsing */
   char				attropstr[MAX_LONGCHAR_LENGTH];
				   /* Attribute value operator string */
   char				*tmpattropstrp;
				   /* Temp. attribute value operator str ptr */
   register size_t		tmplen;
				   /* Temp. attribute value operator str len */
   

   cparseflag = TRUE;
   while (cparseflag  &&			/* if continue parsing */
	  (tmpstrp = tmpattropstrp
	   = strchr (tmpstrp, ESCCHAR)))	/* if '%' char found */
   {
      switch (*++tmpstrp)
      {
         case ESCCHAR:			/* a '%' after '%' */
            tmpstrp++;
            continue;			/* continue looking for '%' */

         case 'I':
            if (*++tmpstrp == SETBEGINCHAR) /* if '[', multiple attributes */
            {
	       char			*tmpstr1p;
						/* Tmp ptr to attr val string */
	       size_t		tmpattrlen;
						/* Temp. attr name length */

	       if (!(tmpstr1p = strchr (++tmpstrp, SETENDCHAR)))
						/* if no matching ']', error
						   do not parse any further */
	       {
                  ERRDISP (0, attrvalflag ? MSG_ATTR_ESETNOMATCH :
			   MSG_ATTRLMT_ESETNOMATCH, attname,
			   attrvallim, 0);
                  noerrorsflag = cparseflag = FALSE, errtot++;
	          continue;			/* continue to parse loop */
	       }

	       /* Now, parse the set of attribute(s) between '[' and ']'
	          and check each attribute */
	       while (TRUE)
	       {
	          if ((tmpattrlen = strcspn (tmpstrp, SETDELIMITCHARS)) !=
		      ATTRNAMELEN)
					/* improper attrname - wrong len */
					/* skip the rest of attr names
					   in this set */
	          {
   		     char 	tmpattrstr[MAXWORK_ATTRNAMELEN + 1];
					/* Tmp attrval str to be displayed  */
		     size_t	worklen;

		     (void) strncpy (tmpattrstr, tmpstrp,
				     worklen = tmpattrlen >
				               sizeof (tmpattrstr) - 1 ?
				               sizeof (tmpattrstr) - 1 :
					       tmpattrlen),
		     tmpattrstr[worklen] = 0;
                     ERRDISP (0, attrvalflag ? MSG_ATTR_INCATTRLEN :
			      MSG_ATTRLMT_INCATTRLEN, attname,
			      tmpattrstr, 0);
                     noerrorsflag = FALSE, errtot++;
		     tmpstrp = tmpstr1p + 1;
	             break;			/* break - while TRUE */
	          }

                  (void) strncpy (attrnm, tmpstrp, ATTRNAMELEN),
                  *(attrnm + ATTRNAMELEN) = 0;
                  tmpstrp += tmpattrlen;

                  /* Try to get ODM record for the just parsed attr name.
                     If found, it is a success; continue parsing.
	             Else, log a failure warning, but continue parsing. */
	          IF_ODMNOTFOUND_DISPERR_BLOCK;

	          /* If this is the last attr. name in the set, then break */
	          if (*tmpstrp++ == SETENDCHAR)
	          {
	             break;			/* break - while TRUE */
	          }
	       }			/* end - while TRUE */

	       break;		/* break - case "I[...]" */
            }
            else				/* a single attribute */
            {
						/* Since the logic is the same
						   for all cases with a single
						   attribute, fall thruough to
						   next cases */
	       tmpstrp--;
            }

         case 'G':
         case 'S':
         case 'L':
         case 'D':
         case '#':
            (void) strncpy (attrnm, ++tmpstrp, ATTRNAMELEN),
            attrnm[sizeof (attrnm) - 1] = 0;
            IF_INCLEN_DISPERR_BLOCK;
            tmpstrp += ATTRNAMELEN;

            /* Try to get ODM record for the just parsed attribute name.
               If found, it is a success; continue parsing.
	       Else, log a failure warning, but continue parsing. */
            IF_ODMNOTFOUND_DISPERR_BLOCK;

            break;		/* break - case 'I' 'G' 'S' 'L' 'D' */

         case '`':
            /* "%`" operator supports two syntaxes: %`xx or %`"..."
	       For the latter syntax, skip the following verification. */
            if (*++tmpstrp == '"')
               break;		/* break - case '`' */

            (void) strncpy (attrnm, tmpstrp, ATTRNAMELEN),
            attrnm[sizeof (attrnm) - 1] = 0;
            IF_INCLEN_DISPERR_BLOCK;
            tmpstrp += ATTRNAMELEN;

            /* Try to get ODM record for the just parsed attribute name.
               If found, it is a success; continue parsing.
	       Else, log a failure warning, but continue parsing. */
            IF_ODMNOTFOUND_DISPERR_BLOCK;

            break;		/* break - case '`' */

         case 'i':
            *attrnm = 'i';
            if ((*(attrnm + 1) = *++tmpstrp) == '!')  /* if "%i!", skip */
            {
	       tmpstrp++;
	       continue;			/* continue to parse loop */
            }
            attrnm[sizeof (attrnm) - 1] = 0;
            IF_INCLEN_DISPERR_BLOCK;
            tmpstrp++;

            /* Try to get ODM record for the just parsed attribute name.
               If found, it is a success; continue parsing.
	       Else, log a failure warning, but continue parsing. */
            IF_ODMNOTFOUND_DISPERR_BLOCK;

	       break;		/* break - case 'i' */

         case 'C':
            *attrnm = '_';
            *(attrnm + 1) = *++tmpstrp;
            attrnm[sizeof (attrnm) - 1] = 0;
            IF_INCLEN_DISPERR_BLOCK;
            tmpstrp++;

            /* Try to get ODM record for the just parsed attribute name.
               If found, it is a success; continue parsing.
	       Else, log a failure warning, but continue parsing. */
            IF_ODMNOTFOUND_DISPERR_BLOCK;

            break;		/* break - case 'C' */

         case 'F':
         case 'f':
            if (*++tmpstrp == SETBEGINCHAR)	/* if '[', multiple flags */
            {
	       if (!strchr (++tmpstrp, SETENDCHAR))
						/* if no matching ']', error
						   do not parse any further */
	       {
                  ERRDISP (0, attrvalflag ? MSG_ATTR_ESETNOMATCH :
			   MSG_ATTRLMT_ESETNOMATCH, attname,
			   attrvallim, 0);
                  noerrorsflag = cparseflag = FALSE, errtot++;
	          continue;			/* continue to parse loop */
	       }

	       /* Now, parse the set of flags(s) between '[' and ']'
	          and check each flag */
	       while (*tmpstrp != SETENDCHAR)
	       {
                  *attrnm = '_';
                  *(attrnm + 1) = *tmpstrp;
                  attrnm[sizeof (attrnm) - 1] = 0;
                  tmpstrp++;

                  /* Try to get ODM record for the just parsed attr name.
                     If found, it is a success; continue parsing.
	             Else, log a failure warning, but continue parsing. */
	          IF_ODMNOTFOUND_DISPERR_BLOCK;
	       }			/* end - while TRUE */
	       tmpstrp++;
            }
            else				/* a single flag */
            {
               (void) strncpy (attrnm, tmpstrp, ATTRNAMELEN),
               attrnm[sizeof (attrnm) - 1] = 0;
               IF_INCLEN_DISPERR_BLOCK;
	       *attrnm = '_';	/* replace 'x' (from "%Fxy") with '_' */
               tmpstrp += ATTRNAMELEN;

               /* Try to get ODM record for the just parsed attribute name.
                  If found, it is a success; continue parsing.
	          Else, log a failure warning, but continue parsing. */
               IF_ODMNOTFOUND_DISPERR_BLOCK;
            }

            break;		/* break - case 'F' "F[...]" 'f' "f[...]" */

         default:
            /* Could be a valid escape command sequence other than the above
	       processed commands (like '%d', '%Px', '%x', '%+', '%^', etc.),
	       or could be a invalid escape command sequence (like '%Qx').
	       Anyway, we're not attempting to validate the entire escape
	       command sequence here.  Hence, let's continue parsing the
	       attribute value string for other possible command sequences
	       that are processed here (in this module). */
            break;
      }		/* end - switch */
   }			/* end - while */

   return noerrorsflag;

#undef IF_INCLEN_DISPERR_BLOCK
#undef IF_ODMNOTFOUND_DISPERR_BLOCK
}		/* function - verify_refers_attrvallim */




/*******************************************************************************
*									       *
* NAME: 	  verify_syntax_attrvallim				       *
*									       *
* DESCRIPTION:	  "verify_syntax_attrvallim" function validates syntax of a    *
*		  given attribute value string.  This is done by invoking      *
*		  "yyparse" function (that is the result of running "lex" and  *
*		  "yacc" on the files included along with the source files).   *
*		  Typically this operation is done by running the make file.   *
*		  Also this function initializes a few flags and counters      *
*		  before it invokes "yyparse".				       *
*		  Also since "yyparse" function calls "show_op" and 	       *
*		  "find_desc", these functions are defined subsequently.  And  *
*		  the output messages generated by the "show_op" function is   *
*		  stored in the file pointed by "out".  Hence, in case of      *
*		  errors, after "yyparse" returns, this file is rewound, and   *
*		  the contents are flushed to stderr with some headers.	       *
*									       *
* PARAMETERS:	  attrnm - name of the attribute to be verified		       *
*		  attrvallim - attribute value/limit string to be verified.    *
*                 attrvalflag - flag to denote whether attribute value is      *
*                               to be verified or not                          *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*		inptr		- input pointer into the given attr value str  *
*		unptr		- uninput pointer (for ungets) 		       *
*		unbfr		- uninput buffer (for ungets)		       *
*		disp		- display flag (TRUE/FALSE) for msgs and errs  *
*		if_while	- flag for if/while stmt errors		       *
*		synt_err	- flag for syntax errors		       *
*									       *
*     REFERENCED: 							       *
*		out		- File ptr to output from yyparse.             *
*									       *
* EXTERNALS:								       *
*									       *
* RETURN VALUES:  success:      TRUE		                               *
*                 failure:      FALSE (syntax error message[s])	               *
*                                                                              *
*******************************************************************************/

static int
verify_syntax_attrvallim (const char *attrnm, const char *attrvallim,
		          int attrvalflag)
{
   register int			noerrorsflag 	= TRUE;
					/* Flag for errors not found */
   register size_t		errmsgsize;
					/* Size of the error msgs output */
   register char		*terrmsg;
					/* Temp error msg pointer */

   /* Initialize a few global flags and counters */
   inptr = (char *) attrvallim;
   unptr = unbfr, *unptr = 0;
   disp = disperronly = TRUE; parseerr = FALSE;
   if_while = synt_err = 0;

   /* Parse the attribute value string and output syntax errors(if any) to
      "out" */
   (void) yyparse (), (void) fflush (out);

   /* If any errors found, output the error messages from "out" file to
      stderr */
   if (if_while)
   {
      register char	*tmperrmsg =  getmsg (OP_CATALOG, 5, OP_BAD_IF_WHILE);
      char		*tmp1errmsg = strdup (tmperrmsg);
      register char	*tmpstrp = tmp1errmsg;
      register int	prev_char_perc = FALSE;

      /* Remove extra '%' chars ('%' out of '%%') from the error msg */
      if (!tmp1errmsg)			/* Memory could not be allocated */
	 tmp1errmsg = tmperrmsg;	/* No removal! */
      else
      {
	 for ( ; *tmperrmsg; tmperrmsg++)
	    if (*tmperrmsg == ESCCHAR)
	    {
	       if (prev_char_perc)	    /* if previous char is also a '%' */
		  prev_char_perc = FALSE;   /* no copying this second char */
	       else
	       {
		  prev_char_perc = TRUE;
		  *tmpstrp++ = *tmperrmsg;
	       }
	    }
	    else
	    {
	       *tmpstrp++ = *tmperrmsg;
	       prev_char_perc = FALSE;
	    }
	  *tmpstrp = 0;
      }					/* end - !tmp1errmsg */

      noerrorsflag = FALSE, errtot++;
      ERRDISP (0, attrvalflag ? MSG_ATTR_SYNTERR : MSG_ATTRLMT_SYNTERR,
	       attrnm, tmp1errmsg, 0);
   }
   if (synt_err)
   {
      noerrorsflag = FALSE, errtot += synt_err;
      errmsgsize = (size_t) ftell (out), rewind (out);
      MALLOC (terrmsg, errmsgsize + 1);
      *(terrmsg + errmsgsize) = 0;
      if (read (out_fd, terrmsg, errmsgsize) == -1)
      {
	 char		tmpsizestr[TMPINTSTRSIZ + 1];

	 (void) sprintf (tmpsizestr, "%d", errmsgsize);
	 ERREXIT (0, MSG_READ_TMPFILE, tmpsizestr, (char *) NULL, errno);
      }
      ERRDISP (0, attrvalflag ? MSG_ATTR_SYNTERR : MSG_ATTRLMT_SYNTERR,
	       attrnm, terrmsg, 0);
      rewind (out);
   }

   return noerrorsflag;
}		/* function - verify_syntax_attrvallim */




/*******************************************************************************
*									       *
* NAME: 	  show_op						       *
*									       *
* DESCRIPTION:	  "show_op" displays the attribute string and description      *
*		  string.  Also prints the argument string (or character)      *
*		  passed in into the description string read from the 	       *
*		  description catalog.
*									       *
* PARAMETERS:	  msg_num - no of the msg to be fetched from the catalog       *
*		  oper - operator string in the attribute value		       *
*		  type - type of the operator field			       *
*		  str - string to be used after getting the specified msg      *
*		  chr - char to be used after getting the specified msg        *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*									       *
*     REFERENCED: 							       *
*		out		- File ptr to output from yyparse.             *
*									       *
* EXTERNALS:								       *
*									       *
* RETURN VALUES:  (none)			                               *
*                                                                              *
*******************************************************************************/

void
show_op (int msg_num, char *oper, int type, char *str, int chr)
{
   char desc[MAXDESCLEN];

   if ( msg_num)
   {
      if (type == NO_FIELD)
         (void) strcpy (desc, getmsg (OP_CATALOG, 5, msg_num));
      if (type == STR_FIELD)
         (void) sprintf (desc, getmsg (OP_CATALOG, 5, msg_num), str);
      if (type == CHR_FIELD)
         (void) sprintf (desc, getmsg (OP_CATALOG, 5, msg_num), (char) chr);
      if (type == NUM_FIELD)
         (void) sprintf (desc, getmsg (OP_CATALOG, 5, msg_num), chr);
   }
   else
      *desc = 0;

   /* Output the messages to "out".  Since we're dealing with only error
      messages here, need to perform extra processing can be obviated.
      Hence, just concatenate the attribute(cum-operator) and description.
      However, perform this output generation in our case, only when
      a syntax error is detected (with "disperronly" flag being set already).
    */
   if (!disperronly  ||  parseerr)
   {
      (void) fprintf (out, "%*s%s%s\n", indent, " ", oper, desc);
   }
   parseerr = FALSE;
   
}		/* function - show_op */




/*******************************************************************************
*									       *
* NAME: 	  find_desc						       *
*									       *
* DESCRIPTION:	  "find_desc" function finds the description from the 	       *
*		  pioattrX.cat message catalog for a given attribute. If       *
*		  not found, the 'attr not found' message is displayed.        *
*									       *
* PARAMETERS:	  attrnm - name of the attribute 			       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*									       *
*     REFERENCED: 							       *
*									       *
* EXTERNALS:								       *
*									       *
* RETURN VALUES:  success:      pointer to description                         *
*                 failure:      NULL pointer			               *
*                                                                              *
*******************************************************************************/

char *
find_desc (char *attrnm)		/*ARGSUSED*/
{
   /* We're dealing with error messages only.  Hence there is no necessity to
      fetch the attribute description from the pioattrx.cat message catalog.
    */
   return (char *) NULL;
}		/* function - find_desc */




/*******************************************************************************
*									       *
* NAME: 	  strip_nonprt_attrvallim				       *
*									       *
* DESCRIPTION:	  "strip_nonprt_attrvallim" function strips any non-printable  *
*		  characters from the given attribute value string of given    *
*		  length.						       *
*		  This operation is done to curcumvent a glitch that shows up  *
*		  in "yyparse" (YACC code) function which assumes that it has  *
*		  reached the end of data, when it encounters a NUL (^A) char, *
*		  which any generic C function does.			       *
*									       *
* PARAMETERS:	  attrval - attribute value string 			       *
*		  avlen   - length of given attribute value string	       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   							       *
*									       *
*     REFERENCED: 							       *
*									       *
* EXTERNALS:								       *
*									       *
* RETURN VALUES:  success:      pointer to description                         *
*                 failure:      NULL pointer			               *
*                                                                              *
*******************************************************************************/

static char *
strip_nonprt_attrvallim (register const char *attrval,
			 register unsigned int avlen)
{
   static char			resattrval[MAX_LONGCHAR_LENGTH];
   register char		*tmpstrp = resattrval;

   if (avlen > sizeof (resattrval) - 1)
      avlen = sizeof (resattrval) - 1;

   (void) memset (resattrval, 0, sizeof (resattrval));

   for ( ;avlen--; attrval++)
      if (isprint ((int) *attrval))
	 *tmpstrp++ = *attrval;
   *tmpstrp = 0;

   return resattrval;
}		/* function - strip_nonprt_attrvallim */

