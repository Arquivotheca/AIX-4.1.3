/* static char sccsid[] = "@(#)55  1.3  src/bos/diag/tu/tablet/exectu.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:19"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS: exectu
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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

#include <stdio.h>
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
    extern int tu50();
    extern int tu60();
    extern int tu70();

#ifdef DIAGNOSTICS
     void setmenunumberbase();

     setmenunumberbase(tucb_ptr->tuenv.tabtype);
#endif

    /* Make copy of tucb_ptr and set up with tu number to perform a
     * HALT. We do this in case a TU fails. */
    
    tmp_tucb = *tucb_ptr;
    tu = tucb_ptr->header.tu;
    loop = tucb_ptr->header.loop;

    for (i=0; i < loop; i++)
    {
        switch(tu)
	{
	case 0x10:
            if ((rc = tu10(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd);
	    break;
	    
	case 0x20:
            if ((rc = tu20(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd);
	    break;
	    
	case 0x30:
            if ((rc = tu30(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd); 
	    break;
	    
	    /* TUs 40, 50, 60, 70 are interactive */
	case 0x40:
	    if ((rc = tu40(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd);
	    break;
	    
	case 0x50:
	    if ((rc = tu50(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd);
	    break;
	    
	case 0x60:
	    if ((rc = tu60(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd); 
	    break;
	    
	case 0x70:
	    if ((rc = tu70(fdes,tucb_ptr)) != SUCCESS)
                close(kbdtufd);
	    break;
	    
	default:
	    return(BAD_TU_NO);
	    
	};  /* end switch(tu) */
	
    }
    return(rc);
}
/* End function */
