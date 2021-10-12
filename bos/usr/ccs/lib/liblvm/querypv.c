static char sccsid[] = "@(#)70	1.33  src/bos/usr/ccs/lib/liblvm/querypv.c, liblvm, bos41B, 412_41B_sync 12/6/94 13:52:37";
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager Library Routines
 *
 * FUNCTIONS: lvm_querypv, partition_maps
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

#include <sys/stat.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <fcntl.h>
#include <liblvm.h>


int partition_maps(
 struct unique_id   *vg_id,	/* volume group id */	
 struct pp_entries  *pp_ptr,	/* pointer to physical partition entry */
 struct pv_header   *pvhead, 	/* pointer to physical volume header */
 struct fheader     *fhead,	/* pointer to vg mapped file header */
 struct querypv     *querypv,	/* pointer to the querypv structure */
 struct vgsa_area   *vgsa);     /* pointer to volume group status area */
/****************************************************************************/
/*
*  Name : lvm_querypv 
*
*  Function: This function is called by a system management command to
*  return information on the physical volume specified.  
*
*  NOTES:
*    Input:
*       pv_id       
*       querypv   
*       pvname
*
*     Output:
*       pv_name - name  of physical volume (ie. /dev/hdisk0)
*       ppsize - Size of physical partitions on this physical volume
*       pp_count - Number of physical partitions on this physical volume
*       alloc_ppcount - Number of allocated pp's on this pv
*       pp_map - A pointer to an array of all physical partitions on this 
*                physical volume.
*                Each array entry contains the state of the pp,the type, and the
*                lv_id of the logical volume it is part of.
*       pv_state - state of the physical volume.
*  
*    Return Value:  LVM_SUCCESS is returned if the routine succeeds, and one
*    of the following is returned if it fails.
*         LVM_OFFLINE
*         LVM_INVALID_PARAM
*         LVM_ALLOCERR
*         LVM_NOTVGMEM
*         LVM_NOPVVGDA
*         LVM_PROGERR
*         LVM_PVDAREAD
*         LVM_LVMRECERR
*         LVM_NOTCHARDEV
*         LVM_MAPFOPN
*         LVM_MAPFSHMAT
*         LVM_DALVOPN
*         LVM_PVOPNERR
*
*/
/**************************************************************************/
int 
lvm_querypv(
struct unique_id *vg_id,    /* volume group id */
struct unique_id *pv_id,    /* Unique id of physical volume to query */
struct querypv **querypv,   /* Pointer to the querypv structure where */
                            /* data is returned */ 
char *pvname)               /* name of pv to read descriptor area from */

{
       int  rc;                   /* contains the return code */
       int  vgfd;                 /* file descriptor for vg mapped file */
       long  buffer_size;         /* current size of buffer */
       char *vgmptr;              /* pointer to the vg mapped file */
       void *malloc();            /* allocates space */
       struct fheader *file_head; /* pointer to vg mapped file's file header */
       struct vg_header *vg_head; /* pointer to vg header in mapped file */
       struct pv_header *pvhead;  /* pointer to physical volume header in */
                                  /* mapped file */
       struct pp_entries *pps;    /* pointer to the first physical partition */
                                  /* entry for the physical volume queried */
       struct lv_entries *lv;     /* pointer to the logical volume entry */
       struct vgsa_area vgsa;	  /* structure for stale pp info */
       int test_state;            /* temporary Missing indicator for pv */
       int mode;                  /* mode of the volume group mapped file */

 
/*
*  if the querypv pointer is null
*     return an error
*/
   
    if(querypv == NULL)
    {   lvm_errors("querypv","    ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*   if the pv_id pointer is NULL
*     return an error
*/

    if(pv_id == NULL)
    {   lvm_errors("querypv","      ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/*
*   if the vg_id is NULL and pvname is null
*      return an error
*/

    if(vg_id == NULL && pvname == NULL)
    {   lvm_errors("querypv","    ",LVM_INVALID_PARAM);
        return(LVM_INVALID_PARAM);
    }

/* 
*  if the pvname is not null
*     call lvm_getpvda() to read the descriptor area into memory
*  else
*     call lvm_chkvaryon() to check that the volume group is on-line
*     call lvm_getvgdef() to open the volume group mapped file and return
*        the file descriptor and file pointer from the shmat
*/
   vgmptr = NULL;
   if(pvname == NULL)
   {
       mode = O_RDONLY | O_NSHARE;
       rc = lvm_getvgdef(vg_id,mode,&vgfd,&vgmptr);
       if(rc < LVM_SUCCESS)
       {
	   lvm_errors("lvm_getvgdef","querypv",rc);
	   free(vgmptr);
           return(rc);
       }
   }
   else
   {   if(strcmp(pvname,"\0") == 0)
       {
	   lvm_errors("querypv","    ",LVM_INVALID_PARAM);
	   free(vgmptr);
           return(LVM_INVALID_PARAM);
       }

       rc = lvm_getpvda(pvname,&vgmptr,FALSE);
       if(rc < LVM_SUCCESS)
       {
	   lvm_errors("lvm_getpvda","querypv",rc);
	   free(vgmptr);
           return(rc);
       }

   }


/* 
*  the get_ptrs routine returns the pointers to structures within the 
*  volume group mapped file for every parameter in the list that is not
*  NULL.
*/   

   rc =get_ptrs(vgmptr,&file_head,&vg_head,&lv,NULL,NULL,NULL);
   if(rc < LVM_SUCCESS)
   {
       if(pvname == NULL)
          close(vgfd);

       free(vgmptr);
       return(rc);
   }

/*
*  call get_pvandpp() to get a pointer to the pv and its first pp
*  (verify the pv is valid )
*/

   rc = get_pvandpp(&pvhead,&pps,NULL,vgmptr,pv_id);
   if(rc < LVM_SUCCESS)
   {
       lvm_errors("get_pvandpp","querypv",rc);
       if(pvname == NULL)
           close(vgfd);

       free(vgmptr);
       return(rc);
   }

   if(pvname != NULL)
   {
      vg_id->word1 = vg_head->vg_id.word1;
      vg_id->word2 = vg_head->vg_id.word2;
   } 
/*
*  calculate the size of the querypv buffer
*/

   buffer_size = sizeof(struct querypv);

/*
*  calculate the size of the pp map structure, based on the number of 
*  pps in the physical volume. Add this to the buffer size
*/

   buffer_size += pvhead->pp_count * sizeof(struct pp_map);

/* 
*  malloc space for the buffer and the map structure
*  If the malloc fails return an error code and exit
*/

   (*querypv) = (struct querypv *)malloc(buffer_size);
   if((*querypv) == NULL)
   {
       lvm_errors("malloc","querypv",LVM_ALLOCERR);
       if(pvname == NULL)
          close(vgfd);

       free(vgmptr);
       return(LVM_ALLOCERR);
   }

/* zero out the entire size of the buffer and map structure */

   bzero(*querypv,buffer_size);

/* 
* if pvname == NULL
* call getstates() to get a copy of the volume group status area 
* in order to check for stale physical partitions and missing
* physical volumes
*/
   if(pvname == NULL) {
   	rc = getstates(&vgsa,vgmptr);
   	if(rc < LVM_SUCCESS){
        	if(pvname == NULL)
           		close(vgfd);

           	free(vgmptr);
        	free(*querypv);
    	    	return(rc);
   	}	
   }

/* fill in the querypv structure */

   (*querypv)->ppsize = vg_head->pp_size; 
   (*querypv)->pv_state = pvhead->pv_state; 
   /*
    *  if a pvname wasn't given, use the macro from vgsa.h to test the
    *	 pv state for missing
    */
   if(pvname == NULL) {
   	test_state = TSTSA_PVMISS(&vgsa,pvhead->pv_num-1) ? TRUE : FALSE;
        if(test_state == TRUE) 
	   if(!(pvhead->pv_state & LVM_PVREMOVED))
              (*querypv)->pv_state = LVM_PVMISSING;
   }
   else
	(*querypv)->pv_state &= LVM_ACTIVEMASK;
   (*querypv)->pp_count = pvhead->pp_count;
   (*querypv)->pp_map = (struct pp_map *)
             ((char *)*querypv + sizeof(struct querypv));

   (*querypv)->pvnum_vgdas = pvhead->pvnum_vgdas; 

/*
*  if a pvname was specified, call partition maps with NULL in the place
*  of the address for the vgsa.
*  else
*  call partition_maps with the vgsa pointer to fill in the partition map array
*/
   if(pvname == NULL) {
   	rc = partition_maps(vg_id,pps,pvhead,file_head,*querypv,&vgsa);
   	if(rc < LVM_SUCCESS)
   	{   if(pvname == NULL)
        	  close(vgfd);

            free(vgmptr);
       	    free(*querypv);
            return(rc);
    	}
        close(vgfd);
   }
   else {
   	rc = partition_maps(vg_id,pps,pvhead,file_head,*querypv,NULL);
   	if(rc < LVM_SUCCESS)
   	{
	    if(pvname == NULL)
        	  close(vgfd);

            free(vgmptr);
       	    free(*querypv);
            return(rc);
    	}
   }
    free(vgmptr);
    return(LVM_SUCCESS);
}
/***************************************************************************/
/* Function Name: partition_maps
*
*  Function: This routine returns a partition map of all pps on the pv. 
*
*  Input:
*     vg_id
*     lv
*     pp_ptr
*     pvhead
*     fhead
*     querypv 
*     vgsa 
*     test_state 
*
*   Output:
*      a list of all partitions in the pv with their state,type and lv_name,
*      the new state of the physical volume if the state has changed,and the
*      pp_count.
*
*   Return Values : LVM_SUCCESS
*
*/
/***************************************************************************/
int partition_maps(
struct unique_id  *vg_id,  /* volume group id */
struct pp_entries *pp_ptr, /* pointer to 1st physical partition entry */
struct pv_header *pvhead,  /* pointer to physical volume being queried */
struct fheader *fhead,     /* pointer to the vg mapped file header */
struct querypv *querypv,   /* points to the querypv buffer */
struct vgsa_area *vgsa)    /* points to volume group status area */

{

struct pp_map *map_ptr; /* pointer to the map list being built */
long   counter;         /* counts physical partitions checked */
int   stale;            /* indicates a stale physical volume */
long alloc_count;       /* counts the allocated pps */
int test_state;	        /* pp state from vgsa area */ 

/*
*  initialize a counter, flag variable and pointer
*/
     map_ptr = querypv->pp_map;
     counter = 1;
     alloc_count = 0;
     stale = FALSE;
     while (counter <= pvhead->pp_count)
/*
* look at each pp_entry and pick up the part_type ,pp_state, and
* the lv_index.   
*/

      { 
        if(pp_ptr->pp_state & LVM_PPALLOC)
/*
* Because we zeroed out the buffer and map structure, when we run 
* across a pp with a state of LVM_PPFREE we don't need to do anything
* because its entry is already all zeroes.
*/
        {
       	   /* fill in the map structure entry for the pp */
           map_ptr->pp_state = pp_ptr->pp_state;
	   /*
	    *  Use the lv_index to fill in the minor number of the lv_id 
            *  and use the vg_id to fill in the rest of the lv_id
            */
           map_ptr->lv_id.minor_num = pp_ptr->lv_index;
           map_ptr->lv_id.vg_id.word1 = vg_id->word1;
           map_ptr->lv_id.vg_id.word2 = vg_id->word2;
           map_ptr->lp_num = pp_ptr->lp_num;
           map_ptr->copy = pp_ptr->copy;
	   if( pp_ptr->fst_alt_vol )
	   {   map_ptr->fst_alt_vol.word1 = 
                fhead->pvinfo[pp_ptr->fst_alt_vol - 1].pv_id.word1;
               map_ptr->fst_alt_vol.word2 = 
                fhead->pvinfo[pp_ptr->fst_alt_vol - 1].pv_id.word2;
               map_ptr->fst_alt_part = pp_ptr->fst_alt_part;
	   }
	   if( pp_ptr->snd_alt_vol )
	   {   map_ptr->snd_alt_vol.word1 =
                fhead->pvinfo[pp_ptr->snd_alt_vol - 1].pv_id.word1;
               map_ptr->snd_alt_vol.word2 = 
                fhead->pvinfo[pp_ptr->snd_alt_vol - 1].pv_id.word2;
               map_ptr->snd_alt_part = pp_ptr->snd_alt_part;
	   }
           if(vgsa != NULL) {
           	/* use macro from vgsa.h to test for stale pps */
           	test_state = TSTSA_STLPP(vgsa,pvhead->pv_num-1,counter-1) ?
				TRUE :  FALSE;
           	if(test_state == TRUE) {
              		if(stale == FALSE)
	                /* if the pp is stale, set the pv state to stale */
              		{ 
                  		querypv->pv_state |= LVM_PVSTALE;
                  		stale = TRUE;
              		}
           	}
           	/* if the test_state is TRUE, turn the stale bit on here, too */
           	if(test_state == TRUE)
              		map_ptr->pp_state |= LVM_PPSTALE;
           }
           alloc_count ++;
        }
        /* increment all pointers and the counter */
        pp_ptr ++;
        counter ++;
        map_ptr ++;
      }
      querypv->alloc_ppcount = alloc_count;
      return(LVM_SUCCESS);
}
/****************************************************************************/
