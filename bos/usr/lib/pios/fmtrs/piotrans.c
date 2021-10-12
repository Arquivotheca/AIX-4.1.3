static char sccsid[] = "@(#)62  1.4.1.2  src/bos/usr/lib/pios/fmtrs/piotrans.c, cmdpios, bos411, 9428A410j 4/1/94 14:23:19";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: piocharin, piopushin, piotab1, piotab2
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** piotrans.c ***/

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include "piostruct.h"
#include "piobe_msg.h"

#define STG1_XLATE_DIR "d1"     /* data base attribute name for directory \
                                   containing stage 1 translate tables */

#define SUBS_CHAR   0x005F	/* Underscore printed when char not defined */

#define RANGE1_LOW  0x0000	/* These are the three valid ranges in the */
#define RANGE1_HIGH 0x06FF	/* UNICODE stage 1 translate table */
#define RANGE2_LOW  0x2000
#define RANGE2_HIGH 0x25FF
#define RANGE3_LOW  0xFE70
#define RANGE3_HIGH 0xFEFF

#define RANGE3_OFFSET 0xF170	/* offsets into the UNICODE stage 1 translate*/
#define RANGE2_OFFSET 0x1900	/* table for range 2 and 3 */

typedef struct {		/* Stucture used for multi-byte data (UTF) */
    unsigned char cmask;
    unsigned char cval;
    unsigned short umask;
    unsigned short uval;
} Tab;

static Tab tab[] = {		/* Masks used to convert multi-byte data */
    0x80,  0x00,  0x7F,    0,              /* 1 byte sequence */
    0xE0,  0xC0,  0x7FF,   0x80,           /* 2 byte sequence */
    0xF0,  0xE0,  0xFFFF,  0x800,          /* 3 byte sequence */
    0,                                     /* end of table */
};

extern int errno;

static struct stage1_info *chainptr = NULL; /* ptr to info. for data stream
                                               code page currently selected */
static ushort_t pushed_chars[280];  /* stack for pushed characters */
static int newest = -1;         /* index to newest character pushed on stack */

static union {                  /* used to convert among pointer types */
    int  *intptr;
    char *charptr;
    short *shortptr;
    struct transtab *transtabptr;
} memptr;



/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     piocharin                                                         |
|                                                                             |
| FUNCTION: gets next input character, optionally translated to               |
|           intermediate code page.  If character stack is not empty,         |
|           get the character from there instead.                             |
|                                                                             |
| INPUT:    fileptr - points to input file                                    |
|           codepagename - name of the input code page (e.g., "850")          |
|                                                                             |
| RETURNS:  code point in intermediate code page                              |
|                                                                             |
| ERROR                                                                       |
|   EXIT:   issues error message and exits                                    |
|                                                                             |
+----------------------------------------------------------------------------*/

piocharin(fileptr, codepagename)
FILE *fileptr;
char *codepagename;
{
struct stage1_info *ip;         /* work pointer */
int codept_in;                  /* input code point (data stream code page) */
int codept_out;                 /* output code point (intermediate code page) */
int codept_tmp;			/* temp input code point used for Unicode */
int out_of_range;               /* flag used when codept is invalid */
int done;			/* flag used to indicate valid ucs char found */
unsigned short ucs;		/* value of converted multibyte character */
char c, c0, mb;			/* temporary values used for multibyte trans.*/
Tab *t;				/* used in multibyte translation */
char wkstr1[20], wkstr2[20];    /* work strings */
struct stage1_info *piotab1();
static struct stage1_info *current_ip = NULL; /* ptr to info. for translate
                                                 table currently being used */

/* If character(s) On Stack, Use Last One Pushed On Stack */
if (newest >= 0) return((int) pushed_chars[newest--]);

/* If 'codepagename' is NULL then this routine is instructed to retrieve      */
/* the next character from the data stream and return it as is - untranslated */
if (!(*codepagename))
    {
    if ((codept_in = piogetc(fileptr)) == EOF) return(EOF);
    else return(codept_in);
    }

/* Check For Switch to Code Page Whose Translate Table Is Already Loaded */
if (current_ip != NULL && strcmp(codepagename, current_ip->cdpgname))
    {
    current_ip = NULL;
    for (ip = chainptr; ip != NULL;) {
        if (!strcmp(ip->cdpgname, codepagename))
            {
            current_ip = ip;
            break;
            }
        ip = ip->infop;
    }
}

/* If the Needed Translate Table Isn't Loaded, Then Load It */
if (current_ip == NULL)
    current_ip = piotab1(codepagename);
    /* Convert Offsets (if any) to Pointers According to Table Format Type */

/* Get Next Character From Input Data Stream and Process It */
do {
    if ((codept_in = piogetc(fileptr)) == EOF) return(EOF);
    switch (current_ip->fmt_type) {
    case 1:  /* short ints, one for each input code point */
	memptr.charptr = current_ip->tabp;
	codept_out = (int) *(memptr.shortptr + codept_in);
        if (codept_out < 0) {
            /* Special Value (Is Not Really a Code Point) */
            switch (codept_out) {
            case CP:  /* Copy the Input Code Point */
                codept_out = codept_in;
                break;
            case RM:  /* Remove (discard) the Input Character */
                break;  /* the "while" loop we're in will handle RM */
            default:
		(void) sprintf(wkstr1, "%d", codept_in);
		(void) sprintf(wkstr2, "%d", codept_out);
		(void) errorexit(MSG_CODEPOINT1,
		      current_ip->filename, wkstr1, wkstr2);
            }
        }
        break;
    case 2:  /* Handle UCS 2-byte data here */
	out_of_range = 0;
	memptr.charptr = current_ip->tabp;
	if ((codept_tmp = piogetc(fileptr)) == EOF) return(EOF);
	codept_in = codept_in << 8 | codept_tmp;
	/* Check to make sure the input code point is in a valid range */
	if (codept_in >= RANGE3_LOW && codept_in <= RANGE3_HIGH)
            codept_in -= RANGE3_OFFSET;
	else if (codept_in >= RANGE2_LOW && codept_in <= RANGE2_HIGH)
	    codept_in -= RANGE2_OFFSET;
	else if (!(codept_in >= RANGE1_LOW && codept_in <= RANGE1_HIGH))
	    out_of_range++;			
	if (out_of_range)      /* character not valid, so use substitute */
	    codept_out = SC;
	else
	    codept_out = (int) *(memptr.shortptr + codept_in);
        if (codept_out < 0) {
            /* Special Value (Is Not Really a Code Point) */
            switch (codept_out) {
            case CP:  /* Copy the Input Code Point */
                codept_out = codept_in;
                break;
            case RM:  /* Remove (discard) the Input Character */
                break;  /* the "while" loop we're in will handle RM */
	    case SC:  /* Substitute a character */
		codept_out = SUBS_CHAR;
		break;
            default:
		(void) sprintf(wkstr1, "%d", codept_in);
		(void) sprintf(wkstr2, "%d", codept_out);
		(void) errorexit(MSG_CODEPOINT1,
		      current_ip->filename, wkstr1, wkstr2);
            }
        }
        break;
    case 3:  /* Convert UTF multi-byte data to UCS 2-byte data */
	out_of_range = 0;		/* Initialize flags */
	done = 0;
	mb = codept_in;			/* Initialize variables to first char*/
	c0 = mb;
	ucs = c0;
        for (t = tab; t->cmask; t++) {
            if ((c0 & t->cmask) == t->cval) { /* check for 1,2, or 3 byte data*/
                ucs &= t->umask;
                if (ucs < t->uval) {
                    break;
                }
		if (ucs) {   /* Valid multi-byte character found and converted*/
		    done++;
		    codept_in = ucs;
		    break;
		}
            }
	    /* Read a 2nd or 3rd character and perform the conversion */
            if ((mb = piogetc(fileptr)) == EOF) return(EOF);
            c = mb ^ 0x80;
            if (c & 0xC0) {
                break;
            }
            ucs = (ucs<<6) | c;
        }
        if (done) { /* character converted to 2-byte,now check for valid range*/
            memptr.charptr = current_ip->tabp;
            if (codept_in >= RANGE3_LOW && codept_in <= RANGE3_HIGH)
                codept_in -= RANGE3_OFFSET;
            else if (codept_in >= RANGE2_LOW && codept_in <= RANGE2_HIGH)
                codept_in -= RANGE2_OFFSET;
            else if (!(codept_in >= RANGE1_LOW && codept_in <= RANGE1_HIGH))
                out_of_range++;
            if (out_of_range)    /* character not valid, so use substitute */
                codept_out = SC;
            else
                codept_out = (int) *(memptr.shortptr + codept_in);
	}
	else  /*Not a valid multi-byte character so print out substitute char */
	    codept_out = SC;
        if (codept_out < 0) {
            /* Special Value (Is Not Really a Code Point) */
            switch (codept_out) {
            case CP:  /* Copy the Input Code Point */
                codept_out = codept_in;
                break;
            case RM:  /* Remove (discard) the Input Character */
                break;  /* the "while" loop we're in will handle RM */
            case SC:  /* Substitute a character */
                codept_out = SUBS_CHAR;
                break;
            default:
                (void) sprintf(wkstr1, "%d", codept_in);
                (void) sprintf(wkstr2, "%d", codept_out);
                (void) errorexit(MSG_CODEPOINT1,
                      current_ip->filename, wkstr1, wkstr2);
            }
        }
        break;
    default:
	(void) sprintf(wkstr1, "%d", current_ip->fmt_type);
	(void) errorexit(MSG_FMT_TYPE, current_ip->filename, wkstr1, NULL);
    }
} while (codept_out == RM); /* if remove (discard), ignore it & go on to next */

return(codept_out);
}


/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     piopushin                                                         |
|                                                                             |
| FUNCTION: pushes a character back on the input data stream                  |
|                                                                             |
| INPUT:    pushchar - character to be pushed back on the input data stream   |
|                                                                             |
| RETURNS:  0                                                                 |
|                                                                             |
| ERROR                                                                       |
|   EXIT:   issues error message and exits                                    |
|                                                                             |
+----------------------------------------------------------------------------*/
piopushin(pushchar)
int pushchar;
{
if (++newest >= 280)
    (void) errorexit(MSG_MANY_CHARS, NULL, NULL, NULL);
pushed_chars[newest] = pushchar;
return(0);
}


/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     piotab1                                                           |
|                                                                             |
| FUNCTION: loads the stage 1 translate table for the specified input         |
|           code page, builds a stage1_info structure, and adds the           |
|           stage1_info structure to the chain of stage1_info structures      |
|           (a stage 1 translate table is used to translate code points       |
|           from the input data stream to an intermediate code page).         |
|                                                                             |
| INPUT:    codepagename - name of the code page for input data               |
|                                                                             |
| RETURNS:  pointer to stage1_info structure                                  |
|                                                                             |
| ERROR                                                                       |
|   EXIT:   issues error message and exits                                    |
|                                                                             |
+----------------------------------------------------------------------------*/

static struct stage1_info *
piotab1(codepagename)
char * codepagename;
{
struct stage1_info *ip;         /* work pointer */
static struct stage1_info *last_ip;
				/* ptr to last stage1_info struct on chain */
struct stat statbuf;            /* status returned by "stat" system call */
int rc;                         /* return code from subroutines */
int cnt;                        /* work integer */
static int fn_offset;           /* offset to file name in filename1 */
int fildes;                     /* file descriptor */
int header_failed = FALSE;      /* indicator that read of header failed */
char header[16];                /* file header */
char wkstr1[20];                /* work strings */
static char filename1[200];     /* path name of Stage 1 translate table file */
char *malloc();

/* Build Path Name For File With Stage 1 Translate Table for the Code Page */
if (chainptr == NULL) {
    rc = piogetstr(STG1_XLATE_DIR, filename1, sizeof(filename1), NULL);
    *(filename1 + rc) = '/';
    fn_offset = rc + 1;
}
cnt = strlen(codepagename) + 1;
if (fn_offset + cnt > sizeof(filename1))
    cnt = sizeof(filename1) - fn_offset;
(void) strncpy(filename1 + fn_offset, codepagename, cnt);

/* Get the Length of the File, and Open the File */
if (stat(filename1, &statbuf) != 0 ||
      (fildes = open(filename1, O_RDONLY)) < 0) {
    (void) sprintf(wkstr1, "%d", errno);
    (void) errorexit(MSG_OPEN_TRANS1, filename1, wkstr1, NULL);
}

/* Read In the Header To Verify That It is a Stage 1 Translate Table */
/* File and That the File Format Version Is One That Is Supported    */
if (read(fildes, header, sizeof(header)) < 0 &&
      statbuf.st_size >= sizeof(header))
    header_failed = TRUE;
else {
    if (statbuf.st_size < sizeof(header) ||
	  strncmp(header, "PIOSTAGE1XLATE", 14))
	(void) errorexit(MSG_BAD_STAGE1, filename1, NULL, NULL);
    if (strncmp(header + 14, "00", 2))
	(void) errorexit(MSG_OLD_STAGE1, filename1, NULL, NULL);
}

/* Malloc the Space Needed */
if ((ip = (struct stage1_info *) malloc(sizeof(struct stage1_info))) == NULL
      || (ip->cdpgname = malloc(strlen(codepagename) + 1)) == NULL
      || (ip->filename = malloc(strlen(filename1) + 1)) == NULL
      || (ip->tabp = malloc(statbuf.st_size - sizeof(header))) == NULL) {
    errorexit(MSG_FMTRS_MALLOC, NULL, NULL, NULL);
}

/* Initialize stage1_info Structure and Add to Chain */
(void) strcpy(ip->cdpgname, codepagename);
(void) strcpy(ip->filename, filename1);
ip->infop = NULL;
if (chainptr == NULL)
    chainptr = ip;
else
    last_ip->infop = ip;
last_ip = ip;  /* save for next time */

/* Read In the File */
if (header_failed ||
      read(fildes, ip->tabp, statbuf.st_size - sizeof(header)) < 0) {
    (void) sprintf(wkstr1, "%d", errno);
    (void) errorexit(MSG_READ_TRANS1, filename1, wkstr1, NULL);
}

/* Save the Translate Table Format Type & Adjust the Table Pointer */
memptr.charptr = ip->tabp; /* get int value for format type */
ip->fmt_type = *(memptr.intptr);
ip->tabp += sizeof(int);   /* jump over value for format type */

return(ip);

}


/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     piotab2                                                           |
|                                                                             |
| FUNCTION: loads the specified stage 2 translate table, builds a stage2_info |
|           structure, and adds the structure to the circular chain of        |
|           stage2_info structures.                                           |
|                                                                             |
| INPUT:    attrname - name of an attribute that specifies the full path      |
|                      names, separated by commas, of stage 2 translate       |
|                      tables (intermediate code page to printer code page)   |
|                      to be loaded and chained.                              |
|           sel_infopp - pointer to a pointer to the stage2_info structure    |
|                        for the code page currently selected by the printer  |
|                                                                             |
| RETURNS:   0 - successful                                                   |
|           -1 - attribute was as null string, so no table was loaded         |
|                                                                             |
| ERROR                                                                       |
|   EXIT:   issues error message and exits                                    |
|                                                                             |
+----------------------------------------------------------------------------*/
piotab2(attrname, sel_infopp)
char *attrname;
struct stage2_info **sel_infopp;
{
struct stage2_info *ip;         /* work pointer */
static struct stage2_info *prev_ip;
                                /* ptr to prev stage2_info structure added */
struct stat statbuf;            /* status returned by "stat" system call */
int rc;                         /* return code from subroutines */
int cnt;                        /* work integer */
int fildes;                     /* file descriptor for input file */
int header_failed = FALSE;      /* indicator that read of header failed */
char *ptr;                      /* work pointer */
char header[16];                /* file header */
char wkstr1[20];                /* work strings */
static char filenames[1000];    /* path names, separated by commas */
char *filenamep;                /* ptr to file name being processed */
char *nextnamep;                /* ptr to next file name to be processed */
char *malloc();

/* Get Path Names for Files Containing Stage 2 Translate Tables */
if ((rc = piogetstr(attrname, filenames, sizeof(filenames), NULL)) < 0)
    pioexit(PIOEXITBAD);  /* piogetstr() output an error message */
else if (rc == 0)
    return(-1);

filenamep = filenames;
if (*filenamep == ',')
    filenamep++;
for (; filenamep && *filenamep; filenamep = nextnamep) {
    if (*filenamep == ',')
        filenamep++;
    if ((nextnamep = strchr(filenamep, ','))) /* look for file name separator*/
	*nextnamep++ = NULL;

    /* Get the Length of the File, and Open the File */
    if (stat(filenamep, &statbuf) != 0 ||
	(fildes = open(filenamep, O_RDONLY)) < 0) {
	(void) sprintf(wkstr1, "%d", errno);
	(void) errorexit(MSG_OPEN_TRANS2, filenamep, wkstr1, NULL);
    }

    /* Read In the Header To Verify That It is a Stage 2 Translate Table */
    /* File and That the File Format Version Is One That Is Supported    */
    if (read(fildes, header, sizeof(header)) < 0 &&
	  statbuf.st_size >= sizeof(header))
	header_failed = TRUE;
    else {
	if (statbuf.st_size < sizeof(header) ||
	      strncmp(header, "PIOSTAGE2XLATE", 14))
	    (void) errorexit(MSG_BAD_STAGE2, filenamep, NULL, NULL);
	if (strncmp(header + 14, "00", 2))
	    (void) errorexit(MSG_OLD_STAGE2, filenamep, NULL, NULL);
    }

    /* Malloc the Space Needed */
    cnt = statbuf.st_size - sizeof(header);
    if ((ip = (struct stage2_info *) malloc(sizeof(struct stage2_info))) == NULL
	  || (ip->filename = malloc(strlen(filenamep) + 1)) == NULL
	  || (ptr = malloc(cnt)) == NULL) {
	errorexit(MSG_FMTRS_MALLOC, NULL, NULL, NULL);
    }
    memptr.charptr = ptr;           /* convert (char *) to (transtab *) */
    ip->tabp = memptr.transtabptr;

    /* Initialize stage2_info Structure and Add to Chain */
    (void) strcpy(ip->filename, filenamep);
    if (*sel_infopp == NULL) {
	*sel_infopp = ip;
	ip->infop = ip;
    } else {
	ip->infop = prev_ip->infop;
	prev_ip->infop = ip;
    }
    prev_ip = ip;  /* save for next time */

    /* Read In the File */
    if (header_failed ||
	  read(fildes, ip->tabp, statbuf.st_size - sizeof(header)) < 0) {
	(void) sprintf(wkstr1, "%d", errno);
	(void) errorexit(MSG_READ_TRANS2, filenamep, wkstr1, NULL);
    }

    /* Save Pointer To Commands List, Number of Table Entries, & Pointer to Table */
    ptr = (char *) ip->tabp;            /* easier to work with "char *" */
    memptr.charptr = ptr;               /* convert (char *) to (int *) */
    cnt = *(memptr.intptr);             /* number of commands */
    ptr += sizeof(int);                 /* jump over value for number of commands */
    ip->cmdp = (char (*)[2]) ptr;       /* ptr to list of two-character cmd names */
    ip->num_cmds = cnt;                 /* number of commands */
    ptr += cnt * 2;                     /* jump over list of command names */
    ip->num_tabentries = (statbuf.st_size - sizeof(header) - sizeof(int) - cnt * 2)
	/ sizeof(struct transtab);      /* number of entries in table */
    memptr.charptr = ptr;               /* (char *) to (struct transtab *) */
    ip->tabp = memptr.transtabptr;      /* put it where it really belongs */
    ip->miscp = NULL;                   /* for formatter use, as needed */
}

return(0);

}
