static char sccsid[] = "@(#)26	1.2  src/bos/diag/tu/sky/e000.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:10";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: cursor_up
 *		cursor_upnf
 *		e001
 *		e002
 *		e003
 *		e00a
 *		e00c
 *		e00e
 *		e051
 *		e052
 *		e053
 *		e054
 *		e055
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
/***************************************************************************
*  Name   : e00a()                                                         *
*  Function : Run all skytu TU's                                           *
*                                                                          *
***************************************************************************/
e00a()
{ word rc;

  do
  { rc = a000();              rc_msg(ALLREG,rc,CPTR); FUSE;
    rc = b000();              rc_msg(ALLVRAMEM,rc,CPTR); FUSE;
    rc = c000();              rc_msg(CoP_TEST ,rc,CPTR); FUSE;
  } while (FALSE);  /* do once, breaking on any error */
  if (rc > ERR_LEVEL)
  { CatErr(ALLTU | SUBTU_FAIL);
    CPTR->err_count--; /* removes error counted in skytu runtu loop */
    return(ALLTU | SUBTU_FAIL);
  }
  else
  { CatErr(ALLTU | GOOD_OP);
    return(ALLTU | GOOD_OP);
  }
}

/***************************************************************************
*  Name   : e00e()                                                         *
*  Function : Run all skytu TU's used in EMC                               *
*                                                                          *
***************************************************************************/
e00e()
{ word rc;
  do
  { rc = a000();              rc_msg(H_SCROLL,rc,CPTR); FUSE;
    rc = e010();              rc_msg(H_SCROLL,rc,CPTR); FUSE;
    rc = e020();              rc_msg(H_SCROLL,rc,CPTR); FUSE;
    rc = e030();   
    rc_msg(H_SCROLL,rc,CPTR); FUSE;
  } while (FALSE);  /* do once, breaking on any error */
  if (rc > ERR_LEVEL)
  { CatErr(ALLEMCTU | SUBTU_FAIL);
    CPTR->err_count--; /* removes error counted in skytu runtu loop */
    return(ALLEMCTU | SUBTU_FAIL);
  }
  else
  { CatErr(ALLEMCTU | GOOD_OP);
    return(ALLEMCTU | GOOD_OP);
  }
}

/********************************************************
* Name       : e00c()                                   *
* Function   : Red Green Blue Screen                    *
*                                                       *
********************************************************/
e00c()
{
  word rc;
  byte color;

  do
  {
    initcop();
    rc = make_map(MAPA, skyvbase, 341-1, 1280-1, BPP8); FUSE;
    rc = clrscr(MAPA, red); FUSE;
    rc = make_map(MAPB, skyvbase+1280*341,  341-1, 1280-1, BPP8); FUSE;
    rc = clrscr(MAPB, green); FUSE;
    rc = make_map(MAPC, skyvbase+1280*341*2,  342-1, 1280-1, BPP8); FUSE;
    rc = clrscr(MAPC, blue); FUSE;
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(THREE_COLOR, E00CCRC , rc, MAPA);
  return(rc);
}
/***************************************************************************
*  Name     : e001()                                                       *
*  Function : Cross Hatch the Screen for Visual Verification               *
*  Note     : The Caller of Skytu or Skyx2tu is Responsible for getting    *
*             Users response and then disabling the sprite using TU 'a0c5' *
*                                                                          *
***************************************************************************/
e001(fcol,bcol)
lword fcol,bcol;     /*forground and background colors */
{ word data, rc = GOOD_OP;
  int ones, line;
  word period;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  { 
    initcop();
    rc = make_map(MAPA, skyvbase, 1023, 1279, BPP8); FUSE;
    clrscr(MAPA, bcol);
    writelword(fcol, FG_COLOR);
    writebyte(SRC, FGD_MIX);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);

    for (period = 0; period < 1280; period += 64)
    { rc=bres(period,0,period,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    rc=bres(1279,0,1279,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW); FUSE;

    for (period = 0; period < 1024; period += 64)
    { rc=bres(0,period,1280,period,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    rc=bres(0,1023,1280,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
    
    rc=cursor_up(0x01,0xEC,0x00,0x5A); FUSE;

  } while (FALSE);  /* do once, breaking on any error */
  return(rc);
}
/***************************************************************************
*  Name     : e002()                                                       *
*  Function : 128X128 Cross Hatch the Screen for Visual Verification       *
*  Note     : The Caller of Skytu or Skyx2tu is Responsible for getting    *
*             Users response                                               *
*                                                                          *
***************************************************************************/
e002(fcol,bcol)
lword fcol, bcol;
{ word data, rc = GOOD_OP;
  int ones, line;
  word period;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  { 
    initcop();
    rc = make_map(MAPA, skyvbase, 1023, 1279, BPP8); FUSE;
    clrscr(MAPA, bcol);
    writelword(fcol, FG_COLOR);
    writebyte(SRC, FGD_MIX);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    for (period = 0; period < 1280; period += 128)
    { rc=bres(period,0,period,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    period -= 1;
    rc=bres(period,0,period,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
    FUSE;
    for (period = 0; period < 1024; period += 128)
    { rc=bres(0,period,1280,period,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    period -= 1;
    rc=bres(0,period,1280,period,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
    FUSE;

    rc=cursor_up(0x01,0xEC,0x00,0x5A); FUSE;

  } while (FALSE);  /* do once, breaking on any error */
  return(rc);
}
/***************************************************************************
*  Name     : e003()                                                       *
*  Function : 160X128 Cross Hatch the Screen for Visual Verification       *
*  Note     : The Caller of Skytu or Skyx2tu is Responsible for getting    *
*             Users response                                               *
*                                                                          *
***************************************************************************/
e003(fcol,bcol)
lword fcol, bcol;
{ word data, rc = GOOD_OP;
  int ones, line;
  word period;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  { 
    initcop();
    rc = make_map(MAPA, skyvbase, 1023, 1279, BPP8); FUSE;
    clrscr(MAPA, bcol);
    writelword(fcol, FG_COLOR);
    writebyte(SRC, FGD_MIX);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    for (period = 0; period < 1280; period += 160)
    { rc=bres(period,0,period,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    period -= 1;
    rc=bres(period,0,period,1023,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
    FUSE;
    for (period = 0; period < 1024; period += 128)
    { rc=bres(0,period,1280,period,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
      FUSE;
    } FUSE;
    period -= 1;
    rc=bres(0,period,1280,period,MMD,DAP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,LDW);
    FUSE;

    rc=cursor_up(0x02,0x0C,0x00,0x5A); FUSE;

  } while (FALSE);  /* do once, breaking on any error */
  return(rc);
}

cursor_upnf(xposhi, xposlo, yposhi,yposlo)
int xposhi,xposlo;
int yposhi,yposlo;
{ word data, rc = GOOD_OP;
  int ones, line;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  {
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
      /* Set sprite x position  Plane 0 (and 1) */  
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
  
  
      /* Set sprite x position  Plane 0 (and 1) */  
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
  
      /* Set sprite y position LO Plane 0 (and 1) */  
      /* Set LO first since cursor only moves when HI is written */
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
  } while (FALSE);  /* do once, breaking on any error */
  return(rc);
}
cursor_up(xposhi, xposlo, yposhi,yposlo)
int xposhi,xposlo;
int yposhi,yposlo;
{ word data, rc = GOOD_OP;
  int ones, line;

  GETBASE;
  GETIOBASE;
  GETVBASE;

  do
  {
    
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
      /* Set sprite x position  Plane 0 (and 1) */  
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
  
  
      /* Set sprite x position  Plane 0 (and 1) */  
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
  
      /* Set sprite y position LO Plane 0 (and 1) */  
      /* Set LO first since cursor only moves when HI is written */
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
  } while (FALSE);  /* do once, breaking on any error */
  return(rc);
}
/***************************************************************************
*  Name   : e051()                                                         *
*  Function : Clear Screen TU to Black                                     *
*                                                                          *
***************************************************************************/
e051()
{ 
  initcop();
  set_pal(black =0x00, 0x00, 0x00, 0x00);
  make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8);
  clrscr(MAPA, black);
  return(0);
}
/***************************************************************************
*  Name   : e052()                                                         *
*  Function : Clear Screen TU to White                                     *
*                                                                          *
***************************************************************************/
e052()
{ 
  initcop();
  make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8);
  set_pal(white =0x01, 0xFF, 0xFF, 0xFF);
  clrscr(MAPA, white);
  return(0);
}
/***************************************************************************
*  Name   : e053()                                                         *
*  Function : Clear Screen TU to Red                                       *
*                                                                          *
***************************************************************************/
e053()
{ 
  initcop();
  make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8);
  set_pal(0x02, 0xFF, 0x00, 0x00);
  clrscr(MAPA, 0x02);
  return(0);
}
/***************************************************************************
*  Name   : e054()                                                         *
*  Function : Clear Screen TU to Green                                     *
*                                                                          *
***************************************************************************/
e054()
{ 
  initcop();
  make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8);
  set_pal(0x03, 0x00, 0xFF, 0x00);
  clrscr(MAPA, 0x03);
  return(0);
}
/***************************************************************************
*  Name   : e055()                                                         *
*  Function : Clear Screen TU to Blue                                      *
*                                                                          *
***************************************************************************/
e055()
{ 
  initcop();
  make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8);
  set_pal(0x04, 0x00, 0x00, 0xFF);
  clrscr(MAPA, 0x04);
  return(0);
}
