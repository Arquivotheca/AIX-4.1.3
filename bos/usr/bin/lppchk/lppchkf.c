static char sccsid[] = "@(#)39  1.3.2.2	src/bos/usr/bin/lppchk/lppchkf.c, cmdswvpd, bos411, 9428A410j 6/11/93 12:05:43" ;
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk (main)
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
 * NAME: lppchkf (check files)
 *
 * FUNCTION: Check size and optionally checksum of files specified.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Check files listed in the current inventory data base, where
 *      those files match the search critera specified, and are associated
 *      with one of the lpp_id's in the list provided.   The size will
 *      be checked against the inventory information and if requested the
 *      checksum will be similarly verified.  If the update option is
 *      specified, the inventory file will be updated when the actual
 *      values differ from those in the inventory.
 *
 * INPUT:
 *      opt     - char with value f (fast, size check only) or c (full
 *                checksum validation).
 *      updt    - boolean indicating if update is to be performed
 *      idlist  - pointer to structure containing lpp_id's to be processed
 *      fileid  - pointer to file name specifier for files to be processed,
 *                may include ODM wild card characters.
 *
 * OUTPUT:
 *      Inventory data base will be updated as needed, when updt is TRUE
 *      Messages if any errors detected (msg_lev, controls what severity
 *      messages will be presented.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - processing errors found.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */


int lppchkf (
  char  opt ,                   /* 'f' or 'c' for fast or complete check*/
  int   updt ,                  /* boolean TRUE if update requested     */
  struct lpp_ids *ids ,         /* pointer to lpp_ids to be processed   */
  char  *fileid			/* pointer to file specifier            */
  )

  {                             /* begin lppchkf                        */

  int   err_file,               /* files with errors                    */
        err_arch,               /* archive members with errors          */
        err_cnt,                /* update inventory errors              */
        cnt_files,              /* files processed                      */
        cnt_memb,               /* archive members processed            */
        cnt_updt,               /* inventory entries updated            */
        vpd_rc,                 /* return code from vpd routines        */
        v_rc,                   /* return code from verify routines     */
        inv_mask,               /* search mask parm to vpdget           */
        old_sum,                /* hold initial checksum from inventory */
        old_size ,              /* hold initial size from inventory     */
	st_time ;		/* hold start for 30 sec check 		*/

  char  *archform ;             /* pointer to ':' if file spec is in    */
                                /* arch:member format, else NULL        */
  char  archname[MAX_INV_LOC0]; /* hold archive file name               */
  inv_t inv ;                   /* working copy of inventory entry      */

/*----------------------------------------------------------------------*/
  TRC(TRC_ENTRY,"lppchkf - fileid = %s, idcnt = %d\n",fileid,ids->idcnt,0);

  err_file = err_arch = err_cnt = 0 ;
  cnt_files = cnt_memb = cnt_updt = 0 ;
				/* initialize all counters		*/

  archform = strchr(fileid,':') ;
                                /* locate any colon - implies mem:arch  */
                                /* name format                          */

  if (archform)                 /* if name is archive:member format     */
    {
    *archform = '\0' ;          /* put in null to stop copy             */
    strncpy(inv.loc0, fileid, sizeof(inv.loc0)) ;
    strncpy(archname, (archform + 1), sizeof(archname)) ;
    *archform = ':' ;           /* copy parts to hold areas, reset input*/
    }
  else
    {
    strncpy(inv.loc0, fileid, sizeof(inv.loc0));
                                /* copy simple name to search struct    */
    }                           /* end if file name is in arch:mem form */

/************************************************************************/
/* when exactly one lpp_id is in the list, then set that id into the    */
/* inv structure and include it as part of the search.  That lets ODM   */
/* filter out only the items we want to see                             */
/************************************************************************/

  if (ids-> idcnt == 1 )        /* exactly one lpp_id is special case   */
    {
    inv.lpp_id = ids->ida->id[0] ;
                                /* put that id into the search struct   */
    inv_mask = INV_LPP_ID | INV_LOC0 ;
    }                           /* include id in search condition       */
  else
    {
    inv_mask = INV_LOC0 ;       /* else can only search on name         */
    } /* endif */

/************************************************************************/
/* begin the search of the inventory data base for entries that match   */
/* the fileid and the lpp_ids.  Those matching entries are then verified*/
/************************************************************************/

  if (ids->idcnt >= 1)          /* only search if there are possible ids*/
    {
    vpd_rc = vpdget(INVENTORY_TABLE, inv_mask, &inv) ;
                                /* get the initial entry from the inv   */

    st_time = time(NULL) ;	/* establish base time			*/
    while (vpd_rc == VPD_OK)    /* while matching inventory entries     */
      {
      if ((time(NULL) - st_time) > 30)
        {			/* report progress every 30 sec		*/
        MSG_S(MSG_INFO, MSG_CHK_PROGRESS, DEF_CHK_PROGRESS,
                cnt_memb+cnt_files, 0, 0) ;
        st_time = time(NULL) ;
        }
	 
      if ((ids->idcnt == 1) ||  /* if lpp_id found is valid             */
          (lppchkil(ids,inv.lpp_id)))
        {
        chk_file_cnt ++ ;	/* count files processed		*/

        old_size = inv.size ;   /* save copy of size and cksum for msg  */
        old_sum  = inv.checksum;
        v_rc = CHK_OK ;         /* set default return code              */
        if (archform)           /* if name is arch member name          */
          {
          if ((inv.format == INV_ARCHIVED) &&
              (strcmp(archname,inv.loc1) == 0))
            {                   /* if archive name matches arg          */
            cnt_memb ++ ;       /* count archive members processed      */
            v_rc = lppchkva(opt, &inv, updt, &err_arch) ;
            }                   /* verify archive member data           */
          }
        else
          {
	  if (inv.format == INV_FILE)
	    {			/* check only if entry is type file	*/
            cnt_files ++ ;      /* count files processed                */
            v_rc = lppchkvf(opt, &inv, updt, &err_file) ;
				/* verify the file specified		*/
	    }			/* end - if type is file		*/
                                /* verify single file data              */
          }                     /* end test if name found for arch srch */

/************************************************************************/
/* if the verify routine determined that the inventory should be        */
/* updated, then do that update here                                    */
/************************************************************************/

        if (v_rc == CHK_UPDT)
          {
          cnt_updt ++ ;         /* count number of update requests      */
          vpd_rc = vpdchgget(INVENTORY_TABLE,&inv) ;
                                /* update the inventory data            */
          if (vpd_rc == VPD_OK)
            {
            MSG_S(MSG_INFO, MSG_CHK_UPDATE, DEF_CHK_UPDATE,
                inv.loc0, inv.size, inv.checksum) ;
            }
          else
            {
            err_cnt ++ ;        /* count errors                         */
            MSG_S(MSG_ERROR, MSG_CHK_VPDERR, DEF_CHK_VPDERR,
                vpd_rc, odmerrno, 0) ;
            }                   /* end test update request return code  */
          }                     /* end if update requested              */
        }                       /* end vpd_id is in the list            */

      lppck_free_inv(&inv) ;	/* free vchar data areas 		*/

      vpd_rc = vpdgetnxt(INVENTORY_TABLE, &inv) ;
                                /* search for next matching entry       */
      }                         /* end while matching inventory entries */

    if (vpd_rc != VPD_NOMATCH)  /* nomatch is only expected code at exit*/
      {
      MSG_S(MSG_ERROR,MSG_CHK_VPDERR,DEF_CHK_VPDERR, vpd_rc, odmerrno,0) ;
      }

    MSG_S(MSG_INFO, MSG_CHK_PROGRESS, DEF_CHK_PROGRESS,
          cnt_memb+cnt_files, 0, 0) ;

    }                           /* end, do searches if any lpp_ids      */

  TRC(TRC_EXIT,"Exit lppchkf - errors = %d, files/memb proc = %d/%d\n",
      err_cnt+err_file+err_arch, cnt_files, cnt_memb) ;
  return (err_cnt + err_file + err_arch) ;
  }                             /* end lppchkf                          */
