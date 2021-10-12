/* @(#)94       1.3  src/bos/diag/tu/swmono/skytu.h, tu_swmono, bos411, 9428A410j 1/28/94 13:49:34 */
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: CatErr
 *		CatErrs
 *		CatMsgs
 *		CatMsgx
 *		MapStart
 *		Mapfmt
 *		Mapht
 *		Mapwth
 *		tan30
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>       /* for bus io file */
#include <sys/types.h>  

#define tan30(x)   ((x) * .57735)

#define byte     unsigned char
#define word     unsigned short
#define lword    unsigned long 
#define TRUE     1
#define FALSE    0
#define YES      1
#define NO       0
#define NIL      0 
#define BLANK    ' '

FILE *hist;
char ccc;

struct cntlparm
{  
   byte break_on_err; 
   byte verbose;     
   lword max_count; 
   lword err_count;
};
struct cntlparm *CPTR; 

#define TU_OPEN    0x90ff
#define TU_CLOSE   0xf0ff
#define ERR_LEVEL  0x7FFF
#define GOOD_OP    0x0000
#define SYS_FAILUR 0x8000
#define CANT_WRITE 0x8010
#define CANT_READ  0x8020
#define MOM_FAILUR 0x9000
#define LFTRCMOPEN 0x90AF
#define LFTDEVBUSY 0x90DB
#define LFTMAKEGP  0x90EE
#define LFTNOTOPEN 0x90FF
#define TU_FAILURE 0xA000
#define THISTUFAIL 0xA010
#define SUBTU_FAIL 0xA0F0
#define CPF        0xC000
#define CPF_INVDAT 0xC010
#define SKYWAYFAIL 0xE000
#define CANT_X_SKY 0xE010
#define RC_NOTFOUND 0xFFFF

#define BUFF_LEN   900
char LASTERR[BUFF_LEN];
char LASTMSG[BUFF_LEN];
#define GET_DEFAULT_PTRX 0x0000000


/*   TU Return IDs */       
lword SKYTU;       /* skytu() */
lword RUN_TU ;     /* runtu() */  
lword TU_OPEN_R;   /* 90ff TU_OPEN in skytu()*/
lword TU_CLOSE_R;  /* f0ff TU_CLOSE in skytu()*/

lword ALLREG;      /* a000() */
lword VPD;         /* a030() */
lword DADCR;       /* a080() */
lword OPMODE;      /* a081() */
lword VRAMWINDCNTL;/* a082() */
lword INTRPT_E;    /* a084() */
lword INTRPT_S;    /* a085() */
lword MEMCNTL;     /* a086() */
lword MEMINTSTAT;  /* a087() */
lword VRAMINDEX;   /* a088() */
lword MEMACCS;     /* a089() */
lword INDEX;       /* a08a() */
lword MCSRR;       /* a090() */
lword MEMCONF;     /* a091() */
lword AUTOCONF;    /* a092() */
lword CRTCHORZ;    /* a0a0() */
lword HORZTOT;     /* a0a1() */
lword HORZEND;     /* a0a2() */
lword HORZEBRD;    /* a0a3() */
lword HORZSBRD;    /* a0a4() */
lword HORZSYNS;    /* a0a5() */
lword HORZSYE1;    /* a0a6() */
lword HORZSYE2;    /* a0a7() */
lword CRTCVERT;    /* a0b0() */
lword VERTTOTL;    /* a0b1() */
lword VERTTOTH;    /* a0b2() */
lword VERTENDL;    /* a0b3() */
lword VERTENDH;    /* a0b4() */
lword VERTEBRDL;   /* a0b5() */
lword VERTEBRDH;   /* a0b6() */
lword VERTSBRDL;   /* a0b7() */
lword VERTSBRDH;   /* a0b8() */
lword VERTSYNSL;   /* a0b9() */
lword VERTSYNSH;   /* a0ba() */
lword VERTSYE;     /* a0bb() */
lword LINCMPL;     /* a0bc() */
lword LINCMPH;     /* a0bd() */
lword SPRITE;      /* a0c0() */
lword SPRITEOFF;   /* a0c5() */
lword MISC;        /* a0d0() */
lword STADDRLO;    /* a0d1() */
lword STADDRMI;    /* a0d2() */
lword STADDRHI;    /* a0d3() */
lword BUFFPILO;    /* a0d4() */
lword BUFFPIHI;    /* a0d5() */
lword DISPMOD1;    /* a0d6() */
lword DISPMOD2;    /* a0d7() */
lword MON_ID;      /* a0d8() */
lword SYSTEMID;    /* a0d9() */
lword CLK_FREQ;    /* a0da() */
lword BORD_COL;    /* a0db() */
lword SPR_PAL;     /* a0e0() */
lword INDEXLO;     /* a0e1() */
lword INDEXHI;     /* a0e2() */
lword PALDACCNTL;  /* a0e3() */
lword PAL_DATA;    /* a0e4() */

lword ALLVRAMEM;   /* b000() */
lword FILL00;      /* b010() */
lword FILLFF;      /* b020() */
lword FILL33;      /* b030() */
lword FILLCC;      /* b040() */
lword FILL55;      /* b050() */
lword FILLAA;      /* b060() */
lword FILLPAT;     /* b070() */
lword FILLWA;      /* b080() */
lword FILLLA;      /* b090() */

lword CoP_TEST;    /* c000() */
lword BRESDRAW0;   /* c030() */
lword BRESDRAW1;   /* c031() */
lword BRESDRAW2;   /* c032() */
lword STEPDRAW0;   /* c040() */
lword STEPDRAW1;   /* c041() */
lword STEPDRAW2;   /* c042() */
lword AREA_FILL;   /* c050() */ 
lword CCC_TEST;    /* c061() */
lword PLANEMASK;   /* c062() */
lword OCT_TEST;    /* c064() */
lword BPP4_TEST;   /* c065() */
lword H_SCROLL;    /* c068() */
lword BMSKTEST;    /* c0b2() */
lword MASKTEST;    /* c0b3() */
lword MEMXFER;     /* c0d0() */
lword INTLEVTEST;  /* c0e0() */

lword ALLTU;       /* e00a() */
lword THREE_COLOR; /* e00c() */
lword ALLEMCTU;    /* e00e() */
lword CLS_BLACK;   /* e00f() */
lword CROSHATCH11; /* e011() */
lword CROSHATCH12; /* e012() */
lword CROSHATCH13; /* e013() */
lword CROSHATCH14; /* e014() */

/* Primitive Return Codes */
lword INIT_COP;    /* p010() */
lword COPDONE;     /* p110() */ 
lword MAKEMAP;     /* p210() */
lword CLRSCR;      /* p220() */
lword MCLRSCR;     /* p221() */
lword SET_POR;     /* p510() */
lword SET_DSR;     /* p511() */
lword SETDMAPOR;   /* p520() */
lword STEPDRAWBOX; /* p610() */  
lword STEPDRAWOCT; /* p611() */ 
lword STEPDRAWCRS; /* p612() */
lword BRESBOX;     /* p620() */
lword HV304560;    /* p621() */
lword BRESCRS;     /* p622() */
lword BRES;        /* p6a0() */
lword S2DPXB;      /* pa10() */  
lword COLXPXB;     /* pa13() */  
lword S2DPXDMA;    /* pa20() */    
lword S2DFAST;     /* pa50() */   
lword PATT_FILL;   /* pa80() */

/* CRC Values */
lword C030CRC;      
lword C031CRC;      
lword C032CRC;      
lword C040CRC;      
lword C041CRC;      
lword C042CRC;
lword C050CRC;
lword C061CRC; 
lword C062CRC;  
lword C064CRC;  
lword C065CRC;       
lword C068CRC; 
lword C0B2CRC; 
lword C0B3CRC; 
lword E00CCRC; 
lword E011CRC;  
lword E012CRC;  
lword E013CRC;  
lword E014CRC;  
 
struct tu_info 
{  struct 
   {  char msg_buff[BUFF_LEN];
      char err_buff[BUFF_LEN];
   } info;
   struct A080
   {  byte op_mode;
      byte vramwctl; 
      byte intrpt_e; 
      byte intrpt_s; 
      byte memcntl; 
      byte memintstat;
      byte vramindx; 
      byte mem_accs; 
      byte index; 
   }  dadcr;
   struct A090
   {  byte memconfg;
      byte autoconf;
   } mcsrr; 
   struct A0A0_A0B0
   { byte horz_tot;
      byte horz_end; 
      byte horz_ebrd;
      byte horz_sbrd;
      byte horz_syncs;
      byte horz_synce1;
      byte horz_synce2;
      byte vert_totl;
      byte vert_toth;
      byte vert_endl; 
      byte vert_endh; 
      byte vert_ebrdl;
      byte vert_ebrdh;
      byte vert_sbrdl;
      byte vert_sbrdh;
      byte vert_syncsl;
      byte vert_syncsh;
      byte vert_synce;
      byte vert_lincmpl;
      byte vert_lincmph;
   } crtcdspl;
   struct A0D0
   { byte staddrlo; 
      byte staddrmi;
      byte staddrhi;
      byte buffpilo;
      byte buffpihi;
      byte dispmod1;
      byte dispmod2;
      byte systemid;
      byte clk_freq;
      byte bord_col;
   } misc;
   struct A0E0
   { byte indexlo; 
      byte indexhi;
      byte Bt_command;
      byte Bt_readmsk;
      byte Bt_blinkmsk;
      byte Bt_test;
   } spr_pal;
   struct B000
   { char runq[200];
   } vramem;
   struct B0x0
   { byte last_data;
     byte sa_topbot;
     byte rw16_32;
     lword memtop;
     lword membot;
     lword lastpattern;
     lword newpattern;
   } memory;
   int  skyway_rcm_fdes;
   char *skyway_ldn;
} tinfo;

#define CatErrs(x)   sprintf(tinfo.info.err_buff, "%s:%s", tinfo.info.err_buff, (x));
#define CatErr(x)    sprintf(tinfo.info.err_buff, "%s:%X", tinfo.info.err_buff, (x));
#define CatMsgs(x)   sprintf(tinfo.info.msg_buff, "%s %s", tinfo.info.msg_buff, (x));
#define CatMsgx(x)   sprintf(tinfo.info.msg_buff, "%s %X", tinfo.info.msg_buff, (x));
#define FUSE         if (rc > ERR_LEVEL) break;
#define CHECK        fflush(stdin);fscanf(stdin,"%c",&ccc);fflush(stdin);
#define Mapht(m)     ( ((m)==MAPA) ? MapAht :(((m)==MAPB) ? MapBht  : MapCht));
#define Mapwth(m)    ( ((m)==MAPA) ? MapAwth:(((m)==MAPB) ? MapBwth : MapCwth));
#define Mapfmt(m)    ( ((m)==MAPA) ? MapAfmt:(((m)==MAPB) ? MapBfmt : MapCfmt));
#define MapStart(m)  ( ((m)==MAPA) ? MapAorg:(((m)==MAPB) ? MapBorg : MapCorg));

volatile lword base;      /* refers to the base address of the CoP regs at the specified instance */
volatile lword vbase;     /* refers to the VRAM base address from the IO bus */
volatile lword skyvbase;  /* refers to the VRAM base address within the skyway card */
volatile lword iobase;    /* refers to the IO BASE address of the IO registers */

extern struct tinfo;
extern byte  instance;  /* refers to the instance # of the skyway card */

#define GETBASE            base   = getbaseadd();
#define GETIOBASE          iobase = getiobaseadd();
#define GETVBASE           vbase  = getvbaseadd();skyvbase = vbase & 0x0FFFFFFF;

#define OP_MODE_REG        (byte  *)(iobase + 0x00) 
#define VRAM_WNDW_CNTL_REG (byte  *)(iobase + 0x01) 
#define INT_EN_REG         (byte  *)(iobase + 0x04) 
#define INT_STAT_REG       (byte  *)(iobase + 0x05) 
#define VM_MEM_CNTL_REG    (byte  *)(iobase + 0x06) 
#define VM_INTRPT_STAT_REG (byte  *)(iobase + 0x07) 
#define VRAM_INDX_ADDR     (byte  *)(iobase + 0x08) 
#define MEM_ACCESS_MODE    (byte  *)(iobase + 0x09) 
#define INDX_ADDR          (byte  *)(iobase + 0x0A) 
#define DATAB_ADDR         (byte  *)(iobase + 0x0B) 
#define DATAW_ADDR         (word  *)(iobase + 0x0C) 
#define DATAL_ADDR         (lword *)(iobase + 0x0C) 
 
#define PG_DIR_BASE_ADDR   (lword *)(base + 0x00)
#define VM_ADDR            (lword *)(base + 0x04)
#define STATE_LEN_A        (byte  *)(base + 0x0E)
#define STATE_LEN_B        (byte  *)(base + 0x0F)
#define PIX_MAP_INDEX      (word  *)(base + 0x10)
#define PI_CNTL_REG        (byte  *)(base + 0x12)
#define PIX_MAP_BASE       (lword *)(base + 0x14) 
#define PIX_MAP_HT         (word  *)(base + 0x18) 
#define PIX_MAP_WTH        (word  *)(base + 0x1A) 
#define PIX_MAP_FMT        (word  *)(base + 0x1E) 
#define BRES_ERR_TERM      (lword *)(base + 0x20)
#define BRES_K1            (lword *)(base + 0x24)
#define BRES_K2            (lword *)(base + 0x28)
#define DIR_STEP_REG       (lword *)(base + 0x2C) 
#define COLOR_CMP_COND     (word  *)(base + 0x48) 
#define BGD_MIX            (byte  *)(base + 0x4A) 
#define FGD_MIX            (byte  *)(base + 0x4B) 
#define COLOR_CMP_VAL      (lword *)(base + 0x4C) 
#define PLANE_MASK         (lword *)(base + 0x50) 
#define CARRY_CHAIN_MASK   (lword *)(base + 0x54) 
#define FG_COLOR           (lword *)(base + 0x58) 
#define BG_COLOR           (lword *)(base + 0x5C) 
#define OP_DIM2            (word  *)(base + 0x60) 
#define OP_DIM1            (word  *)(base + 0x62) 
#define MASK_MAP_YOFFSET   (word  *)(base + 0x6C) 
#define MASK_MAP_XOFFSET   (word  *)(base + 0x6E) 
#define SOURCE_Y_PTR       (word  *)(base + 0x70) 
#define SOURCE_X_PTR       (word  *)(base + 0x72) 
#define PATTERN_Y_PTR      (word  *)(base + 0x74) 
#define PATTERN_X_PTR      (word  *)(base + 0x76) 
#define DESTINATION_Y_PTR  (word  *)(base + 0x78) 
#define DESTINATION_X_PTR  (word  *)(base + 0x7A) 
#define PIX_OP_REG         (lword *)(base + 0x7C) 


#define COL_CMP_TRUE      0x00      /* color compare coditions */
#define DEST_LTCCV        0x01
#define DEST_EQCCV        0x02                       
#define DEST_GTCCV        0x03
#define COL_CMP_OFF       0x04
#define DEST_LECCV        0x05
#define DEST_NECCV        0x06
#define DEST_GECCV        0x07
#define ZEROS             0x00      /* fore/back ground mix */
#define SRCandDEST        0x01
#define SRCandnotDEST     0x02
#define SRC               0x03
#define notSRCandDEST     0x04
#define DEST              0x05
#define SRCxorDEST        0x06
#define SRCorDEST         0x07
#define notSRCandnotDEST  0x08
#define SRCxornotDEST     0x09
#define notDEST           0x0A
#define SRCornotDEST      0x0B
#define notSRC            0x0C
#define notSRCorDEST      0x0D
#define notSRCnotDEST     0x0E
#define ONES              0x0F
#define MAXIMUM           0x10
#define MINIMUM           0x11
#define ADDSATURATE       0x12
#define DEST_SCR          0x13
#define SCR_DEST          0x14
#define AVERAGE           0x15

/* Pixel Operations Register Bit Field Values - see skyway wkbk p.84 7/1/88 */
#define BGC            0x00 /* BS and FS */
#define FGC            0x00
#define SPM            0x02
#define DSR            0x02 /* STEP */
#define LDR            0x03
#define DSW            0x04
#define LDW            0x05
#define PXBLT          0x08
#define IPXBLT         0x09
#define FPXBLT         0x0A
#define MASK           0x00 /*Source, Destination, Pattern, Mask Maps */
#define MAPA           0x01 
#define MAPB           0x02
#define MAPC           0x03
#define FIXED_PATTERN  0x08 /* Pattern */
#define GFS_PATTERN    0x09
#define MMD            0x00 /*Mask */
#define MMBE           0x01
#define MME            0x02
#define DAP            0x00 /* DM */
#define FPN            0x01
#define LPN            0x02
#define DAB            0x03
#define BPP1           0x08    /* Bits Per Pixel (Pixel Map Format) Codes */
#define BPP4           0x0A
#define BPP8           0x0B
#define WHITE          0x0000  /* Color Codes */
#define BLACK          0xFFFF
#define MOVE           0x00    /* Pixel Operation Register m_d codes */
#define DRAW           0x01
#define NWP            0x00    /* PxBlt Starting Corner Codes */
#define SWP            0x02
#define NEP            0x04
#define SEP            0x06
#define EASTS          0x00    /* Step and Draw direction Codes */
#define NES            0x01
#define NORTHS         0x02
#define NWS            0x03
#define WESTS          0x04 
#define SWS            0x05
#define SOUTHS         0x06
#define SES            0x07

byte CoP_Busy;/*AND- Tests the CoProcessor Busy bit of PI control reg */
byte Sus_Op;  /*OR-  Suspends the current PI operation */
byte UnSus_Op;/*AND- Unsuspends the current PI operation */
byte Op_Sus;  /*AND- Tests the Op Suspended bit of PI control reg */
byte Diag_SR; /*OR-  Sets Diagnos Save and Restore bit of PI Cntl Reg */
byte Reg_SR;  /*AND- Sets Regular Save and Restore bit of PI Cntl Reg */
byte Rstr;   /* OR-  Sets the Save bit of PI control reg */
byte Save;   /* AND- Sets the Restore bit of PI control reg */


word MapAht,MapAwth,MapAfmt;/*Ext vars to use in place of nonreadable CoP Regs*/
word MapBht,MapBwth,MapBfmt;/*ext vars to use in place of nonreadable CoP Regs*/
word MapCht,MapCwth,MapCfmt;/*ext vars to use in place of nonreadable CoP Regs*/
lword *MapAorg,*MapBorg,*MapCorg;/*ext vars to use in place of nonreadable CoP Regs*/

#ifndef dummy
extern ulong segreg;
extern lword fd;
#endif

lword black,white,red,green,blue,purple,brown,yellow,grey,ltgren,ltcyan,pink;
lword sblack,swhite,sred,sgreen,sblue,spurple,sbrown,syellow;

int retract;
