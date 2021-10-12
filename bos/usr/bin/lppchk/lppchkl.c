static char sccsid[] = "@(#)40  1.4.1.3  src/bos/usr/bin/lppchk/lppchkl.c, cmdswvpd, bos411, 9428A410j 3/31/94 17:45:56";
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
 * NAME: lppchkl (link verify)
 *
 * FUNCTION: Verify the links specified in the inventory database
 *
 * EXECUTION ENVIRONMENT:
 *
 *      For each file processed, any symbolic links specified in the
 *      inventory entry will be verified.  If the link does not exist
 *      and update has been specified, the link will be established.
 *      otherwise an error message will be issued describing the
 *      missing symbolic link.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - processing errors found.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */
#include        "sysdbio.h"     /* sysck definitions for types of files */
#include        <sys/errno.h>

static int cklink( char *, char *, int) ;
static int mklink(char *, char *, int, int, char *) ;

int lppchkl(                    /* drive link verification process      */
  struct lpp_ids *ids,          /* lpp_id list control structure        */
  char    *fileid,              /* pointer to file name to be checked   */
  int     opt_u)                /* boolean update flag                  */

  {
  inv_t inv ;                   /* local inventory structure            */
  int   inv_mask ;              /* search mask to vpdget                */
  int   vpd_rc ;                /* return code from vpd routines        */
  int   rc ;			/* return code to caller		*/
  char  *p0, *p1 ;              /* pointers into list of symlinks       */

/************************************************************************/
/* when exactly one lpp_id is in the list, then set that id into the    */
/* inv structure and include it as part of the search.  That lets ODM   */
/* filter out only the items we want to see                             */
/************************************************************************/

  rc = 0 ;			/* init return code 			*/
  strncpy(inv.loc0 , fileid, sizeof(inv.loc0));
                                /* copy file name to structure          */

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

    while (vpd_rc == VPD_OK)    /* while matching inventory entries     */
      {
      if ((ids->idcnt == 1) ||  /* if lpp_id found is valid             */
          (lppchkil(ids,inv.lpp_id)))
        { 
        chk_file_cnt ++ ;	/* step count of matching files		*/
                                /* if symbolic link string is not null  */
				/* and this is not a hard link type     */
        if ((inv.loc2 != NULL) && ( *(inv.loc2) != '\0') && 
			(inv.file_type != TCB_LINK))
        {
	  if (inv.file_type == TCB_SYMLINK) {
            if (cklink(inv.loc0,inv.loc2,opt_u) != 0) {rc += 1;}
          }
	  else {
            p0 = p1 = inv.loc2 ;  /* init pointer to list of link names */
            while (p0 != NULL)    /* process each link name separately  */
            {
              p1 = strchr(p0,',');/* names are delimited by commas      */
              if (p1 != NULL)     /* if not at last link name           */
                { *p1 = '\0' ;}   /* put null delimiter over next comma */
              if (cklink(p0,inv.loc0,opt_u) != 0) {rc += 1;}
                                /* go check that link                   */
              if (p1 != NULL)     /* if not last link already           */
                { *p1++ = ',' ;}  /* restore comma and step over it     */
              p0 = p1 ;           /* set up for next iteration          */
             }                   /* endwhile - more link names specified*/
           }                     /* end else this is not a TCB_LINK     */
          }			/* end entry has symlinks to be checked */
        }                       /* end if inventory entry found         */
      lppck_free_inv(&inv);
      vpd_rc = vpdgetnxt(INVENTORY_TABLE, &inv) ;
      }                         /* end while still finding matching ents*/
    }                           /* end one or more lpp_id's specified   */
  return(rc) ;			/* return with count of errors		*/
  }                             /* end lppchkl - check links            */


/*
 * NAME: cklink (check link)
 *
 * FUNCTION: Verify the link specified by the parameters
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The source specifies the symbolic link entry in the directory,
 *      that is the pointer to the real data.  The target specifies the
 *      real file that is to be accessed using the symbolic link.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - processing errors found.
 *
 */


static int cklink(
  char  *ls ,                   /* pointer to link source name          */
  char  *lt ,                   /* pointer to link target name          */
  int   updt)                   /* boolean - update requested if error  */

  {

  int   rc ;                    /* return code                          */
  struct stat st ;              /* result from stat system call         */
  char  tname[1024] ;           /* hold actual link name                */

  TRC(TRC_ENTRY,"cklink - source = %s, Target = %s\n",ls,lt,0);

  rc = lstat(ls, &st) ;         /* determine if source exists already   */
  if (rc != 0 )
    {
    rc = mklink(ls, lt, updt, MSG_CHK_LNOSRC, DEF_CHK_LNOSRC) ;
    }                           /* no pre existing source, go do link   */
  else
    {
    if (st.st_type == VLNK)     /* if source is a symbolic link entry   */
      {
      rc = readlink(ls, tname, sizeof(tname));
                                /* read the content of the link         */
      if (rc == -1)             /* if error reading the link            */
        {
        rc = mklink(ls, lt, updt, MSG_CHK_LRLERR, DEF_CHK_LRLERR) ;
        }
      else
        {
        tname[rc] = '\0' ;      /* supply null delimiter                */
        if (strcmp(tname, lt) != 0)
          {                     /* check for match to specified target  */
          rc = mklink(ls, lt, updt, MSG_CHK_LNEQ, DEF_CHK_LNEQ) ;
                                /* if not matched - go rebuild          */
          }                     /* end existing symlink w/ wrong target */
        else {rc = 0;} 		/* else all is normal, set return code	*/
        }                       /* end no error reading link contentif  */
      }                         /* end existing file is a link          */
    else                        /* source exists but is not a link      */
      {
      rc = mklink(ls, lt, updt, MSG_CHK_LNLNK, DEF_CHK_LNLNK) ;
      }                         /* end existing file but not link       */
    }                           /* end existing file at source          */

  TRC(TRC_EXIT,"exit cklink - return code = %d\n",rc,0,0);
  return (rc) ;                 /* return to caller                     */
  }                             /* end cklink routine                   */


/*
 * NAME: mklink (make link)
 *
 * FUNCTION: Establish the specified link (if updating)
 *
 * EXECUTION ENVIRONMENT:
 *
 *      If the update flag indicates update is to be done, establish the
 *      link from 'source' to 'target',  else issue the message
 *      specified by the parameters.
 *
 * NOTE:
 *      All messages passed to this routine must expect the insertion
 *      specifications to have the source file name inserted first, and
 *      the target file name inserted second.
 *
 * RETURNS: 0 - always - errors result in messages
 *
 */


static int mklink(
  char  *ls,                    /* pointer to name of source for link   */
  char  *lt,                    /* pointer to name of target for link   */
  int   updt,                   /* boolean - true indicates create link */
  int   msgn,                   /* if updt false issue this message     */
  char  *msgt)                  /* default string for message           */
  {

  int   rc ;                    /* return code                          */


  TRC(TRC_ENTRY,"entry mklink - reason %s\n",msgt,0,0);

  if (updt)
    {
    rc = access(ls, R_ACC) ;    /* determine if the source exists       */

    if (rc == 0)                /* if the source exists it must be      */
      { rc = remove(ls) ; }     /* removed                              */
    else
      { rc = 0 ; }              /* indicate no error thus far           */

    if (rc == 0)                /* if no error so far, set the link     */
      { rc = symlink(lt, ls) ; }

    if (rc != 0)                /* if error on remove or symlink        */
      {
      rc = errno ;		/* capture error code value		*/     
                                /* report the error.                    */
      MSG_S(MSG_ERROR, MSG_CHK_LNEWE, DEF_CHK_LNEWE, ls, lt, rc) ;
      fprintf(stderr, "	");     
      perror("");               /* Pass perror null str to supress ":"  */
      fprintf(stderr, "\n");
      }
    else
      {
      MSG_S(MSG_INFO, MSG_CHK_LNEW, DEF_CHK_LNEW,ls, lt, 0);
                                /* report new link established          */
      }                         /* end no error creating symlink        */
    }                           /* end update requested logic           */
  else
    {
    rc = 1 ;			/* have error, indicate that		*/
    MSG_S(MSG_ERROR, msgn, msgt, ls, lt, 0);
                                /* report reason link is needed         */
    } /* endif */
  TRC(TRC_EXIT,"exit mklink ",0,0,0);
  return (rc);
  }                             /* end mklink routine                   */
