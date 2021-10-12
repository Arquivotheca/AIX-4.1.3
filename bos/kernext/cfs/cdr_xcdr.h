/* @(#)26	1.3  src/bos/kernext/cfs/cdr_xcdr.h, sysxcfs, bos411, 9428A410j 2/19/93 15:23:25 */

#ifndef _H_CDRFS_XCDR
#define _H_CDRFS_XCDR

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
 * 	cdr_xcdr.h: XCDR definitions and declarations 
 *
 * reference:
 *	cdrom.h - XCDR definitions and declarations,
 *	X/Open Preliminary Specification CD-ROM Support Component (XCDR),
 *	X/Open Document Number: XO/PRELIM/91/020
 *
 * note on byte ordering of numeric fields:
 * only big-endian (the most significant byte is stored at the lowest 
 * (first) address of the field) format is supported 
 * (e.g., IBM RISC System/6000, Motorola 680x0 family).
 */


/*
 * 	iso9660_pvd: Primary Volume Descriptor
 *
 * usage: cd_pvd()
 */
struct iso9660_pvd {
	uchar	voldestype;	/* Volume Descriptor Type */
	uchar	std_id[5];	/* Standard Identifier */
	uchar	voldesvers;	/* Volume Descriptor Version */
	uchar	sys_id[32];	/* System Identifier */
	uchar	vol_id[32];	/* Volume Identifier */
	ulong	volspcsize;	/* Volume Space Size in Logical Blocks */
	ushort	volsetsize;	/* Volume Set Size */
	ushort	volseqno;	/* Volume Sequence Number */
	ushort	lblksize;	/* Logical Block Size in bytes */
	ulong	ptsize;		/* Path Table Size in bytes */
	ulong	locpt_l; 	/* Location of Occurrence of Type L Path Table */
	ulong	locptopt_l; 	/* Location of Optional Occurrence of Type L Path Table */
	ulong	locpt_m; 	/* Location of Occurrence of Type M Path Table */
	ulong	locptopt_m; 	/* Location of Optional Occurrence of Type M Path Table */
	uchar  	rootdir[34];	/* Directory Record for Root Directory */
	uchar	volset_id[128];	/* Volume Set Identifier */
	uchar	pub_id[128];	/* Publisher Identifier */
	uchar	dtpre_id[128];	/* Data Preparer Identifier */
	uchar	app_id[128];	/* Application Identifier */
	uchar	cpfile_id[37];	/* Copyright File Identifier */
	uchar	abfile_id[37];	/* Abstract File Identifier */
	uchar	bgfile_id[37];	/* Bibliographic File Identifier */
	time_t	cre_time;	/* Volume Creation Date and Time */
	time_t	mod_time;	/* Volume Modification Date and Time */
	time_t	exp_time;	/* Volume Expiration Date and Time */
	time_t	eff_time;	/* Volume Effective Date and Time */
	uchar	filestrver;	/* File Structure Version */
	uchar	res1;		/* Reserved for future standardization:
				 * byte position 838 */
	uchar	appuse[512];	/* Application Use */
	uchar	res2[653];	/* Reserved for future standardization:
				 * byte position 1396 - 2048 */
};

/* the length of the Primary Volume Descriptor used by cd_cpvd() */
#define	CD_PVDLEN	2048


/*
 *	iso9660_xar: Extended Attribute Record 
 *
 * usage: cd_xar()
 */
struct iso9660_xar {
	ushort	own_id;		/* Owner Identification */
	ushort	grp_id;		/* Group Identification */
	ushort	permissions;	/* Permissions */
	time_t	cre_time;	/* File Creation Date and Time */
	time_t	mod_time;	/* File Modification Date and Time */
	time_t	exp_time;	/* File Expiration Date and Time */
	time_t	eff_time;	/* File Effective Date and Time */
	uchar	rec_form;	/* Record Format */
	uchar	rec_attr;	/* Record Attributes */
	ushort	rec_len;	/* Record Length */
	uchar	sys_id[32];	/* System Identifier */
	uchar	sys_use[64];	/* System Use */
	uchar	xar_vers;	/* XAR Version */
	uchar	esc_len;	/* Length of Escape Sequences */
	uchar	resv[64];	/* Reserved for future standardisation */
	ushort	appuse_len;	/* Length of Application Use */
	uchar	*app_use;	/* Pointer to Application Use field data */
	uchar	*esc_seq;	/* Pointer to Escape Sequence field data */
};

/* symbolic constants for iso9660_xar permissions
 * note: permission is granted when the bit is set to 0 */
#define	CD_RSYS		0x0001	/* read permission, system class */	
#define	CD_XSYS		0x0004	/* execute permission, system class */
#define	CD_RUSR		0x0010	/* read permission, owner */
#define	CD_XUSR		0x0040	/* execute permission, owner */
#define	CD_RGRP		0x0100	/* read permission, group */
#define	CD_XGRP		0x0400	/* execute permission, group */
#define	CD_ROTH		0x1000	/* read permission, others */
#define	CD_XOTH		0x4000	/* execute permission, others */

/* the length of fixed part of XAR used by cd_cxar() */
#define	CD_XARFIXL	250


/*
 * 	iso9660_drec: Directory Record 
 *
 * usage: cd_drec()
 */
struct iso9660_drec {
	uchar	drec_len;	/* Length of Directory Record in bytes */
	uchar	xar_len;	/* XAR Length in Logical Blocks */
	ulong	locext;		/* Location of Extent in LBLK number */
	ulong	data_len;	/* Data Length in bytes */
	time_t	rec_time;	/* Recording Date and Time */
	uchar	file_flags;	/* File Flags */
	uchar	file_usize;	/* File Unit Size */
	uchar	ileav_gsize;	/* Interleave Gap Size */
	ushort	volseqno;	/* Volume Sequence Number whence Extent */
	uchar	fileid_len;	/* Length of File Identifier */
	uchar	file_id[37];	/* File Identifier: note1 */
	uchar	sysuse_len;	/* Length of System Use: note2 */
	uchar	sys_use[218];	/* System Use */
};

/* symbolic constants for iso9660_drec file_flags */
#define	CD_EXIST	0x01	/* file can be hidden from user */
#define	CD_DIR		0x02	/* file is a directory */
#define	CD_ASSOFILE	0x04	/* file is an Associated File */
#define	CD_RECORD	0x08	/* file has a record format */
#define	CD_PROTEC	0x10	/* file has Owner/Group protection in XAR */
#define	CD_MULTIEXT	0x80	/* not final Directory Record for
				 * the multi-extent file */

/* the maximum length of a Directory Record used by cd_cdrec() */
#define	CD_MAXDRECL	255


/*
 * 	iso9660_ptrec: Path Table Record 
 *
 * usage: cd_ptrec()
 */
struct iso9660_ptrec {
	uchar	dirid_len;	/* Length of Directory Identifier in bytes */
	uchar	xar_len;	/* XAR Length in Logical Blocks */
	ulong	loc_ext;	/* Location of Extent */
	ushort	pdirno;		/* Parent Directory Number */
	uchar	dir_id[31];	/* Directory Identifier */
};

/* the maximum length of a Path Table Record used by cd_cptrec() */
#define	CD_MAXPTRECL	40


/* ISO 9660 compliant format used by cd_type() */
#define	CD_ISO9660	0x01	/* ISO 9660 format */	


/* symbolic constants used by cd_defs() for argument cmd */
#define	CD_SETDEFS	0x01	/* set default values */
#define	CD_GETDEFS	0x02	/* get default values */

/* struct cd_defs used by cd_defs() */
struct cd_defs {
	uid_t	def_uid;	/* Default User ID */
	gid_t	def_gid;	/* Default Group ID */
	mode_t	def_fperm;	/* Default File Permissions */
	mode_t	def_dperm;	/* Default Directory Permissions */
	int	dirsperm;	/* Directory search permission */
};

/* symbolic constants for cd_defs dirsperm */
#define	CD_DIRXAR	0x01	/* execute permission for directories are */
				/* set as provided by XAR permissions of  */
				/* the directory */
#define	CD_DIRRX	0x02	/* execute permission for directories are */
				/* set if read or execute bits are set in */
				/* the XAR of the directory */


/* symbolic constants used by cd_idmap() for argument cmd */
#define CD_SETUMAP	0x01	/* set User ID mapping */
#define	CD_SETGMAP	0x02	/* set Group ID mapping */
#define	CD_GETUMAP	0x03	/* get User ID mapping */
#define	CD_GETGMAP	0x04	/* get Group ID mapping */

/* struct cd_idmap used by cd_idmap() */
struct cd_idmap {
	ushort	from_id;	/* Owner Identification */
	uid_t	to_uid;		/* Owner ID in XSI file hierarchy */
	gid_t	to_gid;		/* Group ID in XSI file hierarchy */
};

/* limit on the number of User/Group ID mappings */
#define	CD_MAXUMAP	50	/* maximum User ID mapping */
#define	CD_MAXGMAP	50	/* maximum Group ID mapping */


/* symbolic constants used by cd_nmconv() for argument cmd */
#define	CD_SETNMCONV	0x01	/* set file name conversion */
#define	CD_GETNMCONV	0x02	/* get file name conversion */

/* symbolic constants used by cd_nmconv() for argument flag */
#define	CD_NOCONV	0x01	/* cdmntsuppl -c: no conversion */
#define	CD_LOWER	0x02	/* cdmntsuppl -l: 
				 * lower case;
				 * no '.' for null filename extension
				 */
#define	CD_NOVERSION	0x04	/* cdmntsuppl -m: no ';' and version number */

#endif /* _H_CDRFS_XCDR */
