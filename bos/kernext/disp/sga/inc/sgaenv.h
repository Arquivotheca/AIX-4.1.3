/* @(#)20       1.2.1.2  src/bos/kernext/disp/sga/inc/sgaenv.h, sgadd, bos411, 9428A410j 10/29/93 10:46:33 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _SGA_ENV
#define _SGA_ENV

struct vttenv {

/**********************************************************************/
/*                                                                    */
/* Virtual Driver Mode: Current mode of the Virtual Device Driver     */
/*                             0 => monitored mode                    */
/*                             1 => charaacter mode (KSR)             */
/*                             2 => APA mode (AVT)                    */
/*                                                                    */
/*                      NOTE: the current state of the real device    */
/*                            (rscr_mode) is stored in the RMA.       */
/*                                                                    */
/*                      NOTE: the default mode is character           */
/*                                                                    */
/**********************************************************************/

    unsigned int        vtt_mode;

/**********************************************************************/
/*                                                                    */
/* Virtual Driver State: Current state of the Virtual Display Driver  */
/*                             0 => inactive (presentation space      */
/*                                            updated)                */
/*                             1 => active (frame buffer updated)     */
/*                                                                    */
/*                        NOTE: the default state is inactive.        */
/*                                                                    */
/**********************************************************************/


    unsigned int        vtt_active;


/**********************************************************************/
/*                                                                    */
/* Virtual Terminal ID: Index into the RMA's virtual terminal table.  */
/*                      This value is passed to the VDD when the      */
/*                      Initialize command is called.                 */
/*                                                                    */
/**********************************************************************/

    short               vt_id ;

/**********************************************************************/
/*                                                                    */
/* Real Screen ID: Index into the RMA's real screen table.            */
/*                 This value is calculated by the Initialize         */
/*                 command (the virtual terminal table contains the   */
/*                 id of the currently selected display).             */
/**********************************************************************/

    short               rscr_id  ;


/**********************************************************************/
/*                                                                    */
/*  Presentation Space: Matches the CGA Color A/N Frame Buffer        */
/*                                                                    */
/*                      NOTE: the character in the presentation space */
/*                            is initialized as a "space" with a      */
/*                            "white character/black background"      */
/*                            attribute.                              */
/*                                                                    */
/**********************************************************************/

    ulong               *pse ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Presentation Space Size (bytes)                                   */
/*                                                                    */
/*              Contains the total number of bytes in the ps .        */
/*              (width = height = -1 implies the ps is not allocated).*/
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*  Presentation Space Size (full characters)                         */
/*                                                                    */
/*              Contains the current width and height of the ps       */
/*              (width = height = -1 implies the ps is not allocated) */
/*              in characters.                                        */
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

   int ps_bytes;                     /* number of bytes in ps         */
   int ps_words;                     /* number of words in ps         */

   struct {
        short ht;                    /* ps height (row)               */
        short wd;                    /* ps width (in font_width)      */
   } ps_size;                        /* dimensions of ps              */

   short rowlen[128];                /* text length on each row       */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Font Table: Contains 8 elements. The font selector in VTT_ATTR    */
/*              (canonical attribute specified in VTTCMD) indexes     */
/*              this table.                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/

     struct {
         ushort id;                   /* font id                       */
         int  index;                  /* index into rma                */
         char *addr;                  /* segment 0 offset to font      */
         short height,width;
         long size;                   /* size in bytes of the font     */
     } font_table[8];                 /* font selector                 */


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Cursor Attributes:                                                 */
/*                                                                    */
/*              Characteristics of the cursor prior  to the           */
/*              execution of the next VDD command.                    */
/*                                                                    */
/*              NOTE: if the virtural terminal is active, this        */
/*              field contains the attributes of the cursor that      */
/*              is currently being displayed.                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
        unsigned short height;          /* height of character (pels)    */
        unsigned short width;           /* width of character (pels)     */
    } character_box;                    /* dimensions of character box    */

    struct {
        unsigned long  lastmalloc;      /* size of last malloced cursor data*/
        char           *data;           /* pointer to malloced data area  */
        char           *ichar;          /* pointer to storage for the     */
                                        /* cursor inverted character      */
                                        /*   (see change_cursor_shape() ) */
                                        /*   (see draw_char()           ) */
    } cursor;                           /* software cursor related info   */

    struct {
        unsigned short fg;                 /* cursor foreground color       */
        unsigned short bg;                 /* cursor background color       */
    } cursor_color;                     /* cursor color                  */

/**********************************************************************/
/*                                                                    */
/* Cursor Shape: width and height of the cursor shape                 */
/*               (width of (0)  means the cursor is not visible).     */
/*                                                                    */
/*               NOTE: the default value is a DOUBLE underscore       */
/*                                                                    */
/* Cursor Select: Invisible            = 0       Single Underscore=1  */
/*                Double Underscore    = 2       Half Blob        =3  */
/*                Mid Char Double Line = 4       Full Blob        =5  */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
#define INVISIBLE   0
#define UNDERSCORE1 1
#define UNDERSCORE2 2
#define HALF_BLOB   3
#define MID_SCORE_2 4
#define FULL_BLOB   5

    struct cursr_shape {
         ushort   cursor_top  ;
         ushort   cursor_bot  ;
    } cursor_shape ;

    int cursor_select;
    int cursor_show;


/**********************************************************************/
/*                                                                    */
/* Cursor Position: character offset into the frame buffer            */
/*                  or index into the presentation space              */
/*                  ( ((row-1) * 80) + col-1 )                        */
/*                                                                    */
/*                  NOTE: the default value is 0 (the upper-left      */
/*                        corner of the screen)                       */
/*                                                                    */
/**********************************************************************/

    struct cursr_pos {
        int                 cursor_col  ;
        int                 cursor_row  ;
    } cursor_pos ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Screen position of top left of TEXT portion in pels.               */
/*                                                                    */
/*--------------------------------------------------------------------*/

     struct {
          short pel_x;                  /* pel indentation to tl of     */
                                        /* TEXT portion of screen.      */
          short pel_y;                  /* pel indentation to tl of     */
     } scr_pos;                         /* TEXT portion of screen.      */

    long font_box_width;              /* Holds alignment of font Glyphs*/

    ushort  current_attr;           /* four byte attribute          */

};      /* END of vttenv structure */


#endif /* _SGA_ENV */

