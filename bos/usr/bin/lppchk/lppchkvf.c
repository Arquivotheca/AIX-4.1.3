static char sccsid[] = "@(#)43  1.5  src/bos/usr/bin/lppchk/lppchkvf.c, cmdswvpd, bos411, 9428A410j 5/26/94 13:17:27";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: lppchkvf (verify file data)
 *
 * FUNCTION: Check size and optionally checksum of file specified in
 *      the inventory record passed.  If needed and requested update the
 *      size and checksum fields in the inventory record structure
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Verify that the size of the file specified in the inventory
 *      matches the size value in that structure.  Values in the structure
 *      of 0 or -1 will 'match' any real size.  If update is requested
 *      and the size in the structure not -1 (indicating no size value
 *      should be computed), the structure will be updated and the return
 *      code will be set to indicate that the inventory needs to be updated.
 *      If the option is 'c', then the checksum will be computed and verified
 *      and updated as was the size value.
 *
 * INPUT:
 *      opt     - char with value f (fast, size check only) or c (full
 *                checksum validation).
 *      inv     - pointer to inventory structure with file information.
 *      updt    - boolean indicating if update is to be performed
 *      ecnt    - pointer to error counter. To be incremented if an
 *                error is found with the specified file.
 *
 * OUTPUT:
 *      Inventory structure will be updated with size and checksum if
 *      needed.  Error messages as appropriate.
 *
 * RETURNS: CHK_OK - no errors, no need to update inventory
 *          CHK_UPDT - no errors, inventory needs to be updated
 *          CHK_BAD_RET - file size or checksum error, or system
 *                        error.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */

int lppchkvf (
  char  opt ,                   /* option letter, 'f' or 'c'            */
  inv_t *inv ,                  /* inventory structure with file data   */
  int   updt ,                  /* boolean, update requested if needed  */
  int   *ecnt )                 /* error counter                        */

  {                             /* begin lppchkvf - verify single file  */

  int   vf_rc,                  /* return code from this program        */
        rc,                     /* hold system routine return code      */
        sum,                    /* computed file checksum               */
        szx;                    /* computed file size                   */

  struct stat st ;              /* result from stat system call         */
  FILE  *fc ;                   /* stdio file control pointer           */
/*----------------------------------------------------------------------*/
  TRC(TRC_ENTRY,"lppchkvf entry - file = %s\n",inv->loc0,0,0);

  vf_rc = CHK_OK ;              /* set default return code              */
/************************************************************************/
/* use stat to verify file existance and get size information           */
/************************************************************************/

  rc = lstat(inv->loc0,&st) ;    /* get file status data                 */
  if (rc == -1)
    {
    if (errno == EACCES)
      {
      MSG_S(MSG_ERROR, MSG_CHK_NOPERM, DEF_CHK_NOPERM, inv->loc0, 0, 0) ;
      }
    else
      {
      MSG_S(MSG_ERROR, MSG_CHK_NOFILE, DEF_CHK_NOFILE, inv->loc0, 0, 0) ;
      }
    *ecnt += 1 ;
    vf_rc = CHK_BAD_RET ;       /* report error, and set retcode        */
    }
  else
    {
    if ((inv->size != -1) &&	/* entry size of 0 or -1 implies the   	*/
        (inv->size != 0))       /* size is to be ignored - (-1 should   */
				/* be replaced by syschk, 0 is for a    */
				/* file that varies in size) 		*/
      {                         /* only verify file existance           */

      if (inv->size != st.st_size)
                                /* if size does not match inventory val */
        {
        if (updt)               /* if update has been requested         */
          {
          MSG_S(MSG_INFO, MSG_CHK_NEWSZ, DEF_CHK_NEWSZ, inv->loc0,
              inv->size, st.st_size) ;
          inv->size = st.st_size ;
          vf_rc = CHK_UPDT ;    /* report changing size for file        */
          }
        else
          {
          MSG_S(MSG_ERROR,MSG_CHK_BADSZ, DEF_CHK_BADSZ, inv->loc0,
              st.st_size, inv->size) ;
          *ecnt += 1 ;          /* report error and step count          */
          vf_rc = CHK_BAD_RET ;
          } /* endif */
        }                       /* end - if size is wrong and not ignore*/
      }                         /* end if size in inventory is not -1   */
    }                           /* end if - return code from stat       */
/************************************************************************/
/* if requested and reasonable, compute checksum and verify it          */
/************************************************************************/
  if ((vf_rc != CHK_BAD_RET) && /* continue only if no prior error      */
      (opt == 'c'))             /* and request to verify checksum       */
    {
    if ((inv->checksum == 0) || /* zero checksum ==> don't check it     */
        (inv->checksum == -1))  /* likewise if checksum is -1 		*/
      {
      MSG_S(MSG_INFO+1, MSG_CHK_NOCKSUM, DEF_CHK_NOCKSUM, inv->loc0, 0, 0) ;
      }
    else                        /* inventory cksum is not zero - verify */
      {
      fc = fopen(inv->loc0,"r");/* create file descriptor for file      */

      if (fc == NULL)           /* if open failed need to report error  */
        {                       /* since status did not fail, error must*/
                                /* be permissions.                      */
        MSG_S(MSG_ERROR, MSG_CHK_RDPERM, DEF_CHK_RDPERM, inv->loc0, 0 ,0);
        *ecnt += 1 ;
        vf_rc = CHK_BAD_RET ;
        }
      else
        {
        sum = lppchkck(fc,&szx);/* go compute the checksum value        */
	fclose(fc) ;		/* close file after computing sum	*/

        if (sum != inv->checksum)
                                /* if checksum does not match inventory */
          {
          if (updt)             /* if update is requested               */
            {
            MSG_S(MSG_INFO, MSG_CHK_NEWSUM, DEF_CHK_NEWSUM, inv->loc0,
                inv->checksum, sum) ;
            inv->checksum = sum ;
            vf_rc = CHK_UPDT ;  /* inform user, set new sum, set updt   */
            }
          else                  /* mismatch but no update requested     */
            {
            MSG_S(MSG_ERROR, MSG_CHK_BADCK, DEF_CHK_BADCK, inv->loc0,
                sum, inv->checksum) ;
            vf_rc = CHK_BAD_RET ;
            *ecnt += 1 ;        /* report it, count it, set return code */
            }                   /* end checksum not to be updated       */
          }                     /* end if checksum does not match       */
        }                       /* end successfully opened the file     */
      }                         /* end - inventory cksum not zero       */
    }                           /* end process checksum verify request  */

/************************************************************************/
/* ensure return code is set properly and exit                          */
/************************************************************************/
  TRC(TRC_EXIT,"lppchkvf exit - return code %d\n",vf_rc,0,0);

  return(vf_rc) ;               /* return to caller                     */
  }                             /* end lppchkvf                         */

