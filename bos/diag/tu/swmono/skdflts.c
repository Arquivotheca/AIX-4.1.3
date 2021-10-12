static char sccsid[] = "@(#)92  1.3  src/bos/diag/tu/swmono/skdflts.c, tu_swmono, bos411, 9428A410j 1/28/94 13:49:25";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: skydflts
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
#include "skytu.h"

skydflts()
{

  GETVBASE;

  sprintf(tinfo.vramem.runq,"%s",
  "16bb110 16tb120 32bb130 32tb140 16tb150 16tb160 16tb170 32bb170 16bb170 32tb180 16bb190");
  tinfo.memory.rw16_32 = 32;
  tinfo.memory.sa_topbot = 'T';
  tinfo.memory.last_data = 0x00;
  tinfo.memory.memtop = skyvbase;
  tinfo.memory.membot = skyvbase+0x0A0000;

  tinfo.memory.newpattern = 0x80808080;
  tinfo.memory.lastpattern = 0x20202020;

  /*  Direct Access Display Control Registers (dadcr) */
  tinfo.dadcr.op_mode =  0x0C;  /* Native, Motorola Mode */
  tinfo.dadcr.vramwctl = 0x00;  /* No Window, Direct Access to 4MB of VRAM */
  tinfo.dadcr.intrpt_e = 0x00;  /* Base+4:disable interrupts              */
  tinfo.dadcr.intrpt_s = 0x00;  /* Base+5:Write of '1'b clears the interrupt */
  tinfo.dadcr.memcntl  = 0x00;  /* Base+6: */
  tinfo.dadcr.memintstat=0x00;  /* Base+7:Clear all interrupts */
  tinfo.dadcr.vramindx = 0x00;  /* N/A due to vramcntl=0 */
  tinfo.dadcr.mem_accs = 0x0A;  /* 8bit, Motorola  */
  tinfo.dadcr.index    = 0x0A;  /* Base+A */

  /*  Indirect Access Display Control Registers */
  tinfo.mcsrr.memconfg = 0x96;  /* 64bSerializer, 256x4k, 32bSerWidth */ 
  tinfo.mcsrr.autoconf = 0x01;  /* expecting a 32bit LEPB */

  tinfo.crtcdspl.horz_tot    = 0xE1;/* 10 */
  tinfo.crtcdspl.horz_end    = 0x9F;/* 12 */
  tinfo.crtcdspl.horz_ebrd   = 0x9F;/* 14 */
  tinfo.crtcdspl.horz_sbrd   = 0x9F;/* 16 */
  tinfo.crtcdspl.horz_syncs  = 0xB4;/* 18 */
  tinfo.crtcdspl.horz_synce1 = 0xD4;/* 1A */
  tinfo.crtcdspl.horz_synce2 = 0x1B;/* 1C */
  tinfo.crtcdspl.vert_totl   = 0x1F;/* 20 */
  tinfo.crtcdspl.vert_toth   = 0x04;/* 21 */
  tinfo.crtcdspl.vert_endl   = 0xFF;/* 22 */
  tinfo.crtcdspl.vert_endh   = 0x03;/* 23 */
  tinfo.crtcdspl.vert_ebrdl  = 0xFF;/* 24 */
  tinfo.crtcdspl.vert_ebrdh  = 0x03;/* 25 */
  tinfo.crtcdspl.vert_sbrdl  = 0x1F;/* 26 */
  tinfo.crtcdspl.vert_sbrdh  = 0x04;/* 27 */
  tinfo.crtcdspl.vert_syncsl = 0xFF;/* 28 */
  tinfo.crtcdspl.vert_syncsh = 0x03;/* 29 */
  tinfo.crtcdspl.vert_synce  = 0x08;/* 2A */
  tinfo.crtcdspl.vert_lincmpl= 0xFF;/* 2C */
  tinfo.crtcdspl.vert_lincmph= 0x07;/* 2D */

  tinfo.misc.staddrlo = 0x00;       
  tinfo.misc.staddrmi = 0x00;        
  tinfo.misc.staddrhi = 0x00;        
  tinfo.misc.buffpilo = 0x50;         
  tinfo.misc.buffpihi = 0x00;          
  tinfo.misc.dispmod1 = 0xC7;           
  tinfo.misc.dispmod2 = 0x02;            
  tinfo.misc.clk_freq = 0x02;
  tinfo.misc.systemid = 0x10;   
  tinfo.misc.bord_col = 0x00;            

  tinfo.spr_pal.indexlo    = 0x09; 
  tinfo.spr_pal.indexhi    = 0x00;      
  tinfo.spr_pal.Bt_command = 0x43;     
  tinfo.spr_pal.Bt_readmsk = 0x0F;     
  tinfo.spr_pal.Bt_blinkmsk= 0x00;     
  tinfo.spr_pal.Bt_test    = 0x00;     


/*   TU Return IDs              */
SKYTU         = 0x10000000;
RUN_TU        = 0x10440000;       
TU_OPEN_R     = 0x90FF0000;
TU_CLOSE_R    = 0xF0FF0000;

ALLREG        = 0xA0000000;
DADCR         = 0xA0800000;
OPMODE        = 0xA0810000;
VRAMWINDCNTL  = 0xA0820000;
INTRPT_E      = 0xA0840000;
INTRPT_S      = 0xA0850000;
MEMCNTL       = 0xA0860000;
MEMINTSTAT    = 0xA0870000;
VRAMINDEX     = 0xA0880000;
MEMACCS       = 0xA0890000;
INDEX         = 0xA08A0000;
MCSRR         = 0xA0900000;
MEMCONF       = 0xA0910000;
AUTOCONF      = 0xA0920000;
CRTCHORZ      = 0xA0A00000;
HORZTOT       = 0xA0A10000;
HORZEND       = 0xA0A20000;
HORZEBRD      = 0xA0A30000;
HORZSBRD      = 0xA0A40000;
HORZSYNS      = 0xA0A50000;
HORZSYE1      = 0xA0A60000;
HORZSYE2      = 0xA0A70000;
CRTCVERT      = 0xA0B00000;
VERTTOTL      = 0xA0B10000;
VERTTOTH      = 0xA0B20000;
VERTENDL      = 0xA0B30000;
VERTENDH      = 0xA0B40000;
VERTEBRDL     = 0xA0B50000;
VERTEBRDH     = 0xA0B60000;
VERTSBRDL     = 0xA0B70000;
VERTSBRDH     = 0xA0B80000;
VERTSYNSL     = 0xA0B90000;
VERTSYNSH     = 0xA0BA0000;
VERTSYE       = 0xA0BB0000;
LINCMPL       = 0xA0BC0000;
LINCMPH       = 0xA0BD0000;
SPRITE        = 0xA0C00000;
SPRITEOFF     = 0xA0C50000;
MISC          = 0xA0D00000;
STADDRLO      = 0xA0D10000;
STADDRMI      = 0xA0D20000;
STADDRHI      = 0xA0D30000;
BUFFPILO      = 0xA0D40000;
BUFFPIHI      = 0xA0D50000;
DISPMOD1      = 0xA0D60000;
DISPMOD2      = 0xA0D70000;
MON_ID        = 0xA0D80000;
SYSTEMID      = 0xA0D90000;
CLK_FREQ      = 0xA0DA0000;
BORD_COL      = 0xA0DB0000;
SPR_PAL       = 0xA0E00000;
INDEXLO       = 0xA0E10000;
INDEXHI       = 0xA0E20000;
PALDACCNTL    = 0xA0E30000;
PAL_DATA      = 0xA0E40000;
VPD           = 0xA0F00000;

ALLVRAMEM     = 0xB0000000;
FILL00        = 0xB0100000;
FILLFF        = 0xB0200000;
FILL33        = 0xB0300000;
FILLCC        = 0xB0400000;
FILL55        = 0xB0500000;
FILLAA        = 0xB0600000;
FILLPAT       = 0xB0700000;
FILLWA        = 0xB0800000;
FILLLA        = 0xB0900000;

CoP_TEST      =0xC0000000;
BRESDRAW0     =0xC0300000; 
BRESDRAW1     =0xC0310000;  
BRESDRAW2     =0xC0320000;   
STEPDRAW0     =0xC0400000;    
STEPDRAW1     =0xC0410000;
STEPDRAW2     =0xC0420000; 
AREA_FILL     =0xC0500000;    
CCC_TEST      =0xC0610000;   
PLANEMASK     =0xC0620000;   
OCT_TEST      =0xC0640000;   
BPP4_TEST     =0xC0650000;   
H_SCROLL      =0xC0680000;   
BMSKTEST      =0xC0B20000;  
MASKTEST      =0xC0B30000;  
MEMXFER       =0xC0D00000;  
INTLEVTEST    =0xC0E00000;

ALLTU         = 0xE00A0000;
THREE_COLOR   = 0xE00C0000;
ALLEMCTU      = 0xE00E0000;
CLS_BLACK     = 0xE00F0000;
CROSHATCH11   = 0xE0110000;
CROSHATCH12   = 0xE0120000;
CROSHATCH13   = 0xE0130000;
CROSHATCH14   = 0xE0140000;

/* Primitive Return Codes */
INIT_COP      =0xF0100000;
COPDONE       =0xF1100000;
MAKEMAP       =0xF2100000;
CLRSCR        =0xF2200000;
MCLRSCR       =0xF2210000;
SET_POR       =0xF5100000; 
SET_DSR       =0xF5110000;  
SETDMAPOR     =0xF5200000;    
STEPDRAWBOX   =0xF6100000;  
STEPDRAWOCT   =0xF6110000;   
STEPDRAWCRS   =0xF6120000;    
BRESBOX       =0xF6200000;
HV304560      =0xF6210000;
BRESCRS       =0xF6220000;
BRES          =0xF6A00000;
S2DPXB        =0xFA100000;    
COLXPXB       =0xFA130000;
S2DPXDMA      =0xFA200000;    
S2DFAST       =0xFA500000;
PATT_FILL     =0xFA800000;   

C030CRC       =0x196B;
C031CRC       =0x876B;
C032CRC       =0x99FB;
C040CRC       =0x23AF;
C041CRC       =0x0729;
C042CRC       =0x10E0;
C050CRC       =0xDD59;
C061CRC       =0x78E1;
C062CRC       =0x75C4;
C064CRC       =0x76A7;
C065CRC       =0x4CDA;
C068CRC       =0xA7D5;
C0B2CRC       =0x056D;
C0B3CRC       =0x06A5;
E00CCRC       =0xA03A;
E011CRC       =0x15AD;
E012CRC       =0x5F60;
E013CRC       =0x1772;
E014CRC       =0x5DBF;


CoP_Busy = 0x80;   /* AND- Tests the CoProcessor Busy bit of PI control reg */
Sus_Op   = 0x08;   /* OR-  Suspends the current PI operation */
UnSus_Op = 0xF7;   /* AND- Unsuspends the current PI operation */
Op_Sus   = 0x10;   /* AND- Tests the Operation Suspended bit of PI control reg */
Diag_SR  = 0x04;   /* OR-  Sets the Diagnostic Save and Restore bit of PI Cntl Reg */
Reg_SR   = 0xFB;   /* AND- Sets the Regular Save and Restore bit of PI Cntl Reg */
Rstr     = 0x02;   /* OR-  Sets the Save bit of PI control reg */
Save     = 0xFD;   /* AND- Sets the Restore bit of PI control reg */

}
