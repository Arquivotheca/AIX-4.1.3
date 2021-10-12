/* @(#)15       1.21.2.3  src/bos/kernext/lft/inc/vt.h, lftdd, bos411, 9428A410j 7/5/94 11:30:04 */
/*
 *
 * COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/font.h>

/* ------------------------------------------------------------------------ */
/* presentation space structure						    */
/* ------------------------------------------------------------------------ */

struct ps_s {
        long ps_w;                      /* presentation space width */
        long ps_h;                      /* presentation space height */
};

/* ------------------------------------------------------------------------ */
/* The vtt_box_rc_parms structure is supplied as a parameter to the	    */
/* VDD clear rectangle routine						    */
/* ------------------------------------------------------------------------ */

struct vtt_box_rc_parms {
        long row_ul;                    /* row number of upper-left corner */
                                        /*   of the upright rectangle      */
        long column_ul;                 /* col number of upper-left corner */
                                        /*   of the upright rectangle      */
        long row_lr;                    /* row number of lower-right corner */
                                        /*   of the upright rectangle      */
        long column_lr;                 /* col number of lower-right corner */
                                        /*   of the upright rectangle      */
};

/* ------------------------------------------------------------------------ */
/* Color palette structure.  Define data sent to VDD			    */
/* ------------------------------------------------------------------------ */

#define COLORPAL        16

struct colorpal {
        short numcolors ;               /* # of palette entries */
        int   rgbval[COLORPAL];         /* adapter-specific value settings 
                                           for color palette entry */
};

/* ------------------------------------------------------------------------ */
/* Font palette structure.  Define data sent to VDD			    */
/* ------------------------------------------------------------------------ */

#define FONTPAL         8
struct fontpal {
        short  font_index[FONTPAL];       /* index into font.h structure  */
};

/* ------------------------------------------------------------------------ */
/* row column length structure interfaces to VDD Routine		    */
/* ------------------------------------------------------------------------ */

struct vtt_rc_parms {
        ulong string_length;            /* length of char string that 	    */
                                        /*   must be displayed 	            */
        ulong string_index;             /* index of the 1st char to display */
        long start_row;                 /* starting row for draw/move       */
                                        /*   operations, unity based        */
        long start_column;              /* starting column for draw/move    */
                                        /*   operations, unity based        */
        long dest_row;                  /* destination row number for move  */
                                        /*   operations, zero based         */
        long dest_column;               /* destination column number for move*/
                                        /*   operations, zero based         */
};

/* ------------------------------------------------------------------------ */
/* cursor positioning structure used as parameter to VDD routine	    */
/* ------------------------------------------------------------------------ */

struct vtt_cursor {
        long                    x;
        long                    y;
};

/* ------------------------------------------------------------------------ */
/* code point base/mask and attribute parameters			    */
/* ------------------------------------------------------------------------ */

struct vtt_cp_parms {
        ulong                   cp_mask;	/* code point mask for 	    */
						/* implementing 7 or 8 bit  */
						/* ascii		    */
        long                    cp_base;	/* code point base, added to*/
						/* code point if base >=0   */
        ushort                  attributes;	/*  attribute bits	    */
        struct vtt_cursor       cursor;		/* cursor x, y position     */
};

/* ------------------------------------------------------------------------ */
/*                          masks for attributes field			    */
/* ------------------------------------------------------------------------ */

#define FG_COLOR_MASK    0xf000
#define BG_COLOR_MASK    0x0f00
#define FONT_SELECT_MASK 0x00e0
#define NO_DISP_MASK     0x0010           /* non displayable char attr */
#define BRIGHT_MASK      0x0008           /* bright char attr */
#define BLINK_MASK       0x0004           /* blink char attr */
#define REV_VIDEO_MASK   0x0002           /* reverse video char attr */
#define UNDERSCORE_MASK  0x0001           /* underscore char attr */

#define REV_VIDEO_MASK   0x0002           /* reverse video char attr */
#define UNDERSCORE_MASK  0x0001           /* underscore char attr */
#define ATTRIBUTES_INITIAL 0x2000         /* */

/* ------------------------------------------------------------------------ */
/* Define rvalue macros used to access bit-oriented fields in "attribute"   */
/* ------------------------------------------------------------------------ */

#define ATTRIBRI(attribute)             ((attribute) & BRIGHT_MASK)
#define ATTRIUNSC(attribute)            ((attribute) & UNDERSCORE_MASK)
#define ATTRIBLI(attribute)             ((attribute) & BLINK_MASK)
#define ATTRIRV(attribute)              ((attribute) & REV_VIDEO_MASK)
#define ATTRIBLA(attribute)             ((attribute) & NO_DISP_MASK)
#define ATTRIBAKCOL(attribute)          (((attribute) >> 8) & 0x000f)
#define ATTRIFORECOL(attribute)         (((attribute) >> 12) & 0x000f)
#define ATTRIFONT(attribute)            (((attribute) >> 5) & 0x0007)

/* ------------------------------------------------------------------------ */
/* Define rvalue macros used to set multibit fields in "attribute":	    */
/* ------------------------------------------------------------------------ */
#define SET_ATTRIFONT(attribute, value) attribute = \
                      (((attribute) & ~FONT_SELECT_MASK) | ((value) << 5))
#define SET_ATTRIFORECOL(attribute, value) attribute = \
                      (((attribute) & ~FG_COLOR_MASK) | ((value) << 12))
#define SET_ATTRIBAKCOL(attribute, value) attribute = \
                      (((attribute) & ~BG_COLOR_MASK) | ((value) << 8))

/* ------------------------------------------------------------------------ */
/* The vtm structure and some of the elements in the structure have been    */
/* included to allow for backward compatibility with existing display device*/
/* drivers. This structure contains device dependent information for a given*/
/* display adapter.  vtm structures are allocated and initialized by the    */
/* lft device drivers.  Each available display has a corresponding vtm      */
/* structure.								    */
/* ------------------------------------------------------------------------ */

#define KSR_MODE        	1               /* Keyboard Send/Receive Mode*/
#define GRAPHICS_MODE 		0               /* Monitor Mode              */

struct vtmstruc {
	struct phys_displays	*display;	/* display this vt is using */
	struct vtt_cp_parms	mparms;		/* attribute+cursor position*/
	char			*vttld;		/* pointer to VTT local data*/
	off_t			vtid;		/* virtual terminal id = 0  */
	uchar			vtm_mode;	/* mode = KSR		    */
	int			font_index;	/* -1 means use 'best' font */
	int			number_of_fonts; /* number of fonts found   */
	struct font_data	*fonts;		/* font information         */
	int			(*fsp_enq)();	/* font request enqueue func*/
};

