/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: descend
 *		makelink
 *		mklinks
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: mklinks.c,v $
 * Revision 1.8.4.3  1993/04/08  19:53:13  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  19:53:01  damon]
 *
 * Revision 1.8.4.2  1993/01/13  16:43:49  damon
 * 	CR 382. Removed INC_DIRENT NO_DIRENT S_ISDIR S_ISLNK S_ISREG
 * 	[1993/01/05  21:02:21  damon]
 * 
 * Revision 1.8.2.5  1992/12/03  17:21:16  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:26  damon]
 * 
 * Revision 1.8.2.4  1992/11/06  16:23:09  damon
 * 	CR 296. Removed TRUE/FALSE defs
 * 	[1992/11/06  16:22:59  damon]
 * 
 * Revision 1.8.2.3  1992/09/24  19:01:55  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/21  20:35:03  gm]
 * 
 * Revision 1.8.2.2  1991/12/31  17:01:26  damon
 * 	ported to sparc_sunos
 * 	[1991/12/31  16:48:01  damon]
 * 
 * Revision 1.7.1.2  1991/12/17  15:26:47  damon
 * 	Ported to hp300_hpux
 * 	[1991/12/17  14:31:42  damon]
 * 
 * Revision 1.7  1991/12/05  21:05:22  devrcs
 * 	Removed include of dirent.h
 * 	[91/02/03  11:50:00  damon]
 * 
 * 	Added # include <dirent.h> explicitly.
 * 	[91/02/03  11:33:23  damon]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:18  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	Also a fairly thorough once-over to make this routine comply
 * 	with the changes in mklinks.c the program.
 * 	[91/01/08  12:16:33  randyb]
 * 
 * 	Add test so -copy would overwrite existing files even if they exist if
 * 	the -over option is thrown.
 * 
 * 	Added -debug level to the output so -v and -debug do not give the same
 * 	information.
 * 	[90/11/07  16:53:23  randyb]
 * 
 * Revision 1.5  90/10/07  20:03:41  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:36  gm]
 * 
 * Revision 1.4  90/08/09  14:23:20  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:32:20  gm]
 * 
 * Revision 1.3  90/07/17  12:36:50  devrcs
 * 	More changes for gcc.
 * 	[90/07/04  22:30:53  gm]
 * 
 * Revision 1.2  90/05/24  23:12:31  devrcs
 * 	Added missing set of braces.
 * 	[90/05/10  19:54:39  gm]
 * 
 * 	Created.
 * 	[90/05/06            gm]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)93  1.1  src/bldenv/sbtools/libode/mklinks.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:29";
#endif /* not lint */

#include <dirent.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <ode/interface.h>
#include <ode/odedefs.h>
#define  CERROR	(-1)
    extern int 	  errno;
    extern char * prog;

int
makelink ( stp, isnew, isdir, overwrite, namebuf, srcpath,
	   curpath, srcpref, cmpfunc, linkfunc )
    
    struct 	stat *stp;
    int 	  isnew, isdir, overwrite;
    char 	* namebuf, *srcpath, *curpath, *srcpref;
    int        (* cmpfunc)();
    int        (* linkfunc)();

{
    char 	  buf1 [MAXPATHLEN];
    struct 	stat st, bst;
    int 	  status;

  ui_print ( VDEBUG, "makelink ( stp %x isnew %d isdir %d overwrite %d\n",
      		      stp, isnew, isdir, overwrite );
  ui_print ( VCONT,
		 "namebuf %s\n   srcpath %s\n   curpath %s\n   srcpref %s )\n",
	          namebuf, srcpath, curpath, srcpref ? srcpref : NULL );

  if (isdir) {
    if (!isnew) {
      if (stat(curpath, &st) < 0) {
	if (errno != ENOENT) {
	  ui_print ( VFATAL, "stat %s: %s\n", curpath, strerror(errno));
	  return ( CERROR );
	}
      }
      else if (S_ISDIR(st.st_mode))
        return(FALSE);
    }

    if (cmpfunc != NULL)
      return(TRUE);

    if (mkdir(curpath, 0777) < 0) {
      ui_print ( VFATAL, "mkdir %s: %s\n", curpath, strerror(errno));
	return(CERROR);
    }

    ui_print ( VDETAIL, "%s: created directory\n", namebuf);
    return(TRUE);
  }

  if (( !isnew && lstat ( curpath, &st ) == 0 ) && !overwrite ) {
    if (cmpfunc == NULL) {
      ui_print ( VDETAIL, "%s already exists\n", namebuf );
      return(FALSE);
    } /* if */

    ui_print ( VDEBUG, "%s: mode %o\n", curpath, st.st_mode );

    if (S_ISLNK(st.st_mode)) {
      if (stat(curpath, &st) != 0) {
	ui_print ( VFATAL, "stat %s: %s\n", curpath, strerror(errno));
	return(CERROR);
      }
    }

    ui_print ( VCONT, "%s: mode %o\n", curpath, st.st_mode );

    if (!S_ISREG(st.st_mode))
      return(FALSE);

    if (stp == NULL) {
      stp = &bst;
      if (lstat(namebuf, stp) != 0) {
	ui_print ( VFATAL, "lstat %s: %s\n", namebuf, strerror(errno));
	return(CERROR);
      }
    }
    
    ui_print ( VDEBUG, "%s: mode %o\n", namebuf, stp->st_mode );

    if (S_ISLNK(stp->st_mode)) {
      if (stat(namebuf, stp) != 0) {
	ui_print ( VFATAL, "stat %s: %s\n", namebuf, strerror(errno));
	return(CERROR);
      }
    }
    
    ui_print ( VCONT, "%s: mode %o\n", namebuf, stp->st_mode );

    if (!S_ISREG(stp->st_mode))
      return(FALSE);
    
    ui_print ( VCONT, "%s: size %d vs %d\n",
			curpath, st.st_size, stp->st_size );

    if (st.st_size != stp->st_size || st.st_size == 0)
      return(FALSE);
    status = (*cmpfunc)(namebuf, curpath);
    ui_print ( VCONT, "%s: cmp %d\n", namebuf, status );

    if (status != 0)
      return((status == CERROR) ? CERROR : FALSE);

    (void) unlink(curpath);
  } /* if */
  
  else {
    if (( !isnew && errno != ENOENT ) && !overwrite ) {
      ui_print ( VFATAL, "lstat %s: %s\n", curpath, strerror(errno));
      return(CERROR);
    }
    if (cmpfunc != NULL)
      return(FALSE);
  }

  if (srcpref != NULL && *srcpref != '\0') {
    if (concat(buf1, sizeof(buf1), srcpref, srcpath, NULL) == NULL) {
      ui_print ( VFATAL, "%s: path too long\n", buf1);
      return(FALSE);
    }

    ui_print ( VDEBUG, "linkfunc(%s,\n   %s)\n", buf1, curpath );

    if ((*linkfunc)(buf1, curpath) == CERROR)
      return(CERROR);
  } /* if */
  
  else {
    ui_print ( VDEBUG, "linkfunc(%s,\n   %s)\n", srcpath, curpath );

    if ((*linkfunc)(srcpath, curpath) == CERROR)
      return(CERROR);
  }

  ui_print ( VDETAIL, "%s: created\n", namebuf);
  return(FALSE);
}  								 /* makelink */



void
descend ( ndirs, isnew, query, recurse, overwrite, namebuf, endname, srcpath,
	  endsrc, curpath, endcur, srcpref, endpref, cmpfunc, linkfunc )

    int 	  ndirs;
    int 	  isnew;
    int 	  query;
    int 	  recurse;
    int 	  overwrite;
    char 	* namebuf,
		* endname;
    char 	* srcpath, 
		* endsrc;
    char 	* curpath, 
		* endcur;
    char 	* srcpref, 
		* endpref;
    int        (* cmpfunc)();
    int        (* linkfunc)();

{
    struct 	stat st;
    /* struct 	direct *dp; 			   old name, OSF uses dirent */
    struct 	dirent *dp;
    DIR 	* dirp;
    int 	  subisnew;
    char 	* np, *sp, *cp, *pp;
    char 	  buf [MAXPATHLEN];

  ui_print ( VDEBUG, "namebuf: %s\n", namebuf );
  ui_print ( VCONT,
	    "descend ( ndirs %d isnew %d query %d recurse %d overwrite %d\n",
	     ndirs, isnew, query, recurse, overwrite );
  ui_print ( VCONT, "srcpath %s\n   curpath %s\n   srcpref %s)\n",
		     srcpath, curpath, srcpref ? srcpref : NULL);

  if ((dirp = opendir(namebuf)) == NULL) {
    perror(namebuf);
    return;
  }

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }
    np = concat(endname, namebuf+MAXPATHLEN-endname, "/", dp->d_name, NULL);

    if (np == NULL) {
      ui_print ( VWARN, "%s: path too long\n", namebuf);
      continue;
    }
    sp = concat(endsrc, srcpath+MAXPATHLEN-endsrc, "/", dp->d_name, NULL);

    if (sp == NULL) {
      ui_print ( VWARN, "%s: path too long\n", srcpath);
      continue;
    }
    cp = concat(endcur, curpath+MAXPATHLEN-endcur, "/", dp->d_name, NULL);

    if (cp == NULL) {
      ui_print ( VWARN, "%s: path too long\n", curpath);
      continue;
    }

    if (ndirs == 0) {
      (void) makelink (( struct stat *)NULL, isnew, FALSE, overwrite, namebuf,
			 srcpath, curpath, srcpref, cmpfunc, linkfunc );
      continue;
    }

    if (lstat(namebuf, &st) != 0) {
      perror(namebuf);
      continue;
    }

    if (!S_ISDIR(st.st_mode)) {
      (void) makelink ( &st, isnew, FALSE, overwrite, namebuf, srcpath,
			curpath, srcpref, cmpfunc, linkfunc );
      continue;
    }

    ndirs--;

    if (query) {
      (void) concat (buf, sizeof(buf), "Link directory ", namebuf, " ?", NULL);
      if (!getbool(buf, TRUE))
	continue;
    }

    if (!recurse)
      continue;

    subisnew = makelink( &st, isnew, TRUE, overwrite, namebuf, srcpath,
			 curpath, srcpref, cmpfunc, linkfunc);

    if (subisnew == CERROR)
      continue;

    if (subisnew && cmpfunc != NULL)
      continue;

    if (srcpref != NULL) {
      pp = concat (endpref, srcpref+MAXPATHLEN-endpref, "../", NULL);

      if (pp == NULL) {
	ui_print ( VFATAL, "%s: path too long\n", srcpref);
	return;
      }
    }
    
    else
      pp = NULL;

    descend ( st.st_nlink-2, subisnew, query, recurse, overwrite, namebuf, np,
	      srcpath, sp, curpath, cp, srcpref, pp, cmpfunc, linkfunc );

    if (endpref != NULL)
      *endpref = '\0';
  } /* for */

  (void) closedir(dirp);
}  								  /* descend */



mklinks ( isnew, query, recurse, overwrite, srcpath, curpath, srcpref,
	  cmpfunc, linkfunc)

    int 	  isnew, query, recurse, overwrite;
    char 	* srcpath, *curpath, *srcpref;
    int        (* cmpfunc) ();
    int        (* linkfunc) ();

{
    char 	  namebuf[MAXPATHLEN];
    struct 	stat st;

  namebuf[0] = '.';
  namebuf[1] = '\0';

  if ( stat ( namebuf, &st ) < 0 )
    uquit( ERROR, FALSE, "stat %s: %s\n", srcpath, strerror(errno));

  descend ( st.st_nlink - 2, isnew, query, recurse, overwrite,
	    namebuf, namebuf + 1,
	    srcpath, srcpath + strlen(srcpath),
	    curpath, curpath + strlen(curpath),
	    (srcpref != NULL) ? srcpref : NULL,
	    (srcpref != NULL) ? srcpref + strlen(srcpref): NULL,
	    cmpfunc, linkfunc );
}  								  /* mklinks */
