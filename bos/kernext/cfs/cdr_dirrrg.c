static char sccsid[] = "@(#)30	1.8  src/bos/kernext/cfs/cdr_dirrrg.c, sysxcfs, bos411, 9428A410j 10/21/93 08:23:23";
/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS: CDRFS directory lookup/read for RRG 
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
 * 	cdr_dirrrg.c: directory lookup/read for Rock Ridge Group (RRG) protocols
 *
 * reference: 
 * . ISO 9660:1988
 *	Information Processing - Volume and File Structure of CD-ROM
 *	for Information Interchange
 * . System Use Sharing Protocol, 
 *	A Mechanism for Extensible Sharing of ISO 9660:1988 System
 *	Use Areas, 
 *	Version 1, Revision 1.09, 1992 
 * . Rock Ridge Interchange Protocol, 
 *	An ISO 9660:1988 Compliant Approach To Providing Adequate CD-ROM
 *	Support for POSIX File System Semantics, 
 *	Version 1, Revision 1.09, 1992
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
#include <sys/utsname.h>
#include <sys/vfs.h>

#include "cdr_xcdr.h"
#include "cdr_rrg.h"
#include "cdr_cdrfs.h"
#include "cdr_cdrnode.h"

BUGXDEF(buglevel)

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
 *	cdrlookup_rrg
 *
 * function: called by cdr_lookup() for RRG specific directory lookup
 *
 * in: dcdrp - directory to search
 *     nmp - pathname component to search
 *
 * out: rc - == 0
 *		cdrpp - cdrnode for the pathname component
 *	   - != 0 lookup failure
 *
 * note: NM SUF for ISO 9660 directory entries with names 
 *       0x00 ("." entry) and 0x01 (".." entry) are ignored.
 *
 * 	 moved directory:
 *	 CL SUF - set i_number from CL SUF
 *       PL SUF - set i_number from PL SUF (".." entry only)
 *	 RE SUF - ignore the directory entry
 *
 * serialization: This function must be called with the cdrfs lock held.
 *		  it will be released before any calls outside the CDRFS
 *		  and across code which may page fault.
 */
int
cdrlookup_rrg (dcdrp, nmp, cdrpp)
	struct cdrnode *	dcdrp;	/* cdrnode for directory	*/
	dname_t *		nmp;	/* pathanme component to search	*/
	struct cdrnode **	cdrpp;	/* cdrnode for pathname component */
{
	int	rc = 0;				/* return code */
	struct	cdrfsmount	*cdrfsp;	/* mounted file system data */
	char 			*dstart = NULL;	/* mapped directory start */
	char 			*dend;		/* mapped directory end */
	daddr_t			pstart;		/* physical directory start */
	struct cdrdirent 	*de;		/* directory record cursor */
	int			dxoffset;	/* directory record offset in extent */
	int			xar_len;	/* xar length */
	daddr_t			dirent;		/* physical directory entry */
	int			found = FALSE;	/* entry name found ? */
	int			dir = FALSE;	/* entry is directory ? */
	uchar			rr;		/* RRIP SUFs recorded */
	mode_t			mode;
	dname_t			dname;
	char			dnamebuf[MAXNAMLEN + 1];
	daddr_t			moved_dir;	/* moved directory extent */
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

	/*
	 * Release the cdrfs lock if we have it. we may page fault.
	 */
	CDRFS_UNLOCK();

	/* special case for ".." entry */
	if (nmp->nmlen == 2 &&
	    *nmp->nm == '.' && *(nmp->nm + 1) == '.') {
		/* locate '..' entry */
		de = (struct cdrdirent *)((char *)de + de->d_drec_len);
		/* get the real parent directory address from PL SUF
		 * for moved directory
		 */
		if ((rc = cdrgetname(cdrfsp, de, &rr, &mode, NULL, &moved_dir)) != 0)
			goto done;
		/* directory: dirent = directory address */
		if (rr & RRIP_RR_PL)
			/* moved directory from PL SUF */
			dirent = LOFFSET(cdrfsp, moved_dir + de->d_xar_len, 0);	
		else
			dirent = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
		dir = TRUE;	
		found = TRUE;
		goto done;
	}

	/* scan the directory for name match */
	while ((caddr_t)de < dend) {
		/* test for paddings of 0x00 to the end of logical sector */
		if (de->d_drec_len < CDR_DIRENT_MIN) {
			/* advance pointer to next logical sector */
			dxoffset = NEXTLSCTR(dxoffset);
			de = (struct cdrdirent *)(dstart +  dxoffset -
							    xar_len);
			continue;
		}

		/* compare the directory entry with the pathname component
		 */
		dname.nm = dnamebuf;
		dname.nmlen = 0;
		if ((rc = cdrgetname(cdrfsp, de, &rr, &mode, &dname, &moved_dir)) != 0)
			goto done;

		/* skip if relocated directory entry (RE SUF specified) or 
		 * name does not match
		 */
		if (rr & RRIP_RR_RE) 
			goto next_entry;
		else if (nmp->nmlen != dname.nmlen ||
			strncmp(nmp->nm, dname.nm, dname.nmlen) != 0)
			goto next_entry;
					
		/* directory entry with pathname component found:
		 * determine dirent for the entry
		 */
		if ((mode & IFMT) == IFDIR) {
			/* directory: dirent = directory address */
			dirent = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
			dir = TRUE;
		} else if (rr & RRIP_RR_CL) {
			/* moved directory from CL SUF */
			dirent = LOFFSET(cdrfsp, moved_dir + de->d_xar_len, 0);	
			dir = TRUE;
		} else
			/* non-directory: dirent = directory entry address 
			 * in parent directory 
			 */
			dirent = (daddr_t)(pstart + ((char *)de - dstart));
		found = TRUE;
		goto done;

next_entry:
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
 *	cdrgetname
 *
 * function: called by directory lookup/read
 *	     to retrieve SUFs
 *
 * out: rc == 0 
 *	   != 0 format/system error 
 *
 * special case: moved directory
 *		 CL SUF - set i_number from CL SUF
 *		 RE SUF - ignore the directory entry
 *
 * note: cdrgetname() is not to be called for names 0x00 ("." entry) 
 * and 0x01 (".." entry).
 */
static
int
cdrgetname(cdrfsp, de, rr, mode, nmp, moved_dir)
	struct cdrfsmount	*cdrfsp;	/* mounted file system data */
	struct cdrdirent	*de;		/* directory record */
	uchar	*rr;				/* RRIP fields recorded */
	mode_t	*mode;				/* mode */
	dname_t	*nmp;				/* directory entry name */
	int	*moved_dir;			/* moved directory extent */
{
	int	rc = 0;
	uchar	*sua;
	int	sualen;
	ushort	signature;
	uchar	flags;
	struct rrg_suf		*suf;
	struct rrip_px_suf	*px_suf;
	struct rrip_nm_suf	*nm_suf;
	struct rrip_cl_suf	*cl_suf;
	struct rrip_pl_suf	*pl_suf;
	struct susp_ce_suf	*ce_suf;
	int	nm_continue = FALSE;
	char	*ce_found = NULL;
	caddr_t	lsbuffer = NULL;	/* malloc()ed Logical Sector buffer */

	/* determine system use area */
	sualen = de->d_drec_len - de->d_fileid_len - CDR_DIRENT_FIXED;
	if ((de->d_fileid_len & 0x01) == 0)
		sualen -= 1;
	if (sualen > 0) 
		sua = (char *)de + de->d_drec_len - sualen + 
		      cdrfsp->fs_rrg_sufoffset;
	else
		return EFORMAT; /* RRG must have PX SUF */

	*rr = 0;

ca_sua: /* loop for continuation area extension for system use area */
	ce_found = NULL;

	while (sualen > 0 && *sua != 0x00) { /* watch for pad for sua */	
		suf = (struct rrg_suf *) sua; 

		/* process identifier SUF */
		signature = BYTE2SHORT(suf->signature);
		switch (signature) {
		/*
 		 *	RRIP_PX: POSIX file attributes
 		 */
		case RRIP_PX:
			px_suf = (struct rrip_px_suf *) suf;

			/* PX length: 36 */
			/* PX version: 1 */

			/* POSIX file mode: sys/stat.h st_mode */
			*mode = BYTE2INT(px_suf->file_mode) & 0177777;
			*rr |= RRIP_RR_PX;
			break;

		/*
		 *	RRIP_NM: alternate name
		 *
		 * note: if no NM SUF is recorded, 
		 * ISO 9660 file identifier is used. 
		 */
		case RRIP_NM: 
			/* take the first name only */
			if (nmp == NULL || *rr & RRIP_RR_NM)
				break;

			nm_suf = (struct rrip_nm_suf *) suf;
			flags = nm_suf->flags;
			if (nm_continue) {
				/* name continued from previous NM */
				nmp->nmlen += nm_suf->len - RRIP_SUF_SLVF;
				if (nmp->nmlen > MAXNAMLEN) {
					rc = EFORMAT;
					goto done;
				}
				strncpy(nmp->nm + nmp->nmlen,
					nm_suf->name, nm_suf->len - RRIP_SUF_SLVF);
				if (!(flags & RRIP_NM_CONTINUE))
					/* last NM */
					*rr |= RRIP_RR_NM;
			} else {
				/* a single NM SUF name cannot exceed MAX_NAME */
				/* first NM */
				if (flags & RRIP_NM_CONTINUE) {
					/* name continue to next NM */
					nmp->nmlen = nm_suf->len - RRIP_SUF_SLVF;
					strncpy(nmp->nm + nmp->nmlen,
						nm_suf->name, nmp->nmlen);
					nm_continue = TRUE;
				} else { /* only NM */
					if (nm_suf->len > RRIP_SUF_SLVF) {
						/* copy name */
						nmp->nmlen = nm_suf->len - RRIP_SUF_SLVF;
						strncpy(nmp->nm, nm_suf->name, nmp->nmlen);
					} else { /* special names */
						switch (flags) {
						case RRIP_NM_HOST:
							/* copy local host name: MAXHOSTNAMELEN = 32 */
							nmp->nmlen = strlen(utsname.nodename);
							strncpy(nmp->nm, utsname.nodename, nmp->nmlen);
							break;
						case RRIP_NM_CURRENT: /* not applicable for unix system */
						case RRIP_NM_PARENT: /* not applicable for unix system */
						default:
							nmp->nmlen = 0; /* force name match failure */
						}
					}
					*rr |= RRIP_RR_NM;
				}
			}
			if (*rr == (RRIP_RR_NM | RRIP_RR_CL))
				goto done; /* NM and optional CL */
			break;

		/*
		 *	RRIP_CL: child link 
		 *
 		 * note: CL is recorded in the directory record specifying
		 * the moved directory.
 		 * Attributes of the moved directory are retrieved from
 		 * the current (.) directory record of the moved directory
		 * specified by the CL SUF, and
		 * override the attributes of the current Directory
		 * Record.
		 * note: RRIP_PX file type for the entry is IFREG !!!
		 */
		case RRIP_CL:
			if (moved_dir == NULL)
				break;
			cl_suf = (struct rrip_cl_suf *) suf;
			*moved_dir = BYTE2INT(cl_suf->loc_cd);
			*rr |= RRIP_RR_CL;
			if (*rr & RRIP_RR_NM)
				goto done; /* NM and optional CL */
			break;

		/*
 		 *	RRIP_PL: parent link
 		 *
 		 * note: PL is recorded only in the parent (..) directory 
		 * record of the moved directory.
 		 * The directory location in the d_locext field specifies 
		 * the ISO 9660 forster parent directory, while
		 * the directory location in PL SUF specifies 
		 * the RRG parent directory.
 		 */
		case RRIP_PL:
			if (moved_dir == NULL)
				break;
			pl_suf = (struct rrip_pl_suf *) suf;
			*moved_dir = BYTE2INT(pl_suf->loc_pd);
			*rr |= RRIP_RR_PL;
			goto done; /* PL exclusive */
			break;

		/*
 		 *	RRIP_RE: relocated directory
 		 *
 		 * note: RE is recorded in the alias directory record in 
 		 * the foster parent directory of the moved directory.
		 * The directory entry with RE SUF is recorded for 
		 * ISO 9660 visibility only and ignored in RRG processing.
 		 */
		case RRIP_RE:
			/* ignore the entry */
			*rr |= RRIP_RR_RE;
			goto done;
			break;

		/*
		 *	SUSP_CE: continuation area (CA)
		 *
		 * note: Each CA consist of a single Logical Sector.
		 * The CA specified by the current CE SUF should be processed 
		 * after the current SUA or CA is processed.
		 */
		case SUSP_CE:
			ce_found = sua;
			break;
	
		default:
			break;
		} /* end switch(signature) */
		
		/* advance to next SUF */
		sua += suf->len;
		sualen -= suf->len;
	} /* end while */

	/* check for continuation area */
	if (ce_found) {
		ce_suf = (struct susp_ce_suf *) ce_found;
		if (lsbuffer == NULL) {
			if ((lsbuffer = malloc(CDR_LSCTRSIZE)) == NULL)
				return ENOMEM;
		}
		sua = lsbuffer + BYTE2INT(ce_suf->offset);
		sualen = BYTE2INT(ce_suf->len_cont);
		if (rc = cdrbread(cdrfsp->fs_fp, LOFFSET(cdrfsp, BYTE2INT(ce_suf->location), 0), CDR_LSCTRSIZE, lsbuffer))
			goto done;
		goto ca_sua;
	}
	
	if (!(*rr & RRIP_RR_NM) && nmp != NULL) {
		/* NM SUF not found - retrieve ISO 9660 file identifier.
		 * convert the ISO 9660 file identifier to lower case and 
		 * suppress version number.
		 */
		char	*fid;
		char	*nm;
		char	c;
		int	nmlen;

		fid = de->d_file_id;
		nm = nmp->nm;
		nmlen = de->d_fileid_len;
		while (nmlen && (c = *fid) != ';') {
			*nm = ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c);
			fid++, nm++, nmlen--;
		}
		nmp->nmlen = de->d_fileid_len - nmlen;
	}
	
done:
	if (lsbuffer)
		free(lsbuffer);
	return rc;
}


/*
 *	cdrread_rrg
 *
 * function: called by cdrget() to finalize RRG cdrnode 
 *
 * in/out: cdrp - cdrnode to finalize from on-disk directory entry 
 *	      (refer to cdrget() for initialization of the cdrnode)
 *
 * note: the directory to be mapped and the directory entry to read
 *	 for non-directory file (when cdrp->pdirent != 0):
 *		map parent directory (specified by cdrp->pdirent) and 
 *		read directory entry (specified by cdrp->dirent)
 *	 for directory file (when cdrp->pdirent == 0): 
 *		map directory (specified by cdrp->dirent) and
 *		read '.' directory entry
 *
 * serialization: This function must be called with the cdrfs lock held.
 *		  it will be released before any calls outside the CDRFS
 *		  and across code which may page fault.
 */
int
cdrread_rrg (cdrp)
	struct cdrnode *	cdrp;
{
	int		rc = 0;		/* return code		*/
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	struct cdrnode 	*dcdrp;		/* directory cdrnode */
	daddr_t		dirent;		/* physical directory entry */
	daddr_t		pdirent;	/* physical parent directory */
	char 		*dstart = NULL;	/* mapped directory start */
	char 		*dend;		/* mapped directory end */
	daddr_t		pstart;		/* physical directory start */
	struct cdrdirent 	*de;	/* directory record cursor */
	int		doffset;	/* directory record offset in file section */
	int		dxoffset;	/* directory record offset in extent */
	struct cdrxd	*xd;		/* extent descriptor */
	int		xdn = 0; 
	struct cdrxd	*xdtail = NULL;
	struct cdrxar 	*xar = NULL;	/* extended attribute record */
	int		xar_len;	/* xar length */
	uchar		rr;		/* RRIP SUFs recorded */
	mode_t		mode;
	daddr_t		moved_dir; /* moved directory extent */
	label_t		jmpbuf;	/* exception return jump buffer	*/
	cdrfsp = cdrp->cn_cdrfs;

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
	pstart = dcdrp->cn_xd[0].cx_locdata << LBLKSHIFT(cdrfsp);

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

	/*
	 * Release the cdrfs lock if we have it. we may page fault.
	 */
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
			doffset = dxoffset - xar_len;
			de = (struct cdrdirent *)(dstart +  doffset);
			continue;
		}

		/* single volume file system only: 
	 	 * check for d_volseqno == pvd_volseqno is suppressed 
		 * to make anti-social disk right.
		 */

		/*  multi-extent for regular file only */
		if (de->d_file_flags & CD_MULTIEXT) {
			if (rc = cdrgetname(cdrfsp, de, &rr, &mode, NULL, NULL))
				goto done;
			if ((mode & IFMT) != IFREG) {
				rc = EFORMAT;
				goto done;
			}
		} else /* only/final extent */
			break;

		/* check file type of interleaved mode */
		if (de->d_file_usize || de->d_ileav_gsize) 
			cdrp->cn_format = CD_NTRLVD;

		/* create and fill in extent descriptor */
		if (rc = cdrmakexd(cdrp, de, &xdn, &xdtail))
				goto done;

		/* update file size in cdrnode */
		cdrp->cn_size += SHORT2INT(de->d_data_len);

		/* advance pointer next directory entry */
		doffset += de->d_drec_len;
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
	if (de->d_file_usize || de->d_ileav_gsize) 
		cdrp->cn_format = CD_NTRLVD;

	/* read in RRG System Use Area POSIX semantics */
	if (rc = cdrgetattr(cdrp, de))
		goto done;
		
	/* update file size and number of blocks used in cdrnode:
	 * special files: i_size == 0 
	 * symbolic link files: i_size set by cdrgetattr()
	 */
	mode = cdrp->cn_mode & IFMT;
	if (mode == IFREG || mode == IFDIR) {
		cdrp->cn_size += SHORT2INT(de->d_data_len);
		cdrp->cn_nblocks = 1 + LBLKNO(cdrfsp, cdrp->cn_size - 1);

		/* create and fill in extent descriptor */
		if (rc = cdrmakexd(cdrp, de, &xdn, &xdtail))
			goto done;

		/* set 'i_number' of the cdrnode - file object address */
		cdrp->cn_number = cdrp->cn_xd[0].cx_locdata << LBLKSHIFT(cdrfsp); 
	} else /* symlink, special file - directory entry address in parent directory */
		/* set 'i_number' of the cdrnode */
		cdrp->cn_number = pstart + doffset;

	/* determine time stamps if TF SUF does not exist:
	 * if XAR does not exist
	 *   st_ctime = st_mtime = st_atime = 
	 *              Directory Record Recording Date and Time
	 * if XAR exist
	 *   st_ctime = XAR File Creation Date and Time 
	 *   st_mtime = XAR File Modification Date and Time 
	 *   st_atime = Directory Record Recording Date and Time
	 */
	if (cdrp->cn_atime == 0) { /* TF SUF does not exist */
		cdrp->cn_atime = cdrgettime(&de->d_rec_time);

		if (de->d_xar_len == 0) {
			/* XAR does not exist */
			cdrp->cn_ctime = cdrp->cn_atime;
			cdrp->cn_mtime = cdrp->cn_atime;
		} else {
			/* read time stamp from XAR */
			if ((xar == NULL) &&
		    	(xar = (struct cdrxar *)malloc(CDR_PBLKSIZE)) == NULL) {
				rc = ENOMEM;
				goto done;
			}
			if (rc = cdrbread(cdrfsp->fs_fp, LOFFSET(cdrfsp, SHORT2INT(de->d_locext), 0), 
					CDR_PBLKSIZE, xar))
				goto done; 
					
			cdrp->cn_ctime = cdrgettime_long(&xar->xar_cre_time);
			cdrp->cn_mtime = cdrgettime_long(&xar->xar_mod_time);
		}
	}

	/* for directory cdrnode, set its parent's directory address 
	 * from the (..) entry of the child directory
	 */
	if (!pdirent) {
		/* advance to '..' entry */
		de = (struct cdrdirent *)
				((char *)de + de->d_drec_len);
		/* check for PL SUF for moved directory */
		cdrgetname(cdrfsp, de, &rr, &mode, NULL, &moved_dir);
		if (rr & RRIP_RR_PL)
			cdrp->cn_pdirent =
			LOFFSET(cdrfsp, moved_dir + de->d_xar_len, 0);
		else
			cdrp->cn_pdirent =
			LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
	}

done:
	/* unmap the segment, clear exception return */
	if (dstart)
		cdrpundo(dstart);
	cdrfs_clrjmpx(&jmpbuf);

	/* Free extended attribute record, if it was allocated. */
	if (xar != NULL)
		free(xar);

	if (pdirent)
		cdrput(dcdrp);	/* release parent cdrnode */

	return rc;
}


/*
 *	cdrgetattr
 *
 * function: retrieve attribute SUFs
 *
 * out: rc == 0 
 *	   != 0 format/system error 
 */
static
int
cdrgetattr(cdrp, de)
	struct cdrnode	*cdrp;
	struct cdrdirent	*de;
{
	int	rc = 0;
	struct cdrfsmount	*cdrfsp;
	uchar	*sua;
	int	sualen;
	ushort	signature;
	uchar	flags;
	struct rrg_suf		*suf;
	struct rrip_px_suf	*px_suf;
	struct rrip_pn_suf	*pn_suf;
	struct rrip_tf_suf	*tf_suf;
	struct rrip_sl_suf	*sl_suf;
	struct susp_ce_suf	*ce_suf;
	char	*ce_found = NULL;
	uchar	*time_stamps;
	int	sl_found = FALSE;
	int	sl_continue = FALSE;
	int	cr_continue = TRUE;
	char	*slca;
	int	slcalen;
	char	*symfile;
	char	dnamebuf[MAXPATHLEN];
	dname_t	dname = {dnamebuf, 0};
	caddr_t	lsbuffer = NULL; /* malloc()ed Logical Sector buffer */

	cdrfsp = cdrp->cn_cdrfs;

	/* determine system use area */
	sualen = de->d_drec_len - de->d_fileid_len - CDR_DIRENT_FIXED;
	if ((de->d_fileid_len & 0x01) == 0)
		sualen -= 1;
	if (sualen > 0) 
		sua = (char *)de + de->d_drec_len - sualen + 
		      cdrfsp->fs_rrg_sufoffset;
	else
		return EFORMAT;

ca_sua: /* loop for continuation area extension for system use area */
	ce_found = NULL;

	while (sualen > 0 && *sua != 0x00) { /* watch for pad for sua */	
		suf = (struct rrg_suf *) sua; 

		/* process attribute SUFs */
		signature = BYTE2SHORT(suf->signature);
		switch (signature) {
		/*
 		 *	RRIP_PX: POSIX file attributes
 		 */
		case RRIP_PX:
			px_suf = (struct rrip_px_suf *) suf;

			/* PX length: 36 */
			/* PX version: 1 */

			/* POSIX file mode: sys/stat.h st_mode */
			cdrp->cn_mode = BYTE2INT(px_suf->file_mode) & 0177777;
	
			/* POSIX file links: sys/stat.h st_nlink */
			cdrp->cn_nlink = BYTE2INT(px_suf->links);

			/* POSIX file user id: sys/stat.h st_uid */
			cdrp->cn_uid = BYTE2INT(px_suf->user_id);

			/* POSIX file group id: sys/stat.h st_gid */
			cdrp->cn_gid = BYTE2INT(px_suf->group_id);

			break;

		/* 
 		 *	RRIP_PN: POSIX device modes
		 *
		 * note: if the receiving system records device numbers
		 * as 32-bit numbers, the dev_t high is set to zero and
		 * ignored.
 		 */
		case RRIP_PN:
			pn_suf = (struct rrip_pn_suf *) suf;

			/* PX file mode of the directory entry must be
		 	 * a block or character device 
		 	 */

			/* PN length: 20 */
			/* PN version: 1 */
			/* dev_t high: ignored by system with ulong dev_t */

			/* dev_t low */
			cdrp->cn_rdev = BYTE2INT(pn_suf->dev_t_low);
			break;

		/*
 		 *	RRIP_TF: time stamp(s) for a file
		 *
		 * note: the existence and order of time stamp(s) are
		 * specified by the value and order of corresponding flag bits.  
 		 */
		case RRIP_TF:
			tf_suf = (struct rrip_tf_suf *) suf;

			/* TF length: */
			/* TF version: 1 */

			/* flags */
			flags = tf_suf->flags;

			/* scan time stamps in the order of 
			 * corresponding flag bit order
			 */
			time_stamps = tf_suf->time_stamps;
			if (!(flags & RRIP_TF_LONG_FORM)) {
				/* short form time stamp */
				if (flags & RRIP_TF_CREATION) {
					cdrp->cn_ctime = cdrgettime(time_stamps);
					time_stamps += sizeof(struct cdrtime);
				} 
				if (flags & RRIP_TF_MODIFY) {
					cdrp->cn_mtime = cdrgettime(time_stamps);
					time_stamps += sizeof(struct cdrtime);
				} 
				if (flags & RRIP_TF_ACCESS) {
					cdrp->cn_atime = cdrgettime(time_stamps);
					time_stamps += sizeof(struct cdrtime);
				}
			} else { /* long form time stamp */
				if (flags & RRIP_TF_CREATION) {
					cdrp->cn_ctime = cdrgettime_long(time_stamps);
					time_stamps += sizeof(struct cdrtime_long);
				} 
				if (flags & RRIP_TF_MODIFY) {
					cdrp->cn_mtime = cdrgettime_long(time_stamps);
					time_stamps += sizeof(struct cdrtime_long);
				} 
				if (flags & RRIP_TF_ACCESS) {
					cdrp->cn_atime = cdrgettime_long(time_stamps);
					time_stamps += sizeof(struct cdrtime_long);
				}
			}
			break;

		/*
 		 *	RRIP_SL: symbolic link
		 *
		 * SL SUF component area consists of a set of contiguous
		 * component records, where 
		 * each pathname component is recorded as one or more
		 * component records.
		 *
		 * component continuation only detremines whether to 
		 * append '/' or not at the end of processing of 
		 * the component record (only the last compont record 
		 * in SL may be continued to the first component record
		 * of the next SL).
		 *
		 * for 'fast' symbolic link, the symbolic link is 
		 * stored in the cdrnode or malloc()ed buffer.
 		 */
		case RRIP_SL:
			/* take the first symlink only */
			if (sl_found)
				break;

			sl_suf = (struct rrip_sl_suf *) suf;

			/* PX file mode of the directory entry must be
 	 		* a symbolic link
 	 		*/

			/* SL version: 1 */

			/* SL flags */
			flags = sl_suf->flags;

			/* component area: component records */
			slca = sl_suf->ca;
			slcalen = sl_suf->len - RRIP_SUF_SLVF; 

			if (sl_continue) {
				/* SL continued from previous SL */
				if (rc = rrip_sl_cr(cdrp, slca, slcalen, &dname, &cr_continue))
					goto done;
				if (!(flags & RRIP_SL_CONTINUE))
					/* last SL */
					sl_continue = FALSE;
			} else {
				/* first SL */
				if (flags & RRIP_SL_CONTINUE) {
					/* name continue to next SL */
					if (rc = rrip_sl_cr(cdrp, slca, slcalen, &dname, &cr_continue))
						goto done;
					sl_continue = TRUE;
				} else { /* only SL */
					if (rc = rrip_sl_cr(cdrp, slca, slcalen, &dname, &cr_continue))
						goto done;
					sl_continue = FALSE;
				}
			}

			if (sl_continue)
				break;

			if (dname.nmlen <= CN_PRIVATE) {
				/* cache symlink in cdrnode */
				strncpy(cdrp->cn_symlink, dname.nm, dname.nmlen);
			} else {
				/* cache symlink in malloc()ed buffer */
				symfile = (char *)malloc(dname.nmlen);
				if (symfile == NULL)
					return ENOMEM;
				strncpy(symfile, dname.nm, dname.nmlen);
				cdrp->cn_symfile = symfile;
			}

			cdrp->cn_size = dname.nmlen;
			/* symlink storage belongs to parent directory */
			cdrp->cn_nblocks = 0;
			sl_found = TRUE;
			break;

		/* 
 		 *	SUSP_CE: continuation area (CA) 
		 *
		 * note: Each CA consist of a single Logical Sector.
		 * The CA specified by the current CE SUF should be processed 
		 * after the current SUA or CA is processed.
 		 */
		case SUSP_CE:
			ce_found = sua;
			break;


		default:
			break;
		} /* end switch(signature) */
		
		sua += suf->len;
		sualen -= suf->len;
	} /* end while */

	/*
	 * check for continuation area
	 */
	if (ce_found) {
		ce_suf = (struct susp_ce_suf *) ce_found;
		if (lsbuffer == NULL) {
			if ((lsbuffer = malloc(CDR_LSCTRSIZE)) == NULL)
				return ENOMEM;
		}
		sua = lsbuffer + BYTE2INT(ce_suf->offset);
		sualen = BYTE2INT(ce_suf->len_cont);
		if (rc = cdrbread(cdrfsp->fs_fp, LOFFSET(cdrfsp, BYTE2INT(ce_suf->location), 0), CDR_LSCTRSIZE, lsbuffer))
			goto done; 
		goto ca_sua;
	}

done:
	/* ensure to free symlink buffer if failure occurred */
	if (rc) { 
		if (cdrp->cn_size > CN_PRIVATE)
			free(cdrp->cn_symfile);
		cdrp->cn_size = 0;
	}

	if (lsbuffer)
		free(lsbuffer);

	return rc;
}

/*
 *	rrip_sl_cr - combine symbolic link component records
 *		in a single SL component record area
 *
 * note: only the last/only component record may be continued
 * into the next SL
 */
static
int
rrip_sl_cr(cdrp, slca, slcalen, nmp, cr_continue)
	struct	cdrnode	*cdrp;
	char	*slca;		/* sl component area */
	int	slcalen;	/* sl ca length */
	dname_t	*nmp;
	int	*cr_continue;	/* is current cr a continuation ? */
{
	struct rrip_sl_cr_suf	*slcr; /* sl component record */
	uchar	crflags;
	int	dnl;
	char	*dn;
	int	nmlen;

	dnl = nmp->nmlen;
	dn = nmp->nm + dnl;

	/* concatenate component records */
	while (slcalen > 0) {
		slcr = (struct rrip_sl_cr_suf *)slca;
		crflags = slcr->flags;

		/* append separator before current component */
		if (!(*cr_continue)) {
			if (dnl + 1 >= MAXPATHLEN)
				return EFORMAT;
			if (dnl != 1 || *(dn - 1) != '/') {
				*dn++ = '/';
				dnl++;
			}
		}

		/* append component name */
		if ((nmlen = slcr->len) != 0) { /* full name */
			if (dnl + nmlen > MAXPATHLEN)
				return EFORMAT;
			strncpy(dn, slcr->component, nmlen);
		} else { /* abbreviated name */
			if (crflags & RRIP_SL_CR_CURRENT) { 
				nmlen = 1;
				if (dnl + nmlen > MAXPATHLEN)
					return EFORMAT;
				*dn = '.';
			} else if (crflags & RRIP_SL_CR_PARENT)  {
				nmlen = 2;
				if (dnl + nmlen > MAXPATHLEN)
					return EFORMAT;
				*dn = '.';
				*(dn + 1) = '.';
			} else if (crflags & RRIP_SL_CR_ROOT) {
				nmlen = 1;
				if (dnl + nmlen > MAXPATHLEN)
					return EFORMAT;
				*dn = '/';
			} else if (crflags & RRIP_SL_CR_VOLROOT) {
				/* copy name of the covered node of this CDRFS
				 * (for device-on-directory mount only)
				 */
				struct cdrfsmount	*cdrfsp;
				struct vfs	*vfsp;
				struct vmount	*vmountp;

				cdrfsp = cdrp->cn_cdrfs;
				vfsp = cdrfsp->fs_vfs;
				vmountp = vfsp->vfs_mdata;
				nmlen = vmt2datasize(vmountp, VMT_STUB);
				if (dnl + nmlen > MAXPATHLEN)
					return EFORMAT;
				strncpy(dn, vmt2dataptr(vmountp, VMT_STUB), nmlen);
			} else if (crflags & RRIP_SL_CR_HOST) {
				/* copy local host name */
				nmlen = strlen(utsname.nodename);
				if (dnl + nmlen > MAXPATHLEN)
					return EFORMAT;
				strncpy(dn, utsname.nodename, nmlen);
			}
		}

		dnl += nmlen;
		dn += nmlen;

		if (crflags & RRIP_SL_CR_CONTINUE) {
			*cr_continue = TRUE;
		} else
			*cr_continue = FALSE;

		slca += slcr->len + 2;
		slcalen -= slcr->len + 2;
	} /* end while */

	nmp->nmlen = dnl;
	return 0;
}


#define CDRDIRSIZ(namlen) \
    ((sizeof (struct dirent) - (MAXNAMLEN+1)) + ((namlen+1 + 3) & ~3))

/*
 *	cdrreaddir_rrg
 *
 * function: readdir() (read struct cdrdirent off disk and 
 *	     translate into struct dirents) for RRG directory
 *
 * note: NM SUF for ISO 9660 directory entries with names 
 *       0x00 ("." entry) and 0x01 (".." entry) are ignored.
 *
 * special case: CL SUF - read '.' entry of the child directory
 *               PL SUF - set i_number from PL SUF (".." entry only)
 *		 RE SUF - ignore the directory entry
 *
 * serialization: This function must be called with the cdrfs lock held.
 *		  it will be released before any calls outside the CDRFS
 *		  and across code which may page fault.
 */
int
cdrreaddir_rrg (cdrp, uiop)
	struct cdrnode *	cdrp;
	struct uio *		uiop;
{
	int		rc = 0;		/* return code			*/
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
	int		fidsize;	/* file identifier size */
	int		versize;	/* file identifier less version number size */
	struct dirent	*lasttdirp;	/* last dirent cursor */
	char *		lastfid;	/* last file identifier */
	int		lastfidlen;	/* length of last file identifier */
	int		dir;
	char		*s, c;
	int		sl;
	uchar		rr;		/* RRIP fields recorded */
	mode_t		mode;		
	char		dnamebuf[MAXNAMLEN + 1];
	dname_t		dname;
	daddr_t		moved_dir;	/* moved directory extent */
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
		     sizeof(struct dirent) * 
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

	/*
	 * Release the cdrfs lock if we have it. we may page fault.
	 */
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
			de = (struct cdrdirent *)(dstart + doffset); 
			continue;
		}

		/* skip entries before the starting offset */
		if (doffset < start)
			goto next_drecord;

		dname.nmlen = 0;
		dname.nm = dnamebuf;

		/* special case for "." and ".." entry */
		if (de->d_fileid_len == 1) {
			if (*de->d_file_id == 0x00) {
				/* "." entry */
				s = dname.nm;
				*s++ = '.';
				*s = '\0';
				dname.nmlen = 1;
				rr = 0;
				mode = IFDIR;
				moved_dir = 0;
				goto xlate_record;
			} else if (*de->d_file_id == 0x01) {
				/* ".." entry */
				s = dname.nm;
				*s++ = '.';
				*s++ = '.';
				*s = '\0';
				dname.nmlen = 2;
				/* check for PL SUF */
				cdrgetname(cdrfsp, de, &rr, &mode, NULL, &moved_dir);
				goto xlate_record;
			}
		}

		/* retrive name from identifier SUFs */
		if ((rc = cdrgetname(cdrfsp, de, &rr, &mode, &dname, &moved_dir)) == 0) {
			if ((rr & RRIP_RR_RE) || /* ignore the entry - RE SUF entry */
			    dname.nmlen == 0)
				goto next_drecord;
		} else
			goto out;

		/* skip subsequent multi-extent directory records
		 * of the previous file by skipping directory records with
		 * the same filename
		 */
		if (dname.nmlen == lastfidlen &&
		    strncmp(dname.nm, lastfid, lastfidlen) == 0) {
			/* update offset of next directory entry of
			 * the last dirent translated:
			 * set the next directory entry offset by
		 	 * skipping all the multi-extent directory
		 	 * records of the same filename
		 	 * so that next readdir() for the same directory
		 	 * will start with new directory entry/name
		 	 */
			lasttdirp->d_offset = doffset + de->d_drec_len;
			goto next_drecord;
		}

xlate_record: /* translate directory entry into struct dirent */

		/* check if this entry will fit */
		if ((ubytes + CDRDIRSIZ(dname.nmlen)) > tbytes)
			break;

		/* set the offset to the next directory entry */
		tdirp->d_offset = doffset + de->d_drec_len;

		/* set 'i_number' of the entry */
		mode = mode & IFMT;
		if (mode == IFDIR) {
			if (rr & RRIP_RR_PL)
				/* moved directory - moved directory file object address */
				tdirp->d_ino = LOFFSET(cdrfsp, moved_dir + de->d_xar_len, 0);
			else
				/* directory - directory file object address */
				tdirp->d_ino = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
		} else if (mode == IFREG) {
			if (rr & RRIP_RR_CL)
				/* moved directory - moved directory file object address */
				tdirp->d_ino = LOFFSET(cdrfsp, moved_dir + de->d_xar_len, 0);
			else
				/* file - file object address */
				tdirp->d_ino = LOFFSET(cdrfsp, SHORT2INT(de->d_locext) + de->d_xar_len, 0);
		} else /* symlink, special file - directory entry address in parent directory */
			tdirp->d_ino = (daddr_t)(pstart + doffset);

		/* set the name and name length for the entry */
		tdirp->d_namlen = dname.nmlen;
		bcopy(dname.nm, tdirp->d_name, dname.nmlen);
		tdirp->d_name[dname.nmlen] = '\0';

		/* set the struct dirent record length */
		tdirp->d_reclen = DIRSIZ(tdirp);

		lasttdirp = tdirp;
		lastfid = tdirp->d_name;
		lastfidlen = dname.nmlen;

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
