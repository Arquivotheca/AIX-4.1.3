static char sccsid[] = "@(#)29	1.10  src/bos/kernext/cfs/cdr_diriso.c, sysxcfs, bos41J, 9521B_all 5/26/95 09:06:59";
/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS:  CDRFS directory lookup/read for ISO 9660:1998/HSG
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * 	cdr_diriso.c: directory lookup/read for 
 *		      ISO 9660:1988/HSG protocol, and
 *		      CD-ROM XA extension. 
 *
 * reference:
 * . ISO 9660:1988
 *	Information Processing - Volume and File Structure of CD-ROM
 *	for Information Interchange
 * . High Sierra:
 * . CD-ROM XA:
 *	System Description CD-ROM XA, Philips/Sony May 1991
 * . XCDR
 *	X/Open Preliminary Specification CD-ROM Support Component (XCDR), 
 *	XO/PRELIM/91/020, 1991
 */

#include <unistd.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/syspest.h>

#include "cdr_xcdr.h"
#include "cdr_xa.h"
#include "cdr_cdrfs.h"
#include "cdr_cdrnode.h"

#define FOUNDNAME	1	/* fid and name match			*/
#define	NOTFOUND	0	/* did not match			*/

BUGXDEF(buglevel);

/*
 * conventions for directory record address computation:
 *
 *   |------- lsctr -------| logical sector (2**11)
 *   |-- lblk --| logical block (2**(9+n) <= 2**11, n = 0, 1, 2)
 *   |-------------- directory extent -----------|
 *   |--- xar --|--- directory file section -----| on-disk w/ xar
 *
 *              |---- mapped directory-----------| in-memory w/o xar
 *              dstart                           dend
 *              |----------|----------+----------|
 *   |          |--de-->| address of directory record
 *   |---- dxoffset --->| offset in directory extent
 *
 * round up/down dxoffset to logical sector boundary
 * as appropriate to compute next directory record address
 * s.t. directory record address de = dstart + dxoffset - xar_len
 */

/*
 *	cdrlookup_iso
 *
 * function: called by cdr_lookup() for ISO 9660 specific directory lookup
 *
 * in: dcdrp - directory to search
 *     nmp - pathname component to search
 *
 * out: rc - == 0
 *		cdrpp - cdrnode for the pathname component
 *	   - != 0 lookup failure
 *
 * serialization: This function must be called with the cdrfs lock held.
 *		  it will be released before any calls outside the CDRFS
 *		  and across code which may page fault.
 */
int
cdrlookup_iso(dcdrp, nmp, cdrpp)
	struct cdrnode *	dcdrp;	/* cdrnode for directory	*/
	dname_t *		nmp;	/* pathanme component to search	*/
	struct cdrnode **	cdrpp;	/* cdrnode for pathname component */
{
	int	rc = 0; /* return code */
	struct cdrfsmount	*cdrfsp;	/* mounted file system data */
	char 			*dstart = NULL; /* mapped directory start */
	char 			*dend;		/* mapped directory end */
	daddr_t			pstart;		/* physical directory start */
	struct cdrdirent 	*de;		/* directory record cursor */
	int			dxoffset;	/* directory record offset in extent */
	int			xar_len;	/* xar length */
	daddr_t			dirent;		/* physical directory entry */
	int			found = FALSE;	/* entry name found ? */
	int			dir = FALSE;	/* entry is directory ? */
	time_t			d_timestamp;
	int			dotdot = FALSE;
	label_t			jmpbuf;		/* exception return jump buffer */

	/* special case for "." entry */
	if (nmp->nmlen == 1 && *nmp->nm == '.') {
		*cdrpp = dcdrp;
		return 0;
	}

	cdrfsp = dcdrp->cn_cdrfs;

	/* map directory into memory - set its start and end address */
	if (rc = cdrptovaddr(dcdrp, &dstart))
		return rc;
	dend = dstart + dcdrp->cn_size;

	pstart = LOFFSET(cdrfsp, dcdrp->cn_xd[0].cx_locdata, 0);

	/* get first directory record address and offset */
	de = (struct cdrdirent *) dstart;
	xar_len = LOFFSET(cdrfsp, dcdrp->cn_xd[0].cx_xar_len, 0); 
	dxoffset = xar_len;

	/* establish exception return point for i/o errors
	 * while we access the mapped directory
	 */
	if (rc = setjmpx(&jmpbuf)) {
		CDRFS_LOCK();
		cdrpundo(dstart);
		rc |= CDRFS_EXCEPTION;
		return rc;
	}

	/* We may page fault so release the cdrfs lock */
	CDRFS_UNLOCK();

	/* special case for ".." entry */
	if (nmp->nmlen == 2 &&
	    *nmp->nm == '.' && *(nmp->nm + 1) == '.') {
		/* locate '..' entry */
		de = (struct cdrdirent *)((char *)de + de->d_drec_len);
		/* directory: dirent = directory address */
		dirent = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
		dotdot = TRUE;
		dir = TRUE;
		found = TRUE;
		goto done;
	}

	/* scan the directory for name match
	 * assuming the sorted directory records.
	 */
	while ((caddr_t)de < dend) {
		/* test for paddings of 0x00 to the end of logical sector */
		if (de->d_drec_len < CDR_DIRENT_MIN) {
			/* advance pointer to next logical sector */
			dxoffset = NEXTLSCTR(dxoffset);
			de = (struct cdrdirent *)(dstart + dxoffset - 
							   xar_len);
			continue;
		}

		/* compare the directory entry with the pathname component
		 */
		if (cdrnamematch_iso(cdrfsp, de, nmp)) {
			/* directory entry with pathname component found:
			 * determine dirent for the entry 
			 */
			if (cdrfsp->fs_format & CDR_ISO9660)
				dir = de->d_file_flags & CD_DIR;
			else
				dir = HSDE(de)->d_file_flags & CD_DIR;

			if (dir) {
				/* directory: dirent = directory address */
				dirent = LOFFSET(cdrfsp, 
					 	 SHORT2INT(de->d_locext) + de->d_xar_len, 0);
				/* save time stamp to pass down */
				d_timestamp = cdrgettime(&de->d_rec_time);
			} else
				/* non-directory: dirent = directory entry 
				 * address in parent directory 
			 	 */
				dirent = (daddr_t)(pstart +
					  ((char *)de - dstart));

			found = TRUE;
			goto done;
		}

		/* advance pointer to next directory entry */
		dxoffset += de->d_drec_len;
		de = (struct cdrdirent *)((char *)de + de->d_drec_len);
	} /* end while */
	
done:
	/* unmap directory from memory, pop exception return */
	if (dstart)
		cdrpundo(dstart);
	cdrfs_clrjmpx(&jmpbuf);

	/* iget() the cdrnode for the pathname component */
	if (found) {
		if (dir) {
			/* get cdrnode for directory file */
			if (dirent == dcdrp->cn_dirent)
				/* root directory ".." entry */
				*cdrpp = dcdrp;
			else {
				rc = cdrget(dcdrp->cn_dev,
					    dirent,
					    0,
					    cdrpp);
			}
			
			/* inherit timestamp from directory record
			 * in the parent directory (for HSG '.')
			 */
			if (rc == 0 && !dotdot &&
			    (*cdrpp)->cn_ctime == 0) {
				(*cdrpp)->cn_ctime = d_timestamp;
				(*cdrpp)->cn_mtime = d_timestamp;
				(*cdrpp)->cn_atime = d_timestamp;
			}
		} else
			/* get cdrnode for non-directory file */ 
			rc = cdrget(dcdrp->cn_dev,
				    dirent,
				    dcdrp->cn_dirent,
				    cdrpp);
	}

	if (rc)
		return rc;
	else
		return (found ? 0 : ENOENT);
}


/*
 * NAME: cdrnamematch_iso
 *
 * FUNCTION: Takes the pathname to look up and matches it with a
 *	     directory entry passed in. It matches first on file
 *	     name, then on file extension, and finally file version
 *	     numbers.
 *
 * NOTE: The ISO-9660 standard requires pathnames in a certain formats
 *	 match the specified directory entries:
 *		Examples:	nmp = AUSTIN	de = AUSTIN; 
 *				nmp = AUSTIN.   de = AUSTIN.;
 *				nmp = AUSTIN.;  de = AUSTIN.;XXX
 *				nmp = AUSTIN;   de = AUSTIN.;
 * 
 * RETURNS: FOUNDNAME - The directory entry matches the pathname
 *	    NOTFOUND  - The directory entry does not match the pathname
 */
static
int
cdrnamematch_iso(cdrfsp, de, nmp)
	struct cdrfsmount	*cdrfsp;	/* mounted file system data */
	struct cdrdirent	*de;		/* directory record */
	dname_t	*nmp;				/* name */
{
	register char	*dn;	/* directory file identifier */
	int	dnl;		/* directory file identifier length */
	char	d; 
	register char	*pn;	/* pathanme component file identifier */
	int	pnl;		/* pathname component file identifier length */
	char	pnbuf[MAXNAMLEN + 1]; /* pathname component buffer */
	char	*s;
	int	i;
	
	/* set up file identifier and length */
	dnl = de->d_fileid_len;
	dn = de->d_file_id;
	pnl = nmp->nmlen;
	pn = nmp->nm; /* pathname component is null terminated */

	/* translate pathname component to upper case: XCDR cdmntsuppl_l */
	s = pnbuf;
	for (d = *pn++, i = pnl; i > 0; d = *pn++, i--)
		*s++ = ((d >= 'a' && d <= 'z') ? d - 'a' + 'A' : d);
	*s = '\0';
	pn = pnbuf;

	/* compare first characters as quick check for difference
	 */
	if (*dn != *pn)
		return NOTFOUND;

	/* match file name
	 */
	while (dnl && (d = *dn) == *pn && d != '.') {
		dnl--, dn++, pnl--, pn++;
	}

	if (dnl == 0) {
		if (*pn == '\0')
			return FOUNDNAME;
		else
			return NOTFOUND;
	}

	if (d != '.')  {
		if (*pn == '\0' && d == ';')
			return FOUNDNAME;
		else
			return NOTFOUND;
	} else {
		if (d != *pn) {
			if (*pn == '\0' || *pn == ';')
				dnl--, dn++;
			else
				return NOTFOUND;
		} else
			dnl--, dn++, pnl--, pn++;
	}

	/* match file name extension
	 */
	while (dnl && (d = *dn) == *pn && d != ';') {
		dnl--, dn++, pnl--, pn++;
	}

	if (dnl == 0) {
		if (*pn == '\0')
			return FOUNDNAME;
		else
			return NOTFOUND;
	}

	if (d != ';')
		return NOTFOUND;
	else {
		if (*pn == '\0')
			return FOUNDNAME;
		else if (d != *pn)
			return NOTFOUND;
		else
			dnl--, dn++, pnl--, pn++;
	}

	/* match file version number
	 * NB: version number is sorted in descending order
	 */
	if (dnl == pnl) {
		while (dnl &&  *dn == *pn) {
			dnl--, dn++, pn++;
		}
		if (dnl == 0) /* end of dname */
			return FOUNDNAME;
	}

	/* version numbers do not match
	 */
	return NOTFOUND;
}


/*
 *	cdrread_iso
 *
 * function: called by cdrget() to initialize ISO 9660/HSG cdrnode.
 *
 * in/out: cdrp - cdrnode to finalize from on-disk directory entry 
 *	      (refer to cdrget() for initialization of the cdrnode)
 *
 * serialization: This function must be called with the cdrfs lock held.
 *		  it will be released before any calls outside the CDRFS
 *		  and across code which may page fault.
 */
int
cdrread_iso (cdrp)
	struct cdrnode *	cdrp;
{
	int		rc = 0;
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	int		format;
	struct cdrnode 	*dcdrp;		/* directory cdrnode */
	daddr_t		dirent;		/* physical directory entry */
	daddr_t		pdirent;	/* physical parent directory */
	char 		*dstart = NULL;	/* mapped directory start */
	char 		*dend;		/* mapped directory end */
	struct cdrdirent 	*de;	/* directory record cursor */
	int		doffset;	/* directory record offset in file section */
	int		dxoffset;	/* directory record offset in extent */
	struct cdrxd	*xd;		/* extent descriptor */
	int		xdn = 0; 
	struct cdrxd	*xdtail = NULL;
	int		xar_len;	/* xar length */
	uchar		file_flags;
	label_t		jmpbuf;		/* exception return jump buffer	*/

	cdrfsp = cdrp->cn_cdrfs;
	format = cdrfsp->fs_format;

	/* determine the directory to map and 
	 * the directory entry to read
	 */
	dirent = cdrp->cn_dirent;
	pdirent = cdrp->cn_pdirent;

	if (pdirent) {
		/* cdrnode for a non-directory:
		 * map parent directory into memory to read 
		 * the directory entry in the parent directory
		 */
		if (rc = cdrget(cdrp->cn_dev, pdirent, 0, &dcdrp))
			return rc;
		doffset = dirent - LOFFSET(cdrfsp,
					   dcdrp->cn_xd[0].cx_locdata,
					   0);
	} else {
		/* cdrnode for a directory:
		 * fill in enough/fake information in cdrnode so that 
		 * the strategy routine can satisfy paging requests
		 * to map '.' and '..' entry of the directory. 
		 */
		cdrp->cn_xd[0].cx_locdata = LBLKNO(cdrfsp, dirent);
		cdrp->cn_xd[0].cx_data_len = CDR_PAGESIZE;
		cdrp->cn_size = CDR_PAGESIZE;
		doffset = 0;

		/* set cdrp to read from */
		dcdrp = cdrp;
	}

	if (rc = cdrptovaddr(dcdrp, &dstart))
		return rc;
	dend = dstart + dcdrp->cn_size;

	/* establish exception return point for i/o errors
	 * while we access the mapped directory
	 */
	if (rc = setjmpx(&jmpbuf))
	{
		CDRFS_LOCK();
		cdrpundo(dstart);
		rc |= CDRFS_EXCEPTION;
		return rc;
	}

	/* We may page fault so release the cdrfs lock */
	CDRFS_UNLOCK();

	/* locate the directory entry to read
	 */
	de = (struct cdrdirent *)(dstart + doffset);
	if (pdirent)
		/* XAR length in parent directory */
		xar_len = LOFFSET(cdrfsp, dcdrp->cn_xd[0].cx_xar_len, 0); 
	else {
		/* XAR length of the directory from its "." entry */
		xar_len = LOFFSET(cdrfsp, de->d_xar_len, 0); 
	}
	dxoffset = xar_len + doffset;

	/* initialize for extent descriptor array/list */
	cdrp->cn_size = 0;

	/* non-final directory records of the directory entry
	 */
	while ((caddr_t)de < dend) {
		/* test for paddings of 0x00 to the end of logical sector */
		if (de->d_drec_len < CDR_DIRENT_MIN) {
			/* advance pointer to next logical sector */
			dxoffset = NEXTLSCTR(dxoffset);
			de = (struct cdrdirent *)(dstart + dxoffset - 
							   xar_len);
			continue;
		}

		/* single volume file system only: 
	 	 * check for d_volseqno == pvd_volseqno is suppressed 
		 * to make anti-social disc right.
		 */

		/*  multi-extent for regular file only */
		if (format & CDR_ISO9660)
			file_flags = de->d_file_flags;
		else
			file_flags = HSDE(de)->d_file_flags;

		if (file_flags & CD_MULTIEXT) {
			if (file_flags & CD_DIR) {
				rc = EFORMAT;
				goto done;
			}
		} else /* only/final extent */
			break;

		/* check file type of interleaved mode */
		if (format & CDR_ISO9660) { 
			if (de->d_file_usize || de->d_ileav_gsize)
				cdrp->cn_format = CD_NTRLVD;
		} else {
			if (HSDE(de)->d_file_usize || HSDE(de)->d_ileav_gsize)
				cdrp->cn_format = CD_NTRLVD;
		}

		/* create and fill in extent descriptor */
		if (rc = cdrmakexd(cdrp, de, &xdn, &xdtail))
				goto done;

		/* update file size in cdrnode */
		cdrp->cn_size += SHORT2INT(de->d_data_len);

		/* advance pointer next directory entry */
		dxoffset += de->d_drec_len;
		de = (struct cdrdirent *)((char *)de + de->d_drec_len);
	} /* end while */

	/* make sure directory scan did not fall off */
	if ((caddr_t) de >= dend) {
		rc = EFORMAT;
		goto done;
	}

	/* 
	 * only/final directory record of the directory entry
	 */

	/* check file type of interleaved mode */
	if (format & CDR_ISO9660) { 
		if (de->d_file_usize || de->d_ileav_gsize)
			cdrp->cn_format = CD_NTRLVD;
	} else {
		if (HSDE(de)->d_file_usize || HSDE(de)->d_ileav_gsize)
			cdrp->cn_format = CD_NTRLVD;
	}

	/* update file size and number of blocks used in cdrnode */
	cdrp->cn_size += SHORT2INT(de->d_data_len);
	cdrp->cn_nblocks = 1 + LBLKNO(cdrfsp, cdrp->cn_size - 1);

	/* create and fill in extent descriptor */
	if (rc = cdrmakexd(cdrp, de, &xdn, &xdtail))
		goto done;

	/* set 'i_number' of the cdrnode to the file object address */
	cdrp->cn_number = cdrp->cn_xd[0].cx_locdata << LBLKSHIFT(cdrfsp);

	/* fill in other non-zero default attributes */
	if (file_flags & CD_DIR)
		cdrp->cn_nlink = 2;
	else
		cdrp->cn_nlink = 1;

	/* 
	 * fill in default/XAR/SUA attributes
	 */
	if (format == CDR_ISO9660 ||
	    format == CDR_HIGHSIERRA) {
		/* 
	 	 * ISO 9660/HSG format
		 */
		if (de->d_xar_len == 0) {
			/* fill in with default attributes */
			/* owner uid/gid */
			/* 
			cdrp->cn_uid = 0;
			cdrp->cn_gid = 0;
			*/
	
			/* file type, permission */
			cdrp->cn_mode =	((file_flags & CD_DIR) ? IFDIR : IFREG) |
					CDR_PERMISSION; 

			/* time stamps */
			cdrp->cn_ctime = cdrgettime(&de->d_rec_time);
			cdrp->cn_mtime = cdrp->cn_ctime;
			cdrp->cn_atime = cdrp->cn_ctime;
		} else {
			/* read attributes from Extended Attribute Record (XAR):
		 	 * uid/gid, permission, time stamp
		 	 */
			if ((rc = cdrgetxar(format, cdrp, de)) != 0)
				goto done;
		}
	} else {
		/* 
		 * CD-ROM XA extension
		 */
		if ((rc = cdrxagetsua(cdrp, de)) != 0)
				goto done;
	}
		
	/* for directory cdrnode, set its parent's directory address 
	 * from the (..) entry of the child directory
	 */
	if (!pdirent) {
		de = (struct cdrdirent *)
				((char *)de + de->d_drec_len);
		cdrp->cn_pdirent =
			LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
	}

done:
	/* unmap the segment, clear exception return */
	if (dstart)
		cdrpundo(dstart);
	cdrfs_clrjmpx(&jmpbuf);
	
	if (pdirent)
		cdrput(dcdrp);	/* release parent cdrnode */

	return rc;
}


/*
 *	cdrgetxar
 *
 * read attributes from Extended Attribute Record (XAR):
 * set cdrnode uid/gid, permission, time stamp.
 */
static
int
cdrgetxar(format, cdrp, de)
	int			format;
	struct cdrnode		*cdrp;
	struct cdrdirent 	*de;
{
	int			rc = 0;
	struct cdrfsmount	*cdrfsp;	/* mounted file system data */
	struct cdrxar 		*xar = NULL;	/* extended attribute record */
	uchar			flags;
	mode_t			mode;
	ushort			perm;

	cdrfsp = cdrp->cn_cdrfs;

	/* read External Attribute Record (XAR) */
	if ((xar = (struct cdrxar *)malloc(CDR_PBLKSIZE)) == NULL)
		return ENOMEM;
					
	if (rc = cdrbread(cdrfsp->fs_fp, LOFFSET(cdrfsp, SHORT2INT(de->d_locext), 0), 
			  CDR_PBLKSIZE, xar))
		goto done;

	/* time stamps: if XAR timestamp is not specified (0),
	 * get the timestamp from directory entry
	 */
	if ((cdrp->cn_ctime = cdrgettime_long(&xar->xar_cre_time)) == 0)
		cdrp->cn_ctime = cdrgettime(&de->d_rec_time);
	if ((cdrp->cn_mtime = cdrgettime_long(&xar->xar_mod_time)) == 0)
		cdrp->cn_mtime = cdrgettime(&de->d_rec_time);
	cdrp->cn_atime = cdrp->cn_mtime;

	/* for CD-ROM XA, its System Use information takes precedence. */
	if (format == CDR_XA)
		return 0;

	if (format & CDR_ISO9660)
		flags = de->d_file_flags;
	else
		flags = HSDE(de)->d_file_flags;

	/* translate file type */
	mode = (flags & CD_DIR) ? IFDIR : IFREG;

	/* check for no protection flag for restricted XAR */
	if (!(flags & CD_PROTEC)) {
		/* 
		cdrp->cn_uid = 0;
		cdrp->cn_gid = 0;
		*/
		cdrp->cn_mode = CDR_PERMISSION | mode; 
		goto done;
	}

	/* owner uid/gid */
	cdrp->cn_uid = xar->xar_own_id;
	cdrp->cn_gid = xar->xar_grp_id;

	/* translate access permissions
	 * NB: permission is granted when the bit is set to 0
	 */
	perm = xar->xar_permissions;
	if (perm & CD_RUSR == 0)
		mode |= S_IRUSR;
	if (perm & CD_XUSR == 0)
		mode |= S_IXUSR;
	if (perm & CD_RGRP == 0)
		mode |= S_IRGRP;
	if (perm & CD_XGRP == 0)
		mode |= S_IXGRP;
	if (perm & CD_ROTH == 0)
		mode |= S_IROTH;
	if (perm & CD_XOTH == 0)
		mode |= S_IXOTH;

	cdrp->cn_mode = mode;

done:
	/* Free extended attribute storage */
	free(xar);

	return rc;
}


/* number of days (for non-leap year) before beginning of month; 
 */
int yday[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/*
 *	cdrgettime
 * 
 * convert broken-down time in Directory Record to
 * seconds since the Epoch (00:00:00, January 1970 UTC)
 * note: the time (without time-zone) is assumed to be in UTC. 
 */
time_t
cdrgettime (ts)
	struct cdrtime	*ts;
{
	unsigned long	t = 0;
	int	years, days;

	years = ts->d_year;
	days = years * 365;

	/* compute leap days since 1900:
	 * note that centennial year is not leap year unless
	 * divisible by 400 (e.g., 1900 is not leap year)
	 * i.e., we only looks for brief period of 1801 - 2099 AD
	 */
	if (years > 0) {
		days += (years - 1) / 4;
		/* check for leap day for current year */
		if ((years % 4) == 0 && ts->d_month > 2)
			days += 1;
	} else
		return 0;

	/* days, month of the year, day of the month */
	t += (days + yday[ts->d_month - 1] + ts->d_day - 1) * 24 * 60 * 60;

	/* hour of the day */
	t += ts->d_hour * 60 * 60;

	/* minute of the hour */
	t += ts->d_minute * 60;

	/* second of the minute */
	t += ts->d_second;

	/* correct zero time from (00:00:00, 1/1/1900) 
	 * to the Epoch (00:00:00, 1/1970):
	 * cough up days for 70 years (with 17 leap years (1900 is not))	
	 */
	t -= (70 * 365 + 17) * 24 * 60 * 60;

	if (t > 0)
		return ((time_t)t);
	else
		return 0;
}


/*
 *	cdrgettime_long
 *
 * convert ascii broken-down time in VD/XAR  to
 * seconds since the Epoch (00:00:00, January 1970 UTC)
 */
time_t
cdrgettime_long(d)
	char	*d;
{
	struct cdrtime	t;
	int	x, i; 

	/* convert VD/XAR timestamp (in alphanumeric) into 
	 * Directory Record timestamp (in numeric) format
	 */
	for (x = 0, i= 0; i < 4; i++)
		x = x * 10 + (*d++ - '0');
	if (x > 0)
		t.d_year = x - 1900; /* convert to years since 1900 */ 
	else
		return 0; /* skip if unspecified/invalid timestamp */

	for (x = 0, i= 0; i < 2; i++)
		x = x * 10 + (*d++ - '0');
	t.d_month = x; 

	for (x = 0, i= 0; i < 2; i++)
		x = x * 10 + (*d++ - '0');
	t.d_day = x; 

	for (x = 0, i= 0; i < 2; i++)
		x = x * 10 + (*d++ - '0');
	t.d_hour = x; 

	for (x = 0, i= 0; i < 2; i++)
		x = x * 10 + (*d++ - '0');
	t.d_minute = x; 

	for (x = 0, i= 0; i < 2; i++)
		x = x * 10 + (*d++ - '0');
	t.d_second = x; 

	/* process as Directory Record timestamp format */
	return (cdrgettime(&t));

}


/*
 *	cdrmakexd - make extent descriptor
 */
int
cdrmakexd (cdrp, de, xdn, xdtail)
	struct cdrnode	*cdrp;
	struct cdrdirent *de;
	int	*xdn;
	struct cdrxd 	**xdtail;
{
	struct cdrxd	*xd;

	/* create extent descriptor */
	if (*xdn <= CDR_NXD)
		xd = &cdrp->cn_xd[*xdn++];
	else {
		xd = (struct cdrxd *) malloc(sizeof *xd);
		if (xd == NULL)
			return ENOMEM;
		if (*xdtail)
			(*xdtail)->cx_next = xd;
		else
			cdrp->cn_xdlist = xd;
		*xdtail = xd;
	}

	/* fill in fields of extent descriptor */
	xd->cx_next =		NULL;
	xd->cx_locdata =	SHORT2INT(de->d_locext) + de->d_xar_len;
	xd->cx_data_len =	SHORT2INT(de->d_data_len);
	xd->cx_xar_len =	de->d_xar_len;

	return 0;
}

/*
 *	cdrxagetsua
 *
 * read attributes from CD-ROM XA System Use Area to
 * set cdrnode uid/gid, permission, file format.
 */
static
int
cdrxagetsua(cdrp, de)
	struct cdrnode	*cdrp;
	struct cdrdirent 	*de;
{
	int			rc = 0;
	struct cdrxa_sua	*cdrxasua;
	int			sualen;
	uchar			*sua;
	uchar			flags;
	ushort			attr;
	mode_t			perm;

	/* determine system use area */
	sualen = de->d_drec_len - de->d_fileid_len - CDR_DIRENT_FIXED;
	if ((de->d_fileid_len & 0x01) == 0)
		sualen -= 1;
	if (sualen > 0) 
		sua = (char *)de + de->d_drec_len - sualen;
	else
		return 0; /* CD-ROM Mode 1 file may not have SUA */ 

	cdrxasua = (struct cdrxa_sua *) sua;

	if (cdrxasua->signature != CDRXA_SUA_SIGNATURE)
		return EFORMAT;

	/* owner uid/gid */
	cdrp->cn_uid = cdrxasua->uid;
	cdrp->cn_gid = cdrxasua->gid;

	/* file type, permission */
	attr = cdrxasua->attributes;

	perm = 0;
	if (attr & CDRXA_IRUSR)
		perm |= S_IRUSR;
	if (attr & CDRXA_IXUSR)
		perm |= S_IXUSR;
	if (attr & CDRXA_IRGRP)
		perm |= S_IRGRP;
	if (attr & CDRXA_IXGRP)
		perm |= S_IXGRP;
	if (attr & CDRXA_IROTH)
		perm |= S_IROTH;
	if (attr & CDRXA_IXOTH)
		perm |= S_IXOTH;

	flags = de->d_file_flags;
	cdrp->cn_mode =	((flags & CD_DIR) ? S_IFDIR : S_IFREG) | perm; 

	if (de->d_xar_len == 0) {
		/* time stamps */
		cdrp->cn_ctime = cdrgettime(&de->d_rec_time);
		cdrp->cn_mtime = cdrp->cn_ctime;
		cdrp->cn_atime = cdrp->cn_ctime;
	} else {
		/* read time stamp from Extended Attribute Record (XAR):
	 	 * System Use information takes precedence over XAR
		 * for uid/gid, permission.
	 	 */
		if ((rc = cdrgetxar(CDR_XA, cdrp, de)) != 0)
			return rc;
	}

	/* file format */
	cdrp->cn_format = (attr & CDRXA_IFMT) >> CDRXA_IFMT_SHFT;

	return rc;
}


#define CDRDIRSIZ(namlen) \
    ((sizeof (struct dirent) - (MAXNAMLEN+1)) + ((namlen+1 + 3) & ~3))

/*
 *	cdrreaddir_iso
 *
 * function: readdir() for ISO 9660/HSG directory  -
 * 	     read struct cdrdirent from disk and 
 *	     translate into struct dirent; 
 */
int
cdrreaddir_iso (cdrp, uiop)
	struct cdrnode *	cdrp;
	struct uio *		uiop;
{
	int		rc = 0;		/* return code */
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	char 		*dstart = NULL;	/* mapped directory start */
	char 		*dend;		/* mapped directory end */
	daddr_t		pstart;		/* physical directory start */
	struct cdrdirent 	*de;	/* directory record cursor */
	int		doffset;	/* directory record offset in file section */
	int		dxoffset;	/* directory record offset in extent */
	int		xar_len;	/* xar length */
	off_t		start;		/* readdir() start offset in directory */
	caddr_t		tbuf;		/* buffer for struct dirents */
	struct dirent *tdirp;		/* dirent cursor */
	int		tbytes;		/* buffer size */
	int		ubytes;		/* uiomove() count */
	int		format;		/* format of CDROM disk	*/
	uchar		flags;		/* directory entry flags */
	int		nmlen;		/* name length */
	char		*fid;		/* file identifier */
	int		fidlen;		/* file identifier length */
	char *		lastfid;	/* last file identifier */
	int		lastfidlen;	/* last file identifier length */
	struct dirent	*lasttdirp;	/* last dirent cursor */
	char		*s, c;
	int		sl;
	label_t		jmpbuf;		/* exception return jump buffer	*/

	start = uiop->uio_offset;
	if (start >= cdrp->cn_size)
		return 0;

	cdrfsp = cdrp->cn_cdrfs; 

	/* allocate buffer for struct dirents for the amount of space 
	 * requested, but no more than the maximum space that would be 
	 * required for all the directory entries that could possibly 
	 * be in the directory
	 */
	tbytes = MIN(uiop->uio_resid,
		     (sizeof(struct dirent) - MAXNAMLEN + CDR_ISO_NAME_MAX) * 
		     (cdrp->cn_size - start) / CDR_DIRENT_MIN);
	if ((tbuf = malloc(tbytes)) == NULL)
		return ENOMEM;

	tdirp = (struct dirent *) tbuf;

	/* force/round-down offset to Directory Record boundary - 
	 * either the first Directory Record (after optional XAR)
	 * or start of Logical Sector 
	 */  
	xar_len = cdrp->cn_xd[0].cx_xar_len << LBLKSHIFT(cdrfsp);
	dxoffset = start + xar_len;
	dxoffset = LASTLSCTR(dxoffset);
	if (dxoffset < xar_len)
		dxoffset = xar_len;
	doffset = dxoffset - xar_len;

	/* map directory into memory */
	if (rc = cdrptovaddr(cdrp, &dstart))
		goto out;
	dend = dstart + cdrp->cn_size;
	pstart = cdrp->cn_xd[0].cx_locdata << LBLKSHIFT(cdrfsp);

	de = (struct cdrdirent *)(dstart + doffset);

	format = cdrfsp->fs_format;

	/* establish exception return point for i/o errors
	 * while we access the mapped directory
	 */
	if (rc = setjmpx(&jmpbuf)) {
		free(tbuf);
		CDRFS_LOCK();
		cdrpundo(dstart);
		rc |= CDRFS_EXCEPTION;
		return rc;
	}

	/* We may page fault so release the cdrfs lock */
	CDRFS_UNLOCK();

	/* translate each directory entry into struct dirent
	 */
	ubytes = 0;
	lastfid = NULL;
	lastfidlen = -1;

	while ((caddr_t)de < dend) {
		/* test for paddings of 0x00 to the end of logical sector */
		if (de->d_drec_len < CDR_DIRENT_MIN) {
			/* advance pointer to next logical sector */
			dxoffset = NEXTLSCTR(dxoffset);
			doffset = dxoffset - xar_len;
			de = (struct cdrdirent *)(dstart +  doffset);
			continue;
		}

		/* skip entries before the starting offset */
		if (doffset < start)
			goto next_drecord;

		if (format & CDR_ISO9660)
			flags = de->d_file_flags;
		else
			flags = HSDE(de)->d_file_flags;

		/* determine file identifier */
		if (flags & CD_DIR) { 
			/* special case for "." and ".." entry */
			if (de->d_fileid_len == 1) {
				fidlen = 1;
				if (de->d_file_id[0] == 0x00) {
					nmlen = 1;
					goto xlate_record;	
				} else if (de->d_file_id[0] == 0x01) {
					nmlen = 2;
					goto xlate_record;	
				} else
					nmlen = 1;
			} else {
				fidlen = de->d_fileid_len;
				nmlen = fidlen;
			}
		} else {
			/* XCDR: if (cdrfsp->fs_xcdr_cdmntsuppl & CD_NOVERSION)
			 * present only the latest version with its 
			 * version number suppressed
			 */
			/* look for a version indicator within the fid */
			sl = de->d_fileid_len;
			s = de->d_file_id;
			while (sl && *s != ';')
				sl--, s++;

			/* get size of fid minus version number */
			fidlen = de->d_fileid_len - sl;

			/* Currently support Sun convention:
			 * present '.' for null extension
			 */
			/* XCDR: if (cdrfsp->fs_xcdr_cdmntsuppl & CD_LOWER)
			 * suppress '.' for null extension.
			if (*(s - 1) == '.')
				nmlen = fidlen - 1;
			else
			 */
				nmlen = fidlen;
		}

		/* skip subsequent multi-extent/version directory records
		 * of the previous file by skipping directory records with
		 * the same filename.filename_extension
		 */
		if (fidlen == lastfidlen &&
		    strncmp(de->d_file_id, lastfid, fidlen) == 0) {
			/* update offset of next directory entry of
			 * the last dirent translated:
			 * set the next directory entry offset by
		 	 * skipping all the multi-version/extent directory
		 	 * records of the same filename.filename_extension
		 	 * so that next readdir() for the same directory
		 	 * will start with new directory entry/name
		 	 */
			lasttdirp->d_offset = doffset + de->d_drec_len;
			goto next_drecord;
		} 
		

xlate_record: /* translate directory entry into struct dirent */

		/* check if this entry will fit */
		if ((ubytes + CDRDIRSIZ(nmlen)) > tbytes)
			break;

		/* set the offset to the next directory entry */
		tdirp->d_offset = doffset + de->d_drec_len;

		/* set 'i_number' of the entry to the address of the object */
		tdirp->d_ino = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);

		/* set the name and name length for the entry */
		tdirp->d_namlen = nmlen;

		if (de->d_fileid_len == 1) {
			/* special case for "." and ".." entry */
			c = de->d_file_id[0];
			if (c == 0x00)
				bcopy(".", tdirp->d_name, nmlen);
			else if (c == 0x01)
				bcopy("..", tdirp->d_name, nmlen);
			else
				/* (cdrfsp->fs_xcdr_cdmntsuppl & CD_LOWER):
			 	 * convert to lower case
			 	 */
				*tdirp->d_name = ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c);
		} else {
			/* XCDR: if (cdrfsp->fs_xcdr_cdmntsuppl & CD_LOWER)
			 * convert to lower case
			 */
			for (s = tdirp->d_name, sl = nmlen, fid = de->d_file_id, c = *fid;
			     sl; sl--, c = *++fid)
				*s++ = ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c);
		}
		tdirp->d_name[nmlen] = '\0';

		/* set the struct dirent record length */
		tdirp->d_reclen = DIRSIZ(tdirp);

		lastfid = de->d_file_id;
		lastfidlen = fidlen;
		lasttdirp = tdirp;

		/* update number of bytes and buffer pointer.	*/
		ubytes += tdirp->d_reclen;
		tdirp = (struct dirent *)((char *)tdirp + tdirp->d_reclen);

next_drecord:
		/* advance pointer to next directory entry */
		doffset += de->d_drec_len;
		dxoffset += de->d_drec_len;
		de = (struct cdrdirent *)((char *)de + de->d_drec_len);
	} /* end while */

	if (ubytes > 0) {
		rc = uiomove(tbuf, ubytes, UIO_READ, uiop);

		/* establish real offset */
		uiop->uio_offset = doffset;
	}

out:
	/* unmap directory from memory, pop exception return */
	if (dstart)
		cdrpundo(dstart);
	cdrfs_clrjmpx(&jmpbuf);

	free(tbuf);
	return rc;
}
