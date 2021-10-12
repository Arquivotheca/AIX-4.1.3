/* @(#)24	1.10.3.4  src/bos/kernext/disp/gem/inc/gem_ldat.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:43:02 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		
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


/***********************************************************************/
/* GM Context Slot Structure Definition for RCM                        */
/***********************************************************************/
typedef struct _rcx_node {      /* def. of node in linked list of pRcxs*/
    struct _rcx *pRcx;          /* pointer to rcx                      */
    struct _rcx_node *pNext;    /* pointer to next node                */
} rcx_node;

typedef struct _CxtSlot {       /* data for each context slot in array */
    int type;                   /* Immediate, Traversal, Rcxp          */
    int status_flags;           /* current state of slot               */
    ulong slot_addr;            /* (rios) address of this slot         */
    ulong slot_len;             /* size of slot                        */
    ulong slot_lock;            /* Word for locking slot array         */
    int num_rcx;                /* number rcx's bound to this slot     */
    rcx_node *pHead;            /* head of linked list of rcx's        */
} CxtSlot;

/***********************************************************************/
/* RCM Private Data Definition                                         */
/***********************************************************************/
typedef struct _rGemRCMPriv {
    
  ulong   gcp_geo;                      /* GCR value to select GCP     */
  ulong   drp_geo;                      /* GCR value to select DRP     */
  ulong   shp_geo;                      /* GCR value to select SHP     */

  volatile ulong   *gcr_reg;            /* GM  control reg ptr         */
  volatile ulong   *gsr_reg;            /* GM  status reg ptr          */
  volatile ulong   *ipr_reg;            /* Interrupt pending reg ptr   */
  volatile ulong   *idc_reg;            /* Inc-dec count ptr           */
  volatile ulong   *iur_reg[4];         /* in-use regs for FIFOs       */
  volatile ulong   *add_reg[4];         /* Add regs for FIFOs          */
  volatile ulong   *sub_reg[4];         /* Sub regs for FIFOs          */
  volatile ulong   *thres_reg[4];       /* Threshold regs for FIFOs    */
  volatile ulong   *gar_reg;            /* Geographical adr reg        */
  volatile ulong   *ser_reg[4];         /* Sync Enable Reg             */
  volatile ulong   *ter_reg[4];         /* Threshold Enable Reg        */
  volatile ulong   *ip_reg[4];          /* fifo write ptrs             */
  volatile char	   *pFunnel[4];		/* Fifo Funnel ptrs	       */
					/* 0: imm fifo se  (gcp)       */
					/* 1: imm fifo blt (drp)       */
					/* 2: trv fifo se  (gcp)       */
					/* 3: trv fifo blt (drp)       */

  ulong   gmbase;                       /* adapter memory base adr     */
  ulong   position_ofst;                /* adapter offset              */
  shmFifoPtr shmem;                     /* Ptr to dd_data in RCM structs */

/***********************************************************************/
/* Configuration Info Filled In By Initscreen                          */
/***********************************************************************/
  ulong  num_of_planes;                 /* Number of installed planes  */

  CxtSlot       slots[MAX_SLOTS];       /* array of context slot struct*/
  
/***********************************************************************/
/* Context slot managing fields					       */
/***********************************************************************/
  short		num_trav_slots;		/* Number of traversal slots alloc'd */
  short		num_imm_slots;		/* Number of immediate slots alloc'd */
  short		first_trav_slot;	/* Index of first trav slot    */
  short		first_imm_slot;		/* Index of first imm slot     */

/***********************************************************************/
/* Hardware ID managing fields                                         */
/***********************************************************************/
  ulong 	num_free_hwid;
  ulong		hwid_head;
  ulong		hwid_tail;
  struct	_hwid {
      rcmWG	*pwg;
      short	next;
      short	prev;
      volatile  ulong     currentlyUsed;
    } hwid[NUM_HWIDS];
  ulong		prot_hwids[NUM_VLTS][2][2];/* [color table][base blanes disp   */
					/*  buf][overlay planes disp buf]   */

  ulong		immfifo_busy;

  gRegionPtr	old_zbuffer;		/* clip region for last window that */
					/*  used the z-buffer		    */

  char		*pCommSave;		/* pointer to saved comm areas when */
					/*  save hardware state is done	    */

  char		*pPrivCxt;		/* pointer to RCM's private context */
  ushort	priv_hwid;		/* RCM's private context's wind id  */
  char		*pCursplanes;		/* pointer to cursor glyph          */

  ulong		imm_sync_cntr;		/* imm fifo hwid synch counter	    */
  ulong		trv_sync_cntr;		/* trav fifo hwid synch counter	    */

  volatile ulong fifo_sem;		/* fifo semaphore counter	    */

  ulong		num_zbuf_wind;		/* # of windows using Z Buffer	    */

  char		*generic1;		/* general purpose pointers	    */
  char		*generic2;		/* available for generic use	    */
  char		*generic3;
  char		*generic4;
  char		*generic5;		/* general purpose pointers	    */
  char		*generic6;		/* available for generic use	    */
  int 		lock;	    		/* a lock for serializing execution */
					/* of upd_geom			    */
  ulong		cur_3D_pid;
    
} rGemRCMPriv, *rGemRCMPrivPtr;

struct color_table
{
  long  num_entries;
  long  colors[16];
} ;

typedef union {
	   struct {
		ushort ps_char;         /* ASCII character and code pg  */
		ushort ps_attr;         /* character attributes         */
	   } ps_entry;
	unsigned int ps_fw;             /* presentation space full word */
	} Pse;                          /* presentation space entry     */

struct vttenv {

/**********************************************************************/
/*  Adapter Id:                                                       */
/**********************************************************************/
	  unsigned short adp_id;

/**********************************************************************/
/*  VT Mode:   monitor = 0 ksr = 1  APA=2                             */
/**********************************************************************/
	  unsigned short vtt_mode;

/**********************************************************************/
/*  Flags:                                                            */
/**********************************************************************/
    struct {
	  unsigned active : 1;       /* VT inactive = 0, VT active = 1*/
	  unsigned cur_vis : 1;      /* cursor invisible = 0 vis = 1  */
	  unsigned cur_blank : 1;    /* cursor non blank = 0 blank = 1*/
	  unsigned attr_valid : 1;   /* set if attribute is valid     */
	  unsigned rsv : 4;          /* reserved                      */
     } flg;

/**********************************************************************/
/*                                                                    */
/*  Presentation Space:                                               */
/*                                                                    */
/*                      NOTE: the character in the presentation space */
/*                            is initialized as a "space" with a      */
/*                            "white character/black background"      */
/*                            attribute.                              */
/*                                                                    */
/**********************************************************************/

    Pse          *ps;

/**********************************************************************/
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
/**********************************************************************/

   int ps_bytes;                     /* number of bytes in ps         */
   int ps_words;                     /* number of words in ps         */

   struct {
	short ht;                    /* ps height (row)               */
	short wd;                    /* ps width (height)             */
   } ps_size;                        /* dimensions of ps              */

/**********************************************************************/
/*                                                                    */
/*  Font Table: Contains 8 elements. The font selector in VTT_ATTR    */
/*              (canonical attribute specified in VTTCMD) indexes     */
/*              this table.                                           */
/*                                                                    */
/**********************************************************************/

     struct {
	 ushort id;                  /* font id                       */
	 int  index;                 /* index into rma                */
	 short height,width;         /* char box heigth and width     */
	 long size;                  /* size of the font in bytes     */
	 aixFontInfoPtr fontptr;     /* Pointer to font head          */
	 aixCharInfoPtr fontcindx;   /* Pointer to char index         */
	 aixGlyphPtr glyphptr;       /* Pointer to glyph pointer      */
     } font_table[MAX_FONTS];        /* font selector                 */


     unsigned int scroll;            /* pse entry offset into ps      */
				     /* (4 bytes per entry)           */

/**********************************************************************/
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
/**********************************************************************/

    struct {
	unsigned short height;          /* height of character (pels)  */
	unsigned short width;           /* width of character (pels)   */
    } char_box;                         /* dimensions of character box */

    struct {
	unsigned short fg;              /* cursor foreground color    */
	unsigned short bg;              /* cursor background color    */
    } cursor_color;                     /* cursor color               */

/**********************************************************************/
/*                                                                    */
/* Cursor Shape: width and height of the cursor shape                 */
/*               (width of (0)  means the cursor is not visible).     */
/*                                                                    */
/*               NOTE: the default value is a DOUBLE underscore       */
/*                                                                    */
/**********************************************************************/

    struct cursr_shape {
	ushort      top;
	ushort      bot;
    } cur_shape ;

    int cursor_select;
    int cursor_show;

/**********************************************************************/
/* This is the size in bytes of the cursor bitmap and a pointer it    */
/**********************************************************************/

    int  cursor_pixmap_size;
    char *cursor_pixmap;

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
	   int    col;
	   int    row;
    } cursor_pos;

/**********************************************************************/
/*                                                                    */
/* Screen position of top left of TEXT portion in pels.               */
/*                                                                    */
/**********************************************************************/

     struct {
	  short pel_x;                  /* pel indentation to tl of     */
					/* TEXT portion of screen.      */
	  short pel_y;                  /* pel indentation to tl of     */
     } scr_pos;                         /* TEXT portion of screen.      */

    ushort  current_attr;              /* current character attributes */
    char *expbuf;                      /* address of expansion buffer  */
    uint expbuf_len;                   /* length of expansion buffer   */
    char *glyphbuf;                    /* address of glyph buffer      */
    uint glyphbuf_len;                 /* length of glyph buffer       */
    uint bytes_in_glyph;
    uint bytes_in_glyph_row;

};

struct gemini_data {

  struct vttenv Vttenv;                 /* GEMINI Environment structure*/

  volatile unsigned int a_gem_gmem;        /* Start of GEMINI memory   */
  volatile unsigned int *sc_r_p;           /*  sys ctrl reg ptr        */
  volatile unsigned int *gc_r_p;           /*      GEM ctrl reg ptr    */
  volatile unsigned int *gadr_r_p;         /* geographic address reg   */
  volatile unsigned long *fifo_ptr[4];     /* Ptrs to FIFOs            */
  volatile unsigned long *fifo_cnt_reg[4]; /* Ptrs to In Use Counters  */
  uint free_store;                         /* free global memory offset*/
  uint free_store_len;                     /* length of free global mem*/
  char *component;                         /* pointer to component name*/

  struct ipl_shp_flgs *ipl_shp_fp;
  struct gmcrdslots *gcardslots;

  struct ipl_shp_flgs {                /* shared struct among DD and VDDs*/
	  int ipl_flg;                 /* Load micro code flag and Sha_ */
	  int shp_flg;                 /* ding proc card existance flg  */
  } iplshp_flgs;

 struct gmcrdslots                     /* gemini gcard slots          */
  {
      ulong   magic;                    /* slot 0,1                     */
      ulong   drp;                      /* slot 9                       */
      ulong   gcp;                      /* slot 6,2                     */
      ulong   shp;                      /* slot 7,3                     */
      ulong   imp;                      /* slot 0                       */
      ulong   gcp_start;                /* start address of GCP ucode   */
   } gm_crdslots;

  ushort fgcol;                        /* foreground color              */
  ushort bgcol;                        /* background color              */

 struct dw_rect {                       /* struct passed to ps_to_scr    */
	short  ul_row;                  /* upper left row                */
	short  ul_col;                  /* upper left column             */
	short  lr_row;                  /* lower right row               */
	short  lr_col;                  /* lower right column            */
	unsigned attr_valid:1;          /* 1 = use attr field for attrs  */
					/* 0 = check each char for attrs */
	unsigned rsvd1 : 15;            /* reserved                      */
	ushort attr;                    /* attributes                    */
     } draw_rect;

   struct color_table ctbl;

  rGemRCMPriv	GemRCMPriv;		/* Private area for RCM	      @1*/
};

typedef struct gemini_data	rGemData, *rGemDataPtr;		/*    @3*/
