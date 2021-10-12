/* @(#)08	1.1  src/bldenv/pkgtools/mkodmextract.h, pkgtools, bos412, GOLDA411a  6/30/93  11:52:17 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <varargs.h>

#define BUFSIZE 30
#define OBJSIZE 2048
#define VALSIZE 2048
#define COMMANDNAME	"mkodmextract"
#define MAXARGS		20			/* arbitrary */
#define TRUE		1			/* arbitrary */
#define FALSE		0			/* arbitrary */
#define STANZA		0			/* arbitrary */

/*-----------------------
| Function Declarations	|
-----------------------*/

void	usage ();
void	fatal ();
FILE   *openFile (char *, char *);
void   getdescriptor (char *, char *);
char   *getkeys (char *);
char   getlineobj (char *, char *, FILE * );
char   *getfield (char *, char, char *);
char   verify (char *, char *, char );
void    vnoupdate(char *, char *);

/*---------------------------------------
| Message Variables - see mkodmextractmsg.c	|
---------------------------------------*/
extern int errno;
extern char *File_Open_Failed ;
extern char *Noupdate_Object_Class;
extern char *Unrecognized_Object_Class;
extern char *Invalid_Field;
extern char *Missing_Opt;
extern char *Usage;
extern char *Descriptor_Not_Found;
extern char *Keyname_Not_Found;
extern char *File_Open_Failed;
