static char sccsid[] = "@(#)16  1.1  src/bos/diag/tu/ppckbd/exectu.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:31:18";
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: exectu
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*                                                                         */
/*       If the mfg mode in the tu control block (tucb) is set to be       */
/*       invoked by HTX then TU program will look at variables in tu       */
/*       control block for values from the rule file. Else, TU program     */
/*       uses the predefined values.                                       */
/*                                                                         */
/***************************************************************************/

#include "tu_type.h"          /* This also includes hxihtx.h */

int exectu(int fdes, TUTYPE *tucb_ptr)
{
  register i, loop, tu;  /* Loop Index */
  int rc = SUCCESS;      /* return code */
  TUTYPE tmp_tucb;
  extern int tu10();
  extern int tu20();
  extern int tu30();
  extern int tu40();

  struct htx_data *htx_sp;

  /*
   * Make copy of tucb_ptr and set up with tu number to perform a HALT.
   * We do this in case a TU fails.
   */

  tmp_tucb = *tucb_ptr;
#ifdef nodiag
  tmp_tucb.keybd_s.htx_sp = NULL;
#endif
  tu = tucb_ptr->header.tu;
  loop = tucb_ptr->header.loop;
#ifdef nodiag
  if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
        tucb_ptr->keybd_s.htx_sp = NULL;

  htx_sp = tucb_ptr->keybd_s.htx_sp;
#endif

  for (i=0; i < loop; i++)
     {
        switch(tu)
           {
              case 0x10:
                    if ((rc = tu10(fdes,tucb_ptr)) != SUCCESS)
                       {
                          rel_sem();
			  if (kbdtufd != FAILURE)
                          	close(kbdtufd);
                       }
#ifdef nodiag
                    if(rc)
                          htx_sp->bad_others++;
                    else
                          htx_sp->good_others++;
#endif
                    break;

              case 0x20:
                    if ((rc = tu20(fdes,tucb_ptr)) != SUCCESS)
                       {
                          rel_sem();
			  if (kbdtufd != FAILURE)
                          	close(kbdtufd);
                       }
#ifdef nodiag
                    if (rc)
                          htx_sp->bad_others++;
                    else
                          htx_sp->good_others++;
#endif
                    break;

       /*
        * Test Units 30,40 are interactive therefore not
        * to be executed under HTX
        */

              case 0x30:
                    if ((rc = tu30(fdes,tucb_ptr)) != SUCCESS)
                       {
                       rel_sem();
		       if (kbdtufd != FAILURE)
                       	     close(kbdtufd);
                       }
                    break;

              case 0x40:
                    if ((rc = tu40(fdes,tucb_ptr)) != SUCCESS)
                       {
                       rel_sem();
		       if (kbdtufd != FAILURE)
                             close(kbdtufd);
                       }
                    break;

              default:
                    return(BAD_TU_NO);

           };  /* end case */

        if (rc)
           {

           /*
            * if running manuf. diagnostic and a tu returns
            * an error, then break out and return.
            */

              if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
                    break;

#ifdef nodiag
           /* check on retries keyword.  If set
            * from rule file stanza, then
            * continue loop to retry tu, else
            * break out and return.
            */

              if (!(*tucb_ptr->keybd_s.retries))
                    break;
#endif
           }
#ifdef nodiag
        if (tucb_ptr->header.mfg == INVOKED_BY_HTX)
              hxfupdate(UPDATE, tucb_ptr->keybd_s.htx_sp);
#endif
     }
  return(rc);
}
/* End function */
