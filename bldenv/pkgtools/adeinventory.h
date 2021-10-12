/* @(#)84       1.13  src/bldenv/pkgtools/adeinventory.h, pkgtools, bos412, GOLDA411a 8/22/94 15:38:26 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: toblocks
 *		
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

#include <sys/param.h>
#include <sys/stat.h>
#include "list.h"

#define COMMANDNAME "adeinv"
#define ADE_MAXPATHS 20
#define LPPNAMELEN 100
#define IDSIZE 10
#define MAXBUFFER (4*MAXPATHLEN)
#define		MINBLOCKS	8
#define		BLOCKSIZE	512
#define		MINBYTES	(MINBLOCKS*BLOCKSIZE)
#define		toblocks(x)	\
	( ( ((x)/(MINBYTES)) + ( ((x)%(MINBYTES)) ? 1 : 0) ) * (MINBLOCKS))

/*-----------------------------------------------------------------------
| LIBHASHTABLE can be modified as the number of libraries per update	|
| grows.  The goal is for each bucket to contain 5 or fewer entries	|
| assuming a random hash function.  This will keep search time minimum.	|
-----------------------------------------------------------------------*/
#define LIBHASHTABLE	15

struct Table_Nam {
                 int idnum;
                 char idname[IDSIZE+1];
                 int groupnum;
                 char groupname[IDSIZE+1];
                 };

typedef	struct {
	char	Dir[MAXPATHLEN+1];	
	off_t	insize;		
	off_t	tempsize;	
	char	inputOperator;	
	off_t	totsize;		
} DirEntry;

typedef struct {
	struct stat st;
	char linkname[PATH_MAX+1];
} Stat;

typedef struct {
	char filename[255];
	Stat f_st;
	char ship_name[PATH_MAX+1];
	int file_fd;
} Fileinfo;

typedef struct LibEntry {
	char		libname[MAXPATHLEN+1];
	off_t		libEntrySize;
	struct LibEntry	*nextLib;
} LibEntry;
	
/*-----------------------
| Global Variables	|
-----------------------*/

int	permtable = 0;
char	secDir[14] = "/etc/security";
char	reposDir[MAXPATHLEN+1];
char	savespaceDir[MAXPATHLEN+1];
List	*dirList;
char    *shipPaths[ADE_MAXPATHS];
int     num_paths;

FILE    *opinv_ptr;
FILE    *opal_ptr;
FILE    *opsize_ptr;
FILE    *optcb_ptr;
FILE    *perm_ptr;
FILE    *opxref_ptr;

/*---------------------------------------
| Message identifiers - see adeinvmsg.c	|
---------------------------------------*/

extern char *commandName;

#define CantOpenTableFile   0  
#define FileNotFound        1
#define CantResolveUid      2
#define CantResolveGid      3
#define CantGetOption       4
#define MultipleUid         5
#define MultipleGid         6
#define InvalidTableEnt     7
#define StatFailed         10
#define InvalidSizeInsize  11
#define NoValueForOperator 12
#define ACFloadError	   13
#define lppacfError        14
#define LibNotFound        15
#define ACFaddFailure	   16
