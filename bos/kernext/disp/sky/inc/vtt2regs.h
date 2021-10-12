/* @(#)16	1.7  src/bos/kernext/disp/sky/inc/vtt2regs.h, sysxdispsky, bos411, 9428A410j 4/18/94 14:24:06 */
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

 /***START OF SPECIFICATIONS ****************************************
  *   IDENTIFICATION: VTT2REGS.H                                    *
  *   DESCRIPTIVE name: Skyway Adapter Register Declarations        *
  *                                                                 *
  *   FUNCTION: Declare literals that contain the addresses of the  *
  *             PC Color A/N adapter's registers.                   *
  *                                                                 *
  *             NOTE: These variables are declared to               *
  *                   give the VDD an image of the adapters I/O   . *
  *                   mapped register areas.                        *
  *   ATTRIBUTES:                                                   *
  *******************************************************************
  *                                                                 *
  **** END OF SPECIFICATIONS ***************************************/


/* Base address registers */

struct sky_io_regs {
 unsigned char    op_mode ;        /* Defines Adapter operating mode normal
                                      values will be '06'x or '07'x for
                                      Native Motorola mode                */

 unsigned char    pc_vram_window ; /* Register controls window into adapter
                                       memory. Window is either 64Kb or 1Mb
                                                                         */

 unsigned char    resv1 ;           /* Reserved register                 */

 unsigned char    resv2 ;           /* Reserved register                 */

 unsigned char    int_enable ;      /* Flags in this register control which
                                       interrupts are enabled on the adapter

                                       Bit 0 - Frame flyback
                                       Bit 1 - Reserved
                                       Bit 2 - Coprocessor access rejected
                                       Bit 3 - Coprocessor op complete
                                       Bits 4 - 7 Reserved               */

 unsigned char    int_status ;      /* Register containing status
                                       information as to which interrupt
                                       has occurred                      */

 unsigned char    vmem_int_enable;  /* Interrupt enable for virtual memory
				       accesses and other control fields:
                                        Bit 0 - Virtual address lookup
                                        Bit 1 - Reserved
                                        Bit 2 - Protection level
                                        Bit 3 - 5 - Reserved
                                        Bit 4 - Protection interrupt enable
                                        Bit 5 - Page fault interrupt enable
                                                                          */

 unsigned char    vmem_int_stat ;   /*  ??????


                                                                         */

 unsigned char    vram_index ;      /* Provides adress bits into VRAM on
                                       adapter when address space is smaller
                                       than installed VRAM. Shouldnt be used
                                       in this implementation.            */

 unsigned char    mem_acc_mode ;    /* Register defines pixel layout and
                                       format. Should be set to '0A'x or
                                       '0B'x for 4 or 8 bit pixel mode.
                                                                          */
 unsigned char    index;
				 /* Used to select native registers on
				   adapter card. Set to number of
				   register needed for read/write.
								      */
 unsigned char    data_b ;    /* Data register for 8 bit ops        */

 unsigned char    data_c ;        /* Data register for 16/32 bit ops    */
 unsigned char    data_d ;        /* Data register for 16/32 bit ops    */
 unsigned char    data_e ;        /* Data register for 32 bit ops       */
 unsigned char    data_f ;        /* Data register for 32 bit ops       */

} ;




struct sky_cop_regs {

 unsigned int  page_dir_base;  /*
                                                                        */

 unsigned int  cur_virt_addr;  /*
                                                                        */

 char          resvd1;         /* Unused space                           */

 unsigned char pollreg;        /* Polling register                       */

 short         resvd1a;        /* Unused space                           */

 short         resvd2;         /* Unused space                           */

 unsigned char state_a_len;    /* State sav length register             */

 unsigned char state_b_len;    /* State save length register            */

 unsigned short pix_index;     /* Pixel map index
                                                                        */

 unsigned char pi_control;     /* Pixel map control register
                                                                        */

 unsigned char resvd3;         /* Unused space                          */

 char *   pixmap_base;        /* Pointer to the base of the selected
				  pixel map                             */

 unsigned long pix_hw;

 short         resvd4;         /* Unused space                          */
 char          resvd4a;        /* Unused space                          */

 unsigned char pixmap_fmat;   /* format of pixel map                   */

 unsigned int  bres_errterm;   /* Error term for Bresenham line draw     */

 unsigned int  bres_k1;        /* K1 consant for Bresenham line draw     */

 unsigned int  bres_k2;        /* K2 consant for Bresenham line draw     */

 unsigned int  dir_steps;      /* Used for draw and step codes           */

 int           resvd5;         /* Unused space                           */
 int           resvd6;         /* Unused space                           */
 int           resvd7;         /* Unused space                           */
 int           resvd8;         /* Unused space                           */
 int           resvd9;         /* Unused space                           */
 int           resvd9a;         /* Unused space                           */

 unsigned short color_comp;   /* Color compare condition                */

 short bgfgmix;


 long          color_compval;  /* Color compare value                    */

 long          plane_mask;     /* Mask register for color masking        */

 long          car_chainmsk;   /* Carry chain mask for arithmetic mixing */

 long          fgd_color;      /* Index into pallette for foreground color
				  same number of bpp as the dest map     */

 long          bgd_color;      /* Index into pallette for background color
				  same number of bpp as the dest map     */

 long opdim21;

 long          resvda;         /* Unused space                           */
 long          resvdb;         /* Unused space                           */

 long maskyx;

 long src_yx;

 long pat_yx;

 long dst_yx;

 unsigned long        pixel_op_reg;

/* This register is defined as follows:                                 */
/* Byte 1 - XX|XX|XXXX                                                  */
/*          __    ____                                                  */
/*           ³ __   À----------- Step field - specifies operation       */
/*           ³  À--------------- Foreground color source                */
/*           À------------------ Background color source                */
/*                                                                      */
/* Byte 2 - XXXX|XXXX                                                   */
/*          ____ ____                                                   */
/*            ³    À------------ Specifies which of three pixel maps to */
/*            ³                  use as the destination.                */
/*            À----------------- Specifies which of three pixel maps to */
/*                               use as source.                         */
/*                                                                      */
/* Byte 3 - XXXX|XXXX                                                   */
/*          ____ ____                                                   */
/*            ³    À------------ Unused field                           */
/*            À----------------- Specifies which of three pixel maps to */
/*                               use as the pattern map.                */
/*                                                                      */
/* Byte 4 - XX|XX|X|XXX                                                 */
/*             __   ___                                                 */
/*          __  ³ _  À---------- Octant field for PxBlts and Bresenham  */
/*           ³  ³ ³              lines                                  */
/*           ³  ³ À------------- Unused                                 */
/*           ³  À--------------- Attributes for line drawing            */
/*           À------------------ Mask Control                           */
/*                                                                      */
/* See the PO defines in vtt2def.h for all combinations                 */
/*                                                                      */
/*                                                                      */

}; /* end of structure for coprocessor registers */


/*struct sky_cop_regs {
/*
/* unsigned int  page_dir_base;  /*
/*                                                                      */
/*
/* unsigned int  cur_virt_addr;  /*
/*                                                                      */
/*
/* int           resvd1;         /* Unused space                         */
/*
/* short         resvd2;         /* Unused space                         */
/*
/* unsigned char state_a_len;    /*
/*                                                                      */
/*
/* unsigned char state_b_len;    /*
/*                                                                      */
/*
/* short         pix_index;      /*
/*                                                                      */
/*
/* unsigned char pi_control;     /*
/*                                                                      */
/*
/* unsigned char resvd3;         /* Unused space                        */
/*
/* char    * pixmap_base;        /* Pointer to the base of the selected
/*                                  pixel map                           */
/* union  {
/*   unsigned int full;                  /* Fullword for assignments    */
/*
/*   struct {
/*    unsigned short pixmap_ht;   /* Height of pixel map in pixel rows   */
/*    unsigned short pixmap_wd;   /* Width of pixel map in pixel columns */
/*   } halfs;
/* }pix_hw;
/*
/*
/* short         resvd4;         /* Unused space                        */
/*
/* unsigned short pixmap_fmat;   /*
/*                                                                      */
/*
/* unsigned int  bres_errterm;   /* Error term for Bresenham line draw   */
/*
/* unsigned int  bres_k1;        /* K1 consant for Bresenham line draw   */
/*
/* unsigned int  bres_k2;        /* K2 consant for Bresenham line draw   */
/*
/* unsigned int  dir_steps;      /* Used for draw and step codes         */
/*
/* int           resvd5;         /* Unused space                         */
/* int           resvd6;         /* Unused space                         */
/* int           resvd7;         /* Unused space                         */
/* int           resvd8;         /* Unused space                         */
/* int           resvd9;         /* Unused space                         */
/*
/* unsigned short color_comp;   /* Color compare condition              */
/*
/*
/* union {                        /* Union to map mix values         */
/*  unsigned short    full;
/*
/*  struct {
/*    unsigned char bgd_mix;      /* Background mix value            */
/*    unsigned char fgd_mix;      /* Foreground mix value            */
/*   } bytes;
/* } bgfgmix;
/*
/*
/* long          color_compval;  /* Color compare value                  */
/*
/* long          plane_mask;     /* Mask register for color masking        */
/*
/* long          car_chainmsk; /* Carry chain mask for arithmetic mixing */
/*
/* long          fgd_color;    /* Index into pallette for foreground color
/*                                same number of bpp as the dest map     */
/*
/* long          bgd_color;    /* Index into pallette for background color
/*                                same number of bpp as the dest map     */
/*
/* union {
/*  uint         full;
/*
/*  struct {
/*    unsigned short op_dim2;      /* Holds width of rectangle for PxBlt or
/*                                   line length for a line draw          */
/*    unsigned short op_dim1;      /* Holds height of rectangle for PxBlt */
/*    } halfs;
/* } opdim21;
/*
/* long          resvda;       /* Unused space                           */
/* long          resvdb;       /* Unused space                           */
/*
/* union {       /* x,y parameters for mask     map */
/*  unsigned int full;
/*
/*  struct {
/*    ushort      y_offset;   /* Y offset of mask map from dest map org */
/*    ushort      x_offset;   /* X offset of mask map from dest map org */
/*  } halfs;
/* } maskyx;
/*
/*
/*
/* union {       /* x,y parameters for source map */
/*  uint full;
/*
/*  struct {
/*    ushort      y_coord;      /* Specifies y coordinate of source pixel */
/*    ushort      x_coord;      /* Specifies x coordinate of source pixel */
/*  } halfs;
/* } src_yx;
/*
/*
/* union {       /* x,y parameters for pattern map */
/*  uint full;
/*
/*  struct {
/*   ushort      y_coord;      /* Specifies y coordinate of pattern pixel */
/*   ushort      x_coord;      /* Specifies x coordinate of pattern pixel */
/*  } halfs;
/* } pat_yx;
/*
/* union {       /* x,y parameters for destination map */
/*  uint full;
/*
/*  struct {
/*   ushort      y_coord;      /* Specifies y coordinate of dest pixel */
/*   ushort      x_coord;      /* Specifies x coordinate of dest pixel */
/*  } halfs;
/* } dst_yx;
/*
/* ulong        pixel_op_reg;
/*
/* This register is defined as follows:                                 */
/* Byte 1 - XX|XX|XXXX                                                  */
/*          __    ____                                                  */
/*           ³ __   À----------- Step field - specifies operation       */
/*           ³  À--------------- Foreground color source                */
/*           À------------------ Background color source                */
/*                                                                      */
/* Byte 2 - XXXX|XXXX                                                   */
/*          ____ ____                                                   */
/*            ³    À------------ Specifies which of three pixel maps to */
/*            ³                  use as the destination.                */
/*            À----------------- Specifies which of three pixel maps to */
/*                               use as source.                         */
/*                                                                      */
/* Byte 3 - XXXX|XXXX                                                   */
/*          ____ ____                                                   */
/*            ³    À------------ Unused field                           */
/*            À----------------- Specifies which of three pixel maps to */
/*                               use as the pattern map.                */
/*                                                                      */
/* Byte 4 - XX|XX|X|XXX                                                 */
/*             __   ___                                                 */
/*          __  ³ _  À---------- Octant field for PxBlts and Bresenham  */
/*           ³  ³ ³              lines                                  */
/*           ³  ³ À------------- Unused                                 */
/*           ³  À--------------- Attributes for line drawing            */
/*           À------------------ Mask Control                           */
/*                                                                      */
/* See the PO defines in vtt2def.h for all combinations                 */
/*                                                                      */
/*                                                                      */
/*
/*}; /* end of structure for coprocessor registers */



