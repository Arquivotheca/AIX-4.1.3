/* @(#)47	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmcb.h, libKJI, bos411, 9428A410j 7/23/92 03:16:24	*/

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcmcb.h
 *
 * DESCRIPTIVE NAME:  Control block for interface between KKC and KJ-monitor
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       
 *
 ******************** END OF SPECIFICATIONS *****************************/

/**************************************************************************/
/**************************************************************************/
/* Monitor Control Block (MCB)						  */
/*                                                                        */
/*    The MCB is a control block which is used to realize the interface   */
/*  requirement which is unique to IP E/S. All fields in the MCB are      */
/*  under the control of IP E/S, hence all MCB field names, formats       */
/*  or meanings can be changed without any notification to the other      */
/*  department.                                                           */
/**************************************************************************/
/**************************************************************************/

#include "_Kcmsys.h"                    /* Multi System Dictionary        */

struct MCB
{

/**************************************************************************/
/**************************************************************************/
/* Communication Section                                                  */
/*                                                                        */
/*    This section is used as communication area between KKC and the user */
/*  (monitor). When a user requests one of the KKC functions, appropriate */
/*  fields in this MCB must be set prior to making a function call.       */
/*  Upon completion of a function request, the KKC places outputs         */
/*  in some of the MCB fields. No other section than this Communications  */
/*  Section is allowed to be referenced by the user. All other sections   */
/*  are for KKC internal use only.                                        */
/**************************************************************************/

/*----------------------------------------------------------------------*
 *      FIXED AREA 
 *----------------------------------------------------------------------*/
 long       length;                     /* length of struct MCB         */
 long       id;                         /* indentifier for Version      */

/*----------------------------------------------------------------------*
 *      COMMUNICATION BUFFER
 *----------------------------------------------------------------------*/
 struct YMI *ymiaddr;                   /* Ptr of associated yomi buffer*/
 struct SEI *seiaddr;                   /* Ptr of seisho buffer         */
 struct SEM *semaddr;                   /* Ptr of seisho map area       */
 struct YMM *ymmaddr;                   /* Ptr of yomi map area         */
 struct GRM *grmaddr;                   /* Ptr of grammer map area      */

/*----------------------------------------------------------------------*
 *      CONVERSION MODE
 *----------------------------------------------------------------------*/
 uschar     mode;                       /* Conversion mode              */

#define    TSMFUKU   0x00               /* Fukugou-go                   */
#define    TSMTAN    0x01               /* Bunsetsu (single)            */
#define    TSMREN    0x02               /* Bunsetsu (multiple)          */
#define    TSMALPKN  0x0E               /* Alphabet & Kansuji           */
#define    TSMALPHA  0x0F               /* Alphabet                     */
#define    TSMRYAKU  0x10               /* Ryakushou                    */
#define    TSMKANSU  0x11               /* Kansuji                      */

/*----------------------------------------------------------------------*
 *      REQUEST FOR 'KAKUTEI'
 *----------------------------------------------------------------------*/
 uschar     cnvx;                       /* Explicit req for kakutei     */
                                        /* x'00': no  (use KAKNO below) */
                                        /* other: yes                   */
 uschar     cnvi;                       /* Req for internal kakutei     */
                                        /* (valid when cnvx=00)         */
                                        /* x'00': do not kakutei        */
                                        /* other: do kakutei whenever   */
                                        /*        possible              */
#define    TSKNO     0x00               /* No Kakutei Request           */

/*----------------------------------------------------------------------*
 *      LEFT/RIGHT CHARACTER
 *----------------------------------------------------------------------*/
uschar      charl;                      /* Jishu-Code, left to the      */
                                        /*             current yomi     */
uschar      charr;                      /* Jishu-Code, right to the     */
                                        /*             current yomi     */

/*----------------------------------------------------------------------*
 *      STATUS OF ALPHABET/KANSUJI CONVERSION
 *----------------------------------------------------------------------*/
uschar      alpkan;                     /* status of alpha/kansu conv   */
                                        /*  0x01: alpha conv only       */
                                        /*  0x02: kansu conv only       */
                                        /*  0x03: alpha & kansu conv    */

/*----------------------------------------------------------------------*
 *      LENGTH OF COMMUNICATION BUFFER
 *----------------------------------------------------------------------*/
 short       ymimaxll;                  /* Max yomi buffer len, bytes   */
 short       ymill1;                    /* Length of input yomi in bytes*/
 short       ymill2;                    /* Offset to the J/F boundary   */
/*----------------------------------------------------------------------*/
 short       seimaxll;                  /* Max seisho buffer len, bytes */
 short       seill   ;                  /* Current seisho len, bytes    */
/*----------------------------------------------------------------------*/
 short       semmaxll;                  /* Max seisho map area len, byte*/
 short       semll   ;                  /* Current seisho map len, bytes*/
/*----------------------------------------------------------------------*/
 short       ymmmaxll;                  /* Max yomi map area len, bytes */
 short       ymmll   ;                  /* Current yomi map len, bytes  */
/*----------------------------------------------------------------------*/
 short       grmmaxll;                  /* Max grammer map len, bytes   */
 short       grmll   ;                  /* Current grammer map len, byte*/

/*----------------------------------------------------------------------*
 *      PARAMETERS FOR ALL_CANDIDATES CONVERSION
 *----------------------------------------------------------------------*/
 short       totcand;                   /* Total no of cand for the     */
                                        /* current environment.         */
 short       reqcand;                   /* No of candidates to be       */
                                        /* returned to the caller.      */
 short       outcand;                   /* No of candidates in the curr */
                                        /* output buffer.               */
 short       currfst;                   /* Number of the fast candidate */
                                        /* in the current output buffer */
 short       currlst;                   /* Number of the last candidate */
                                        /* in the current output buffer */
/*----------------------------------------------------------------------*/
 short       maxsei;                    /* Max seisho len, in bytes, for*/
                                        /* zenkoho operation.           */

/*----------------------------------------------------------------------*
 *      FILE DISCRIPTERS FOR DICTIONARIES
 *----------------------------------------------------------------------*/
 long        dsyfd[SDICT_NUM+1];        /* AIX file descriptor, negative*/
                                        /* if not opened.               */
 long        dusfd;                     /* AIX file descriptor, negative*/
                                        /* if not opened.               */
 long        dfzfd;                     /* AIX file descriptor, negative*/
                                        /* if not opened.               */
 long        dumtime;                   /* Time of last data modification  */
 long        ductime;                   /* Time of last file status change */

/**************************************************************************/
/**************************************************************************/
/* System Section                                                         */
/*                                                                        */
/*    This section is for KKC internal operations, thus no user program   */
/*  is allowed to reference or change fields in this section.             */
/*  All field names, formats and meanings are subject to change without   */
/*  any notification to the user.                                         */
/**************************************************************************/
/**************************************************************************/

/*------------------------------------------------------------------------*
 *      POINTER OF KCB
 *------------------------------------------------------------------------*/
 struct KCB *kcbaddr;                   /* Addr of my KCB                 */
					
/*------------------------------------------------------------------------*
 *      POINTER OF MCB INCLUDING IDINTIFIER
 *------------------------------------------------------------------------*/
 char     *mcbaddr;                   	/* addr of mcbtop                 */

/*------------------------------------------------------------------------*
 *      INDEX LENGTH OF USER DICTIONARY
 *------------------------------------------------------------------------*/
 long     indlen;                     	/* length of dictinnary buffer    */

/*------------------------------------------------------------------------*
 * Diagnostics/Trace Control Information     not used 
 *------------------------------------------------------------------------*/
#if NOT_USED
 uschar      diag1;                 	/* specify trace event            */
 uschar      diag2;                   	/* specify trace output           */
			
 #define     DIAGNONE 	0x00            /* no trace                       */
 #define     DIAGALL  	0xff            /* trace all events               */
 #define     DIAG1     	0x40            /* reserved                       */
 #define     DIAG2     	0x20            /* reserved                       */
 #define     DIAGNONE 	0x00            /* need no diag output            */
 #define     DIAGALL  	0xff            /* need all diag output           */
 #define     DIAGFLOW 	0x80            /* req ctl flow trace             */
 #define     DIAG3     	0x40            /* reserved                       */
 #define     DIAG4     	0x20            /* reserved                       */
#endif  /* NOT_USED */

/*------------------------------------------------------------------------*
 * System Dictionary Information (6100/AIX dependent) 
 *------------------------------------------------------------------------*/
#if NOT_USED
 uschar      dsynm[80];               	/* Path Name (File Name)          */
 short       dsynmll;                 	/* Path Name length               */
 uschar      *dsyseg;                  	/* Addr of shared mem segment,    */
                                        /* null if not usable.            */
 struct SXE  *sxesxe;                 	/* Addr of sys dict index buffer  */
 uschar      *sdesde;                 	/* Addr of sys dict buffer        */
 uschar      *txetxe;                 	/* Addr of single dict indx buff  */
 uschar      *tdetde;                 	/* Addr of single dict buffer     */

 short       dsyxerrn;                	/* RRN of curr Ndx Buff Content   */
 short       dsyderrn;                	/* RRN of curr Dat Buff Content   */
 short       dtyxerrn;                	/* RRN of curr Dat Buff Content   */
 short       dtyderrn;                	/* RRN of curr Dat Buff Content   */
 uschar      dsyxectl;                	/* Current Ndx Buff Ctl flag      */
 uschar      dsydectl;                	/* Current Data Buff Ctl Flag     */
 uschar      dtyxectl;                	/* Current Ndx Buff Ctl Flag      */
 uschar      dtydectl;                	/* Current Data Buff Ctl Flag     */
 uschar      dsylock;                 	/* Current lock mode.             */
 #define     DSYXEINV 	0x80            /* Buffer content is not usable   */
 #define     DSYXEWRT 	0x40            /* Write pending for current buff */
 #define     DSYDEINV 	0x80            /* Buffer content is not usable   */
 #define     DSYDEWRT 	0x40            /* Write pending for current buff */
 #define     DTYXEINV 	0x80            /* Buffer content is not usable   */
 #define     DTYXEWRT 	0x40            /* Write pending for current buff */
 #define     DTYDEINV 	0x80            /* Buffer content is not usable   */
 #define     DTYDEWRT 	0x40            /* Write pending for current buff */
 #define     DSYLK    	0x80            /* DSYLK DSYLKEX lock-status----- */
 #define     DSYLKEX  	0x40            /*   0      0    none             */
                                        /*   0      1    invalid          */
                                        /*   1      0    share lock       */
                                        /*   1      1    exclusive lock   */
#endif  /* NOT_USED */

/*------------------------------------------------------------------------*
 * User Dictionary Information (6100/AIX Dependent)   
 *------------------------------------------------------------------------*/
 uschar      dusnm[80];               	/* Path Name (File Name)          */
 short       dusnmll;                 	/* Path Name length               */
 struct UXE  *uxeuxe;                 	/* Addr of user dict index buff   */
 uschar      *udeude;                 	/* Addr of user dict buffer       */
 uschar      *mdemde;                 	/* Addr of MRU buffer             */

#if NOT_USED
 uschar      duslock;                 	/* Current lock mode.             */
 #define     DUSLK    	0x80          	/* DUSLK DUSLKEX lock-status  ----*/
 #define     DUSLKEX  	0x40          	/*   0      0    none             */
                                      	/*   0      1    invalid          */
                                      	/*   1      0    share lock       */
                                      	/*   1      1    exclusive lock   */
 short       dusxerrn;                	/* RRN of current Ndx Buf Content */
 short       dusderrn;                	/* RRN of current Dat Buf Content */
 short       dusmrrrn;                	/* RRN of current MRU Buf Content */
 uschar      dusxectl;                	/* Current Ndx Buff Ctl flag      */
 uschar      dusdectl;                	/* Current Data Buff Ctl Flag     */
 uschar      dusmrctl;                	/* Current MRU Buff Ctl Flag      */
 #define     DUSXEINV 	0x80          	/* Buffer content is not usable   */
 #define     DUSXEWRT 	0x40          	/* Write pending for current buff */
 #define     DUSDEINV 	0x80          	/* Buffer content is not usable   */
 #define     DUSDEWRT 	0x40          	/* Write pending for current buff */
 #define     DUSMRINV 	0x80          	/* Buffer content is not usable   */
 #define     DUSMRWRT 	0x40          	/* Write pending for current buff */
#endif  /* NOT_USED */

/*------------------------------------------------------------------------*
 * Fuzokugo Gakushuu File Information (6100/AIX Dependent Portion)   
 *------------------------------------------------------------------------*/
 uschar      dfznm[80];               	/* Path Name (File Name)          */
 short       dfznmll;                 	/* Path Name length               */
 uschar      *dfgdfg;                 	/* Addr of fuzoku dict buffer     */

#if NOT_USED
 uschar      dfzlock;                 	/* Current lock mode.             */
 define      DFZLK    	0x80            /* DFZLK DFZLKEX lock-status----- */
 define      DFZLKEX  	0x40            /*   0      0    none             */
                                      	/*   0      1    invalid          */
                                      	/*   1      0    share lock       */
                                      	/*   1      1    exclusive lock   */
 short       dfzderrn;                	/* RRN of current Dat Buf Content */
 uschar      dfzdectl;                	/* Current Data Buff Ctl Flag     */
 #define     DFZDEINV 	0x80            /* Buffer content is not usable   */
 #define     DFZDEWRT 	0x40            /* Write pending for current buff */

/*------------------------------------------------------------------------*
 *      RESERVED AREA          not used
 *------------------------------------------------------------------------*/
 uschar     *rsv02[10];               	/* Reserved                       */
#endif  /* NOT_USED */

/*------------------------------------------------------------------------*
 *      ADDITION INFORMATION OF MULTI SYSTEM DICTIONARY 
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*
 * System Dictionary Information (RS/6000 AIX V3.2.3 dependent) 
 *------------------------------------------------------------------------*/
   uschar      *dsynm[SDICT_NUM+1];     /* Path Name (File Name)          */
   short       dsynmll[SDICT_NUM+1];    /* Path Name length               */
   uschar      *dsyseg[SDICT_NUM+1];    /* Addr of shared mem segment,    */
                                        /* null if not usable.            */
   struct SXE  *sxesxe[SDICT_NUM+1];    /* Addr of sys dict index buffer  */
   uschar      *sdesde[SDICT_NUM+1];    /* Addr of sys dict buffer        */
   uschar      *txetxe[SDICT_NUM+1];    /* Addr of single dict indx buff  */
   uschar      *tdetde[SDICT_NUM+1];    /* Addr of single dict buffer     */

   uschar      mul_file;		/* Multi dict file count	  */
   struct      MULSYS mulsys[SDICT_NUM+1]; /* System dict Information	  */
};

 struct MCB *mcbptr1;                   /* primary   MCB pointer          */

#define mcb      (*mcbptr1)
