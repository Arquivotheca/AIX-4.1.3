/* @(#)25	1.3  src/bos/kernext/cfs/cdr_dr.h, sysxcfs, bos411, 9428A410j 2/19/93 15:22:58 */

/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	cdr_dr.h: Directory Record
 *
 * note: cfs_dr.h is included in cfs_cdrfs.h to generate customized
 * definitions of Directory Record for ISO 9660:1988 and HSG format.
 */

/*
 *	Directory
 *
 * A directory consists of a single File Section.
 * A directory Extent is aligned on Logical Sector boundary and
 * consist of contiguous Logical Sectors.
 * A directory consists of a set of Directory Records each of
 * which identifies a File Section or another/child directory.
 * Each Directory Record is contained within a single Logical Sector;
 * unused bytes after the last Directory Record in a Logical Sector
 * shall be padded with 0x00.
 * (note: the first Directory Record may on Logical Block bounary
 * but not be on Logical Sector boundary if the directory is associated 
 * with an Extended Attribute Record.)
 *
 * The Directory Records are ordered by the following criteria
 * in descending order of significance:
 * 1. in lexicographic ascending order of File Name 
 *    (padded right with 0x20 (blank))
 * 2. in lexicographic ascending order of File Name Extension
 *    (padded right with 0x20 (blank))
 * 3. in lexicographic descending order of File Version Number
 *    (padded left with 0x30 (0))
 * 4. in descending order of the value of Associated File flag bit
 * 5. the order of the File Sections of the file.
 */

/*
 *	Directory Record
 *
 * Directory Record consists of 
 *  . ISO 9660 specified area (upto including File Identifier)
 *  . optional System Use Area (SUA) not specified by ISO 9660.
 *
 * The Directory Record and its System Use Area is aligned on
 * even number of bytes and consists of even number of bytes:
 * padding field of a single byte of 0x00 shall be present 
 * 1. after fileid if fileid_len is an even number so that ISO 9660 
 * specified fields comprises an even number of bytes, and
 * 2. after sysuse, if present, so that the Directory Record comprises
 * an even number of bytes where
 * sysuse_len = drec_len - fileid_len - offset_to_fileid (- 1 if fileid_len is even)
 */

/* Date and Time format in Directory Record
 * note: the timestamp is in UTC.
 */
#ifdef HIGHSIERRA
struct hscdrtime	/* HSG Date and Time format */
#else
struct cdrtime		/* ISO 9660:1988 Date and Time format */
#endif
{
	uchar	d_year;		/* number of years since 1900 */
	uchar	d_month;	/* month of the year: 1 - 12 */
	uchar	d_day;		/* day of the month: 1 - 31 */
	uchar	d_hour;		/* hour of the day: 0 - 23 */
	uchar	d_minute;	/* minute of the hour: 0 - 59 */
	uchar	d_second;	/* second of the minute: 0 - 59 */
#ifndef HIGHSIERRA
	signed char	d_tz;		/* offset from UTC in number of */ 
				/* 15 minute intervals */
#endif
};

#ifdef HIGHSIERRA
struct hscdrdirent	/* HSG Directory Record format */
#else
struct cdrdirent	/* ISO 9660:1988 Directory Record format */
#endif
{
	uchar	d_drec_len;	/* length of Directory Record in bytes */
	uchar	d_xar_len;	/* XAR length in lblk */
	uchar	d_locext_le[4];
	uchar	d_locext[4];	/* location of Extent in lblkn */
	uchar	d_data_len_le[4];
	uchar	d_data_len[4];	/* data length of File Section in bytes */
#ifdef HIGHSIERRA
	struct hscdrtime  d_rec_time; /* Recording Date and Time */
#else
	struct cdrtime	d_rec_time; /* Recording Date and Time */
#endif
	/* following fields are at different offset for both ISO 9660 and HSG */
	uchar	d_file_flags;	/* File Flags */
	uchar	d_file_usize;	/* File Unit size */
	uchar	d_ileav_gsize;	/* Interleave Gap size */
#ifdef HIGHSIERRA
	uchar	d_pad;		/* padding */
#endif
	/* following fields are at the same offset for both ISO 9660 and HSG */
	ushort	d_volseqno_le;
	ushort	d_volseqno;	/* Volume Sequence Number whence Extent */
	uchar	d_fileid_len;	/* length of File Identifier */
	uchar	d_file_id[1];	/* File Identifier of fileid_len */
};
