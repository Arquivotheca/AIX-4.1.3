/* @(#)58	1.12  src/bos/usr/bin/ex/ex_tune.h, cmdedit, bos41J, 9515B_all 3/31/95 16:13:26 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_tune.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

/*
 * Definitions of editor parameters and limits
 */

/*
 * Pathnames.
 *
 * Only exstrings is looked at "+4", i.e. if you give
 * "/usr/lib/..." here, "/lib" will be tried only for strings.
 */

#ifndef EXRECOVER
#define EXRECOVER       "/usr/lbin/exrecover"
#endif /* EXRECOVER */

#ifndef EXPRESERVE
#define EXPRESERVE       "/usr/lbin/expreserve"
#endif /* EXPRESERVE */

#ifndef EXSTRINGS
#define EXSTRINGS       "/usr/lbin/exstrings"
#endif /* EXSTRINGS */


/*
 * If your system believes that tabs expand to a width other than
 * 8 then your makefile should cc with -DTABS=whatever, otherwise we use 8.
 */

#ifndef TABS
#define	TABS	8
#endif

/*
 * Caution: These constants are not definable but rather are defined
 * by other constants.  Basically these constants should not exist but
 * its easier to keep fix them than to remove them.
 *
 * Note: Because BUFFERSIZ is very large, BUFBYTES is only required to
 * LBSIZE * sizeof(wchar_t).
 * Most other definitions are quite generous.
 */

#define LBSIZE		2048   /* Line buffer size */	

#define ESIZE		522		/* Enough for 512 byte RE's */
/* FNSIZE is also defined in expreserve.c */
#define	FNSIZE		1024		/* Max file name size */
#define	RHSSIZE		256		/* Size of rhs of substitute */
#define	NBRA		128		/* Number of re \( \) pairs */
#define	TAGSIZE		32		/* Tag length */
#define	ONMSZ		128		/* Option name size */
#define	GBSIZE		256		/* Buffer size */
#define	UXBSIZE		128		/* Unix command buffer size */
#define	VBSIZE		128		/* Partial line max size in visual */
/* LBLKS is also defined in expreserve.c */
#define	LBLKS		1024
#define	HBLKS		2
#define	MAXDIRT		12		/* Max dirtcnt before sync tfile */
#define TCBUFSIZE	1024		/* Max entry size in termcap, see
					   also termlib and termcap */
#define MAXLINES	1048560		/* Line limit; shouldn't have to exist
					   but too hard to change memory
					   allocation algorithm. */
/*
 * Except on VMUNIX, these are a ridiculously small due to the
 * lousy arglist processing implementation which fixes core
 * proportional to them.  Argv (and hence NARGS) is really unnecessary,
 * and argument character space not needed except when
 * arguments exist.  Argument lists should be saved before the "zero"
 * of the incore line information and could then
 * be reasonably large.
 */
#define	NARGS	(NCARGS/6) /* NCARGS is a system defined value */

/*
 * Note: because the routine "alloca" is not portable, TUBESIZE
 * bytes are allocated on the stack each time you go into visual
 * and then never freed by the system.  Thus if you have no terminals
 * which are larger than 24 * 80 you may well want to make TUBESIZE
 * smaller.  TUBECOLS should stay at 160 since this defines the maximum
 * length of opening on hardcopies and allows two lines of open on
 * terminals like adm3's (glass tty's) where it switches to pseudo
 * hardcopy mode when a line gets longer than 80 characters.
 */
#define	TUBELINES	200
#define	TUBECOLS	400	/* opened up to allow for present day use */
#define	TUBESIZE	80000	/* 200 * 400 */

/*
 * Output column (and line) are set to this value on cursor addressible
 * terminals when we lose track of the cursor to force cursor
 * addressing to occur.
 */
#define	UKCOL		-20	/* Prototype unknown column */

/*
 * Attention is the interrupt character (normally 0177 -- delete).
 * Quit is the quit signal (normally fs -- control-\) and quits open/visual.
 */
#define	ATTN	(-2)
#define QUIT	('\\' & 037)	/* put back when old code page setup dropped */
