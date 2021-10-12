/* @(#)60	1.10  src/bos/usr/include/cur05.h, libcurses, bos411, 9428A410j 5/14/91 17:17:56 */
#ifndef _H_CUR05
#define _H_CUR05

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur05.h
 *
 * ORIGINS: 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

				/* see cur01.h for NLSCHAR      */
#define    NLSCHAR    wchar_t


#define PANEL	struct Panel
#define PANE	struct Pane
#define PANEPS	struct Paneps


struct Panel {                  /* panel header structure for */

				/* description of glass area */
    short int   p_depth;	/* number of rows in panel   */
    short int   p_width;	/* number of columns in panel */

    short int   orow;		/* origin row (top left)     */
    short int   ocol;		/* origin column	     */

    char   *title;		/* title string pointer      */
    char    divty;		/* divide type code	     */
    char    bordr;		/* border flag byte	     */

    char    __FILL1;		/* filler                    */
    char    __FILL2;		/* filler                    */

				/* the following fields are  */
				/* -- used to relate multiple */
				/* -- panels on the display  */

    PANEL   *p_under;           /* next panel in chain       */
				/* -- 'under' this panel     */
    PANEL   *p_over;            /* previous panel in chain   */
				/* -- 'over' this panel      */

				/* fields used by ecurses    */
				/* -- others use with caution */

    PANE    *fpane;             /* first pane after divisions */
    PANE    *dpane;             /* first root pane for div   */
    PANE    *apane;             /* current active pane       */
    WINDOW  *p_win;              /* window struct for panel   */
    int     dfid;		/* external panel ident      */

    char    plobsc;		/* panel obscured flag       */
    char    plmodf;		/* panel modified flag       */

    char    PLfill[6];		/* Filler to ease expansion  */
};


struct Pane {			/* Structure for a pane defn. */
				/* define presentation space */
    short int   w_depth;	/* rows of data for pane     */
    short int   w_width;	/* columns of data for pane  */

				/* define glass area includes */
				/* -- space for borders      */
    short int   v_depth;	/* rows being shown of pane  */
    short int   v_width;	/* columns being shown of pn */
    short int   orow;		/* top row on panel of view  */
				/* -- for this pane         */
    short int   ocol;		/* first col on panel of view */
				/* -- for this pane         */
    PANE    *vscr;              /* pane to scroll vertically */
				/* -- with this pane        */
    PANE    *hscr;              /* pane to scroll horizon.   */
				/* -- with this pane        */
    PANE    *nxtpn;             /* next pane in chain        */
    PANE    *prvpn;             /* previous pane in chain    */
    PANE    *divs;              /* next pane that is part of */
				/* -- current division spec  */
    PANE    *divd;              /* start of division of this */
				/* -- pane into smaller parts */
    char    divty;		/* division type code	     */
				/* -- applies to divisions of */
				/* -- this pane may have     */
				/* -- values below          */

    char    __FILL1;		/* filler                    */

    short int   divsz;		/* division size spec.	     */
    char    divszu;		/* division size unit spec.  */
				/* -- code indicate form for */
				/* -- divsz value (absolute  */
				/* -- fraction or fill)      */
    char    bordr;		/* border flag for this pane */

    char    __FILL2;		/* filler                    */
    char    __FILL3;		/* filler                    */

    WINDOW  *w_win;             /* pointer to pspace window  */
    WINDOW  *v_win;             /* pointer to view window    */
    int     pnvsid;		/* external ident for this   */
				/* -- p-pspace and view      */
    PANEL   *hpanl;             /* pointer to panel which    */
				/* -- contains this pane     */
    PANEPS  *exps;              /* ptr to chain of extra     */
				/* -- p-spaces for this pane */
    char    alloc;		/* flag - allocation of      */
				/* - w_win for this panel    */
				/* - indicates if ecdfpl did */
				/* - the allocate and thus if */
				/* - ecrlpl should free it   */

    char    pnobsc;		/* pane obscured flag        */
    char    pnmodf;		/* pane modified flag        */

    char    PNfill[5];		/* Filler to ease expansion  */
};

				/* valid codes for divty     */
#define Pdivtyv '0'		/* divide vertical dimension */
				/* -- of this pane,         */
				/* -- divisions will appear  */
				/* -- above each other      */
#define Pdivtyh '1'		/* divide horizontal dim     */
				/* -- of this pane,         */
				/* -- divisions will appear  */
				/* -- alongside each other.  */

				/* valid codes for divszu    */
#define Pdivszc '1'		/* size is fixed constant    */
#define Pdivszp '2'		/* size is proportional value */
#define Pdivszf '0'		/* size is float, ignore val */
				/* fixed constant must be in */
				/* -- range 1 to dimension   */
				/* -- being divided.        */
				/* proportional value must be */
				/* -- in range 1 - 10,000 and */
				/* -- represents numerator of */
				/* -- fraction to be given   */
				/* -- pane with denominator  */
				/* -- equal to 10,000       */
				/* float indicates an equal  */
				/* -- share with other float */
				/* -- panes of space avail   */
#define Pbordry '1'		/* pane has border           */
#define Pbordrn '0'		/* pane has no border        */

#define Pallocy 'y'		/* ecdfpl allocated w_win for */
				/* - this panel              */
#define Pallocn 'n'		/* ecdfpl did not allocate   */
				/* - w_win for this pane     */

#define Pmodfy  'y'		/* pane/panel modified       */
#define Pmodfn  'n'		/* pane/panel not modified   */

#define Pobscy  'y'		/* pane/panel obscured       */
#define Pobscn  'n'		/* pane/panel not obscured   */

struct Paneps {                 /* link structure for pspace added
				   to a pane	     */
    WINDOW  *extps;             /* pointer to added p-space  */
    short int   expvsid;	/* assigned id for the pspace */
    PANEPS  *extnxt;            /* next such structure       */
    PANEPS  *extprv;            /* previous such structure   */
};


extern
	PANEL   *_toppl;        /* ptr to top panel for display  */

extern
	PANEL   *_botpl;        /* bottom panel for display      */


/*      The following four externs are set to represent the resolved    */
/*      location for the locator cursor when the select button is       */
/*      pressed.                                                        */

extern
	PANEL   *LC_PNL;        /* panel containing locator     */

extern
	PANE    *LC_PANE;       /* pane in LC_PNL containing loc */

extern
int     LC_ROW;			/* row in LC_PAN for locator    */

extern
int     LC_COL;			/* column in LC_PAN for locator */

/*      externs used for global switches and status flags               */

extern
int     ECRFCLR;		/* flag to ecrfpl, clear needed? */

extern
        NLSCHAR ECFILLC;	/* character used to fill the   */
				/* - display for a field when   */
				/* - a character is deleted     */

extern
char    ECINSMD;		/* Insert mode for field input  */

/*      function declarations for those functions which do not          */
/*      return an integer value                                         */

PANEL   *ecbpls();              /* build panel structure        */
PANE    *ecbpns();              /* build pane structure         */
WINDOW  *ecblks();              /* blank window structure       */

/*      codes for second parameter to ecrfpl                            */

#define PLRFCLR 0		/* clear display and refresh    */
				/* - with all panels from arg to */
				/* - top panel, use after change */
				/* - to set of panels presented */
#define PLRFNCL 1		/* do not clear display, do refr */
				/* - with all panels from art to */
				/* - top panel, use after change */
				/* - to a panel content but not */
				/* - the set of panels displayed */
#define PLRFSTD 2		/* move updated panel to stdscr */
				/* - do not update display.     */
				/* - move only argument panel.  */
				/* - special purpose function   */



/*********************************************************************/
/*                                                                   */
/*      The following defines provide pseudo functions which will    */
/*      update the p-space for a pane and also set the flag          */
/*      in the panel block indicating that the panel has been        */
/*      modified.                                                    */
/*                                                                   */
/*********************************************************************/

#define ecpnmodf(p)         (((p)->hpanl)->plmodf = Pmodfy )

#define paddch(p,c)         (ecpnmodf(p),waddch((p)->w_win,(c)))
#define mvpaddch(p,y,x,c)   (ecpnmodf(p),mvwaddch((p)->w_win,(y),(x),(c)))
#define paddstr(p,s)        (ecpnmodf(p),waddstr((p)->w_win,(s)))
#define mvpaddstr(p,y,x,s)  (ecpnmodf(p),mvwaddstr((p)->w_win,(y),(x),(s)))
#define pchgat(p,n,m)       (ecpnmodf(p),wchgat((p)->w_win,(n),(m)))
#define mvpcggat(p,y,x,n,m) (ecpnmodf(p),mvwchgat((p)->w_win,(y),(x),(n),(m)))
#define mvpchgat(p,y,x,n,m) (ecpnmodf(p),mvwchgat((p)->w_win,(y),(x),(n),(m)))
#define perase(p)           (ecpnmodf(p) ,werase((p)->w_win))

#endif				/* _H_CUR05 */
