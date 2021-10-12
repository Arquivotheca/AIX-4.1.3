/* @(#)25	1.8  src/bos/usr/include/ar.h, cmdar, bos411, 9428A410j 6/16/90 00:07:48 */
/* ar.h	5.1 - 86/12/09 - 06:03:39 */
#ifndef _H_AR
#define _H_AR
/*
 * COMPONENT_NAME: CMDAR
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*		AIX INDEXED ARCHIVE FORMAT
*
*	ARCHIVE File Organization:
*	 _____________________________________________ 
*       |__________FIXED HEADER "fl_hdr"______________|
*  +--- |					      |
*  |    |__________ARCHIVE_FILE_MEMBER_1______________|
*  +--> |					      |
*       |	Archive Member Header "ar_hdr"        |
*  +--- |.............................................| <--+
*  |    |		Member Contents		      |    |
*  |    |_____________________________________________|    |
*  |    |________ARCHIVE_FILE_MEMBER_2________________|    |
*  +--> |					      | ---+
*       |	Archive Member Header "ar_hdr"        |
*  +--- |.............................................| <--+
*  |    |		Member Contents		      |    |
*  |    |_____________________________________________|    |
*  |    |	.		.		.     |    |
*  .    |	.		.		.     |    .
*  .    |	.		.		.     |    .
*  .    |_____________________________________________|    .
*  |    |________ARCHIVE_FILE_MEMBER_n-1______________|    |
*  +--> |					      | ---+
*       |	Archive Member Header "ar_hdr"        |
*  +--- |.............................................| <--+
*  |    |	Member Contents 		      |    |
*  |    |       (Member Table, always present)        |    |
*  |    |_____________________________________________|    |
*  |    |_____________________________________________|    |
*  |    |________ARCHIVE_FILE_MEMBER_n________________|    |
*  |    |					      |    |
*  +--> |	Archive Member Header "ar_hdr"        | ---+
*       |.............................................|
*       |	Member Contents 		      |
*       |       (Global Symbol Table if present)      |
*       |_____________________________________________|
*
*/

#define AIAMAG	"<aiaff>\n"
#define SAIAMAG	8
#define AIAFMAG	"`\n"

struct fl_hdr		/* archive fixed length header - printable ascii */
{
	char	fl_magic[SAIAMAG];	/* Archive file magic string */
	char	fl_memoff[12];		/* Offset to member table */
	char	fl_gstoff[12];		/* Offset to global symbol table */
	char	fl_fstmoff[12];		/* Offset to first archive member */
	char	fl_lstmoff[12];		/* Offset to last archive member */
	char	fl_freeoff[12];		/* Offset to first mem on free list */
};
#define FL_HDR struct fl_hdr
#define FL_HSZ sizeof(FL_HDR)


struct ar_hdr		/* archive file member header - printable ascii */
{
	char	ar_size[12];	/* file member size - decimal */
	char	ar_nxtmem[12];	/* pointer to next member -  decimal */
	char	ar_prvmem[12];	/* pointer to previous member -  decimal */
	char	ar_date[12];	/* file member date - decimal */
	char	ar_uid[12];	/* file member user id - decimal */
	char	ar_gid[12];	/* file member group id - decimal */
	char	ar_mode[12];	/* file member mode - octal */
	char	ar_namlen[4];	/* file member name length - decimal */
	union
	{
		char	ar_name[2];	/* variable length member name */
		char	ar_fmag[2];	/* AIAFMAG - string to end header */
	}	_ar_name;		/*      and variable length name */
};
/*
*	Note: 	'ar_namlen' contains the length of the member name which
*		may be up to 255 chars.  The character string containing
*		the name begins at '_ar_name.ar_name'.  The terminating
*		string AIAFMAG, is only cosmetic. File member contents begin
*		at the first even byte boundary past 'header position + 
*		sizeof(struct ar_hdr) + ar_namlen',  and continue for
*		'ar_size' bytes.
*/
#define AR_HDR struct ar_hdr
#define AR_HSZ sizeof(AR_HDR)

#endif /* _H_AR */
