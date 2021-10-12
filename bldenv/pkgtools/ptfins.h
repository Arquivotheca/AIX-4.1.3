/* @(#)22	1.7  src/bldenv/pkgtools/ptfins.h, pkgtools, bos412, GOLDA411a  10/11/94  10:03:59 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <varargs.h>
#include <errno.h>
#include <sys/stat.h>
#include "ade.h"

#define BUFSIZE		2048
#define TOP		"TOP"                   /* environment variable */
#define BLDCYCLE	"BLDCYCLE"             /* environment variable*/
#define MAXARGS		20			/* arbitrary */
#define STANZA		0			/* arbitrary */
 
typedef struct {
       struct stat st;
 } Stat;

/*-----------------------
| Function Declarations	|
-----------------------*/
void	usage();
void	printLine( InsEntry *insentry, char typechar, FILE * outFp, char *fileName);
void	invEntry(char *top, char *option, char *bldcycle, char *insfile);
void	printInslist(FILE *fileptr, char *flibptr, InsEntry *finsentry, char *fname);
char	*libcase(char *fileName);
int	processLine(char *fname, char *insfile, char *option_name);
char	getNewType(char typeChar);

/*-----------------------------------------------
| Message Variables - see mkodmextractmsg.c	|
-------------------------------------------------*/
extern int errno;
extern char *File_Open_Failed ;
extern char *Missing_Opt;
extern char *No_Env_Var_TOP;
extern char *noMatch;
extern char *Usage;
extern char *commandName;
