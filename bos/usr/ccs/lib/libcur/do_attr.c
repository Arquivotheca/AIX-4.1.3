static char sccsid[] = "@(#)67  1.10  src/bos/usr/ccs/lib/libcur/do_attr.c, libcur, bos411, 9428A410j 12/16/92 16:33:08";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: chg_attr_mode, putout
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"


static char hold_str[200];

/*
 * NAME:                chg_attr_mode
 *
 * FUNCTION: This routine does the actual changing of
 *      the various attribute modes on the physical device (glass).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      chg_attr_mode(pp, pf), where 'pp' is the present attribute
 *      pattern and 'pf' is the desired (future) attribute pattern.  The
 *      patterns are the same masks which are normally stored in _csbp.
 *
 * EXTERNAL REFERENCES: _puts(), mvcur(), strcat()
 *
 * DATA STRUCTURES:     struct attr_mask
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

chg_attr_mode(pp, pf)
register    ATTR pp,
            pf;
{
    register int    i,
                    tmp;
    register    ATTR wp1,
                wp2;

    extern int  mask_cnt;
    extern struct attr_mask MASK[];
    extern short    _ly,
                    _lx;

    if ((pp & (~pf) & sw_mask) != 0) {/* if change resets any switch */
				   /* - type attributes           */
	_puts(ME)               /* reset all switch attributes */
	if (SG)
	    mvcur(_ly, _lx + SG, _ly, _lx);
				/* if needed reset cursor pos  */
	pp &= (~sw_mask);	/* reflect all switch attr rest */
    }

    if (pf != NORMAL)
	curscr->_flags = _STANDOUT;
				/* if not normal mode set flag */

/*
 * We now go through the stored masks and send the strings necessary
 * to get into the mode we desire.
 */
    for (i = 0; i < mask_cnt; i++) {
	wp1 = pf & MASK[i].act_attr;/* what we want on this attr   */
	wp2 = pp & MASK[i].act_attr;/* what we have on this attr   */
	if (wp1 != wp2) {	/* if there is a change here   */
	    switch (MASK[i].type_attr) {/* based on type of attribute  */
		case 1: 	/* switch type                 */
		    if (wp1 != 0)/* turn on this attribute      */
			putout(MASK[i].set[0]);
				/* set off covered above by    */
				/* - the _puts of ME           */
		    break;
		default: 	/* one of the set types attr.  */
		    if ((tmp = (int) wp1) < 0)
			tmp += 256;
				/* consider case of high bit on */
		    tmp = tmp >> MASK[i].start_bit;
				/* convert mask to index       */
		    putout(MASK[i].set[tmp]);
				/* put out the string to set   */
		    break;
	    }			/* end:  switch (MASK...       */
	}			/* end:  if any change for mask */
    }				/* end:  for (i = 0;...        */

    return OK;
}				/* end:  chg_attr_mode()       */



/*
 * NAME:                putout
 *
 * FUNCTION: send the indicated string and if needed reposition the cursor
 *      based on the terminal attribute SG
 */

putout(str)
register char  *str;
{
    extern short    _ly,
                    _lx;

    _puts(str)
	if (SG)
	mvcur(_ly, _lx + SG, _ly, _lx);

}
