/* @(#)27	1.3  src/bos/kernext/cfs/cdr_rrg.h, sysxcfs, bos411, 9428A410j 2/19/93 15:23:56 */

#ifndef _H_CDRFS_RRG
#define _H_CDRFS_RRG

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
 *	cdr_rrg.h: CD-ROM Rock Ridge Group (RRG) protocols
 *
 * reference: 
 * . System Use Sharing Protocol, 
 *	A Mechanism for Extensible Sharing of ISO 9660:1988 System
 *	Use Areas, 
 *	Version 1, Revision 1.09, 1992 
 * . Rock Ridge Interchange Protocol, 
 *	An ISO 9660:1988 Compliant Approach To Providing Adequate CD-ROM
 *	Support for POSIX File System Semantics, 
 *	Version 1, Revision 1.09, 1992
 */

/*
 * note: System Use Area (SUA) length and offset in Directory Record:
 *  sua_len = drec_len - fileid_len - offset_to_fileid
 *              (- 1 if fileid_len is even)
 *  sua_offset = drec + drec_len - sua_len
 */

/* generic RRG SUF template */
struct rrg_suf {
	uchar	signature[2];
	uchar	len;
	uchar	ver;
	uchar	data[1];
};


/*
 * 	System Use Sharing Protocol (SUSP)
 *	----------------------------------
 *
 * note: System Use Field (SUF) in root Directory Record:
 * the instance of the root directory record in 
 * the primary volume descriptor cannot contain a system use area,
 * thus the root directory record as recorded in the "." entry of
 * the root directory must be used to record and retrieve SUFs 
 * required in the directory record of the root directory. 
 */
#define	CDR_SUSP_CHECKBYTES	0xbeef

#define SUSP_CE	0x4345	/* "CE": continuation area */
#define	SUSP_PD	0x5044	/* "PD": padding field */
#define SUSP_SP 0x5350	/* "SP": SUSP indicator */
#define SUSP_ST	0x5354	/* "ST": SUSP terminator */
#define	SUSP_ER	0x4552	/* "ER": extension reference */

/*
 * 	SUSP_SP: SUSP indicator
 *
 * SP must start at bp1 in SUA of the first directory record ("." entry)
 * of the root directory of each directory structure with SUSP
 */
struct susp_sp_suf {
	char	signature[2];	/* signature word: SP */
	uchar	len;		/* SP length : 7 */
	uchar	ver;		/* SP version: 1 */
	char	check_bytes[2];	/* check bytes: 0xbeef */
	uchar	len_skp;	/* number of bytes skipped within SUA before recording SUFs */
};

/*
 * 	SUSP_ER: extension reference
 *
 * RRIP ER must appear in SUA of the first directory record ("." entry)
 * of the root directory of each directory structure with RRIP ER
 */
struct susp_er_suf {
	uchar	signature[2];	/* signature word: ER */
	uchar	len;	 	/* ER length: 8 + 10 + 84 + 135 = 237 */
	uchar	ver; 		/* ER version: 1 */
	uchar	len_id;		/* identifier length: 10 */
	uchar	len_des;	/* descriptor length: 84 */
	uchar	len_src;	/* source length: 135 */
	uchar	ext_ver;	/* extension version: 1 */
	char	ext_id[10];	/* extension identifier */
	char	ext_des[84];	/* extension descriptor */
	char	ext_src[135];	/* extension source */
};


/* 
 * 	SUSP_CE: continuation area 
 *
 * Each CA consist of a single Logical Sector.
 * The CA specified by the current CE SUF should be processed 
 * after the current SUA or CA is processed.
 */
struct	susp_ce_suf {
	uchar	signature[2];	/* signature word: CE */
	uchar	len;		/* CE length: 28 */
	uchar	ver;		/* CE version: 1 */
	uchar	location_le[4];	/* location of continuation of system use area */
	uchar	location[4];	/* location of continuation of system use area */
	uchar	offset_le[4];	/* offset to start of continuation */
	uchar	offset[4];	/* offset to start of continuation */
	uchar	len_cont_le[4];	/* length of continuation */
	uchar	len_cont[4];	/* length of continuation */
};


/*
 * 	SUSP_PD: padding field 
 */
struct susp_pd_suf {
	char	signature[2];	/* signature word: PD */
	uchar	len;		/* PD length */
	uchar	ver;		/* PD version: 1 */
	uchar	padding_area[1];/* padding area: SUF length - 4 */
};


/*
 * 	SUSP_ST: SUSP terminator
 */
struct susp_st_suf {
	char	signature[2];	/* signature word: ST */
	uchar	len; 		/* ST length: 4 */
	uchar	ver;		/* ST version: 1 */
};


/*
 * 	Rock Ridge Interchange Protocol (RRIP)
 *	--------------------------------------
 */
#define CDR_RRIP_EXTID	"RRIP_1991A"

#define RRIP_PX	0x5058	/* "PX": POSIX file attributes */
#define	RRIP_PN	0x504e	/* "PN": POSIX device modes */
#define	RRIP_SL	0x534c	/* "SL": symbolic link */
#define	RRIP_NM	0x4e4d	/* "NM": alternate name */
#define	RRIP_CL	0x434c	/* "CL": child link */
#define	RRIP_PL	0x504c	/* "PL": parent link */
#define	RRIP_RE	0x5245	/* "RE": relocated directory */
#define	RRIP_TF	0x5446	/* "TF": time stamp(s) for a file */
#define	RRIP_RR	0x5252	/* "RR": RRIP fields recorded */

#define RRIP_SUF_SLVF	5	/* size of (signature, len, ver, and flag) */

/*
 * 	RRIP_PX: POSIX file attributes
 *
 * PX is mandatory in SUA of each ISO 9660 Directory Record
 */
struct rrip_px_suf {
	char	signature[2];	/* signature word: PX */
	uchar	len;		/* PX length: 36 */
	uchar	ver;		/* PX version: 1 */
	uchar	file_mode_le[4];	/* POSIX file mode: sys/stat.h st_mode */
	uchar	file_mode[4];	/* POSIX file mode: sys/stat.h st_mode */
	uchar	links_le[4];	/* POSIX file links: sys/stat.h st_nlink */
	uchar	links[4];	/* POSIX file links: sys/stat.h st_nlink */
	uchar	user_id_le[4];	/* POSIX file user id: sys/stat.h st_uid */
	uchar	user_id[4];	/* POSIX file user id: sys/stat.h st_uid */
	uchar	group_id_le[4];	/* POSIX file group id: sys/stat.h st_gid */
	uchar	group_id[4];	/* POSIX file group id: sys/stat.h st_gid */
};


/* 
 * 	RRIP_PN: POSIX device modes
 */
struct rrip_pn_suf {
	char	signature[2];	/* signature word: PN */
	uchar	len;		/* PN length: 20 */
	uchar	ver;		/* PN version: 1 */
	uchar	dev_t_high_le[4];	/* dev_t high */
	uchar	dev_t_high[4];	/* dev_t high */
	uchar	dev_t_low_le[4];	/* dev_t low */
	uchar	dev_t_low[4];	/* dev_t low */
};


/*
 * 	RRIP_SL: symbolic link
 *
 * SL SUF component area consists of a set of contiguous
 * component records, where 
 * each pathname component is recorded as one or more
 * component records.
 */
/* symbolic link component record */
struct rrip_sl_cr_suf { 
	uchar	flags;		/* component flags */
	uchar	len;		/* component length: */
	char	component[1];	/* component */
};

/* rrip_sl_cr_suf flags */
#define	RRIP_SL_CR_CONTINUE	0x01	/* continue into next SL */
#define	RRIP_SL_CR_CURRENT	0x02	/* componet refers to (.) */
#define RRIP_SL_CR_PARENT	0x04	/* component refers to (..) */
#define RRIP_SL_CR_ROOT		0x08	/* component refers to root */
#define RRIP_SL_CR_VOLROOT	0x10	/* component refers to covered directory */
#define	RRIP_SL_CR_HOST		0x20	/* component refers to local host name */


struct rrip_sl_suf {
	char	signature[2];	/* signature word: SL */
	uchar	len;		/* SL length: */
	uchar	ver;		/* SL version: 1 */
	uchar	flags;		/* flags */
	uchar	ca[1];		/* component area */
};

/* rrip_sl_suf flags */
#define RRIP_SL_CONTINUE	0x01	/* symbolic link continues in next SL */
#define RRIP_SL_RESERVED	0xfe	/* reserved - value must be 0 */


/*
 * 	RRIP_NM: alternate name
 *
 * if no NM field is recorded for a specific directory record,
 * the ISO 9660 file identifier shall be used.
 */
struct rrip_nm_suf {
	char	signature[2];	/* signature word: NM */
	uchar	len;		/* NM length: */
	uchar	ver;		/* NM version: 1 */
	uchar	flags;		/* flags */
	char	name[1];	/* alternate name */
};

/* rrip_nm_suf flags */
#define RRIP_NM_CONTINUE	0x01	/* alternate name continues in next NM */
#define	RRIP_NM_CURRENT		0x02	/* alternate name refers to the current directory (.) */
#define RRIP_NM_PARENT		0x04	/* alternate name refers to the parent of the current directory (..) */
#define RRIP_NM_HOST		0x10	/* the local host name should be inserted as the value of the alternate name */
#define RRIP_NM_RESERVED	0xd8	/* reserved - must be 0 */ 


/*
 * 	RRIP_CL: child link
 *
 * CL is recorded in the directory record which specifies 
 * a (non-directory !) file with the same name as the moved directory. 
 * All attributes of the moved directory are stored in
 * the current (.) directory record of the moved directory.
 */
struct rrip_cl_suf {
	char	signature[2];	/* signature word: CL */
	uchar	len;		/* CL length: 12 */
	uchar	ver;		/* CL version: 1 */
	uchar	loc_cd_le[4];	/* location of child directory */
	uchar	loc_cd[4];	/* location of child directory */
};


/*
 * 	RRIP_PL: parent link
 *
 * PL is recorded in the parent (..) directory record of the moved directory.
 */
struct rrip_pl_suf {
	char	signature[2];	/* signature word: PL */
	uchar	len;		/* PL length: 12 */
	uchar	ver;		/* PL version: 1 */
	uchar	loc_pd_le[4];	/* location of parent directory */
	uchar	loc_pd[4];	/* location of parent directory */
};


/*
 * 	RRIP_RE: relocated directory
 *
 * RE is recorded in the alias directory record in the foster parent 
 * directory of the moved directory for ISO 9660 visibility only and
 * ignored for RRG processing.
 */
struct	rrip_re_suf {
	char	signature[2];	/* signature word: RE */
	uchar	len;		/* RE length: 4 */
	uchar	ver;		/* RE version: 1 */
};


/*
 * 	RRIP_TF: time stamp(s) for a file
 *
 * if TF SUF does not exist:
 *   if XAR does not exist
 *     st_ctime = st_mtime = st_atime = 
 *                Directory Record Recording Date and Time
 *   if XAR exist
 *     st_ctime = XAR File Creation Date and Time 
 *     st_mtime = XAR File Modification Date and Time 
 *     st_atime = Directory Record Recording Date and Time
 */
struct	rrip_tf_suf {
	char	signature[2];	/* signature word: TF */
	uchar	len;		/* TF length: */
	uchar	ver;		/* TF version: 1 */
	uchar	flags;		/* flags */
	uchar	time_stamps[1];		/* time stamps */
};

/* rrip_tf_suf flags */
#define RRIP_TF_CREATION	0x01	/* creation time recorded */
#define RRIP_TF_MODIFY		0x02	/* modification time recorded */
#define RRIP_TF_ACCESS		0x04	/* last access time recorded */
#define RRIP_TF_ATTRIBUTES	0x08	/* last attribute change time recorded */
#define RRIP_TF_BACKUP		0x10	/* last backup time recorded */
#define RRIP_TF_EXPIRATION	0x20	/* expiration time recorded */
#define RRIP_TF_EFFECTIVE	0x40	/* effective time recorded */
#define RRIP_TF_LONG_FORM	0x80	/* ISO 9660 17-byte time format used */


/*
 * 	RRIP_RR: RRIP fields recorded
 */
struct rrip_rr_suf {
	char	signature[2];	/* signature word: RR */
	uchar	len;		/* RR length: 5 */
	uchar	ver;		/* RR version: 1 */
	uchar	flags;		/* flags */
};

/* rrip_rr_suf flags */
#define	RRIP_RR_PX	0x01	/* PX SUF recorded */
#define	RRIP_RR_PN	0x02	/* PN SUF recorded */
#define RRIP_RR_SL	0x04	/* SL SUF recorded */
#define RRIP_RR_NM	0x08	/* NM SUF recorded */
#define RRIP_RR_CL	0x10	/* CL SUF recorded */
#define RRIP_RR_PL	0x20	/* PL SUF recorded */
#define RRIP_RR_RE	0x40	/* RE SUF recorded */
#define RRIP_RR_TF	0x80	/* TF SUF recorded */

#endif /* _H_CDRFS_RRG */
