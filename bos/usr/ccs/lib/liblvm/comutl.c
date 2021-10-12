static char sccsid[] = "@(#)64	1.20.1.4  src/bos/usr/ccs/lib/liblvm/comutl.c, liblvm, bos411, 9438C411a 9/23/94 16:51:22";
/*
 * COMPONENT_NAME: (LIBLVM) Logical Volume Manager library - comutl.c
 *
 * FUNCTIONS: lvm_chkvaryon,
 *            lvm_mapoff,
 *            lvm_rdiplrec,
 *            lvm_relocmwcc,
 *            lvm_tscomp,
 *            lvm_updtime
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990,
 *               1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************
 *   Include files                                                     *
 ***********************************************************************
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/time.h>
#include <sys/bbdir.h>
#include <sys/buf.h>
#include <sys/shm.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/cfgodm.h>
#include <odmi.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_config.h>
#include <liblvm.h>
#include <lvmrec.h>
#include <sys/hd_psn.h>









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_chkvaryon                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This function checks whether the volume group is varied on.       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     vg_id                                                           *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_OFFLINE                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_chkvaryon (

struct unique_id * vg_id)
  /* the id of the volume group */


{ /* BEGIN lvm_chkvaryon */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

struct queryvgs *queryvgs;	  
  /* structure to hold query info of all on-line volume groups in system */

mid_t kmid;	
  /* kernel module id */

char online;
  /* indicator that the vg is on-line */

int cnt;
  /* for loop counter */

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


online = FALSE;
  /* get the kmid from the data base */

kmid = (mid_t) loadext(LVM_LVDDNAME,FALSE,TRUE);

if(kmid == NULL) {
	return(LVM_PROGERR);
}

retcode = lvm_queryvgs(&queryvgs,kmid);
  /* call lvm_queryvgs to see which vgs are on-line */
if(retcode < LVM_SUCCESS)
	return(retcode);

/*
*  loop for the number of vgs that are on-line 
*  and try to find one with the same vgid as the one passed in
*/

for(cnt=0; cnt < queryvgs->num_vgs; cnt++) {
	if(queryvgs->vgs[cnt].vg_id.word1 == vg_id->word1 &&
       	   queryvgs->vgs[cnt].vg_id.word2 == vg_id->word2) { 
		online = TRUE;
	        break;
	}
}

lvm_freebuf(queryvgs);
/*
*  if the vg in question is not on-line return an error
*/

if(online == FALSE)
   	return(LVM_OFFLINE);
else
	return(LVM_SUCCESS);
        
} /* END lvm_chkvaryon */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_mapoff                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine stores offsets from the beginning of the mapped      *
 *   file.                                                             *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     mapfilehdr                                                      *
 *     vgda_ptr                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     Information has been stored in the mapped file header.          *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   None                                                              *
 *                                                                     *
 ***********************************************************************
 */


void
lvm_mapoff (

struct fheader * mapfilehdr,
  /* a pointer to the mapped file header which contains the offsets of
     the different data areas within the mapped file */

caddr_t vgda_ptr)
  /* a pointer to the beginning of the volume group descriptor area */


{ /* BEGIN lvm_mapoff */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct vg_header * vghdr_ptr;
  /* a pointer to the volume group descriptor area header */

struct vg_trailer * vgtrail_ptr;
  /* a pointer to the volume group descriptor area trailer */

struct lv_entries * lvlist_ptr;
  /* a pointer to the list of logical volume entries in the volume group
     descriptor area */

struct pv_header * pv_ptr;
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */

char * namelist_ptr;
  /* a pointer to the area within the volume group descriptor area where
     the list of logical volume names is stored */

long size;
  /* variable to hold an interim size calculation */

long namelist_size;
  /* length in bytes of the list of logical volume names in the volume
     group descriptor area */

long lvlist_size;
  /* length in bytes of the list of logical volume entries in the volume
     group descriptor area */

long pventry_size;
  /* length in bytes for a particular physical volume entry in the list
     of physical volumes in the volume group descriptor area */

short int pv_number;
  /* loop index variable for physical volume number */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


vghdr_ptr = (struct vg_header *) vgda_ptr;
  /* set a pointer to the header of the volume group descriptor area */

mapfilehdr -> vginx = (caddr_t) vghdr_ptr - (caddr_t) vgda_ptr +
			sizeof (struct fheader);
  /* calculate the offset from the beginning of the mapped file to the
     volume group header of the descriptor area and store in the mapped
     file header */

vgtrail_ptr = (struct vg_trailer *) ((caddr_t) vgda_ptr +
		 (vghdr_ptr -> vgda_size) * DBSIZE - DBSIZE);
  /* set a pointer to the trailer of the volume group descriptor area,
     which begins one block from the end of the descriptor area */

mapfilehdr -> trailinx = (caddr_t) vgtrail_ptr - (caddr_t) vgda_ptr +
			   sizeof (struct fheader);
  /* calculate the offset from the beginning of the mapped file to the
     volume group trailer of the descriptor area and store in the mapped
     file header */

size = (vghdr_ptr -> maxlvs) * LVM_NAMESIZ;
  /* calculate the size needed for the device names for the maximum
     number of logical volumes allowed for this volume group */

namelist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size reserved in the descriptor area for the
     list of logical volume device names */

namelist_ptr = (char *) ((caddr_t) vgtrail_ptr - namelist_size);
  /* find the pointer to the logical volume name list area by subtracting
     the size of the list from the pointer to the trailer, since the
     name list immediately precedes the trailer */

mapfilehdr -> name_inx = (caddr_t) namelist_ptr - (caddr_t) vgda_ptr +
			   sizeof (struct fheader);
  /* calculate the offset from the beginning of the mapped file to the
     name list area of the descriptor area and store in the mapped file
     header */

lvlist_ptr = (struct lv_entries *) ((caddr_t) vgda_ptr + DBSIZE);
  /* set a pointer to the beginning of the list of logical volumes in the
     volume group descriptor area */

mapfilehdr -> lvinx = (caddr_t) lvlist_ptr - (caddr_t) vgda_ptr +
			sizeof (struct fheader);
  /* calculate the offset from the beginning of the mapped file to the
     list of logical volumes in the descriptor area and store in the
     mapped file header */

size = (vghdr_ptr -> maxlvs) * sizeof (struct lv_entries);
  /* calculate the size needed for the descriptor area list of logical
     volumes for the maximum number of logical volumes allowed for this
     volume group */

lvlist_size = LVM_SIZTOBBND (size);
  /* calculate the actual size to be reserved in the descriptor area
     for the list of logical volumes by rounding the size up to the
     nearest multiple of the blocksize of the physical volume */

pv_ptr = (struct pv_header *) (vgda_ptr + DBSIZE + lvlist_size);
  /* set the pointer to the first physical volume entry to point to the
     beginning of the list of physical volumes in the volume group
     descriptor area */

mapfilehdr -> pvinx = (caddr_t) pv_ptr - (caddr_t) vgda_ptr +
			sizeof (struct fheader);
  /* calculate the offset from the beginning of the mapped file to the
     list of physical volumes in the descriptor area and store in the
     mapped file header */

for (pv_number=1; pv_number <= vghdr_ptr->numpvs; pv_number=pv_number+1)
  /* loop for each physical volume in the list of physical volumes in the
     volume group descriptor area */

{

  mapfilehdr -> pvinfo [pv_ptr -> pv_num - 1].pvinx =
       (caddr_t) pv_ptr - (caddr_t) vgda_ptr + sizeof (struct fheader);
    /* store the offset from the beginning of the mapped file of the
       beginning of the physical volume list */

  mapfilehdr -> pvinfo [pv_ptr -> pv_num - 1].pv_id = pv_ptr -> pv_id;
    /* save the PV id in the element of the array represented by the PV
       number minus one in order to be able to associate PV id and PV
       number */

  size = sizeof (struct pv_header) + pv_ptr -> pp_count *
				       sizeof (struct pp_entries);
    /* calculate the size of the space needed to contain the header and
       the physical partition entries for this PV */

  pventry_size = LVM_SIZTOBBND (size);
    /* calculate the actual size reserved in the descriptor area for this
       PV entry by rounding the size up to the nearest multiple of the
       blocksize of the physical volume */

  pv_ptr = (struct pv_header *) ((caddr_t) pv_ptr + pventry_size);
    /* set pointer to point at the next PV entry in the list of physical
       volumes in the volume group descriptor area */

} /* loop for each PV in the VGDA */

mapfilehdr -> endpvs = (caddr_t) pv_ptr - (caddr_t) vgda_ptr +
			 sizeof (struct fheader);
  /* store the offset from the beginning of the mapped file of the
     end of the physical volume list (i.e., area where next physical
     volume will be added to the list) */

return;
  /* return to caller */

} /* END lvm_mapoff */


/***********************************************************************
 *                                                                     *
 * NAME:  lvm_rdiplrec                                                 *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads the IPL record from the disk.                  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     ipl_rec                                                         *
 *                                                                     *
 *   OUTPUT:                                                           *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine.              *
 *     LVM_SUCCESS                                                     *
 *     LVM_RDPVID                                                      *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_rdiplrec (

char *pvname,

int pv_fd,
  /* the file descriptor for the physical volume device */

IPL_REC_PTR ipl_rec)
  /* a pointer to the buffer into which the IPL record will be read */


{ /* BEGIN lvm_rdiplrec */

/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* the offset in bytes from the beginning of the physical volume where
     the file pointer is to be placed for the next read */

/* used when calling odm data */
char crit[256];
struct CuAt cuat;
struct CuDv cudv;

/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

offset = DBSIZE * PSN_IPL_REC;
  /* find offset from the beginning of the hard file of the IPL record */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position the file pointer at the beginning of the IPL record */

if (offset == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return with code for programming error */

}

retcode = read (pv_fd, (caddr_t) ipl_rec, sizeof (IPL_REC));
  /* read the IPL record from the disk */

if (retcode != sizeof (IPL_REC))
  /* if an error occurred */

{

    return (LVM_RDPVID);
      /* return error code to indicate we could not obtain a valid PV id
	 from this disk */

}

/*
   set media's pvid equal to enclosure's pvid
   search the CuDv odm for the name of the device and compare the PdDvLn
   to make sure it's a rwoptical (since the devices don't need to be called
   omd0..n

*/
odm_initialize();
odm_set_path(CFGPATH);

sprintf(crit,"name='%s'", pvname);
if ((int)odm_get_first(CuDv_CLASS,crit,&cudv) > 0)
{
    if (strncmp(cudv.PdDvLn_Lvalue,"rwoptical",9) == 0) {
    	sprintf(crit,"name='%s' and attribute='pvid'", pvname);
    	if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
    	{
         	sscanf(cuat.value, "%08x%08x%08x%08x", 
		&(ipl_rec->pv_id.word1),
                &(ipl_rec->pv_id.word2),
		&(ipl_rec->pv_id.word3),
                &(ipl_rec->pv_id.word4));
    	}
    }
}
else
{
     if ((int) (ipl_rec -> IPL_record_id) != IPLRECID  ||
         (ipl_rec -> pv_id.word1 == 0  &&  ipl_rec -> pv_id.word2 == 0))
     {
    		odm_terminate();
         	return (LVM_RDPVID);
     }
}	

odm_terminate();
return (LVM_SUCCESS);

} /* END lvm_rdiplrec */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_relocmwcc                                                *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine attempts to write relocate the mirror write          *
 *   consistency cache block by writing it with a request for          *
 *   hardware relocation.                                              *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     pv_fd                                                           *
 *     mwcc                                                            *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     The MWCC block has been hardware relocated, if possible.        *
 *                                                                     *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_relocmwcc (

int pv_fd,
  /* file descriptor of physical volume where block containing the mirror
     write consistency cache needs to be relocated */

char mwcc [DBSIZE])
  /* buffer which contains data to be written to mirror write consistency
     cache */


{ /* BEGIN lvm_relocmwcc */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int retcode;
  /* return code */

off_t offset;
  /* offset at which to position file pointer */

char mwcc2 [DBSIZE];
  /* buffer in which to read MWCC which has been written with request
     for hardware relocation */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */

retcode = LVM_UNSUCC;
  /* initialize return code to unsuccessful */

offset = PSN_MWC_REC0 * DBSIZE;
  /* calculate the offset in bytes from beginning of the physical disk
     for the block containing the mirror write consistency cache */

offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
  /* position file pointer to beginning of MWCC */

if (offset != LVM_UNSUCC)
  /* if no errors on seek */

{

    retcode = writex (pv_fd, mwcc, DBSIZE, HWRELOC);
      /* attempt to write the MWCC, requesting hardware relocation of the
	 block */

    if (retcode == DBSIZE)
      /* if write with hardware relocation is successful */

    {

	offset = PSN_MWC_REC0 * DBSIZE;
	  /* calculate the offset in bytes from beginning of the physical
	     disk for the block containing the mirror write consistency
	     cache */

	offset = lseek (pv_fd, offset, LVM_FROMBEGIN);
	  /* position file pointer to beginning of MWCC */

	if (offset != LVM_UNSUCC)
	  /* if no errors on seek */

	{

	    retcode = read (pv_fd, mwcc2, DBSIZE);
	      /* attempt to read the MWCC block just written */

	    if (retcode == DBSIZE)
	      /* if read is successful */

	    {

		retcode = LVM_SUCCESS;
		  /* set return code to successful */

	    }

	} /* successful seek */

    } /* successful write with hardware relocation */

} /* successful seek */

return (retcode);
  /* return with return code */

} /* END lvm_relocmwcc */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_tscomp                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine compares two timestamps for greater than, equal      *
 *   to, or less than.                                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     ts1                                                             *
 *     ts2                                                             *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     None                                                            *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *    LVM_GREATER    If ts1 > ts2.                                     *
 *    LVM_EQUAL      If ts1 = ts2.                                     *
 *    LVM_LESS       If ts1 < ts2.                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_tscomp (

struct timestruc_t * ts1,
  /* first timestamp value */

struct timestruc_t * ts2)
  /* second timestamp value */


{ /* BEGIN lvm_tscomp */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

int status;
  /* status which indicates if first timestamp is greater than, equal to,
     or less than second timestamp */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


if (ts1 -> tv_sec > ts2 -> tv_sec)
  /* if the seconds portion of the first timestamp is greater than the
     seconds portion of the second timestamp */

{

    status = LVM_GREATER;
      /* set timestamp status to indicate first timestamp is greater than
	 second timestamp */

}

else
  /* seconds portion of first less than or equal */

{

    if (ts1 -> tv_sec == ts2 -> tv_sec)
      /* if seconds portion of two timestamps are equal */

    {

	if (ts1 -> tv_nsec > ts2 -> tv_nsec)
	  /* if nanoseconds portion of first timestamp is greater than
	     nanoseconds portion of second timestamp */

	{

	    status = LVM_GREATER;
	      /* set status to indicate first timestamp is greater than
		 second timestamp */

	}

	else
	  /* nanoseconds portion of first less than or equal */

	{

	    if (ts1 -> tv_nsec == ts2 -> tv_nsec)
	      /* if nanoseconds portion of two timestamps are equal */

	    {

		status = LVM_EQUAL;
		  /* set status to indicate two timestamps are equal */

	    }

	    else
	      /* nanoseconds portion of first timestamp less than
		 nanoseconds portion of second timestamp */

	    {

		status = LVM_LESS;
		  /* set status to indicate first timestamp is less than
		     second timestamp */

	    }

	} /* nanoseconds of first less than or equal */

    } /* seconds portion of two timestamps equal */

    else
      /* seconds portion of first is less than */

    {

	status = LVM_LESS;
	  /* set status to indicate first timestamp is less than second
	     timestamp */

    }

} /* seconds portion of first is less than or equal */

return (status);
  /* return timestamp status to indicate if first timestamp is greater
     than, less than, or equal to second */

} /* END lvm_tscomp */









/***********************************************************************
 *                                                                     *
 * NAME:  lvm_updtime                                                  *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine is used to update the beginning and ending           *
 *   timestamps for the volume group descriptor area and status area.  *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     beg_time                                                        *
 *     end_time                                                        *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *beg_time                                                       *
 *     *end_time                                                       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value >= 0 indicates successful return.                  *
 *                                                                     *
 *   The following return values are set by this routine:              *
 *     LVM_SUCCESS                                                     *
 *     LVM_PROGERR                                                     *
 *                                                                     *
 ***********************************************************************
 */


int
lvm_updtime (

struct timestruc_t * beg_time,
  /* a pointer to the beginning timestamp to be updated */

struct timestruc_t * end_time)
  /* a pointer to the ending timestamp to be updated */


{  /* BEGIN lvm_updtime */


/***********************************************************************
 *   Data declarations                                                 *
 ***********************************************************************
 */

struct timestruc_t cur_time;
  /* a structure which contains the current time from the system clock */

int retcode;
  /* return code */



/***********************************************************************
 *   Start of code                                                     *
 ***********************************************************************
 */


retcode = gettimer (TIMEOFDAY, &cur_time);
  /* get the current time from the system clock */

if (retcode == LVM_UNSUCC)
  /* if an error occurred */

{

    return (LVM_PROGERR);
      /* return error code for programming error */

}

if (cur_time.tv_sec < beg_time -> tv_sec  ||
     (cur_time.tv_sec == beg_time -> tv_sec &&
     cur_time.tv_nsec <= beg_time -> tv_nsec))
  /* if the current time is less than or equal to the beginning time to be
     updated */

{

    beg_time -> tv_sec = beg_time -> tv_sec + 1;
    end_time -> tv_sec = end_time -> tv_sec + 1;
      /* set the new beginning and ending timestamps to one second greater
	 than the old stored time */

}

else
  /* current time is greater than time to be updated */

{

    *beg_time = cur_time;
    *end_time = cur_time;
      /* update the timestamp for the beginning and ending times */

}

return (LVM_SUCCESS);
  /* return with successful return code */

} /* END lvm_updtime */

