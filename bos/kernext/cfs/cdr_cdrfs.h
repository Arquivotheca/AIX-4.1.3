/* @(#)23	1.8  src/bos/kernext/cfs/cdr_cdrfs.h, sysxcfs, bos411, 9428A410j 8/27/93 16:46:39 */

#ifndef _H_CDRFS_CDRFS
#define _H_CDRFS_CDRFS

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

#include <sys/lockl.h>

/*
 *	cdr_cdrfs.h: CD-ROM file system global definitions 
 */

/* 
 *	CD-ROM file system format
 *
 * Volume and File Structure of CD-ROM for Information Interchange
 *
 * note: Rock Ridge Group and CD-ROM XA extensions are
 * based on the ISO 9660
 */
#define CDR_ISO9660	0x01	/* ISO 9660:1988 format		*/
#define CDR_HIGHSIERRA	0x02	/* High Sierra Group format	*/
#define CDR_ROCKRIDGE	0x05	/* Rock Ridge Group extension */
#define CDR_XA		0x09	/* CD-ROM XA extension */


/*
 *	CD-ROM file format
 * 
 * note: CDRFS supports access for non-interleaved CD-ROM Mode 1
 * and CD-ROM XA Mode 2 Form 1 files only.
 * note: the symbolic constants values corresponds to 
 * the CD-ROM XA file format attributes in cdr_xa.h
 */
#define	CD_ROM_M1	0x00	/* CD-ROM Mode 1 */
#define	CD_ROMXA_M2F1	0x01	/* CD-ROM XA Mode 2 Form 1 */
#define	CD_ROMXA_M2F2	0x02	/* CD-ROM XA Mode 2 Form 2 */
#define	CD_NTRLVD	0x04	/* interleaved sectors */
#define	CD_DA		0x08	/* CD-DA */


/* 
 *	CD-ROM mounted file system data staructure
 */
struct cdrfsmount {
	struct vfs	*fs_vfs;	/* mounted vfs */
	struct file 	*fs_fp;		/* file pointer to mounted device */
	uchar		fs_format;	/* format of media */
	uint		fs_lblksize;	/* logical block size in byte */
	uint		fs_lblkshift; 	/* log base 2 of lblk size */
	daddr_t		fs_rootdirent; 	/* root directory lblkn */
	struct cdrpvd 	*fs_pvd; 	/* primary volume descriptor */
	/* RRG specific area */
	int		fs_rrg_sufoffset;	/* RRG SUF offset in SUA */
	uint		fs_xcdr_cdmntsuppl;	/* XCDR cdmntsuppl options */
};
#define CDRVFSDATA(vfsp)		((struct cdrfsmount *)(vfsp)->vfs_data)


/* Date and Time format in Volume Descriptor and XAR.
 * note: timestamp is in UTC.
 */
struct cdrtime_long {
	char	ct_year[4];	/* years since 1900: digits (1 to 9999)	*/
	char	ct_month[2];	/* month of the year: digits (1 to 12)	*/
	char	ct_day[2];	/* day of the month: digits (1 to 31)	*/
	char	ct_hour[2];	/* hour of the day: digits (0 to 23)	*/
	char	ct_minute[2];	/* minute of the hour: digits (0 to 53)	*/
	char	ct_second[2];	/* second of the minute: digits (0 to 53) */
	char	ct_csecond[2];	/* 1/100 second: digits (0 to 99)	*/
	signed char ct_tz;	/* time zone: offset from GMT in 	*/
				/* 15 minute intervals			*/
				/* (from -48 [west] to +52 [east])	*/
};

/* 
 * Define the locking MACROS for the cdrfs and cdrpager locks
 */

extern 	Simple_lock	cdrfs_lock;

/* 
 * macros for the cdrfs simple lock 
 */
#define CDRFS_LOCK()		simple_lock(&cdrfs_lock)
#define CDRFS_UNLOCK()		simple_unlock(&cdrfs_lock)

/*
 * Define the cdr_state flag. This flag provides the state of
 * the cdrfs. It serializes deletes with mounts of the cdrfs.
 * cdr_mount will check this global flag first before trying to
 * mount the file system. This flag is initialized at the end
 * of cdr_config(CFG_INIT), and set to terminated in cdr_config(CFG_TERM) 
 * when we are unloading the kernel extension.
 */

extern int cdr_state;

#define CDRFS_UNCONFIGURED	0
#define CDRFS_INITIALIZING	1
#define CDRFS_TERMINATING	2
#define CDRFS_CONFIGURED	3


/*
 *	Directory Record
 *	----------------
 *
 * note: cdr_dr.h is included to generate customized definition of
 * Directory Record for ISO 9660:1988 and HSG format
 */

/* 
 * Directory Record for High Sierra format 
 */
#define HIGHSIERRA	1
#include "cdr_dr.h"

/* macro for accessing high sierra direntory entries */
#define HSDE(de)	((struct hscdrdirent *)(de))

/* 
 * Directory Record for ISO 9660:1988 format 
 */
#undef HIGHSIERRA
#include "cdr_dr.h"

/* Directory Record File/Directory Identifier limits */
#define	CDR_DIRNAME_MAX		31 /* maximum length of directory name */
#define CDR_FILENAME_MAX	37 /* maximum length of file name	*/
#define CDR_ISO_NAME_MAX	CDR_FILENAME_MAX
#define CDR_RRG_NAME_MAX	255

/* directory entry size */
#define CDR_DIRENT_MIN		(sizeof(struct cdrdirent))
#define CDR_DIRENT_MAX		255
#define CDR_DIRENT_FIXED	33

/* default access permissions */
#define CDR_PERMISSION	S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH
#define USRMODESHIFT	6	/* from OTH to USR */
#define GRPMODESHIFT	3	/* from USR/OTH to GRP and vice versa */
#define OTHMODESHIFT	6	/* from USR to OTH */

/*
 *	Extended Attribute Record (XAR)
 *
 * note: only final XAR is processed (i.e., XAR associated with
 * the last/only File Section of the file).
 *
 * note: when XAR is restricted XAR (i.e., the Protection bit of
 * the Directory Record File Flags is set to 0), the Owner/Group 
 * Identifications and Permissions of the XAR are ignored
 */
struct cdrxar {
	ushort	xar_own_id_le;
	ushort	xar_own_id;	/* Owner Identification */
	ushort	xar_grp_id_le;
	ushort	xar_grp_id;	/* Group Identification */
	ushort	xar_permissions;/* Permissions */
	struct cdrtime_long	xar_cre_time; /* File Creation Date and Time */
	struct cdrtime_long	xar_mod_time; /* File Modification Date and Time */
	struct cdrtime_long	xar_exp_time; /* File Expiration Date and Time */
	struct cdrtime_long	xar_eff_time; /* File Effective Date and Time */
	uchar	xar_rec_form;	/* Record Format */
	uchar	xar_rec_attr;	/* Record Attributes */
	ushort	xar_rec_len_le;
	ushort	xar_rec_len;	/* Record Length */
	uchar	xar_sys_id[32];	/* System Identifier */
	uchar	xar_sys_use[64];	/* System Use */
	uchar	xar_xar_vers;	/* XAR Version */
	uchar	xar_esc_len;	/* Length of Escape Sequences */
	uchar	xar_resv[64];	/* Reserved for future standardisation */
	ushort	xar_appuse_len_le;
	ushort	xar_appuse_len;	/* Length of Application Use */
	uchar	xar_app_use[1];	/* Application Use field data */
	/* if the xar_esc_len field is non-zero, the application use area */
	/* is followed by escape sequences that designate the code sets  */
	/* used to interpret the contents of the file.		*/
};


/*
 *	Volume Descriptor
 *	-----------------
 *
 * note: cdr_vd.h is included to generate customized definition of
 * Volume Descriptor for ISO 9660:1988 and HSG format
 *
 * note: volume descriptor set size is set to the maximum number of
 * volume descriptors to read to locate the Primary Volume Descriptor.
 */
#define CDR_DATAAREA	16	/* Data Area start Logical Sector number */
#define CDR_VDSAREA	16	/* Volume Descriptor Set area */
#define CDR_VDSIZE	2048	/* volume descriptor size */

#define	CDR_STDID_ISO9660	"CD001"	/* ISO 9660:1988 standard identifier */
#define CDR_STDID_HIGHSIERRA "CDROM" /* High Sierra Group standard identifier */
#define CDR_STDIDLEN	5	/* length of standard identifer string	*/
#define CDR_VOLIDLEN	32	/* length of volume identifier */

/*
 * Volume Descriptor type 
 */
#define	CDR_BR		0x00	/* Boot Record */
#define	CDR_PVD		0x01	/* Primary Volume Descriptor */
#define CDR_SVD		0x02	/* Supplementatry Volume Descriptor */
#define	CDR_VPD		0x03	/* Volume Partition Descriptor */
#define	CDR_VDST	0xff	/* Volume Descriptor Set Terminator */

/*
 *  Volume Descriptor format for High Sierra Group (HSG)
 */
#define HIGHSIERRA	1
#include "cdr_vd.h"

/*
 *  Volume Descriptor format for ISO 9660:1988
 */
#undef HIGHSIERRA
#include "cdr_vd.h"

/* predicate for determining the format of the volume associated with	*/
/* a virtual file system						*/
#define IS_HIGHSIERRA(vfsp)	(CDRVFSDATA(vfsp)->format == CDFS_HIGHSIERRA)

/* translate ISO 9660 Volume Descriptor pointer to
 * High Sierra Volume Descriptor pointer		
 */
#define HSVD(vd)	((struct hscdrvd *)vd)
#define HSPVD(pvd)	((struct hscdrpvd *)pvd)


/* 
 *	Path Table Record
 * note: currently not supported.
 */
struct cdrptr {
	uchar	ptr_dirid_len;	/* Length of Directory Identifier in bytes */
	uchar	ptr_xar_len;	/* XAR Length in Logical Blocks */
	/* ISO 9660 do not specify byte-ordering of the following two fields */
	uchar	ptr_loc_ext[4];	/* Location of Extent */
	ushort	ptr_pdirno;	/* Parent Directory Number */
	uchar	ptr_dir_id[1];	/* Directory Identifier */
				/* if diridlen is an odd number, it is  */
				/* padded with a null byte		*/
};


/* 
 *	conversion macros
 *	-----------------
 *
 * macros for conversion from logical structure to physical structure
 *
 * Volume Structure:
 * The Volume Space is organized in Logical Sectors and Logical Blocks:
 * . A Logical Sector size is 2048 (Logical Sector size = MAX(2048, 2**n) 
 *   where (2**n <= sector size) and (physical) sector size <= 2048);
 *   Logical Sector Number starts with 0. 
 *
 * . A Logical Block size is 2**(n+9) where n >= 0 and 
 *   Logical Block size <= Logical Sector size 
 *   (i.e., a Logical Sector is a multiple of power of 2 of a Logical Block);
 *   Logical Block Number starts with 0. 
 *
 * The Volume Space is partitioned into
 * . System Area consists of the first 16 Lofical Sectors
 * . Data Area comprises the remaining Logical Sectors.
 *
 * File Structure:
 * A File Section is the part of a file that is recorded
 * in any one extent where
 * an Extent is a set of contiguous Logical Blocks.
 *
 * Each file consists of one or more File Sections where
 * each File Section is identified by a Directory Record
 * in the same directory where
 * the sequence of the File Sections of a file is identified by
 * the order of the corresponding Directory Records 
 * in the directory.
 *
 * Directory Structure:
 * A directory consists of a single File Section.
 * A directory Extent is aligned on Logical Sector boundary and
 * consists of contiguous Logical Sectors.
 */

#define	CDR_PAGESIZE	4096

/* physical sector (aka physical block of 512 bytes) */
#define CDR_PBLKSIZE	512	/* physical block size in bytes		*/
#define	CDR_PBLKSHIFT	9	/* log base 2 of CDR_PBLKSIZE		*/
#define PBLKOFFSET(blk, off)	/* offset in object with pblk */ \
	(((daddr_t)(blk) << CDR_PBLKSHIFT) | (uint)(off))
#define	PBLKNO(daddr)		/* pblock number */ \
	((daddr_t)(daddr) >> CDR_PBLKSHIFT)
#define	PBLKOFF(daddr)		/* offset in pblock */ \
	((daddr_t)(daddr) & (CDR_PBLKSIZE - 1))

/* logical blocks */
#define LBLKSIZE(fs)		(((struct cdrfsmount *)(fs))->fs_lblksize)
#define LBLKSHIFT(fs)		(((struct cdrfsmount *)(fs))->fs_lblkshift)
#define LOFFSET(fs, blk, off)	/* offset in object with lblk */ \
	(((daddr_t)(blk) << LBLKSHIFT(fs)) | (uint)(off))
#define	LBLKNO(fs, daddr)	/* lblk number */ \
	((daddr_t)(daddr) >> LBLKSHIFT(fs))
#define	LBLKOFF(fs, daddr)	/* offset in lblk */ \
	((daddr_t)(daddr) & (LBLKSIZE(fs) - 1))
#define LTOPBLK(fs, daddr)	/* lblk to pblk */ \
	((daddr_t)(daddr) << (LBLKSHIFT(fs) - CDR_PBLKSHIFT))

/* logical sectors */
#define CDR_LSCTRSIZE	2048	/* logical sector size in bytes		*/
#define	CDR_LSCTRSHIFT	11	/* log base 2 of CDR_LSCTRSIZE		*/
#define	LASTLSCTR(offset)	/* rounddown to start of current sctr */ \
	((offset) & ~(CDR_LSCTRSIZE - 1))
#define NEXTLSCTR(offset)	/* roundup to start of next lsctr */ \
	(((daddr_t)(offset) + CDR_LSCTRSIZE) & ~(CDR_LSCTRSIZE - 1))
#define LSCTR2LBLK(fs, daddr)	/* lsctr to lblk */ \
	((daddr_t)(daddr) << (CDR_LSCTRSHIFT - LBLKSHIFT(fs)))
#define LSCTR2PBLK(daddr)	/* lsctr to pblk */ \
	((daddr_t)(daddr) << (CDR_LSCTRSHIFT - CDR_PBLKSHIFT))


/* 
 * macros for conversion of numeric fields 
 * from byte/short aligned integer (byte array at address x) to integer 
 */
#define	BYTE2SHORT(x)	(((ushort)((uchar *)x)[0] << 8) | \
			 ((ushort)((uchar *)x)[1]))
#define	BYTE2INT(x)	(((uint)((uchar *)x)[0] << 24) | \
			 ((uint)((uchar *)x)[1] << 16) | \
			 ((uint)((uchar *)x)[2] << 8)  | \
			 ((uint)((uchar *)x)[3]))
#define	SHORT2INT(x)	(((uint)((ushort *)x)[0] << 16) | \
			 ((uint)((ushort *)x)[1]))

/*
 * CDRFS recognized exception return from mapped file access
 */
extern	int cdr_elist[];
#define	CDRFS_EXCEPTION	0x80000000
#define	RETURNX(rc, el)	\
	return((rc & CDRFS_EXCEPTION) ? cdrfs_exception(rc, el) : rc)

#endif /* _H_CDRFS_CDRFS */
