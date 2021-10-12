static char sccsid[] = "@(#)70  1.7  src/bos/usr/lib/nls/loc/imk/ked/kedInit.c, libkr, bos411, 9428A410j 6/9/94 09:55:41";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kedInit.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM ED  
 *
 *  Module:       kedInit.c
 *
 *  Description:  Makes all necessary resources available for
 *		  subsequent function calls and saves all 
 *		  information passed as input parameters internally 
 *              
 *  Functions:   kedInit()
 *              
 *  History:      
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <memory.h>
#include <fcntl.h>
#include <im.h>
#include <imP.h>
#include "kimerrno.h"
#include "kfep.h"
#include "kedconst.h"
#include "ked.h"
#include "Hhcim.h"

#define	 FOREWARD	1	
#define  UDICT_DEFAULTFILE  "/usr/lpp/kls/dict/usrdict"	
#define  O_UD_CREAT	    (O_RDWR | O_CREAT)

/**********************/
/* External reference */
/**********************/
extern ushort get_udstat();
extern ushort set_udstat();
extern ushort *get_ix_block();
extern int    init_MRU();

/*****************************************************************/
/* the interface will be changed, we need additional parameters  */
/*****************************************************************/
int		kedInit(textmaxwidth, auxformat, auxsize, prostr)
int		textmaxwidth;
int		auxformat;
AuxSize		auxsize;
kedprofile	*prostr;
{
        /*******************/
        /* local variables */
        /*******************/
        KIMED         *kimed ;
        char          **aux_str;
        char          **aux_atr;
        register      int  i;
	char	      *sdict;
	char	      *udict;
	ushort	      stat;
	int	      sdict_fd;
	int	      udict_fd;
	int 	      tmp;



        if ( !(kimed = (KIMED*)malloc(sizeof(KIMED))) ) {
                return (KP_ERR);
        }

	sdict = prostr->dictstru.sys;
	udict = prostr->dictstru.user;
	kimed->learn   = prostr->learn;
	kimed->acm = prostr->acm;
	if (udict == NULL) {
	   kimed->learn = OFF;
	}
	/****************/
	/* Open sysdict */
	/****************/
	if ((sdict_fd=open(sdict, O_RDONLY)) >= 0) {
	   if (prostr->learn == ON) {
		/******************/
		/* learning is ON */
		/******************/
		if (udict != NULL) {
		    /****************************/
		    /* Opens the existing udict */
		    /****************************/
	      	    udict_fd = open(udict, O_RDWR); 
		    if (udict_fd < 0) {
		      /************************/
		      /* Cannot open usr dict */	
		      /************************/
		      close(sdict_fd); free(kimed);
		      return (KP_ERR); 	
		    }
                    /**********************/
                    /* Gets udict status. */
                    /**********************/
                    stat = get_udstat(udict_fd);
                    if (stat == UD_NORMAL) {
                        /***********************/
                        /* Other process only  */
                        /* can read the udfile */
                        /***********************/
                        set_udstat(udict_fd, UD_ORDWR);
			kimed->udict.status = UDP_UPDATE;
		    } else {
                        /***********************/
                        /* Other process only  */
                        /* can read the udfile */
                        /***********************/
			kimed->udict.status = UDP_RDONLY;
			fprintf(stderr, "\nWarning: You cannot update User Dictionary!\n");
		    }
		    kimed->udict.fdesc  = udict_fd;
		    kimed->udict.fdstat = VALID_FDESC;
		    kimed->udict.dbunit = UDBLOCK_SIZE; /* 1024 */
		} else {
		   fprintf(stderr, "Warning: You must have"
		   " .usrdict at HOME directory!\n"
		   "         The learning is turned off.\n");
		   kimed->learn = OFF;
		   kimed->udict.status = (ushort)UDP_RDONLY;
		   kimed->udict.fdstat = INVALID_FDESC;
	 	} 
	   } else {
		/****************************************/
		/* If learning is off and udict is nil, */
		/* don't need to create udict. 		*/
		/****************************************/

		/****************************************/
		/* If learning is off, but udict isn't  */
		/* nil, then the process don't need to  */
		/* update MRU area.			*/ 
		/****************************************/
		if (udict != NULL) {
		    /***************************/
		    /* Opens the existing file */
		    /***************************/
	      	    if((udict_fd = open(udict, O_RDONLY)) < 0) { 
		  	/************************/
		  	/* Cannot open usr dict */	
		  	/************************/
		  	close(sdict_fd); free(kimed);
		  	return (KP_ERR); 	
		    }
		    kimed->udict.status = (ushort)UDP_RDONLY;
		    kimed->udict.fdesc  = udict_fd;
		    kimed->udict.fdstat = VALID_FDESC;
		    kimed->udict.dbunit = UDBLOCK_SIZE;
		} else {
		    /*****************************/
		    /* Now, in this case, access */
		    /* udict is always invalid   */
		    /*****************************/
		    kimed->udict.status = (ushort)UDP_RDONLY;
		    kimed->udict.fdesc  = (-1);
		    kimed->udict.fdstat = INVALID_FDESC;
		}
	   }
	} else {
		/************************/
		/* Cannot open sys dict */	
		/************************/
		free(kimed);
		return (KP_ERR);
	}
        if(textmaxwidth < MINECHO) {
	  close(sdict_fd);
	  if (kimed->udict.fdstat == VALID_FDESC)
	      close(udict_fd);
	  free(kimed);
	  return(KP_ERR);
	}
        /*******************/
        /* aux buffer size */
        /*******************/
 	if(auxformat == KP_LONGAUX) {     
             if((auxsize.itemsize < AUXITEMLEN) ||
                (auxsize.itemnum < AUXITEMNUM)) {
		    close(sdict_fd);
	  	    if (kimed->udict.fdstat == VALID_FDESC)
	     		 close(udict_fd);
		    free(kimed);
                    return(KP_ERR);
            }
 	}
        else {
            /***********************/
            /* aux of short format */
            /***********************/
            if((auxsize.itemsize < MINAUX) || 
	    (auxsize.itemnum < 1)) {
		    close(sdict_fd);
	  	    if (kimed->udict.fdstat == VALID_FDESC)
	     		 close(udict_fd);
		    free(kimed);
                    return(KP_ERR);
	    }
        }    
  	/**************************************/
  	/* echo, echo attribute , fix buffers */
  	/**************************************/
        kimed->echobufs = (uchar*)malloc(textmaxwidth);
        kimed->echobufa = (uchar*)malloc(textmaxwidth);
        kimed->fixbuf   = (uchar*)malloc(textmaxwidth);
	kimed->echosvch = (uchar*)malloc(MAXEUMLEN);

  	/**************************************/
  	/* candbuf's size is dynamic. 	*/	
  	/**************************************/
        kimed->auxbufs = 
	(uchar**)malloc(sizeof(caddr_t) * KIM_AUXROWMAX);
        kimed->auxbufa = 
	(uchar**)malloc(sizeof(caddr_t) * KIM_AUXROWMAX);

        aux_str = kimed->auxbufs;
        for(i = 0; i < KIM_AUXROWMAX; i++)
                *(aux_str)++ = malloc(KIM_AUXCOLMAX);

  	/**************************************/
  	/* attribute buffer 			*/
  	/**************************************/
        aux_atr = kimed->auxbufa;
        for(i = 0; i < KIM_AUXROWMAX; i++)
                *(aux_atr)++ = malloc(KIM_AUXCOLMAX);

  	/****************************************/
  	/* initialize information between KIMED */
  	/****************************************/
  	/* echo, echo attribute , fix buffers */
        memset(kimed->echobufs, NULL, textmaxwidth);
        memset(kimed->echobufa, NULL, textmaxwidth);
        memset(kimed->fixbuf, NULL, textmaxwidth);

  	/**************************************/
  	/* aux buffer 			*/
  	/* string buffer 			*/
  	/**************************************/
        aux_str = kimed->auxbufs;
        for(i = 0; i < KIM_AUXROWMAX; i++)
                memset(*(aux_str)++, NULL, KIM_AUXCOLMAX);

  	/**************************************/
  	/* attribute buffer 			*/
  	/**************************************/
        aux_atr = kimed->auxbufa;
        for(i = 0; i < KIM_AUXROWMAX; i++)
                memset(*(aux_atr)++, NULL, KIM_AUXCOLMAX);

  	/*****************************************/
  	/*  initializes values in KIMED	     	 */
  	/*****************************************/
	kimed->sdict.fdesc  = sdict_fd;
	kimed->sdict.fdstat = VALID_FDESC;
	kimed->sdict.status = (ushort)O_RDONLY;
	kimed->sdict.ixsz   = SIXBLOCK_SIZE;
	kimed->sdict.dbunit = SDBLOCK_SIZE; 
	kimed->sdict.ixb    = 
	get_ix_block(kimed->sdict.fdesc, SYSDICT, &tmp);

	kimed->udict.fdesc = udict_fd;

	/*********************/
	/* Inits MRU.        */
	/* Make the mru list */
	/*********************/
	if (kimed->learn == ON) 
	   init_MRU(kimed);
	else
	   *(&kimed->mrulist) = NULL;

	kimed->auxformat = auxformat;
	kimed->auxsize.itemsize = auxsize.itemsize;
	kimed->auxsize.itemnum = auxsize.itemnum;     
        kimed->echosize = textmaxwidth;
	kimed->fixsize = 		textmaxwidth;
	kimed->echoover = 		textmaxwidth;
	kimed->fixacsz = 		KIMED_FIXBUF_EMTY;
	kimed->echoacsz = 		KIMED_ECHOBUF_EMTY;
	kimed->curadv = 		KIMED_CURADV_FIRSTPOS;
	kimed->candgetfg = 		FOREWARD;
	kimed->echosvchlen = 		KIMED_ECHOSVCH_EMTY;
	kimed->echosvchsp = 		KIMED_ECHOSVCH_SP;
	kimed->candsize = 		KIMED_CANDBUF_EMTY;
	kimed->candcrpos = 		KIMED_CANDCR_FIRSTPOS;

        kimed->imode.basemode          = MD_ENG;
        kimed->imode.sizemode          = MD_BANJA;
        kimed->imode.hjmode            = MD_HJOFF;
        kimed->imode.supnormode        = MD_NORM;
        kimed->imode.insrepmode        = MD_REPLACE;

	kimed->echochfg.flag = 		OFF;
	kimed->echochfg.chtoppos = 	KIMED_ECHOBUF_SP;
	kimed->echochfg.chlenbytes = 	KIMED_NOCHANGED_LEN;
	kimed->echocrps = 		KIMED_ECHOBUF_SP;
	kimed->eccrpsch = 		OFF;
	kimed->echoacsz = 		KIMED_ECHOBUF_EMTY;
	kimed->auxuse = 		AUXBUF_NOTUSED;
	kimed->auxchfg  = 		OFF;
	kimed->auxcrps.colpos = 	KIMED_AUXCR_FIRST;
	kimed->auxcrps.rowpos = 	KIMED_AUXCR_FIRST;
	kimed->axcrpsch = 		OFF ;
	kimed->auxacsz.itemsize = 	KIMED_AUXBUF_EMTY;
	kimed->auxacsz.itemnum = 	KIMED_AUXBUF_EMTY;
	kimed->indchfg  = 		OFF ;
	kimed->needindrw =		ON;
	kimed->isbeep = 		OFF;
	kimed->hgstate = 		HG_ST_FINAL;
	kimed->prevhemode = 		MD_ENG; 
	kimed->interstate = 		ST_ENG;
	for (i=0; i<=4; i++)
	{
		kimed->hg_status_buf[i].state = HG_ST_FINAL;
		kimed->hg_status_buf[i].cmpshg = NULL;
		kimed->hg_status_buf[i].cmplhg = NULL;
	}
	kimed->hg_status_ps = -1 ;
	return ((int)kimed) ;
} /* end of  kedInit */
