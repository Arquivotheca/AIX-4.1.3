static char sccsid[] = "@(#)34  1.3  src/bldenv/pkgtools/rename/geninptf/geninptf.c, pkgtools, bos412, GOLDA411a 3/10/92 16:21:26";
/*
* COMPONENT_NAME: geninptf.c
*	The purpose of this program is to modify 'inst_root'
*	directory references in the 'inslist' file during the
*	update cycle.  The changes created are:
*	    1.  If file-name contains 'inst_root';
*		see if 'oldpath' attribute in stanza,
*		if not, create oldpathline, using file-name;
*		insert 'inst_$PTF' before 'inst_root' in
*		file-name.
*	    2.  If 'symlinks' attribute exists,
*		insert 'inst_$PTF' before 'inst_root'.
*
* FUNCTIONS:
*	main()		Program control
*	checkName()	Modify stanza file name, if appropriate.
*	chackLink()	Modify symlinks entry, if appropriate.
*	addToStanza()	Add line to stanza, if necessary.
*	(Standard Orbit attribute file handling functions.)
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business machines Corp. 1989
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*	geninptf is called from genais as part of update process.
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/mode.h>

/* Define includes that are needed by the "cfg" functions.  These were in
   include file & library that were discarded. Source at end of this file. */
#define CFG_SUCC  0		/* Successful termination.		*/
#define CFG_EOF   4004		/* End of file encountered.		*/
#define CFG_EXBF -4013		/* Input exceeds buffer size.		*/
#define CFG_SZNF -4015		/* Stanza not found.			*/
#define CFG_SZBF -4016		/* Stanza size exceeds buffer size.	*/
#define CFG_UNIO -4017		/* Unrecoverable I/O error.		*/
#define MAXVAL   1024		/* Maximum size for keyword value.	*/
#define MAXKWD   75		/* Maximum keyword length.		*/
#define MAXDIR   512		/* Maximum directory path length.	*/

typedef struct {
    FILE *sfile;		/* Pointer returned from fopen().	*/
    char *defbuf;		/* Buffer containing default stanza.	*/
    short *defmap;		/* Array of line indexes in default.	*/
} CFG__SFT;
CFG__SFT *cfgcopsf();		/* The "open" returns struct pointer.	*/

/* Define the "Link" structure used in the list	handling routines. */
struct Link {
	struct Link	*next;
	char		*data;
};
typedef	struct Link	Link;

/* Define the "List" structure used in the list	handling routines. */
struct List {
	Link	*last;
	Link	*current;
	int	count;
};
typedef	struct List	List;

/* Declare non-integer returning functions. */
char		*malloc();
char		*getenv();
static int	checkName();
static int	checkLink();

/* Declare some	global variables. */

static int	foundFlag= 0;		/* On if any stanza is changed.      */
CFG__SFT	*cfg;
char		stanza[BUFSIZ];
char		fileName[MAXDIR];	/* Store the inslist stanza file-nm. */
char		inputName[MAXDIR];	/* Store input file name, from va[1].*/
char		inst_ptf[20];		/* Place to construct const for cmpr.*/
char		*PTF;			/* Pntr to PTF environmental variable*/
char		*whoami;		/* This pgm nm as got from ac.	     */
int		whoLen;			/* Length of 'whoami'.		     */
char		inst_root[]="/inst_root/";  /* constant for compare.	     */
FILE		*fpOut;			/* Pntr to output inslist file.	     */

/* */

main(ac, av)

int	ac;
char	*av[];
{
	int		rc;		/* Misc return code.		 */
	char		cmd[80];	/* Construct 'mv' command here.	 */

	/* Set program name, and name length, for use in error messages. */
	whoami = ((whoami = strrchr(av[0], '/')) == NULL ? av[0] : whoami+1);
	whoLen = strlen(whoami);

	/** Store input file name. Can be "inslist" or "sysck_input".   **/
	strcpy(inputName, av[1]);

	if (( PTF = getenv("PTF")) == NULL) {
	    printf("%s: Cannot read env variable PTF.\n", whoami);
	    exit(10);
	}
	strcpy(inst_ptf, "/inst_");
	strcat(inst_ptf, PTF);			/* Create 'inst_ptf' arg*/

	if ((cfg = cfgcopsf(inputName)) == NULL) {
	    printf("%s: Cannot open input \"inslist\" file.\n", whoami);
	    exit(11);
	}
	fpOut = fopen("insnew", "w"); 			/* Open out insnew. */
	if (fpOut == NULL) {				/* Did it open?     */
	    printf("%s: Cannot open output \"insnew\" file.\n", whoami);
	    exit(12);
	}
	while (rc != CFG_EOF) {
	    rc = cfgcread(cfg, stanza, BUFSIZ);	/* Standard stanza read */
	    if (rc == CFG_EOF) break;		/* End of inslist?	*/
	    if (rc != CFG_SUCC) {		/* Read error?		*/
		printf("%s: Error reading \"inslist\".\n", whoami);
		exit(13);
	    }
	    rc = checkName();			/* Change stanza name?	*/
	    rc = checkLink();			/* Change symlinks name?*/
	    fprintf(fpOut, "%s", stanza);	/* Write insnew.	*/
	}
	fclose(cfg->sfile);
	fclose(fpOut);
	unlink(inputName);			/* rm input file.	*/
	strcpy(cmd, "mv insnew ");
	strcat(cmd, inputName);			/* Construct 'mv' cmd.  */
	system(cmd);				/* Rename insnew to inpt*/
	exit(foundFlag);

}	/* End of main() **  */

static int
checkName()
{
	char	*nameLen;			/* Length of stanza name*/
	char	*ptr;				/* Utility pointer.	*/
	char	oldPath[MAXVAL];		/* Area to create oldpth*/
	char	buf[BUFSIZ];			/* Work area buffer.	*/

	/*** Put stanza file-name (1st line) into fileName. ***/
	nameLen = strchr(stanza, ':');
	strncpy(fileName, stanza, nameLen - stanza);
	fileName[nameLen - stanza] = '\0';

	/*** Does fileName contain inst_root? if so, prepend 'inst_$PTF' */
	if ( (ptr = strstr(fileName, inst_root)) == NULL) 
	    return(0);
	foundFlag = 69;				/* Set flag that some chg*/

	/*** If oldpath not in stanza, create an oldpath line.		*/
	if (cfgcskwd("oldpath", stanza, oldPath) == ~CFG_SUCC) {
	    strcpy(oldPath, "   oldpath = ");
	    strcat(oldPath, fileName);
	    addToStanza(oldPath);
	}
	/*** Create stanza in buf with revised fileName.		*/
	strncpy(buf, fileName, ptr - fileName);	/* Put 1st part of flNm */
	buf[ptr - fileName] = '\0';		/* Terminate it.	*/
	strcat(buf, inst_ptf);			/* Insert inst_ptf in Nm*/
	strcat(buf, stanza + (ptr - fileName));	/* finish stanza.	*/
	/*** Put revised stanza from buf back into stanza.		*/
	strcpy(stanza, buf);
	return(0);

}	/* End of checkName() ***  */

static int
checkLink()
{
	char	symline[MAXVAL];		/* Place to work symlinks. */
	char	symline2[MAXVAL];		/* Place to work symlinks. */
	char	*ptr;				/* Working pntr.	*/
	char	*ptr2;				/* another working ptr.	*/
	int	strLen;				/* Length of string.	*/

	if (cfgcskwd("symlinks", stanza, symline) != CFG_SUCC)
	    return(0);				/* No symlinks.		*/
	if ( (ptr = strstr(symline, inst_root)) == NULL)
	    return(0);				/* 'inst_root' not found*/
	foundFlag = 69;				/* Set flag that some chg*/
	while (ptr != NULL) {
	    strLen = ptr - symline;
	    strncpy(symline2, symline, strLen);		/* Put 1st of symlk*/
	    symline2[strLen] = '\0';			/* Terminate it.   */
	    strcat(symline2, inst_ptf);			/* Insert inst_ptf */
	    strcat(symline2, ptr);			 /* Finish symlk ln */
	    strcpy(symline, symline2);			/* Replace symline */
	    ptr2 = ptr + strlen(inst_root) + strlen(inst_ptf); /* point past*/
	    ptr = strstr(ptr2, inst_root);		/* Look for another*/
	}
	strcpy(symline2, "  symlinks = ");	/* Construct new symlinks */
	strcat(symline2, symline);		/*  line to replace old.  */
	delStanza(stanza, "symlinks =");	/* Delete old symlinks ln */
	addToStanza(symline2);			/* Add new symlinks line  */
	return(0);

}	/* End of checkLink() ***  */

int
addToStanza(valbuf)
	/********************************************************************/
	/* This function puts contents of valbuf at the end of the stanza.  */
	/********************************************************************/
char	*valbuf;
{
	char	*ptr;

	ptr = 1 + strstr(stanza, "\n\n");	/* Find 2 new lines at end. */
	if (ptr == 1)
	    strcat(stanza, valbuf);
	else
	    strcpy(ptr, valbuf);		/* Put new text at end.	    */
	strcat(stanza, "\n\n");			/* Put 2 new lines back in. */
	return(0);

}	/* End of addToStanza() **  */

delStanza(stanza, att)

char *stanza;			/* Points to a stanza buffer.		*/
char *att;			/* Name of attribute to delete.		*/
{
    char *line;			/* Point to each "line" in the stanza.	*/
    char *temp;			/* Utility character pointer.		*/
    char *buf;			/* Buffer to build updated stanza.	*/
    int  attLen;		/* Length of the attribute name.	*/
    
    /********* Allocate memory for the stanza buffer. *******************/
    if ((buf = malloc(strlen(stanza) + 1)) == NULL) {
	printf("%s: Error allocating a stanza deletion buffer.\n", whoami);
	exit(1);
    }

    /**** Force the first byte of the buffer to be a null, and get the ***
     **** length of the attribute name for compares.			*/
    *buf = '\0';
    attLen = strlen(att);

    /********* Point to the first "line" in this stanza. ****************/
    line = strtok(stanza, "\n");
    while (line != NULL) {
	 /*-------------------------------------------------------------+
	 | If the line does not contain the named attribute, it is OK	|
	 | to append it to the stanza buffer.  Skip past white space,	|
	 | then compare to the supplied name.  If the compare fails,	|
	 | append the line to the new stanza.				|
	 +-------------------------------------------------------------*/
	for (temp = line; *temp != '\0'; temp++)
	    if (*temp != ' ' && *temp != '\t') break;

	if (strncmp(temp, att, attLen))
	    strcat(strcat(buf, line), "\n");

	/******* Attempt to get the next line in the stanza. **********/
	line = strtok(NULL, "\n");

    } /* Done processing stanza lines. */

    /* Append a newline to the buffer, then copy the buffer over the    *
     * supplied stanza, free the buffer, and return.			*/
    strcat(buf, "\n");
    strcpy(stanza, buf);
    free(buf);
    
    return;

} /* End of delStanza(). */

		     /**/
/* The routines below used to be in a library, but that library was **
** discontinued for the 3.1 release.				    */
/********************************************************************/
/*                                                                  */
/* Module Name:  cfgcread                                           */
/*                                                                  */
/* Descriptive Name:  Attribute File Read Routine                   */
/*                                                                  */
/* Copyright:  Copyright IBM Corporation 1984                       */
/*                                                                  */
/* Function:  This routine will read the next stanza from an open   */
/*      attribute file.                                             */
/*                                                                  */
/* Normal Return:                                                   */
/*      CFG_SUCC - successful completion                            */
/*      CFG_EOF  - end of file reached                              */
/*                                                                  */
/* Error Return:                                                    */
/*      I/O errors returned by system calls                         */
/*      CFG_SZNF - requested stanza not found                       */
/*      CFG_BFSZ - requested stanza larger than nbyte               */
/*                                                                  */
/********************************************************************/

#define DEFAULT "default:"

static int
cfgcread(sfptr,buf,nbyte)

CFG__SFT *sfptr;
char *buf;
int  nbyte;
{
    int c;                     /* temporary character storage */
    int i = 0;                  /* index variable */
    int rc = 0;                 /* return code */
    char namebuf[MAXDIR];       /* stanza name buffer */
    struct {                    /* flags */
	unsigned nmfnd : 1;     /* name found */
	unsigned snfnd : 1;     /* stanza end found */
	unsigned one_nl : 1;    /* one newline found */
    } flag;

    flag.nmfnd = 0;                     /* initialize loop flag         */
    while (!flag.nmfnd) {               /* while name not found         */
	i = 0;                          /* initialize name buffer index */
					/* while delimiter not found    */
	while ((c = getc(sfptr->sfile)) != ' ' && (c != '\n')) {
	    if (feof(sfptr->sfile)) {   /* return on end of file        */
		flag.nmfnd = 1;
		break;
	    }
	    /* if 1st char is an * this is a comment, so read to line end. */
	    if (c == '*' && i == 0) {
	     while ((c = getc(sfptr->sfile)) != '\n') {
	       if (feof(sfptr->sfile)) {   /* return on end of file */
		flag.nmfnd = 1;
		break;
	       }
	     }
	     break;
	    }
					/* if name hasn't overflowed    */
	    if (i < sizeof(namebuf)-2) {
		namebuf[i++] = c;       /* add character to name        */
		if (c == ':')           /* if name terminator           */
		    flag.nmfnd = 1;     /* set up to terminate loop     */
	    }
	    else                        /* else                         */
		flag.nmfnd = 0;         /* continue looping             */
	}
    }
    if (feof(sfptr->sfile))             /* return on end of file        */
	rc = CFG_EOF;
    else {
	ungetc(c,sfptr->sfile);         /* put back space or newline    */
	namebuf[i] = '\0';              /* terminate stanza name        */

	if (i < nbyte) {                /* if stanza name < buffer size */
	    strcpy(buf,namebuf);        /* move stanza name to buffer   */
	    flag.snfnd = 0;             /* initialize loop flags        */
	    flag.one_nl = 0;		/* while end of stanza not found*/
					/*   and buffer space left      */
	    while ((!flag.snfnd) && (i < nbyte)) {
		c = getc(sfptr->sfile); /* get character from file.     */
					/* if end of file               */
		if (feof(sfptr->sfile)) {
		    flag.snfnd = 1;     /* set up loop termination      */
		    buf[i++] = '\n';    /* terminate stanza correctly   */
		}
		else {
		    buf[i++] = c;       /* store character in buffer    */
		    if (c == '\n')      /* if char is newline           */
					/* if second in a row set up    */
					/*   to terminate               */
			if (flag.one_nl) flag.snfnd = 1;
					/* else indicate one found      */
			else flag.one_nl = 1;
					/* else not a newline           */
		    else { if (c > ' ') flag.one_nl = 0;}
		}
	    }
	    if (i == nbyte) {
		rc = CFG_SZBF;          /* return buffer too small      */
		i--;                    /* prepare to insert terminator */
	    }
	    else
		rc = CFG_SUCC;          /* indicate successful read     */
	    buf[i] = '\0';              /* terminate buffer with null   */
	}
	else
	    rc = CFG_SZBF;              /* return buffer too small      */
    }
    if (ferror(sfptr->sfile)) {         /* check for error on read      */
	rc = CFG_UNIO;                  /* set return code              */
	clearerr(sfptr->sfile);         /* reset error indication       */
    }
    return(rc);
}

/********************************************************************/
/*                                                                  */
/* Module Name:  cfgcopsf                                           */
/*                                                                  */
/* Descriptive Name:  Attribute File Open Routine                   */
/*                                                                  */
/* Copyright:  Copyright IBM Corporation 1984                       */
/*                                                                  */
/* Function:  This routine will open an attribute file for update.  */
/*      It allocates a structure which contains the file pointer    */
/*      and space for a pointer to a default buffer which will be   */
/*      allocated by cfgcread if it is required.                    */
/*                                                                  */
/* Error Return:                                                    */
/*      NULL pointer                                                */
/*                                                                  */
/********************************************************************/

static CFG__SFT *
cfgcopsf(fname)

char *fname;

{
    FILE *fptr;
    CFG__SFT *sfptr;
    CFG__SFT *retval = NULL;

    fptr = fopen(fname,"r");
    if (fptr != NULL) {
	sfptr = (CFG__SFT *)malloc(sizeof(CFG__SFT));
	if (sfptr != NULL) {
	    sfptr->sfile = fptr;
	    sfptr->defbuf = NULL;
	    retval = sfptr;
	}
	else {
	    fclose(fptr);
	}
    }
    return(retval);
}
		/**/
static int
cfgcskwd(keyword,buf,ptr)

char *keyword;        /* keyword to scan for */
char *buf;            /* pointer to buffer containing stanza */
char *ptr;            /* ptr to string of max value length */
{                          /* begin scan for keyword & return value fn */

int rc = CFG_SUCC;         /* return code */
char *returnval;           /* ptr to string to return value */
int len;                   /* length of keyword */
int cmp;		   /* compare result */
int match = FALSE;         /* match found -- yes or no */
int i;                     /* working index */
char *sptr;                /* working pointer */

returnval = ptr;           /* return ptr to string or NULL */
len = strlen (keyword);    /* len is length of keyword, not counting \0 */
sptr = buf;                /* start at beginning of stanza */

sptr = strchr (sptr,'\n'); /* scan past stza name to first newline */
sptr++;                    /* point to first character after newline */
			   /* loop through stanza looking for match */
while ((*sptr != '\0') && (match == FALSE))
{
   while ((*sptr <= ' ') && (*sptr != '\n') && (*sptr != '\0')) sptr++;
			   /* ignore comment lines and blank lines */
   if ((*sptr != '*') && (*sptr != '\n') && (*sptr != '\0'))
   {
	 if (( cmp = strncmp (sptr,keyword,len)) == 0)
	 {                 /* entire string matches */
			   /* check that blank or `=' follows keyword in
			      stanza */
	    i = 0;
	    while ((i < len) && (*sptr != '\0')) { sptr++; i++; }

	    if ((*sptr == '=') || (*sptr == ' '))
	    {              /* match found */
	       match = TRUE;
			   /* scan to value */
	       while ((*sptr == ' ') || (*sptr == '=')) sptr ++;

			   /* write value to return string */
	       i = 0;
	       while ((*sptr != '\n') && (i < MAXVAL))
	       {
		  returnval[i++] = *sptr;
		  sptr++;
	       }
	       if (i < MAXVAL)
		  returnval[i] = '\0';     /* add terminating \0 */
	       else
		  returnval[--i] = '\0';   /* add terminating \0 */
	    }
	    else if (*sptr != '\0') {
	       sptr = strchr(sptr,'\n');
	       sptr++;
	    }
	 }
	 else if (*sptr != '\0') {
	    sptr = strchr(sptr,'\n');
	    sptr++;
	 }
   }
   else if (*sptr != '\0') {
      sptr = strchr(sptr,'\n');
      sptr++;
   }
}
if (match == FALSE)
   rc = ~CFG_SUCC;

return (rc);
}

/***** End of geninptf *** End of geninptf *** End of geninptf *****/
