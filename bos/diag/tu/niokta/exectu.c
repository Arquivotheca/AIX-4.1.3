/* static char sccsid[] = "@(#)42  1.2  src/bos/diag/tu/niokta/exectu.c, tu_adpkbd, bos411, 9431A411a 7/12/94 16:03:33"; */
/*
 * COMPONENT_NAME: tu_adpkbd
 *
 * FUNCTIONS: exectu
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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
	
    /*
     * Make copy of tucb_ptr and set up with tu number to perform a HALT.
     * We do this in case a TU fails.
     */
    
    tmp_tucb = *tucb_ptr;
    tu = tucb_ptr->header.tu;
    loop = tucb_ptr->header.loop;

    for (i=0; i < loop; i++)
    {
        switch(tu)
        {
        case 0x10:
            rc = tu10(fdes,tucb_ptr);
            break;
            
        case 0x20:
	    rc = tu20(fdes,tucb_ptr);
            break;
            
        case 0x30:
 	    rc = tu30(fdes,tucb_ptr);
            break;

	case 0x40:
	    rc = tu40(fdes,tucb_ptr);
	    break;
            
        default :
            return(BAD_TU_NO);
            
        };  /* end case */
  
     }   
    return(rc);
}
/* End function */

