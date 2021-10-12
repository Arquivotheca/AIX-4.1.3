/* @(#)55       1.4  src/bldenv/pkgtools/adecopyright.h, pkgtools, bos41J, 9519A_all 4/11/95 12:53:24 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <locale.h>
#include <sys/param.h>

#define ENOENT 2
#define MAXBUFSIZE 1024
#define MAXIDLINE  132
#define MAXNUMKEYS 1000
#define MAXKEYSIZE 80       /* Maximum lgth of the substitution keyword */
#define MAXSUBSIZE 1024     /* Maximum length of quoted substitution text */
#define MAXSUBLINE 2048     /* Maximum length of substitution pair line */
#define MAXSRCLINE 10240    /* Maximum length of processed source line */
#define KEYPATTERN "%%_"    /* Recognized keywork pattern */
#define KEYOEMHEADER  "OEM_HEADER"
#define KEYHEADER     "IBM_HEADER"
#define KEYOEMFOOTER  "OEM_FOOTER"
#define KEYFOOTER     "IBM_FOOTER"
#define KEYKEYS    "KEYS"
#define KEYIBM     "%%_IBM"
#define KEYPROG    "PROG#"
#define CR_ERROR 1
 
typedef struct {
     struct stat st;
     char linkname[PATH_MAX+1];
 } Stat;
/*--------------------------------------
 Functiontruct Declarations	 	|
---------------------------------------*/

FILE	*openFile (char *, char *);
int     compare();
void    qsort();
void	usage ();
void    fatal();

/*---------------------------------
| Message Variables - see crmsg.c |
---------------------------------*/

extern char *commandName;
extern char *Usage;
extern char *noIDinTable;
extern char *keyWriteErr;
extern char *keyPatternErr;
extern char *crReadErr;
extern char *noKeyCrFile;
extern char *quoteError1;
extern char *quoteError2;
