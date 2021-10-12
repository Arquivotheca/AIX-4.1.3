static char sccsid[] = "@(#)73	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopen.c, libKJI, bos411, 9428A410j 7/23/92 03:16:51";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcopen
 *
 * DESCRIPTIVE NAME:  ALLOC & INIT OF TUKIJI INTERFACE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0108(WSPOVER):    work space is too small
 *                    0x0208(ELEM_SIZE):  element size is incorrect 
 *                    0x0b10(RQ_RECOVER): user dictionary need to be recoverd
 *                    0x0388(MEM_MALLOC): memory allocation error
 *                    0x0181(SYS_OPEN):   open error of system dictionary
 *                    0x0281(SYS_CLOSE):  close error of system dictionary
 *                    0x0981(SYS_SHMAT):  shared memory allocation error
 *                    0x1081(SYS_INCRR):  system dictionary is incorrect
 *                    0x0182(USR_OPEN):   open error of user dictionary
 *                    0x0282(USR_CLOSE):  close error of user dictionary
 *                    0x0582(USR_LSEEK):  seek error of user dictionary
 *                    0x0682(USR_READ):   read error of user dicitonary
 *                    0x0882(USR_LOCKF):  lock error of user dictionary
 *                    0x1082(USR_INCRR):  user dictionary is incorrect
 *                    0x0184(FZK_OPEN):   open error of adjunct dictionary
 *                    0x0584(FZK_LSEEK):  seek error of adjunct dictionary
 *                    0x0684(FZK_READ):   read error of adjunct dictionary
 *                    0x0884(FZK_LOCKF):  lock error of adjunct dictionary
 *                    0x1084(FZK_INCRR):  adjunct dictionary is incorrect
 *                    0x7fff(UERROR):     unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
/**********************************************************/
/* following portion commented out by #ifdef 7/04/89  $01 */
#ifdef _OLD_VER_70489
#ifdef _OLD
short _Kcopen( z_mcbadd )

struct MCB   *(*z_mcbadd);              /* pointer of MCB's pointer     */
#else
short _Kcopen( z_mcbadd , z_extinf)

struct MCB   *(*z_mcbadd);              /* pointer of MCB's pointer     */
struct INF   *z_extinf;
#endif
#endif _OLD_VER_70489
/* above portion commented out by #ifdef 7/04/89  $01    */
/*********************************************************/

short _Kcopen( z_mcbadd , z_extinf)

struct MCB   *(*z_mcbadd);              /* pointer of MCB's pointer     */
struct INF   *z_extinf;
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE                                                    
 *----------------------------------------------------------------------*/
#ifdef _AIX
   extern  short   _Kcopdcs(); 
   extern  short   _Kcopdcu(); 
   extern  short   _Kcopdcf(); 
   extern  short   _Kccldc0();
#endif

   extern  short   _Kcofkkc();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcmcb.h"   /* Monitor Control Block (MCB)                  */
#include   "_Kcinf.h"   /* Extended information Block (INF)             */
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcuxe.h"   /* User dictionary indeX Entry (UXE)            */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_i;                    /* loop counter                 */
   char         *z_mcball;              /* pointing at the top of MCB
                                         *            including INDEX   */
   short        z_ropdcx;               /* return code from _Kcopdc.()  */
   short        z_rofkkc;               /* return code from _Kcofkkc()  */
/*----------------------------------------------------------------------* 
 *      DEFINITION OF INDEX
 *----------------------------------------------------------------------*/
#define       Z_IDSIZE     40           /* size of INDEX                */
   char         *z_index = "**** KKC Control Block ****             ";

/**********************************************************/
/* following portion commented out by #ifdef 7/04/89  $01 */
#ifdef _OLD_VER_70489
#ifdef _OLD
   struct INF   *z_extinf;
   z_extinf = NULL;
#endif
#endif _OLD_VER_70489
/* above portion commented out by #ifdef 7/04/89  $01     */
/**********************************************************/
/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "MCB"
 *----------------------------------------------------------------------*/
/*--------------------   ALLOCATE MEMORY   -----------------------------*/
   if ( ( z_mcball = malloc( sizeof( struct MCB ) + Z_IDSIZE ) ) == NULL )
      return( MEM_MALLOC );

/*--------------------   SET MCB POINTER   -----------------------------*/
   *z_mcbadd =  ( struct MCB * )( z_mcball + Z_IDSIZE );
   mcbptr1 = *z_mcbadd;    

   mcb.mcbaddr = z_mcball;
/*--------------------   SET INDEX IN MCB   ----------------------------*/
   for( z_i = 0; z_i < Z_IDSIZE; z_i++ )
      z_mcball[z_i] = z_index[z_i];

/*--------------------   INITIALIZE MCB   ------------------------------*/
   mcb.length  = sizeof(struct MCB);
#ifdef _AIX
   mcb.id      = AIXID;
#else
#   if defined(_AGC) || defined(_GSW)
      mcb.id      = S5080ID;
#   endif
#endif
   mcb.mode    = 0;
   mcb.cnvx    = 0;
   mcb.cnvi    = 0;
   mcb.charl   = 0;
   mcb.charr   = 0;
   mcb.ymimaxll= YMIMAX;
   mcb.ymill1  = 0;
   mcb.ymill2  = 0;
   mcb.seimaxll= SEIMAX;
   mcb.seill   = 0;
   mcb.semmaxll= SEMMAX;
   mcb.semll   = 0;
   mcb.ymmmaxll= YMMMAX;
   mcb.ymmll   = 0;
   mcb.grmmaxll= GRMMAX;
   mcb.grmll   = 0;
   mcb.totcand = 0;
   mcb.reqcand = 0;
   mcb.outcand = 0;
   mcb.currfst = 0;
   mcb.currlst = 0;
   mcb.maxsei  = 0;
   for( z_i = 0; z_i < SDICT_NUM; z_i++ )
       mcb.dsyfd[z_i]  = -1;
   mcb.dusfd  = -1;
   mcb.dfzfd  = -1;

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "KCB"
 *----------------------------------------------------------------------*/
   if ( ( mcb.kcbaddr = (struct KCB *)malloc( sizeof( struct KCB ) ) ) 
                                                                == NULL )
   {
      free( z_mcball );                 /* free MCB area                */
      return( MEM_MALLOC );
   }

   kcbptr1 = mcb.kcbaddr;
   kcb.myarea  = ( uschar *)mcbptr1;

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "WORKING STORAGE POOL"
 *----------------------------------------------------------------------*/
   kcb.wsplen = ( (int)( MAXWSP / 4 ) + 1 ) * 4;

   if ( ( kcb.wsp = ( uschar *)malloc( kcb.wsplen ) ) == NULL )
   {
      free( mcb.kcbaddr );              /* free KCB area                */
      free( z_mcball );                 /* free MCB area                */
      return( MEM_MALLOC );
   }

   infptr1 = z_extinf;                  /* set ext.inf.pointer          */

#ifdef _AIX
/*----------------------------------------------------------------------*
 *      OPEN DICTIONARY FILES
 *----------------------------------------------------------------------*/
/*--------------------   SYSTEM DICTIONARY   ---------------------------*/
  for( z_i = 0; z_i < SDICT_NUM; z_i++ ) {
   if(infptr1 == NULL)
      z_ropdcx = _Kcopdcs( *z_mcbadd,NULL );
   else {
      if( *inf.sysnm[z_i] == NULL ) {
	  mcb.mul_file = z_i;
	  break;
      }
      else
	  z_ropdcx = _Kcopdcs( *z_mcbadd,inf.sysnm[z_i], z_i );
   }
   switch( z_ropdcx )
   {
      case SUCCESS:   break;
      case WSPOVER:
      case SYS_OPEN:
      case SYS_CLOSE:
      case SYS_SHMAT:
      case SYS_INCRR:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, SYS_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( z_ropdcx );
      default:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, SYS_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( UERROR );
   }
  }
  if( !mcb.mul_file )
       mcb.mul_file = z_i;

/*--------------------   USER DICTIONARY   -----------------------------*/
   if(infptr1 == NULL)
      z_ropdcx = _Kcopdcu( *z_mcbadd,NULL );
   else
      z_ropdcx = _Kcopdcu( *z_mcbadd,inf.usrnm );

   switch( z_ropdcx )
   {
      case SUCCESS:   break;
      case WSPOVER:
      case USR_OPEN:
      case USR_CLOSE:
      case USR_LSEEK:
      case USR_READ:
      case USR_LOCKF:
      case USR_INCRR:
      case RQ_RECOVER:
      case MEM_MALLOC:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, USR_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( z_ropdcx );
      default:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, USR_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( UERROR );
   }

/*--------------------   FUZOKU-GO DICTIONARY   ------------------------*/
   if(infptr1 == NULL)
      z_ropdcx = _Kcopdcf( *z_mcbadd,NULL );
   else
      z_ropdcx = _Kcopdcf( *z_mcbadd,inf.adjnm );

   switch( z_ropdcx )
   {
      case SUCCESS:   break;
      case WSPOVER:
      case FZK_OPEN:
      case FZK_INCRR:
      case MEM_MALLOC:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, FZK_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( z_ropdcx );
      default:
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
	 _Kccldc0( mcbptr1, FZK_CLOSE );
         free( z_mcball );              /* free MCB area                */
         return( UERROR );
   }
#else
#   if defined(_AGC) || defined(_GSW)
/*--------------------   SYSTEM DICTIONARY   ---------------------------*/
   if((mcb.dsyseg = inf.sysptr)== NULL) /* get of the top of dict addr. */
   {
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
         free( z_mcball );              /* free MCB area                */
         return( SYS_OPEN  );           /* error, file is closed        */
   }

   mcb.dsynm[ 0 ] = 0x00;               /* reset the path name area     */

/*--------------------   USER DICTIONARY   -----------------------------*/
   if((mcb.mdemde = inf.usrptr)== NULL) /* get of the top of dict addr. */
   {
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
         free( z_mcball );              /* free MCB area                */
         return( USR_OPEN  );           /* error, file is closed        */
   }
   uxeptr1 = mcb.uxeuxe = ( struct UXE *)( mcb.mdemde + MDESIZE );
                                        /* set pointer for INDEX of UDCT*/

   mcb.indlen = 0;                      /* Initial Index Recode Length  */
                                     /* Index Recode is 1 Record(1Kbyte)*/
   if ((uxe.har >= U_HAR_V1) && (uxe.har <= U_HAR_V2))  mcb.indlen = 1;
                                    /* Index Recode is 2 Record(2Kbyte) */
   if ((uxe.har >= U_HAR_V3) && (uxe.har <= U_HAR_V4))  mcb.indlen = 2;
                                    /* Index Recode is 3 Record(3Kbyte) */
   if ((uxe.har >= U_HAR_V5) && (uxe.har <= U_HAR_V6))  mcb.indlen = 3;
                                     /* Invalid har Value                */
   if ( mcb.indlen == 0 )  return ( RQ_RECOVER );

   mcb.udeude = ( uschar *)mcb.uxeuxe + (UXESIZE * mcb.indlen );
                                        /* set pointer for DATA of UDCT */

   mcb.dusnm[ 0 ] = 0x00;               /* reset the path name area     */

/*--------------------   FUZOKU-GO DICTIONARY   ------------------------*/
   if((mcb.dfgdfg = inf.adjptr)== NULL) /* get of the top of dict addr. */
   {
         free( kcb.wsp     );           /* free WSP area                */
         free( mcb.kcbaddr );           /* free KCB area                */
         free( z_mcball );              /* free MCB area                */
         return( FZK_OPEN  );           /* error, file is closed        */
   }
   mcb.dfznm[ 0 ] = 0x00;               /* reset the path name area     */
#   endif
#endif

/*----------------------------------------------------------------------*
 *      INITIALIZE KCB
 *----------------------------------------------------------------------*/
   kcb.func = FUNINITW;

   z_rofkkc = _Kcofkkc( kcbptr1 );

   switch( z_rofkkc )
   {
      case SUCCESS:   break;

      case WSPOVER:
      case ELEM_SIZE:
      case RQ_RECOVER:
      case USR_LSEEK:
      case FZK_LSEEK:
      case USR_READ:
      case FZK_READ:
      case USR_LOCKF:
      case FZK_LOCKF:
         _Kcclose( mcbptr1 );           /* free all of allocated memory */
                                        /* and close all dictionaries   */
         return( z_rofkkc );

      default:
         _Kcclose( mcbptr1 );           /* free all of allocated memory */
                                        /* and close all dictionaries   */
         return( UERROR );
   }

   mcb.ymiaddr = kcb.ymiaddr;
   mcb.seiaddr = kcb.seiaddr;
   mcb.semaddr = kcb.semaddr;
   mcb.ymmaddr = kcb.ymmaddr;
   mcb.grmaddr = kcb.grmaddr;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
