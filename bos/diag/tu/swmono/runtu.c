static char sccsid[] = "@(#)89  1.6  src/bos/diag/tu/swmono/runtu.c, tu_swmono, bos411, 9428A410j 1/28/94 13:49:21";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: 
 *              run_tu
 *              writeA
 *              writeB
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
#include "exectu.h"
#include <sys/rcm_win.h> /*must go before include of aixgsc.h*/
#include <sys/aixgsc.h>
#include <sys/rcmioctl.h>

extern lword segreg;  /*segment register nibble */
extern struct sky_map skydat;
make_gp makemap;
extern unmake_gp unmakemap;

/**************************************************************
* Name     : run_tu                                           *
* Function : Run the specified TU one time and return the     *
*            results.                                         *
*                                                             *
**************************************************************/
run_tu(name)
word name;
{  lword rc;
   byte color;
   lword killtime;

   name = name & 0xF0FF; /* remove control nibble of TU name */
   switch (name)
   {
     case 0x90FF: rc = 0;
                  break;
     case 0xA000: rc = a000();
                  break;
     case 0xA001: rc = a001();
                  break;
     case 0xA080: rc = testdadcr();
                  break;
     case 0xA090: rc = testmcsrr();
                  break;
     case 0xA0A0: rc = testcrtchorz();
                  break;
     case 0xA0B0: rc = testcrtcvert();
                  break;
     case 0xA0C0: rc = testsprite();
                  break;
     case 0xA0C5: rc = disablesprite();
                  break;
     case 0xA0D0: rc = testmisc();
                  break;
     case 0xA0E0: rc = testspr_pal();
                  break;
     case 0xA0F0: rc = a0f0();
                  break;
     case 0xB000: rc = b000();
                  break;
     case 0xB010: rc = b010();
                  break;
     case 0xB020: rc = b020();
                  break;
     case 0xB030: rc = b030();
                  break;
     case 0xB040: rc = b040();
                  break;
     case 0xB050: rc = b050();
                  break;
     case 0xB060: rc = b060();
                  break;
     case 0xB070: rc = b070();
                  break;
     case 0xB080: rc = b080();
                  break;
     case 0xB090: rc = b090();
                  break;
     case 0xC000: rc = c000();
                  break;
     case 0xC030: rc = c030();
                  break;
     case 0xC031: rc = c031();
                  break;
     case 0xC032: rc = c032();
                  break;
     case 0xC040: rc = c040();
                  break;
     case 0xC041: rc = c041();
                  break;
     case 0xC042: rc = c042();
                  break;
     case 0xC050: rc = c050();
                  break;
     case 0xC061: rc = c061();
                  break;
     case 0xC062: rc = c062();
                  break;
     case 0xC064: rc = c064();
                  break;
     case 0xC065: rc = c065();
                  break;
     case 0xC0B2: rc = c0b2();
                  break;
     case 0xC0B3: rc = c0b3();
                  break;

/*  DEBUG   OMIT DMA and Interrupt TU's until LFT updates working
 *   case 0xC0D1: rc = c0d1();
 *                break;
 *   case 0xC0D5: rc = c0d5();
 *                break;
 *   case 0xC0E0: rc = c0e0();
 *                break;
 *  DEBUG */

 /* DEBUG  set return codes to zero until interrupts and dma support fixed */
     case 0xC0D1: rc = 0; /* DEBUG */
                  break; /* DEBUG */
     case 0xC0D5: rc = 0; /* DEBUG */
                  break; /* DEBUG */
     case 0xC0E0: rc = 0; /* DEBUG */
                  break; /* DEBUG */
 /* DEBUG set return codes to zero until interrupts and dma support fixed */


     case 0xC0F1: rc = c0f1();
                  break;
 /*  case 0xE00A: rc = e00a();         */
 /*               break;               */
     case 0xE00C: rc = e00c();
                  break;
     case 0xE00E: rc = e00e();
                  break;
     case 0xE001: rc=e001(0,0xF);
		  disablesprite();
                  break;
     case 0xE011: rc = e001(0,0xF);
                  rc = set_ret(CROSHATCH11,E011CRC,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE002: rc=e001(0xF,0);
		  disablesprite();
                  break;
     case 0xE012: rc = e001(0xF,0);
                  rc = set_ret(CROSHATCH12,E012CRC,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE003: rc=e002(0,0xF);
		  disablesprite();
                  break;
     case 0xE013: rc = e002(0,0xF);
                  rc = set_ret(CROSHATCH13,E013CRC,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE004: rc=e002(0xF,0);
		  disablesprite();
                  break;
     case 0xE014: rc = e002(0xF,0);
                  rc = set_ret(CROSHATCH14,E014CRC,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE005: rc=e003(0,0xF);
		  disablesprite();
                  break;
     case 0xE015: rc = e003(0,0xF);
                  rc = set_ret(0xE0150000,0x492A,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE006: rc=e003(0xF,0);
		  disablesprite();
                  break;
     case 0xE016: rc = e003(0xF,0);
                  rc = set_ret(0xE0160000,0x03E7,rc,MAPA);
		  disablesprite();
                  break;
     case 0xE009: for (color=0; color<0x09; color++)
		 { rc = e002(color,0x0F-color); FUSE; disablesprite();
		    for (killtime=0;killtime<0x2FFFFF;killtime++);
		    strcpy(tinfo.info.msg_buff," ");
		    strcpy(tinfo.info.err_buff," ");
                  }
                  break;
     case 0xE010: rc = e010();
                  break;
     case 0xE020: rc = e020();
                  break;
     case 0xE030: rc = e030();
                  break;
     case 0xE051: rc = e051();
                  break;
     case 0xE052: rc = e052();
                  break;
     case 0xE053: rc = e053();
                  break;
     case 0xE054: rc = e054();
                  break;
     case 0xE055: rc = e055();
                  break;
     case 0xF0FE: sprintf(tinfo.info.msg_buff,"%s%s%s%s%s%s%s%s%s%s%s",
		  "1 111111111111111111111111111111111111111111111111111111\n",
		  "2 222222222222222222222222222222222222222222222222222222\n",
		  "3 333333333333333333333333333333333333333333333333333333\n",
		  "4 444444444444444444444444444444444444444444444444444444\n",
		  "5 111111111111111111111111111111111111111111111111111111\n",
		  "6 222222222222222222222222222222222222222222222222222222\n",
		  "7 333333333333333333333333333333333333333333333333333333\n",
		  "8 444444444444444444444444444444444444444444444444444444\n",
		  "9 111111111111111111111111111111111111111111111111111111\n",
		  "10 22222222222222222222222222222222222222222222222222222\n",
		  "11 33333333333333333333333333333333333333333333333333333\n");
		  rc = 0xFFFFFFFF;
                  break;
     case 0xF0FF: rc = 0;
                  break;
     default:
     {  sprintf(tinfo.info.msg_buff, 
		"Bad TU name sent by calling routine: %X",name);
        CatErr (RUN_TU | CPF_INVDAT);
        rc = (RUN_TU | CPF_INVDAT);
     } /* end default */
   } /* end switch */
   if ((rc & 0xFFFF0000) == RUN_TU)
   {
     return(rc);  /* already logged in this proceedure */
   }
   else if ((word)rc > ERR_LEVEL) 
   { /*return code signifies an unsuccessful operation */
     CatErr(RUN_TU | SUBTU_FAIL);
     return(RUN_TU | SUBTU_FAIL);
   }
   else /* if return code signifies a successful operation */
   { CatErr(RUN_TU | GOOD_OP);
     return(RUN_TU | GOOD_OP);
   }
}

writeA()
{ int rc;
  lword x,y,a,b,k,r1,r2;
  rc=0;
  GETBASE;
  do
  { *(byte *)(base+0x78) = y = 0x0A;
    x = *(byte *)(base+0x78);
    if (x != y) {rc = 1; break; }
    *(word *)(base+0x78) = y = 0x0AAA;
    x = *(word *)(base+0x78);
    if (x != y){rc = 1; break;}
    *(lword *)(base+0x78) = y = 0x0AAA0AAA; 
    x = *(lword *)(base+0x78);
    if (x != y) {rc = 1; break;}
  } while (0);

  if (rc)
  { *(word *)(base+0x7A) = a = 0x0BBB;
    b = *(lword *)(base+0x78);
    r1 = *(lword *)(base+0x78);
    for (k=0;k<0x10000;k++);
    r2 = *(lword *)(base+0x78);
     sprintf(tinfo.info.msg_buff, 
      "Read Write A's Failure at 0x%X:\nread %X\nwrote %X\nread %X\nwrote %X\nreread1=%X  read2=%X",
	 (base+0x78),x,y,b,a,r1,r2);
     CatErr (RUN_TU | CPF_INVDAT);
     rc = (RUN_TU | CPF_INVDAT);
  } 
  else rc = (RUN_TU | 0);
  return(rc); 
}

writeB()
{ int rc;
  lword x,y;
  rc=0;
  GETBASE;
  do
  { *(byte *)(base+0x78) = y = 0x0A;
    x = *(byte *)(base+0x78);
    if (x != y) {rc = 1; break; }
    *(word *)(base+0x78) = y = 0x0AAA;
    x = *(word *)(base+0x78);
    if (x != y){rc = 1; break;}
    *(lword *)(base+0x78) = y = 0x0AAA0AAA; 
    x = *(lword *)(base+0x78);
    if (x != y) {rc = 1; break;}
  } while (0);

    *(word *)(base+0x78) = y = 0x0BBB;
    x = *(word *)(base+0x78);
     sprintf(tinfo.info.msg_buff, 
	 "Read Write A's Failure at 0x%X:\nread  %X\nwrote %X\nDid BBB W/R ",
	 (base+0x78),x,y);
     CatErr (RUN_TU | CPF_INVDAT);
     rc = (RUN_TU | CPF_INVDAT);
  return(rc); 
} 

