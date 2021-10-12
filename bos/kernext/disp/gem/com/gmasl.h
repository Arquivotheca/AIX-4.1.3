/* @(#)79	1.4.1.9  src/bos/kernext/disp/gem/com/gmasl.h, sysxdispgem, bos411, 9428A410j 1/22/93 09:19:09 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
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




  /* View Table							      @1*/
  typedef struct vwtbl

    {
	long    vwtindx;                /* View Table Index             */
	ushort  vopflg;                 /* Viewing operation flags      */
	char    rsvd1[2];      		/* ---- pad ----                */
#ifdef _KERNEL
	ulong	npczclip[2];		/* Z Clipping Boundaries in NPC	*/
	ulong	truvp[6];   		/* True Viewport           	*/
	ulong	vtm[16];		/* Viewing Transformation Matrix*/
	ulong	prp[3];     		/* Projection Reference Point	*/
	ulong	vpd;        		/* View Plane Distance      	*/
	ulong	truw2vr[3];		/* True Window to Viewport	*/
	    				/*   Ratio			*/
	ulong	truiw2vr[3];		/* True Inverse Window to	*/
	    				/*   Viewport Ratio		*/
	ulong	truwvc[6];		/* True Window in VC		*/
	ulong	shearparm[3];		/* Shearing Parameters		*/
#else
	float	npczclip[2];		/* Z Clipping Boundaries in NPC	*/
	float	truvp[6];   		/* True Viewport           	*/
	float	vtm[16];		/* Viewing Transformation Matrix*/
	float	prp[3];     		/* Projection Reference Point	*/
	float	vpd;        		/* View Plane Distance      	*/
	float	truw2vr[3];		/* True Window to Viewport	*/
	    				/*   Ratio			*/
	float	truiw2vr[3];		/* True Inverse Window to	*/
	    				/*   Viewport Ratio		*/
	float	truwvc[6];		/* True Window in VC		*/
	float	shearparm[3];		/* Shearing Parameters		*/
#endif
    } VWTBL;

  /* Drawing State Vector					      @1*/
  /*                                                                  @1*/
  /*      field naming convention:                                    @1*/
  /*            either three letters for each descriptive word        @1*/
  /*            or     one letter for each descriptive word           @1*/
  /*                   (e.g., intcoli   for Interior Color Integer    @1*/
  typedef struct drstvc

    {
	char    dsvrsvd1[272];          /* ---- pad ----                */
#ifdef _KERNEL
	ulong   lgvtm[12];              /* Local*Global*View Transfor-  */
					/*   mation Matrix		*/
#else
	float   lgvtm[12];              /* Local*Global*View Transfor-  */
#endif
	char    dsvrsvd2[12];           /* ---- pad ----                */
	long    plylincoli;             /* Polyline Color(RGB) Integer  */
	char    dsvrsvd3[60];           /* ---- pad ----                */
	long    intcoli;                /* Interior Color(RGB) Integer  */
	char    dsvrsvd4[204];          /* ---- pad ----                */
	long    curlintyp;              /* Current Line Type            */
#ifdef _KERNEL
	ulong   curlinwid;              /* Current Line Width           */
#else
	float   curlinwid;              /* Current Line Width           */
#endif
	char    dsvrsvd5[24];           /* ---- pad ----                */
	long    curintsty;              /* Current Interior Style       */
	long    curintstyind;           /* Current Interior Style Indx  */
	char    dsvrsvd6[208];          /* ---- pad ----                */
	long    curvwtbli;              /* Current View Table Index     */
	char    dsvrsvd7[68];           /* ---- pad ----                */
	long    frbufmsk;               /* Frame Buffer Mask            */
	char    dsvrsvd8[8];            /* ---- pad ----                */
	long    dispmsk;                /* Display Mask(Base Planes)    */
	long    dsvlogop;               /* Logical Operation            */
	long    colpromoddat;           /* Color Processor Mode Data    */
	char    dsvrsvd9[4];            /* ---- pad ----                */
	long    dsvflgs;                /* Flags                        */
					/*   Bits 31-4 -  Reserved      */
					/*         0   -  Draw Control  */
					/*         1   -  View Parms    */
					/*                  valid       */
					/*         3-2 -  Filter Flags  */
	char    dsvrsvd10[1092];        /* ---- pad ----                */
    } DRSTVC;

  /* View Addressing Table						*/
  typedef struct vwadrtb

    {
	off_t  vto;                     /* View Table Offset            */
    } VWADRTB;

  /* Pattern Table							*/
  typedef struct pattbl

    {
	short	patszx;			/* Pattern Size X Dimension	*/
	short	patszy;			/* Pattern Size Y Dimension	*/
	long    datbtsz;                /* Data Bit Size(8 or 32)       */
	off_t   pato;                   /* Pattern Array Table Offset   */
	off_t   cicpato;                /* Converted Integer Color      */
					/* Pattern Array Table Offset   */
    } PATTBL;

  /* Hatch Table                                                        */
  typedef struct harep

    {
	short   hatszx;                 /* Hatch Size X Dimension       */
	short   hatszy;                 /* Hatch Size Y Dimension       */
	off_t   hatdatao;               /* Hatch Data Offset            */
    } HAREP;

  /* Line Type Table							*/
  typedef struct lprep

    {
	long    nsegs;                  /* Number of Segments           */
	short	nunits[8];		/* Number of Units in each	*/
					/*   Segment, 1-8		*/
    } LPREP;

  /* Resource Management Info						*/
  typedef struct rminfo
    {
					/********************************/
					/* RM CHANGE INDICATORS		*/
	long    rmchngind;              /* Modification Bits            */
					/* Bit14= WindoPln Attr Changed */
					/* Bit16= Windo Attribs Changed */
					/********************************/
					/* WINDOW PLANES MASKS/VALUES	*/
					/*   (BITS 7-0)			*/
	long    wfrmbufmsk;             /* Frame Buffer Mask            */
	long    wcmpmask;               /* Compare Mask                 */
	long    wcmpval;                /* Compare Value(Drawing Windo  */
					/*   ID)			*/
	long    polylclr;               /* Polyline Color               */
	long    intclr;                 /* Interior Color               */
	long    wlogop;                 /* Logical Operation            */
					/********************************/
					/* WINDOW ATTRIBUTES		*/
#ifdef _KERNEL
	ulong   windorgx;               /* Window Origin(X Coordinate)  */
	ulong   windorgy;               /* Window Origin(Y Coordinate)  */
	ulong	windwidth;		/* Window Width -1		*/
	ulong	windheight;		/* Window Height -1		*/
#else
	float   windorgx;               /* Window Origin(X Coordinate)  */
	float   windorgy;               /* Window Origin(Y Coordinate)  */
	float	windwidth;		/* Window Width -1		*/
	float	windheight;		/* Window Height -1		*/
#endif
	long    woflgs;                 /* Flags                        */
					/* Bit 0 = Link Base/Window	*/
					/*   Planes			*/
					/* Bit 1-2 = Reserved		*/
					/* Bit 4-5 = Drawing Frame	*/
					/*   Buffer Type		*/
					/* Bit 6-7 = Reserved		*/
					/* Bit 8 = Echo Traversal	*/
					/* Bit 9 = Single View mode	*/
					/* Bit 10 = Base Planes Logical	*/
					/*   Display Buffer		*/
					/*     0 = A			*/
					/*     1 = B			*/
					/* Bit 11 = Base Planes Logical	*/
					/*   Update Buffer		*/
					/* Bit 12 = Overlay Planes	*/
					/*   Logical Display Buffer	*/
					/* Bit 13 = Overlay Planes	*/
					/*   Logical Update Buffer	*/
					/* Bit 14 = Bit 10 XOR Bit 11	*/
					/* Bit 15 = Bit 12 XOR Bit 13	*/
					/* Bit 16-31 = Reserved		*/
					/********************************/
	long	numpikatst;		/* # Pick Attrib States		*/
	long	pikdetcntr;		/* Pick Detect Counter		*/
	long	pikrepsiz;		/* Size of Pick Report in Bytes	*/
	long	flushtag;		/* Flush Tag in 'Flush Mode'	*/
					/********************************/
    } RMINFO;

  /* Adapter State List                                               @1*/
  typedef struct 

    {					/********************************/
	off_t	prminfo;		/* Pointer Resource Info	*/
	off_t    dsvo;                  /* Offset  to Drawing State     */
					/*   Vector			*/
					/********************************/
					/* VIEW TABLE SETTINGS		*/
	long    nviewtab;               /* Number of View Tables        */
	off_t   viewato;                /* Offset  to View Addressing   */
					/********************************/
	                                /* BUNDLE TABLE SETTINGS      @A*/
        long    npylnbtb;               /* Number of Polyline bundle tbl*/
        off_t   pylnbtbo;               /* Offset to Polyline bundle tbl*/
        long    npymkbtb;               /* Number of Polymarker bun tbl */
        off_t   pymkbtbo;               /* Offset to Polymarker bun tbl */
        long    ntextbtb;               /* Number of Text bundle table  */
        off_t   textbtbo;               /* Offset to Text bundle table  */
        long    nintrbtb;               /* Number of Interior bun tbl   */
        off_t   intrbtbo;               /* Offset to Interior bun tbl   */
        long    nedgebtb;               /* Number of Edge bundle table  */
        off_t   edgebtbo;               /* Offset to Edge bundle table  */
					/********************************/
					/* OTHER TABLE SETTINGS		*/
	long    npattbl;                /* Number of Pattern Table      */
					/*   Entries			*/
	off_t   pattao;                 /* Offset  to Pattern Table     */
	long    nhattbl;                /* Number of Htach Table        */
					/*   Entries			*/
	off_t   htblo;                  /* Offset  to Hatch Table       */
	long    nlntbl;                 /* Number of Line Type Table    */
					/*   Entries			*/
	off_t   lnptbo;                 /* Offset  to Line Type Table   */
	off_t   lnprtbo;                /* Offset  to Line Type rend tbl */
        long    nmktytb;                /* Number of Marker type tbl  @A*/ 
        off_t   mktytbo;                /* Offset to Marker type tbl  @A*/
        long    ncullsztb;              /* Number of Cull size table  @A*/
        off_t   cullsztbo;              /* Offset to Cull size table  @A*/
        long    ndepthcutb;             /* Number of Depth cue table  @A*/ 
        off_t   depthcutbo;             /* Offset to Depth cue table  @A*/
        long    nlghtsrctb;             /* Number of Light src table  @A*/
        off_t   lghtsrctbo;             /* Offset to Light src table  @A*/
                                        /* Color Processing-Nrm Trav    */
        long    nclrproctbnrm;          /* Number of Color proc tbl   @A*/
        off_t   clrproctbonrm;          /* Offset to Color proc tbl   @A*/
                                        /* Color Processing-Echo Trav   */
        long    nclrproctbech;          /* Number of Color Proc tbl   @A*/
        off_t   clrproctboech;          /* Offset to Color Proc tbl   @A*/
        long    nlogclrtb;              /* Number of Logical color tbl@A*/
        off_t   logclrtbo;              /* Offset to Logical color tbl@A*/ 
                                        /********************************/    
        off_t   pkcorcntlblk;           /* Offset to Pick Correlation @A*/
                                        /* Control Block                */
                                        /********************************/
        off_t   pkinvhlghtfl;           /* Offset to Pick, Invisibility */
                                        /* and Highlighting Filter tbl@A*/
					/********************************/
					/* OFFSETS TO VARIOUS WORK AREAS*/
	off_t   hatwkao;                /* Offset  to Hatch Work Area   */
        off_t   trsurwkao;              /* Offset to Trim Surface     @A*/
                                        /* Work Area                    */ 
        off_t   shpwkao;                /* Offset to Shp Work Area    @A*/
                                        /********************************/
        long    rsvd1;                  /* Rsvd for Image Card Supp   @A*/
        long    rsvd2;                  /* Rsvd for Image Card Supp   @A*/
        long    rsvd3;                  /* Rsvd for Image Card Supp   @A*/
                                        /********************************/
                                        /* GEOMETRIC TEXT TABLE         */
        off_t   langlktbo;              /* Offset to Language Lookup  @A*/   
        long    nalang;                 /* National Language(Default  @A*/     
                                        /* CSID)                        */
                                        /********************************/
                                        /* ECHO VIEW TABLE SETTINGS     */
        long    nechvwtb;               /* Number of Echo view table  @A*/
        off_t   echvwtbo;               /* Offset to Echo view table  @A*/
					/********************************/
					/* CHANGE INDICATORS		*/
	long    chngind;                /* Modification Bits            */
					/* Bit 2= Line Type Changed	*/
					/* Bit 4= Pattern Changed	*/
					/* Bit 5= Hatch Changed		*/
					/* Bit14= WindoPln Attr Changed */
					/* Bit15= OverlPln Attr Changed */
					/* Bit17= View Table(s) changed */
					/* Bit18= Gtxt Cul Meth changed	*/
					/********************************/
                                        /* LOGICAL COLOR TABLE          */
                                        /* (max support is 256 entries) */
        short   clrtblbg;               /* color table begin change   @A*/ 
        short   clrtblend;              /* color table end change     @A*/
					/********************************/
					/* CHANGE BIT MAPS		*/
	long    lntblchngbm;            /* Line Type Table Entries      */
					/*   Changed Bit Map		*/
	long    pattblchngbm;           /* Pattern Table Changed Bit    */
					/*   Map			*/
	long    hattblchngbm;           /* Hatch Table Changed Bit Map  */
					/********************************/
					/* OVERLAY PLANES MASKS?VALUES	*/
					/*   (BITS 3-0)			*/
	long    ofrmbufmsk;             /* Frame Buffer Mask            */
	long    ocmpmsk;                /* Compare Mask                 */
	long    ocmpval;                /* Compare Value(Overlay        */
					/*   Drawing Window)		*/
	long    dispmsk;                /* Display Mask                 */
	long    ologop;                 /* Logical Operation            */
					/********************************/

#ifdef _KERNEL
	ulong   woanntxtratio;          /* Ann Txt NCP-DC Ratio         */
        off_t   vwtbsgmdo;              /* Offset to View table for   @A*/
                                        /* Single View Mode             */
	ulong	lwdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Line Width		*/
	ulong	pmdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Polymarker size	*/
	ulong	athsfdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Annot Text Height	*/
					/*   Scale Factor		*/
					/********************************/
#else
	float   woanntxtratio;          /* Ann Txt NCP-DC Ratio         */
        off_t   vwtbsgmdo;              /* Offset to View table for   @A*/
                                        /* Single View Mode             */
	float	lwdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Line Width		*/
	float	pmdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Polymarker size	*/
	float	athsfdcwcrat;		/* DC to Window Coordinate Ratio*/
					/*   for Annot Text Height	*/
					/*   Scale Factor		*/
					/********************************/
#endif
	long	gtxtculdspmth;		/* Geometric Text Culling	*/
					/*  Display Method		*/
#ifdef _KERNEL
	ulong	gtxtculhgt;		/* Geometric Text Culling Height*/
#else
	float	gtxtculhgt;		/* Geometric Text Culling Height*/
#endif

      } ASL;				

  /*------------------------------------------------------------------  */
  /* Immediate FIFO Context structure.                                  */
  /*                                                                    */
  /*------------------------------------------------------------------  */

  typedef struct rcm_imm_context {      /********************************/
	ASL     asl;                    /* Adapter State List           */
	DRSTVC  dsv;                    /* Drawing State Vector         */
	VWTBL   vt;                     /* View Table                   */
        PATTBL  pt[MaxPatterns];	/* Pattern Table (none)         */
	HAREP   ht[GAIMaxHatchs];       /* Hatch Table                  */
        char    pat[32*32*4*MaxPatterns];/*pattern array table-         */
					/* len*wide*(max deep)*(max #)  */
	ulong   hd[32*GAIMaxHatchs];    /* hatch data                   */
	ulong   opct[MaxOPlanesColor];  /* Overlay Planes Color Table   */
	off_t   vat[4];                 /* View Addressing Table      @2*/
					/********************************/
	RMINFO  rminfo;			/* Resource Management Info 	*/
    } IMM_CONTEXT;


  /*------------------------------------------------------------------  */
  /* Traversal FIFO Context structure.                                  */
  /*                                                                    */
  /*------------------------------------------------------------------  */

  typedef struct rcm_trav_context {                                     /*@A*/
	ASL     asl;                    /* Adapter State List           */
	RMINFO  rminfo;			/* Resource Management Info 	*/
	VWTBL   vt;                     /* View Table                   */
	off_t   vat[4];                 /* View Addressing Table        */
    } TRAV_CONTEXT;



typedef struct trav_slot {
	ASL     asl;                    /* Adapter State List           */
	uchar	buffer[99*1024];	/* Scratch buffer		*/
	RMINFO  rminfo;			/* Resource Management Info 	*/
	uchar	pick_buffer[32*1024];	/* Pick save area		*/
	uchar	dsv_save_area[2*1024];	/* DSV save area for CXT Switch */
      } TRAV_SLOT;

typedef struct imm_slot {
  IMM_CONTEXT	imm_cxt;
}  IMM_SLOT;
