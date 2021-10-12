/* @(#)80	1.5.2.8  src/bos/kernext/disp/gem/com/gmcomm.h, sysxdispgem, bos411, 9428A410j 1/22/93 09:19:19 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: ADD_INIT
 *		AIXGSC
 *		AIXGSC_DEF
 *		CHANGE_WID
 *		CLEAR
 *		DRAW_IN_WINDOW
 *		END_WINDOW
 *		FIFO_AVAL
 *		FIFO_EMPTY
 *		FI_INIT
 *		FUNNEL_INIT
 *		GMBASE_INIT
 *		GM_OFST
 *		IP_INIT
 *		IUR_INIT
 *		READ_DATA
 *		RESET_BIT
 *		RESET_PLANE
 *		RESTORE_REGS
 *		SELECT_WINDOW
 *		SET_BIT
 *		SET_FBMASK
 *		SET_FILL_PROCS
 *		SET_LINE_PROCS
 *		SET_MISC_PROCS
 *		SET_TEXT_PROCS
 *		SET_WINDOW
 *		SET_WIN_COMP
 *		SET_WORG
 *		TER_INIT
 *		THRES_INIT
 *		UNDRAW_REGIONS
 *		VME_ADR
 *		WRITE_BYTE
 *		WRITE_CONTIG
 *		WRITE_DATA
 *		WRITE_WORD
 *		WTFIFO
 *		WTFIFO_SHORT
 *		
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


/*----------------------------------------------------------------------*/
/*    defines for the gcp cursor control block                      @P@4*/
/*----------------------------------------------------------------------*/
#define NUMCSRCLR 3             /* number cursor color table entries  @4*/
#define NUMCSRPAT 2             /* number of cursor pattern pointers  @G*/
#define NUMCSRCRD 2             /* number of cursor coordinates       @4*/
#define CURSPMAPH 64            /* hight of cursor pixmap             @9*/
#define CURSPMAPW 64            /* width of cursor pixmap             @9*/
#define CURSPMSIZ CURSPMAPH*CURSPMAPW/8  /* number of bytes in cursor pixmap*/

#define DEFCURSOR   0x00000001       /* define cursor                 @A*/
#define ACTCURSOR   0x00000002       /* activate cursor               @A*/
#define DEACTCURSOR 0x00000004       /* deactivate cursor             @A*/
#define COLCURSOR   0x00000008       /* set cursor colors             @A*/

/*   defines for cursor control *//*;@G*/
    
#define CURSFMT    0x00000010    /* bit 4 is cursor format control   @G*/
#define XHAIR_EN   0x00000020    /* bit 5 is cursor crosshair enable @G*/
#define PAT_EN     0x00000040    /* bit 6 is cursor pattern enable   @G*/
#define DUAL_PL_EN 0x00000100    /* bit 8 is dual plane enable       @G*/
#define LOADCURPAT 0x00000200    /* bit 9 is load cursosr patterh    @G*/


/*----------------------------------------------------------------------*/
/* Defines for macros WTFIFO, BLT_COMPLT, FIFO_EMPTY                    */
/*                                                                      */
/*      note: THRES_P, ADD_P, FIFO_P & FI will have indexes appended    */
/*            to them because there are 4 of each.                      */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
#define GCR_P   pDevP->gcr_reg			/*		      @1*/
#define GSR_P   pDevP->gsr_reg			/*		      @1*/
#define IPR_P   pDevP->ipr_reg                  /* Int Pending Reg.   @1*/
#define IUR_P   pDevP->iur_reg                  /* In Use Reg.        @1*/
#define ADD_P   pDevP->add_reg			/*		      @1*/
#define SUB_P   pDevP->sub_reg                  /* Subtract Reg.      @1*/
#define FIFO_P  pDevP->pFunnel                  /*                      */
#define THRES_P pDevP->thres_reg                /*                    @1*/
#define FI      pDevP->shmem->fi                /*                    @D*/
#define TER_P   pDevP->ter_reg                  /* threshold enabl reg@E*/
#define SER_P   pDevP->ser_reg                  /* sync enable reg    @E*/
 
/*----------------------------------------------------------------------*/
/* Common constants							*/
/*----------------------------------------------------------------------*/
#define GM_BASE gm_base                     /* Address of gm adapter    */
#define GAI_DRAW_PLANE  0x00000080      	/* Plane for GAI Prims@2*/
#define GAI_GPM_PLANE   0x00000040      /* Plane for Global Plane Mask@I*/
#define GAI_GPM_WPMASK  (~GAI_GPM_PLANE & 0xff)
					/* Write Protect Mask for GPM @I*/
#define GAI_WIN_PLANES  0x0000003F      /* Planes for HW Windows      @K*/
#define GAI_WIN_WPMASK  (~GAI_WIN_PLANES & 0xff)
					/* Write Protect Mask HW WINs @K*/

#define IMAGE_DEFAULT_FMT 0x08080800    /* Default FMT for base planes@3*/

/*----------------------------------------------------------------------*/
/* TO BE DONE:                                                          */
/* check values of all these h/w specific equates.                      */
/*-----   GM    Hardware Specific Equates ------------------------------*/


/* SpaceAge Backplane defines @C*/
#define SA_SELECT_SHP 0x00000007        /* select Shp (via Geo adr reg@E*/
#define SA_SELECT_GCP 0x00000006        /* select GCP (via Geo adr reg@E*/
#define SA_SELECT_DRP 0x00000009        /* select DrP (via Geo adr reg@E*/

/* GM Backplane defines @C*/
#define GM_SELECT_SHP 0x00000003        /* select Shp (via Geo adr reg@E*/
#define GM_SELECT_GCP 0x00000002        /* select GCP (via Geo adr reg@E*/
#define GM_SELECT_DRP 0x00000009        /* select DrP (via Geo adr reg@E*/

/* Misc				*/
#define FRAME_WIDTH     1280            /*  memory width in pels        */
#define FRAME_HEIGHT    1024            /*  memory height in pels       */
#define GM_WIDTH       1280            /*  viewable width in pels      */
#define GM_HEIGHT      1024            /*  viewable height in pels     */
#define CUR_WD_MIN         1            /*  minimum cursor width   pels */
#define CUR_HG_MIN         1            /*  minimum cursor height  pels */
#define CUR_WD_MAX        64            /*  maximum cursor width   pels */
#define CUR_HG_MAX        64            /*  maximum cursor height  pels */
#define MSK_WD_MAX        32            /* maximum masked cursor width  */
#define MSK_HG_MAX        32            /* maximum masked cursor height */
#define LOGICAL_OPS     0xffffffff      /*  supported logical operations*/
#define CUR_STORAGE     2048            /*  maximum storage required    */
#define FONT_CLASS         2            /*  font class compressed       */
#define BUS_DELAY       0x80E0          /*  bitmap memory address       */
 
 
#define BITMAP_SIZE_GM  1048576         /*  visable bitmap size         */
#define PHY_WIDTH_GM     377            /*  physical width in mm      @Z*/
#define PHY_HEIGHT_GM    301            /*  physical height in mm     @Z*/
#define N_PLANES_GM        8            /*  number of color planes      */
#define VLT_SIZE_GM      256            /** video lookup table size     */
#define ADP_CHAR_GM     0x60000000      /*  adapter characteristics     */
#define COLOR_DAC_MGM      4            /*  bits for RGB in dac         */
#define COL_DAC_GM16       8            /* Bits for 16m RGB in dac      */

 
#define RETRY_MAX         10            /*  Time to wait for comm area  */
#define MAX_FIFO           4            /*  Four FIFOs                  */
#define FifoLen          65536          /*  Size of fifo              @b*/
#define FifoLimit        65524          /*  Max bytes in SE fifo      @b*/
#define IURMASK         0x0001FFFF      /*  Valid 17 bits of in-use rg@5*/

#define GMGraphicsRegsOfst  0x00001000  /*  for set of graphics regs  @G*/
#define GMGaiRegsOfst       0x00000000  /*  for set of gai regs       @G*/
 
 
/*----------------------------------------------------------------------*/
/* Color Plane Select Register                                          */
/*----------------------------------------------------------------------*/
#define DEF_PLANES      0x000000ff      /*  all planes                  */
#define GMDefaultColor  0x0000001f      /* initial default color white@9*/

/*----------------------------------------------------------------------*/
/* Video Lookup Table Color Mappings                                    */
/*----------------------------------------------------------------------*/
#define CT_LOAD_SYNCHRONOUS  0          /* color table load synchronous */
#define CT_LOAD_ASYNC        1         	/* color table load asynchronous */
#define COLOR_INT_16M   0x3FC0          /*  color isol mask 16m col     */
#define COLOR_INTENSITY 0x3C00          /*  color isolation mask        */
#define BLUE_SHIFT      0x06            /*  color right shift values    */
#define GREEN_SHIFT     0x02            /*  color left shift            */
#define RED_SHIFT       0x0a            /*  color left shift            */
 
/*----------------------------------------------------------------------*/
/* Video Lookup Table Default Color Mappings                            */
/*----------------------------------------------------------------------*/
#define COLOR_0         0x000000        /*  black                       */
#define COLOR_1         0xF00000        /*  red                         */
#define COLOR_2         0x00F000        /*  green                       */
#define COLOR_3         0xF0F000        /*  yellow                      */
#define COLOR_4         0x0000F0        /*  blue                        */
#define COLOR_5         0xF000F0        /*  magenta                     */
#define COLOR_6         0x00F0F0        /*  cyan                        */
#define COLOR_7         0xD0D0D0        /*  white                       */
#define COLOR_8         0x404040        /*  grey                        */
#define COLOR_9         0xF04040        /*  light red                   */
#define COLOR_A         0x70F000        /*  light green                 */
#define COLOR_B         0xC08020        /*  brown                       */
#define COLOR_C         0xD070F0        /*  light blue                  */
#define COLOR_D         0xE030E0        /*  light magenta               */
#define COLOR_E         0x70F0D0        /*  light cyan                  */
#define COLOR_F         0xF0F0F0        /*  high intensity white        */
 
 
#define ring_buf        1024            /*  Size of default ring buffer */
 
#define bank_set      0x00007fff        /*  high intensity white        */
 
 
/*----------------------------------------------------------------------*/
/* Frame Buffer Control Values                                        @B*/
/*----------------------------------------------------------------------*/
#define FBC_DRAW_A	0x00000000
#define FBC_DRAW_B	0x80000000
#define FBC_DISP_A	0x00000000
#define FBC_DISP_B	0x40000000
#define FBC_FILL	0x10000000
  
/*----------------------------------------------------------------------*/
/* Frame Buffer Select  Values                                        @B*/
/*----------------------------------------------------------------------*/
#define BASE_PLANES    0x00000000   /* Flag value for to select dest of WP */
#define OVERLAY_PLANES 0x00000001   /* Flag value for to select dest of WP */
#define WINDOW_PLANES  0x00000002   /* Flag value for to select dest of WP */
#define Z_BUFFER       0x00000003   /* Flag value for to select dest of WP */
#define CURRENT_FB     0x00000004   /* Flag value for to select dest of WP */


 
/*----------------------------------------------------------------------*/
/*   GM   FPGI Structure Element Codes (based on FPGI spec of 7/28/88)  */
/*----------------------------------------------------------------------*/
#define SE_NOOP         0x0000          /* no operation                 */
#define SE_PLI          0x0001          /* set polyline index           */
#define SE_PMI          0x0002          /* set polymarker index         */
#define SE_TXI          0x0003          /* set text index               */
#define SE_EI           0x0004          /* set edge index               */
#define SE_II           0x0005          /* set interior index           */
#define SE_DCI          0x0006          /* set depth cue index          */
#define SE_CPI          0x0007          /* set color processing index   */
#define SE_LT           0x0008          /* set linetype                 */
#define SE_LWSC         0x0009          /* set linewidth scale factor   */
#define SE_PLCI         0x000A          /* set polyline color index     */
#define SE_MT           0x000B          /* set marker type              */
#define SE_MSSC         0x000C          /* set marker size scale factor */
#define SE_PMCI         0x000D          /* set polymarker color index   */
#define SE_TXFO         0x000E          /* set text font                */
#define SE_TXPR         0x000F          /* set text precision           */
#define SE_CHXP         0x0010          /* set character expansion factr*/
#define SE_CHSP         0x0011          /* set character spacing        */
#define SE_TXCI         0x0012          /* set text color index         */
#define SE_CHH          0x0013          /* set character height         */
#define SE_CHUP         0x0014          /* set character up vector      */
#define SE_TXPT         0x0015          /* set text path                */
#define SE_CHPM         0x0016          /* set character positioning mod*/
#define SE_TXAL         0x0017          /* set text alignment           */
#define SE_IS           0x0018          /* set interior style           */
#define SE_ISI          0x0019          /* set interior style index     */
#define SE_ICI          0x001A          /* set interior color index     */
#define SE_EF           0x001B          /* set edge flag                */
#define SE_ELT          0x001C          /* set edge linetype            */
#define SE_ECI          0x001D          /* set edge color index         */
#define SE_ESC          0x001E          /* set edge scale factor        */
#define SE_PLET         0x001F          /* set polyline end type        */
#define SE_AS           0x0020          /* set annotation style         */
#define SE_AHSC         0x0021          /* set annotation ht scal factor*/
#define SE_AH           0x0022          /* set annotation height        */
#define SE_AUP          0x0023          /* set annotation up vector     */
#define SE_APT          0x0024          /* set annotation path          */
#define SE_AAL          0x0025          /* set annotation alignment     */
#define SE_CHUB         0x0026          /* set character up and base vec*/
#define SE_CHLS         0x0027          /* set character line scale fact*/
#define SE_PLCD         0x0028          /* set polyline color direct    */
#define SE_PMCD         0x0029          /* set polymarker color direct  */
#define SE_TXCD         0x002A          /* set text color direct        */
#define SE_ICD          0x002B          /* set interior color direct    */
#define SE_ECD          0x002C          /* set edge color direct        */
#define SE_FBM          0x0031          /* set frame buf wt prot mask   */
#define SE_FBC          0x0032          /* set frame buf comparison     */
#define SE_ASF          0x0035          /* set aspect source flag       */
#define SE_BICI         0x003F          /* set back interior color index*/
#define SE_BICD         0x0040          /* set back interior color direc*/
#define SE_SCI          0x0041          /* set specular color index     */
#define SE_SCD          0x0042          /* set specular color direct    */
#define SE_BSCI         0x0043          /* set back specular color index*/
#define SE_BSCD         0x0044          /* set back specular color direc*/
#define SE_PGC          0x0045          /* set polygon culling          */
#define SE_SPR          0x0046          /* set surface properties       */
#define SE_BSPR         0x0047          /* set back surface properties  */
#define SE_FDMO         0x0048          /* set face distinguish mode    */
#define SE_LSS          0x0049          /* set light source state       */
#define SE_HID          0x004A          /* set HLHSR identifier         */
#define SE_CST          0x004B          /* set color source type        */
#define SE_CAC          0x004C          /* set curve approx criteria    */
#define SE_SAC          0x004D          /* set surface approx criteria  */
#define SE_PHEC         0x004E          /* set polyhedron edge culling  */
#define SE_LMO          0x004F          /* set lighting calculation mode*/
#define SE_SETA         0x0052          /* set anti-aliasing            */
#define SE_ZBM		0x0055		/* set z-buffer protect mask	*/
#define SE_MLX3         0x00D0          /* set modelling xform 3        */
#define SE_MLX2         0x00D1          /* set modelling xform 2        */
#define SE_GLX3         0x00D2          /* set global xform 3           */
#define SE_GLX2         0x00D3          /* set global xform 2           */
#define SE_HLCI         0x00E0          /* set hilighting color index   */
#define SE_HLCD         0x00E1          /* set hilighting color direct  */
#define SE_ADCN         0x00E2          /* add class name to set        */
#define SE_RCN          0x00E3          /* remove class name from set   */
#define SE_INAD         0x00E4          /* insert application data      */
#define SE_CRET         0x00F0          /* conditional return           */
#define SE_TEX3         0x00F1          /* test extent 3                */
#define SE_TEX2         0x00F2          /* test extent 2                */
#define SE_COND         0x00F3          /* set condition                */
#define SE_EXST         0x00FA          /* execute structure            */
#define SE_INLB         0x00FB          /* insert label                 */
#define SE_PKID         0x00FC          /* set pick identifier          */
#define SE_EXRI         0x00FD          /* exec struct w/rev inherit    */
#define SE_CEXS         0x00FE          /* conditional execute struct   */
#define SE_PL3          0x0101          /* polyline 3                   */
#define SE_PL2          0x0102          /* polyline 2                   */
#define SE_DPL3         0x0103          /* disjoint polyline 3          */
#define SE_DPL2         0x0104          /* disjoint polyline 2          */
#define SE_PM3          0x0105          /* polymarker 3                 */
#define SE_PM2          0x0106          /* polymarker 2                 */
#define SE_TX3          0x0107          /* text 3                       */
#define SE_TX2          0x0108          /* text 2                       */
#define SE_AN3          0x0109          /* annotation text 3            */
#define SE_AN2          0x010A          /* annotation text 2            */
#define SE_PXL3         0x010F          /* pixel 3                      */
#define SE_PXL2         0x0110          /* pixel 2                      */
#define SE_CR2          0x0111          /* circle 2                     */
#define SE_CRA2         0x0112          /* circular arc 2               */
#define SE_NBC3         0x0116          /* non-uniform B-spline curve 3 */
#define SE_NBC2         0x0117          /* non-uniform B-spline curve 2 */
#define SE_EL3          0x0118          /* ellipse 3                    */
#define SE_EL2          0x0119          /* ellipse 2                    */
#define SE_ELA3         0x011A          /* elliptical arc 3             */
#define SE_ELA2         0x011B          /* elliptical arc 2             */
#define SE_PG3          0x0121          /* polygon 3                    */
#define SE_PG2          0x0122          /* polygon 2                    */
#define SE_MG3          0x0125          /* marker grid 3                */
#define SE_MG2          0x0126          /* marker grid 2                */
#define SE_LG3          0x0127          /* line grid 3                  */
#define SE_LG2          0x0128          /* line grid 2                  */
#define SE_PGD3         0x012B          /* polygon with data 3          */
#define SE_PGD2         0x012C          /* polygon with data 2          */
#define SE_TS3          0x012D          /* triangle strip 3             */
#define SE_TS2          0x012E          /* triangle strip 2             */
#define SE_CHL3         0x012F          /* character line 3             */
#define SE_CHL2         0x0130          /* character line 2             */
#define SE_NBS          0x0131          /* non-uniform B-spline surface */
#define SE_TNBS         0x0132          /* trimmed non-unif B-splin surf*/
#define SE_CFA3         0x0133          /* composite fill area 3        */
#define SE_CFA2         0x0134          /* composite fill area 2        */
#define SE_PHE          0x0135          /* polyhedron edge              */
 
 
/*----------------------------------------------------------------------*/
/*   GM   FPGI Command Element Codes (based on   GM   s/w spec 9/15/88  */
/*        Assigned values to these based on new s/w/ spec.            @6*/
/*----------------------------------------------------------------------*/
#define CE_SVWI         0x00D8          /* set view index               */
#define CE_WRIT         0x0180          /* write pixel                  */
#define CE_READ         0x0181          /* read pixel                   */
#define CE_COPY         0x0182          /* VPM copy                     */
#define CE_PTDAT        0x0184          /* compos fill-2 w/connectiv    */
#define CE_PLCI         0x0185          /* set polyline color - integ   */
#define CE_INCI         0x0186          /* set interior color - integ   */
#define CE_IPLY         0x0187          /* integer polyline             */
#define CE_IDPL         0x0188          /* integer disjoint polyline    */
#define CE_IPLG         0x0189          /* integer polygon              */
#define CE_FMSK         0x018A          /* set frame buffer mask        */
#define CE_LOGO         0x018B          /* set logicval op              */
#define CE_LINP         0x018C          /* reset line pattern           */
#define CE_PATR         0x018D          /* Pattern Reference Point      */
#define CE_FRECT	0x018E		/* Fill Rectangle		*/
#define CE_ACTC         0x01C0          /* activate context             */
#define CE_LDSV         0x01C1          /* load drawing state vector    */
#define CE_SDSV         0x01C2          /* store drawing state vector   */
#define CE_SDDS         0x01C3          /* set default drawing state    */
#define CE_STTT         0x01C4          /* set traversal type           */
#define CE_EPCO         0x01C5          /* end pick correlation         */
#define CE_RPAS         0x01C6          /* restore pick attrib state    */
#define CE_SYNC         0x01C7          /* synchronize                  */
#define CE_SDFB         0x01C8          /* select drawing frame buffer  */
#define CE_FCTL         0x01C9          /* frame buffer control         */
#define CE_LBPC         0x01CA          /* load base planes color tabl  */
#define CE_LOPC         0x01CB          /* load overlay plane colr tab  */
#define CE_SCPM         0x01CC          /* set color processing mode    */
#define CE_LBWP         0x01CD          /* link base/window planes      */
#define CE_UBWP         0x01CE          /* unlink base/window planes    */
#define CE_WATT         0x01CF          /* set window origin            */
#define CE_DMSK         0x01D0          /* set display mask             */
#define CE_EDOF         0x01D1          /* enable/disable other FIFO    */
#define CE_AECT         0x01D2          /* adjust element counter       */
#define CE_SRSR         0x01D3          /* save+replace selected regs   */
#define CE_RESR         0x01D4          /* restore selected registers   */
#define CE_SBIT         0x01D5          /* set bit                      */
#define CE_RBIT         0x01D6          /* reset bit                    */
#define CE_GMEM         0x01D7          /* set global memory            */
#define CE_GDMP         0x01D8          /* GCP dump                     */
#define CE_FBC          0x01D9          /* set frame bfr comparison     */
#define CE_PZBF		0x01DA		/* protect Z buffer		*/
#define CE_CZBF		0x01DB		/* clear Z buffer		*/
#define CE_SPSTK	0x01DC		/* Save Pick State 		*/
#define CE_RPSTK	0x01DD		/* Restore Pick State 		*/
#define CE_SETBV	0x01DE		/* Set Base View Index		*/
#define CE_BFLSH	0x01DF		/* conditional begin FIFO	*/
					/*  element flush		*/
#define CE_EFLSH	0x01E0		/* end FIFO element flush	*/
#define CE_DBS		0x01E1		/* display buffer sync		*/
#define CE_TS 		0x01E2		/* Test Set           		*/
#define CE_SGCA		0x01E3		/* Set Comm Area      		*/
#define CE_RSHVY	0x01E4		/* Resume from Heavy Context	*/
					/*  Switch			*/
#define CE_GSYNC	0x01E5		/* Gsync			*/
#define CE_SWAPI	0x01E6		/* Swap Interval		*/
#define CE_BLINK	0x01E7		/* Blink color			*/
#define CE_ACTF         0x01E8          /* Fast Activate Context        */

/*----------------------------------------------------------------------*/
/* GM MAGIC Memory Map Microchannel address offsets                     */
/*----------------------------------------------------------------------*/
#define COMM_ofst       0x00000000      /* cVME comm area offset      @E*/
#define GCP_IRP_ofst    0x0000007C      /* GCP imm fifo read pointer  @R*/
#define DRP_IRP_ofst    0x00000084      /* DRP imm fifo read pointer  @R*/
#define GCP_TRP_ofst    0x00000084      /* GCP trv fifo read pointer  @R*/
#define DRP_TRP_ofst    0x0000008C      /* DRP trv fifo read pointer  @R*/
#define GCR_ofst        0x00000300      /*   GM    control reg offset @E*/
#define GSR_ofst        0x00000304      /*   GM    status reg offset  @E*/
#define IPR_ofst        0x00000308      /* Interrupt Pending reg offse@E*/
#define IDC_ofst        0x0000030C      /* Inc/Dec/Clear reg offset.  @E*/
#define	SCR_ofst	0x00000400	/* System control reg offset	*/
#define FNL0_ofst       0x00000600      /* FIFO 0 funnel offset         */
#define FNL1_ofst       0x00000800      /* FIFO 1 funnel offset         */
#define FNL2_ofst       0x00001600      /* FIFO 2 funnel offset         */
#define FNL3_ofst       0x00001800      /* FIFO 3 funnel offset         */
#define IP0_ofst        0x00000a14      /* FIFO 0 write pointer offset@E*/
#define IUR0_ofst       0x00000a18      /* FIFO 0 in use  reg offset  @E*/
#define	CTR0_ofst	0x00000a1c	/* FIFO 0 control reg offset  @e*/
#define ADD0_ofst       0x00000a20      /* FIFO 0 add reg offset      @E*/
#define SUB0_ofst       0x00000a24      /* FIFO 0 subtract reg offset @E*/
#define THR0_ofst       0x00000a08      /* FIFO 0 threshold reg offset@E*/
#define IP1_ofst        0x00000b14      /* FIFO 1 write pointer offset@E*/
#define IUR1_ofst       0x00000b18      /* FIFO 1 in use  reg offset  @E*/
#define	CTR1_ofst	0x00000b1c	/* FIFO 1 control reg offset  @e*/
#define ADD1_ofst       0x00000b20      /* FIFO 1 add reg offset      @E*/
#define SUB1_ofst       0x00000b24      /* FIFO 1 subtract reg offset @E*/
#define THR1_ofst       0x00000b08      /* FIFO 1 threshold reg offset@E*/
#define IP2_ofst        0x00001a14      /* FIFO 2 write pointer offset@E*/
#define IUR2_ofst       0x00001a18      /* FIFO 2 in use  reg offset  @E@e*/
#define	CTR2_ofst	0x00001a1c	/* FIFO 2 control reg offset  @e*/
#define ADD2_ofst       0x00001a20      /* FIFO 2 add reg offset      @E@e*/
#define SUB2_ofst       0x00001a24      /* FIFO 2 subtract reg offset @E@e*/
#define THR2_ofst       0x00001a08      /* FIFO 2 threshold reg offset@E@e*/
#define IP3_ofst        0x00001b14      /* FIFO 3 write pointer offset@E*/
#define IUR3_ofst       0x00001b18      /* FIFO 3 in use  reg offset  @E*/
#define	CTR3_ofst	0x00001b1c	/* FIFO 3 control reg offset  @e*/
#define ADD3_ofst       0x00001b20      /* FIFO 3 add reg offset      @E*/
#define SUB3_ofst       0x00001b24      /* FIFO 3 subtract reg offset @E*/
#define THR3_ofst       0x00001b08      /* FIFO 3 threshold reg offset@E*/
#define GAR_ofst        0x00000310      /* Geographical addr reg ofst @E*/
#define SER0_ofst       0x00000320      /* sync enable intr reg0 ofst @E*/
#define SER1_ofst       0x00000324      /* sync enable intr reg1 ofst @E*/
#define SER2_ofst       0x00000328      /* sync enable intr reg2 ofst @E*/
#define SER3_ofst       0x0000032C      /* sync enable intr reg3 ofst @E*/
#define TER0_ofst       0x00000a00      /* thresh enabl intr rg0 ofst @E*/
#define TER1_ofst       0x00000b00      /* thresh enabl intr rg1 ofst @E*/
#define TER2_ofst       0x00001a00      /* thresh enabl intr rg2 ofst @E*/
#define TER3_ofst       0x00001b00      /* thresh enabl intr rg3 ofst @E*/
 
#define SMSG_X    (1<<(SIGMSG-1))       /*  mask for SIGMSG             */

#if 0
/*----------------------------------------------------------------------*/
/*   GM    CONTROL REGISTER    READ / WRITE                             */
/*----------------------------------------------------------------------*/
#define GcrBos          0x01000001      /* byte order select            */
#define GcrModeLd       0x00000002      /* mode load                    */
#define GcrResetCvme    0x00000004      /* reset CVME bus               */
#define GcrResetGem     0x00000008      /* reset   GM   Registers       */
#define GcrGcpInProg    0x00010000      /* GCP in progress              */
#define GcrDrpInProg    0x00020000      /* DRP in progress              */
#define GcrFnCode0      0x00100000      /* function code 0              */
#define GcrFnCode1      0x00200000      /* function code 1              */
#define GcrFnCode2      0x00400000      /* function code 2              */
#define GcrEnaCvme      0x02000000      /* enable CVME bus intrpt       */
#define GcrEnaBerr      0x08000000      /* enable berr bus intrpt       */
 
/*----------------------------------------------------------------------*/
/*   GM    STATUS  REGISTER    READ ONLY                                */
/*----------------------------------------------------------------------*/
#define GsrIntPend      0x00000001      /* Interrupt pending.           */
#define GsrCvmeInt      0x00000008      /* cVME int                   @E*/
#define GsrSync0        0x00000010      /* sync0 counter is zero        */
#define GsrSync1        0x00000020      /* sync1 counter is zero        */
#define GsrSync2        0x00000040      /* sync2 counter is zero        */
#define GsrSync3        0x00000080      /* sync3 counter is zero        */
#define GsrThres0       0x00000100      /* in-use reg <= thres0 reg     */
#define GsrThres1       0x00000200      /* in-use reg <= thres1 reg     */
#define GsrThres2       0x00000400      /* in-use reg <= thres2 reg     */
#define GsrThres3       0x00000800      /* in-use reg <= thres3 reg     */
 
/*----------------------------------------------------------------------*/
/* INTERRUPT PENDING REGISTER   READ ONLY                               */
/*----------------------------------------------------------------------*/
#define IprSync0Pend    0x00000010      /* sync0 intrpt pending         */
#define IprSync1Pend    0x00000020      /* sync1 intrpt pending         */
#define IprSync2Pend    0x00000040      /* sync2 intrpt pending         */
#define IprSync3Pend    0x00000080      /* sync3 intrpt pending         */
#define IprThres0Pend   0x00000100      /* threshold0 intrpt pending    */
#define IprThres1Pend   0x00000200      /* threshold1 intrpt pending    */
#define IprThres2Pend   0x00000400      /* threshold2 intrpt pending    */
#define IprThres3Pend   0x00000800      /* threshold3 intrpt pending    */
#define IprCvmePend     0x00001000      /* CVME bus intrpt pending      */
#define IprBerrPend     0x00002000      /* Bus error intrpt pending     */

/*--------------------------------------------------------------------  */
/* INCREMENT DECREMENT CLEAR REG.    READ / WRITE                       */
/*--------------------------------------------------------------------  */
#define IdcSync0        0x00000001      /* Sync0 counter select.        */
#define IdcSync1        0x00000002      /* Sync1 counter select         */
#define IdcSync2        0x00000004      /* Sync2 counter select.        */
#define IdcSync3        0x00000008      /* Sync3 counter select.        */
#define IdcIncrCnt      0x00000100      /* Increment counter.           */
#define IdcDecrCnt      0x00000200      /* Decrement counter.           */
#define IdcClearCnt     0x00000800      /* Clear counter.               */

/*----------------------------------------------------------------------*/
/* Graphical Address Register        READ / WRITE                     @E*/
/*----------------------------------------------------------------------*/
#define GarAdr0         0x00000001      /* Geographical Adr 0         @E*/
#define GarAdr1         0x00000002      /* Geographical Adr 1         @E*/
#define GarAdr2         0x00000004      /* Geographical Adr 2         @E*/
#define GarAdr3         0x00000008      /* Geographical Adr 3         @E*/

/*--------------------------------------------------------------------  */
/* Interrupts from GCP to RIOS                                          */
/* TO BE DONE - ASSIGN VALUES TO ALL OF THESE .... ALL ARE INCORRECT    */
/*--------------------------------------------------------------------  */
#define R_AckRequest    0x0000          /* acknowledge Rios request     */
#define R_InitComplete  0x0000          /* adapter initialization done  */
#define R_SyncEvent     0x0000          /* synchronize event            */
#define R_PickEvent     0x0000          /* pick event                   */
#define R_ElemExcpEvent 0x0000          /* cmd/struct elmt excep event  */
#define R_DebugResponse 0x0000          /* debug responses              */
#endif

/*--------------------------------------------------------------------  */
/* FIFO names                                                           */
/*--------------------------------------------------------------------  */
#define ImmSeFifo       0               /* immediate SE FIFO            */
#define ImmDataFifo     1               /* immediate data FIFO          */
#define TravSeFifo      2               /* traversal SE FIFO            */
#define TravDataFifo    3               /* traversal Data FIFO          */

/*--------------------------------------------------------------------  */
/* Maximum values for CONTEXT tables                                    */
/*--------------------------------------------------------------------  */
#define MaxLineTypes     16             /* max line type tables         */
#define MaxPatterns       2             /* max pattern tables         @P*/
#define MaxHatchs        24             /* max hatch tables             */
#define MaxOPlanesColor  16             /* max overlay planes colr tab  */
#define MaxBPlanesColor 256             /* base planes color map size 	*/
#define MaxBPlanesCMaps   5             /* max base planes color maps @A@P*/
#define MaxHatchArea   65536		/* Hatch work area (64K)    @a@J*/
					/* Doubles as Trim workarea too */
#define GAIMaxLineTypes   0             /* max line type tables (GAI) @1@P*/
#define GAIMaxPatterns    0             /* max pattern tables   (GAI) @1*/
#define GAIMaxHatchs      1             /* max hatch tables     (GAI) @1@P*/
#define GAIMaxViews       1             /* max view tables      (GAI) @1*/

/*--------------------------------------------------------------------  */
/* Values for line styles.                                              */
/*--------------------------------------------------------------------  */
#define LSSolid          1              /* line style solid           @N*/
 
  /*
   * Disable Other FIFO defines
   */
#define DISABLE_FIFO 1
#define ENABLE_FIFO  0

/*--------------------------------------------------------------------  */
/* Context 'changed' values.                                            */
/*--------------------------------------------------------------------  */
#define GMPatternChanged    0x00000010  /* patterns changed             */
#define GMHatchChanged      0x00000020  /* hatchs  changed              */
#define GMLineTypeChanged   0x00000004  /* line type changed            */
#define GMWindowOrigChanged 0x00004000  /* window origin changed        */

 
#define GM_THRES_INTR_ENABLE 0x00000001 /* TER Reg enable bit         @E*/

/*------------------------------------------------------------------@S@8*/
/* Values for VME and system addresses.                             @S@8*/
/*------------------------------------------------------------------@S@8*/
#define ZeroTopChar  0x001fffff
#define CvmeChar     0x03000000L
#define GMEM_BASE    CvmeChar
#define GM_OFST(a) ((ulong)(a) - pDevP->gmbase)  /* adapter memory offset        */
#define VME_ADR(a) (vme_t)(((ulong)(a) & ZeroTopChar) | CvmeChar)


/*--------------------------------------------------------------------@B*/
/* GAI defines for GM                                             @B*/
/*--------------------------------------------------------------------@B*/
#define GAI_TILE_PATTERN 	1
#define GAI_STIPPLE_PATTERN	2
#define GAI_HATCH 		1
#define PATTERN_CHANGE		0x00000010
#define GAI_TPATCHG_BIT		0x00000001
#define GAI_SPATCHG_BIT		0x00000002
#define HATCH_CHANGE		0x00000020
#define GAI_HATCHG_BIT		0x00000001

#define NumMonitors             1                                  /* @H*/

/*--------------------------------------------------------------------@M*/
/* GAI defines for Integer Polygon Flags                              @M*/
/*--------------------------------------------------------------------@M*/
#define CONCAVE 0x00000000
#define CONVEX  0x00020000

/*--------------------------------------------------------------------  */
/* Sizes for initialization.                                            */
/*--------------------------------------------------------------------  */

typedef struct _2D_ucode_flags {
  volatile ulong imm_rp_start;		/* Read Pixel start flags for   */
  volatile ulong unused0;
  volatile ulong unused1;
  volatile ulong unused2;
} GM_2D_UCFLAGS ;

typedef struct _ucode_flags {
  volatile ulong freeze_3d;  		/* Semaphore for RCM<->ucode    */
  volatile ulong reset_3d;  		/* Semaphore for RCM<->ucode    */
  volatile ulong flush_drp; 		/* Semaphore for RCM<->ucode    */
  volatile ulong imm_cxt_add;		/* address of immediate context	*/
  volatile ulong trv_cxt_add;		/* address of traversal context	*/
  GM_2D_UCFLAGS  *ptr_imm_rp_start;	/* ptr in 2D domain to		*/
  					/* Read Pixel start flags for   */
                                  	/* ensuring data is being xfer'd*/

                                   	/* Read Pixel start flags for   */
  volatile ulong trv_rp_start;		/* ensuring data is being xfer'd*/
  volatile ulong unused0;
  volatile ulong unused1;
  volatile ulong unused2;
  volatile ulong unused3;
} GM_UCFLAGS ;

#define GMWindowPlaneSize   FRAME_WIDTH*FRAME_HEIGHT                /*  */
#define GMOverlayPlaneSize  FRAME_WIDTH*FRAME_HEIGHT                /*  */
#define GMZBufferSize       FRAME_WIDTH*FRAME_HEIGHT                /*  */
#define GMBitmapSize        FRAME_WIDTH*FRAME_HEIGHT*4              /*  */
#define GMDefaultDisplayMask 0x00ffffff                             /*  */
#define GMOverlayDisplayMask 0x000f0f0f				    /*@X*/
#define GMMaxGeoRegs        16          /* # geographical regs          */
#define GMMaxMemRegs        128         /* # memory regs                */
#define GMCommAreaSize      512         /* common area size             */
#define RCMPrivCxtSize	    3072    				    /*@d*/
#define GemMemSize	0x00200000				    /*@T*/
#define GemMemSize4Meg	0x00400000				    /*@T*/
#define DISPINDSIZE	24		/* Num bytes for disp buf array */
#define RSVD_SIZE	16		/* Num bytes for reserved area */
#define GMMaxGblMem  (GemMemSize - 0x10000 -				\
  			(2 * FifoLen) - RCMPrivCxtSize -	    /*@d*/ \
  			(MaxBPlanesCMaps * MaxBPlanesColor * 4) -	   \
                        MaxHatchArea - (NUMCSRPAT * CURSPMSIZ)  -	   \
		      DISPINDSIZE - (2 * FifoLen)) - RSVD_SIZE -	   \
  		      (2*MaxOPlanesColor*sizeof(ulong)) -		   \
		      sizeof(ulong) -    /* unused */			   \
		      sizeof(GM_UCFLAGS) - /* ucode flags */		   \
		      sizeof(GM_2D_UCFLAGS)  /* 2d domain ucode flags */
  			
  					/*Global Mem Size     @F@P@Q@U@V*/

/*----------------------------------------------------------------------*/
/*   GM   memory map for 2Meg adapter                                   */
/*									*/
/*  NB: If this structure changes such that the address of either	*/
/* gm_ucflags or gm_2d_ucflags changes, then some hard-coded addresses	*/
/* in gem_vdd.c must be changed.  (Search for 'Clear Special address'.)	*/
/*----------------------------------------------------------------------*/
 
typedef struct gem_mem_map {            /*               start ofst     */
  char           gm_comm[GMCommAreaSize];/* comm area      +0           */
  ulong          geo_reg0[GMMaxGeoRegs];/*                +0200       @E*/
  char           pad0[192];             /*                +0240       @E*/
  ulong          mem_reg0[GMMaxMemRegs];/*                +0300       @E*/
  char           pad1[3584];            /*                +0500       @E*/
  ulong          mem_reg1[GMMaxMemRegs];/*                +1300       @E*/
  char		 pad2[0x10000 - 0x1500];
  char           tse_fifo[FifoLen];     /* Trav SE FIFO               @V*/
  char           tpd_fifo[FifoLen];     /* Trav Pixel Data FIFO       @V*/
  char		 rcm_priv_cxt[RCMPrivCxtSize];	/* RCM's private cxt  @d*/
  ulong		 unused;  		/* leftover			*/
  char           hatwa[MaxHatchArea];   /* Hatch Work Area            @R*/
  ulong		 olay_ct_2D[MaxOPlanesColor]; /* Overlay load area for 2D  */
  ulong		 olay_ct_3D[MaxOPlanesColor]; /* Overlay load area for 3D  */
  ulong		 bpct[MaxBPlanesCMaps][MaxBPlanesColor];/* Base Planes 	*/
							/* Color table@S*/
  volatile char	 disp_buf_ind[DISPINDSIZE];  /* Display Buffer Indicators */
  GM_UCFLAGS	 gm_ucflags;		     /* ucode flags		*/
  char		 rsvd[RSVD_SIZE];
  char           mem[GMMaxGblMem];   /* global memory  +1500         */
  GM_2D_UCFLAGS	 gm_2d_ucflags;		     /* ucode flags		*/
  char           cursplane[NUMCSRPAT][CURSPMSIZ];/* Cursor planes  @P@4*/
  char           ise_fifo[FifoLen];     /* immed SE FIFO                */
  char           ipd_fifo[FifoLen];     /* immed Pixel Data FIFO        */
} GM_MMAP ;                                                         /*  */


/*----------------------------------------------------------------------*/
/*   GM   memory map for 4Meg adapter                                   */
/*----------------------------------------------------------------------*/
#define GMMaxGblMem4Meg (GMMaxGblMem + 0x00200000) 
 
typedef struct gem_mem_map4meg {       /*               start ofst     */
  char		 pad[0x10000];		/* Skip 64K Bdy after regs 	*/
  char           tse_fifo[FifoLen];     /* Trav SE FIFO               @V*/
  char           tpd_fifo[FifoLen];     /* Trav Pixel Data FIFO       @V*/
  char		 rcm_priv_cxt[RCMPrivCxtSize];	/* RCM's private cxt  @d*/
  ulong		 unused;  		/* leftover			*/
  char           hatwa[MaxHatchArea];   /* Hatch Work Area            @R*/
  ulong		 olay_ct_2D[MaxOPlanesColor]; /* Overlay load area for 2D  */
  ulong		 olay_ct_3D[MaxOPlanesColor]; /* Overlay load area for 3D  */
  ulong		 bpct[MaxBPlanesCMaps][MaxBPlanesColor];/* Base Planes 	*/
							/* Color table@S*/
  char		 disp_buf_ind[DISPINDSIZE];  /* Display Buffer Indicators */
  GM_UCFLAGS	 gm_ucflags;		     /* ucode flags		*/
  char		 rsvd[RSVD_SIZE];
  char           mem[GMMaxGblMem4Meg]; /* global memory       */
  GM_2D_UCFLAGS	 gm_2d_ucflags;		     /* ucode flags		*/
  char           cursplane[NUMCSRPAT][CURSPMSIZ];/* Cursor planes   @P@4*/
  char           ise_fifo[FifoLen];     /* immed SE FIFO                */
  char           ipd_fifo[FifoLen];     /* immed Pixel Data FIFO        */
} GM_MMAP4 ;      



  /* Misc */
#define NO_CHANGE -1
  
  /* Interior Style */
#define GM_HOLLOW       1
#define GM_SOLID        2
#define GM_PATTERN      3
#define GM_HATCH        4
#define GM_EMPTY        5

  /* Interior Style Index */
#define GM_DONT_CARE            1                               /*  @2*/
#define GM_HORZ_LINES           8                                 /*  */
#define GM_VERT_LINES           9                                 /*  */
#define GM_DIAG_LRUL            10                             /*     */
#define GM_DIAG_LLUR            11                             /*     */
#define GM_HORZ_VERT            12                             /*     */
#define GM_DIAG_XHATCH          13                             /*     */

  /* Hatch patterns used to implement Polymarkers */
#define GM_USER_POLYM   7
#define GM_DOT          8
#define GM_PLUS         9
#define GM_ASTERISK     10
#define GM_CIRC         11
#define GM_CROSS        12
#define GM_BOX          13
  
  /* GM Logops */
#define GM_REPLACE      0
#define GM_INVERT       5                                       /*  */
#define GM_AND          12                                          /*@4*/
#define GM_ANDINVERT    13                                          /*@5*/

  /* GM Window Attributes constants @W */
#define CTID_FLAG	0x0f	/* unset bit in MSB makes param valid */
#define CPM_FLAG  	0x17	/* unset bit in MSB makes param valid */
#define OBSC_FLAG 	0x1b	/* unset bit in MSB makes param valid */
#define WORG_FLAG 	0x1d	/* unset bit in MSB makes param valid */
#define WSIZ_FLAG 	0x1e	/* unset bit in MSB makes param valid */

#define CPM8           	0x0000	/* 8 bit mode				*/
#define CPM24          	0x0004	/* 24 bit mode				*/
#define UNOBSCURED	0x0002	/* window is unobscured			*/

  /* GM Line End Types */                                           /*@3*/
#define GM_FLAT         1               /* Flat                       @3*/
#define GM_NOTLAST      4               /* Not Last                   @3*/


/*--------------------------------------------------------------------@3*/
/* Typedefs for types of adapter variables                            @3*/
/*--------------------------------------------------------------------@3*/
typedef ulong vme_t;            /* vme type address                   @3*/


/*----------------------------------------------------------------------*/
/*   GM    adapter element  for Communications Area                     */
/*                                                                      */
/*      Purge any thoughts of   MP    communication area when thinking  */
/*      about   GM  . There is a mapped comm   area in the MAGIC 2mb    */
/*      (the first 512 bytes). It is the communication area of whichever*/
/*      card is accessed via the geographical address field of the      */
/*        GM   control register. Therefore, there will be a union of    */
/*      comm area structures, one for each area associated with a       */
/*      valid geographical address ( e.g., for the comm area on the     */
/*      GCP).                                                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

struct font_index                               /* Index entry in comm  */
{                                               /* area point to by ATF */
  ushort font_cnt;                              /* number of font tbls  */
                                                /* for each font        */
  ushort font_id;                               /* Id of the font entr  */
  ushort font_adrs1;                            /* addr of font wd 1    */
  ushort font_adrs2;                            /* addr of font wd 2    */
};                         /* as many of these as there are font tbls   */
 
struct c_tab {             /*  cursor table structure in comm area      */
  ushort tabl_status;                           /* tablet status word   */
  ushort tabl_button;                           /* tablet buttons stat  */
  ushort tabl_x[2];                             /* tablet x position    */
  ushort tabl_y[2];                             /* tablet y position    */
  ushort tabl_z[2];                             /* tablet z position    */
 };


/*----------------------------------------------------------------------*/
/* SCP to GCP data blocks                                               */
/*----------------------------------------------------------------------*/

struct scpgcp_db {                      /*                              */
        ushort  len;                    /* length in bytes              */
        char    busy;                   /* busy flag                    */
        char    req;                    /* request code from SCP to GCP */
        ulong   scp_tcda;               /* SCP tack contrl data area    */
        char    data[16];               /* data (union all combination  */
                                        /* WANT A UNION HERE????????????*/
};

/*----------------------------------------------------------------------*/
/* GCP to SCP data blocks                                               */
/*----------------------------------------------------------------------*/

struct gcpscp_db {                      /*                              */
        ushort  len;                    /* length in bytes              */
        char    busy;                   /* busy flag                    */
        char    rea;                    /* reason  code from GCP to SCP */
        ulong   scp_tcda;               /* SCP tack contrl data area    */
        char    data[16];               /* data (union all combination  */
                                        /* WANT A UNION HERE????????????*/
};


  typedef struct gm_ccb {       /* local image of cursor cntl blk     @4*/

                                /* offset x00  cursor request code      */
    volatile unsigned long  curreqcde;  
    				/* cursor reqst code                  @J*/
                                /* only loworder 16 bits are significant*/

                                /* offset x04                           */
                                /* upper left of cursor crosshair window*/

    union {                     /* cursor window coordinate             */
      ulong buss;               /* buss acces is in 32 bit units        */
      struct {                  /* locally 16 bit units                 */
               ushort x;
               ushort y;
      } local;
    }csrwin;                    /* upper left cursor cross hair window @H*/

                                /* offset x08                           */
                                /* size of cursor cross hair window     */

    union {                     /* cursor window size                   */
      ulong buss;               /* buss acces is in 32 bit units        */
      struct {                  /* locally 16 bit units                 */
               ushort w;        /* width                                */
               ushort h;        /* height                               */
      } local;
    }csrsiz;                    /* upper left cursor cross hair window @H */

                                /* offset x0A thru x14                  */
                                /* rgb colormap for cursor colors       */

    ulong       csrclrmap[NUMCSRCLR];  /* reserve space for the colormap*/

    
   /*;deleted number of cusor patterns                               @G*/

    char * pCsrpat[NUMCSRPAT];  /* pointers to cursor patterns          */


    ulong csrctrl;              /* cursor control                    @H   */

    ulong csrcordsel;           /* cursor coordinate select  0 or 1  @H   */


                                /* offset x3C location of cursor        */

    union {                     /* cursor coordinates                   */
      ulong buss;               /* buss access is in 32 bit units       */
      struct {                  /* locally 16 bit units                 */
               ushort x;
               ushort y;
      } local;
    }csrcrd[NUMCSRCRD];         /* cursor coordinates                @H   */

  } GM_CCB;                                                     /*    @4*/


#ifdef GEMGAI							/*    @O*/
/*----------------------------------------------------------------------*/
/* Comm area structures mapped into cVME comm area                      */
/*   structs for SCP-GCP communications                                 */
/*   structs for SCP-DRP communications                                 */
/*----------------------------------------------------------------------*/

struct gcp_comm {                       /* GCP communication area       */
        off_t   scp_gcp_dbo;            /* SCP to GCP data block ofst   */
        off_t   gcp_scp_dbo;            /* GCP to SCP data block ofst   */
        char    rsvd0[92];              /* reserved                     */
        char    rsvd1[8];               /* reserved for image card      */
        ulong   cc;                     /* condition code               */
        ulong   ilf;                    /* initialization load flag     */
        char    rsvd2[4];               /* reserved                     */
        vme_t   ise_strto;              /* immed SE  fifo start offset@6*/
        vme_t   ise_reado;              /* immed SE  fifo read  offset@6*/
        vme_t   tse_strto;              /* trav SE   fifo start offset@6*/
        vme_t   tse_reado;              /* trav SE   fifo read  offset@6*/
        ulong   features;               /* installed features           */
        char    rsvd3[8];               /* reserved                     */
        ulong   ff;                     /* filter flags                 */
        struct scpgcp_db db[15];        /* data blocks                  */
};

struct drp_comm {                       /* DRP communication area     @4*/
        char       iva[32];             /* interrupt vector area        */
        char       pad1[8];             /* ---- pad ----      (irb)     */
        char       pad2[8];             /* ---- pad ----      (irb)     */
        char       rsvd1[80];           /* ---- reserved                */
        vme_t      ipd_strto;           /* imm Pix Data FIFO strt ofst@6*/
        vme_t      ipd_reado;           /* imm Pix Data FIFO read ofst@6*/
        vme_t      tpd_strto;           /* trav Pix Data FIFO strt ofs@6*/
        vme_t      tpd_reado;           /* trav Pix Data FIFO read ofs@6*/
        char       dcb[80];             /* debugger control block       */
        char       ercb[32];            /* error reporting control blk@7*/
        GM_CCB     ccb;                 /* cursor control block         */
        char       pad3[188];           /* ---- pad ----              @7*/
};
#endif GEMGAI


union gem_comm {
        char    byte[GMCommAreaSize];   /* access individual bytes      */
        struct gcp_comm gcp_comm;       /* GCP comm area                */
        struct drp_comm drp_comm;       /* DRP comm area                */
};
/*----------------------------------------------------------------------*/
/* Typedefs for generic structure elements                              */
/*----------------------------------------------------------------------*/

/* SINGLE 32-BIT INTEGER PARAMETER */
typedef struct generic_se {             /* Standard looking se buffer   */
  ushort len;                           /* with one 32 bit integer      */
  ushort opcode;                        /* for its parameter            */
  ulong  data;                          /*                              */
} generic_se;

/* SINGLE 32-BIT FLOATING POINT PARAMETER */
typedef struct generic_fltse {          /* Standard looking se buffer   */
  ushort len;                           /* with one 32 bit float point  */
  ushort opcode;                        /* for its parameter            */
  float  data;                          /*                              */
} generic_fltse;

/* TWO 16-BIT PARAMETERS */
typedef struct generic_xyse {           /* Standard looking se buffer   */
  ushort len;                           /* with two 16 bit integers     */
  ushort opcode;                        /* for parameters               */
  ushort x;                             /*                              */
  ushort y;                             /*                              */
} generic_xyse;

/* TWO 32-BIT INTEGER PARAMETERS */
typedef struct generic_flagse {         /* Standard looking se buffer   */
  ushort len;                           /* with one 32 bit integer flag */
  ushort opcode;                        /* and one 32 bit data field    */
  ulong  flags;                         /*                              */
  ulong  data;                          /*                              */
} generic_flagse;

/* SINGLE 32-BIT VME ADDRESS PARAMETER */
typedef struct generic_vme {            /* Standard looking se buffer   */
  ushort len;                           /* with one vme address         */
  ushort opcode;                        /* for its parameter            */
  vme_t  adr;                           /*                              */
} generic_vme;

/*----------------------------------------------------------------------*/
/* Typedefs for commonly used GM structure elements.                    */
/*----------------------------------------------------------------------*/
typedef struct scpm_se {                 /* Set Color Processing Mode 345 */
  ushort len;                           
  ushort opcode;                        
  ulong  PadRsh;                        /* Shift values for color mapping  */
  ulong  GshBsh;                        /*
                                         * "mapping" is defined as:
                                         *    char      scpm_R;
                                         *    char      scpm_pad;
                                         *    ushort    scpm_rsh;
                                         *    ushort    scpm_gsh;
                                         *    ushort    scpm_bsh;
                                         */   
} scpm_se;

typedef generic_vme ac_se;               /* Activate Context             */

typedef generic_vme sgm_se;               /* Set Global Memory          @I*/

typedef struct _sgm1 {			/* one word set global		*/
  generic_vme	sgm;	
  ulong		data;
} sgm1;

/*--------------------------------------------------------------------@B*/
/* Structures for the GAI attribute setting structure element buffers.  */
/* These are in alpha order roughly according to GAI attribute name.    */
/*----------------------------------------------------------------------*/

typedef struct fg_sebuf {                       /* Foreground color     */
  generic_se    plci;
  generic_se    inci;
} fg_sebuf;

typedef struct fs_sebuf {                       /* Fill Style           */
  generic_se    ins;
  generic_se    insi;
} fs_sebuf;

typedef struct alu_sebuf {                      /* ALU                @B*/
  generic_se    alu;
  fg_sebuf      fg;                             /* Foreground color     */
  fs_sebuf      fs;                             /* Fill Style           */
} alu_sebuf ;

typedef struct cs_sebuf {                       /* Cap Style            */
  generic_se    caps;
} cs_sebuf;

typedef struct ls_sebuf {                       /* Line Style           */
  generic_se    lt;
} ls_sebuf ;

typedef struct lw_sebuf {                       /* Line Width           */
  generic_fltse lwsf;
} lw_sebuf;

typedef struct mo_sebuf {                       /* Pattern Origin       */
  generic_xyse  refpt;
} mo_sebuf;

typedef struct pg_sebuf {                       /* Plane Group          */
  generic_se    sfb;
} pg_sebuf;

typedef struct pm_sebuf {                       /* Plane Mask           */
  generic_se    pm;
} pm_sebuf ;

typedef struct ct_sebuf {                       /* Color Table        @3*/
  generic_se    ct;
  vme_t         ctadr;
} ct_sebuf ;

typedef struct ptdat {                          /* Data for Pat Table @M*/
  short         patszx;
  short         patszy;
  long          datbtsz;
  off_t         pato;
} ptdat;

typedef struct htdat {                          /* Data for Hat Table @M*/
  short         hatszx;
  short         hatszy;
  off_t		hato;
} htdat;

typedef struct pat_sebuf {                      /* Pattern            @M*/
  sgm_se        sgm_numpat;
  ulong         numpat;
  sgm_se        sgm_pattabl;
  ptdat         pattabl;
  sgm_se        sgm_chng;
  ulong         chng;
  sgm_se        sgm_patchbt;
  ulong         patchbt;
  sgm_se        sgm_patdat;
  ulong         patdat[32*32];
} pat_sebuf;

typedef struct hat_sebuf {                      /* Hatch              @M*/
  sgm_se        sgm_numhat;
  ulong         numhat;
  sgm_se        sgm_hattabl;
  htdat         hattabl;
  sgm_se        sgm_chng;
  ulong         chng;
  sgm_se        sgm_hatchbt;
  ulong         hatchbt;
  sgm_se        sgm_hatdat;
  ulong         hatdat[32];
} hat_sebuf;

/*----------------------------------------------------------------------*/
/* Typedefs for the 2d primitives' attribute setting buffers          @B*/
/*----------------------------------------------------------------------*/
typedef struct fillattrs {                                          /*@N*/
  pm_sebuf              pm_buf;                 /* Plane mask           */
  alu_sebuf             alu_buf;                /* Logical Op         @9*/
  mo_sebuf              mo_buf;                 /* Pattern Origin       */
} fillattrs ;

typedef struct lineattrs {                                          /*@N*/
  pm_sebuf              pm_buf;                 /* Plane mask           */
  alu_sebuf             alu_buf;                /* Logical Op         @9*/
  mo_sebuf              mo_buf;                 /* Pattern Origin       */
  lw_sebuf              lw_buf;                 /* Line Width           */
  cs_sebuf              cs_buf;                 /* End Type             */
  ls_sebuf              ls_buf;                 /* Line Style           */
} lineattrs;

typedef struct miscattrs {
  pm_sebuf              pm_buf;                 /* Plane mask           */
  alu_sebuf             alu_buf;                /* Logical Op           */
} miscattrs;

typedef struct textattrs {                      /*                  @9@N*/
  pm_sebuf              pm_buf;                 /* Plane mask           */
  alu_sebuf             alu_buf;                /* Logical Op           */
  mo_sebuf              mo_buf;                 /* Pattern Origin       */
} textattrs;


/*----------------------------------------------------------------------*/
/* Typedefs for Window Attributes                                       */
/*----------------------------------------------------------------------*/

typedef struct windattrs {              /* Window attr se */
  ushort        len;
  ushort        opcode;
  uchar		mask;			/* which attributes to set:	*/
					/*   0 = set attribute		*/
					/*   1 = do not set attribute	*/
					/*     Bits 7 - 5 reserved	*/
					/*     Bit  4 = Color Table ID	*/
					/*     Bit  3 = Color Mode	*/
					/*     Bit  2 = Obscurity	*/
					/*     Bit  1 = Window Origin	*/
					/*     Bit  0 = Window Size	*/
  uchar		ctid;			/* must be 0 - 4 (3?)		*/
  ushort        flags;			/* flags - 31 - 3 reserved	*/
					/*         2      0 8 bit color	*/
					/*		  1 24 bit color*/
					/*         1      0 obscured	*/
					/*		  1 unobscured	*/
					/*	   0      0 this FIFO	*/
					/*		  1 other FIFO	*/
  ushort        x;
  ushort        y;
  ushort	width;
  ushort	height;
} windattrs;

typedef struct lastwinrect {            /* winOrg, width & height of   @L*/
  short         x;                      /* last updatewingeom            */
  short         y;
  ushort        w;
  ushort        h;
} lastwinrect;

/*----------------------------------------------------------------------*/
/* Typedefs for using GAI's window plane.                             @F*/
/*----------------------------------------------------------------------*/
typedef struct {
    ushort length;                      /* Header - Length              */
    ushort opc;                         /* Header - Op Code             */
    ulong fbmask;                       /* Frame Buffer Mask            */
    ulong cmpmask;                      /* Compare Mask                 */
    ulong cmpval;                       /* Compare Value                */
    ulong lincol;                       /* Line Color - Integer         */
    ulong lintype;                      /* Line Type                    */
    float linwid;                       /* Line Width                   */
    ulong endtyp;                       /* End Type                     */
    ulong intcol;                       /* Interior Color - Integer     */
    ulong intst;                        /* Interior Style               */
    ulong intsti;                       /* Interior Style Index         */
    ulong logop;                        /* Logical Operator             */
  } save_buf;

typedef struct {
    save_buf save;                      /* Save & Rest Selected Regs SE */
    generic_se clear;                   /* Frame Buffer Control SE      */
  } setwin;

typedef struct {
    ushort rlen;                        /* Header - Length              */
    ushort ropc;                        /* Header - Op Code             */
    ushort slen;                        /* Header - Length              */
    ushort sopc;                        /* Header - Op Code             */
    ulong mask;                         /* Mask                         */
  } write_with_window;

								    /*@8*/
typedef struct {
    ushort len;			/* Header - Length			*/
    ushort opcode;		/* Header - Op Code			*/
    ulong  flags;		/* flags  - 31 - 3   reserved   	*/
				/*	    2        0 update mask	*/
				/*		     1 do not update	*/
				/*          1        0 update value	*/
				/*		     1 do not update	*/
				/*          0        0 current fifo	*/
				/*                   1 other fifo	*/
    ulong  mask;		/* frame buffer comparison mask		*/
    ulong  value;		/* comparison pixel value       	*/
  } frame_buf_cmp;

typedef struct {                /* rectangle part of a SE       	*/
    ushort	len;		/* number of bytes in SE = 0x20 	*/
    ushort	opcode;
    ulong	flags;		/* concave                      	*/
    ulong	length;		/* number of bytes that follow = 0x18	*/
    struct {
	short x;
	short y;
    } pt[5];			/* ul - ll - lr - ur - ul       	*/
  } rectangle;

typedef struct {
    ushort	len;
    ushort	opcode;
    vme_t	adr;		/* Address in global memory		*/
    ulong	ofst;		/* Offset into GCP comm area		*/
    ulong	flags;		/* flags - 31 - 2   reserved		*/
				/*         1        0 update imm cntr	*/
				/*		    1 do not update	*/
				/*         0        0 update trv cntr	*/
				/*		    1 do not update	*/
    ulong	imm_cntr;
    ulong	trv_cntr;
  } disp_buf_sync;

/* Support for virtual window ID's (using the Ped windowing architecture) */
#define NUM_DWA_HWIDS  8
#define FIRST_DWA_HWID 0
#define LAST_DWA_HWID  (FIRST_DWA_HWID+NUM_DWA_HWIDS-1)

/* MACROS --------------------------------------------------------------*/

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SELECT_WINDOW          (macro)                       */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure elements to select the window planes      */
/*      to draw in.							*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SLECT_WINDOW( BUF );                                            */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a save_buf				*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SELECT_WINDOW( BUF, MASK, COLOR, CMASK, CVAL )                   \
{ ((save_buf *)BUF)->length = sizeof(save_buf);                          \
  ((save_buf *)BUF)->opc = CE_SRSR;                                      \
  ((save_buf *)BUF)->fbmask = (((~MASK) & 0x000000ff)      );            \
  ((save_buf *)BUF)->cmpmask = CMASK;                                    \
  ((save_buf *)BUF)->cmpval = CVAL;                                      \
  ((save_buf *)BUF)->lincol = ((COLOR) & 0x000000ff)      ;              \
  ((save_buf *)BUF)->lintype = LSSolid;                                  \
  ((save_buf *)BUF)->linwid = 1.0;                                       \
  ((save_buf *)BUF)->endtyp = GM_FLAT;                                   \
  ((save_buf *)BUF)->intcol = ((COLOR) & 0x000000ff)      ;              \
  ((save_buf *)BUF)->intst = GM_SOLID;                                   \
  ((save_buf *)BUF)->intsti = 1;                                         \
  ((save_buf *)BUF)->logop = GM_REPLACE;                                 \
  BUF += sizeof(save_buf);		  				 \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: RESTORE_REGS             (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure elements to restore registers after       */
/*      writing to window planes.					*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      RESTORE_REGS( BUF )                                             */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a long 					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define RESTORE_REGS( BUF )						\
{ 									\
  *((long *)BUF) = (0x00040000 | CE_RESR);                              \
  BUF += sizeof(long);							\
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: CLEAR                    (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Clear to zero the current plane(s) selected for drawing.        */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      CLEAR( BUF )                                                    */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a generic_se 				*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define CLEAR( BUF )							\
{ 									\
  ((generic_se *)BUF)->len = sizeof(generic_se);			\
  ((generic_se *)BUF)->opcode =  CE_FCTL;				\
  ((generic_se *)BUF)->data = FBC_FILL;				  /*@S*/\
  BUF += sizeof(generic_se);						\
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SET_WIN_COMP             (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure elements to use the compare value for     */
/*      hardware windows.                                               */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_WIN_COMP( BUF, MASK, VALUE )                                */
/*                                                                      */
/*  ALGORITHM:                                                          */
/*     a) select window planes                                          */
/*     b) set compare mask and value to passed parameter                */
/*     c) select base planes                                            */
/*                                                                      */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SET_WIN_COMP( BUF, MASK, VALUE )                                \
{                                                                       \
                                                                        \
/* set default buffer to window planes                                */\
                                                                        \
  ((generic_se *)BUF)->len = sizeof(generic_se);                        \
  ((generic_se *)BUF)->opcode = CE_SDFB;                                \
  ((generic_se *)BUF)->data = WINDOW_PLANES;                            \
  BUF += sizeof(generic_se);                                    	\
                                                                        \
/* set frame buffer comparison up for HW windowing planes             */\
                                                                        \
  ((frame_buf_cmp *)BUF)->len    = sizeof(frame_buf_cmp);               \
  ((frame_buf_cmp *)BUF)->opcode = CE_FBC;                              \
  ((frame_buf_cmp *)BUF)->flags = 0x00000000;				\
  ((frame_buf_cmp *)BUF)->mask   = MASK;				\
  ((frame_buf_cmp *)BUF)->value  = VALUE;				\
  BUF += sizeof(frame_buf_cmp);                                         \
                                                                        \
/* set default buffer to base planes                                  */\
                                                                        \
  ((generic_se *)BUF)->len = sizeof(generic_se);                        \
  ((generic_se *)BUF)->opcode =  CE_SDFB;                               \
  ((generic_se *)BUF)->data = BASE_PLANES;                              \
  BUF += sizeof(generic_se);                                    	\
                                                                        \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SET_BIT                 (macro)                      */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets a bit in the window plane compare mask, compare value,     */
/*      and framebuffer mask.                                           */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_BIT( BUF, MASK )                                            */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a long 					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SET_BIT( BUF, MASK )                                            \
{                                                                       \
                                                                        \
  ((generic_se *)BUF)->len = sizeof(generic_se);                        \
  ((generic_se *)BUF)->opcode = CE_SBIT;                                \
  ((generic_se *)BUF)->data = (((MASK)    ) & DEF_PLANES);              \
  BUF += sizeof(generic_se);                                    	\
                                                                        \
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: RESET_BIT                (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Resets a bit in the window plane compare mask, compare value,   */
/*      and framebuffer mask.                                           */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      RESET_BIT( BUF, MASK )                                          */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a long 					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define RESET_BIT( BUF, MASK )                                          \
{                                                                       \
                                                                        \
  ((generic_se *)BUF)->len = sizeof(generic_se);                        \
  ((generic_se *)BUF)->opcode = CE_RBIT;                                \
  ((generic_se *)BUF)->data = (((~MASK)    ) & DEF_PLANES);             \
  BUF += sizeof(generic_se);                                    	\
                                                                        \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SET_WORG                 (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Set Window Origin                                               */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_WORG( BUF, X, Y )                                           */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF - is a pointer to a long 					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SET_WORG( BUF, X, Y )                                           \
{                                                                       \
                                                                        \
  ((wo_sebuf *)BUF)->len = sizeof(wo_sebuf);                            \
  ((wo_sebuf *)BUF)->opcode = CE_WORG;                                  \
  ((wo_sebuf *)BUF)->flags = 0;                                         \
  ((wo_sebuf *)BUF)->x = (X);                                           \
  ((wo_sebuf *)BUF)->y = (Y);                                           \
  BUF += sizeof(wo_sebuf);                                      	\
                                                                        \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: CHANGE_WID          (macro)                          */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure elements to change window pixels of a     */
/*      given window id to zero window id.                              */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      CHANGE_WID( BUF, MASK, OLD_PIX, NEW_PIX);                       */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF     - is a pointer to a save_buf				*/
/*      MASK    - bits to be used during comparison                     */
/*      OLD_PIX - pixel value of the pixels to change                   */
/*      NEW_PIX - new pixel value to used                               */
/*                                                                      */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
 /* save selected registers, compare value = old_pix, color = new_pix */ 
 /* draw a rectangle the size of the screen                            */
 /* restore registers                                                  */
/************************************************************************/
#define CHANGE_WID( BUF, MASK, OLD_PIX, NEW_PIX )                        \
{									 \
									 \
  ((save_buf *)BUF)->length  = sizeof(save_buf);                         \
  ((save_buf *)BUF)->opc     = CE_SRSR;                                  \
  ((save_buf *)BUF)->fbmask  = (((~MASK) & 0x000000ff)      );           \
  ((save_buf *)BUF)->cmpmask = (((MASK)  & 0x000000ff)      );           \
  ((save_buf *)BUF)->cmpval  = (((OLD_PIX) & 0x000000ff)      );         \
  ((save_buf *)BUF)->lincol  = (((NEW_PIX) & 0x000000ff)      );         \
  ((save_buf *)BUF)->lintype = LSSolid;                                  \
  ((save_buf *)BUF)->linwid  = 1.0;                                      \
  ((save_buf *)BUF)->endtyp  = GM_FLAT;                                  \
  ((save_buf *)BUF)->intcol  = (((NEW_PIX) & 0x000000ff)      );         \
  ((save_buf *)BUF)->intst   = GM_SOLID;                                 \
  ((save_buf *)BUF)->intsti  = 1;                                        \
  ((save_buf *)BUF)->logop   = GM_REPLACE;                               \
  BUF += sizeof(save_buf);	         				 \
									 \
									 \
  ((rectangle *)BUF)->len         = sizeof(rectangle);                   \
  ((rectangle *)BUF)->opcode      = CE_IPLG;				 \
  ((rectangle *)BUF)->flags       = CONCAVE;				 \
  ((rectangle *)BUF)->length      = 0x18;				 \
  ((rectangle *)BUF)->pt[0].x     = 0x0000;			         \
  ((rectangle *)BUF)->pt[0].y     = 0x0000;			         \
  ((rectangle *)BUF)->pt[1].x     = (GM_WIDTH-1);			 \
  ((rectangle *)BUF)->pt[1].y     = 0x0000;				 \
  ((rectangle *)BUF)->pt[2].x     = (GM_WIDTH-1);			 \
  ((rectangle *)BUF)->pt[2].y     = (GM_HEIGHT-1);			 \
  ((rectangle *)BUF)->pt[3].x     = 0x0000;				 \
  ((rectangle *)BUF)->pt[3].y     = (GM_HEIGHT-1);			 \
  ((rectangle *)BUF)->pt[4].x     = 0x0000;			         \
  ((rectangle *)BUF)->pt[4].y     = 0x0000;			         \
  BUF += sizeof(rectangle);	         				 \
									 \
									 \
  *((long *)BUF) = (0x00040000 | CE_RESR);                               \
  BUF += sizeof(long);				 			 \
  }


/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SET_FBMASK               (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Set frame buffer write protect mask 			        */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_FBMASK( BUF, MASK )                                         */
/*                                                                      */
/*  ALGORITHM:                                                          */
/*     b) set frame buffer mask 				        */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      BUF   - is a pointer to a long 					*/
/*      MASK  - comparison mask, 1=protect                              */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SET_FBMASK( BUF, MASK ) 	                                \
{                                                                       \
                                                                        \
/* set frame buffer comparison & value                                */\
                                                                        \
  ((generic_se *)BUF)->len    = sizeof(generic_se);                     \
  ((generic_se *)BUF)->opcode = CE_FMSK;                                \
  ((generic_se *)BUF)->data   = (((~MASK)    )&DEF_PLANES);    		\
  BUF += sizeof(generic_se);                                    	\
                                                                        \
}

#define SPIN_CAUSE_NO_SLIH
#define TRACE   kgntrace


/*****  Main Entry Point  ***********************************************/
#define SET_MISC_PROCS(Ptr,sfx)                                   /*@K*/\
{                                                                       \
  g2dGCProcPtr ProcPtr;                                                 \
  ProcPtr = Ptr;                                                        \
  /* change pointers to misc routines */                                \
  ProcPtr->CopyPixToWin   = iggm_pixtowin/**/sfx/**/;                   \
  ProcPtr->CopyWinToPix   = iggm_wintopix/**/sfx/**/;                   \
  ProcPtr->CopyWinToWin   = iggm_wintowin/**/sfx/**/;                   \
  ProcPtr->CopyPixBpToWin = iggm_pixbptowin/**/sfx/**/;                 \
  ProcPtr->CopyWinBpToPix = iggm_winbptopix/**/sfx/**/;                 \
  ProcPtr->CopyWinBpToWin = iggm_winbptowin/**/sfx/**/;                 \
  ProcPtr->PolyPoint      = iggm_polypoint/**/sfx/**/;                  \
  ProcPtr->PutImage       = iggm_putimage/**/sfx/**/;                   \
  }

#define SET_LINE_PROCS(Ptr,sfx)                                         \
{                                                                       \
  g2dGCProcPtr ProcPtr;                                                 \
  ProcPtr = Ptr;                                                        \
  ProcPtr->PolyLines     = iggm_polylines/**/sfx/**/;                   \
  ProcPtr->PolySegment   = iggm_polysegment/**/sfx/**/;                 \
  ProcPtr->PolyArc       = iggm_polyarc/**/sfx/**/;                     \
  ProcPtr->PolyRectangle = iggm_polyrectangle/**/sfx/**/;               \
  }

#define SET_FILL_PROCS(Ptr,sfx)                                         \
{                                                                       \
  g2dGCProcPtr ProcPtr;                                                 \
  ProcPtr = Ptr;                                                        \
  ProcPtr->PolyFillRect = iggm_polyfillrect/**/sfx/**/;                 \
  ProcPtr->FillPolygon  = iggm_fillpolygon/**/sfx/**/;                  \
  ProcPtr->PolyFillArc  = iggm_polyfillarc/**/sfx/**/;                  \
  ProcPtr->PushPixels   = iggm_push_pix/**/sfx/**/;                     \
  }

#define SET_TEXT_PROCS(Ptr,sfx)                                         \
{                                                                       \
  g2dGCProcPtr ProcPtr;                                                 \
  ProcPtr = Ptr;                                                        \
  ProcPtr->ImageText8     = iggm_imagetext8/**/sfx/**/;                 \
  ProcPtr->PolyText8      = iggm_polytext8/**/sfx/**/;                  \
  ProcPtr->ImageText16    = iggm_imagetext16/**/sfx/**/;                \
  ProcPtr->PolyText16     = iggm_polytext16/**/sfx/**/;                 \
  }

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      READ_DATA(macro)                                                */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Moves data from a specified FIFO.  Assumes data is available    */
/*      and does not update the in-use reg of the fifo.                 */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      READ_DATA(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                  */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      char *  BUFP      - ptr to buffer being read from  FIFO         */
/*      int     LEN       - length of buffer to be read                 */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      BUFP                                                            */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/*    Defines used:                                                     */
/*      FIFO_P                                                          */
/*      FI                                                              */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
#define READ_DATA(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                  \
{                                                                       \
   /*-----------------------------------------------------------*/      \
   /* Check In-use register for FIFO.                           */      \
   /* If no room in FIFO, wait via SIGPAUSE.                    */      \
   /*-----------------------------------------------------------*/      \
                                                                        \
    GMBASE_INIT(seg_reg, pDevP);                                        \
    FUNNEL_INIT(FIFO_NUM, pDevP);                                       \
    IUR_INIT(ImmDataFifo, pDevP);                                       \
    read_data(FIFO_NUM, BUFP, LEN, pDevP) ;                             \
                                                                        \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      WRITE_DATA(macro)                                               */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Moves data to a specified FIFO.  Assumes space is available     */
/*      and does not update the in-use reg of the fifo.                 */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      WRITE_DATA(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                 */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      char *  BUFP      - ptr to buffer being read from  FIFO         */
/*      int     LEN       - length of buffer to be read                 */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      BUFP                                                            */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/*    Defines used:                                                     */
/*      FIFO_P                                                          */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
 
#define WRITE_DATA(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                 \
{                                                                       \
    GMBASE_INIT(seg_reg, pDevP);                                        \
    FUNNEL_INIT(FIFO_NUM, pDevP);                                       \
    write_data_bytes(FIFO_NUM, BUFP, LEN, pDevP) ;                      \
                                                                        \
}
  
/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      WTFIFO  (macro)                                                 */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Writes elements to a specified FIFO.                            */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      WTFIFO(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                     */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      char *  BUFP      - ptr to buffer to be written to FIFO         */
/*      int     LEN       - length of buffer to be written              */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      BUFP                                                            */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/*    Defines used:                                                     */
/*      FIFO_P                                                          */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
 
#define WTFIFO(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                     \
 {                                                                      \
                                                                        \
   /*-----------------------------------------------------------*/      \
   /* Ensure room in fifo                                       */      \
   /*-----------------------------------------------------------*/      \
  GMBASE_INIT(seg_reg, pDevP);                                          \
  FUNNEL_INIT(FIFO_NUM, pDevP);                                         \
  THRES_INIT(FIFO_NUM, pDevP);                                          \
  wtfifo(FIFO_NUM, BUFP, LEN, seg_reg, pDevP);                          \
                                                                        \
 }
 
/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      WTFIFO_SHORT (macro)                                            */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Writes elements to a specified FIFO using pointer & index.      */
/*      For use when direct addressing is more efficient than memcpy.   */
/*      Crossover is thought to be at 32 bytes.                         */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      WTFIFO_SHORT(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)               */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      char *  BUFP      - ptr to buffer to be written to FIFO         */
/*      int     LEN       - length of buffer to be written              */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      BUFP                                                            */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/*    Defines used:                                                     */
/*----------------------------------------------------------------------*/
 
 
#define WTFIFO_SHORT(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)               \
 {                                                                      \
                                                                        \
  WTFIFO(FIFO_NUM, BUFP, LEN, seg_reg, pDevP);                    	\
                                                                        \
 }
 
 
/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      WTFIFO_WORD, WRITE_BYTE, WRITE_CONTIG                           */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Special purpose algorithms for optimum transfer of data to      */
/*      a FIFO when the nature of the data is known at compile time.    */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      WRITE_xxxxxx(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)               */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      char *  BUFP      - ptr to buffer to be written to FIFO         */
/*      int     LEN       - length of buffer to be written              */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      BUFP                                                            */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
#define WRITE_WORD(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                 \
 {                                                                      \
                                                                        \
  WTFIFO(FIFO_NUM, BUFP, LEN, seg_reg, pDevP);                    	\
                                                                        \
 }
 
#define WRITE_BYTE(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)                 \
 {                                                                      \
    GMBASE_INIT(seg_reg, pDevP);                                        \
    FUNNEL_INIT(FIFO_NUM, pDevP);                                       \
    IUR_INIT(FIFO_NUM, pDevP);                                          \
    THRES_INIT(FIFO_NUM, pDevP);                                        \
    write_byte(FIFO_NUM, BUFP, LEN, seg_reg, pDevP);                    \
                                                                        \
 }
 
#define WRITE_CONTIG(FIFO_NUM, BUFP, LEN, seg_reg, pDevP)               \
 {                                                                      \
                                                                        \
  WRITE_BYTE(FIFO_NUM, BUFP, LEN, seg_reg, pDevP);                   	\
                                                                        \
 }

 

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      FIFO_AVAL   (macro)                                             */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Ensures that specified FIFO can handle specified number of bytes*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      FIFO_AVAL(FIFO_NUM, LEN, seg_reg, pDevP)                        */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      int     LEN       - # bytes needed in FIFO                      */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - device private area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      LEN                                                             */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/


/*----------------------------------------------------------------------*/
/*                                                                      */
/*    Defines used:                                                     */
/*      THRES_P                                                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
#define FIFO_AVAL(FIFO_NUM, LEN, seg_reg, pDevP)                        \
 {                                                                      \
   /*-----------------------------------------------------------*/      \
   /* Check In-use register for FIFO.                           */      \
   /* If no room in FIFO, wait via SIGPAUSE.                    */      \
   /*-----------------------------------------------------------*/      \
                                                                        \
                                                                        \
   GMBASE_INIT(seg_reg, pDevP);                                         \
   IUR_INIT(FIFO_NUM, pDevP);                                           \
   if(FifoLimit - (IURMASK & *(IUR_P[FIFO_NUM])) < LEN) {               \
     while(FifoLimit - (IURMASK & *(IUR_P[FIFO_NUM]))< LEN)             \
     {                                                                  \
     }                                                                  \
   }                                                                    \
}
 

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:                                                      */
/*      FIFO_EMPTY    (macro)                                           */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Ensures that FIFO is empty.                                     */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      FIFO_EMPTY(FIFO_NUM, seg_reg, pDevP)                            */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     -   device private area                       */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/


/*----------------------------------------------------------------------*/
/*                                                                      */
/*    Logic:                                                            */
/*      While FIFO not empty, set threshold to 0   ( meaning that       */
/*      an interrupt indicates FIFO is empty).                          */
/*      Issue sigpause and wait.                                        */
/*                                                                      */
/*    Defines used:                                                     */
/*      GSR_P                                                           */
/*      THRES_P                                                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
 
#define FIFO_EMPTY(FIFO_NUM, seg_reg, pDevP)                            \
 {                                                                      \
                                                                        \
   /*-----------------------------------------------------------*/      \
   /* Check In-use register for FIFO.                           */      \
   /* If FIFO not empty , wait via SIGPAUSE.                    */      \
   /*-----------------------------------------------------------*/      \
                                                                        \
     GMBASE_INIT(seg_reg, pDevP);                                       \
     IUR_INIT(FIFO_NUM, pDevP);                                         \
     while((IURMASK & *(IUR_P[FIFO_NUM]))> 0)                           \
     {                                                                  \
     }                                                  /*@E*/          \
   /*-----------------------------------------------------------*/      \
   /* FIFO is now empty. Continue.                              */      \
   /*-----------------------------------------------------------*/      \
                                                                        \
 }

/************************************************************************/
/*                                                                    @P*/
/*  FUNCTION NAME: DRAW_IN_WINDOW       (macro)                         */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Checks to see if the fill style/alu is such that we need to     */
/*      draw into the window plane                                      */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      DRAW_IN_WINDOW( P )                                             */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      P - pointer to GC attribute structure                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define DRAW_IN_WINDOW(P)                                               \
  (P->fillStyle != FillSolid &&                                         \
   (P->fillStyle == FillStippled ||                                     \
    P->alu != GXclear && P->alu != GXinvert && P->alu != GXset))

/************************************************************************/
/*                                                                    @O*/
/*  FUNCTION NAME: SET_WINDOW           (macro)                         */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up structure elements to be switch to GAI's window plane,  */
/*      and to clear the plane.                                         */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_WINDOW( BUF, ALU, WID )                                     */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF - is a setwin structure (defined in gsgmstr.h)              */
/*      ALU - a value to set the log op to                              */
/*      WID - a value to set the line width to                          */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define SET_WINDOW( BUF, ALU, WID )                             /*@Q@@VR*/\
{ (BUF).save.length = sizeof(save_buf);                                 \
  (BUF).save.opc = CE_SRSR;                                             \
  (BUF).save.fbmask = (~GAI_DRAW_PLANE & 0x000000ff);	                \
  (BUF).save.cmpmask = 0;                                               \
  (BUF).save.cmpval = 0;        /* irrelevant, since comp. mask = 0     */\
  (BUF).save.lincol = 0x000000ff;                                 /*@R@V*/\
  (BUF).save.lintype = LSSolid;                                         \
  (BUF).save.linwid = (WID);                                        /*@V*/\
  (BUF).save.endtyp = GM_FLAT;                                          \
  (BUF).save.intcol = 0x000000ff;                                 /*@R@V*/\
  (BUF).save.intst = GM_SOLID;                                        /**/\
  (BUF).save.intsti = GM_DONT_CARE;/* irrelevant, since int. style = solid */\
  (BUF).save.logop = (ALU);                                         /*@Q*/\
  (BUF).clear.len = sizeof(generic_se);                                 \
  (BUF).clear.opcode = CE_FCTL;                                         \
  (BUF).clear.data = FBC_FILL;  /* fill buffer with 0's             /*@S*/\
}


/************************************************************************/
/*                                                                    @O*/
/*  FUNCTION NAME: RESET_PLANE          (macro)                         */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure elements to restore after writing to      */
/*      GAI's window plane, and to set the appropriate bit in the       */
/*      compare mask and compare value to use GAI's window plane in     */
/*      underpaint.                                                     */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      RESET_PLANE( BUF )                                              */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF - is a write_with_window structure (defined in gsgmstr.h)   */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define RESET_PLANE( BUF )                                              \
{ (BUF).rlen = sizeof(long);                                            \
  (BUF).ropc = CE_RESR;                                                 \
  (BUF).slen = sizeof(generic_se);                                      \
  (BUF).sopc = CE_SBIT;                                                 \
  (BUF).mask = GAI_DRAW_PLANE      ;                                    \
}


/************************************************************************/
/*                                                                    @O*/
/*  FUNCTION NAME: END_WINDOW           (macro)                         */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets up the structure element to turn off underpaint for GAI's  */
/*      window plane.                                                   */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      END_WINDOW( BUF )                                               */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF - is a generic_se structure (defined in gsgmstr.h)          */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define END_WINDOW( BUF )                                               \
{ (BUF).len = sizeof(generic_se);                                       \
  (BUF).opcode = CE_RBIT;                                               \
  (BUF).data = (~GAI_DRAW_PLANE & 0x000000ff)      ;                    \
}


/************************************************************************/
/*                                                                    @X*/
/*  FUNCTION NAME: AIXGSC           (macro)                         	*/
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets the device dependant data and passes it as the 4th arg     */
/*      to aixgsc.  Upon return, it sets Aixgsc_rc, and increments      */
/*      the immediate mode fifo index.  The macro uses Aixgsc_pA        */
/*      as a pointer to the gAdapter structure.  The macro AIXGSC_DEF   */
/*      defines and initializes the required variables.                 */
/*                                                                      */
/*  INVOCATION:                                                         */
/*      AIXGSC_DEF(adapter pointer)                                     */
/*      AIXGSC( HANDLE, OPCODE, ARG )                                   */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      HANDLE - is a gsc_handle                                        */
/*      OPCODE - is which aixgsc function is being called               */
/*      ARG - is the pointer to the arguments passed for that function  */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      Immediate Fifo index                                            */
/*      Aixgsc_rc                                                       */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define AIXGSC( HANDLE, OPCODE, ARG )                   	        \
{	  								\
/* (*ARG).dd_data[0] = (ulong)Aixgsc_pA->gai.shmem ;		    /*@#*/\
 Aixgsc_rc = aixgsc( HANDLE, OPCODE, ARG );				\
                                                                        \
}


/************************************************************************/
/*                                                                    @X*/
/*  FUNCTION NAME: AIXGSC_DEF         (macro)                           */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Defines vars needed by AIXGSCxx macros.				*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      AIXGSC_DEF(adapter pointer) 					*/
/*      example:  AIXGSC_DEF(pWin->pAdapter)				*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      adapter pointer - the right hand side of an assignment		*/
/*              	statement that initializes a gAdapterPtr        */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      Aixgsc_rc                                                       */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define AIXGSC_DEF( ADAPTER_POINTER )	                                \
  int		Aixgsc_rc;						\
  gAdapterPtr	Aixgsc_pA = ADAPTER_POINTER;				\

/************************************************************************/
/*                                                                    @c*/
/*  FUNCTION NAME: UNDRAW_REGIONS         (macro)                       */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Fills clip regions of passed window geometry with NUM_HWIDS	*/
/*	on window planes.						*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      UNDRAW_REGIONS( BUF, PWG, NEW_PIX )				*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF	- The buffer to put SE's into (must be char *)		*/
/*	PWG	- Pointer to the window geometry with the clip regions	*/
/*		  to be "undrawn"					*/
/*	NEW_PIX	- The window id being cleared - used as a compare	*/
/*		  value when filling					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None								*/
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define UNDRAW_REGIONS( BUF,  PWG,  NEW_PIX )                            \
{									 \
									 \
  ((save_buf *)BUF)->length  = sizeof(save_buf);                         \
  ((save_buf *)BUF)->opc     = CE_SRSR;                                  \
  ((save_buf *)BUF)->fbmask  = (((~GAI_WIN_PLANES) & 0x000000ff));       \
  ((save_buf *)BUF)->cmpmask = (((GAI_WIN_PLANES)  & 0x000000ff));       \
  ((save_buf *)BUF)->cmpval  = (((NEW_PIX) & 0x000000ff));		 \
  ((save_buf *)BUF)->lincol  = ((PROTECT_HWID & 0x000000ff));		 \
  ((save_buf *)BUF)->lintype = LSSolid;                                  \
  ((save_buf *)BUF)->linwid  = itof(1.0);                                \
  ((save_buf *)BUF)->endtyp  = GM_FLAT;                                  \
  ((save_buf *)BUF)->intcol  = ((PROTECT_HWID & 0x000000ff));		 \
  ((save_buf *)BUF)->intst   = GM_SOLID;                                 \
  ((save_buf *)BUF)->intsti  = 1;                                        \
  ((save_buf *)BUF)->logop   = GM_REPLACE;                               \
  BUF += sizeof(save_buf);	         				 \
									 \
									 \
									 \
  ((rectangle *)BUF)->len         = sizeof(rectangle);                   \
  ((rectangle *)BUF)->opcode      = CE_IPLG;				 \
  ((rectangle *)BUF)->flags       = CONVEX;				 \
  ((rectangle *)BUF)->length      = 0x18;				 \
  ((rectangle *)BUF)->pt[0].x     = PWG->wg.winOrg.x;		         \
  ((rectangle *)BUF)->pt[0].y     = PWG->wg.winOrg.y;		         \
  ((rectangle *)BUF)->pt[1].x     = PWG->wg.winOrg.x+(PWG->wg.width-1);	 \
  ((rectangle *)BUF)->pt[1].y     = PWG->wg.winOrg.y;			 \
  ((rectangle *)BUF)->pt[2].x     = PWG->wg.winOrg.x+(PWG->wg.width-1);	 \
  ((rectangle *)BUF)->pt[2].y     = PWG->wg.winOrg.y+(PWG->wg.height-1); \
  ((rectangle *)BUF)->pt[3].x     = PWG->wg.winOrg.x;			 \
  ((rectangle *)BUF)->pt[3].y     = PWG->wg.winOrg.y+(PWG->wg.height-1); \
  ((rectangle *)BUF)->pt[4].x     = PWG->wg.winOrg.x;		         \
  ((rectangle *)BUF)->pt[4].y     = PWG->wg.winOrg.y;		         \
  BUF += sizeof(rectangle);	         				 \
									 \
									 \
  *((long *)BUF) = (0x00040000 | CE_RESR);                               \
  BUF += sizeof(long);				 			 \
}
 

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME:  ADD_INIT, FI_INIT, FUNNEL_INIT, GMBASE_INIT,        */
/*                  IUR_INIT, THRES_INIT                                */
/*                                                                      */
/*      DEFINES FOR BUSMEM_ATT & BUSMEM_DET IMPLEMENTATION              */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      AIX 4.1 requires the use of io_att and io_det instead           */
/*      of the BUSACC/RESTORE implementation.  Since we have            */
/*      no guarantee of which segment register we will have             */
/*      access to, these changes are required.  All of these            */
/*      variables are initialized in gem_ircx.c and we used             */
/*      that as the model for our changes                               */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      xxxx_INIT(FIFO_NUM, seg_reg)                                    */
/*      int     FIFO_NUM  - choices are 0,1,2 or 3.                     */
/*      long    seg_reg   - segment register number returned by         */
/*                            io_att                                    */
/*              pDevP     - private device area                         */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*      FIFO_NUM                                                        */
/*      seg_reg                                                         */
/*      pDevP                                                           */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      none                                                            */
/*                                                                      */
/************************************************************************/


#define GMBASE_INIT(seg_reg, pDevP)                                           \
 {                                                                            \
   pDevP->gmbase &= 0x0fffffff;                                               \
   pDevP->gmbase |= seg_reg;                                                  \
 }


#define ADD_INIT(FIFO_NUM, pDevP)                                             \
 {                                                                            \
  ulong add_offst;                                                            \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        add_offst = ADD0_ofst;                                                \
        break;                                                                \
     case 1:                                                                  \
        add_offst = ADD1_ofst;                                                \
        break;                                                                \
     case 2:                                                                  \
        add_offst = ADD2_ofst;                                                \
        break;                                                                \
     case 3:                                                                  \
        add_offst = ADD3_ofst;                                                \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->add_reg[FIFO_NUM] = (volatile ulong *) (pDevP->gmbase + add_offst);  \
 }

#define FI_INIT(FIFO_NUM, seg_reg, pDevP)                                     \
  GMBASE_INIT(seg_reg, pDevP);


#define FUNNEL_INIT(FIFO_NUM, pDevP)                                          \
 {                                                                            \
  ulong fun_offst;                                                            \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        fun_offst = FNL0_ofst;                                                \
        break;                                                                \
     case 1:                                                                  \
        fun_offst = FNL1_ofst;                                                \
        break;                                                                \
     case 2:                                                                  \
        fun_offst = FNL2_ofst;                                                \
        break;                                                                \
     case 3:                                                                  \
        fun_offst = FNL3_ofst;                                                \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->pFunnel[FIFO_NUM] = (volatile char *) (pDevP->gmbase + fun_offst);   \
 }


#define IP_INIT(FIFO_NUM, pDevP)                                              \
 {                                                                            \
  ulong ip_offst;                                                             \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        ip_offst = IP0_ofst;                                                  \
        break;                                                                \
     case 1:                                                                  \
        ip_offst = IP1_ofst;                                                  \
        break;                                                                \
     case 2:                                                                  \
        ip_offst = IP2_ofst;                                                  \
        break;                                                                \
     case 3:                                                                  \
        ip_offst = IP3_ofst;                                                  \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->ip_reg[FIFO_NUM] = (volatile ulong *) (pDevP->gmbase + ip_offst);    \
 }


#define IUR_INIT(FIFO_NUM, pDevP)                                             \
 {                                                                            \
  ulong iur_offst;                                                            \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        iur_offst = IUR0_ofst;                                                \
        break;                                                                \
     case 1:                                                                  \
        iur_offst = IUR1_ofst;                                                \
        break;                                                                \
     case 2:                                                                  \
        iur_offst = IUR2_ofst;                                                \
        break;                                                                \
     case 3:                                                                  \
        iur_offst = IUR3_ofst;                                                \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->iur_reg[FIFO_NUM] = (volatile ulong *) (pDevP->gmbase + iur_offst);  \
 }

#define TER_INIT(FIFO_NUM, pDevP)                                             \
 {                                                                            \
  ulong ter_offst;                                                            \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        ter_offst = TER0_ofst;                                                \
        break;                                                                \
     case 1:                                                                  \
        ter_offst = TER1_ofst;                                                \
        break;                                                                \
     case 2:                                                                  \
        ter_offst = TER2_ofst;                                                \
        break;                                                                \
     case 3:                                                                  \
        ter_offst = TER3_ofst;                                                \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->ter_reg[FIFO_NUM] = (volatile ulong *) (pDevP->gmbase + ter_offst);  \
 }

#define THRES_INIT(FIFO_NUM, pDevP)                                           \
 {                                                                            \
  ulong thres_offst;                                                          \
                                                                              \
  switch (FIFO_NUM)                                                           \
  {                                                                           \
     case 0:                                                                  \
        thres_offst = THR0_ofst;                                              \
        break;                                                                \
     case 1:                                                                  \
        thres_offst = THR1_ofst;                                              \
        break;                                                                \
     case 2:                                                                  \
        thres_offst = THR2_ofst;                                              \
        break;                                                                \
     case 3:                                                                  \
        thres_offst = THR3_ofst;                                              \
        break;                                                                \
  }                                                                           \
                                                                              \
  pDevP->thres_reg[FIFO_NUM] = (volatile ulong *) (pDevP->gmbase + thres_offst); \
 }
