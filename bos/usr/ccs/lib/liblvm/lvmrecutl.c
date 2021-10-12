static char sccsid[] = "@(#)66	1.11  src/bos/usr/ccs/lib/liblvm/lvmrecutl.c, liblvm, bos411, 9439B411a 9/28/94 12:46:46";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - lvmrecutl.c
 *
 * FUNCTIONS: lvm_rdlvmrec,
 *            lvm_wrlvmrec,
 *            lvm_zerolvm,
 *            lvm_cmplvmrec
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <fcntl.h>;
#include <errno.h>;
#include <sys/ioctl.h>;
#include <sys/devinfo.h>;
#include <sys/time.h>;
#include <sys/bbdir.h>;
#include <sys/buf.h>;
#include <sys/shm.h>;
#include <sys/sysconfig.h>;
#include <sys/types.h>;
#include <sys/param.h>;
#include <sys/dasd.h>;
#include <sys/hd_config.h>;
#include <lvmrec.h>;
#include <liblvm.h>;
#include <sys/hd_psn.h>;









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_rdlvmrec                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads the LVM record from the disk.                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     lvm_rec                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_LVMRECERR                                                   *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_rdlvmrec (


int pv_fd,
  /* the file descriptor for the physical volume device */

struct lvm_rec * lvm_rec)
  /* a pointer to the buffer into which the LVM information record will
     be read */


{ /* BEGIN lvm_rdlvmrec */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


offset = DBSIZE * PSN_LVM_REC;
  /* find offset from the beginning of the hard file of the LVM
     information record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the LVM information
     record */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with code for programming error */

}

retcode = read (pv_fd, (char *) lvm_rec, sizeof (struct lvm_rec));
  /* read the LVM information record from the disk */

if (retcode != sizeof (struct lvm_rec))
  /* if an error occurred */

{

    offset = DBSIZE * PSN_LVM_BAK;
      /* find offset from the beginning of the hard file of the backup
	 LVM information record */

    offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
      /* position the file pointer at the beginning of the LVM
	 information record */

    if (offset == LVM_UNSUCC)
      /* if an error occurred */

    {

	return (LVM_PROGERR);
	  /* return with code for programming error */

    }

    retcode = read (pv_fd, (char *) lvm_rec, sizeof (struct lvm_rec));
      /* read the backup LVM information record from the disk */

    if (retcode != sizeof (struct lvm_rec))
      /* if an error occurred */

    {

	return (LVM_LVMRECERR);
	  /* return with error code for error reading or writing the LVM
	     information record */

    }

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_rdlvmrec */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_wrlvmrec                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes the LVM record onto the disk.                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     lvm_rec                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *     LVM_LVMRECERR                                                   *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_wrlvmrec (


int pv_fd,
  /* the file descriptor for the physical volume device */

struct lvm_rec * lvm_rec)
  /* a pointer to the buffer which contains the LVM information record
     to be written */


{ /* BEGIN lvm_wrlvmrec */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


offset = DBSIZE * PSN_LVM_REC;
  /* calculate offset in bytes from beginning of the disk for the LVM
     information record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the LVM information
     record */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with code for programming error */

}

retcode = writex (pv_fd, (char *) lvm_rec, sizeof (struct lvm_rec),
		  WRITEV);
  /* write the LVM information record to the disk */

if (retcode != sizeof (struct lvm_rec))
  /* if an error occurred */

{

    return (LVM_LVMRECERR);
      /* return with error code for error reading or writing the LVM
	 information record */

}

offset = DBSIZE * PSN_LVM_BAK;
  /* calculate offset in bytes from beginning of the disk for the backup
     LVM information record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the backup LVM
     information record */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with code for programming error */

}

retcode = writex (pv_fd, (char *) lvm_rec, sizeof (struct lvm_rec),
		  WRITEV);
  /* write the backup LVM information record to the disk */

if (retcode != sizeof (struct lvm_rec))
  /* if an error occurred */

{

    return (LVM_LVMRECERR);
      /* return with error code for error reading or writing the LVM
	 information record */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_wrlvmrec */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_zerolvm                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function zeroes out the LVM information record and the       *
 *   backup LVM record on the disk.                                    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The LVM record and the backup LVM record are written to the     *
 *     physical volume.                                                *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_zerolvm (


int pv_fd)
  /* the file descriptor for the physical volume device */


{ /* BEGIN lvm_zerolvm */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

char lvm_rec [DBSIZE];
  /* structure into which the LVM information record will be read */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


bzero (lvm_rec, DBSIZE);
  /* zero out the LVM information record */

offset = DBSIZE * PSN_LVM_REC;
  /* calculate offset in bytes from beginning of the disk for the LVM
     information record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the LVM information
     record */

if (offset != LVM_UNSUCC)
  /* if successful seek */

{

    retcode = writex (pv_fd, lvm_rec, DBSIZE, WRITEV);
      /* write the LVM information record to the disk */

}

offset = DBSIZE * PSN_LVM_BAK;
  /* calculate offset in bytes from beginning of the disk for the backup
     LVM information record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the backup LVM
     information record */

if (offset != LVM_UNSUCC)
  /* if successful seek */

{

    retcode = writex (pv_fd, lvm_rec, DBSIZE, WRITEV);
      /* write the backup LVM information record to the disk */

}

return;
  /* return to caller */

} /* END lvm_zerolvm */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_cmplvmrec                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function compares the vgid passed in with the vgid stored    *
 *   in the LVM record on the disk.                                    *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vgid                                                            *
 *     match                                                           *
 *     pvname                                                          *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   LVM_PVOPNERR                                                      *
 *   LVM_LVMRECERR                                                     *
 *   LVM_SUCCESS                                                       *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_cmplvmrec (


struct unique_id *vgid, 	/* Pointer to the volume group id to be */
				/* compared with the lvm record */
char *match,			/* indicates that the vgid matches */
char pvname[LVM_NAMESIZ])	/* name of physical volume to open */

{
	struct lvm_rec lvmrec;	/* holds lvm record info */
	int 	       pvfd;	/* physical volume file descriptor */
	int 	       rc;	/* return code */
	
	
        /* initialize the match indicator to FALSE */
	
	*match = FALSE;

	/* open the physical volume */
	
	pvfd = open(pvname, O_RDONLY);
	if(pvfd == LVM_UNSUCC)
		return;

	/* call lvm_rdlvmrec() to read in the lvm record */ 

	rc = lvm_rdlvmrec(pvfd,&lvmrec);
	if(rc < LVM_SUCCESS) {
		close(pvfd);
		return;
	}

	/* compare the vgids and set an indicator to true if they match */

	if(lvmrec.vg_id.word1 == vgid->word1 &&
             lvmrec.vg_id.word2 == vgid->word2)
		*match = TRUE; 
	else
		*match = FALSE;

	/* close the physical volume */

	close(pvfd);

	return;
}	



/***********************************************************************
 *                                                                     *
 * NAME:  lvm_updversion                                               *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function takes a vgptr and will open all the pvs associated  *
 *   with the vg, and write out a new lvm_rec (with updated version)   *
 *   on the disks it can access                                        *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vgptr                                                           *
 *     version                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   none                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_updversion (
char *vgptr, 
short version)
{
	int rc;				/* return code */
	int cnt;			/* counter for loops */
	struct pv_header *pv;		/* physical volume header pointer */
	struct fheader   *fhead;	/* file header pointer */
	struct lvm_rec   lvm_rec;	/* lvm record */
	char pv_id[LVM_NAMESIZ];	/* physical volume id */
	char pvname[LVM_NAMESIZ];	/* physical volume name */

        get_ptrs(vgptr,&fhead,NULL,NULL,NULL,NULL,NULL);

	for(cnt=0; cnt<LVM_MAXPVS; cnt++) {

		/* if there is no entry at that location, skip to next */

		if(fhead->pvinfo[cnt].pvinx != 0) {
			pv = (struct pv_header *) ((char *)vgptr +
				fhead->pvinfo[cnt].pvinx);

			sprintf(pv_id,"%08x%08x%08x%08x", 
				pv->pv_id.word1,
				pv->pv_id.word2,
				pv->pv_id.word3,
				pv->pv_id.word4);
   			get_odm_pvname(pv_id, &pvname);

			/* only attempt a write of the LVM-record to the
			 * disks available (ie. not missing) and can be
			 * accessed.
			 */
			if (getlvmrec(pvname, &lvm_rec) == LVM_SUCCESS) {
				if (lvm_rec.version != version) {
					lvm_rec.version = version;
					putlvmrec(pvname, &lvm_rec); 
				}
			} 
		}
	}
}	

/***********************************************************************
 *                                                                     *
 * NAME:  lvm_getversion                                               *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function takes a file header ptr and will open all pvs in a  *
 *   volume group, and return the maximum version field, if no pv      *
 *   (ie. first disk) it will return LVM_VERSION_1                     *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vgptr                                                           *
 *     version                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   LVM_VERSION_1                                                     *
 *   version number in lvmrec                                          *
 *                                                                     *
 ***********************************************************************
 */


short
lvm_getversion (
struct fheader *fhead) 
{
	int rc;				/* return code */
	int cnt;			/* counter for loops */
	struct unique_id *pv;		/* physical volume header pointer */
	struct lvm_rec   lvm_rec;	/* lvm record */
	char pv_id[LVM_NAMESIZ];	/* physical volume id */
	char pvname[LVM_NAMESIZ];	/* physical volume name */
	short max_version;		/* max version number found on all pv */

	max_version = 0;
	for(cnt=0; cnt<LVM_MAXPVS; cnt++) {

		/* if there is no entry at that location, skip to next */

		if(fhead->pvinfo[cnt].pvinx != 0) {
			pv = &(fhead->pvinfo[cnt].pv_id);
			sprintf(pv_id,"%08x%08x%08x%08x", 
				pv->word1,
				pv->word2,
				pv->word3,
				pv->word4);
   			get_odm_pvname(pv_id, &pvname);

			if ((rc = getlvmrec(pvname, &lvm_rec)) == LVM_SUCCESS) 
				if (lvm_rec.version > max_version)
					max_version = lvm_rec.version;
		}
	}
	return ((max_version <= 0 || max_version > LVM_MAX_VERSION) ? LVM_VERSION_1 : max_version);
}	
