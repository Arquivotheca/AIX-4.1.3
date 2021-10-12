#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)54  1.1  src/bos/usr/ccs/lib/libcurses/curs_getchtype.c, libcurses, bos411, 9428A410j 2/9/94 12:52:30";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: getchtype
 *              setchtype
 *              add_ch
 *              wadd_ch
 *              mvadd_ch
 *              mvwadd_ch
 *              echo_char
 *              wecho_char
 *              box_set
 *              in_ch
 *              win_ch
 *              mvin_ch
 *              mvwin_ch
 *              ins_ch
 *              wins_ch
 *              mvins_ch
 *              mvwins_ch
 *              pecho_char
 *              un_ctrl
 *
 *   ORIGINS: 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#endif /* _POWER_PROLOG_ */

#include <curses.h>
#include <errno.h>

int getchtype(const chtype *cval, char *ch, attr_t *attrs, short *color)
{

     if ( cval == NULL )
       {
           errno = EINVAL;
           return(ERR);                   /* Handle the error condition  */
       }

     /* Fill in the pointers if they are not NULL. All paramaters except */
     /* cval may be NULL.                                                */

     if ( attrs != NULL ) *attrs = (*cval & A_ATTRIBUTES) & ~A_COLOR;
     if ( color != NULL ) *color = PAIR_NUMBER((*cval & A_ATTRIBUTES));
     if ( ch != NULL ) *ch = *cval & A_CHARTEXT;

     return(OK);
}

int setchtype(chtype *cval, char ch, attr_t attrs, short color)
{

     if ( cval == NULL )
       {
           errno = EINVAL;
           return(ERR);                   /* Handle the error condition  */
       }

     *cval = (COLOR_PAIR(color) & A_COLOR) |
             (attrs & A_ATTRIBUTES) |
             (ch & 0xff);

     return(OK);
}


/*
 *
 * glue code for SPEC 1170 curses interfaces
 *
 */

/*
 * curs_addch routines
 */

int add_ch(chtype *ch)
{
     return(addch(*ch));
}

int wadd_ch(WINDOW *win, chtype *ch)
{
     return(waddch(win,*ch));
}

int mvadd_ch(int y, int x, chtype *ch)
{
     return(mvaddch(y,x,*ch));
}

int mvwadd_ch(WINDOW *win, int y, int x, chtype *ch)
{
     return(mvwaddch(win,y,x,*ch));
}

int echo_char(chtype *ch)
{
     return(echochar(*ch));
}

int wecho_char(WINDOW *win, chtype *ch)
{
     return(wechochar(win,*ch));
}

/*
 * curs_border routines
 */

int box_set(WINDOW *win, chtype *verch, chtype *horch)
{
     return(box(win,*verch,*horch));
}

/*
 * curs_inch routines
 */

int in_ch(chtype *cval)
{
    *cval = inch();
    return(OK);
}

int win_ch(WINDOW *win, chtype *cval)
{
    *cval = winch(win);
    return(OK);
}

int mvin_ch(int y, int x, chtype *cval)
{
    *cval = mvinch(y,x);
    return(OK);
}

int mvwin_ch(WINDOW *win, int y, int x, chtype *cval)
{
    *cval = mvwinch(win,y,x);
    return(OK);
}

/*
 * curs_insch routines
 */

int ins_ch(chtype *ch)
{
    return(insch(*ch));
}

int wins_ch(WINDOW *win, chtype *ch)
{
    return(winsch(win,*ch));
}

int mvins_ch(int y, int x, chtype *ch)
{
    return(mvinsch(y,x,*ch));
}

int mvwins_ch(WINDOW *win, int y, int x, chtype *ch)
{
    return(mvwinsch(win,y,x,*ch));
}

/*
 * curs_pad routines
 */

int pecho_char(WINDOW *pad, chtype *ch)
{
    return(pechochar(pad,*ch));
}

/*
 * curs_util routines
 */

char *un_ctrl(chtype *ch)
{
    return(unctrl(*ch));
}

