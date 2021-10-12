static char sccsid[] = "@(#)21	1.1  src/bos/usr/lpp/kls/dictutil/huipfld.c, cmdkr, bos411, 9428A410j 5/25/92 14:46:36";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		huipfld.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       huipfld.c
 *
 *  Description:  user dictionary string data input.
 *
 *  Functions:    huipfld()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include "hut.h"
#include <stdio.h>
#include <memory.h>


#define	MSG_lin		( 14 )
#define	MSG_col		(  3 )
#define HUDICUPH	( 1 )

int huipfld(udcb,line,col,data,data_len,mode,iplen,initd, where)
UDCB    *udcb;                  /* pointer to UDCB      */
short   line;                   /* input field start line       */
short   col;                    /* input field start column     */
char    *data;                  /* data input area      */
short   data_len;               /* length of data area  */
short   mode;                   /* mode of input field  */
short   *iplen;                 /* length of input data */
short   initd;                  /* initial data display flag    */
short   where;
{

    int         hugetc();       /* get vharacter        */
    int         hudisply();     /* display data         */
    int         hugetcmp();     /* get cursor move point        */

    char        *waptr;         /* pointer to work area */
    char        *padptr;        /* pointer to padding area      */
    short       curpos;         /* cursor position      */
    short       lastch;         /* position of last character   */
    short       curmp;          /* cursor move point    */
    short       triger;         /* triger key flag      */
    short       invalid;        /* invalid key flag     */
    short       insmode;        /* insert mode flag     */
    short       field_len;      /* input filed length (0 -- data_len-1) */
    short       dispos;         /* display start position       */
    short       dislen;         /* display length       */
    short	merflg;		/* message erase flag   */
    short	msgx;		/* message area x axis  */
    short	msgy;		/* message area y axis  */
    int         j;            /* work varible */
    ushort      i;            /* work varible */
    int         ret_code;       /* ret_code     */
    int         getcode;        /* getcharacter code    */
    int         getcode2;       /* getcharacter code    */


    /* check of data    */
    if((data_len < 1) || (data == NULL)) {
      return( -1 );
    }

    /* check of display position        */
    if( (line < 0) || (udcb->ymax < line) ||
	(col < 0)  || (U_MAXCOL + udcb->xoff< col)         ) {
      return( -2 );
    };

    /* check of mode    */
    if( (mode != T_CAND) && (mode != T_KEY) && (mode != T_FILE) ) {
      return( -3 );
    };

    /* set cursor move point    */
    curmp = ((mode == T_FILE) ? 1:2);

    /* set data length  */
    data_len = MIN( data_len , (U_MAXCOL + udcb->xoff - col) );
    /* data_len = data_len - (data_len % 2);    */
    field_len = data_len - 1;

    /* allocate of work area    */
    waptr = (char *)malloc( (unsigned)(data_len) );
    if(waptr == NULL) {
      return( -4 );
    };

    /* allocate and set of padding area */
    padptr = (char *)malloc( (unsigned)(data_len) );
    i = 0;
    if(mode == T_FILE) {
      while(i<data_len) {
	*(padptr + i)     = U_1SPACE;
	i++;
      };
    } else {
      while(i<data_len) {
	*(padptr + i)     = U_2SPACEH;
	*(padptr + i + 1) = U_2SPACEL;
	i = i + 2;
      };
    };

    /* search NULL code from top of data */
    for(i=0;i<data_len;i++) {
      if(*(data + i) == NULL) {
	if(mode == T_FILE) {
	  for(j=i;j<data_len;j++) {
	    *(data + j) = U_1SPACE;
	  };
	} else {
	  for(j=i;j<data_len;j=j+2) {
	    *(data + j)     = U_2SPACEH;
	    *(data + j + 1) = U_2SPACEL;
	  };
	};
	break;
      };
    };
    lastch = i - 1;


    /* find valid code from last of data        */
    while(lastch>-1) {
      if( (lastch > 0)                    &&
	  (*(data+lastch-1) == U_2SPACEH) &&
	  (*(data+lastch) == U_2SPACEL)            ) {
	/* DBCS blank code    */
	lastch = lastch - 2;
	continue;
      } else if(*(data + lastch) == U_1SPACE) {
	/* SBCS blank code    */
	lastch = lastch - 1;
	continue;
      };
      break;
    };
    if(lastch < 0) lastch = -1;
    insmode = 0;
    curpos = 0;
    dispos = col;
    dislen = data_len;

    /* initial data display     */
    if(initd == C_SWON) {
      ret_code = hudisply(udcb,line,col,data,data_len);
    };

    merflg = C_SWOFF;
    msgx = MSG_col + udcb->xoff;
    if (where == HUDICUPH)
    	msgy = udcb->ymax - 4;
    else
    	msgy = MSG_lin + udcb->yoff;
    /* action of input field.  loop end is #### */
    triger = 1;
    while(triger) {

      /* move coursor   */
      CURSOR_MOVE(line , col + curpos);
      fflush(stdout);

      /* action of key input and this key process.  end mark is $$$$    */
      while(TRUE) {

/*???MOVE */
      CURSOR_MOVE(line , col + curpos);
      fflush(stdout);
	/* getcharacter */
	getcode = hugetc();

	/* check to getcode be triger key       */
	if( (getcode == U_TABKEY)   || (getcode == U_BTABKEY  ) ||
	    (getcode == U_CRKEY )   || (getcode == U_ACTIONKEY) ||
	    (getcode == U_ENTERKEY) || (getcode == U_RESETKEY ) ||
	    (getcode == U_C_UP  )   || (getcode == U_C_DOWN   ) ||
	    ((U_PF1KEY <= getcode) && (getcode <= U_PF12KEY))      ) {

	  if (merflg == C_SWON) {
           	humsg(udcb,msgy,msgx,(short)(U_ENDID));
		merflg = C_SWOFF;
	  }
	  triger = 0;
	  ret_code = getcode;
	  break;
	};

	/* check process key    */
	if(getcode == U_INSERTKEY) {
	  /* insert key process */
	  if(insmode == 1) {
	    insmode = 0;
	  } else {
	    insmode = 1;
	  };
	  continue;
	};

	if(getcode == U_DELKEY) {
	  /* get cursor mp      */
	  curmp = (ushort)hugetcmp( (int)(data[curpos]) );
	  /* delete key process */
	  if(curpos > lastch) continue;
	  memcpy(waptr,padptr,data_len);
	  memcpy(waptr,&data[curpos+curmp],(int)(lastch-(curpos+curmp-1)));
	  memcpy(&data[curpos],waptr,(int)(data_len-curpos));
	  dispos = curpos;
	  dislen = lastch - curpos + 1;
	  lastch = lastch - curmp;
	  if (merflg == C_SWON) {
           	humsg(udcb,msgy,msgx,(short)(U_ENDID));
		merflg = C_SWOFF;
	  }
	  break;
	};

	if(getcode == U_C_RIGHT) {
	  /* get cursor mp      */
	  curmp = (ushort)hugetcmp( (int)(data[curpos]) );
	  /* right cursor process       */
	  if(curpos+curmp > field_len) continue;
	  curpos = curpos + curmp;
	  /* move cursor */
	  CURSOR_MOVE(line , col + curpos);
	  fflush(stdout);
	  continue;
	};

	if(getcode == U_C_LEFT) {
	  /* get cursor move point      */
	  i = j = 0;
	  while(1) {
	    if(j == curpos) {
	      curmp = i;
	      break;
	    };
	    i = hugetcmp( (int)(data[j]) );
	    j = j + i;
	  };
	  /* left cursor process        */
	  if(curpos-curmp < 0) continue;
	  curpos = curpos - curmp;
	  /* move cursor */
	  CURSOR_MOVE(line , col + curpos);
	  fflush(stdout);
	  continue;
	};

	/* backspace key process        */
	if(getcode == U_BSPACEKEY) {

	  if (merflg == C_SWON) {
           	humsg(udcb,msgy,msgx,(short)(U_ENDID));
		merflg = C_SWOFF;
	  }

          /* IBM-J change (lastch & curpos check routines) */
          if(curpos > lastch + 1) {
	     /* get cursor move point      */
	     i = j = 0;
	     while(1) {
	       if(j == curpos) {
	         curmp = i;
	         break;
	       };
	       i = hugetcmp( (int)(data[j]) );
	       j = j + i;
	     };
	     /* left cursor process        */
	     if(curpos-curmp < 0) continue;
	     curpos = curpos - curmp;
	     /* move cursor */
	     CURSOR_MOVE(line , col + curpos);
	     fflush(stdout);
	     continue;
          }

	  if((curpos == 0) && (lastch >= 0)) {
	    /* get cursor mp    */
	    curmp = (ushort)hugetcmp( (int)(data[curpos]) );
	    memcpy(waptr,padptr,data_len);
	    memcpy(waptr,&data[curpos+curmp]
			,(int)(lastch-(curpos+curmp-1)));
	    memcpy(&data[curpos],waptr,(int)(data_len-curpos));
	    dislen = lastch + 1;
	    lastch = lastch - curmp;
	  } else if((curpos == 0) && (lastch < 0)) {
	    continue;
	  } else {
	    i = j = 0;
	    while(1) {

	      if(j == curpos) {
		curmp = i;
		break;
	      };
	      i = hugetcmp( (int)(data[j]) );
	      j = j + i;
	    };
	    memcpy(waptr,padptr,data_len);
	    memcpy(waptr,&data[curpos],(int)(data_len-curpos));
	    memcpy(&data[curpos-curmp],waptr,(int)(data_len-curpos+curmp));
	    curpos = curpos - curmp;
	    dislen = lastch - curpos + 1;
	    if(curpos <= lastch) {
	      lastch = lastch - curmp;
	    };
	  };
	  dispos = curpos;
	  break;
	};

	/* getcode is input data        */
	/* check of data        */
	curmp = hugetcmp( getcode );

	/* check  getcode match with mode       */
	if(curmp == 2) {
	  /* check DBCS         */
	  getcode2 = hugetc();
	  i = (ushort)getcode<<8;
	  i = i | (ushort)getcode2;
	  if(mode == T_KEY) {
	    if( ((U_CHK_NL <= i) && (i <= U_CHK_NH))	/* numeric code */
	       || ((U_CHK_HL <= i) && (i <= U_CHK_HH)) ) /* hangeul code */
	    {
	      /* valid key */
	    } else {
	      /* invalid key */
		humsg(udcb, msgy, msgx, U_ACMSGN);
		putp(bell);
		merflg = C_SWON;
		continue;
	    };
	  }
          else if(mode == T_CAND) {

	      /* valid key */

	  };
	} else {
	  /* check SBCS */
	  if(    ( (getcode < U_1START)  || (U_1END  < getcode))
	      && ( (getcode < U_1STARTK) || (U_1ENDK < getcode)) ) {
	    /* beep , because input invalid key */
	    putp(bell);
	    fflush(stdout);
	    continue;
	  };
	  if(mode != T_FILE) {
	    continue;
	  };
	};
	/* getcode fit for mode         */

	/* check insert mode    */
	if(insmode == 1) {      /* end if mark == \\\\  */
	  /* set lastch */
	  while(lastch>0) {
	    if( (*(data+lastch-1) == U_2SPACEH) &&
		(*(data+lastch) == U_2SPACEL)            ) {
	      /* DBCS blank code    */
	      lastch = lastch - 2;
	      continue;
	    } else if(*(data + lastch) == U_1SPACE) {
	      /* SBCS blank code    */
	      lastch = lastch - 1;
	      continue;
	    };
	    break;
	  };
	  if(lastch <= 0) lastch = -1;

	  /* padding space code from lastch+1 to data_len       */
	  i = lastch + 1;
	  if(i<data_len) {
	    memcpy(&data[i],padptr,(int)(data_len-lastch-1));
	  };
	  if(lastch < curpos) {
	    /* data set check   */
	    if( curpos + curmp > field_len + 1 ) {
	      /* beep , because there are not enough space      */
		if (mode == T_KEY) {
			humsg(udcb, msgy, msgx, U_AJMSGN);
			merflg = C_SWON;
		}
		else if (mode == T_CAND) {
			humsg(udcb, msgy, msgx, U_BHMSGN);
			merflg = C_SWON;
		}
	      putp(bell);
	      fflush(stdout);
	      continue;
	    } else {
	      /* set display length */
	      dislen = curmp;
	    };
	  } else {
	    /* check space for key insert */
	    if( lastch + curmp > field_len ) {
	      /* beep , because there are not enough space      */
		if (mode == T_KEY) {
			humsg(udcb, msgy, msgx, U_AJMSGN);
			merflg = C_SWON;
		}
		else if (mode == T_CAND) {
			humsg(udcb, msgy, msgx, U_BHMSGN);
			merflg = C_SWON;
		}
	      putp(bell);
	      fflush(stdout);
	      continue;
	    };
	    /* move data to backword  */
	    memcpy(waptr,padptr,data_len);
	    memcpy(waptr,&data[curpos],(int)(lastch-curpos+1));
	    memcpy(&data[curpos+curmp],waptr,(int)(lastch-curpos+1));
	    lastch = lastch + curmp;
	    /* set display length */
	    dislen = lastch - curpos + 1;
	  };

	} else {
	  /* check replaced data        */
	  if( curpos + curmp > field_len + 1 )  {
	    /* beep , because there are not enough space        */
		if (mode == T_KEY) {
			humsg(udcb, msgy, msgx, U_AJMSGN);
			merflg = C_SWON;
		}
		else if (mode == T_CAND) {
			humsg(udcb, msgy, msgx, U_BHMSGN);
			merflg = C_SWON;
		}
	    putp(bell);
	    fflush(stdout);
	    continue;
	  };
	  /* get data type at curpos    */
	  dislen = curmp;
	  if( (i == 2) && (curmp == 1) ) {
	      /* case set SBCS to DBCS    */
	      /* set space padding        */
	      data[curpos+1] = U_1SPACE;
	      dislen ++;
	  } else if( (i == 1) && (curmp == 2) ) {
	      /* case set DBCS to SBCS  */
	      /* get data type at curpos+1      */
	      j = hugetcmp( data[curpos+1]  );
	      if(j == 2) {
		/* set space padding    */
		data[curpos+2] = U_1SPACE;
		dislen ++;
	      };
	  };
	}; /* end if of \\\\    */

	/* set getcode to data  */
	data[curpos] = getcode;
	if(curmp == 2) {
	  data[curpos + 1] = getcode2;
	};
	/* set lastch */
	if(curpos > lastch) {
	  lastch = curpos + curmp - 1;
	};

	/* set display area     */
	dispos = curpos;

	/* move cursor point  */
/*???   field_len -> field_len + 1 ***/
	if(curpos + curmp <= field_len + 1) {
	  curpos = curpos + curmp;
	}
	break;
      }; /* end loop $$$$       */
      if(triger != 0) {
	/* diplay data    */
	ret_code = hudisply(udcb,line,dispos+col,&(data[dispos]),dislen);
      };

    }; /* end loop #### */

    /* set input data length    */
    while(lastch>-1) {
	if( (lastch >  0    )               &&
	    (*(data+lastch-1) == U_2SPACEH) &&
	    (*(data+lastch) == U_2SPACEL)            ) {
	  /* DBCS blank code    */
	  lastch = lastch - 2;
	  continue;
	} else if(*(data + lastch) == U_1SPACE) {
	  /* SBCS blank code    */
	  lastch = lastch - 1;
	  continue;
	};
      break;
    };

    *iplen = lastch + 1;

    return( ret_code );
};



