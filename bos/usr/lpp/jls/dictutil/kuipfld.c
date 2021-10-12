static char sccsid[] = "@(#)49	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kuipfld.c, cmdKJI, bos411, 9428A410j 7/23/92 01:28:23";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuipfld
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ***********************
 *
 * MODULE NAME:         kuipfld
 *
 * DESCRIPTIVE NAME:    user dictionary string data input
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
 *  MODULE SIZE:        3430 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kuipfld
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kuipfld(udcb,line,col,data,data_len,mode,iplen)
 *
 *  INPUT:              udcb            : pointer to UDCB
 *                      line            : input field line
 *                      col             : input field column
 *                      data            : pointer to data area
 *                      data_len        : length of data area
 *                      mode            : input field mode
 *
 *  OUTPUT:             data            : pointer to data area
 *                      iplen           : length of input data
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
 *                              kugetc
 *                              kudisply
 *                              kugetcmp
 *                      Standard Liblary.
 *                              memcpy
 *                              memset
 *                              strcpy
 *                              strcat
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

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include "kut.h"
#include "kucode.h"
#include "kje.h"
#include <stdio.h>
/*#include <memory.h>*/

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

extern   int        cnvflg;     /* conversion Type Code */

#define	DSIZE	256		/* buffer size				*/
#define	CLENGTH	mbswidth( data, ((lastch == -1) ? 0 : lastch+1) )

int	kuipfld(udcb, line, col, datai, data_len, mode, iplen, initd, convert)

UDCB    *udcb;                  /* pointer to UDCB      		*/
short   line;                   /* input field start line       	*/
short   col;                    /* input field start column     	*/
char    *datai;                 /* data input area      		*/
short   data_len;               /* length of data area (max DSIZE) 	*/
short   mode;                   /* mode of input field  		*/
short   *iplen;                 /* length of input data 		*/
short   initd;                  /* initial data display flag    	*/
int     convert;                /* conversion flag      		*/
{

    int         kugetc();       /* get vharacter        		*/
    int         kudisply();    /* display data         		*/
    int         kugetcmp();     /* get cursor move point        	*/

    short       curpos;         /* cursor position      		*/
    short       ncurpos;        /* cursor position      		*/
    short       lastch;         /* position of last character   	*/
    short       curmp;          /* cursor move point    		*/
    short       triger;         /* triger key flag      		*/
    short       insmode;        /* insert mode flag     		*/
    short       field_len;      /* input filed length (0 -- data_len-1) */
    short       dispos;         /* display start position       	*/
    short       ndispos;        /* display start position       	*/
    short       dislen;         /* display length       		*/
    int         i,j,k;          /* work varible 			*/
    int         ret_code;       /* ret_code     			*/
    int         getcode;        /* getcharacter code    		*/
    int         getcode2;       /* getcharacter code    		*/
    int         getcode3;       /* getcharacter code (EUC 3rd byte) 	*/
    int		length;

    wchar_t     p_wc;
    int         rc;

    size_t	ilen;		/* conversion input length		*/
    size_t	olen;		/* conversion output length		*/

    char	cnvdata[DSIZE+1]; 	/* data area for display	*/
    char	*data;			/* pointer to data area		*/
    char	waptr[DSIZE+1];		/* work buffer			*/
    char	padptr[DSIZE+1];	/* work buffer			*/

    /*------------------------------------------------------------------*
     * data conversion
     *------------------------------------------------------------------*/
    if ( convert == U_SJIS ) {
	strcpy( cnvdata, datai );
    }
    else {	/* EUC 	*/
	ilen = strlen(datai);
	if ( ilen > 0 ) {
	    olen = DSIZE;
	    kjcnvste( datai, &ilen, cnvdata, &olen );
	    cnvdata[DSIZE-olen] = '\0';
	}
	else {
	    cnvdata[0] = '\0';
	}
    }
    data = cnvdata;

    /*------------------------------------------------------------------*
     * check of data
     *------------------------------------------------------------------*/
    if ( (data_len < 1) || (data == NULL) ) {
      return( -1 );
    }

    /*------------------------------------------------------------------*
     * check of display position
     *------------------------------------------------------------------*/
    if ( (line < 0) || (udcb->ymax < line) ||
	(col < 0)  || (U_MAXCOL + udcb->xoff< col) ) {
      return( -2 );
    }

    /*------------------------------------------------------------------*
     * check of mode
     *------------------------------------------------------------------*/
    if ( (mode != T_GOKU) && (mode != T_YOMI) && (mode != T_FILE) ) {
      return( -3 );
    }

    /*------------------------------------------------------------------*
     * set cursor move point
     *------------------------------------------------------------------*/
    curmp = ((mode == T_FILE) ? 1:2);

    /*------------------------------------------------------------------*
     * set data length
     *------------------------------------------------------------------*/
    data_len = MIN( data_len , (U_MAXCOL + udcb->xoff - col) );
    /* data_len = data_len - (data_len % 2);    */
    field_len = data_len - 1;

    /*------------------------------------------------------------------*
     * set SPACE to padptr[DSIZE]
     *------------------------------------------------------------------*/
    if ( mode == T_FILE ) {
	for ( i=0; i<DSIZE; i++ )
	    *(padptr + i) = U_1SPACE;
    }
    else if ( cnvflg == U_SJIS ) {
	for ( i=0; i<DSIZE; i+=2 ) {
	    *(padptr + i)     = U_2SPACEH;   /*  sjis 1  */
	    *(padptr + i + 1) = U_2SPACEL;
	}
    }
    else if ( cnvflg == U_EUC ) {
	for ( i=0; i<DSIZE; i+=2 ) {
	    *(padptr + i)     = E_2SPACEH;   /*  euc 1  */
	    *(padptr + i + 1) = E_2SPACEL;
	}
    }
    else {
	for ( i=0; i<DSIZE; i++ )
	    *(padptr + i) = U_1SPACE;
    }

    /*------------------------------------------------------------------*
     * serch NULL code from top of data area
     * The data area is filled by SPACE except for the effective data
     *------------------------------------------------------------------*/
    for ( i=0; i<DSIZE; i++ ) {
    	if ( *(data + i) == NULL ) {
	    if ( mode == T_FILE ) {
	  	for ( j=i; j<DSIZE; j++ )
	    	    *(data + j) = U_1SPACE;
	    }
	    else {
	     	if ( cnvflg == U_EUC ) {
		    for ( j=i; j<DSIZE; j+=2 ) {
		    	*(data + j)     = E_2SPACEH;  /* euc 2 */
		    	*(data + j + 1) = E_2SPACEL;
		    }
             	}
		else {	/* cnvflg == U_SJIS */
		    for ( j=i; j<DSIZE; j+=2 ) {
		    	*(data + j)     = U_2SPACEH;  /* sjis 2 */
		    	*(data + j + 1) = U_2SPACEL;
		    }
		}
	    }
	    break;
       	}
    }
    lastch = i - 1;	/* (number of effective length based on bytes)-1*/

    /*------------------------------------------------------------------*
     * These buffers are terminated by NULL.
     *------------------------------------------------------------------*/
    cnvdata[DSIZE] = padptr[DSIZE] = waptr[DSIZE] = 0x00;

    /*------------------------------------------------------------------*
     * find valid code from last of data
     * Recalculate lastch value
     *------------------------------------------------------------------*/
    while ( lastch > -1 ) {
	if ( cnvflg == U_SJIS ) {
	    if( (lastch > 0)                    &&
		(*(data+lastch-1) == U_2SPACEH) &&   /*  sjis 3 */
		(*(data+lastch) == U_2SPACEL)            ) {
		/* DBCS blank code    */
		lastch = lastch - 2;
		continue;
     	    } else if(*(data + lastch) == U_1SPACE) {
		/* SBCS blank code    */
		lastch = lastch - 1;
		continue;
            }
       	}
	if ( cnvflg == U_EUC ) {
            if( (lastch > 0)                    &&
		(*(data+lastch-1) == E_2SPACEH) &&   /*  euc 3 */
		(*(data+lastch) == E_2SPACEL)            ) {
		/* DBCS blank code    */
		lastch = lastch - 2;
		continue;
     	    } else if(*(data + lastch) == U_1SPACE) {
		/* SBCS blank code    */
		lastch = lastch - 1;
		continue;
            }
       	}
   	break;
    }
    if ( lastch < 0 ) lastch = -1;

    insmode = 0;	/* Replace mode is initial		*/
    ncurpos = 0;	/* Cursor position in coulmn is 0	*/
    ndispos = 0;	/* Cursor position in byte is 0		*/
    dislen = data_len;	/* The all string is displayed in first	*/

    /*------------------------------------------------------------------*
     * initial data display
     *------------------------------------------------------------------*/
    if ( initd == C_SWON ) {
	/*
	 * length(bytes) of string: lastch + 1
  	 * length(bytes) of SPACEs: data_len - CLENGTH
	 */
	length = (lastch + 1) + (data_len - CLENGTH);
    	ret_code = kudisply( udcb, line, col, data, length );
    }

    /*------------------------------------------------------------------*
     * action of input field.  loop end is ####
     *------------------------------------------------------------------*/
    triger = 1;
    while(triger) {

	/*----- move coursor -------------------------------------------*/
      	CURSOR_MOVE(line , col + ncurpos);
      	fflush(stdout);

        /*-- action of key input and this key process.  end mark is $$$$*/
      	while(TRUE) {

	    dispos = ndispos;
	    curpos = ncurpos;
	    getcode = kugetc(); /* getcharacter */

	    /*----- check to getcode be triger key ---------------------*/
	    if( (getcode == U_TABKEY)   || (getcode == U_BTABKEY  ) ||
	    	(getcode == U_CRKEY )   || (getcode == U_ACTIONKEY) ||
	    	(getcode == U_ENTERKEY) || (getcode == U_RESETKEY ) ||
	    	(getcode == U_C_UP  )   || (getcode == U_C_DOWN   ) ||
	    	((U_PF1KEY <= getcode) && (getcode <= U_PF12KEY))      ) {
	  	triger = 0;
	  	ret_code = getcode;
	  	break;
	    }

	    /*----- check process key ----------------------------------*/
	    if ( getcode == U_INSERTKEY ) {
	  	/* insert key process */
	  	if ( insmode == 1 ) {
	    	    insmode = 0;	/* replace mode			*/
	  	} else {
	    	    insmode = 1;	/* insert mode			*/
	  	}
	  	continue;
	    }

	    if ( getcode == U_DELKEY ) {
	  	/* delete key process */
		rc = mbtowc(&p_wc, &data[dispos], MB_CUR_MAX);
		if ( rc < 0 ) {
		    rc = 1;
		    curmp = 1;
		} else {
		    curmp  = wcwidth( p_wc );
		}

		/* remake data area	*/
	  	if ( dispos > lastch ) continue;
	  	i = lastch - (dispos + rc) + 1;
	  	memcpy( waptr, &data[dispos+rc], i );
	  	memcpy( &waptr[i], padptr, (int)(DSIZE-i) );
	  	memcpy( &data[dispos], waptr, (int)(DSIZE-dispos) );
		/*
		 * if((mode != T_FILE) &&  (((DSIZE-i) % 2) != 0)) {
		 *   data[] = '\0';
		 * }
		 */
		dislen = lastch - dispos + 1;
	  	lastch -= rc;
	  	break;
	    }

	    if(getcode == U_C_RIGHT) {
	  	/* right cursor process       */
		rc = mbtowc( &p_wc, &data[dispos], MB_CUR_MAX );
		if ( rc < 0 ) {
			rc = 1;
			curmp = 1;
		} else {
			curmp  = wcwidth(p_wc);
		}

		if ( curpos + curmp  > data_len ) continue;
	  	ncurpos = curpos + curmp;
	  	ndispos = dispos + rc;

	  	/* move cursor */
	  	CURSOR_MOVE(line , col + ncurpos);
	  	fflush(stdout);
	  	continue;
	    }

	    if ( getcode == U_C_LEFT ) {
	  	/* get cursor move point      */
	  	k = j = i = 0;
	  	rc = curmp = 0;
	  	while(1) {
	    	    if ( k >= curpos ) {
	      	    	curmp = i;
	      	    	break;
	    	    }
		    rc = mbtowc( &p_wc, &data[j], MB_CUR_MAX );
		    if ( rc < 0 ) {
			rc = 1;
			i = 1;
		    } else {
			i  = wcwidth( p_wc );
		    }

	    	    j += rc;
	            k += i;
	  	}

	  	/* left cursor process        */
	  	if ( dispos-rc < 0 ) {
		    ncurpos = 0;
		    ndispos = 0;
	  	} else {
		    ncurpos = k - curmp;
		    ndispos = j - rc;
	  	}

	  	/* move cursor */
	  	CURSOR_MOVE(line , col + ncurpos);
	  	fflush(stdout);
	  	continue;
	    }

	    if ( getcode == U_BSPACEKEY ) {
		/* backspace key process        */
          	/* IBM-J change (lastch & curpos check routines) */
          	if ( dispos > lastch + 1 ) {
	     	    /* get cursor move point      */
	     	    k = j = i = 0;
	     	    rc = curmp = 0;
	     	    while(1) {
	       		if ( k >= curpos ) {
	         	    curmp = i;
	         	    break;
	       	   	}
			rc = mbtowc(&p_wc, &data[j], MB_CUR_MAX);
			if ( rc < 0 ) {
			    rc = 1;
			    i = 1;
			} else {
			    i = wcwidth(p_wc);
			}
	       		j += rc;
	       		k += i;
	     	    }

	     	    /* left cursor process        */
	     	    if ( dispos-rc < 0 ) {
			ncurpos = 0;
			ndispos = 0;
	     	    } else {
			ncurpos = k - curmp;
			ndispos = j - rc;
	     	    }

	     	    /* move cursor */
	     	    CURSOR_MOVE(line , col + ncurpos);
	     	    fflush(stdout);
	     	    continue;
          	}

	  	if ((curpos == 0) && (lastch >= 0)) {
		    rc = mbtowc(&p_wc, &data[dispos], MB_CUR_MAX);
		    if ( rc < 0 ) {
			rc = 1;
			curmp = 1;
		    } else {
			curmp  = wcwidth(p_wc);
		    }

		    if ( dispos > lastch ) continue;
		    i = lastch - (dispos + rc) + 1;
		    memcpy( waptr, &data[dispos+rc], i );
		    memcpy( &waptr[i], padptr, (int)(DSIZE-i) );
		    memcpy( &data[dispos], waptr, (int)(DSIZE-dispos) );
		    /*
		    if((mode != T_FILE) && 
			(((((cnvflg==U_SJIS)?data_len:DSIZE)-i) % 2) != 0)) {
			data[((cnvflg==U_SJIS)?data_len:DSIZE)-1] = '\0';
		    }
		    */
		    dislen = lastch - dispos + 1;
		    lastch -= rc;
	  	} else if ((curpos == 0) && (lastch < 0)) {
	    	    continue;
	  	} else {
	    	    k = j = i = 0;
	    	    rc = curmp = 0;
	    	    while(1) {
	       		if ( k >= curpos ) {
	         	    curmp = i;
	         	    break;
	       		}
			rc = mbtowc(&p_wc, &data[j], MB_CUR_MAX);
			if ( rc < 0 ) {
			    rc = 1;
			    i = 1;
			} else {
			    i  = wcwidth(p_wc);
			}

	       		j += rc;
	       		k += i;
	    	    }
	    	    if (dispos-rc < 0){
			ncurpos = 0;
			ndispos = 0;
	    	    } else {
			ncurpos = k - curmp;
			ndispos = j - rc;
	    	    }
	    	    i = lastch - j + 1;
	    	    memcpy( waptr, &data[j], i );
	    	    memcpy( &waptr[i], padptr, (int)(DSIZE-i) );
	    	    memcpy( &data[ndispos], waptr, (int)(DSIZE-ndispos) );
		    /*
	    	    if((mode != T_FILE) && 
			(((((cnvflg==U_SJIS)?data_len:DSIZE)-i) % 2) != 0)) {
			data[((cnvflg==U_SJIS)?data_len:DSIZE)-1] = '\0';
	    	    }
		    */
	    	    dislen = lastch - ndispos + 1;
	    	    lastch -= rc;
	    	    curpos = ncurpos;
	    	    dispos = ndispos;
	  	}
	  	break;
	    }

	    /*----------------------------------------------------------*
	     *  getcode is data
	     *----------------------------------------------------------*/
	    /*----- check 1st byte of DBCS data ------------------------*/
	    curmp = kugetcmp( getcode );
	    rc = 1;

	    /*----- check  getcode match with mode ---------------------*/
	    if ( curmp == 2 ) {
	  	/*----- check DBCS -------------------------------------*/
	  	getcode2 = kugetc();
	  	rc = 2;
	  	if ( cnvflg == U_EUC ) {
		    if ( getcode == 0x8f ) {
			getcode3 = kugetc();
			rc = 3;
		    } else if ( getcode == 0x8e ) {
			curmp = 1;
		    }
	  	}
	  	i = getcode<<8;
	  	i = i | getcode2;

	  	if ( mode == T_YOMI ) {

	    	    if( cnvflg == U_SJIS ) {

	       		if(  ((U_CHK_NL  <= i) && (i <= U_CHK_NH ))
	          		|| ((U_CHK_ALL <= i) && (i <= U_CHK_ALH))
	          		|| ((U_CHK_ASL <= i) && (i <= U_CHK_ASH))
	          		|| ((U_CHK_KL  <= i) && (i <= U_CHK_KH ))
	          		|| ((U_CHK_HL  <= i) && (i <= U_CHK_HH ))
                  		|| (i == U_2CHOUON )
	          		|| (i == U_2SPACE  ) || (i == U_2SEMCOLN)
	          		|| (i == U_2COLN   ) || (i == U_2QUES   )
	          		|| (i == U_2BIKKU  ) || (i == U_2SURA   )
	          		|| (i == U_2MLKA   ) || (i == U_2MRKA   )
	          		|| (i == U_2LKUCHI ) || (i == U_2RKUCHI )
	          		|| (i == U_2PLUS   ) || (i == U_2MAI    )
	          		|| (i == U_2IQU    ) || (i == U_2SPOCHI )
	          		|| (i == U_2DPOCHI ) || (i == U_2PER    )
	          		|| (i == U_2SHAR   ) || (i == U_2AND    )
	          		|| (i == U_2HOSHI  ) || (i == U_2TANKA  ) ) {

				;

	       		} else {

	         	    /* input code is Kanji  */
	         	    continue;
	       		}
            	    }

	    	    if ( cnvflg == U_EUC ) {

       	       		if( ((E_CHK_NL  <= i) && (i <= E_CHK_NH ))
	          	    || ((E_CHK_ALL <= i) && (i <= E_CHK_ALH))
	          	    || ((E_CHK_ASL <= i) && (i <= E_CHK_ASH))
	          	    || ((E_CHK_KL  <= i) && (i <= E_CHK_KH ))
	          	    || ((E_CHK_HL  <= i) && (i <= E_CHK_HH ))
                  	    || (i == E_2CHOUON )
	          	    || (i == E_2SPACE  ) || (i == E_2SEMCOLN)
	          	    || (i == E_2COLN   ) || (i == E_2QUES   )
	          	    || (i == E_2BIKKU  ) || (i == E_2SURA   )
	          	    || (i == E_2MLKA   ) || (i == E_2MRKA   )
	          	    || (i == E_2LKUCHI ) || (i == E_2RKUCHI )
	          	    || (i == E_2PLUS   ) || (i == E_2MAI    )
	          	    || (i == E_2IQU    ) || (i == E_2SPOCHI )
	          	    || (i == E_2DPOCHI ) || (i == E_2PER    )
	          	    || (i == E_2SHAR   ) || (i == E_2AND    )
	          	    || (i == E_2HOSHI  ) || (i == E_2TANKA  ) ) {

			    ;

	       		} else {
	         	    /* input code is Kanji or invalid */
                            printf("%c",0x07);    /* bell */
                            fflush(stdout);
	         	    continue;
               		}
             	    }
	  	}
          	else if ( mode == T_GOKU ) {

		    if ( cnvflg == U_SJIS ) {
			if ( i == U_2DOLLAR   ) {
                            printf("%c",0x07);    /* bell */
                            fflush(stdout);
			    continue;
			}
		    } else if ( cnvflg == U_EUC ) {
			if( i == E_2DOLLAR   ) {
                            printf("%c",0x07);    /* bell */
                            fflush(stdout);
			    continue;
			} else if( getcode == 0x8e ) {
                            printf("%c",0x07);    /* bell */
                            fflush(stdout);
			    continue;	/* single width katakana */
			}
		    }
	  	}

	    } else {

	  	/* check SBCS */
	  	if ( cnvflg == U_SJIS ) {
		    if( ((getcode < U_1START)  || (U_1END  < getcode))
			&& ((getcode < U_1STARTK) || (U_1ENDK < getcode)) ) {
			/* beep , because input invarid key */
			printf("%c",0x07);    /* bell */
			fflush(stdout);
			continue;
		    }
	  	} else if ( cnvflg == U_EUC ) {
		    if((getcode < U_1START)  || (U_1END  < getcode)) {
			/* beep , because input invarid key */
			printf("%c",0x07);    /* bell */
			fflush(stdout);
			continue;
		    }
	  	}
	  	if ( mode != T_FILE ) {
	    	    continue;
	  	}
	    }

	    /*----- getcode fit for mode -------------------------------*/
	    /*----- check insert mode ----------------------------------*/
	    if ( insmode == 1 ) {      /* end if mark @@@@     */
	  	/* set lastch */
	  	while( lastch>0 ) {

	     	    if ( cnvflg == U_SJIS ) {
		 	if( (*(data+lastch-1) == U_2SPACEH) &&    /* sjis 4 */
	             	    (*(data+lastch) == U_2SPACEL)     ) {
		     	    /* DBCS blank code    */
	    	     	    lastch = lastch - 2;
	     	     	    continue;
	  	 	} else if (*(data + lastch) == U_1SPACE ) {
	     	     	    /* SBCS blank code    */
	    	     	    lastch = lastch - 1;
	     	     	    continue;
	   	 	}
             	    }

	     	    if ( cnvflg == U_EUC ) {
		 	if ( (*(data+lastch-1) == E_2SPACEH) &&    /* euc 4 */
	             	    (*(data+lastch) == E_2SPACEL)     ) {
		    	    /* DBCS blank code    */
	    	    	    lastch = lastch - 2;
	     	    	    continue;
	  	 	} else if(*(data + lastch) == U_1SPACE) {
	     	 	    /* SBCS blank code    */
	    	    	    lastch = lastch - 1;
	     	    	    continue;
	   	 	}
             	    }
             	    break;
	  	}

	  	if (lastch <= 0) lastch = -1;

	  	/*-- padding space code from lastch+1 to data_len --*/
	  	i = lastch + 1;
	  	if ( i < DSIZE ) {
	    	    i = DSIZE - i;
	    	    memcpy( &data[lastch+1], padptr, i );
		    /*
	    	    if((mode != T_FILE) && ((i % 2) != 0)) 
			data[((cnvflg==U_SJIS)?data_len:DSIZE)-1] = '\0';
		     */
	  	}

	      	if (lastch < dispos ) {
	    	    /* data set check   */
	    	    if ( data_len < (curpos + curmp) ) {
	      	 	/* beep , because there are not enough space      */
			printf("%c",0x07);
	      		fflush(stdout);
	      		continue;
	    	    } else {
	      	    	lastch = dispos + rc;
	      	    	/* set display length */
	      	    	dislen = rc;
	    	    }
	  	} else {
	    	    /* check space for key insert */
	    	    if( data_len < (CLENGTH + curmp) ) {
	      		/* beep , because there are not enough space      */
			printf("%c",0x07);
	      		fflush(stdout);
	      		continue;
	    	    }
	    	    /* move data to backword  */
	    	    i = lastch - dispos + 1;
	    	    memcpy( waptr, &data[dispos], i );
	    	    memcpy( &waptr[i], padptr, (int)(DSIZE-i) );
	    	    memcpy( &data[dispos+rc], waptr, (int)(DSIZE-(dispos+rc)));
		    /*
	    	    if((mode != T_FILE) && 
			(((((cnvflg==U_SJIS)?data_len:DSIZE)-i) % 2) != 0)){
			data[((cnvflg==U_SJIS)?data_len:DSIZE)-1] = '\0';
	    	    }
		    */
	    	    lastch += rc;
	    	    /* set display length */
	    	    dislen = lastch - dispos + 1;
	  	}

	    } else {
	  	/* check replaced data        */
	  	if ( data_len < (curpos + curmp) ) {
	    	    /* beep , because there are not enough space        */
		    printf("%c",0x07);
	     	    fflush(stdout);
	    	    continue;
	  	}

	  	/* get data type at curpos    */
	  	j = mbtowc(&p_wc, &data[dispos], MB_CUR_MAX);
	  	if(j < 0){
		    j = 1;
		    k = 1;
	  	} else {
		    k = wcwidth(p_wc);
	  	}
	  	if (k < curmp) {
	  	    i = mbtowc(&p_wc, &data[dispos+j], MB_CUR_MAX);
		    if ( i < 0 ) {
			j += 1;
			k += 1;
		    } else {
			j += i;
			k += wcwidth(p_wc);
		    }
	  	}

	  	/* move data */
	  	if (field_len < (CLENGTH+curmp-k) ) {
		    printf("%c",0x07);
		    fflush(stdout);
		    continue;
	  	}
	  	if ( j > 0 ) {
		    if(rc != j){
			if(lastch >= (dispos + j)) {
			    i = lastch - (dispos + j) + 1;
			    memcpy( waptr, &data[dispos+j], i );
			    memcpy( &waptr[i], padptr,  (int)(DSIZE-i) );
			    memcpy( &data[dispos+rc], waptr,
				  (int)(DSIZE-(dispos+rc)) );
			    /*
			    if((mode != T_FILE) && 
				  (((((cnvflg==U_SJIS)?data_len:DSIZE)-i) 
				  % 2) != 0)){
			    	data[((cnvflg==U_SJIS)?data_len:DSIZE)-1] 
					= '\0';
			    }
			    */
			} else {
			    i = dispos + rc;
			    memcpy(&data[i],padptr,(int)(DSIZE-i));
			    /*
			    if((mode != T_FILE) && (((data_len-i) % 2) != 0)){
					data[field_len] = '\0';
			    }
			    */
			}
		    }
		    /* set lastch */
		    if((dispos+rc) > lastch) {
			lastch = dispos + rc - 1;
			dislen = lastch - dispos + 1;
		    }else{
			dislen = lastch - dispos + 1;
			lastch += (rc - j);
		    }
	  	} else {
		    dislen = rc;
	  	}
	    } /* end if of @@@@    */

	    /* set getcode to data  */
	    data[dispos] = getcode;
	    if(rc == 2) {
	  	data[dispos + 1] = getcode2;
	    } else if(rc == 3) {
	  	data[dispos + 1] = getcode2;
	  	data[dispos + 2] = getcode3;
	    }

	    /* move cursor point  */
	    ncurpos = curpos + curmp;
	    ndispos = dispos + rc;

	    break;
      	} /* end loop $$$$       */

      	if(triger != 0) {
	    /* diplay data    */
	    ret_code = kudisply(udcb,line,curpos+col,&(data[dispos]),dislen);

        }

    } /* end loop #### */

    /* set input data length    */
    while(lastch>-1) {

          if( cnvflg == U_SJIS ){
		if( (lastch >  0    )               &&
		  (*(data+lastch-1) == U_2SPACEH) &&   /* sjis 5 */
	  	  (*(data+lastch) == U_2SPACEL)            ) {
	  	/* DBCS blank code    */
	  	lastch = lastch - 2;
	  	continue;
		} else if(*(data + lastch) == 0x20) {
	  	/* SBCS blank code    */
	  	lastch = lastch - 1;
	  	continue;
		};
          };

          if( cnvflg == U_EUC ){
		if( (lastch >  0    )             &&
		  (*(data+lastch-1) == E_2SPACEH) &&       /* euc 5 */
	  	  (*(data+lastch) == E_2SPACEL)            ) {
	  	/* DBCS blank code    */
	  	lastch = lastch - 2;
	  	continue;
		} else if(*(data + lastch) == 0x20) {
	  	/* SBCS blank code    */
	  	lastch = lastch - 1;
	  	continue;
		};
          };
          break;
    };

    /* data conversion */
    if ( convert == U_SJIS ) { 
	*iplen = lastch + 1;
	memcpy( datai, data, *iplen );
	datai[*iplen] = '\0';
    } else {
	ilen = lastch + 1;
	if(ilen > 0){
	    olen = data_len;
	    kjcnvets(cnvdata, &ilen, datai, &olen);
	    *iplen = data_len-olen;
	    datai[*iplen] = '\0';
	} else {
	    *iplen = 0;
	    datai[0] = '\0';
	}
    }

    return( ret_code );
}
