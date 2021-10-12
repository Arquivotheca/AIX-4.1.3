static char sccsid[] = "@(#)18  1.8  src/bldenv/pkgtools/rename/genset/genset.c, pkgtools, bos412, GOLDA411a 3/20/92 11:19:35";
/*
* COMPONENT_NAME: genset.c
*	The purpose of this program is to create an install set of files
*	in 'set_<lppname> directory.  To do this, ins_exp is read, and
*	all files in stanzas are copied there, and all direcories built.
*
* FUNCTIONS:
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business machines Corp. 1991
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/access.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <sys/id.h>

/* Define includes that are needed by the "cfg" functions.  These were in
   include file & library that were discarded. Source at end of this file. */

#define CFG_SUCC  0		/* Successful termination.		*/
#define CFG_EOF   4004		/* End of file encountered.		*/
#define CFG_EXBF -4013		/* Input exceeds buffer size.		*/
#define CFG_SZNF -4015		/* Stanza not found.			*/
#define CFG_SZBF -4016		/* Stanza size exceeds buffer size.	*/
#define CFG_UNIO -4017		/* Unrecoverable I/O error.		*/
#define MAXVAL   512		/* Maximum size for keyword value.	*/
#define MAXKWD   75		/* Maximum keyword length.		*/
#define MAXDIR   300		/* Maximum directory path length.	*/

/**/
/* Get some other defines out of the way. */

#define		maxLen		300

typedef struct {
    FILE *sfile;
    char *defbuf;
    short *defmap;
} CFG__SFT;

/* Declare non-integer returning functions. */
char		*getwd();		/* Get working directory c function. */
char		*malloc();		/* Memory allocation c function.     */
CFG__SFT	*cfgcopsf();		/* Open stanza file.		     */

/* Declare some	global variables. */

static int	applyFlag;		/* On if 'apply' found in class.     */
static int	shareFlag;		/* On if curr path end in 'share'.   */
static int	rootFlag;		/* On if 'root' subdirectory found.  */
static int	fileFlag;		/* On if 'type = FILE' found.	     */
static int	oldFlag;		/* On if 'oldpath = ' found in stanza*/
static int	dirFlag;		/* On if 'type = DIRECTORY' found.   */
static int	realUser = ID_REAL;	/* Define value of real user ID.     */
static int	tpathFlag = FALSE;	/* Use tpath in system calls?	     */

CFG__SFT	*cfg;
char		cmd[650];		/* Miscellaneous cmd construct area. */
char		stanza[BUFSIZ];		/* This buffer also used for copying.*/
char		path[MAXDIR];		/* Output path name when appended to.*/
char		*TOP;			/* Path to the 'set' parent dir.     */
char		oldName[MAXDIR];	/* Source file name if old path.     */
char		lppName[MAXDIR];	/* lpp name found in 'lpp_name' file.*/
char		tpathName[MAXDIR];	/* name of tpath text, if used.      */
char		*whoami;		/* This pgm nm as got from ac.	     */
int		whoLen;			/* Length of 'whoami'.		     */
FILE		*fpLpp;			/* Pntr to 'lpp_name' file.	     */
DIR		*dirPtr;		/* Pntr to directory structure.      */
struct dirent	*dp;			/* Pntr to dir entry structure.      */

/*
   This program should be called with one argument.
   Use relative path names for files and dirs.
*/

main(ac,av)

int	ac;
char	*av[];
{
	int		rc;
	char		*ptr;		/* working pointer.		*/
	char		*bldenv;	/* Pntr to BLDENV, if used.     */
	char	source[maxLen];		/* Area to compose input path.  */
	char	target[maxLen];		/* Area to compose output path. */

	/* Set program name, and name length, for use in error messages. */
	whoami = ((whoami = strrchr(av[0], '/')) == NULL ? av[0] : whoami+1);
	whoLen = strlen(whoami);

	if (ac == 1)			/*********************************/
	    lppName[0] = '\0';		/* Allow lpp-name path to be a   */
	else				/* parameter or use getLpp().    */
	    strcpy(lppName, av[1]);	/*********************************/
	if ((bldenv = getenv("BLDENV")) != NULL) {
	    sprintf(tpathName, "%s/usr/bin/tpath", bldenv);
	    if (!access(tpathName, 4))
		tpathFlag = TRUE;
	}

	if ((TOP = getenv("TOP")) == NULL) {
	    printf("%s: TOP env variable not found.\n", whoami);
	    printf("%*c  Cannot process further.\n", whoLen, ' ');
	    exit(3);
	}
	getwd(path);			/* Put working dir in 'path'.	 */
	dirPtr = *malloc(sizeof(DIR));
	dp     = (struct dirent *) malloc(sizeof(struct dirent));

	if (strstr(path, "/share") != NULL)
		++shareFlag;		/* Path name end with 'share'.	 */
	strcpy(path, TOP);		/* Now construct path to set dir.*/
	if (shareFlag)
	    strcat(path, "/sets_");
	else
	    strcat(path, "/set_");
	rootFlag = seekDir("root");	/* Call function to look for root*/
	if (lppName[0] == '\0')
	    getLpp();			/* Get lppName from 'lpp_name'	 */
	strcat(path, lppName);		/* Complete set dir name.	 */
	if (access(path, 0) == 0) { 	/* If set dir already exists,	 */
	    strcat(cmd, "rm -rf ");
	    strcat(cmd, path);
	    if (rc = system(cmd)) {	/*    If delete fails, give up.  */
		printf("%s: Cannot remove dir \"%s\" that already existed.\n",
			whoami, path);
		printf("%*c  Please remove the directory, then try again.\n",
			whoLen, ' ');
		exit(-1);
	    }
	}
	/* Set the id to the calling ID. This program was invoked as root */
	rc = setuid(getuidx(realUser));
	if (rc) {
		printf("%s: Cannot reset user ID \n", whoami);
		exit(8);
	    } 
	/****** Make directory read & write for usr and group. ******/
  	rc = mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	if (rc) {
	    printf("%s: Cannot create %s directory.\n", whoami, path);
	    exit(-1);
	}
	if ((fpLpp = fopen("lpp_name", "r")) == NULL) {
	    printf("%s: Can't open \"lpp_name\" file.\n", whoami);
	    printf("%*c  Cannot process further.\n", whoLen, ' ');
	    exit(2);
	}
	strcpy(cmd, "cp -p lpp_name ");/* Compose copy command to put	 */
	strcat(cmd, path);		/* lpp_name file into set dir.	 */
	if (rc = system(cmd)) 		/* copy lpp_name file.		 */
	    printf("%s: Cannot copy \"lpp_name\" to %s.\n", whoami, path);

	/******* Copy liblpp.a file(s) to appropriate directory(s). ******/
	if (shareFlag) {			/** Share liblpp.a copy **/
	    strcpy(target, path);   strcat(target, "/usr");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/share");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/lpp");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/");  strcat(target, lppName);
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    if (tpathFlag) {
		sprintf(cmd, "cp -p `%s %s%s%s%s` %s",
		    tpathName, TOP, "/ship/usr/share/lpp/", lppName,
		    "/liblpp.a", target);
	    }
	    else {
		strcpy(cmd, "cp -p ");  strcat(cmd, TOP);
		strcat(cmd, "/ship/usr/share/lpp/");  strcat(cmd, lppName);
		strcat(cmd, "/liblpp.a "); strcat(cmd, target);
	    }
	    if (rc = system(cmd))
		printf("%s: Cannot copy %s/ship/usr/share/%s/liblpp.a to set dir.\n",
			whoami, TOP, lppName);
	}
	else
	if (rootFlag) {				/** root liblpp.a copy. **/
	    strcpy(target, path);   strcat(target, "/usr");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/lpp");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/");  strcat(target, lppName);
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/inst_root");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    if (tpathFlag) {
		sprintf(cmd, "cp -p `%s %s%s%s%s` %s",
		    tpathName, TOP, "/ship/usr/lpp/", lppName,
		    "/inst_root/liblpp.a", target);
	    }
	    else {
		strcpy(cmd, "cp -p ");  strcat(cmd, TOP);
		strcat(cmd, "/ship/usr/lpp/");  strcat(cmd, lppName);
		strcat(cmd, "/inst_root/liblpp.a "); strcat(cmd, target);
	    }
	    if (rc = system(cmd))
		printf("%s: Cannot copy %s/ship/usr/lpp/%s/inst_root/liblpp.a to set dir.\n",
			whoami, TOP, lppName);
            /* copy user lpp_name file to root.  This is same path  */
	    /* as where the liblpp.a file was put.                  */
	    strcpy(cmd, "cp -p "); 
	    strcat(cmd, "lpp_name ");  strcat(cmd, target);
	    if (rc = system(cmd))
		printf("%s: Cannot copy %s/lpp_name to set/root dir.\n",
			whoami, lppName);
	}
	if (!shareFlag) {			/** Normal liblpp.a copy */
	    strcpy(target, path);   strcat(target, "/usr");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/lpp");
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    strcat(target, "/");  strcat(target, lppName);
  	    mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP);
	    if (tpathFlag) {
		sprintf(cmd, "cp -p `%s %s%s%s%s` %s",
		    tpathName, TOP, "/ship/usr/lpp/", lppName,
		    "/liblpp.a", target);
	    }
	    else {
		strcpy(cmd, "cp -p ");  strcat(cmd, TOP);
		strcat(cmd, "/ship/usr/lpp/");  strcat(cmd, lppName);
		strcat(cmd, "/liblpp.a "); strcat(cmd, target);
	    }
	    if (rc = system(cmd))
		printf("%s: Cannot copy %s/ship/usr/lpp/%s/liblpp.a to set dir.\n",
			whoami, TOP, lppName);
	}

	cfg = cfgcopsf("ins_exp");	/* Open input stanza file.	 */
	if (cfg == NULL) {
	    printf("%s: Cannot find \"ins_exp\" file.\n", whoami);
	    printf("%*c  This file should have been created by mkinventory.\n",
			whoLen, ' ');
	    printf("%*c  Cannot process further.\n", whoLen, ' ');
	    exit(5);
	}
	while (rc != CFG_EOF) {		/* As long as there is another stanza*/
	    applyFlag = 0;
	    fileFlag  = 0;
	    oldFlag   = 0;
	    dirFlag   = 0;
	    rc        = procExp();	/* Process each ins_exp stanza.	 */
	}
	exit(0);
}
		  /**/
procExp()			/****************************************/
				/*     Process each ins_exp stanza.	*/
{				/****************************************/
	int	rc;			/* Exiting return code.		*/
	char	*nameLen;		/* Used to calc length file name*/
	char	source[maxLen];		/* Area to compose input path.  */
	char	target[maxLen];		/* Area to compose output path. */
	char	fileName[MAXDIR];	/* Filename as found in stanza. */

	rc = cfgcread(cfg, stanza, BUFSIZ);	/* Standard stanza read.*/
	if (rc == CFG_EOF)
		return (rc);
	if (rc != CFG_SUCC) {			/* Read error?		*/
	    printf("%s: Error reading stanza of ins_exp.\n", whoami);
	    return(-1);
	}
	rc = textParse();			/* Examine the stanza.	*/

	if (!applyFlag)				/* Found 'apply'?	*/
	    return(0);				/* Only process if true.*/

	nameLen = strchr(stanza, ':');		/* count chars to colon.*/
	strncpy(fileName, stanza, nameLen-stanza);
	fileName[nameLen-stanza] = '\0';	/* End file name.	*/
	    strcpy(target, path);		/* Compose file name as	*/
	    strcat(target, fileName);		/* path + stanza filenm.*/

	if (dirFlag) {				/* Directory stanza?	*/
	    rc = mkdir(target,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP); 
	    if (rc && errno != EEXIST)		/* Already exist err ok.*/
		printf("%s: Cannot create %s directory.\n", whoami, target);
	    return(0);
	}
	if (!fileFlag)				/* if not dir or file,	*/
	    return(0);				/*   do not process.	*/
	/**** Process a file. *** Process a file. *** Process a file. ***/
	strcpy(source, TOP);
	strcat(source, "/ship");
	if (oldFlag)				/* 'oldpath =' found?	*/
	    strcat(source, oldName);
	else
	    strcat(source, fileName);
	if (tpathFlag) {
		sprintf(cmd, "cp -p `%s %s` %s",
		    tpathName, source, target);
	}
	else {
	    strcpy(cmd, "cp -p ");		/* Construct copy cmd.	*/
	    strcat(cmd, source);
	    strcat(cmd, " ");
	    strcat(cmd, target);		/* append target path.	*/
	}
	if (rc = system(cmd))			/* Execute copy cmd.	*/
	    printf("%s: Could not execute \"%s\".\n", whoami, cmd);
	return(0);
}
		  /**/
static int			/*--------------------------------------*/
textParse()			/* Parse the lines of stanza.		*/
{				/*--------------------------------------*/
	char	strng[maxLen];	/* Work area for text after '='.	*/
	char	*token;		/* Work area for text bounded by comma. */
	char	valbuf[MAXVAL+25];

	if (cfgcskwd("type", stanza, valbuf) == CFG_SUCC) {
	    dirFlag   = (strstr(valbuf, "DIRECTORY"));
	    fileFlag  = (strstr(valbuf, "FILE"));
	    if (!dirFlag)  dirFlag  = (strstr(valbuf, "directory"));
	    if (!dirFlag)  dirFlag  = (strstr(valbuf, "Directory"));
	    if (!fileFlag) fileFlag = (strstr(valbuf, "File"));
	    if (!fileFlag) fileFlag = (strstr(valbuf, "file"));
	}

	if (cfgcskwd("class", stanza, valbuf) == CFG_SUCC) {
		strcpy(strng, valbuf);
		token =	strtok(strng,",");
		while (token !=	NULL) {
		    if (!strcmp("apply",token))	/* If apply found,	*/
			++applyFlag;		/*    set applyFlag.	*/
		    token = strtok(NULL, ",");
		}
	}
	/*** If oldpath attribute found, save old name & set oldFlag. ***/
	if (cfgcskwd("oldpath", stanza, oldName) == CFG_SUCC)
		++oldFlag;

	return(0);

} /* End of textParse(). */

		     /**/
getLpp()		/******************************************/
{			/*  Read lpp_name file into lppname var.  */
			/******************************************/
char	*ptr;		/*  Work pointer.			  */

	if ((fpLpp = fopen("lpp_name", "r")) == NULL) {
	   printf("%s: Can't open \"lpp_name\" file.\n", whoami);
	    printf("%*c  and lpp was not specified.\n", whoLen, ' ');
	    printf("%*c  Cannot process further.\n", whoLen, ' ');
	    exit(6);
	}
	fgets(lppName,sizeof(lppName), fpLpp); /* Read 1st line.	*/
	fgets(lppName,sizeof(lppName), fpLpp); /* Read 2nd line.	*/
	fclose(fpLpp);
	ptr = strpbrk(lppName, " .\t\n");      /* Ptr to end of name.   */
	*ptr = '\0';			       /* End string after name */
	return(0);
}
		     /**/
seekDir(name)			/*****************************************/
char	*name;			/*  Search for specified subdirectory.   */
{				/*****************************************/
	int		len;		/* Length of subdir name.	 */

	len = strlen(name);
	dirPtr = opendir(".");		/* Open current directory.	*/
	for (dp = readdir(dirPtr); dp != NULL; dp = readdir(dirPtr))
	    if (dp->d_namlen == len && !strcmp(dp->d_name, name)) {
		closedir(dirPtr);	/* No found, close & return.	*/
		return (1);
	    }
	closedir(dirPtr);		/* Found it! Close * return.	*/
	return (0);
}

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
    char namebuf[MAXVAL];       /* stanza name buffer */
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
}	/* End of cfgcread()	 */

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
int match = FALSE;         /* match found -- yes or no */
int cmp;                   /* return from compare routine */
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

/**** End of genset *** End of genset *** End of genset *** End of genset *****/
