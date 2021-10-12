static char sccsid[] = "@(#)47	1.3  src/bos/diag/tu/swmono/b000.c, tu_swmono, bos411, 9428A410j 6/16/94 17:27:25";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: b000
 *		b010
 *		b020
 *		b030
 *		b040
 *		b050
 *		b060
 *		b070
 *		b080
 *		b090
 *		ischx
 *		load_palette
 *		memq
 *		testvram
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
/**************************************************************
* Name     : b000()                                           *
* Function : VRAM Memory Test                                 *
*                                                             *
* Note:   when specifying tinfo.memory.memtop/membot...       *
*           memtop is always LESS THAN membot                 *
*           (memtop is the lesser address)                    *
*           (memtop refers to the top of the screen or window)*
*           (membot refers to the address to test up to, but  *
*                   not including)                            *
**************************************************************/
char *lptr;
b000()
{ char rw16_32[3];
  char sa_topbot[2];
  char tuname[50];
  int name;
  word rc;
  char errbuff[BUFF_LEN];
  char msgbuff[BUFF_LEN];
byte x;
  strcpy(errbuff,""); 
  strcpy(msgbuff,""); 
  load_palette();
  lptr = tinfo.vramem.runq;
  while (*lptr != NULL)
  { 
    rc = (word)memq(rw16_32,sa_topbot,tuname);
    /* work with rw, topbot, tuname */;
    if (rc <=  ERR_LEVEL)
    { sscanf(rw16_32, "%d", &name);       /*convert string to hex */
      tinfo.memory.rw16_32 = (byte)name;/*sscanf requires int, not byte type */
      sscanf(sa_topbot,"%c",&tinfo.memory.sa_topbot); /*convert string to char*/
      sscanf(tuname,"%x",&name);                      /*convert string to hex */
      rc = (word)run_tu((word)name);              
      sprintf(errbuff,"%s:%s:%X",errbuff,LASTERR,(ALLVRAMEM | rc));
      sprintf(msgbuff,"%s:%s",msgbuff,LASTMSG);
      CatErr(ALLVRAMEM | rc);
      rc_msg(0xB000, rc, CPTR);           
    }
    else
    { sprintf(errbuff,"%s:%s:%X",errbuff,LASTERR,(ALLVRAMEM | rc));
      sprintf(msgbuff,"%s:%s",msgbuff,LASTMSG);
      CatErr(ALLVRAMEM | rc);
      rc_msg(0xB000, rc, CPTR);           
    }
  }
  if ( CPTR->err_count > 0)
  { CatErrs(errbuff);
    CatMsgs(msgbuff);
    CatErr(ALLVRAMEM | SUBTU_FAIL);
    CPTR->err_count--; /* removes error counted in skytu runtu loop */
    return(ALLVRAMEM | SUBTU_FAIL);
  }
  else
  { CatErr(ALLVRAMEM | GOOD_OP);
    return(ALLVRAMEM | GOOD_OP);
  }
}
/**********************************************************
* Name     : memq()                                       *
* Function : decipher the VRAM run tu queue               *
*                                                         *
**********************************************************/
memq(rw16_32,sa_topbot,tuname)
char rw16_32[];
char sa_topbot[];
char tuname[];
{
  char *tuname_start;
  int x;


  if (lptr == NIL)
  { strcpy(tinfo.info.msg_buff, "The 'Test VRAM Memory' Run Queue is Empty");
    return(CPF_INVDAT);
  }
  
  /* process the VRAM RUN QUEUE */
  do
  { while (*lptr == BLANK)
    { lptr++;
    } 
    if (*lptr != NULL)
    {
      tuname_start = lptr;

      /* GET RW_16_32 */
      if ((*lptr == '1') && (*(lptr+1) == '6'))
      { rw16_32[0] = *lptr++;
        rw16_32[1] = *lptr++;
        rw16_32[2] = NULL;   /*set end of strings*/
      }
      else if ((*lptr == '3') && (*(lptr+1) == '2'))
      { rw16_32[0] = *lptr++;
        rw16_32[1] = *lptr++;
        rw16_32[2] = NULL;   /*set end of strings*/
      }
      else
      { sprintf(tinfo.info.msg_buff,
                "Invalid '16 or 32 bit read/write' indicator");
        break;
      }

      /* GET sa_topbot */
      if ((*lptr == 'B') || (*lptr == 'T') || (*lptr == 'b') || (*lptr == 't'))
      { sa_topbot[0] = *lptr++;
        sa_topbot[1] = NULL;
      }
      else
      { sprintf(tinfo.info.msg_buff,
                "Invalid 'Start at Top or Bottom' Indicator");
        break;
      }

       /* GET tuname */
      if ((*lptr == 'B') || (*lptr == 'b'))
      { tuname[0] = *lptr++;
        tuname[4] = NULL;
      }
      else
      { sprintf(tinfo.info.msg_buff,
                "Invalid VRAM Memory Test Unit Name!");
        break;
      }

      for (x = 1; x < 4; x++,lptr++)
      { if (ischx(*lptr)) /*received a hex character (0-f or 0-F)*/
        { tuname[x] = *lptr;
        }
        else if (*lptr == NULL) /*received an End of String character*/
        { tuname[x] = NULL;
          sprintf(tinfo.info.msg_buff,
                  "Reached end of VRAM RUN QUEUE list prematurely.");
          x = 999;
        }
        else if (*lptr == BLANK) /* ---> tu name is too short */
        { 
          x = 999;
          lptr--;   
          sprintf(tinfo.info.msg_buff,
                  "VRAM Memory Test Queue entry is too short!");
        } /*end if lptr == BLANK*/
        else /* illegal character */
        { 
          x = 999;
          sprintf(tinfo.info.msg_buff,
                 "VRAM Memory Test Queue entry contains an illegal character!");
        } /*end if lptr == BLANK*/
      } /*end for x<4 */
      if (x != 4) break;

      /*test for and remove any excessive digits in the TU name */
      if (*lptr != NULL   &&   *lptr != BLANK)
      { x = 999; /*throw out newtu*/
        sprintf(tinfo.info.msg_buff,
                "VRAM Memory Test Queue entry is too long!");
      } /* *lptr != NULL or BLANK */   
    } /* if not NULL */
  } while(FALSE); /*process once, allowing for breaks */

  if (x == 4)        /* processed a valid TU name          */
  { return (GOOD_OP);
  }
  else               /* entry was invalid */    
  { for (;((*lptr != BLANK) && (*lptr != NULL)); lptr++);
    if (*lptr == NULL) /*tu name string is terminated by an end-of-string*/
    { CatMsgs("\n--Ignoring Final Entry of the VRAM Mem. Test Queue list: ");
      CatMsgs(tuname_start);
    }
    else /* *lptr == BLANK */ /*tu name is delimited by a BLANK*/
    { *lptr = NULL;
      CatMsgs("\n--Ignoring Illegal Entry of the VRAM Mem. Test Queue list: ");
      CatMsgs(tuname_start);
      *lptr = BLANK;
    } /* if-else lptr == NULL or BLANK */
    return (CPF_INVDAT);
  }/* else entry was invalid */
} /*end memq*/
/**********************************************************************
*  Name      : b010                                                   *
*  Function  : Memory Fill with 0x00:                                 *
*              -This TU first verifies that the memory contains the   *
*               data of the last fill, if any (specified by           *
*               tinfo.memory.last_data).  This done starting at the   *
*               top or bottom (tinfo.memory.sa_topbot) in 16 or 32    *
*               bit reads (tinfo.memory.rw16_32) over the specified   *
*               range (from (tinfo.memory.memtop,tinfo.memory.membot) *
*              -Next, this TU will write 0x0000 or 0x00000000 as      *
*               specified by rw16_32, starting at the top or bottom,  *
*               and will fill the specified area.  As memory locations*
*               are written, they are immeadiately read back and      *
*               verified.                                             *
*  Return Values:                                                     *
*              Error Codes are returned for Invalid Last_data codes,  *
*               Bad compare results for verification of last fill, and*
*               and write-verify-read results.                        *
*              A Good Operation Code is returned if all verifications *
*               pass.                                                 *
*  Called by:  Usually called by the run_tu.c routine                 *
*  Calls    :  Makes calls on the testvram() routine in this file     *
**********************************************************************/
b010()
{ word retcode; 
  retcode = testvram(FILL00,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x10);
  return(retcode);
}

b020()
{ word retcode; 
  retcode = testvram(FILLFF,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x20);
  return(retcode);
}

b030()
{ word retcode; 
  retcode = testvram(FILL33,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x30);
  return(retcode);
}

b040()
{ word retcode; 
  retcode = testvram(FILLCC,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x40);
  return(retcode);
}

b050()
{ word retcode; 
  retcode = testvram(FILL55,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x50);
  return(retcode);
}

b060()
{ word retcode; 
  retcode = testvram(FILLAA,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x60);
  return(retcode);
}

b070()
{ word retcode; 
  retcode = testvram(FILLPAT,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x70);
  return(retcode);
}

b080()
{ word retcode; 
  retcode = testvram(FILLWA,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x80);
  return(retcode);
}

b090()
{ word retcode; 
  retcode = testvram(FILLLA,tinfo.memory.rw16_32,tinfo.memory.sa_topbot,0x90);
  return(retcode);
}

testvram(tucode, rw, sa, datacode)
lword  tucode;   /* tucode used in message generation and error traces */
byte   rw;       /* Specifies 16 or 32 bit reads and writes */
byte   sa;       /* Specifies whether to start at the top or bottom of VRAM */
byte  datacode;  /* Specifies the data pattern to write (encoded) */
                 /* Note: data pattern expected during the read (ie from the 
                    last write is encoded in tinfo.memory.last_data */
{ 
  lword newdata;
  lword lastdata;
  lword readdata;
  word  *cur_addrw;
  word  *end_addrw;
  lword *cur_addrl;
  lword *end_addrl;
  byte  rwerr = NO;
  byte  done  = NO;
  char  addr[30];
  byte  ldcode;
  word  rc;

do
{ 
  rc = test_if_in_vram(((tinfo.memory.memtop & 0x0FFFFFFF)+segreg), tucode); 
  if (rc > ERR_LEVEL)
  { CatMsgs("--\nCheck tinfo.memory.memtop");
    CatErr (tucode | CPF_INVDAT);
    return (tucode | CPF_INVDAT);
  }
  rc = test_if_in_vram(((tinfo.memory.membot & 0x0FFFFFFF)+segreg), tucode); 
  if (rc > ERR_LEVEL)
  { CatMsgs("--\nCheck tinfo.memory.membot");
    CatErr (tucode | CPF_INVDAT);
    return (tucode | CPF_INVDAT);
  }

  switch(ldcode = tinfo.memory.last_data) /* get data pattern last used */
  { case 0x00 : /*no lastdata - will skip the read lastdata section */ break;
    case 0x10 : lastdata = (0x00000000); break;
    case 0x20 : lastdata = (0xFFFFFFFF); break;
    case 0x30 : lastdata = (0x33333333); break;
    case 0x40 : lastdata = (0xCCCCCCCC); break;
    case 0x50 : lastdata = (0x55555555); break;
    case 0x60 : lastdata = (0xAAAAAAAA); break;
    case 0x70 : lastdata = (tinfo.memory.lastpattern); break;
    case 0x80 : /*lastdata=((word)cur_addrw); -set when addr generated*/ break;
    case 0x90 : /*lastdata=((lword)cur_addrl);-set when addr generated*/ break;
    default :
           sprintf(tinfo.info.msg_buff,
                  "Illegal 'tinfo.memory.last_data' code : %X",ldcode);
           CatErr (tucode | CPF_INVDAT);
           return (tucode | CPF_INVDAT);
  } /* end switch */

  switch(datacode) /* get new data pattern to fill vram with */
  { case 0x10 : newdata = (0x00000000); break;
    case 0x20 : newdata = (0xFFFFFFFF); break;
    case 0x30 : newdata = (0x33333333); break;
    case 0x40 : newdata = (0xCCCCCCCC); break;
    case 0x50 : newdata = (0x55555555); break;
    case 0x60 : newdata = (0xAAAAAAAA); break;
    case 0x70 : newdata = (tinfo.memory.newpattern);
                tinfo.memory.lastpattern = newdata; break;
    case 0x80 : /*newdata=(( word)cur_addrw);-set when addr generated*/ break;
    case 0x90 : /*newdata=((lword)cur_addrl);-set when addr generated*/ break;
    default :
           sprintf(tinfo.info.msg_buff,
                  "Illegal Fill data-code,'%X', sent from %X",datacode,tucode);
           CatErr (tucode | THISTUFAIL);
           return (tucode | THISTUFAIL);
  } /* end switch */


/*  READ LAST DATA FILL AND VERIFY  */

    if ((sa == 't') || (sa == 'T'))  
    { cur_addrw = (word *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
      cur_addrl = (lword *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
      end_addrw = (word *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-2); 
      end_addrl = (lword *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-4); 
      sa = 'T';
    }
    else  /* if ((sa == 'b') || (sa == 'B')) */ 
    { cur_addrw = ( word *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-2); 
      cur_addrl = (lword *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-4); 
      end_addrw = ( word *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
      end_addrl = (lword *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
      sa = 'B';
    } 


  /*last_data code=0x00 signifies VRAM contains non testable data patterns */
  while ((ldcode != 0x00) && (rwerr != YES) && (done != YES))
  {
    if ( (ldcode != 0x90) && ((rw == 16) || (ldcode == 0x80)) )
    { /* comparing 16 bit data */ 
      if (ldcode==0x80) lastdata=(word)cur_addrw;/*data should=lo address word*/
      readdata=(*cur_addrw);  /* read 16 bit data from the current address */
      lastdata = (word)lastdata; /*remove most significant word for comparison*/
      if ( readdata != lastdata) /*compare data last written to data read */
      { sprintf(addr,"Address %X",cur_addrw);
        rwerr = YES;
      }
      (sa == 'T' ? cur_addrw++ : cur_addrw--); /* inc/dec address pointer */
    }
    if ( (ldcode != 0x80) && ((rw == 32) || (ldcode == 0x90)) )
    { /* comparing 32 bit data */ 
      if (ldcode==0x90) lastdata=(lword)cur_addrl;/*data should equal address */
      readdata =  (*cur_addrl);  /* read 32 bit data from the current address */
      if ( readdata != lastdata) /*compare data last written to data read */
      { sprintf(addr,"Address %X",cur_addrl);
        rwerr = YES;
      }
      (sa == 'T' ? cur_addrl++ : cur_addrl--); /* inc/dec address pointer */
    }
    if  ((sa == 'T' && (( cur_addrw > end_addrw) || (cur_addrl > end_addrl)))
      || (sa == 'B' && (( cur_addrw < end_addrw) || (cur_addrl < end_addrl))))
    { done = YES;
    } /*end if st == end*/
  } /* while (ld code != 0  and  rwerr != YES and done != yes) */
  if (FALSE) break;

  if (rwerr == YES)
  { sprintf(tinfo.info.msg_buff,
            "Bad Compare:  Data last written to ");
    CatMsgx(addr);
    CatMsgs(" = ");
    CatMsgx(lastdata);
    CatMsgs("\n--Data read from ")
    CatMsgx(addr);
    CatMsgs(" = ");
    CatMsgx(readdata);
    CatErr(tucode | THISTUFAIL);

    /*data in VRAM will not be consistant for next VRAM Memory Test*/ 
    tinfo.memory.last_data = 0x00; 

    return (tucode | THISTUFAIL);
  } /* end if (RD != LD) */
  /*end of read last data written verification */


  /*start of write and read-verify of new data fill */
  if ((sa == 't') || (sa == 'T'))  
  { cur_addrw = (  word *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
    cur_addrl = ( lword *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
    end_addrw = (  word *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-2); 
    end_addrl = ( lword *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-4); 
    sa = 'T'; 
  }
  else  /* if ((sa == 'b') || (sa == 'B')) */ 
  { cur_addrw = ( word *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-2); 
    cur_addrl = (lword *)((tinfo.memory.membot & 0x0FFFFFFF)+segreg-4); 
    end_addrw = (  word *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
    end_addrl = ( lword *)((tinfo.memory.memtop & 0x0FFFFFFF)+segreg); 
    sa = 'B'; 
  }
  rwerr = NO; done = NO;
  while ((rwerr != YES) && (done != YES))
  {
    if ( (datacode != 0x90) && ((rw == 16) || (datacode == 0x80)) )
    { /* writing 16 bit data */ 
      if (datacode==0x80) newdata=(word)cur_addrw;/*data should=addressword lo*/
      *cur_addrw = newdata;
      if ((word)newdata != (word)(readdata = /*readword*/(*cur_addrw)))
      { rwerr = YES;
        sprintf(addr,"Address %X",cur_addrw);
      }
      (sa == 'T' ? cur_addrw++ : cur_addrw--); /* inc/dec address pointer */
    }
    else if ( (datacode != 0x80) && ((rw == 32) || (datacode == 0x90)) )
    { /* writing 32 bit data */ 
      if (datacode==0x90) newdata=(lword)cur_addrl;/*data should equal address*/
      *cur_addrl = newdata;
      if ((lword)newdata != (lword)(readdata = /*readlword*/(*cur_addrl)))
      { rwerr = YES;
        sprintf(addr,"Address %X",cur_addrl);
      }
      (sa == 'T' ? cur_addrl++ : cur_addrl--); /* inc/dec address pointer */
    }
    if  ((sa == 'T' && (( cur_addrw > end_addrw) || (cur_addrl > end_addrl)))
      || (sa == 'B' && (( cur_addrw < end_addrw) || (cur_addrl < end_addrl))))
    { done = YES;
      /*inform next VRAM test that the last data written was 0x0000 */
      tinfo.memory.last_data = datacode;
      CatErr(tucode | GOOD_OP);
      return(tucode | GOOD_OP);
    } /*end if st == end*/
  } /* end while (rwerr != YES and done != yes) */
  if (rwerr == YES)
  { sprintf(tinfo.info.msg_buff,
    "Write Verification Failed Immediately After Write:");
    CatMsgs("\n--Data written to ")
    CatMsgx(addr);
    CatMsgs(" = ");
    CatMsgx(newdata);
    CatMsgs("\n--Data read from ")
    CatMsgx(addr);
    CatMsgs(" = ");
    CatMsgx(readdata);
    CatErr(tucode | THISTUFAIL);

    /*data in VRAM will not be consistant for next VRAM Memory Test*/ 
    tinfo.memory.last_data = 0x00;

    return (tucode | THISTUFAIL);
  } /* end if (RD != LD) */
} while (FALSE);/* do once, but redo if we were interrupted   */
} /* end test_vram() */

load_palette()
{ word pal_data;   /* palette data and index counter */
  GETIOBASE; /* get the IO Base Address to access the Display Controller Regs */
  
  writebyte(0x60, INDX_ADDR);  /* access indexed reg 60: Palette Index Lo */
  writebyte(0x00, DATAB_ADDR); /*set Palette Index Lo to 0x00 (first location)*/
  /* NOTE: for Skyway: Brooktree Palette DAC, don't need Palette Index Hi */

  writebyte(0x65, INDX_ADDR);  /* access indexed reg 65: Palette Data */
  for (pal_data=0x00; (pal_data >= 0xFF); pal_data++)
  { writebyte(pal_data, DATAB_ADDR); /* write the palette RED data */
    writebyte(pal_data, DATAB_ADDR); /* write the palette GREEN data */
    writebyte(pal_data, DATAB_ADDR); /* write the palette BLUE data */
  }
}

/****************************************************************
*  Name:  ischx.h                                               *
*  Funct: Tests a character (chr) to see IS the Character HeX   *
*                                                               *
****************************************************************/
#define  IS_HEX   1
#define  ISNT_HEX 0
ischx(chr) 
char chr;
{ if (chr >= '0' && chr <= '9' ||
      chr >= 'a' && chr <= 'f' ||
      chr >= 'A' && chr <= 'F' ) 
  {   return(IS_HEX);
  }
  else
  {   return(ISNT_HEX);
  }
} 
        
