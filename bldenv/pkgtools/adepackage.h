/* @(#)87       1.3  src/bldenv/pkgtools/adepackage.h, pkgtools, bos412, GOLDA411a 1/29/93 15:49:38 */
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

#ifndef _h_ADEPACKAGE
#define _h_ADEPACKAGE
#define COMMANDNAME "adepackage"

#include <limits.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>


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

typedef struct {
  char device_name[PATH_MAX+1];
  int blocksize;
  int tapeflag;
  int tape_fd;
} Tapeinfo;

#define TAR    1
#define BACKUP 2

/*
  Function prototypes for public functions
*/

int init_tape(Tapeinfo *);

int write_file_to_tape(Tapeinfo *, Fileinfo *);

void write_tape_eot(Tapeinfo *);

int findfile(char *, char **, int, char *, struct stat *, char *cksum);

#endif
