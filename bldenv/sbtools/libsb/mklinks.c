static char sccsid[] = "@(#)45  1.1  src/bldenv/sbtools/libsb/mklinks.c, bldprocess, bos412, GOLDA411a 4/29/93 12:22:03";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 * ODE 2.1.1
 */

# include <sys/param.h>
# include <sys/stat.h>
# include <sys/errno.h>
# include <sys/time.h>
# include <ode/odedefs.h>
#ifdef INC_DIRENT
# include <sys/dirent.h>
#endif
# define  CERROR	(-1)
# define  FALSE		0
# define  TRUE		1
#ifdef NO_DIRENT
#define dirent direct
#endif
    extern int 	  errno;
    extern char * prog;

#ifndef S_ISDIR
# define	S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
# define	S_ISREG(m)	(((m)&S_IFMT) == S_IFREG)
#endif
#ifndef S_ISLNK
# define	S_ISLNK(m)	(((m)&S_IFMT) == S_IFLNK)
#endif

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
	  ui_print ( VFATAL, "stat %s: %s\n", curpath, errmsg(-1));
	  return ( CERROR );
	}
      }
      else if (S_ISDIR(st.st_mode))
        return(FALSE);
    }

    if (cmpfunc != NULL)
      return(TRUE);

    if (mkdir(curpath, 0777) < 0) {
      ui_print ( VFATAL, "mkdir %s: %s\n", curpath, errmsg(-1));
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
	ui_print ( VFATAL, "stat %s: %s\n", curpath, errmsg(-1));
	return(CERROR);
      }
    }

    ui_print ( VCONT, "%s: mode %o\n", curpath, st.st_mode );

    if (!S_ISREG(st.st_mode))
      return(FALSE);

    if (stp == NULL) {
      stp = &bst;
      if (lstat(namebuf, stp) != 0) {
	ui_print ( VFATAL, "lstat %s: %s\n", namebuf, errmsg(-1));
	return(CERROR);
      }
    }
    
    ui_print ( VDEBUG, "%s: mode %o\n", namebuf, stp->st_mode );

    if (S_ISLNK(stp->st_mode)) {
      if (stat(namebuf, stp) != 0) {
	ui_print ( VFATAL, "stat %s: %s\n", namebuf, errmsg(-1));
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
      ui_print ( VFATAL, "lstat %s: %s\n", curpath, errmsg(-1));
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
    uquit( ERROR, FALSE, "stat %s: %s\n", srcpath, errmsg(-1));

  descend ( st.st_nlink - 2, isnew, query, recurse, overwrite,
	    namebuf, namebuf + 1,
	    srcpath, srcpath + strlen(srcpath),
	    curpath, curpath + strlen(curpath),
	    (srcpref != NULL) ? srcpref : NULL,
	    (srcpref != NULL) ? srcpref + strlen(srcpref): NULL,
	    cmpfunc, linkfunc );
}  								  /* mklinks */
