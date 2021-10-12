static char sccsid[] = "@(#)35	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicupm.c, cmdKJI, bos411, 9428A410j 7/23/92 01:24:25";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicupm, kukakko
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

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kudicupm
 *
 * DESCRIPTIVE NAME:    User Dictionary Buffer Display
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            1. Display user dictionary
 *                      2. Display next page
 *                      3. Display before page
 *                      4. Redrw input data at one of input field
 *                      5. Redrw original data at one of input field
 *                      6. Redrw input data at one of input field to be revers
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        4494 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicupm
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicupm (udcb,df,dmode,dmappt,fldno
 *                               ,dbufpt,topptr,lastptr,erstr,erl)
 *
 *  INPUT:              *udcb   : pointer to UDCB
 *                      *df     : pointer to display field data
 *                      dmode   : display mode
 *                      *dmappt : pointer to display map
 *                      fldno   : field No. to be reversed
 *                      *dbufpt : pointer to update buffer
 *                      *topptr : pointer to top of update buffer
 *                      *lastptr: pointer to last of update buffer
 *                      hajime  : start Y axis of input field
 *                      gyosu   : maximum line number of input field
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IUSUCC  : sucess return
 *                      IUFAIL  : fail no process
 *
 * EXIT-ERROR:          UDNODTE : uer dictionary no data error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudisply: Display stringth.
 *                      Standard Liblary.
 *                              memcpy  : Copy character string.
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

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory package.                              */

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*
 *      include Kanji Project.
 */
#include "kje.h"        /* Kanji Utility Define File.                   */
#include "kut.h"                        /* Kanji Utility Define File.   */

extern  int     cnvflg;         /* Conversion Type Code                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int     kudicupm ( udcb   , df     , dmode   , dmappt , fldno
                 , dbufpt , topptr , lastptr , hajime , gyosu   )

UDCB    *udcb;          /* pointer to UDCB                              */
DIFLD   *df;            /* pointer to display field data area           */
short   dmode;          /* display mode                                 */
UDMS    *dmappt;        /* pointer to display map                       */
short   fldno;          /* field No. to be reversed                     */
UDCS    *dbufpt;        /* pointer to update buffer                     */
UDCS    *topptr;        /* pointer to top of update buffer              */
UDCS    *lastptr;       /* pointer to last of update buffer             */
short   hajime;         /* start Y axis of input field                  */
short   gyosu;          /* maximum line number of input field           */

{
	int     kudisply();     /* display stringth.                    */
	int     kukakko();      /* write bracket.                       */

	UDCS    *pthdd;         /* pointer to head of display data      */
        UDCS    *pthdd1;        /* pointer to head of display data      */
        UDCS    *rtopptr;       /* real pointer to top of update buffer */
        uchar   *wyomi;         /* pointer to yomi data                 */
        uchar   wyomilen;       /* pointer to yomi len                  */

        char    *data;          /* pointer to input data field          */

        int     rc;             /* return code                          */
        int     i;              /* work variable                        */
        int     j;              /* work variable                        */
        short   maxfld;         /* maximum field nimber                 */

	short   col;            /* input field x axis                   */
	short   line;           /* input field y axis                   */
	short   dtlen;          /* length of input field                */

        size_t  pclen,euclen;
	char	pcstr[100];
	char	eucstr[100];

	static char ydata[2][5];

        strcpy(ydata[0], CU_MNYAG1);
        strcpy(ydata[1], CU_MNYAG2);


                                        /* allow data.                  */

	/*  0.
         *      check !  input parameter.
         */
        if(    (dmode != U_NEXTP) && (dmode != U_BEFORP)
            && (dmode != U_RESET) && (dmode != U_REDRW )
	    && (dmode != U_DISP ) && (dmode != U_REVER )   )
          return( IUFAIL );

        /*
	 *      case(dmode == U_RESET )  reset            process
	 *      case(dmode == U_REDRW )  redrw            process
	 *      case(dmode == U_REVER )  revers & redrwb  process
	 *      case(dmode == U_NEXTP )  next page        process
	 *      case(dmode == U_BEFORP)  before page      process
	 *      case(dmode == U_DISP  )  dsplay           process
         */

	if( (dmode == U_REDRW) || (dmode == U_REVER) ) {
            /*  1.
             *  redrw process.
             */

            /*
             *  field redrw and display
             */
            /* check !  redrow field.                                   */
	    if( dmappt->fld[fldno].fstat == U_YOMIF )
                {
                    /* case : redorw field is Yomi.                     */
                    /* set displa X_axis.                               */
		    col    = U_XYOMI;
                    /* set data length.                                 */
                    dtlen  = U_YOMLEN;
                    /* set pointer to input field.                      */
		    data   = df[fldno].yofld;
                }
            else
                {
                    /* case : redorw field is Goki.                     */
                    /* set displa X_axis.                               */
		    col      = U_XGOKU;
                    /* set data length.                                 */
                    dtlen  = U_GOKLEN;
                    /* set pointer to input field.                      */
		    data   = df[fldno].gofld;
                }

            /* set redrw data display field axis.                       */
	    col = col + udcb->xoff;
	    line = hajime + fldno;

	    /* set bracket      */
	    rc = kukakko(line,col,dtlen);

	    /* field redrow     */
	    if(dmode == U_REVER) {
	      /* set revers mode        */

/* B.EXTCUR */
		tputs(enter_reverse_mode,1,putchar);
/* E.EXTCUR */

	    };
            if ( cnvflg == U_EUC ) {
                euclen = 100;
		pclen = dtlen;
                rc = kjcnvste(data, &pclen, eucstr, &euclen);
                if( rc < 0 )
                        return( IUFAIL );
		eucstr[100-euclen]='\0';
                rc = kudisply2(udcb,line,col,eucstr,dtlen);
            } else {
                rc = kudisply(udcb,line,col,data,dtlen);
            }
	    if(dmode == U_REVER) {
	      /* reset revers mode      */

/* B.EXTCUR */
		tputs(exit_attribute_mode,1,putchar);
/* E.EXTCUR */

	    };
	    fflush(stdout);


            return( IUSUCC );
        };


        if( dmode == U_RESET )  {
            /*  2.
             *  reset process.
             */
            /* get pointer to UDCS of reset field.                      */
	    pthdd = dmappt->fld[fldno].dbufpt;

	    if( dmappt->fld[fldno].fstat == U_YOMIF )  {
	      /* case : reset field is Yomi.                      */
	      /* set display X_axis.                              */
	      col      = U_XYOMI;
	      /* set data length.                                 */
	      dtlen  = U_YOMLEN;
	      /* set pointer to input field.                      */
	      data   = df[fldno].yofld;
	      /* set init data stringth.                          */
	      (void)memcpy(data,pthdd->yomi,(int)(pthdd->yomilen));
	      data[pthdd->yomilen] = NULL;
	    } else {
	      /* case : reset field is Goki.                      */
	      /* set display X_axis.                              */
	      col      = U_XGOKU;
	      /* set display data length.                         */
	      dtlen  = U_GOKLEN;
	      /* set pointer to input field.                      */
	      data   = df[fldno].gofld;
	      /* set init strungth.                               */
	      (void)memcpy(data,pthdd->kan,(int)(pthdd->kanlen));
	      data[pthdd->kanlen] = NULL;
	    }

	    /* set reset field display axis.    */
	    col = col + udcb->xoff;
	    line = hajime + fldno;

	    /* set bracket      */
	    rc = kukakko(line,col,dtlen);

	    /* field redrow     */
            if ( cnvflg == U_EUC ) {
                euclen = 100;
		pclen = dtlen;
                rc = kjcnvste(data, &pclen, eucstr, &euclen);
                if( rc < 0 )
                        return( IUFAIL );
		eucstr[100-euclen]='\0';
                rc = kudisply2(udcb,line,col,eucstr,dtlen);
            } else {
                rc = kudisply(udcb,line,col,data,dtlen);
            }
	    fflush(stdout);

            return( IUSUCC );
        };

        /* erase display area process.                                  */
	CURSOR_MOVE(hajime,0);
	for(i=0;i<gyosu;i++) {

/* B.EXTCUR */
		tputs(clr_eol,1,putchar);
		tputs(cursor_down,1,putchar);
/* E.EXTCUR */

	};

        /* set pointer to UDCS and check.                               */
        i = C_SWOFF;
        pthdd = dbufpt;
	/* fprintf(stderr,"parameter pthdd = %ld\n",pthdd);     */
        while(TRUE) {
          if(    (pthdd->status == U_S_INIT)
              || (pthdd->status == U_S_YOMA)
              || (pthdd->status == U_S_KNJU)    )  {
            i = C_SWON;
            break;
          };
          if( pthdd->pr_pos == NULL )  break;
          pthdd = pthdd->pr_pos;
        };
        if( i == C_SWOFF )  {
          pthdd = dbufpt;
          while(TRUE) {
            if(    (pthdd->status == U_S_INIT)
                || (pthdd->status == U_S_YOMA)
                || (pthdd->status == U_S_KNJU)    )  break;
            /* case : data nothing.                                     */
            if( pthdd->nx_pos == NULL )  return( UDNODTE );
            pthdd = pthdd->nx_pos;
          };
        };

        /* get real pointer to top of UDCS.                             */
        rtopptr = topptr;
        while(TRUE) {
          if(    (rtopptr->status == U_S_INIT)
              || (rtopptr->status == U_S_YOMA)
              || (rtopptr->status == U_S_KNJU)    )  break;
          rtopptr = rtopptr->nx_pos;
        };

        /* set maxfld.                                                  */
	maxfld = gyosu - 1;


        if( dmode == U_NEXTP ) {
            /*  3.
             *  next page process
             */
            /*
             *  search last field
             */
	    for(i=maxfld;dmappt->fld[i].fstat==U_NODIS;i--);
	    if(i <= 2)  {
              /* case : last to top.                                    */
              pthdd = rtopptr;
            }
            else  {
              /* case : next page.                                      */
              pthdd = dmappt->fld[i].dbufpt;
            };


        };

        if( dmode == U_BEFORP ) {
            /*  4.
             *  before page process
             */
            if(dmappt->fld[0].fstat == U_NODIS)  {
              /* case : top to last.                                    */
              pthdd = lastptr;
	    } else if(dmappt->fld[0].dbufpt == rtopptr)  {
              /* case : top to bottom line.                             */
              /* all display map to be no use.                          */
	      for(i=0;i<=maxfld;i++)
                    dmappt->fld[i].fstat = U_NODIS;
              /*
               *      display yomi data and set display map
               */
	      /* set befor of bottom line no.   */
	      fldno    = gyosu - 2;

              /* set bottom line field.                                 */
	      col      = U_XYOMI + udcb->xoff;
	      line     = hajime + fldno;

              /* set data,length,field No.                              */
              dtlen  = U_YOMLEN;
	      data   = df[fldno].yofld;

              /* set display map.                                       */
              /* set pointer to UDCS.                                   */
	      dmappt->fld[fldno].dbufpt = rtopptr;
              /* set status code.                                       */
	      dmappt->fld[fldno].fstat  = U_YOMIF;

              /* display Yomi field.                                    */
              /* set init stringth.                                     */
	      (void)memcpy(data,rtopptr->yomi,(int)(rtopptr->yomilen));
	      data[pthdd->yomilen] = NULL;
	      rc = kukakko(line,col,dtlen);
              if ( cnvflg == U_EUC ) {
                        euclen = 100;
			pclen = dtlen;
                        rc = kjcnvste(data, &pclen, eucstr, &euclen);
                        if( rc < 0 )
                                return( IUFAIL );
                        rc = kudisply(udcb,line,col,eucstr,dtlen);
              } else {
                        rc = kudisply(udcb,line,col,data,dtlen);
              }

              /*
               *      display goki data and set display map
               */
	      /* set fldno      */
	      fldno++ ;
              /* set Goki field X_axis.                                 */
	      col    = U_XGOKU + udcb->xoff;
	      line++;

              /* set data,length,field No.                              */
              dtlen  = U_GOKLEN;
	      data   = df[fldno].gofld;

              /* set display map.                                       */
              /* set pointer to UDCS.                                   */
	      dmappt->fld[fldno].dbufpt = rtopptr;
              /* set status code.                                       */
	      dmappt->fld[fldno].fstat  = U_GOKUF;

              /* display Goki data.                                     */
              /* set init stringth.                                     */
	      (void)memcpy(data,rtopptr->kan,(int)(rtopptr->kanlen));
	      data[pthdd->kanlen] = NULL;
              /* set conversion flag.                                   */
	      rc = kukakko(line,col,dtlen);
              if ( cnvflg == U_EUC ) {
                        euclen = 100;
			pclen = dtlen;
                        rc = kjcnvste(data, &pclen, eucstr, &euclen);
                        if( rc < 0 )
                                return( IUFAIL );
                        rc = kudisply(udcb,line,col,eucstr,dtlen);
              } else {
                        rc = kudisply(udcb,line,col,data,dtlen);
              }

              /* write allow sighin.                                    */
              /* set current yomi data.                                 */
              wyomi    = rtopptr->yomi;
              wyomilen = rtopptr->yomilen;
              pthdd1   = rtopptr;

              /* check !  need display allow data.                      */
              while(TRUE) {
                pthdd1 = pthdd1->nx_pos;

                if(    (pthdd1->status == U_S_INIT)
                    || (pthdd1->status == U_S_YOMA)
                    || (pthdd1->status == U_S_KNJU)    )  {
                  /* case : data exist.                                 */
                  if(  (pthdd1->yomilen == wyomilen)
                     && (memcmp(pthdd1->yomi,wyomi,wyomilen) == 0) )  {
                    /* case : need display allow data.                  */
                    dmappt->poststat = C_SWON;
		    col   = U_XYAJI + udcb->xoff;
                    dtlen = U_YAJLEN;
		    rc = kudisply(udcb,line,col,ydata[1],dtlen);
                  };
                  break;
                }
                else  {
                  if(pthdd1->nx_pos == NULL)  break;
                  pthdd1 = pthdd1->nx_pos;
                };
	      }; /* loop end for while  */
	      col = U_XGOKU + udcb->xoff + U_GOKLEN;
	      CURSOR_MOVE(line,col);
              return( IUSUCC );
	    } else {
              pthdd = dmappt->fld[0].dbufpt;
	    };
            /*
             *  search and set pointer to head of display data
             */
            i = 0;
	    wyomi = NULL;
	    wyomilen = 0;
            pthdd1 = pthdd;
            while(TRUE) {

	      /* fprintf(stderr,"pthdd1 = %ld\n",pthdd1);       */

	      if( pthdd1 == NULL )  break;
	      if(    (pthdd1->status == U_S_INIT)
		  || (pthdd1->status == U_S_YOMA)
		  || (pthdd1->status == U_S_KNJU)    )  {
		if(   (wyomilen == pthdd1->yomilen )
		   && (memcmp(pthdd1->yomi,wyomi,(int)(wyomilen)) == 0)) {
		  i++;
		} else {
		  i += 2;
		  wyomi = pthdd1->yomi;
		  wyomilen = pthdd1->yomilen;
		};
		/*
		 * fprintf(stderr,"naw culc gyosu = %d\n",i);
		 * fprintf(stderr,"wyomi          = %s\n",wyomi);
		 * fprintf(stderr,"wyomilen       = %d\n",wyomilen);
		 */
		if(i > gyosu) {
		  break;
		} else {
		  pthdd = pthdd1;
		};
	      };
	      pthdd1 = pthdd1->pr_pos;
            };
        };

	/* fprintf(stderr,"culc gyosu = %d\n",i);       */
	/* fprintf(stderr,"gyosu      = %d\n",gyosu);   */
        /*  5
         *      display process
         */

        /*
         *      pre data check
         */
        /* initialize previos flag.                                     */
        dmappt->prestat = C_SWOFF;

        pthdd1 = pthdd;

        while(TRUE) {
            if( pthdd1->pr_pos == NULL )  break;
            /* set pointer to UDCS of next data.                        */
            pthdd1 = pthdd1->pr_pos;

            if(    (pthdd1->status == U_S_INIT)
                || (pthdd1->status == U_S_YOMA)
                || (pthdd1->status == U_S_KNJU)    )  {

                if( (pthdd->yomilen == pthdd1->yomilen )
                 && (memcmp(pthdd->yomi,pthdd1->yomi,(int)(pthdd->yomilen))
                                                                     == 0) )
                    {
                        /* case : current Yomi match previos Yomi.      */
                        /* set previos flag.                            */
                        dmappt->prestat = C_SWON;
                        /* set axis of display allow.                   */
			col     = U_XYAJI + udcb->xoff;
			line    = hajime + 1;
			dtlen   = U_YAJLEN;

                        /* display allow data.                          */
			rc = kudisply(udcb,line,col,ydata[0],dtlen);

                    };
                    break;
            };
        };

        /*
         *      first line display
         */
        /*
         *      init yomigana
         */
        wyomi    = NULL;
        wyomilen = 0;

        /*
         *      display yomi data and set display map
         */
	/* set poststat.        */
        dmappt->poststat = C_SWOFF;

	/* init field & line counter    */
	fldno = -1;     /* filed no     */
        while(TRUE) {
	  if( (pthdd == NULL) || (fldno >= maxfld) )  break;
	  if(    (pthdd->status == U_S_INIT)
	      || (pthdd->status == U_S_YOMA)
	      || (pthdd->status == U_S_KNJU)    )  {
	    /*
	     *      display yomi data and set display map
	     */
	    /* set display field No.  */
	    fldno ++;

	    /* set display Y_axis.    */
	    line = hajime + fldno;
	    if(    (pthdd->yomilen != wyomilen )
		|| (memcmp(pthdd->yomi,wyomi,wyomilen) != 0)  )  {
	      /* case : display Yomigana. */

	      /* check display position   */
	      if(fldno >= maxfld) {
		dmappt->fld[fldno].fstat = U_NODIS;
		dmappt->fld[fldno].dbufpt = NULL;
		break;
	      };

	      /* keep current Yomi data.  */
	      wyomilen = pthdd->yomilen;
	      wyomi    = pthdd->yomi;

	      /* set display Yomi X_axis. */
	      col    = U_XYOMI + udcb->xoff;
	      /* set display data length. */
	      dtlen  = U_YOMLEN;
	      /* set pointer to input field.      */
	      data   = df[fldno].yofld;
	      /* set init stringth.       */
	      (void)memcpy(data,pthdd->yomi,(int)(pthdd->yomilen));
	      data[pthdd->yomilen] = NULL;
	      /* set display map.         */
	      /* set pointer to UDCS.     */
	      dmappt->fld[fldno].dbufpt = pthdd;
	      /* set status code. */
	      dmappt->fld[fldno].fstat  = U_YOMIF;
	      /* display Yomi data.       */
	      rc = kukakko(line,col,dtlen);

              if ( cnvflg == U_EUC ) {
                        euclen = 100;
			pclen = dtlen;
                        rc = kjcnvste(data, &pclen, eucstr, &euclen);
                        if( rc < 0 )
                                return( IUFAIL );
                        rc = kudisply(udcb,line,col,eucstr,dtlen);
              } else {
                        rc = kudisply(udcb,line,col,data,dtlen);
              }

	      continue;

	    } else {
	      /* case : display Yomigana. */

	      if( (pthdd->nx_pos != NULL) && (fldno == maxfld) ) {
	      /*
	       *      in case of draw allow to down.
	       */
		pthdd1 = pthdd;
		while(TRUE) {
		  /* set pointer to next data.      */
		  pthdd1 = pthdd1->nx_pos;

		  if(    (pthdd1->status == U_S_INIT)
		      || (pthdd1->status == U_S_YOMA)
		      || (pthdd1->status == U_S_KNJU)    )  {
		    /* case : this data is varid.   */
		    if(  (pthdd1->yomilen == wyomilen)
		     && (memcmp(pthdd1->yomi,wyomi,wyomilen) == 0) )  {
		      /* case : need display allow data.    */
		      /* set post flag.     */
		      dmappt->poststat = C_SWON;
		      /* set allow data X_axis.     */
		      col   = U_XYAJI + udcb->xoff;
		      /* set allow length.  */
		      dtlen = U_YAJLEN;
		      /* display allow data.        */
		      rc = kudisply(udcb,line,col,ydata[1],dtlen);
		    };
		    break;
		  } else  {
		    if(pthdd1->nx_pos == NULL)  break;
		    pthdd1 = pthdd1->nx_pos;
		  };
		}; /* loop end for while    */
	      };

	      /*
	       *      display goki data and set display map
	       */
	      /* set Goki X_axis.   */
	      col    = U_XGOKU + udcb->xoff;
	      /* set display Goki length.   */
	      dtlen  = U_GOKLEN;
	      /* set pointer to input field.*/
	      data   = df[fldno].gofld;
	      /* set init stringth. */
	     (void)memcpy( data , pthdd->kan , (int)(pthdd->kanlen) );
	      data[pthdd->kanlen] = NULL;
	      /* set display map.   */
	      /* set pointer to UDCS.       */
	      dmappt->fld[fldno].dbufpt = pthdd;
	      /* set status code.   */
	      dmappt->fld[fldno].fstat  = U_GOKUF;
	      /* display Goki field.        */
	      rc = kukakko(line,col,dtlen);
              if ( cnvflg == U_EUC ) {
                        euclen = 100;
			pclen = dtlen;
                        rc = kjcnvste(data, &pclen, eucstr, &euclen);
                        if( rc < 0 )
                                return( IUFAIL );
			eucstr[100-euclen]='\0';
                        rc = kudisply2(udcb,line,col,eucstr,dtlen);
              } else {
                        rc = kudisply(udcb,line,col,data,dtlen);
              }

	    };
	  };
	  /* set pointer to next data.                                */
	  pthdd = pthdd->nx_pos;
        };

        /* set status code to no use input field.                       */
	for(j=fldno+1;j<=maxfld;j++) {
	  dmappt->fld[j].fstat = U_NODIS;
	  dmappt->fld[j].dbufpt = NULL;
	};

	  return( IUSUCC );
}


int kukakko(line,col,dtlen)

short   line;   /* data display line    */
short   col;    /* data display column  */
short   dtlen;  /* data length  */

{
    int         rp,lp;
    static char kakko[2][4];


    strcpy(kakko[0], CU_MNKAK2);
    strcpy(kakko[1], CU_MNKAK3);


    rp = col - 2;
    lp = col + dtlen;

    /* write right bracket      */
    CURSOR_MOVE(line,rp);
    fprintf(stdout,"%s",kakko[0]);

    /* write left bracket       */
    CURSOR_MOVE(line,lp);
    fprintf(stdout,"%s",kakko[1]);

    return( IUSUCC );
}
