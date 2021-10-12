static char sccsid[] = "@(#)90  1.2  src/bldenv/pkgtools/buffer.c, pkgtools, bos412, GOLDA411a 1/29/93 17:26:45";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: buf_allocate
 *		buf_out_avail
 *		buf_pad
 *		buf_use
 *		outdata
 *		outflush
 *		outwrite
 *		write_tar_eot
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
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
/* $Source: /u/mark/src/pax/RCS/buffer.c,v $
 *
 * $Revision: 1.2 $
 *
 * buffer.c - Buffer management functions
 *
 * DESCRIPTION
 *
 *	These functions handle buffer manipulations for the archiving
 *	formats.  Functions are provided to get memory for buffers, 
 *	flush buffers, read and write buffers and de-allocate buffers.  
 *	Several housekeeping functions are provided as well to get some 
 *	information about how full buffers are, etc.
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
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log:	buffer.c,v $
 * Revision 1.2  89/02/12  10:04:02  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:01  mark
 * Initial revision
 * 
 */

/* Headers */

#include "do_tar.h"
#include <errno.h>

/* Global data */

char *bufend;
char *bufstart;
char *bufidx;
long total;

/* Function Prototypes */


static void buf_pad(Tapeinfo *, long);
static void outflush(Tapeinfo *);
static void buf_use(uint);
static uint buf_out_avail(Tapeinfo *, char **);


/* outdata - write archive data
 *
 * DESCRIPTION
 *
 *	Outdata transfers data from the named file to the archive buffer.
 *	It knows about the file padding which is required by tar, but no
 *	by cpio.  Outdata continues after file read errors, padding with 
 *	null characters if neccessary.   Closes the input file descriptor 
 *	when finished.
 *
 * PARAMETERS
 *
 *	Tapeinfo *tapeinfo - Tape information
 *	Fileinfo *fileinfo - File name an real location
 *
 */


void outdata(Tapeinfo *tapeinfo, Fileinfo *fileinfo)
{
  uint            chunk;
  int             got;
  int             oops;
  uint            avail;
  int		    pad;
  char           *buf;
  long            size = fileinfo->f_st.st.st_size;

  oops = got = 0;
  if (pad = (size % BLOCKSIZE)) {
    pad = (BLOCKSIZE - pad);
  }
  while (size) {
    avail = buf_out_avail(tapeinfo, &buf);
    size -= (chunk = size < avail ? (uint) size : avail);
    if (oops == 0 && (got = read(fileinfo->file_fd, buf, (unsigned int) chunk)) < 0) {
      oops = -1;
      warn(fileinfo->ship_name, strerror(errno));
      got = 0;
    }
    if (got < chunk) {
      if (oops == 0) {
	oops = -1;
      }
      warn(fileinfo->ship_name, "Early EOF");
      while (got < chunk) {
	buf[got++] = '\0';
      }
    }
    buf_use(chunk);
  }
  close(fileinfo->file_fd);
  buf_pad(tapeinfo, pad);
}

/* write_tar_eot -  write the end of archive record(s)
 *
 * DESCRIPTION
 *
 *	Write out an End-Of-Tape record.  We actually zero at least one 
 *	record, through the end of the block.  Old tar writes garbage after 
 *	two zeroed records -- and PDtar used to.
 */


void write_tar_eot(Tapeinfo *tapeinfo)

{
    long            pad;
   
    pad = 2 * BLOCKSIZE;
    buf_pad(tapeinfo, pad);
    outflush(tapeinfo);
    close(tapeinfo->tape_fd);
}

 
/* outwrite -  write archive data
 *
 * DESCRIPTION
 *
 *	Writes out data in the archive buffer to the archive file.  The
 *	buffer index and the total byte count are incremented by the number
 *	of data bytes written.
 *
 * PARAMETERS
 *	
 *	char   *idx	- pointer to data to write
 *	uint	len	- length of the data to write
 */


void outwrite(Tapeinfo *tapeinfo, char *idx, uint len)

{
    uint            have;
    uint            want;
    char           *endx;

    endx = idx + len;
    while (want = endx - idx) {
	if (bufend - bufidx < 0) {
	    fatal("Buffer overflow in write");
	}
	if ((have = bufend - bufidx) == 0) {
	    outflush(tapeinfo);
	}
	if (have > want) {
	    have = want;
	}
	memcpy(bufidx, idx, (int) have);
	bufidx += have;
	idx += have;
	total += have;
    }
}


/* buf_allocate - get space for the I/O buffer 
 *
 * DESCRIPTION
 *
 *	buf_allocate allocates an I/O buffer using malloc.  The resulting
 *	buffer is used for all data buffering throughout the program.
 *	Buf_allocate must be called prior to any use of the buffer or any
 *	of the buffering calls.
 *
 * PARAMETERS
 *
 *	int	size	- size of the I/O buffer to request
 *
 * ERRORS
 *
 *	If an invalid size is given for a buffer or if a buffer of the 
 *	required size cannot be allocated, then the function prints out an 
 *	error message and returns a non-zero exit status to the calling 
 *	process, terminating the program.
 *
 */


void buf_allocate(long size)

{
    if (size <= 0) {
	fatal("Invalid value for blocksize");
    }
    if ((bufstart = malloc((unsigned) size)) == (char *)NULL) {
	fatal("Cannot allocate I/O buffer");
    }
    bufend = bufidx = bufstart;
    bufend += size;
}


/* outflush - flush the output buffer
 *
 * DESCRIPTION
 *
 *	The output buffer is written, if there is anything in it, to the
 *	archive file.
 */


static void outflush(Tapeinfo *tapeinfo)

{
  char           *buf;
  int             got;
  uint            len;
  
  /* if (bufidx - buf > 0) */
  for (buf = bufstart; len = bufidx - buf;) {
    if ((got = write(tapeinfo->tape_fd, buf,MIN(len, tapeinfo->blocksize)))> 0) {
      buf += got;
    } else if ((got < 0) && (errno == ENXIO || errno == ENOSPC)) {
      fatal("Out of space");
    } else {
      warn("write", strerror(errno));
      fatal("Tape write error");
    }
  }
  bufend = (bufidx = bufstart) + tapeinfo->blocksize;
}


/* buf_pad - pad the archive buffer
 *
 * DESCRIPTION
 *
 *	Buf_pad writes len zero bytes to the archive buffer in order to 
 *	pad it.
 *
 * PARAMETERS
 *
 *	OFFSET	pad	- number of zero bytes to pad
 *
 */


static void buf_pad(Tapeinfo *tapeinfo, long pad)

{
    int             idx;
    int             have;

    while (pad) {
	if ((have = bufend - bufidx) > pad) {
	    have = pad;
	}
	for (idx = 0; idx < have; ++idx) {
	    *bufidx++ = '\0';
	}
	total += have;
	pad -= have;
	if (bufend - bufidx == 0) {
	    outflush(tapeinfo);
	}
    }
}


/* buf_use - allocate buffer space
 *
 * DESCRIPTION
 *
 *	Buf_use marks space in the buffer as being used; advancing both the
 *	buffer index (bufidx) and the total byte count (total).
 *
 * PARAMETERS
 *
 *	uint	len	- Amount of space to allocate in the buffer
 */


static void buf_use(uint len)

{
    bufidx += len;
    total += len;
}


/* buf_out_avail  - index buffer space for archive output
 *
 * DESCRIPTION
 *
 * 	Stores a buffer pointer at a given location. Returns the number 
 *	of bytes available. 
 *
 * PARAMETERS
 *
 *	char	**bufp	- pointer to the buffer which is to be stored
 *
 * RETURNS
 *
 * 	The number of bytes which are available in the buffer.
 *
 */


static uint buf_out_avail(Tapeinfo *tapeinfo, char **bufp)

{
    int             have;

    if (bufend - bufidx < 0) {
	fatal("Buffer overflow in buf_out_avail");
    }
    if ((have = bufend - bufidx) == 0) {
	outflush(tapeinfo);
    }
    *bufp = bufidx;
    return (have);
}





