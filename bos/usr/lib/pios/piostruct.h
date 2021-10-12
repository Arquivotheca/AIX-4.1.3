/* @(#)42       1.9  src/bos/usr/lib/pios/piostruct.h, cmdpios, bos411, 9428A410j 12/8/93 18:33:13 */
/*
 *   COMPONENT_NAME: CMDPIOS
 *
 *   FUNCTIONS: piogetc
 *		piogetchar
 *		pioputc
 *		pioputchar
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
*  piostruct.h - Definitions for Print Formatter, Message Parse Routine       *
******************************************************************************/
#ifndef _PIOSTRUCT_H
#define _PIOSTRUCT_H

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  0
#endif

#define PIOEXITGOOD 0   /* good exit */
#define PIOEXITBAD  1   /* bad exit */

#define piogetc(stream) getc(stream)
#define piogetchar()    piogetc(stdin)
#define pioputc(x, p) { if (!piopgskip)  {          \
                                putc(x, p);         \
                                piodatasent = TRUE; \
                        }}

#define pioputchar(x)   pioputc((x), stdout)

#define SPOOLER statusfile
extern int statusfile;

struct str_info {       /* string information */
    char *ptr;              /* pointer to string (string may contain nulls) */
    int  len;               /* current length of string */
};


struct lktable {        /* string-to-integer lookup table */
    char *str;              /* pointer to string value */
    int  value;             /* resulting integer value */
};


struct attrparms {      /* attribute parameter input to piogetvals() */
    char *name;             /* two-character attribute name (null terminated) */
    int  datatype;          /* date type of the attribute */
#define VAR_INT   1             /* variable integer */
#define VAR_STR   2             /* variable string */
    struct lktable *lktab;  /* pointer to lookup table (if NULL, use atoi()) */
    union dtypes            /* ptr to where piogetvals() is to store data ptr */
    {
        struct str_info *sinfo; /* pointer to pointer to str_info structure */
        int  *addr;             /* pointer to pointer to integer */
    } *types;
};



/* Pointers To Vertical Spacing Values Shared With the Formatter Driver  */
/* NOTE: All pointers in the table, and all values they point to (except */
/*       "_vtab_base"), are initialized and maintained by the formatter. */
/*       "Vres units" is the number of units into which a vertical inch  */
/*       is divided.                                                     */
struct shar_vars {
    int    *_pl;        /* page length, in Vres units.  A value of zero means
                           that page boundaries will be ignored. */
    int    *_tmarg;     /* top margin, in Vres units */
    int    *_bmarg;     /* bottom margin, in Vres units */
    int    *_vpos;      /* vertical position where the next line is to be
                           be printed (in Vres units, relative to the top
                           of the physical page) */
    int    *_vtab_base; /* base vertical position for vertical tab stops.
                           Adjusted to reflect top margin and security label.
                           Set by formatter driver.  Used by formatter
                           to calculate the new vertical position when a
                           vertical tab is encountered */
    int    *_vincr;     /* vertical increment: the number of Vres units
                           that the command whose attribute name is pointed
                           to by "_vincr_cmd" will advance the page */
    char   *_vincr_cmd; /* pointer to the attribute name of the vertical
                           spacing command (e.g., a line feed, or line
                           feed/carrier return) that will space down the
                           number of Vres units specified by the the integer
                           pointed to by "_vincr". */
    int    *_vdecr;     /* vertical decrement: the number of Vres units
                           that the command whose attribute name is pointed
                           to by "_vdecr_cmd" will back up the page */
    char   *_vdecr_cmd; /* pointer to the attribute name of the vertical
                           backspacing command (e.g., a reverse line feed)
                           that will space up the number of Vres units
                           specified by the the integer pointed to by
                           "_vdecr". */
    char   *_ff_cmd;    /* pointer to the attribute name for the form feed
                           command. */
    int    *_ff_at_eof; /* if TRUE, output form feed at end of input file */
    char   *_set_cmd;   /* pointer to the attribute name of the command to be
                           sent to the printer by the formatter driver to put
                           the printer in the proper state when switching back
                           & forth between "current" (PIO_CURR_MODE) mode and
                           "security label" (PIO_SECUR_MODE) mode. The command
                           should normally be similar to the printer
                           initialization and restore commands, except that the
                           page length should not be changed. */
    int    *_reserved1; /* reserved for future use; must be set to NULL */
    int    *_reserved2; /* reserved for future use; must be set to NULL */
    char   *_reserved3; /* reserved for future use; must be set to NULL */
    char   *_reserved4; /* reserved for future use; must be set to NULL */

};


extern int piopgskip;   /* number of pages to skip before starting to print */

extern int piodatasent; /* if TRUE, print data has been sent, so a form feed
                           (if requested) is needed at completion */

extern int piomode;     /* determines which set of attribute values to use
                           NOTE:  A formatter should normally have no reason
                                  to reference the "piomode" variable or these
                                  mode definitions directly.  The formatter
				  driver manages mode values */
#define PIO_DBASE_MODE 0    /* use attribute values from the data base */
#define PIO_CURR_MODE  1    /* use current attribute values.  These are data
                               base values, overlaid by command line values
                               (if any), and updated by the formatter to
                               reflect the current state of the printer. */
#define PIO_SECUR_MODE 2    /* use data base values, overlaid by command line
			       values (if any).  Reserved for future use. */
#define PIO_NUM_MODES  3    /* number of modes */

#define PIO_CFLEVEL_ATTR	"zL"	/* level of a colon file */
#   define PIO_CFLEVEL_NEW	2	/* new colon files with limits */
#   define PIO_CFLEVEL_CONV	1	/* old colon files converted to new */
#   define PIO_CFLEVEL_OLD	0	/* old colon files */
#define PIO_CFLEVEL_DECL	\
   static int			*_piocflevel;	\
   static struct attrparms	piocfltbl[] =  {	\
      { PIO_CFLEVEL_ATTR, VAR_INT, NULL, (union dtypes *)&_piocflevel }, \
      { NULL            , 0      , NULL, NULL }	\
   }
#define Piocflevel (*(_piocflevel + piomode))


/* Definitions For Translate Tables */

struct transtab {               /* entry in translate table */
  short outchar;                  /* char to output (or, see #define's below) */
  unsigned char index1;           /* index to prefix cmd (or translate table) */
  unsigned char index2;           /* index to suffix cmd (or is a code point) */
};
/* defines for outchar (used instead of the code point) */
#define CP              -1      /* copy input character value to output */
#define RM              -2      /* remove (discard) the input character */
#define SC              -3      /* use substitute char (can't print real one) */


struct stage1_info {            /* info. needed to translate from a particular
                                   data stream code page to the intermediate
                                   code page */
    int fmt_type;                   /* translate table format type */
    char *tabp;                     /* ptr to translate table info. */
    char *cdpgname;                 /* ptr to code page name */
    char *filename;                 /* ptr to tranlate table file name */
    struct stage1_info *infop;      /* ptr to translate info. for another */
};                                  /* data stream code page */


struct stage2_info {            /* info. needed to translate from the
                                   intermediate code page to a
                                   particular printer code page */
    struct transtab *tabp;          /* ptr to translate table */
    int num_tabentries;             /* number of entries in translate table */
    char (*cmdp)[2];                /* pointer to array of command names
                                       (first cmd selects printer code page) */
    int num_cmds;                   /* number of commands */
    char *miscp;                    /* pointer to misc. formatter info.
                                       (for formatter use, as needed) */
    char *filename;                 /* ptr to tranlate table file name */
    struct stage2_info *infop;      /* pointer to translate info. for
                                       another printer code page */
};

#define PIODEFMSG \
"0782-000 Cannot access message text (index %d)\n\
\tin message catalog file %s\n\
\tUse local problem reporting procedures.\n"

/* Define a structure for storing error info. by functions "piocmdout"
   and "piogetstr" */
typedef struct error_info {		/* error info. */
   int		errorcode;	/* error code */
#define ERR_NOATTR		1	/* attribute doesn't exist */
   char		*errormsg;	/* error message text */
}			error_info_t;



/******************************************************************************
*  Definitions for a pioout (device driver interface program) exit routine,   *
*  parsemsg(), which can be used to parse ASCII messages sent by a PostScript *
*  printer.  The routine is a linked module loaded by the pioout command.  It *
*  is made known to a virtual printer by specifying its full path name with   *
*  the "mr" attribute in the data base (refer to the chvirprt command).       *
******************************************************************************/

/* Exit Codes */
#define PIOR_CONT  0    /* continue processing */
#define PIOR_END   1    /* print processing is completed */
#define PIOR_BAD   2    /* terminate the print job */
#define PIOR_FAIL  3    /* fatal error; terminate the print job, put it
			   back on the print queue, and shut down the
			   print queue. Don't try to print trailer
			   page (if requested). */

/* Message Beginning and Ending Strings */
#define BEGSTR "%%[ "
#define ENDSTR " ]%%"
#define BEGLEN (sizeof(BEGSTR)-1)
#define ENDLEN (sizeof(ENDSTR)-1)

/* Structure Pointed To by the Pointer Passed to parsemsg() from pioout */
struct piormsg {
    char *textptr;      /* pointer to message text received from printer
			   (end-of-file characters (hex 04) already removed) */

    int sub_predef;     /* indicates whether the predefined message text
			   "Message from printer xxx." should precede the
			   output message text specified by sub_text (if any).
			   0 - no
			   1 - yes (default) */
    char *sub_text;     /* pointer to null string at the beginning of a buffer
			   the length of the input message pointed to by
			   textptr, plus 100 bytes.  parsemsg() should put in
			   the buffer whatever text is to be sent to the print
			   job submitter and terminate the text with a null. */

    int irq_predef;     /* indicates whether the predefined message text
			   "Message from printer xxx." should precede the
			   output message text specified by irq_text (if any).
			   0 - no
			   1 - yes (default) */
    char *irq_text;     /* pointer to null string at the beginning of a buffer
			   the length of the input message pointed to by
			   textptr, plus 100 bytes.  parsemsg() should put in
			   the buffer whatever text is to be sent to the
			   "intervention required" user(s) specified by the
			   "si" attribute and terminate the text with a null.
			   Refer to the -I flag for the pioout command. */

    struct element *irq_typep;
			/* chain of structures identifying message types to be
			   sent to "intervention required" user(s) instead of
			   to the print job submitter. Example: "PrinterError".
			   Refer to the -I and -L flags for the pioout
			   command. */

    struct element *discardp;
			/* chain of structures specifying text strings that
			   identify messages to be discarded. Example:
			   "warming up". Refer to the -K flag for the
			   for the pioout command. */
};

/* Structures Chained Together To Specify a Variable Number of Text Strings */
struct element {
    struct element *next;  /* ptr to next structure in chain (null if last) */
    char *text;            /* ptr to text string */
};

/* Example Parse Routine (similar to the default routine)

#include <piostruct.h>

parsemsg(msgp)
struct piormsg *msgp;
{
char *sub_ptr, *irq_ptr, *p1, *p2, *p3, charsave;
struct element *ep;

sub_ptr = msgp->sub_text;
irq_ptr = msgp->irq_text;
p1 = msgp->textptr;
while (p1 = strstr(p1, BEGSTR)) {
    if (!(p2 = strstr(p1 + BEGLEN, ENDSTR)))
	break;
    charsave = *(p2 + ENDLEN);
    *(p2 + ENDLEN) = '\0';
    for (ep = msgp->discardp; ep; ep = ep->next)
	if (strstr(p1, ep->text))
	    goto NEXTMSG;
    for (ep = msgp->irq_typep; ep; ep = ep->next)
	if (p3 = strstr(p1 + BEGLEN, ep->text)) {
	    p3 += strlen(ep->text) + 2;
	    (void) strncpy(irq_ptr, p3, (p2-p3));
	    irq_ptr += (p2-p3);
	    *irq_ptr++ = '\n';
	    goto NEXTMSG;
	}
    (void) strncpy(sub_ptr, p1, p2 + ENDLEN - p1);
    sub_ptr += p2 + ENDLEN - p1;
    *sub_ptr++ = '\n';
  NEXTMSG:
    p1 = p2 + ENDLEN;
    *p1 = charsave;
}
*sub_ptr = *irq_ptr = '\0';
return(PIOR_CONT);
}

To compile and link the example parse routine, enter:
   cc -o /somepath/parse -bI:parse.imp -bE:parse.exp parse.c
where parse.c contains the parsemsg function,
/somepath is the path to some directory where the module can be stored,
parse.imp is an imports file containing two lines:
#!
main
and parse.exp is an exports file containing two lines:
#!
parsemsg
Make the parse routine known to a virtual printer by running the
chvirprt command (or use the smit virtual printers menu) and specifying:
   mr=/somepath/parse

*/

/******************	Declarations for colon file API    ********************/
/* Given below are declarations for colon file API functions piogetattrs()
   and pioputattrs().  These functions are used to fetch and update printer
   attribute database information.  For further details, refer to man/info
   pages on these functions. */
/******************************************************************************/

#define PIOATTRNMLEN		2
#define PIOCFAPI		"/usr/lib/lpd/pio/etc/piocfapi"

/* Format of each attribute element passed to piogetattrs() or pioputattrs() */
typedef struct pioattr	{
   char			pa_name[PIOATTRNMLEN];	/* attribute name */
   unsigned char	pa_type;		/* type of value */
#  define PA_AVALT		1		/* attribute value */
#  define PA_ADSCT		2		/* attribute description
						   (get only) */
#  define PA_ALIMT		3		/* attribute limits
						   (get only) */
   void			*pa_value;		/* pointer to resolved value */
   unsigned short	pa_vallen;		/* length of resolved value */
   char			pa_retcode;		/* code returned from get/put
						   TRUE - operation successful
						   FALSE - otherwise */
} pioattr_t;

#ifdef _NO_PROTO
   extern int		piogetattrs();	/* Function to get attribute info */
   extern int		pioputattrs();	/* Function to update attribute info */
#else
   extern int		piogetattrs(const char *,const char *,unsigned short,
				    pioattr_t *);
   extern int		pioputattrs(const char *,const char *,unsigned short,
				    pioattr_t *);
#endif


/******************	Declarations for MTF from piobe    ********************/
/* Given below are declarations for message header and text frames that
   are sent from the printer backend via an ipc mechanism provided by
   a print supervisor.  For further details, refer to man/info pages on
   message transmission from the printer backend. */
/******************************************************************************/

#define PIOMSGIPCENV		"PIO_IPCWRITEFD"
#define PM_CATNMMAX		(127)
#define PM_MAXMSGARGNO		(9)

typedef struct piomsghdr {		/* Message header frame */
   unsigned char	pm_msgtype;	/* message type */
#  define	ID_VAL_EVENT_ABORTED_BY_SERVER				(0)
#  define	ID_VAL_EVENT_DOCUMENT_ABORTED				(1)
#  define	ID_VAL_EVENT_ERROR_PAGE_SELECT				(2)
#  define	ID_VAL_EVENT_ERROR_PAGE_SELECT				(2)
#  define	ID_VAL_EVENT_ERROR_DOCUMENT_CONTENT			(3)
#  define	ID_VAL_EVENT_ERROR_OTHER				(4)
#  define	ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION		(5)
#  define	ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_OPERATOR		(6)
#  define	ID_VAL_EVENT_WARNING_CRITERION_EXCEEDED			(7)
#  define	ID_VAL_EVENT_WARNING_OTHER				(8)
#  define	ID_VAL_EVENT_REPORT_CHECKPOINT_TAKEN			(9)
#  define	ID_VAL_EVENT_STATE_CHANGED_PRINTER			(10)
   unsigned short	pm_msgno;	/* message no in the catalog */
   unsigned short	pm_setno;	/* set no in the catalog */
   char			pm_catnm[PM_CATNMMAX+1]; /* message catalog file name */
   unsigned short	pm_msglen;	/* length of expanded message including
					   the trailing NUL character */
   unsigned char	pm_msgargno;	/* no of message arguments (max. 9) */
} piomsghdr_t;

typedef struct piomarghdr {		/* Message argument header frame */
   unsigned char	pa_argtype;	/* message argument type */
#  define	PA_ATSTRING	0
#  define	PA_ATINT	1
   unsigned short	pa_arglen;	/* message argument length including
					   the trailing NUL character */
} piomarghdr_t;


#endif   /* _PIOSTRUCT_H */
