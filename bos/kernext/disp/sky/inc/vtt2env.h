/* @(#)17	1.7  src/bos/kernext/disp/sky/inc/vtt2env.h, sysxdispsky, bos411, 9428A410j 4/18/94 14:18:22 */
/*
 *   COMPONENT_NAME: SYSXDISPSKY
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
        short wd;                    /* ps width (height)             */
   } ps_size;                        /* dimensions of ps              */

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
     } font_table[8];                  /* font selector                 */


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
    } character_box;                    /* dimensions of character box   */

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
/*                                                                    */
/**********************************************************************/

    struct cursr_shape {
	ushort              cursor_top  ;
	ushort              cursor_bot  ;
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


/* /*----------------------------------------------------------------------*
/*  *   IDENTIFICATION: MEGENV.H                                           *
/*  *   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal     *
/*  *                     data structures for the Megapel adapter.         *
/*  *                                                                      *
/*  *   FUNCTION: Declare and initialize data structures that are          *
/*  *             global to all Megapel  VDD procedures.                   *
/*  *                                                                      *
/*  *----------------------------------------------------------------------*/
/*
/*typedef union {
/*           struct {
/*                ushort ps_char;         /* ASCII character and code pg  */
/*                ushort ps_attr;         /* character attributes         */
/*           } ps_entry;
/*        unsigned int ps_fw;             /* presentation space full word */
/*        } Pse;                          /* presentation space entry     */
/*
/*
/*struct  vttenv  {
 
/*--------------------------------------------------------------------*/
/*                                                                    */
/* Virtual Driver Mode: Current mode of the Virtual Device Driver     */
/*                             0 => monitored mode                    */
/*                             1 => character mode (KSR)              */
/*                                                                    */
/*                      NOTE: the current state of the real device    */
/*                            (rscr_mode) is stored in the RMA.       */
/*                                                                    */
/*                      NOTE: the default mode is character           */
/*                                                                    */
/*                                                                    */
/* Virtual Driver State: Current state of the Virtual Display Driver  */
/*                             0 => inactive (presentation space      */
/*                                            updated)                */
/*                             1 => active (frame buffer updated)     */
/*                                                                    */
/*                        NOTE: the default state is inactive.        */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*
/*     struct {
/*          unsigned mode : 1;           /* monitor = 0 ksr = 1           */
/*          unsigned active : 1;         /* term inactive = 0 active = 1  */
/*          unsigned cur_vis : 1;        /* cursor invisible = 0 vis = 1  */
/*          unsigned cur_blank : 1;      /* cursor non blank = 0 blank = 1*/
/*          unsigned attr_valid : 1;     /* set if attribute is valid     */
/*          unsigned rsv : 3;            /* reserved                      */
/*     } flg;
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Presentation space scroll pointer:                                 */
/* /*              offset into the ps of the character in the logical    */
/* /*              upper-left corner of the ps. These variables allow    */
/* /*              the ps to be scrolled without moving lots of data.    */
/* /*                                                                    */
/* /*              NOTE: the default value is 0                          */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*     unsigned int scroll;              /* pse entry offset into ps      */
/*                                       /* (4 bytes per entry)           */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Cursor Attributes:                                                 */
/* /*                                                                    */
/* /*              Characteristics of the cursor prior  to the           */
/* /*              execution of the next VDD command.                    */
/* /*                                                                    */
/* /*              NOTE: if the virtural terminal is active, this        */
/* /*              field contains the attributes of the cursor that      */
/* /*              is currently being displayed.                         */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*      struct {
/*           short top;                   /* top cursor scan line (pels)   */
/*           short bot;                   /* bottom cursor scan line (pels)*/
/*      } cur_shape;
/*
/*      struct {
/*           short ht;                    /* height of character (pels)    */
/*          short wd;                    /* width of character (pels)     */
/*     } char_box;                       /* dimensions of character box   */
/*
/*     struct {
/*          unsigned int col;            /* cursor position in ps (y)     */
/*          unsigned int row;            /* cursor position in ps (x)     */
/*     } cur_pos;                        /* cursor coordinates in ps      */
/*
/*     struct {
/*          unsigned fg: 4;              /* cursor foreground color       */
/*          unsigned bg: 4;              /* cursor background color       */
/*     } cur_color;                      /* cursor color                  */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Screen position of bottom left of TEXT portion in pels.            */
/*                                                                     */
/* --------------------------------------------------------------------*/
/*
/*      struct {
/*           short pel_x;                  /* pel indentation to ll of     */
/*                                         /* TEXT portion of screen.      */
/*           short pel_y;                  /* pel indentation to ll of     */
/*     } scr_pos;                         /* TEXT portion of screen.      */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Active character mode color table for this instance.               */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*      struct vttcolt vtt8_ct;
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Megapel default color table.                                       */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*      struct vttcolt vtt8_ctm;
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /* Virtual Terminal ID: Index into the RMA's virtual terminal table.  */
/* /*                      This value is passed to the VDD when the      */
/* /*                      Initialize command is called.                 */
/* /*                                                                    */
/* /*                      NOTE: default is -1                           */
/* /*                                                                    */
/* /* Real Screen ID: Index into the RMA's real screen table.            */
/* /*                 This value is calculated by the Initialize         */
/* /*                 command (the virtual terminal table contains the   */
/* /*                 id of the currently selected display).             */
/* /*                                                                    */
/* /*                      NOTE: default is -1                           */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*
/*      short vt_id;                      /* index into RMA's virt trm tbl */
/*      short rscr_id;                    /* index into RMA's real scr tbl */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /*  Presentation Space:                                               */
/* /*                                                                    */
/*  /*              Area that stores the two-byte character codes         */
/*  /*              and two-byte attribute codes (character and           */
/*  /*              attribute codes are stored together in one full       */
/*  /*              word).                                                */
/*  /*                                                                    */
/*  /*--------------------------------------------------------------------*/
/*
/*       Pse *ps;                          /* starting address of ps        */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /*  Presentation Space Size (bytes)                                   */
/*                                                                     */
/*               Contains the total number of bytes in the ps .        */
/*               (width = height = -1 implies the ps is not allocated).*/
/*                                                                     */
/*               NOTE: default value is -1                             */
/*                                                                     */
/*   Presentation Space Size (full characters)                         */
/*                                                                     */
/*              Contains the current width and height of the ps       */
/*              (width = height = -1 implies the ps is not allocated) */
/*              in characters.                                        */
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*
/*     int ps_bytes;                     /* number of bytes in ps         */
/*     int ps_words;                     /* number of words in ps         */
/*
/*     struct {
/*          short ht;                    /* ps height (row)               */
/*          short wd;                    /* ps width (height)             */
/*     } ps_size;                        /* dimensions of ps              */
/*
/* /*--------------------------------------------------------------------*/
/* /*                                                                    */
/* /*  Font Table: Contains 8 elements. The font selector in VTT_ATTR    */
/* /*              (canonical attribute specified in VTTCMD) indexes     */
/* /*              this table.                                           */
/* /*                                                                    */
/* /*--------------------------------------------------------------------*/
/*
/*      struct {
/*           ushort id;                   /* font id                       */
/*          int  index;                  /* index into rma                */
/*          char *addr;                  /* segment 0 offset to font      */
/*     } font_table[8];                  /* font selector                 */
/*
/*     char *line_buf;                   /* address of character buffer   */
/*
/*     struct {
/*            unsigned int cd_page;      /* code page of last char sent   */
/*            unsigned short curr_attr;  /* attributes of last char sent  */
/*     } crnt_attr;                      /* current attributes of char    */
/*
/*};
/*
/*
/*
/*
/*
/*
/*
/*
/*
*/
