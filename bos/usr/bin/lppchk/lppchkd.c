static char sccsid[] = "@(#)38  1.5  src/bos/usr/bin/lppchk/lppchkd.c, cmdswvpd, bos411, 9428A410j 3/31/94 20:44:27";
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
 * NAME: lppchkd (driver)
 *
 * FUNCTION: Drive the set of check requests identified by command line
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Controls the processing of the lppchk operations specified in
 *      the parameter list.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - processing errors found.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */

int lppchkd (
        char    *opt_str ,      /* pointer to string of function letters*/
                                /* f, c, l and/or v                     */
        int     opt_u ,         /* boolean - update database            */
        char    *rep_str ,      /* string of codes for repositories to  */
                                /* be processed                         */
        char    *lppname ,      /* pointer to LPP selection specifier   */
        char    **file_spec ,   /* pointer to first of file specifiers  */
        int     file_count )    /* number of file specifiers            */

  {                             /* begin lppchkd - driver               */

  int   i, j, f ;               /* iteration indexes                    */
  int   rc, mrc ;               /* return code value and max value      */
  char  opt ;                   /* current option letter                */
  char  rep ;                   /* current repository code letter       */

  char  *fileid ;               /* pointer to current file spec         */

  struct lpp_ids idlist ;       /* structure with lpp_ids that match    */

/*----------------------------------------------------------------------*/
  TRC(TRC_ENTRY,"Entry lppchkd - opt_str = %s, file_1 = %s\n",
      opt_str, *file_spec ,0) ;

  idlist.idcnt = 0 ;            /* make idlist structure empty          */
  idlist.ida = NULL ;

  chk_file_cnt = 0 ;		/* initialize count of matching files	*/
  chk_lpp_cnt = 0 ;		/* initialize count of matching LPPs	*/
 
  mrc = rc = 0 ;                /* initialize return code value         */

  for (i=0; i < strlen(opt_str) ; i++ )
    {                           /* step thru the requested options      */
     
    /*
     * The v option is handled differently.  There won't be multiple
     * calls for multiple repositories.  Call the verification routine
     * and exit.  Also, there can never be more than one option specified
     * for this option.
     */ 
    if ((*opt_str) == 'v')
        return (lppchkv(lppname, opt_u));

    opt = *(opt_str+i) ;        /* save selected option                 */
    for (j=0; j < strlen(rep_str) ; j++ )
      {
      rep = *(rep_str+j) ;      /* save code for current repository     */

      switch (rep)
        {
        case 'r' :
          vpdlocalpath(VPD_ROOT_PATH) ;
          break ;
        case 's' :
          vpdlocalpath(VPD_SHARE_PATH) ;
          break ;
        case 'u' :
          vpdlocalpath(VPD_USR_PATH) ;
          break ;
        default :
          MSG_S(MSG_SEVERE,MSG_CHK_BAD_REP,DEF_CHK_BAD_REP,rep,0,0) ;
          return(CHK_BAD_RET) ;
          /*break ;*/
        }                       /* end switch based on repository code  */

      vpdlocal() ;              /* make specified path the active path  */
      if (opt != 'v')           /* if not verify level - build lppid lst*/
        { 
        lppchkbl(lppname, &idlist) ;
        chk_lpp_cnt += idlist.idcnt ;
        }

      for (f=0 ;f < file_count ;f++ )
        {
        fileid = file_spec[f];  /* save pointer to file specifier       */

	TRC(TRC_ALL, "lppchkd next file = %s, first file = %s, n=%d\n",
		fileid, file_spec[0], f) ;

        switch (opt)            /* based on current option char         */
          {
          case 'f' :            /* do file/archive check                */
          case 'c' :
            rc = lppchkf(opt, opt_u , &idlist, fileid );
            break ;
          case 'l':             /* do link checking                     */
            rc = lppchkl(&idlist, fileid, opt_u) ;
            break ;
          default :
            MSG_S(MSG_SEVERE, MSG_CHK_BAD_OPT, DEF_CHK_BAD_OPT, opt,0,0) ;
            return(CHK_BAD_RET);
            /*break ;*/

          }                     /* end switch - which option            */
          if (rc > mrc)         /* save worst case return code          */
            { mrc = rc ; }
        }                       /* endfor - each file specified         */
      }                         /* end for each specified repository    */
    if (chk_lpp_cnt == 0) 	/* if no matching lpp entries found	*/
      {
      MSG_S(MSG_ERROR, MSG_CHK_NOLPP, DEF_CHK_NOLPP, lppname, 0, 0) ;
      mrc += 1 ;
      } 
				/* if no matching LPP entries - warn	*/

    if ((chk_file_cnt == 0) &&	/* if no matching files 		*/
        ( opt != 'v' ))		/* and option is not verify - 		*/
      {
      MSG_S(MSG_ERROR, MSG_CHK_FC0, DEF_CHK_FC0, lppname, 0, 0) ;
      mrc += 1 ;
      }
				/* warn if no files found		*/
    }                           /* end for each command option          */

  TRC(TRC_EXIT,"Exit lppchkd - LPP_cnt = %d, File_cnt = %d\n",
               chk_lpp_cnt, chk_file_cnt, 0) ;

  return(mrc) ;                 /* return worst case return code        */
  }                             /* end lppchkd - driver                 */


