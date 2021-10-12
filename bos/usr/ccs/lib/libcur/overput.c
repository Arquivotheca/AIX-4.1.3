static char sccsid[] = "@(#)11	1.9  src/bos/usr/ccs/lib/libcur/overput.c, libcur, bos411, 9428A410j 7/31/91 10:10:29";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: overput, is_blank
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

wchar_t         nlpic = '@';            /* how to show partial 2-bytes  */
/* This was copied from the old addch.c  */

#define 	min(a,b)	(a < b ? a : b)
#define 	max(a,b)	(a < b ? b : a)

/*
 * NAME:                overput
 *
 * FUNCTION: This routine writes win1 on win2, either
 *      destructively or non-destructively depending on the arguements.
 *      (The implementation is a little involved - beware...).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      overput(win1, win2, destruct), where 'win1' & 'win2' are
 *      pointers to the windows, and 'destruct' is a flag which if TRUE
 *      causes an overwrite to take place, else an overlay to take place.
 *
 * EXTERNAL REFERENCES: mvwaddch()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

overput(win1, win2, destruct)
register    WINDOW  *win1,
		    *win2;
register char   destruct;
{
    register    NLSCHAR *sp,
			*end,
			*dd;
    register    ATTR    *ap,
			*da;
    int     x,
            y,
            startx,
            temp3,
            temp2,
            temp1,
            endy;
    ATTR    asave;
    int     endx,
            x_left,
            starty,
            y_top,
            y_bot,
            x_rite,
            sy,
            sx;
    int     sty2,
            stx2,
            eny2,
            enx2,
            y2;
    int     retval = OK;

 /* save win2's standout mode & current location */
    asave = win2->_csbp;
    getyx(win2, sy, sx);

 /* calculate the amount of overlapping between the windows */
    y_top = max(win1->_begy, win2->_begy);
    y_bot = min((win1->_maxy + win1->_begy - 1),
					(win2->_maxy + win2->_begy - 1));
    x_left = max(win1->_begx, win2->_begx);
    x_rite = min((win1->_maxx + win1->_begx - 1),
					(win2->_maxx + win2->_begx - 1));

    starty = y_top - win1->_begy;
    startx = x_left - win1->_begx;
    endy = y_bot - win1->_begy;
    endx = x_rite - win1->_begx;

    sty2 = y_top - win2->_begy;
    stx2 = x_left - win2->_begx;
    eny2 = y_bot - win2->_begy;
    enx2 = x_rite - win2->_begx;

    if (destruct) {
	for (y = starty, y2 = sty2;/* step through rows needing chg */
		y <= endy; y++, y2++) {
	    end = &(win1->_y[y][endx]);/* end of data to move          */
	    sp = &(win1->_y[y][startx]);
				/* first data to move           */
	    ap = &(win1->_a[y][startx]);
				/* first attr to move           */
	    da = &(win2->_a[y2][stx2]);/* first attr destination       */
	    dd = &(win2->_y[y2][stx2]);/* first data destination       */

	    /* !!! may want to convert two byte blank into one byte blank rather
	       than the partial character indicator in the following code. */

	    /* handle overwrite at start edge of a partial character */
	    if (is_second_byte (dd[0])) {
		 dd[-1] = PART_CHAR;
		 win2->_firstch[y2] = min(win2->_firstch[y2],stx2-1) ;
	    }

	    /* handle overwrite at end edge of a partial character */
	    if (is_first_byte (win2->_y[y2][enx2])) {
		 win2->_y[y2][enx2+1] = PART_CHAR;
		 win2->_lastch[y2] = max(win2->_lastch[y2],enx2+1) ;
	    }

	    /* handle first byte to transfer being a partial character */
	    if (is_second_byte (*sp)) {
		 *dd++ = PART_CHAR;
		 sp++;
		 *da++ = *ap++;
	    }

	    /* handle last byte to transfer being a partial character */
	    if (is_first_byte (*end)) {
		 win2->_y[y2][enx2] = PART_CHAR;
		 win2->_a[y2][enx2] = win1->_a[y][endx];
		 end--;
	    }

	    if (win2->_firstch[y2] == _NOCHANGE) {
		 win2->_firstch[y2] = stx2;
		 win2->_lastch[y2] = enx2;
	    }
	    else {
		win2->_firstch[y2] = min(win2->_firstch[y2], stx2);
		win2->_lastch[y2] = max(win2->_lastch[y2], enx2);
	    /* update change vectors        */
	    }

	    for (; sp <= end;) {/* loop through data to move    */
		*da++ = *ap++;	/* move attribute character     */
		*dd++ = *sp++;	/* move data character          */
	    }

	}
    }else {
	NLSCHAR prev_nlpic = nlpic;
	nlpic = PART_CHAR;      /* for overlay replace partial chars */
				/* with special indicator */

	temp1 = win1->_begy - win2->_begy;
				/* temp(s) added for performance */
	temp2 = x_left - win2->_begx;

	/* for each line they have in "common" space do:  */
	for (y = starty; y <= endy; y++) {
	    end = &(win1->_y[y][endx]);
	    ap = &(win1->_a[y][startx]);
	    sp = &(win1->_y[y][startx]);
	    temp3 = y + temp1;
	    /* for each character in common space, transfer non-blanks */

	    x = 0;
	    /* if at end overwriting 1st byte of a two byte character, replace
	       it by the partial character indicator */
	    if (is_first_byte (win2->_y[temp3][enx2]) &&
		!is_blank(end, win2->_a[temp3][enx2]))
		 if (mvwaddch (win2, temp3, enx2+1, PART_CHAR) == ERR)
		      retval = ERR;

	    /* if at start overwriting 2nd byte of a two byte character, replace
	       it by the partial character indicator */
	    if (is_second_byte (win2->_y[temp3][stx2]) && 
		!is_blank(sp,*ap))
		 if (mvwaddch (win2, temp3, stx2-1, PART_CHAR) == ERR)
		      retval = ERR;

	    /* if last character to transfer is the first byte of a two byte
	       character, use the partial character indicator instead */
	    if (is_first_byte (*end) && 
		!is_blank(end, win1->_y[y][endx])) {
		 if (mvwaddch (win2, temp3, enx2, PART_CHAR) == ERR)
		      retval = ERR;
		 end--;
	    }

	    /* if first character to transfer is second byte of a two byte
	       character, use the partial character indicator instead */
	    if (is_second_byte (*sp) && 
		!is_blank(sp, *ap)) {
		 if (mvwaddch (win2, temp3, temp2, PART_CHAR) == ERR)
		      retval = ERR;
		 x = 1;
		 sp++;
		 ap++;
	    }

	    /* do the overlay of the windows */
	    for (; sp <= end; sp++, ap++, x++)
		if (!is_blank(sp,*ap)) {
		    if (win2->_csbp != *ap) /* use win1's attributes */
			win2->_csbp = *ap;

		    /* overwrite the non-blank character */
		    if (mvwaddch(win2, temp3, x + temp2, *sp) == ERR)
			 retval = ERR;
	       }                           /* end: if (not a space) */
       }			/* end: for y = starty... */

	/* restore win2's standout mode & current location */
	win2->_csbp = asave;

	nlpic = prev_nlpic;
	wmove(win2, sy, sx);
   }
    return retval;
}

/*
 * NAME:		is_blank
 *
 * FUNCTION:	Return whether the indicated character is a portion of a 
 *	normal blank and which direction it extends.
 */

static is_blank(p, a)
NLSCHAR *p;
ATTR a;
{
     register NLSCHAR	ch = *p;
     if(a != NORMAL)
	  return 0;

     if( ch != WEOF )
	  return iswspace( (wint_t)ch );
     else
	  return iswspace( (wint_t)(*(p-1)) );
/*	  
     if(ch & BYTE_MASK)
	  if(is_first_byte (ch))
	       return ((ch == B1BLANK && p[1] == B2BLANK) ? BLANK2R : NOTBLANK);
     else
	  return ((p[-1] == B1BLANK && ch == B2BLANK) ? BLANK2L : NOTBLANK);
     return ((ch == ' ') ? BLANK1 : NOTBLANK);
*/
}

