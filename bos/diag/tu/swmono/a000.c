static char sccsid[] = "@(#)45	1.3  src/bos/diag/tu/swmono/a000.c, tu_swmono, bos411, 9428A410j 6/1/94 10:17:02";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: a000
 *		a001
 *		disablesprite
 *		initsprite
 *		test_if_in_vram
 *		test_response
 *		testcrtchorz
 *		testcrtcvert
 *		testdadcr
 *		testmcsrr
 *		testmisc
 *		testreg
 *		testspr_pal
 *		testsprite
 *		tstreg
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "skytu.h"
byte X_PLUS_2 = FALSE;
/**************************************************************
* Name     : a000()                                           *
* Function : Test Reset Results and Register Initialization   *
*                                                             *
**************************************************************/
a000()
{ word rc;

  do
  {

    rc = (word)testdadcr();   rc_msg(DADCR ,rc,CPTR); FUSE;       /* A080 */
    rc = (word)testmcsrr();   rc_msg(MCSRR ,rc,CPTR); FUSE;       /* A090 */
    rc = (word)testcrtchorz();rc_msg(CRTCHORZ,rc,CPTR); FUSE;     /* A0A0 */
    rc = (word)testcrtcvert();rc_msg(CRTCVERT,rc,CPTR); FUSE;     /* A0B0 */
    rc = (word)testmisc();    rc_msg(MISC  ,rc,CPTR); FUSE;       /* A0D0 */
    rc = (word)testspr_pal(); rc_msg(SPR_PAL,rc,CPTR); FUSE;      /* A0E0 */
    rc = (word)disablesprite(); rc_msg(SPRITEOFF,rc,CPTR); FUSE;  /* A0C5 */
    rc = (word)testsprite();  rc_msg(SPRITE,rc,CPTR); FUSE;       /* A0C0 */
    rc = (word)a0f0();        rc_msg(VPD,rc,CPTR); FUSE;          /* A0F0 */

  } while (FALSE);  /* executes only once, allowing breaks if error */

  if (rc > ERR_LEVEL) 
  { CatErr(ALLREG | SUBTU_FAIL);
    sprintf(tinfo.info.msg_buff, "%s",LASTMSG);
    sprintf(tinfo.info.err_buff, "%s",LASTERR);
    return(THISTUFAIL);
  }
  else
  { CatErr (ALLREG | GOOD_OP);
    return(GOOD_OP);
  }
}
/**************************************************************
* Name     : a001()                                           *
* Function : Test Reset Results and Register Initialization   *
*            (Like a000, but a lot faster)                    *
*            (no VPD. Sprite is setup - not tested)           *
**************************************************************/
a001()
{ word rc;

  do
  {

    rc = (word)testdadcr();   rc_msg(DADCR ,rc,CPTR); FUSE;       /* A080 */
    rc = (word)testmcsrr();   rc_msg(MCSRR ,rc,CPTR); FUSE;       /* A090 */
    rc = (word)testcrtchorz();rc_msg(CRTCHORZ,rc,CPTR); FUSE;     /* A0A0 */
    rc = (word)testcrtcvert();rc_msg(CRTCVERT,rc,CPTR); FUSE;     /* A0B0 */
    rc = (word)testmisc();    rc_msg(MISC  ,rc,CPTR); FUSE;       /* A0D0 */
    rc = (word)testspr_pal(); rc_msg(SPR_PAL,rc,CPTR); FUSE;      /* A0E0 */
    rc = (word)initsprite();  rc_msg(SPRITE,rc,CPTR); FUSE;

  } while (FALSE);  /* executes only once, allowing breaks if error */

  if (rc > ERR_LEVEL) 
  { CatErr(ALLREG | SUBTU_FAIL);
    sprintf(tinfo.info.msg_buff, "%s",LASTMSG);
    sprintf(tinfo.info.err_buff, "%s",LASTERR);
    return(THISTUFAIL);
  }
  else
  { CatErr (ALLREG | GOOD_OP);
    return(GOOD_OP);
  }
}
/************************************************************************
* Name     : testdadcr                                                  *
* Function : Write/Read-Verify the Direct Access Display Control Regs   *
*            (the non indexed registers)                                *
*                                                                       *
************************************************************************/
testdadcr()
{ word ret_code;
  byte *addr;  /*pointer to the address of the register to be tested*/

  GETIOBASE;    
  do
  { addr = (byte *) (iobase + 0);
    ret_code = testreg(addr,tinfo.dadcr.op_mode,OPMODE,"Operating Mode Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 1);
    ret_code = testreg(addr,tinfo.dadcr.vramwctl,VRAMWINDCNTL,"VRAM Window Control Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 4);
    ret_code = testreg(addr,tinfo.dadcr.intrpt_e,INTRPT_E,"Interrupt Enable/Disable Reg");
    if (ret_code > ERR_LEVEL) break;



    addr = (byte *) (iobase + 5);
    writebyte(0xFF, addr); /* clear the interrupt status, then check for 0's */
    ret_code = testreg(addr,tinfo.dadcr.intrpt_s,INTRPT_S,"Interrupt Status Reg");
    if (ret_code > ERR_LEVEL)
    { /* incase frameflyback or vertblankingend occured*/
      writebyte(0xFF, addr); /*clear the interrupt status, then check for 0's */
      ret_code=testreg(addr,tinfo.dadcr.intrpt_s,INTRPT_S,"Interrupt Status Reg"); 
    }
    if (ret_code > ERR_LEVEL)
    { /* incase frameflyback or vertblankingend occured*/
      writebyte(0xFF, addr); /*clear the interrupt status, then check for 0's */
      ret_code=testreg(addr,tinfo.dadcr.intrpt_s,INTRPT_S,"Interrupt Status Reg"); 
    }
    if (ret_code > ERR_LEVEL)
    { /* incase frameflyback or vertblankingend occured*/
      writebyte(0xFF, addr); /*clear the interrupt status, then check for 0's */
      ret_code=testreg(addr,tinfo.dadcr.intrpt_s,INTRPT_S,"Interrupt Status Reg"); 
    }
    if (ret_code > ERR_LEVEL)
    { /* incase frameflyback or vertblankingend occured*/
      writebyte(0xFF, addr); /*clear the interrupt status, then check for 0's */
      ret_code=testreg(addr,tinfo.dadcr.intrpt_s,INTRPT_S,"Interrupt Status Reg"); 
    }
    if (ret_code > ERR_LEVEL) break;



    addr = (byte *) (iobase + 6);
    ret_code = testreg(addr,tinfo.dadcr.memcntl,MEMCNTL,"Memory Controller Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 7);
    writebyte(0xFF, addr); /* clear all status regs, then test for zeros */
    ret_code = testreg(addr,tinfo.dadcr.memintstat,MEMINTSTAT,"Memory Interrupt Status Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 8);
    ret_code = testreg(addr,tinfo.dadcr.vramindx,VRAMINDEX,"VRAM Index Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 9);
    ret_code = testreg(addr,tinfo.dadcr.mem_accs,MEMACCS,"Memory Access Mode Reg");
    if (ret_code > ERR_LEVEL) break;
    addr = (byte *) (iobase + 0x0A);
    ret_code = testreg(addr,tinfo.dadcr.index,INDEX,"Base_A Index Reg");
    if (ret_code > ERR_LEVEL) break;
  } while (FALSE);  /*do one time, allowing for breaks */
  if (ret_code > ERR_LEVEL) 
  { CatErr(DADCR | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (DADCR | GOOD_OP);
    return(GOOD_OP);
  }
}
/**********************************************************
* Name     :  testmcsrr                                   *  
* Function :  Write/Read-Verify the Memory Configuration  *
*             and Save Restore Registers (indexed regs)   *
*                                                         *
**********************************************************/
testmcsrr()
{ word ret_code;
  
  GETIOBASE;
  do
  {

    writebyte(0x00, INDX_ADDR);
    ret_code = testreg(DATAB_ADDR,tinfo.mcsrr.memconfg,MEMCONF,"Memory Configuration Reg");
    if (ret_code > ERR_LEVEL) 
    {
      writebyte(0x00, INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0x0D,MEMCONF,"Memory Configuration Reg");
      if (ret_code > ERR_LEVEL) break;
      X_PLUS_2 = TRUE;
      writebyte(0x01, INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0x02,MEMCONF,"Memory Parameter Reg");
      if (ret_code > ERR_LEVEL) break;
    }



    writebyte(0x04, INDX_ADDR);
    ret_code = testreg(DATAB_ADDR,tinfo.mcsrr.autoconf,AUTOCONF,"Auto Configuration Reg");
    if (ret_code > ERR_LEVEL) break;

  } while (FALSE);  /*do one time, allowing for breaks */
  if (ret_code > ERR_LEVEL) 
  { CatErr(MCSRR | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (MCSRR | GOOD_OP);
    return(GOOD_OP);
  }
}

/**********************************************************
* Name     :  testcrtchorz                                *  
* Function :  Write/Read-Verify the CRTC Horizontal       *
*             Display Registers (indexed regs)            *  
*                                                         *
**********************************************************/
testcrtchorz()
{ word ret_code;

  GETIOBASE;
  do
  { writebyte(0x10,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.horz_tot,HORZTOT,"Horz. Scan Length Reg ");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x12,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.horz_end,HORZEND,"Horz. End Reg ");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x14,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.horz_ebrd,HORZEBRD,"End Horz. Border Reg");
    if (ret_code > ERR_LEVEL) break;
    if (X_PLUS_2)
    {
      writebyte(0x16,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR ,0xE1,HORZSBRD,"Start Horz. Border Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x18,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0xB4,HORZSYNS,"Horz. Sync Start Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x1A,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0xD4,HORZSYE1,"Horz. Sync End 1 Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x1C, INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0xC5,HORZSYE2,"Horz. Sync End 2 Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x1E, INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,0x06,HORZSYE2+1,"Horz Sync Position");
      if (ret_code > ERR_LEVEL) break;
    }
    else
    {
      writebyte(0x16,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.horz_sbrd,HORZSBRD,"Start Horz. Border Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x18,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,tinfo.crtcdspl.horz_syncs,HORZSYNS,"Horz. Sync Start Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x1A,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,tinfo.crtcdspl.horz_synce1,HORZSYE1,"Horz. Sync End 1 Reg");
      if (ret_code > ERR_LEVEL) break;
      writebyte(0x1C,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR,tinfo.crtcdspl.horz_synce2,HORZSYE2,"Horz. Sync End 2 Reg");
      if (ret_code > ERR_LEVEL) break;
    }
  } while (FALSE);  /*do one time, allowing for breaks */
  if (ret_code > ERR_LEVEL) 
  { CatErr(CRTCHORZ | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (CRTCHORZ | GOOD_OP);
    return(GOOD_OP);
  }
}

/**********************************************************
* Name     :  testcrtcvert                                *  
* Function :  Write/Read-Verify the CRTC Vertical Display *
*             Control Registers (indexed regs)            *
*                                                         *
**********************************************************/
testcrtcvert()
{ word ret_code;

  GETIOBASE;
  do
  {
    writebyte(0x20,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR, tinfo.crtcdspl.vert_totl,  VERTTOTL,"Vert. Scan Length Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x21,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR, tinfo.crtcdspl.vert_toth,  VERTTOTH,"Vert. Scan Length Reg 2");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x22,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_endl,  VERTENDL,"Vert. End Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x23,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_endh,  VERTENDH,"Vert. End Reg 2");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x24,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_ebrdl, VERTEBRDL,"End Vert. Border Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x25,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_ebrdh, VERTEBRDH,"End Vert. Border Reg 2");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x26,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_sbrdl, VERTSBRDL,"Start Vert. Border Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x27,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_sbrdh, VERTSBRDH,"Start Vert. Border Reg 2");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x28,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_syncsl,VERTSYNSL,"Vert. Sync Start Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x29,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_syncsh,VERTSYNSH,"Vert. Sync Start Reg 2");
    if (ret_code > ERR_LEVEL) break;
    if (X_PLUS_2)
    {
      writebyte(0x2A,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR ,0x07 ,VERTSYE,"Vert. Sync End 1 Reg ");
      if (ret_code > ERR_LEVEL) break;
    }
    else
    {
      writebyte(0x2A,INDX_ADDR);
      ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_synce ,VERTSYE,"Vert. Sync End 1 Reg ");
      if (ret_code > ERR_LEVEL) break;
    }
    writebyte(0x2C,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_lincmpl ,LINCMPL,"Line Compare Reg 1");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x2D,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.crtcdspl.vert_lincmph ,LINCMPH,"Line Compare Reg 2");
  } while (FALSE);  /*do one time, allowing for breaks */
  if (ret_code > ERR_LEVEL) 
  { CatErr(CRTCVERT | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (CRTCVERT | GOOD_OP);
    return(GOOD_OP);
  }
}
/**********************************************************
* Name     :  testsprite                                  *  
* Function :  Write/Read-Verify the Sprite Registers      *
*                                                         *
**********************************************************/
testsprite()
{ word data, rc = GOOD_OP;
  int xposhi,xposlo;
  int yposhi,yposlo;
  int ones, line;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  {
    rc = spritecol(white =0x00, 0xFF, 0xFF, 0xFF);FUSE;
    rc = spritecol(blue  =0x01, 0x00, 0x00, 0xFF);FUSE;
    rc = spritecol(red   =0x02, 0xFF, 0x00, 0x00);FUSE;
    rc = spritecol(green =0x03, 0x00, 0xFF, 0x1F);FUSE;
    
    /*Load and test the SRAM */
    writebyte(0x59, INDX_ADDR);  /*Set SRAM Plane 1 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x57, INDX_ADDR);  /*Set SRAM Plane 1 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5B, INDX_ADDR);  /*Access the image data register */
    for (line = 0; line < 64; line++)
    { for (ones = 0x00; ones < 8; ones++) /* write sprite pattern, auto */
      { if (ones <= line/8)                   /* incrementing the address   */
        { writebyte(0xFF, DATAB_ADDR);     /* after each write           */
	}
	else
        { writebyte(0x00, DATAB_ADDR);
	} /* end else */
      } /* end for ones */
    } /* end for line */
    writebyte(0x58, INDX_ADDR);  /*Set SRAM Plane 0 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x56, INDX_ADDR);  /*Set SRAM Plane 0 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5A, INDX_ADDR);  /*Access the image data register */
    for (line = 63; line >= 0 ; line--)
    { for (ones = 0x00; ones < 8; ones++) /* write sprite pattern, auto */
      { if (ones > line/8)                   /* incrementing the address   */
        { writebyte(0xFF, DATAB_ADDR);     /* after each write           */
	}
	else
        { writebyte(0x00, DATAB_ADDR);
	} /* end else */
      } /* end for ones */
    } /* end for line */

    writebyte(0x59, INDX_ADDR);  /*Set SRAM Plane 1 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x57, INDX_ADDR);  /*Set SRAM Plane 1 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5B, INDX_ADDR);  /*Access the image data register */
    data = readbyte(DATAB_ADDR);   /* initial read for pipeline */
    for (line = 0; line < 64; line++)
    { for (ones = 0x00; ones < 8; ones++)   /* read sprite pattern, auto */
      { if (ones <= line/8)                 /* incrementing the address   */
        { data = readbyte(DATAB_ADDR);       /* after each read           */
	  if (data != 0xFF)
	  { sprintf(tinfo.info.msg_buff,"Bad Compare in SRAM Plane 1 Test: ");
	    CatMsgs("\n--Ones         = 0x");
	    CatMsgx(ones);
	    CatMsgs("\n--Data written = 0xFF");
	    CatMsgs("\n--Data read    = 0x");
	    CatMsgx(data);
            break;
          } /* if data */
	}   /* if ones */
	else
	{ data = readbyte(DATAB_ADDR);       /* after each read           */
	  if (data != 0x00)
	  { sprintf(tinfo.info.msg_buff,"Bad Compare in SRAM Plane 1 Test: ");
	    CatMsgs("\n--Ones         = 0x");
	    CatMsgx(ones);
	    CatMsgs("\n--Data written = 0x00");
	    CatMsgs("\n--Data read    = 0x");
	    CatMsgx(data);
            break;
          } /* if data */
        } /* end else */
      } /* for ones */ 
      FUSE;
    } /* for line */
    FUSE;
    writebyte(0x58, INDX_ADDR);  /*Set SRAM Plane 0 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x56, INDX_ADDR);  /*Set SRAM Plane 0 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5A, INDX_ADDR);  /*Access the image data register */
    data = readbyte(DATAB_ADDR);   /* initial read for pipeline */
    for (line = 63; line >= 0 ; line--)
    { for (ones = 0x00; ones < 8; ones++) /* write sprite pattern, auto */
      { if (ones > line/8)                 /* incrementing the address   */
        { data = readbyte(DATAB_ADDR);     /* after each write           */
	  if (data != 0xFF)
	  { sprintf(tinfo.info.msg_buff,"Bad Compare in SRAM Plane 0 Test: ");
	    CatMsgs("\n--Ones         = 0x");
	    CatMsgx(ones);
	    CatMsgs("\n--Data written = 0xFF");
	    CatMsgs("\n--Data read    = 0x");
	    CatMsgx(data);
            break;
          } /* if data */
	} /*if ones */
	else
        { data = readbyte(DATAB_ADDR);   
	  if (data != 0x00)
	  { sprintf(tinfo.info.msg_buff,"Bad Compare in SRAM Plane 0 Test: ");
	    CatMsgs("\n--Ones         = 0x");
	    CatMsgx(ones);
	    CatMsgs("\n--Data written = 0x00");
	    CatMsgs("\n--Data read    = 0x");
	    CatMsgx(data);
            break;
          } /* if data */
        } /* end else */
      } /* for ones */ 
      FUSE;
    } /* for line */
    FUSE;
	




    /* enable sprite */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Enable Sprite Plane 0 (and 1) */
    writebyte(0x44, DATAB_ADDR);  
    /* verify in both planes */
    writebyte(0x56, INDX_ADDR);   /* Test Sprite Plane 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x6C, INDX_ADDR); 
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);
    if (data != 0x44)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 0 Enable Data");
      CatMsgs("\n--Data written = 0x44");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      break;
    } /* if data */
    writebyte(0x57, INDX_ADDR);  /* Test Sprite Plane 1 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x6D, INDX_ADDR);  
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);

    if (data != 0x44)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 1 Enable Data");
      CatMsgs("\n--Data written = 0x44");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      break;
    } /* if data */





    /* Set sprite position */
    for (xposhi=0+2; xposhi<5+2; xposhi++)         /* move sprite from 0 to 0x500*/
    { /* Set sprite x position  Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x02, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(xposhi, DATAB_ADDR);  
      /* verify in both planes */
      writebyte(0x56, INDX_ADDR); /* Test Sprite Plane 0 */
      writebyte(0x02, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != xposhi)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 0 X PositionHi");
        CatMsgs("\n--Data written = 0x");
        CatMsgx(xposhi);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
      writebyte(0x57, INDX_ADDR); /* Test Sprite Plane 1 */
      writebyte(0x02, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != xposhi)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 1 X PositionHi");
        CatMsgs("\n--Data written = 0x");
        CatMsgx(xposhi);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
  
  
    for (xposlo=0; xposlo<0x100; xposlo+=0x10)  /* move sprite from 0 to 0x500*/
    { /* Set sprite x position  Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x01, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(xposlo, DATAB_ADDR);  
      /* verify in both planes */
      writebyte(0x56, INDX_ADDR); /* Test Sprite Plane 0 */
      writebyte(0x01, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != xposlo)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 0 X PositionLo");
        CatMsgs("\n--Data written = 0x");
        CatMsgx(xposlo);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
      writebyte(0x57, INDX_ADDR); /* Test Sprite Plane 1 */
      writebyte(0x01, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != xposlo)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 1 X PositionLo");
        CatMsgs("\n--Data written = 0x");
        CatMsgs(xposlo);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
  
    for (yposhi=0; yposhi<4; yposhi++)         /* move sprite from 0 to 0x400*/
    { /* Set sprite y position  Plane 0 (and 1) */  
    for (yposlo=0+0x19; yposlo<0x100; yposlo++)/* move sprite from 0 to 0x400*/
    { /* Set sprite y position  Plane 0 (and 1) */  
      /* Set LO first since cursor only moves when HI is written */

      /* Set sprite y position LO Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x03, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(yposlo, DATAB_ADDR);  
      /* verify in both planes */
      writebyte(0x56, INDX_ADDR); /* Test Sprite Plane 0 */
      writebyte(0x03, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != yposlo)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 0 Y PositionLo");
        CatMsgs("\n--Data written = 0x");
        CatMsgs(yposlo);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
      writebyte(0x57, INDX_ADDR); /* Test Sprite Plane 1 */
      writebyte(0x03, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != yposlo)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 1 Y PositionLo");
        CatMsgs("\n--Data written = 0x");
        CatMsgs(yposlo);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */

      /* Set sprite y position HI Plane 0 (and 1) */  
      /* --and move the cursor */
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x04, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(yposhi, DATAB_ADDR);  
      /* verify in both planes */
      writebyte(0x56, INDX_ADDR); /* Test Sprite Plane 0 */
      writebyte(0x04, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != yposhi)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 0 Y PositionHi");
        CatMsgs("\n--Data written = 0x");
        CatMsgx(yposhi);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
      writebyte(0x57, INDX_ADDR); /* Test Sprite Plane 1 */
      writebyte(0x04, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);  
      data = readbyte (DATAB_ADDR);
      data = readbyte (DATAB_ADDR);
      if (data != yposhi)
      { sprintf(tinfo.info.msg_buff,"Bad Compare: Sprite Plane 1 Y PositionHi");
        CatMsgs("\n--Data written = 0x");
        CatMsgx(yposhi);
        CatMsgs("\n--Data read    = 0x");
        CatMsgx(data);
        break;
      } /* if data */
    } /* end for yposhi */ 
    FUSE;
    } /* end for yposlo */ 
    FUSE;
    } /* end for xposlo */ 
    FUSE;
    } /* end for xposhi */ 
    FUSE;


    /* disable sprite */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Disable Sprite Plane 0 (and 1) */
    writebyte(0x04, DATAB_ADDR);  
    /* verify in both planes */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Test Sprite Plane 0 */
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);
    if (data != 0x04)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 0 Disable Data");
      CatMsgs("\n--Data written = 0x04");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      break;
    } /* if data */
    writebyte(0x57, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6D, INDX_ADDR);  /* Test Sprite Plane 1 */
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);
    if (data != 0x04)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 1 Disable Data");
      CatMsgs("\n--Data written = 0x04");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      break;
    } /* if data */
  } while (FALSE);
  if (rc > ERR_LEVEL) 
  { CatErr(SPRITE | THISTUFAIL); 
    return(0x00);
  }
  else
  { CatErr (SPRITE | GOOD_OP);
    return(GOOD_OP);
  }
}
/**********************************************************
* Name     :  initsprite                                  *  
* Function :  Write/Read-Verify the Sprite Registers      *
*             but don't test sprite positions             *
*                                                         *
**********************************************************/
initsprite()
{ word data, rc = GOOD_OP;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  {
    writebyte(0x59, INDX_ADDR);  /*Set SRAM Plane 1 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x57, INDX_ADDR);  /*Set SRAM Plane 1 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5B, INDX_ADDR);  /*Access the image data register */
    writebyte(0xFF, DATAB_ADDR);    
    
    writebyte(0x58, INDX_ADDR);  /*Set SRAM Plane 0 Addr Hi to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x56, INDX_ADDR);  /*Set SRAM Plane 0 Addr Lo to 0 */
    writebyte(0x00, DATAB_ADDR);  
    writebyte(0x5A, INDX_ADDR);  /*Access the image data register */
    writebyte(0x00, DATAB_ADDR);

    /* enable sprite */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Enable Sprite Plane 0 (and 1) */
    writebyte(0x44, DATAB_ADDR);  

    /* Set sprite position */
    do
    { /* Set sprite x position  Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x02, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(2, DATAB_ADDR);  

      /* Set sprite x position  Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x01, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(0, DATAB_ADDR);  
  
      /* Set sprite y position LO Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x03, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(0x19, DATAB_ADDR);  

      /* Set sprite y position HI Plane 0 (and 1) */  
      writebyte(0x56, INDX_ADDR); 
      writebyte(0x04, DATAB_ADDR);  
      writebyte(0x6C, INDX_ADDR);
      writebyte(0, DATAB_ADDR);  
    } while (FALSE);

    /* disable sprite */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Disable Sprite Plane 0 (and 1) */
    writebyte(0x04, DATAB_ADDR);  
  } while (FALSE);  /* do once, breaking on any error */
  return(GOOD_OP);
}
/**********************************************************
* Name     :  disablesprite                               *  
* Function :  Write/Read-Verify the Sprite Registers      *
*                                                         *
**********************************************************/
disablesprite()
{ word data, rc = GOOD_OP;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  {
    /* disable sprite */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Disable Sprite Plane 0 (and 1) */
    writebyte(0x04, DATAB_ADDR);  
    /* verify in both planes */
    writebyte(0x56, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6C, INDX_ADDR);  /* Test Sprite Plane 0 */
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);
    if (data != 0x04)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 0 Disable Data");
      CatMsgs("\n--Data written = 0x04");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      rc = THISTUFAIL;
      break;
    } /* if data */
    writebyte(0x57, INDX_ADDR);  /* Access Sprite Addr Lo */ 
    writebyte(0x00, DATAB_ADDR); /* Access Sprite Control Reg */ 
    writebyte(0x6D, INDX_ADDR);  /* Test Sprite Plane 1 */
    data = readbyte (DATAB_ADDR);
    data = readbyte (DATAB_ADDR);
    if (data != 0x04)
    { sprintf(tinfo.info.msg_buff,"Bad Compare in Sprite Plane 1 Disable Data");
      CatMsgs("\n--Data written = 0x04");
      CatMsgs("\n--Data read    = 0x");
      CatMsgx(data);
      rc = THISTUFAIL;
      break;
    } /* if data */
  } while (FALSE);  /* do once, breaking on any error */
  if (rc > ERR_LEVEL) 
  { CatErr(SPRITE | THISTUFAIL); 
    return(THISTUFAIL);
  }
  else
  { CatErr (SPRITEOFF | GOOD_OP);
    return(GOOD_OP);
  }
}
/**********************************************************
* Name     :  testmisc                                    *  
* Calls    :  testreg,writebyte                           *
*                                                         *
**********************************************************/
testmisc()
{ word ret_code;
  byte temp,level;

  GETIOBASE;
  do
  {
    writebyte(0x40,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.staddrlo,STADDRLO,"Start of Active Picture Lo Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x41,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.staddrmi,STADDRMI,"Start of Active Picture Mi Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x42,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.staddrhi,STADDRHI,"Start of Active Picture Hi Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x43,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.buffpilo,BUFFPILO,"Buffer Pitch Lo Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x44,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.buffpihi,BUFFPIHI,"Buffer Pitch Hi Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x50,INDX_ADDR);
    ret_code = tstreg(DATAB_ADDR ,tinfo.misc.dispmod1,0xC3,DISPMOD1,"Display Mode 1 Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x51,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.dispmod2,DISPMOD2,"Display Mode 2 Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x53,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.systemid,SYSTEMID,"System ID Reg");
    writebyte(0x54,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.clk_freq,CLK_FREQ, "Clock Frequency Select Reg");
    if (ret_code > ERR_LEVEL) break;
    writebyte(0x55,INDX_ADDR);
    ret_code = testreg(DATAB_ADDR ,tinfo.misc.bord_col,BORD_COL, "Border Color Reg");
    if (ret_code > ERR_LEVEL) break;
  } while (FALSE);  /*do one time, allowing for breaks */
  if (ret_code > ERR_LEVEL)
  { CatErr(MISC | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (MISC | GOOD_OP);
    return(GOOD_OP);
 }
}
/**********************************************************
* Name     :  testspr_pal                                 *  
* Function :  Write/Read-Verify the Sprite and Palette    *
*             Registers (indexed regs)i                   *
* Called by:  run_tu.h, testregs                          *
* Calls    :  testreg, writebyte                          *
*                                                         *
**********************************************************/
testspr_pal()
{ word ret_code;
  word indexlo;
  byte responseR;
  byte responseG;
  byte responseB;

  GETIOBASE;
  do
  {
    /********************Initialize Bt458**************************************/
    writeword(0x6006, INDX_ADDR);  /*access Bt Command Register */
    writebyte(0x64, INDX_ADDR);        
    ret_code = testreg(DATAB_ADDR ,tinfo.spr_pal.Bt_command,PALDACCNTL,"Palette DAC Control Reg-Bt Command");
    if (ret_code > ERR_LEVEL) break;

    writeword(0x6004, INDX_ADDR);  /*access Bt Read Mask Register */
    writebyte(0x64, INDX_ADDR);    
    ret_code = testreg(DATAB_ADDR ,tinfo.spr_pal.Bt_readmsk,PALDACCNTL,"Palette DAC Control Reg-Bt Read Mask");
    if (ret_code > ERR_LEVEL) break;

    
    writeword(0x6005, INDX_ADDR);  /*access Bt Blink Mask Register */
    writebyte(0x64, INDX_ADDR); 
    ret_code = testreg(DATAB_ADDR ,tinfo.spr_pal.Bt_blinkmsk,PALDACCNTL,"Palette DAC Control Reg-Bt Blink Mask");
    if (ret_code > ERR_LEVEL) break;

    writeword(0x6007, INDX_ADDR);  /*access Bt Test Register */
    writebyte(0x64, INDX_ADDR); 
    ret_code = testreg(DATAB_ADDR ,tinfo.spr_pal.Bt_test,PALDACCNTL,"Palette DAC Control Reg-Bt Test");
    if (ret_code > ERR_LEVEL) break;

    /********************Initialize Bt458**************************************/

    /********************Test Palette - see EWay WkBk 3.4.8 *******************/
    writebyte(0x61, INDX_ADDR);   /* access the palette indexhi reg */
    writebyte(0x00, DATAB_ADDR); /* set the palette index reg */
    for (indexlo = 0; indexlo <= 0xFF; indexlo++)
    { writebyte(0x60, INDX_ADDR);   /* access the palette indexlo reg */
      writebyte(indexlo, DATAB_ADDR); /* set the palette index reg */
      writebyte(0x65, INDX_ADDR);   /* access the palette data reg */
      writebyte(indexlo, DATAB_ADDR); /* write data to the above index-Red */ 
      writebyte(indexlo, DATAB_ADDR); /* write data to the above index-Green */ 
      writebyte(indexlo, DATAB_ADDR); /* write data to the above index-Blue */ 

      writebyte(0x60, INDX_ADDR);   /* access the palette index reg */
      writebyte(indexlo, DATAB_ADDR); /*set the index to what was just written*/
      writebyte(0x65, INDX_ADDR);   /* access the palette data reg */
      responseR = readbyte(DATAB_ADDR);  /* read the data just written */
      responseG = readbyte(DATAB_ADDR);  /* read the data just written */
      responseB = readbyte(DATAB_ADDR);  /* read the data just written */
      
    }
    if (ret_code <= ERR_LEVEL) 
    { CatErr(PAL_DATA | GOOD_OP);
      ret_code = (PAL_DATA | GOOD_OP);
    }
    /********************Test Palette - see EWay WkBk 3.4.8 *******************/
  } while (FALSE);  /* do once, breaking on any bad return code */
  if (ret_code > ERR_LEVEL)
  { CatErr(SPR_PAL | SUBTU_FAIL);
    return(THISTUFAIL);
  }
  else
  { CatErr (SPR_PAL| GOOD_OP);
    return(GOOD_OP);
 }
}

/************************************************************************
* Name     : testreg                                                    *
* Function : Write/Read-Verify a register given its address             *
*                                                                       *
************************************************************************/
testreg(addr,stimulus,tucode,regname)
  byte  *addr;
  byte  stimulus;
  lword tucode;
  char  regname[];
{ 
  byte response;
  word rc;

  if ((rc = writebyte(stimulus, addr)) == CANT_WRITE)
  /* then the write to skyway register failed */
  {  test_response(rc, tucode,regname); /*takes care of messages and return codes*/
    sprintf(tinfo.info.msg_buff,
           "Cannot Write '%X' to Skyway register '%s'.", stimulus,regname);
    CatErr(tucode | THISTUFAIL); 
    return(tucode | THISTUFAIL);
  }
  else if ((response = readbyte(addr)) == stimulus)
  /* passed write, passed read, passed compare */
  { sprintf(tinfo.info.msg_buff,"Successful Operation from %X", tucode);
    CatErr(tucode | GOOD_OP); 
    return(tucode | GOOD_OP);
  }
  else if ((rc = test_response(response,tucode,regname)) == RC_NOTFOUND)
  /* passed write, passed read, failed compare */
  { sprintf(tinfo.info.msg_buff,
           "Bad Compare: Data written to %s = %X,\n             Data read from %s = %X",
           regname, stimulus, regname, response);
    CatErr(tucode | THISTUFAIL); 
    return(tucode | THISTUFAIL);
  }
  else
  /* passed write, but failed compare.  test_response() found the problem
     and returned the appropriate messages and error codes               */
  {  return(tucode | THISTUFAIL);
  }
}   



test_response(response,tu,regname)
word response;
lword tu;
char regname[];
{
  if (response == CANT_WRITE)
  { sprintf(tinfo.info.msg_buff,
           "Cannot Write to Skyway %s.  Received '%X'", regname,response);
    CatErr(tu | CANT_WRITE);
    return(tu | CANT_WRITE);
  }
  else if (response == SYS_FAILUR)
  { sprintf(tinfo.info.msg_buff,
           "System Failure.  Received '%X'", response);
    CatErr(tu | SYS_FAILUR);
    return(tu | SYS_FAILUR);
  }
  else if (response == CANT_READ)
  { sprintf(tinfo.info.msg_buff,
           "Cannot Read from Skyway %s.  Received '%X'",regname, response);
    CatErr(tu | CANT_READ);
    return(tu | CANT_READ);
  }
  else if (response == MOM_FAILUR)
  { sprintf(tinfo.info.msg_buff,
           "Monitor Mode Failure.  Received '%X'", response);
    CatErr(tu | MOM_FAILUR);
    return(tu | MOM_FAILUR);
  }
  else if (response == TU_FAILURE)
  { sprintf(tinfo.info.msg_buff,
           "TU Failure.  Received '%X'", response);
    CatErr(tu | TU_FAILURE);
    return(tu | TU_FAILURE);
  }
  else if (response == THISTUFAIL)
  { sprintf(tinfo.info.msg_buff,
           "This TU Failed.  Received '%X'", response);
    CatErr(tu | THISTUFAIL);
    return(tu | THISTUFAIL);
  }
  else if (response == SUBTU_FAIL)
  { sprintf(tinfo.info.msg_buff,
           "A Sub-level TU Failed.  Received '%X'", response);
    CatErr(tu | SUBTU_FAIL);
    return(tu | SUBTU_FAIL);
  }
  else if (response == CPF)
  { sprintf(tinfo.info.msg_buff,
           "The calling program failed.  Received '%X'", response);
    CatErr(tu | CPF);
    return(tu | CPF);
  }
  else if (response == CPF_INVDAT)
  { sprintf(tinfo.info.msg_buff,
           "The calling program sent invalid data to skytu %s.  Received '%X'",regname, response);
    CatErr(tu | CPF_INVDAT);
    return(tu | CPF_INVDAT);
  }
  else if (response == SKYWAYFAIL)
  { sprintf(tinfo.info.msg_buff,
           "Skyway adaptor Failure.  Received '%X'", response);
    CatErr(tu | SKYWAYFAIL);
    return(tu | SKYWAYFAIL);
  }
  else if (response == CANT_X_SKY)
  { sprintf(tinfo.info.msg_buff,
           "Cant Execute the Skyway Function.  Received '%X'", response);
    CatErr(tu | CANT_X_SKY);
    return(tu | CANT_X_SKY);
  }
  else
  { return (RC_NOTFOUND);
  }
}
/******************************************************************************
* Name       : test_if_in_vram                                                *
*                                                                             *
*                                                                             *
******************************************************************************/
test_if_in_vram(vram_addr, tucode)
lword vram_addr;
lword tucode;

{ lword  vram_base;
  lword  vram_end;

  
  vram_base = getvbaseadd();
  vram_end = vram_base + 0x200000; 


  if ((vram_addr < vram_base) || (vram_addr > vram_end))
  {  sprintf(tinfo.info.msg_buff, 
     "VRAM address (%X) is not in the range specifed by POS3 and POS4 (%X..%X)",
     vram_addr,vram_base,vram_end);
     CatErr(tucode | CPF_INVDAT);
     return(tucode | CPF_INVDAT);
  }
  return(GOOD_OP);
}

/************************************************************************
* Name     : tstreg                                                    *
* Function : Write/Read-Verify a register given its address             *
************************************************************************/
tstreg(addr,stimulus,reference,tucode,regname)
  byte  *addr;
  byte  stimulus;
  byte  reference;
  lword tucode;
  char  regname[];
{
  byte response;
  word rc;

  if ((rc = writebyte(stimulus, addr)) == CANT_WRITE)
  /* then the write to skyway register failed */
  {  test_response(rc, tucode,regname); /*takes care of messages and return codes*/
    sprintf(tinfo.info.msg_buff,
           "Cannot Write '%X' to Skyway register '%s'.", stimulus,regname);
    CatErr(tucode | THISTUFAIL);
    return(tucode | THISTUFAIL);
  }
  else if ((response = readbyte(addr)) == reference)
  /* passed write, passed read, passed compare */
  { sprintf(tinfo.info.msg_buff,"Successful Operation from %X", tucode);
    CatErr(tucode | GOOD_OP);
    return(tucode | GOOD_OP);
  }
  else if ((rc = test_response(response,tucode,regname)) == RC_NOTFOUND)
  /* passed write, passed read, failed compare */
  { sprintf(tinfo.info.msg_buff,
           "Bad Compare: Data written to %s = %X,\n             Data read from %s = %X",
           regname, stimulus, regname, response);
    CatErr(tucode | THISTUFAIL);
    return(tucode | THISTUFAIL);
  }
  else
  /* passed write, but failed compare.  test_response() found the problem
     and returned the appropriate messages and error codes               */
  {  return(tucode | THISTUFAIL);
  }
}
