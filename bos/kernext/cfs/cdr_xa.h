/* @(#)94	1.1  src/bos/kernext/cfs/cdr_xa.h, sysxcfs, bos411, 9428A410j 7/26/93 12:49:22 */

#ifndef _H_CDRFS_XA
#define _H_CDRFS_XA

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
 *	cdr_xa.h: CD-ROM XA definitions
 *
 * reference: System Description CD-ROM XA,
 *            Philips/Sony, May 1991
 */


/*
 *	CD-ROM XA disc label
 *
 * CD-ROM XA disc label is located at offset 1024 of PVD
 * (in Application Use field).
 */
#define	CDRXA_LABEL_OFFSET	1024	/* CD-ROM XA disk label offset in PVD */
struct cdrxa_label {
	char	signature[8];	/* identifying signature: 'CD-XA001' */
	ushort	flags;		/* CD-ROM XA flags: reserved */
	char	startup[8];	/* startup directory name */
	char	reserved[18];
};

/* CD-ROM XA disc label signature */
#define CDRXA_SIGNATURE_LEN	8
#define CDRXA_SIGNATURE		"CD-XA001"


/*
 *	CD-ROM XA System Use extension
 *
 * All CD-ROM XA Mode 2 and CD-DA files must include 
 * System Use extension in their directory ecord.
 * note: gid, uid, and attributes fields are in big-endian format.
 */
struct cdrxa_sua {
	ushort	gid;		/* owner group id */
	ushort	uid;		/* owner user id */
	ushort	attributes;	/* attributes */
	ushort	signature;	/* signature: 'XA':0x5841 */
	uchar	file_number;	/* file number */
	char	reserved[5];
};

#define	CDRXA_SUA_SIGNATURE	0x5841	/* 'XA' */

/* cdrxa_sua attributes definitions */
#define	CDRXA_IRUSR	0x0001	/* read permission, owner */
#define	CDRXA_IXUSR	0x0004	/* execute permission, owner */
#define	CDRXA_IRGRP	0x0010	/* read permission, group */
#define	CDRXA_IXGRP	0x0040	/* execute permission, group */
#define	CDRXA_IROTH	0x0100	/* read permission, others */
#define	CDRXA_IXOTH	0x0400	/* execute permission, others */
#define	CDRXA_FORM1	0x0800	/* file contains Form 1 sectors */
#define	CDRXA_FORM2	0x1000	/* file contains Form 2 sectors */
#define	CDRXA_NTRLVD	0x2000	/* file contains interleaved sectors */
#define	CDRXA_DA	0x4000	/* CD-DA file */
#define	CDRXA_IFDIR	0x8000	/* directory file */

#define CDRXA_IFMT	0x7800	/* file format mask */
#define	CDRXA_IFMT_SHFT	11

#endif /* _H_CDRFS_XA */
