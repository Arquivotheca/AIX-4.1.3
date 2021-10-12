/* @(#)27	1.5  src/bos/usr/ccs/lib/libswvpd/vpdremote.c, libswvpd, bos411, 9428A410j 6/20/94 15:48:04 */
/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdlocal, vpdlocalpath, vpdremote, vpdremotepath
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <swvpd0.h>                     /* SWVPD internal definitions   */

/*
 * NAME: vpdremote
 *
 * FUNCTION: Establish the alternate set of SWVPD Tables as current set
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Establishes the path to the 'alternate' SWVPD tables as the
 *      current path.  Supports the ability of an application to have
 *      access to two sets of SWVPD tables available and to switch from
 *      one set to the other.
 *
 * (NOTES:)
 *      Sets the control varible in the vpd_ctl structure which tracks
 *      which of the SWVPD sets is to be accessed.
 *
 * RETURNS:
 *      Always returns VPD_OK ;
 */

int  vpdremote()
{
  vpd_ctl.cur_path = VPD_REMOTE ;       /* establish alternate tables   */
  return(VPD_OK);                       /* as current set               */
}


/*
 * NAME: vpdremotepath
 *
 * FUNCTION: Specifies the path to the alternate SWVPD tables
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Establishes the path name to the 'alternate' SWVPD tables. This
 *      path will not be made active until vpdremote is invoked.
 *
 * (NOTES:)
 *      Sets the control varible in the vpd_ctl structure which tracks
 *      the path to the alternate SWVPD tables.
 *
 * RETURNS:
 *      return code from vpd_setpath
 */

int  vpdremotepath ( path )
  char  *path ;                         /* pointer to path name         */
{
  if ((path == (char *) NULL) ||        /* if default requested         */
      (*path == '\0' ))                 /* pointer null or null string  */
    {
    path = VPD_REMOTE_PATH ;            /* provide default for remote   */
    }

  return(vpd_setpath (path, VPD_REMOTE));
                                        /* go establish the path name   */
}                                       /* end vpdremotepath            */


/*
 * NAME: vpdlocal
 *
 * FUNCTION: Establish the primary set of SWVPD Tables as current set
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Establishes the path to the 'primary' SWVPD tables as the
 *      current path.  Supports the ability of an application to have
 *      access to two sets of SWVPD tables available and to switch from
 *      one set to the other.
 *
 * (NOTES:)
 *      Sets the control varible in the vpd_ctl structure which tracks
 *      which of the SWVPD sets is to be accessed.
 *
 * RETURNS:
 *      Always returns VPD_OK ;
 */


int  vpdlocal()
{
  vpd_ctl.cur_path = VPD_LOCAL  ;       /* establish alternate tables   */
  return(VPD_OK);                       /* as current set               */
}


/*
 * NAME: vpdlocalpath
 *
 * FUNCTION: Specifies the path to the primary SWVPD tables
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Establishes the path name to the 'primary' SWVPD tables. This
 *      path will not be made active until vpdlocal is invoked.
 *
 * (NOTES:)
 *      Sets the control varible in the vpd_ctl structure which tracks
 *      the path to the primary SWVPD tables.
 *
 * RETURNS:
 *      return code from vpd_setpath
 */

int  vpdlocalpath ( path )
  char  *path ;                         /* pointer to path name         */
{
  if ((path == (char *) NULL) ||        /* if default requested         */
      (*path == '\0' ))                 /* pointer null or null string  */
    {
    path = VPD_LOCAL_PATH ;             /* provide default for remote   */
    }

  return(vpd_setpath (path, VPD_LOCAL));
                                        /* go establish the path name   */
}                                       /* end vpdremotepath            */


/*
 * NAME: vpd_setpath
 *
 * FUNCTION: Manages changing the path to either the primary or alternate
 *           set of SWVPD tables
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Ensures that the path name is safely changed for the specified
 *      alternative.   If the name is not changing, exits normally. If
 *      the name is changing, ensures that there are no tables on the
 *      old path that remain open, and then switches the name.
 *
 * (NOTES:)
 *      Sets the path name for the indicated alternative, and closes any
 *      tables that were open on the old path.
 *
 * RETURNS:
 *      VPD_OK          if no errors
 *      VPD_SYS         if odm or system routine error
 */


static vpd_setpath(name,cpath)

  char  *name ;                         /* pointer to path name to use  */

  int   cpath ;                         /* index of path to be changed  */

{                                       /* begin vpd_setpath            */

  int   tix ;                           /* table index                  */

  if ((vpd_ctl.path[cpath].p_name != (char *) NULL) &&
      (strcmp(vpd_ctl.path[cpath].p_name,name) == 0 ))
    { return (VPD_OK) ; } ;             /* if name is not changing,     */
                                        /* return no error              */

  vpdresclr(cpath) ;			/* clear id/name cache table	*/
  for (tix=0; tix < N_tbls ; tix++ )
    {                                   /* check each table on the old  */
                                        /* path and close any open ones */
    if (vpd_ctl.path[cpath].table[tix].t_open == TRUE)
      {
      if (odm_close_class(vpd_ctl.path[cpath].table[tix].t_class) == -1)
        { return(VPD_SYS) ; }
      vpd_ctl.path[cpath].table[tix].t_open = FALSE ;
      } /* endif */
    } /* endfor */

  if (vpd_ctl.path[cpath].p_name != (char *)NULL)
    { free(vpd_ctl.path[cpath].p_name);}/* free any prior name area     */

  vpd_ctl.path[cpath].p_name = (char *) malloc (strlen(name)+1);
                                        /* allocate storage for new name*/

  if (vpd_ctl.path[cpath].p_name == (char *)NULL )
    { return(VPD_SYS) ; }               /* error if no space available  */

  strcpy(vpd_ctl.path[cpath].p_name, name) ;
                                        /* copy name to new space       */
  return(VPD_OK) ;                      /* return ok status             */

}

