/* @(#)79	1.1  src/bos/diag/tu/gem/dgsmon.h, tu_gem, bos411, 9428A410j 5/30/91 12:47:54 */
/*
 * COMPONENT_NAME: tu_gem (dgsmon.h) 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*  Attributes.   */
#define SOLIDINT   0x00000002
#define SOLIDLIN   0x00000001
#define EDGOFF     0x00000001

/*-------------------
	Values of color intensities to be initialised into the color table.
-------------------------*/

#define BLACK_DAT  	0x00
#define WHITE_DAT   	0x00ffffff
#define RED_DAT    	0x00ff0000
#define GREEN_DAT  	0x0000ff00
#define BLUE_DAT   	0x000000ff
#define YELLO_DAT  	0x00ffff00
#define MAGEN_DAT  	0x00ff00ff
#define CYAN_DAT   	0x0000ffff
#define GREY1_DAT  	0x00d6d6d6
#define GREY2_DAT  	0x00b5b5b5
#define GREY3_DAT  	0x00a5a5a5
#define GREY4_DAT  	0x00949494
#define GREY5_DAT  	0x00636363
#define GREY6_DAT  	0x00313131 

/*------------------
	Indices of each of the colors we are using. 
------------------------*/

#define BLACKI      0
#define WHITEI      1
#define REDI        2
#define GREENI      3
#define BLUEI       4
#define YELLOWI     5
#define MAGENTAI    6
#define CYANI       7
#define GRAY1I      8
#define GRAY2I      9
#define GRAY3I      10
#define GRAY4I      11
#define GRAY5I      12
#define GRAY6I      13

/*--------------------
   GM   FPGI Structure Element Codes (based on FPGI spec of 7/28/88)
----------------------------*/

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
 
 
/*----------------------
   GM   FPGI Command Element Codes (based on   GM   s/w spec 9/15/88 
        Assigned values to these based on new s/w/ spec. 
---------------------*/

#define CE_ACTC         0x01C0          /* activate context             */
#define CE_LDSV         0x01C1          /* load drawing state vector    */
#define CE_SDSV         0x01C2          /* store drawing state vector   */
#define CE_SDDS         0x01C3          /* set default drawing state    */
#define CE_STTT         0x01C4          /* set traversal type           */
#define CE_EPCO         0x01C5          /* end pick correlation         */
#define CE_SVWI         0x00D8          /* set view index               */
#define CE_RPAS         0x01C6          /* restore pick attrib state    */
#define CE_SYNC         0x01C7          /* synchronize                  */
#define CE_WRIT         0x0180          /* write pixel                  */
#define CE_READ         0x0181          /* read pixel                   */
#define CE_COPY         0x0182          /* VPM copy                     */
#define CE_CMPF         0x0184          /* compos fill-2 w/connectiv    */
#define CE_PLCI         0x0185          /* set polyline color - integ   */
#define CE_INCI         0x0186          /* set interior color - integ   */
#define CE_IPLY         0x0187          /* integer polyline             */
#define CE_IDPL         0x0188          /* integer disjoint polyline    */
#define CE_IPLG         0x0189          /* integer polygon              */
#define CE_SDFB         0x01C8          /* select drawing frame buffer  */
#define CE_FCTL         0x01C9          /* frame buffer control         */
#define CE_LBPC         0x01CA          /* load base planes color tabl  */
#define CE_LOPC         0x01CB          /* load overlay plane colr tab  */
#define CE_SCPM         0x01CC          /* set color processing mode    */
#define CE_LBWP         0x01CD          /* link base/window planes      */
#define CE_UBWP         0x01CE          /* unlink base/window planes    */
#define CE_FBC          0x01D9          /* set frame bfr comparison     */
                                        /*                              */
#define CE_WORG         0x01CF          /* set window origin            */
                                        /*                              */
#define CE_FMSK         0x018A          /* set frame buffer mask        */
#define CE_DMSK         0x01D0          /* set display mask             */
#define CE_LOGO         0x018B          /* set logicval op              */
#define CE_LINP         0x018C          /* reset line pattern           */
#define CE_PATR         0x018D          /* Pattern Reference Point      */
#define CE_EDOF         0x01D1          /* enable/disable other FIFO    */
#define CE_AECT         0x01D2          /* adjust element counter       */
#define CE_SRSR         0x01D3          /* save+replace selected regs   */
#define CE_RESR         0x01D4          /* restore selected registers   */
#define CE_SBIT         0x01D5          /* set bit                      */
#define CE_RBIT         0x01D6          /* reset bit                    */
#define CE_GMEM         0x01D7          /* set global memory            */
#define CE_GDMP         0x01D8          /* GCP dump                     */

/*---------------------------------
       hispd3d structure element codes 
-------------------------------------*/

#define CE_SDFBLN   0x000801C8             /* Select Drawing Frame Buffer*/
#define CE_FCTLLN   0x000801C9             /* frame buffer control       */
#define CE_ACTCLN   0x000801C0             /* activate context           */
#define CE_LBPCLN   0x000C01CA             /* load base plane color table*/
#define CE_SDDSLN   0x000401C3             /* Set Default Drawing State  */
#define CE_LOGOLN   0x0008018B             /* Set Logical Operation      */
#define CE_COPYLN   0x00240182             /* VPM to VPM Copy            */
#define CE_WRITLN   0x00240180             /* Write Pixel                */
#define SE_ISTYLN   0x00080018             /* Set Interior Style         */
#define SE_INCILN   0x00080186             /* Set Interior Color         */
#define CE_IPLGLN   0x00200189             /* Integer Polygon            */
#define SE_SWATLN   0x001001CF             /* Set Window Attributes      */
#define SE_FCTLDAT  0x000801C9             /* Frame control data         */
#define SE_PL3DAT   0x001C0101             /* Set polyline 3             */
#define SE_PG2      0x0122                 /* FPGI floating polygon 2    */
#define CE_WRIT     0x0180                 /* write pixel                */
#define CE_COPY     0x0182                 /* vpm to vpm copy            */
#define SE_INCI     0x0186                 /* Set Interior Color Index   */
#define CE_LOGO     0x018B                 /* set logical operation      */
#define CE_GMEM     0x01D7                 /* set global memory          */
#define CE_LOPC     0x01CB                 /* load overlay plane colr tab*/
#define SE_PLCD     0x0028                 /* Set polyline color direct */
#define CE_SDDS     0x01C3                 /* Set default drawing state */

/*--------------------------
         FPGI  Commands  Structures
--------------------------*/

/* Load base planes color table structure:  */
   struct loadbct
      { short int len0c;
        short int cmdlodbct;
        short int bctflag;
        short int ctid;
        long  int ctaddr; };
/* Set polyline color direct structure:   */
   struct setplycd
      { short int len10;
        short int cmdplycd;
        float plyred;
        float plygreen;
        float plyblue; };
/* Set polyline linetype structure:       */
   struct setlintyp
      { short int len08;
        short int cmdlintyp;
        long  int lintyp; };
/* Set polyline endtype structure:       */
   struct setendtyp
      { short int len08;
        short int cmdendtyp;
        long  int endtyp; };
/* Set linewidth scale factor structure: */
    struct setlinwid
      { short int len08;
        short int cmdlinwid;
        long  int linwid; };
/* Polyline for single line structure:    */
   struct plyline
      { short int len14;
        short int cmdpl2;
        float plyptx1;
        float plypty1;
        float plyptx2;
        float plypty2; };
/* Set interior color index structure:   */
   struct setintci
      { short int len08;
        short int cmdintci;
        unsigned int colori;  };
/* Set interior color direct structure:   */
   struct setintcd
      { short int len10;
        short int cmdintcd;
        float intred;
        float intgreen;
        float intblue; };
/* Set face distinguish mode structure:   */
   struct setfacemod
      { short int len08;
        short int cmdfacemod;
        long  int facemod; };
/* Set light calculation mode structure:  */
   struct setlitemod
      { short int len08;
        short int cmdlitemod;
        long  int litemod; };
/* Set interior style structure:          */
   struct setintsty
      { short int len08;
        short int cmdintsty;
        long  int intsty; };
/* Set edge flag structure:               */
   struct setedgfl
      { short int len08;
        short int cmdedgfl;
        long  int edgfl; };
/* Polygon-2 fill for rectangle structure: */
   struct fillrect2
      { short int len34;
        short int cmdpg2;
        short int gflag2;
        short int reserve2;
        long  int contlen2;
        float plg2ptx1;
        float plg2pty1;
        float plg2ptx2;
        float plg2pty2;
        float plg2ptx3;
        float plg2pty3;
        float plg2ptx4;
        float plg2pty4;
        float plg2ptx5;
        float plg2pty5; };
/* Polygon-3 fill for rectangle structure:    */
   struct fillrect3
      { short int len48;
        short int cmdpg3;
        short int gflag3;
        short int reserve3;
        long  int contlen3;
        float plg3ptx1;
        float plg3pty1;
        float plg3ptz1;
        float plg3ptx2;
        float plg3pty2;
        float plg3ptz2;
        float plg3ptx3;
        float plg3pty3;
        float plg3ptz3;
        float plg3ptx4;
        float plg3pty4;
        float plg3ptz4;
        float plg3ptx5;
        float plg3pty5;
        float plg3ptz5; };
/* Set frame buffer mask structure:       */
   struct setfrbmask
      { short int len08;
        short int cmdfrbmask;
        long  int frbmask; };
/* Set frame buffer compare structure:       */
   struct setfrbcomp
      { short int len08;
        short int cmdfrbcomp;
        long  int frbcomp; };

