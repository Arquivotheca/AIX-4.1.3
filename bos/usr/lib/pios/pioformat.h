/* @(#)32	1.7.1.2  src/bos/usr/lib/pios/pioformat.h, cmdpios, bos411, 9428A410j 7/23/93 16:22:59 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
*  pioformat.h - definitions used internally by the printer backend            *
*******************************************************************************/

		/* PREFORMATTER (PIOFORMAT)  STRUCTURES AND DEFINES */

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <IN/stfile.h>
#include "piostruct.h"
#include <piobe_msg.h>

#define NOPRINT	 0
#define PRINT	 1
#define EXITGOOD 0

extern char msgbuf[];   /* buffer in which to build message text */
extern struct err_info errinfo; /* error information */
extern int optopt;      /* last flag processed by piogetarg() */
extern int statusfile;  /* TRUE if we're running under the spooler */

/* Define typedefs for integer values to be stored in digested memory */
typedef unsigned short		mem_ushval_t;
					/* this type applies to values like
					   offset of attribute value string in
					   ODM record, length of attribute value
					   string in ODM record, secondary hash
					   table record denoting ODM offset,
					   length of attribute limits string
					   in ODM record */
typedef short			mem_shval_t;
					/* this type applies to primary hash
					   table record denoting ODM offset */

/* Data type for constant string.  Should not be allowed in attrparms.datatype
   from formatter to piogetvals() since (1) is not needed; formatter should
   use piogetstr() or piocmdout() to access data base strings to ensure that
   % commands are resolved, and (2) a str_info returned by piogetvals() for
   a constant string may later become invalid if the constant string is later
   converted to a variable integer or string by a call to piogetvals() or a
   call to piogetopt(). */

#define ATTRNAMELEN 2		/* max attribute name length */
#define CONST_STR 3             /* data type for constant string */
#define ODMFILE_VERSION 	00001	/* ODM file version no */
#define ODMFILE_HEADER		"PIODIGESTED"	/* ODM file header label */
#define STRGZ(str)		#str		/* stringize macro */
#define EXP_STRGZ(mcr)		STRGZ(mcr)	/* stringize expanded macro */
#define ODMFILE_HEADER_TEXT	ODMFILE_HEADER EXP_STRGZ (ODMFILE_VERSION)
						/* complete ODM file header text */
#define LIMITDENOTER		'#'

/* Attribute Names Referenced In the ODM data base by */
/* internal logic (i.e., other than the formatters) */

#define PRINTERTYPE     "mt"    /* printer type */
#define OUT_DATASTREAM  "md"    /* output data stream type */
#define DEVICENAME      "mn"    /* device name */
#define QUEUENAME       "mq"    /* print queue name */
#define VIRTUALPTRNAME  "mv"    /* virtual printer (queue device) name */
#define DB_FILENAME     "mm"    /* file name for (digested) data base info. */
#define DEFAULTFMTR     "mf"    /* default formatter (for standalone) */
#define DB_READROUTINE  "mr"    /* routine to read printer */
#define DB_FORMFEED     "af"    /* form feed string */
#define DB_CANCELSTRING "mc"    /* string to send to printer if cancelled */
#define DB_NUM_CAN_STRS "mz"    /* number of times to send cancel string */
#define PRINTERDESC     "mL"    /* printer description                     */
#define DATASTRDESC     "mA"    /* data stream description                 */
#define DB_INT_REQ_USER "si"    /* user to send intervention req'd msgs to */
#define PREVIEWLEVEL    "_a"    /* 0: normal print.                        */
				/* 1: return attribute values and          */
				/*    document pipeline, but DON'T PRINT   */
#define DIAGLEVEL       "_A"    /* 0: stderr output disabled.              */
				/* 1: return stderr & associated full      */
				/*    pipeline(s) if stderr is produced    */
				/* 2: always return attribute values,      */
				/*    full pipelines, and stderr (if any)  */
#define IN_DATASTREAM   "_d"    /* input data stream type                  */
#define FILTERNAME      "_f"    /* filter name (one character)             */
#define BURSTPAGE       "_B"    /* burst pages: 2 characters; each char.   */
                                /* is 'n' (never), 'a' (each file), or 'g' */
                                /* (each job):                             */
                                /*    first character: header page         */
                                /*    second character: trailer page       */
#define MAILONLY        "_C"    /* mail all msgs instead of displaying     */
#define PRINTQUEUE      "_P"    /* print queue name                        */
#define FLAGCHAR        "_"     /* first character of attribute name for   */
				/* flag values                             */
#define FILTERCHAR      "f"     /* first character of attribute name for   */
				/* filter command string                   */
#define PIPELINECHAR    "i"     /* first character of attribute name for   */
				/* pipeline for an input data stream       */
#define PROHIBITCHAR    "I"     /* first character of attribute name for   */
				/* list of flags prohibited for the input  */
				/* data stream named by second character   */
#define PROHIBITEDFLGS  "sp"    /* prohibited flags for ALL input data streams*/
#define ATTR_WIDTH      "sw"    /* width of attribute value area on header   */
				/* page (zero means ignore the width). Used  */
				/* to break attribute value list into lines. */
#define HDR_PIPELINE    "sh"    /* pipeline for header page */
#define TRLR_PIPELINE   "st"    /* pipeline for trailer page */
#define DDIF_CMDSTRING  "mo"    /* command string to invoke device driver */
				/* interface program (normally pioout)    */
#define COPIES_OPT      "mu"    /* how to produce multiple copies.  0:    */
				/* rerun pipeline;  1: reprint saved copy */
#define PS_STRING       "mp"    /* string that identifies a PostScript */
				/* data stream                         */
#define PS_DS_NAME      "mi"    /* input data stream name for PostScript */
				/* (one character)                       */
#define PS_DBFILE       "mb"    /* name of PostScript data base file */
				/* (if null, search for a file)      */
#define TEMPDIR         "dz"    /* directory for temporary files */
#define COPIES          "_N"    /* number of copies; actually processed by  */
				/* spooler, but simulated by print job mgr. */
				/* so can be prohibited by sys. administrator */
#define FF_AT_END	"_Z"	/* Issue Form Feed Between Copies & At Job */ 
				/* End (!:no;  +: yes) */
#define VALIDATE_AV_FLAG "zV"	/* attribute (as a flag) to indicate whether to
				   validate attribute value strings by
				   "piodigest" or not (!:no;  +: yes) */

/* Definitions for extra attributes in digested data base file for use by */
/* the Print Job Manager (piobe.c) & the Formatter Driver (pioformat.c)   */
#define NUM_EXTRA_ATTR  37      /* number of attributes (37 max.) */
#define EXTRA_ATTRNAME  "@ "    /* base attr name (2nd char is 0-9, A-B) */
#define NULLSTRING      "@0"    /* provides attribute name for null string */
#define PRINTFILE       "@1"    /* full path name of file being printed */
				/* (provided only to pipelines) */
#define PASSTHRUBYTES   "@2"    /* # of bytes that will be passed thru for %X */
				/* (provided only for piocmdout() output) */
#define INTERFACETYPE   "@3"    /* # of bytes that will be passed thru for %X */
				/* (provided only for piocmdout() output) */
#define BASE_DIR        "@4"    /* path for the base (i.e., "pio") directory */
#define BASE_VARDIR     "@5"    /* path for the PIOVAR  (i.e., "pio") directory */
#define DUMMY_ATTR	"@:"	/* dummy attr to be used in "pioparm" */
#define BYTESDONE       "@A"    /* number of bytes already printed */
#define TOTALBYTES      "@B"    /* total number of bytes to print */
#define NUM_CAN_STRS    "@C"    /* number of times to send cancel string */
#define CANCELSTRING    "@D"    /* string to send to printer if print job is */
				/* cancelled; in octal (\xxx\xxx...)         */
#define FF_STRING       "@F"    /* form feed string in octal (\xxx\xxx...) */
#define INT_REQ_USER    "@I"    /* user to send intervention req'd msgs to */
#define NUM_FF_STRS     "@N"    /* # of form feeds to output at EOF in stdin */
#define OUTFILE         "@O"    /* name of file to be generated by pioout in */
				/* which to store data instead of sending it */
				/* to the printer                            */
#define PREFIXFILE      "@P"    /* name of file (normally the header page) to */
				/* be sent to the printer before the first    */
				/* byte of the print data stream is sent      */
#define READROUTINE     "@R"    /* printer read routine */
#define SUFFIXFILE      "@S"    /* name of file (normally the trailer page) */
				/* to be sent to the printer after the last */
				/* byte of the print data stream is sent    */


/* Definitions for flag arguments representing "yes" and "no" */
#define YES_STRING  "+"
#define NO_STRING   "!"

#define DEFDSDIRECTIVE	"~~~"	/* directive in "mp" attribute that specifies
				   a default datastream */

#define MSGBUFSIZE  1000   /* size of buffer in which to build message text */

#define DEFBASEDIR   "/usr/lib/lpd/pio"      /* default base directory */
#define DEFVARDIR    "/var/spool/lpd/pio/@local"    /* default base directory */

/* offset in memory pointing to start of odm table */
#define ODM_TABLE    (mem_start + sizeof(int))

/* offset in memory to the start of the primary hash table */
#define HASH_TABLE   (ODM_TABLE + (objtot * sizeof(struct odm)))

/* size of individual hash tables */
#define HASH_TAB_SIZ (('{' - '0') * sizeof(mem_ushval_t))

/* calculates index into a hash table */
#define GET_INDEX(c) ((c - '0') * sizeof(mem_ushval_t))

/* calculates an offest from the start of memory */
#define GET_OFFSET(val) ((void *)((char *) mem_start + (val)))

/* data used for error messages. In the future, a pointer to this might    */
/* optionally be returned to the subroutine caller instead of issuing an   */
/* error message if the last argument to the subroutine is 1 instead of 0. */

struct err_info {       /* error information */
    int  errtype;           /* type of error */
    char title[S_TITLE];    /* job title */
    char progname[200];     /* program name ("piobe", "pioformat", formatter) */
    char subrname[14];      /* subroutine name (piogetvals, etc.) */
    char qname[14];         /* print queue name */
    char qdname[14];        /* queue device (virtual printer) name */
    char string1[200];      /* two-character attribute name */
    char string2[200];      /* two-character attribute name */
    int  integer;           /* more information about the error */
};

/* attribute data derived from the odm */
struct odm {
    mem_ushval_t  str;    /* offset (from memstart) to attribute value string */
    mem_ushval_t  length; /* length of the attribute value (string) */
    char name[ATTRNAMELEN]; /* attribute name as appears in devices and odm */
    mem_ushval_t  limlength; /* length of the attribute limits string */
};

/* Define a macro to access limit string of a given attribute.
   Note that attribute limit strings are stored right next to the attribute
   value strings. */
#define GET_ATTR_LIMSTR(odmptr)		((char *) GET_OFFSET \
					 (odmptr->str + odmptr->length))

/* integer information to be stored in the attribute table */
struct int_info {
    int value[PIO_NUM_MODES];   /* int attribute value (converted from string)*/
    struct lktable *lkuptabp; /* pointer to lookup table for ints that need */
};

/* This is the main dude structure, each attribute (structure) entry in the
   odm table will be converted into one of these structures by piogetvals() */
struct attr_info {
    union ptr_types
    {
        struct str_info *sinfo;	/* ptr to string data, could be an array */
        struct int_info *ivals; /* ptr to integer data */
    } ptypes;
    short datatype;             /* type of attribute data, see piostruct.h */
    short flags;                /* flag indicators */
#define ONCMDLINE      0x01         /* option flag specified on command line */
#define USEDBYSOMEONE  0x02         /* attribute value has been referenced */
#define USEDBYPIOBE    0x04         /* attribute value is used by piobe */
#define REFSFLAGARG    0x08         /* references one or more flag argument \
				       values, so needs to be rebuilt after \
				       command line flags are processed */
#define OVERRIDE       0x10         /* pioout default value overridden by   \
				       piobe */
#define NOARG          0x20         /* attribute for flag with no argument  \
				       (spooler flags only) */
};

struct attr_info *attr_table;   /* points to start of attribute table */

/* points to variables shared by preformatter and formatter */

#define sh_pl           *sh_vars->_pl        /* form length */
#define sh_tmarg        *sh_vars->_tmarg     /* top margin */
#define sh_bmarg        *sh_vars->_bmarg     /* bottom margin */
#define sh_vpos         *sh_vars->_vpos      /* vertical position on page */
#define sh_vtab_base    *sh_vars->_vtab_base /* base vpos for vert. tabs */
#define sh_vincr        *sh_vars->_vincr     /* vertical increment */
#define sh_vincr_cmd     sh_vars->_vincr_cmd /* vert. inc. command */
#define sh_vdecr        *sh_vars->_vdecr     /* vertical decrement */
#define sh_vdecr_cmd     sh_vars->_vdecr_cmd /* vert. decrement command */
#define sh_ff_cmd        sh_vars->_ff_cmd    /* form feed command */
#define sh_set_cmd       sh_vars->_set_cmd   /* mode switch command */
#define sh_ff_at_eof    *sh_vars->_ff_at_eof /* form feed at EOF on input? */

/* description of why we are trying to exit, i.e. an error description */
char exitbuf[BUFSIZ];

void 		 piomsgout(char *msgstr);

struct odm	 *get_odmtab_ent(char *attrnm);
struct attr_info *get_attrtab_ent(char *attrnm);
int		 piogenmsg(int msgnum, int sendmsg_flag);

int 		 pioparm(char *instring, FILE *fileptr,
			 char *getstrbuf, int getstrsiz,
			 int passthru, int id,
			 error_info_t *err_infop);
int 		 pioexit(int exitcode);

char *alloc_tables();

char errbuf[BUFSIZ];

#define MALLOC(memptr, size) \
   {if ((memptr = malloc(size)) == NULL) { \
	(void) piomsgout(getmsg(MF_PIOBE,1,MSG_MALLOC)); \
	(void) pioexit(PIOEXITBAD); \
    }}

#define CALLOC(memptr, size) \
   {if ((memptr = calloc(size, sizeof(char))) == NULL) { \
	(void) piomsgout(getmsg(MF_PIOBE,1,MSG_MALLOC)); \
	(void) pioexit(PIOEXITBAD); \
    }}

#define REALLOC(newptr, oldptr, size) \
   {if ((newptr = realloc(oldptr, size)) == NULL) { \
	(void) piomsgout(getmsg(MF_PIOBE,1,MSG_MALLOC)); \
	(void) pioexit(PIOEXITBAD); \
    }}


/* Variables available in message text when using ERREXIT() macro (below)
    %1$s   print job title
    %2$s   program name (piobe or pioformat)
    %3$s   subroutine name (piogetvals, piocmdout, etc.)
    %4$s   print queue name
    %5$s   queue device name
    %6$s   stringval1 (below)
    %7$s   stringval2 (below)
    %8$d   integerval (below)
*/

#define ERREXIT(err_type, msgnum, stringval1, stringval2, integerval) \
      { errinfo.errtype = err_type; \
	strncpy(errinfo.string1, stringval1, sizeof(errinfo.string1) - 1); \
	strncpy(errinfo.string2, stringval2, sizeof(errinfo.string2) - 1); \
	errinfo.integer = integerval; \
	(void) piogenmsg(msgnum, TRUE); \
	(void) pioexit(PIOEXITBAD); }

#define ERRDISP(err_type, msgnum, stringval1, stringval2, integerval) \
      { errinfo.errtype = err_type; \
	strncpy(errinfo.string1, stringval1, sizeof(errinfo.string1) - 1); \
	strncpy(errinfo.string2, stringval2, sizeof(errinfo.string2) - 1); \
	errinfo.integer = integerval; \
	(void) piogenmsg(msgnum, TRUE); }

#define STORE_ERROR_INFO(err_type, msgnum, stringval1, stringval2, \
			 integerval, err_infop) \
      { err_infop->errorcode = errinfo.errtype = err_type; \
	strncpy(errinfo.string1, stringval1, sizeof(errinfo.string1) - 1); \
	strncpy(errinfo.string2, stringval2, sizeof(errinfo.string2) - 1); \
	errinfo.integer = integerval; \
	(void) piogenmsg(msgnum, FALSE); \
	err_infop->errormsg = msgbuf; }

/* The ERRETURN() macro is only a place holder for logic that will be needed */
/* if we ever support an option to return to the caller when an error is     */
/* detected instead of issuing an error message and exiting. Until this      */
/* option is supported, we will never get here because the subroutine just   */
/* called will issue an error message and exit.                              */

#define ERRETURN()  /* place holder for return with error return code */


/*******************************************************************************
*                          Security Label Definitions                          *
*******************************************************************************/

/* AIX security enhancement */

#define NUMTABS_H 28

#define READ    0
#define WRITE   1

#ifndef LAB_BOTTOM
#define LAB_BOTTOM      2
#endif

#ifndef LAB_HEADER
#define LAB_HEADER      3
#endif

#ifndef LAB_TRAILER
#define LAB_TRAILER     4
#endif

#ifndef LAB_TOP
#define LAB_TOP         1
#endif

#define REPCHAR(c,n) {int cc; for (cc=n;cc;--cc) putchar(c); }
#define SET_EOL(str) strcpy(eol_char, str)

struct label_attr {
	 char   *label;
unsigned int    label_size;
	 short  label_type;
};

extern  char    eol_char[20];
extern  char    *get_label();
extern  int     auditwrite();
extern  int     runsh();

extern  int     numhtabs;
extern  int     numvtabs;
extern  int     ht[];

	int     numhtabs_sav;
	int     numvtabs_sav;
	int     ht_sav[NUMTABS_H];
/* TCSEC Division C Class C2 */


/*******************************************************************************
*                          Table of Spooler Flags                              *
*******************************************************************************/

#define SPOOLERFLAGS \
static struct splrflags_struct { \
    char flagname; /* flag letter */ \
    char hasarg;   /* command line argument? 0=no;  1=yes */ \
    char *val;     /* default value */ \
} spoolerflags[] = { \
    { 'B', 1, "xx" },       /* burst page */ \
    { 'C', 0, "!" },        /* mail only (don't display messages ) */ \
    { 'D', 1, "" },         /* deliver to */ \
    { 'M', 1, "" },         /* msg to console (text from file) */ \
    { 'N', 1, "1" },        /* number of copies */ \
    { 'P', 1, "" },         /* print queue[:queue device] */ \
    { 'R', 1, "15" },       /* priority */ \
    { 'T', 1, "" },         /* print job title */ \
    { 'c', 0, "!" },        /* copy file & print from copy */ \
    { 'm', 1, "" },         /* msg to console (text from command line) */ \
    { 'n', 0, "!" },        /* notify when job completes */ \
    { 'r', 0, "!" },        /* remove print file when done */ \
};
#define NUMSPOOLERFLAGS \
    (sizeof(spoolerflags) / sizeof(struct splrflags_struct))

