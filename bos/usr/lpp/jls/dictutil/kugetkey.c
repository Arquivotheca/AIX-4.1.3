static char sccsid[] = "@(#)45  1.5  src/bos/usr/lpp/jls/dictutil/kugetkey.c, cmdKJI, bos411, 9428A410j 8/27/91 12:25:17";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kugetkey, kutrnstr, kutrnfnm, mkbl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************  *
 * MODULE NAME:         kugetkey
 *
 * DESCRIPTIVE NAME:    user dictionary get key handore
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        10885 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kugetkey
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kugetkey(function)
 *
 *  INPUT:              function        : process mode
 *
 *  OUTPUT:             NA.
 *
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              strcpy()
 *                              strncpy()
 *                              strchr()
 *                              strrchr();
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
/* B.EXTCUR */
#if defined(EXTCUR)  
#include <stdlib.h>
#include "kuke.h"

/*
 * Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

int kugetkey(function)

int function;
{
int     code;
int     rc;

 	code = getch();    

	switch ( function ) {
	    case GET_CHAR :
		rc = code;
		break;
	    case CHECK_TABLE :
		rc = OK;
		break;
	    default :
		rc = ERR;
		break;
	};
	return( rc );
}
#else
/*
 *      include Standard.
 */
#include "kuke.h"

/*#include <memory.h>*/
#include <sys/select.h>
#include <sys/time.h>

/*
 * Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

int kugetkey(function)
int function;
{
   extern struct fkkey fkkeytbl[];
   extern int          nkey;
   extern int          fkmaxlen;
   extern int          baud;
   extern int          wait_loop;
   extern int          wait_time;
   extern int          scrollflg;
   extern char         trigertable[];

   static int  codebfnchr         = 0;
   static int  readbfnchr         = 0;
   static int  loopflag           = TRUE;
   static int  compulsion_exit    = FALSE;
   static int  dc3flg             = FALSE;
   static int  dc1flg             = FALSE;
   static int  iscode             = FALSE;
   static int  savecode           = 0;

   static char key_reset[]  = {0x1b,0x5b,0x31,0x32,0x32,0x71,0x00};

#ifndef TERMH_CH
   static char key_action[] = {0x1b,0x5b,0x31,0x31,0x34,0x71,0x00};
#endif TERMH_CH

   static char codebuffer[256]    = "";
   static char readbuffer[BUFSIZ] = "";

   register int i;
   register char *fp, *nfp;

   struct timeval timeout;
   int  code, searchcond, byte, readloop;
   int nfdsmsgs, readlist, writelist, exceptlist;
   char workstr[BUFSIZ + 1], *kutrnfnm(), *kutrnstr();
   char *sp, *strncpy(), *strchr(), *strrchr();

   switch(function) {
      case SET_UP_TBL:
	 nkey = 0;
	 strcpy(trigertable, "");
	 nkey += mktbl(key_right       , KEY_RIGHT    );
	 nkey += mktbl(key_left        , KEY_LEFT     );
	 nkey += mktbl(key_down        , KEY_DOWN     );
	 nkey += mktbl(key_up          , KEY_UP       );
	 nkey += mktbl(key_newline     , KEY_NEWL     );
	 nkey += mktbl(key_backspace   , KEY_BACKSPACE);
	 nkey += mktbl(key_dc          , KEY_DC       );
	 nkey += mktbl(key_tab         , KEY_TAB      );
	 nkey += mktbl(key_back_tab    , KEY_BTAB     );
	 nkey += mktbl(key_dl          , KEY_DL       );
	 nkey += mktbl(key_f0          , KEY_F0       );
	 nkey += mktbl(key_f1          , KEY_F(1)     );
	 nkey += mktbl(key_f2          , KEY_F(2)     );
	 nkey += mktbl(key_f3          , KEY_F(3)     );
	 nkey += mktbl(key_f4          , KEY_F(4)     );
	 nkey += mktbl(key_f5          , KEY_F(5)     );
	 nkey += mktbl(key_f6          , KEY_F(6)     );
	 nkey += mktbl(key_f7          , KEY_F(7)     );
	 nkey += mktbl(key_f8          , KEY_F(8)     );
	 nkey += mktbl(key_f9          , KEY_F(9)     );
	 nkey += mktbl(key_f10         , KEY_F(10)    );
	 nkey += mktbl(key_f11         , KEY_F(11)    );
	 nkey += mktbl(key_f12         , KEY_F(12)    );
	 nkey += mktbl(key_home        , KEY_HOME     );
	 nkey += mktbl(key_ic          , KEY_IC       );
	 nkey += mktbl(key_il          , KEY_IL       );
	 nkey += mktbl(key_ll          , KEY_LL       );
	 nkey += mktbl(key_npage       , KEY_NPAGE    );
	 nkey += mktbl(key_ppage       , KEY_PPAGE    );
	 nkey += mktbl(key_quit        , KEY_QUIT     );
	 nkey += mktbl(key_scroll_left , KEY_SCL      );
	 nkey += mktbl(key_scroll_right, KEY_SCR      );
	 nkey += mktbl(key_select      , KEY_SEL      );
	 nkey += mktbl(key_sf          , KEY_SF       );
	 nkey += mktbl(key_sf1         , KEY_SF1      );
	 nkey += mktbl(key_sf2         , KEY_SF2      );
	 nkey += mktbl(key_sf3         , KEY_SF3      );
	 nkey += mktbl(key_sf4         , KEY_SF4      );
	 nkey += mktbl(key_sf5         , KEY_SF5      );
	 nkey += mktbl(key_sf6         , KEY_SF6      );
	 nkey += mktbl(key_sf7         , KEY_SF7      );
	 nkey += mktbl(key_sf8         , KEY_SF8      );
	 nkey += mktbl(key_sf9         , KEY_SF9      );
	 nkey += mktbl(key_sf10        , KEY_SF10     );
	 nkey += mktbl(key_sr          , KEY_SR       );
	 nkey += mktbl(key_stab        , KEY_STAB     );
	 nkey += mktbl(key_catab       , KEY_CATAB    );
	 nkey += mktbl(key_clear       , KEY_CLEAR    );
	 nkey += mktbl(key_command     , KEY_CMD      );
	 nkey += mktbl(key_command_pane, KEY_CPN      );
	 nkey += mktbl(key_ctab        , KEY_CTAB     );
	 nkey += mktbl(key_do          , KEY_DO       );
	 nkey += mktbl(key_eic         , KEY_EIC      );
	 nkey += mktbl(key_end         , KEY_END      );
	 nkey += mktbl(key_eol         , KEY_EOL      );
	 nkey += mktbl(key_eos         , KEY_EOS      );
	 nkey += mktbl(key_help        , KEY_HLP      );
	 nkey += mktbl(key_next_pane   , KEY_NPN      );
	 nkey += mktbl(key_prev_cmd    , KEY_PCMD     );
	 nkey += mktbl(key_prev_pane   , KEY_PPN      );
	 nkey += mktbl(key_action      , U_ACTIONKEY  );
	 nkey += mktbl(key_reset       , U_RESETKEY   );
	 return(OK);
	 break;
      case GET_CHAR:
	 if (iscode) {
	    code     = savecode;
	    savecode = 0;
	    iscode   = FALSE;
	    return(code);
	 }
	 while (TRUE) {
	    if (loopflag) { /* No data in code_buffer or Search failed */
	       if (readbfnchr) { /* There are sevral characters in readbuffer */
		  *(codebuffer + codebfnchr) = *readbuffer;
		  codebfnchr++;
		  *(codebuffer + codebfnchr) = (char)NULL;
		  fp = readbuffer;
		  while(*fp = *(fp+1)) fp++;
		  readbfnchr--;
	       }
	       else {
		  readloop = TRUE;
		  while(readloop) {
		     if (scrollflg) {
			nfdsmsgs        = STDIN + 1;
			readlist        = 1 << STDIN;
			writelist       = 0;
			exceptlist      = 0;
			timeout.tv_sec  = 1; /* Wait 1 sec. then return */
			timeout.tv_usec = 0;
			if (select(nfdsmsgs,   &readlist,
				   &writelist, &exceptlist,
				   &timeout) == 0) { /* time out */
			   scrollflg = 0; /* Continuous scroll stoped */
			   return(KEY_INFO_DISP);
			}
		     }
		     if ( (i = read(STDIN, workstr, BUFSIZ)) == ERR) {
			/* readloop = TRUE; */
			continue; /* read again */
		     }
		     else { /* read succeeded */
			*(workstr + i) = (char)NULL;
			if ( i ) { /* Read some data */
			   if (i != 1) { /* readed data length is longer than 1 */
			      sp = workstr;
			      while((fp = strchr(sp, DC3_CHAR)) != (char *)NULL) { /* Ctrl + S found */
				 nfp = fp;
				 while(*fp == *nfp) nfp++;
				 if (*nfp != (char)NULL) {
				    sp  = fp;
				    i  -= (nfp - fp);
				    while(*(fp++) = *(nfp++));
				 }
				 else { /* there is no code except Ctrl+S after fp */
				    memset(fp, NULL, (i - (fp - workstr)));
				    i      = fp - workstr;
				    dc3flg = TRUE;
				    break; /* break out Ctrl+S search loop */
				 }
			      } /* end of Ctrl+S search while loop */
			      sp = workstr;
			      while((fp = strchr(sp, DC1_CHAR)) != (char *)NULL) { /* Ctrl + Q found */
				 nfp = fp;
				 while(*fp == *nfp) nfp++;
				 if (*nfp != (char)NULL) {
				    sp  = fp;
				    i  -= (nfp - fp);
				    while(*(fp++) = *(nfp++));
				 }
				 else { /* there is no code except Ctrl+Q after fp */
				    memset(fp, NULL, (i - (fp - workstr)));
				    i      = fp - workstr;
				    dc1flg = TRUE;
				    break; /* break out Ctrl+Q search loop */
				 }
			      } /* end of Ctrl+Q search while loop */
			   }
			   else { /* Readed data length is just 1 byte */
			      if (scrollflg) {
				 if ((*workstr == DC3_CHAR) ||
				     (*workstr == DC1_CHAR)   ) {
				    continue; /* Read again */
				 }
			      }
			      if (dc3flg) {
				 if (*workstr == (char)DC3_CHAR) {
				    *workstr = (char)NULL;
				    i        = 0;
				    /* dc3flg   = TRUE; */
				    continue; /* read again   */
				 }
				 else {
				    dc3flg = FALSE;
				 }
			      }
			      if (dc1flg) {
				 if (*workstr == (char)DC1_CHAR) {
				    *workstr = (char)NULL;
				    i        = 0;
				    /* dc1flg   = TRUE; */
				    continue; /* read again   */
				 }
				 else {
				    dc1flg = FALSE;
				 }
			      }
			      break; /* break out while loop */
			   }
			   if ( i ) { /* after remove Ctrl+S and Ctrl+Q code, data length is still not zero */
			      break; /* break out while loop */
			   }
			   else { /* after remove Ctrl+S code, data length becomes zero */
			      continue; /* read again */
			   }
			}
			else { /* Readed data length = 0 */
			   break; /* break read while loop */
			}
		     }
		  }
		  if ( i ) { /* Read some data */
		     *(codebuffer + codebfnchr) = *workstr;
		     codebfnchr++;
		     *(codebuffer + codebfnchr) = (char)NULL;
		     strncpy(readbuffer, workstr + 1, i - 1);
		     readbfnchr = i - 1;
		     loopflag = FALSE;
		  }
		  else { /* Readed data length = 0 */
		     if (codebfnchr) {
			compulsion_exit = TRUE; /* Return all data to caller */
			loopflag = FALSE;
		     }
		     else {
			; /* Do nothing special */
		     }
		  }
	       }
	    }
	    if (compulsion_exit) {
	       code = (int)*codebuffer;
	       if (--codebfnchr == 0) {
		  compulsion_exit = FALSE;
		  loopflag        = TRUE;
	       }
	       else {
		  ; /* Do nothing special */
	       }
	       fp = codebuffer;
	       while(*fp = *(fp+1)) fp++;
	       break; /* Break out main while loop */
	    }
	    if (strchr(trigertable, *codebuffer) == (char *)NULL) {
	       /* Top character of code_buffer is not trriger character */
	       code = (int)*codebuffer;
	       codebfnchr--;
	       fp = codebuffer;
	       while(*fp = *(fp+1)) fp++;
	       if (codebfnchr) { /* codebfnchr not equal 0 */
		  loopflag = FALSE;
	       }
	       else { /* codebfnchr equal 0 */
		  loopflag = TRUE;
	       }
	       break; /* Break out main while loop */
	    }
	    else { /* Top character of code_buffer is trigger character */
	       searchcond = NOSTRING;
	       for(i = 0; i < nkey; i++) {
		  if (strncmp(codebuffer, fkkeytbl[i].fstring, codebfnchr) == 0) {
		     if (strcmp(codebuffer, fkkeytbl[i].fstring) == 0) {
			searchcond = FULLSTRING;
			break;
		     }
		     else {
			searchcond = PARTSTRING;
			break;
		     }
		  }
	       } /* end of for loop */

	       if (searchcond == FULLSTRING) {
		  code = fkkeytbl[i].fcode;
		  memset(codebuffer, NULL, codebfnchr);
		  codebfnchr = 0;
		  break;
	       }
	       else if (searchcond == PARTSTRING) {
		  loopflag = TRUE;
	       }
	       else { /* search failed. */
		  memset(codebuffer, NULL, codebfnchr);
		  memset(readbuffer, NULL, readbfnchr);
		  codebfnchr = 0;
		  code       = UNKNOWN_CHR;
		  loopflag   = TRUE;
		  if (readbfnchr) {
		     /* memset(codebuffer, NULL, codebfnchr); */
		     /* memset(readbuffer, NULL, readbfnchr); */
		     /* codebfnchr = 0;                       */
		     readbfnchr = 0;
		     /* code       = UNKNOWN_CHR;             */
		     /* loopflag   = TRUE;                    */
		  }
		  else {
		     /* memset(codebuffer, NULL, codebfnchr); */
		     /* memset(readbuffer, NULL, readbfnchr); */
		     /* codebfnchr = 0;                       */
		     /* readbfnchr = 0;                       */
		     /* code       = UNKNOWN_CHR;             */
		     /* loopflag   = TRUE;                    */
		     byte = 1;
		     while(byte) {
			ioctl(STDIN, TCFLSH, FLASHINPUT);
			sleep((unsigned int)wait_time);
			ioctl(STDIN, TIONREAD, &byte);
		     }
		  }
		  break;
	       }
	    }
	 } /* End of main while loop */
	 if (scrollflg) {
	    if (scrollflg == SCRUPREF) {
	       if ((code != KEY_DOWN) && (code != KEY_NEWL) && (code != '\n')) {
		  savecode  = code;
		  code      = KEY_INFO_DISP;
		  iscode    = TRUE;
		  scrollflg = 0; /* Continuous scroll stoped */
	       }
	    }
	    else if (scrollflg == SCRDWREF) {
	       if (code != KEY_UP) {
		  savecode  = code;
		  code      = KEY_INFO_DISP;
		  iscode    = TRUE;
		  scrollflg = 0; /* Continuous scroll stoped */
	       }
	    }
	 }
	 return(code);
	 break;
      case CHECK_TABLE:
	 for (i = 0; i < nkey; i++) {
		 fprintf(stdout, "function code = %s, string = %s\n", kutrnfnm(fkkeytbl[i].fcode), kutrnstr(workstr, fkkeytbl[i].fstring));
	 }
	 fprintf(stdout, "Trriger table = %s\n", kutrnstr(workstr, trigertable));
	 fprintf(stdout, "Maximum string length = %d\n", fkmaxlen);
	 return(OK);
	 break;
      default:
	 return(ERR);
	 break;
   }
   return(ERR);
}

int mktbl(fstr, fcode)
char *fstr;
int fcode;
{
	extern struct fkkey fkkeytbl[];
	extern int          nkey;
	extern int          fkmaxlen;
	extern char         trigertable[];

	static char str_null[] = "";

	int len;
	char workstr[2];

	if (strcmp(fstr, str_null) != 0) {
		fkkeytbl[nkey].fstring = fstr;
		fkkeytbl[nkey].fcode   = fcode;
		if (strchr(trigertable, *fstr) == NULL) {
			workstr[0] = *fstr;
			workstr[1] = (char)NULL;
			strcat(trigertable, workstr);
		}
		if ((len = strlen(fstr)) > fkmaxlen) {
			fkmaxlen = len;
		}
		return(1);
	}
	else {
		return(0);
	}
}

char *kutrnfnm(i)
int i;
{
	switch(i) {
		case KEY_BTAB     :  return("KEY_BTAB     ");     break;
		case KEY_BACKSPACE:  return("KEY_BACKSPACE");     break;
		case KEY_CATAB    :  return("KEY_CATAB    ");     break;
		case KEY_CLEAR    :  return("KEY_CLEAR    ");     break;
		case KEY_CMD      :  return("KEY_CMD      ");     break;
		case KEY_CPN      :  return("KEY_CPN      ");     break;
		case KEY_CTAB     :  return("KEY_CTAB     ");     break;
		case KEY_DC       :  return("KEY_DC       ");     break;
		case KEY_DL       :  return("KEY_DL       ");     break;
		case KEY_DO       :  return("KEY_DO       ");     break;
		case KEY_DOWN     :  return("KEY_DOWN     ");     break;
		case KEY_EIC      :  return("KEY_EIC      ");     break;
		case KEY_END      :  return("KEY_END      ");     break;
		case KEY_EOL      :  return("KEY_EOL      ");     break;
		case KEY_EOS      :  return("KEY_EOS      ");     break;
		case KEY_F0       :  return("KEY_F0       ");     break;
		case KEY_F(1)     :  return("KEY_F(1)     ");     break;
		case KEY_F(10)    :  return("KEY_F(10)    ");     break;
		case KEY_F(11)    :  return("KEY_F(11)    ");     break;
		case KEY_F(12)    :  return("KEY_F(12)    ");     break;
		case KEY_F(2)     :  return("KEY_F(2)     ");     break;
		case KEY_F(3)     :  return("KEY_F(3)     ");     break;
		case KEY_F(4)     :  return("KEY_F(4)     ");     break;
		case KEY_F(5)     :  return("KEY_F(5)     ");     break;
		case KEY_F(6)     :  return("KEY_F(6)     ");     break;
		case KEY_F(7)     :  return("KEY_F(7)     ");     break;
		case KEY_F(8)     :  return("KEY_F(8)     ");     break;
		case KEY_F(9)     :  return("KEY_F(9)     ");     break;
		case KEY_HLP      :  return("KEY_HLP      ");     break;
		case KEY_HOME     :  return("KEY_HOME     ");     break;
		case KEY_IC       :  return("KEY_IC       ");     break;
		case KEY_IL       :  return("KEY_IL       ");     break;
		case KEY_LEFT     :  return("KEY_LEFT     ");     break;
		case KEY_LL       :  return("KEY_LL       ");     break;
		case KEY_NEWL     :  return("KEY_NEWL     ");     break;
		case KEY_NPN      :  return("KEY_NPN      ");     break;
		case KEY_NPAGE    :  return("KEY_NPAGE    ");     break;
		case KEY_PPAGE    :  return("KEY_PPAGE    ");     break;
		case KEY_PCMD     :  return("KEY_PCMD     ");     break;
		case KEY_PPN      :  return("KEY_PPN      ");     break;
		case KEY_QUIT     :  return("KEY_QUIT     ");     break;
		case KEY_RIGHT    :  return("KEY_RIGHT    ");     break;
		case KEY_SCL      :  return("KEY_SCL      ");     break;
		case KEY_SCR      :  return("KEY_SCR      ");     break;
		case KEY_SEL      :  return("KEY_SEL      ");     break;
		case KEY_SF       :  return("KEY_SF       ");     break;
		case KEY_SF1      :  return("KEY_SF1      ");     break;
		case KEY_SF10     :  return("KEY_SF10     ");     break;
		case KEY_SF2      :  return("KEY_SF2      ");     break;
		case KEY_SF3      :  return("KEY_SF3      ");     break;
		case KEY_SF4      :  return("KEY_SF4      ");     break;
		case KEY_SF5      :  return("KEY_SF5      ");     break;
		case KEY_SF6      :  return("KEY_SF6      ");     break;
		case KEY_SF7      :  return("KEY_SF7      ");     break;
		case KEY_SF8      :  return("KEY_SF8      ");     break;
		case KEY_SF9      :  return("KEY_SF9      ");     break;
		case KEY_SR       :  return("KEY_SR       ");     break;
		case KEY_STAB     :  return("KEY_STAB     ");     break;
		case KEY_TAB      :  return("KEY_TAB      ");     break;
		case KEY_UP       :  return("KEY_UP       ");     break;
		default           :  return("UNKNOWN      ");     break;
	}
	return((char *)NULL);
}

char *kutrnstr(ret, str)
char *ret, *str;
{
	char hex[3], *i;

	i = str;
	strcpy(ret, "");

	while(*i != NULL) {
		sprintf(hex, "%2x", *i);
		strcat(ret, hex);
		i++;
	}

	return(ret);
}
#endif
