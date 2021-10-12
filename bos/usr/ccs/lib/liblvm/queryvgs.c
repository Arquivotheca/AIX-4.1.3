static char sccsid[] = "@(#)79	1.12  src/bos/usr/ccs/lib/liblvm/queryvgs.c, liblvm, bos411, 9430C411a 7/19/94 11:57:14";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_queryvgs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/hd_config.h>
#include <liblvm.h>
#include <sys/dasd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

extern void * malloc();
/*****************************************************************************
* Name : lvm_queryvgs 
*
* Function:  This function is called by a system management command to 
*  return the information on the system in terms of volume groups.
*
*  NOTES:
*    Input:
*      queryvgs 
*      kmid
*
*    Output:
*      numvgs
*      vg_ids and major numbers of all volume groups that are varied on in
*      the system    
*
*  Return Values : LVM_SUCCESS if successful and one of the following if not
*                  successful:
*                      LVM_INVALID_PARAM
*                      LVM_OFFLINE
*                      LVM_PROGERR
*                      LVM_MAPFOPN
*                      LVM_MAPFSHMAT
*                      LVM_ALLOCERR
*                      LVM_LVOPEN
*/
/***************************************************************************/
int 
lvm_queryvgs(
struct queryvgs **queryvgs, /* pointer to the buffer returned by the routine */
mid_t             kmid)    /* address of kernel module */

{
   int rc;                  /* contains the return codes */
   long buffer_size;        /* current size of buffer */
   char *vgmptr;            /* pointer to the volume group mapped file */
   int vgfd;                /* file descriptor for the volume group */
                            /* mapped file */
   long major_num;          /* major number of volume group corresponding */
                            /* with vg_id */
   struct ddi_info cfgdata; /* input structure for kernel routine */
   struct fheader *fhead;   /* pointer to the mapped file header */

/*
*    initialize return code
*/
 
    rc = LVM_SUCCESS;

/*
*  if the queryvgs field is NULL
*     return an error
*/

    if(queryvgs == NULL)
       return(LVM_INVALID_PARAM);

    kmid = (mid_t)loadext("hd_pin",FALSE,TRUE); /* get the real kmid, avoiding
						 * the possibility of invalid
						 * numbers, ignore what they
						 * sent in (Defect 155717)
						 */
/*
*   if the kmid is NULL
*       return an error
*/
    if(kmid == NULL)
        return(LVM_INVALID_PARAM);
 

/*
* calculate the buffer size for the queryvgs struct
*/

    buffer_size = sizeof(struct queryvgs);

/*
*   malloc the necessary space and return an error code if the 
*   malloc fails
*/
   (*queryvgs) = (struct queryvgs *) malloc(buffer_size);
   if((*queryvgs) == NULL)
   {   lvm_errors("malloc","queryvgs",LVM_ALLOCERR);
       return(LVM_ALLOCERR);
   };

/*  zero out the malloced space */

   bzero((char *)(*queryvgs),buffer_size);

/*
*  set up the cfgdata structure
*  call the kernel config routine and fill in the queryvgs
*  structure.
*/

   bzero((char *)(&cfgdata.parmlist.kqryvgs),sizeof(struct kqryvgs));
   cfgdata.parmlist.kqryvgs.queryvgs = (*queryvgs);
   rc = lvm_config(kmid,0,HD_KQRYVGS,(&cfgdata));
   if(rc < LVM_SUCCESS)
   {   lvm_errors("lvm_config","queryvgs",rc);
       return(rc);
   }

   return(rc);
}

/******************************************************************************/
