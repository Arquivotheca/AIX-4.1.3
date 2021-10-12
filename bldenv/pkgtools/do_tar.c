static char sccsid[] = "@(#)93  1.2  src/bldenv/pkgtools/do_tar.c, pkgtools, bos412, GOLDA411a 1/29/93 17:26:10";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: findgname
 *		finduname
 *		tartype
 *		write_tar_file
 *		
 *
 *   ORIGINS: 18,27,71
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/* $Source: /u/mark/src/pax/RCS/tar.c,v $
 *
 * $Revision: 1.2 $
 *
 * tar.c - tar specific functions for archive handling
 *
 * DESCRIPTION
 *
 *	These routines provide a tar conforming interface to the pax
 *	program.
 *
 * AUTHOR
 *
 *	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log:	tar.c,v $
 * Revision 1.2  89/02/12  10:06:05  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:38  mark
 * Initial revision
 * 
 */
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

#include "do_tar.h"

int write_tar_file ( Tapeinfo * tapeinfo, Fileinfo * fileinfo)
{
  char *p;
  char *name;
  char *prefix = (char *)NULL;
  int  i, sum;
  char hdr[BLOCKSIZE];

  memset(hdr, 0, BLOCKSIZE);

  name = fileinfo->filename;

  if (strlen(name) > 255) {
    fprintf(stderr, "ERROR: Filename too long: %s\n", name);
    return(-1);
  }

  /*
   * If the pathname is longer than 100 characters, but less than 255
   * we can split it up into the prefix and the filename
   */

  if (strlen(name) > 100) {
    prefix = name;
    name += 155;
    while (name > prefix && *name != '/') {
      name--;
    }

    /* no slash found - a big problem */
    if (name == prefix) {
      fprintf(stderr, "ERROR: Filename too long\n");
      return(-1);
    }
    *name++ = '\0';
  }

  if((fileinfo->f_st.st.st_mode & S_IFMT) == S_IFLNK) {
    strcpy(&hdr[157], fileinfo->f_st.linkname);
    fileinfo->f_st.st.st_size = 0;
  }

  strcpy(hdr, name);
  sprintf(&hdr[100], "%06o \0", fileinfo->f_st.st.st_mode & ~S_IFMT);
  sprintf(&hdr[108], "%06o \0", fileinfo->f_st.st.st_uid);
  sprintf(&hdr[116], "%06o \0", fileinfo->f_st.st.st_gid);
  sprintf(&hdr[124], "%011lo ", (long)fileinfo->f_st.st.st_size);
  sprintf(&hdr[136], "%011lo ", (long)fileinfo->f_st.st.st_mtime);
  strncpy(&hdr[148], "        ", 8);
  hdr[156] = tartype(fileinfo->f_st.st.st_mode);
  if (fileinfo->f_st.st.st_nlink > 1) {
    strcpy(&hdr[157], fileinfo->f_st.linkname);
    hdr[156] = LNKTYPE;
  }
  strcpy(&hdr[257], TMAGIC);
  strncpy(&hdr[263], TVERSION, 2);
  strcpy(&hdr[265], finduname((int)fileinfo->f_st.st.st_uid));
  strcpy(&hdr[297], findgname((int)fileinfo->f_st.st.st_gid));
  sprintf(&hdr[329], "%06o \0", major(fileinfo->f_st.st.st_rdev));
  sprintf(&hdr[337], "%06o \0", minor(fileinfo->f_st.st.st_rdev));
  if (prefix != (char *)NULL) {
    strncpy(&hdr[345], prefix, 155);
  }

/* Calculate the checksum */

  sum = 0;
  p = hdr;
  for (i = 0; i < 500; i++) {
    sum += 0xFF & *p++;
  }

/* Fill in the checksum field */

  sprintf(&hdr[148], "%06o \0", sum);

  outwrite(tapeinfo, hdr, BLOCKSIZE);

  if (fileinfo->file_fd > 0)
    outdata(tapeinfo, fileinfo);

}

char tartype(int mode)
{
  switch (mode & S_IFMT)
    {
    case S_IFDIR:
      return(DIRTYPE);
     
    case S_IFLNK:
      return(SYMTYPE);

    case S_IFIFO:
      return(FIFOTYPE);

    case S_IFCHR:
      return(CHRTYPE);

    case S_IFBLK:
      return(BLKTYPE);

    default:
      return(REGTYPE);
    }
}

static int  saveuid = -993;
static char saveuname[TUNMLEN];

static int  savegid = -993;
static char savegname[TGNMLEN];

char *finduname( int uuid )
{
  struct passwd *pw;

  if (uuid != saveuid) {
    saveuid = uuid;
    saveuname[0] = '\0';
    pw = getpwuid(uuid);
    if (pw) {
      strncpy(saveuname, pw->pw_name, TUNMLEN);
    }
  }
  return(saveuname);
}

char *findgname( int ggid )
{
  struct group *gr;

  if (ggid != savegid) {
    savegid = ggid;
    savegname[0] = '\0';
    gr = getgrgid(ggid);
    if (gr) {
      strncpy(savegname, gr->gr_name, TGNMLEN);
    }
  }
  return(savegname);
}
  
