/* @(#)05       1.10  src/bldenv/pkgtools/ins.h, pkgtools, bos412, GOLDA411a 8/30/94 15:08:57 */ 
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

#include <stdio.h>
#include <varargs.h>
#include <sys/param.h>

#define  INS_MAXPATHS 1000
#define  INS_SUCCESS 0
#define  INS_ERROR 1
#define  IDSIZE 10
#define  EXCEPTION_LIST "EXCEPTION_LIST"
#define  DB_FILE_IN "DB_FILE_IN"
#define  ODE_TOOLS "ODE_TOOLS"

struct Table_Nam {
                 int idnum;
                 char idname[IDSIZE];
                 int groupnum;
                 char groupname[IDSIZE];
};

/*---------------------------------------
| Function Declarations		 	|
---------------------------------------*/

void	createPrqPaths();
void	fatal ();
void	inserror ();
void	warning ();
char	*xmalloc (int);
FILE	*openFile (char *, char *);
FILE	*openList();
void	prereqMatch (char *, int *, char [][MAXPATHLEN+1], char [][2*MAXPATHLEN+2], int);
void	usage ();
void	writeDb ();
char	*getFileSys (char *);

/*---------------------------------------
| Message Variables - see ilvmsg.c	|
---------------------------------------*/

extern	char *noInsCurDir;
extern	char *noInsSearchDir;
extern	char *Usage;
extern	char *Malloc_Error;
extern	char *BosNotFound;
extern	char *prereqInslistNotFound;
extern	char *insOpenFailed;
extern	char *OpenError;
extern	char *linkCrossFileSys;
extern	char *uidNotFound;
extern	char *gidNotFound;
extern	char *invalidTableEnt;
extern	char *dbFileError;
extern	char *statFailed;
extern	char *commandName;
extern	char *numInslistsExceededLimit;
extern	char *numOptionsExceededLimit;
extern	char *tcbFlagNotSetRoot;
extern	char *tcbFlagNotSetSys;
