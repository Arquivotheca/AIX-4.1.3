/* @(#)24	1.3  src/bos/kernext/cfs/cdr_vd.h, sysxcfs, bos411, 9428A410j 2/19/93 15:22:34 */

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
 *	cdr_vd.h: Volume Descriptor
 *
 * note: cdr_vd.h is included in cfs_cdrfs.h to generate customized 
 * definition of Volume Descriptor for ISO 9660:1988 and HSG format
 */

/*
 *	Volume Descriptor
 *
 * The Volume Descriptor Set is a sequence of Volume Descriptors
 * recorded in consecutively numbered Logical Sectors starting 
 * with Logical Sector Number 16 (start of Volume Space Data Area).
 * Each Volume Descriptor resides in a single Logical Sector.
 */

/* 
 * generic volume descriptor
 */
#ifdef HIGHSIERRA
struct hscdrvd
#else
struct cdrvd
#endif
{
#ifdef HIGHSIERRA
	uint	vd_lblkn_le;	
	uint	vd_lblkn;	/* logical block number			*/
#endif
	uchar	vd_voldestype;	/* volume descriptor type: CDRFS_PVD	*/
	uchar	vd_std_id[5];	/* standard identifier: CD001 or CDROM	*/
	uchar	vd_voldesvers;	/* volume descriptor version		*/
#ifdef HIGHSIERRA
	char	vd_other[CDR_VDSIZE - 15];	/* fill to size of vd	*/
#else
	char	vd_other[CDR_VDSIZE - 7];	/* fill to size of vd	*/
#endif
};


/* 
 *	Primary Volume Descriptor 
 */
#ifdef HIGHSIERRA
struct hscdrpvd
#else
struct cdrpvd
#endif
{
#ifdef HIGHSIERRA
	uint	pvd_lblknum_bs; 
	uint	pvd_lblknum;	/* logical block number			*/
#endif
	uchar	pvd_voldestype;	/* volume descriptor type: CDRFS_PVD	*/
	uchar	pvd_std_id[5];	/* standard identifier: CD001 or CDROM	*/
	uchar	pvd_voldesvers;	/* volume descriptor version		*/
	uchar	pvd_unused1;	/* unused: set to 0x00			*/
	uchar	pvd_sys_id[32];	/* system identifier			*/
	uchar	pvd_vol_id[32];	/* volume identifier			*/
	uchar	pvd_unused2[8];	/* unused: set to 0x00			*/
	/* offset:  80 bytes (ISO9660) or 88 bytes (High Sierra) */
	uint	pvd_volspcsize_le;
	uint	pvd_volspcsize;	/* volume space size in lblk	*/
	uchar	pvd_unused3[32];	/* unused: set to 0x00		*/
	/* offset:  120 bytes (ISO9660) or 128 bytes (High Sierra) */
	ushort	pvd_volsetsize_le;
	ushort	pvd_volsetsize;	/* volume set size (# volumes in set)	*/
	ushort	pvd_volseqno_le;
	ushort	pvd_volseqno;	/* volume sequence number in volume set	*/
	ushort	pvd_lblksize_le;
	ushort	pvd_lblksize;	/* logical block size in bytes		*/
	uint	pvd_ptsize_le;
	uint	pvd_ptsize;	/* path table size in bytes		*/
	uint	pvd_locpt_l;	/* type L path table	*/
	uint	pvd_locptopt_l;	/* optional type L path table	*/
	uint	pvd_locpt_m;	/* type M path table	*/
	uint	pvd_locptopt_m;	/* optional type M path table */
#ifdef HIGHSIERRA
	uchar	pvd_removed[16]; /* high sierra, removed from ISO9660	*/
#endif
	/* offset:  156 bytes (ISO9660) or 180 bytes (High Sierra) */
	struct cdrdirent	pvd_rootdir; /* directory record for 
					      * root directory (34 bytes) */
	uchar	pvd_volset_id[128];	/* volume set identifier	*/
	uchar	pvd_pub_id[128];	/* publisher indentifier	*/
	uchar	pvd_dtpre_id[128];	/* data preparer identifier	*/
	uchar	pvd_app_id[128];	/* application identifier	*/
	/* offset:  702 bytes (ISO9660) or 726 bytes (High Sierra) */
	uchar	pvd_cpfile_id[37];	/* copyright file identifier	*/
	uchar	pvd_abfile_id[37];	/* abstract file identifier	*/
	uchar	pvd_bgfile_id[37];	/* bibliographic file identifier */
	/* offset:  813 bytes (ISO9660) or 837 bytes (High Sierra) */
	struct cdrtime_long	pvd_cre_time;	/* volume creation time		*/
	struct cdrtime_long	pvd_mod_time;	/* volume modification time	*/
	struct cdrtime_long	pvd_exp_time;	/* volume expiration time	*/
	struct cdrtime_long	pvd_eff_time;	/* volume effective time	*/
	/* offset:  881 bytes (ISO9660) or 905 bytes (High Sierra) */
	uchar	pvd_filestrver;		/* file structure version	*/
	uchar	pvd_res1;		/* reserved: set to 0x00	*/
	uchar	pvd_appuse[512];	/* application use	*/
#ifdef HIGHSIERRA
	uchar	pvd_res2[629];		/* reserved: set to 0x00	*/
#else
	uchar	pvd_res2[653];		/* reserved: set to 0x00	*/
#endif
};


/* 
 *	Supplementary Volume Descriptor
 *
 * Supplementary Volume Descriptor is not interpreted by CDRFS.
 */

/* 
 *	Boot Record
 *
 * Boot Record is not interpreted by CDRFS.
 */

/* 
 *	Volume Partition Descriptor
 *
 * Volume Partition Descriptor is not interpreted by CDRFS.
 */


/* 
 *	Volume Descriptor Set Terminator
 *
 * The recorded set of Volume Descriptors are terminated by a 
 * sequence of one or more Volume Descriptor Set Terminators.
 */
#ifdef HIGHSIERRA
struct hscdrvdst
#else
struct cdrvdst
#endif
{
#ifdef HIGHSIERRA
	uchar	vd_lblknum[8];	/* logical block number			*/
#endif
	uchar	tr_vdt;		/* volume desc type: CDR_VDST	*/
	uchar	tr_std_id[5];	/* standard identifier*/
	uchar	tr_voldesvers;	/* volume descriptor version		*/
#ifdef HIGHSIERRA
	uchar	tr_rsrv[2033];	/* unused:  set to 0x00			*/
#else
	uchar	tr_rsrv[2041];	/* unused:  set to 0x00			*/
#endif
};
