/* @(#) 02	1.15 src/bos/usr/ccs/lib/libswvpd/vpdinit.c, libswvpd, bos411, 9428A410j 6/20/94 12:54:34 */
/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdinit
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

#include <swvpd0.h>                     /* SWVPD internal includes      */

/*---------------------------------------------------------------------------
 * NAME: vpdinit
 *
 * FUNCTION: Initialization for SWVPD access.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Will perform needed initialization for SWVPD access routines
 *      This includes ODM initialization, odm path initialization,
 *      and ODM file open/creation.
 *
 *
 * (DATA STRUCTURES:)
 *      Relies on the information in the vpd_ctl structure for the
 *      current status of odm, and the path information.  That structure
 *      will be updated to reflect any actions taken by this routine.
 *
 * RETURNS:
 *      VPD_OK          all initialization completed normally
 *      VPD_SYS         one or more errors detected during initialization
 *---------------------------------------------------------------------------*/

int vpdinit (tbl_id)
   int   tbl_id;         /* table index to open */
{
   int   rc;             /* hold return code    */

   if ( ! vpd_ctl.odminit)
   {
      rc = odm_initialize();               /* initialize odm structures    */
      if (rc == -1  &&  odmerrno != ODMI_INVALID_PATH)
         return (VPD_SYS);                 /* ignore invalid path here     */
     vpd_ctl.odminit = TRUE;               /* odm initialization done ok   */
   }

   if (vpd_ctl.path[vpd_ctl.cur_path].p_name == (char *) NULL)
   {                                       /* if path name not set yet     */
      if (vpd_ctl.cur_path == VPD_LOCAL)
         vpdlocalpath (NULL);              /* use defaults for the path    */
      else
         vpdremotepath (NULL);
   }

   if (T_OPEN (tbl_id) != TRUE)            /* if the table is not open     */
   {
      char *o_path;

      /* ensure the path is set */
      o_path = odm_set_path (vpd_ctl.path[vpd_ctl.cur_path].p_name);
      if ((int)o_path == -1)
         return(VPD_SYS);

      if (o_path != (char *) NULL)
         free (o_path);

      /* if open fails because table did not exist then create table */
      if ((int)odm_open_class (TABLE(tbl_id)) == -1)
      {                                  
         if (odmerrno == ODMI_CLASS_DNE)   
         {                              
            if (odm_create_class (TABLE(tbl_id)) == -1)
               return(VPD_SYS);
         }
         else
            return (VPD_SYS);            /* if open failed return err    */
      }

      T_OPEN (tbl_id) = TRUE;            /* have successfully opened tbl */
   }

   return (VPD_OK);

} /* vpdinit */


/*-----------------------------------------------------------------------------
 * NAME: vpdterm
 *
 * FUNCTION: Close all the active SWVPD data base tables.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      For each table, if that table is currently initialized and open
 *      close that table and reset the open flag.
 *
 * RETURNS:
 *      VPD_OK          if no errors
 *      VPD_SYS         if odm or system routine error
 *----------------------------------------------------------------------------*/

int vpdterm ()
{
  int   cpath;                         /* index for path to work on    */
  int   tix;                           /* table index                  */

  for (cpath = VPD_LOCAL;              /* check both of the paths      */
       cpath <= VPD_REMOTE;
       cpath++ )
  {
      /* check each table on the old path and close any open ones */
      for (tix=0; tix < N_tbls; tix++ )
      {
         if (vpd_ctl.path[cpath].table[tix].t_open == TRUE)
         {
            if (odm_close_class(vpd_ctl.path[cpath].table[tix].t_class) == -1)
               return(VPD_SYS);

            vpd_ctl.path[cpath].table[tix].t_open = FALSE;
         }
      }
   }

   return (VPD_OK);

} /* vpdterm */
