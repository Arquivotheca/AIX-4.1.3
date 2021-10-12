static char sccsid[] = "@(#)70	1.3.1.2  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopdcf.c, libKJI, bos411, 9428A410j 1/7/93 01:51:25";
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
 * MODULE NAME:       _Kcopdcf
 *
 * DESCRIPTIVE NAME:  OPEN FUZOKUGO GAKUSYU FILE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):  success
 *                    0x0108(WSPOVER):  overflow of work space buffer
 *                    0x0184(FZK_OPEN): error of open()
 *                    0x0a84(FZK_FSTAT):error of fstat on Fuzokugo dict.
 *                    0x1084(FZK_INCRR):fuzokugo Dictionary not true
 *                    0x0388(MEM_MALLOC): error of malloc()
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   <fcntl.h>                    /* define for open              */
#include   <sys/stat.h>                 /* define for fstat             */

#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "dict.h"

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcopdcf( z_mcbptr, z_fzknm )

struct  MCB     *z_mcbptr;              /* pointer of MCB               */
char            *z_fzknm;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   char    *getenv();                   /* define getenv()              */
   char    *strcat();                   /* define strcat()              */
   char    *strcpy();                   /* define strcpy()              */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_EXLEN   20

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_length;               /* set length of path name      */
   char         *z_fname;               /* return value from getenv()   */
   char         *z_envnm;               /* return value from getenv()   */
   char         z_fnmwk[ENVLIM+Z_EXLEN];/* temporaly work area          */
   struct stat  z_buf;                  /* infomation of fstat          */
   short        z_rt;                   /* return code                  */
   FDICTINFO    GetCurrentFDICTINFO();  /* get current Fdict pointer    */
   FDICTINFO    fdictp;                 /* system dict information      */

/*----------------------------------------------------------------------*
 *      Check whether the dictionary is already opened or not
 *----------------------------------------------------------------------*/
   fdictp = GetCurrentFDICTINFO();
   if ( fdictp->dfzfd >= 0 )
      return( _Kcopdcf2( z_mcbptr, z_fzknm ) );

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'th to mcb  */
   z_fname = NULL;

/*----------------------------------------------------------------------*
 *      OPEN THE FILE REQUESTED BY KJ-MONITOR
 *----------------------------------------------------------------------*/
   if(( mcb.dfzfd = OpenFile( z_fzknm, O_RDONLY )) != -1 )
      z_fname = z_fzknm;
   else
      return( FZK_OPEN );

/*******************************************************/
/* following portion is removed by #ifdef 7/04/89  $01 */
#ifdef _OLD_VER_70489
   if ( z_fname == NULL )
   {

   /*-------------------------------------------------------------------*
    *   OPEN THE FILE DEFINED IN ENVIRONMENT VARIABLE
    *-------------------------------------------------------------------*/
      if(( z_envnm = getenv( ENVFZK ) ) != NULL )
      {
         if(( mcb.dfzfd = open( z_envnm, O_RDWR )) != -1 )
            z_fname = z_envnm;
         else
            return( FZK_OPEN );
      }

      else
      {
      /*----------------------------------------------------------------*
       *        OPEN THE FILE UNDER THE CURRENT DIRECTRY
       *----------------------------------------------------------------*/
         if(( mcb.dfzfd = open( FZKDICT, O_RDWR )) != -1 )
            z_fname = FZKDICT;

         else
         {

         /*-------------------------------------------------------------*
          *     OPEN THE FILE UNDER THE HOME DIRECTRY
          *-------------------------------------------------------------*/
            if(( z_envnm = getenv( "HOME" )) != NULL )
            {
               if ( ( strlen(z_envnm) + 1 + strlen( FZKDICT ) )
                                                >= (ENVLIM + Z_EXLEN ) )
                  return( WSPOVER );

               strcpy( &z_fnmwk[0] , z_envnm ); /* set path of $HOME    */
               strcat( &z_fnmwk[0] , "/" );     /* $HOME + "/"          */
               strcat( &z_fnmwk[0] , FZKDICT ); /* $HOME + "/" + dictnm */

               if(( mcb.dfzfd = open( &z_fnmwk[0] , O_RDWR )) != -1 )
                  z_fname  = &z_fnmwk[0];       /* point the path name  */
            }

            if ( z_fname == NULL )
            {

            /*-------------------------------------------------------------*
             *  OPEN THE DEFAULT FILE 
             *-------------------------------------------------------------*/
               if ( ( strlen(z_fnmwk) + 1 + strlen( FZKDICT ) ) 
                                                >= ( ENVLIM + Z_EXLEN ) )
                  return( WSPOVER );

               strcpy( &z_fnmwk[0], DCTPATH );
               strcat( &z_fnmwk[0] , "/"  );
               strcat( &z_fnmwk[0] , FZKDICT );

               if(( mcb.dfzfd = open( &z_fnmwk[0] , O_RDWR )) != -1 )
                  z_fname  = &z_fnmwk[0];  
               else
                  return( FZK_OPEN );
            }
         }
      }
   }
#endif _OLD_VER_70489
/* above portions is removed by #ifdef 7/04/89  $01 */
/****************************************************/

/*----------------------------------------------------------------------*
 *      CHECK FUZOKUGO DICTIONARY
 *----------------------------------------------------------------------*/
   if( fstat( mcb.dfzfd , &z_buf ) == -1 )      /* get file status      */
       return ( FZK_FSTAT );

   /*-------------------------------------------------------------------*
    *      CHECK THE SIZE WHETHER IT IS 512 K BYTE
    *-------------------------------------------------------------------*/
   if(z_buf.st_size != DFGSIZE)
      return( FZK_INCRR );

/*----------------------------------------------------------------------*
 *      SET INFORMATION IF FZOKU-GO DICTIONARY
 *----------------------------------------------------------------------*/
   mcb.dfznm[ 0 ] = 0x00;               /* reset the path name area     */

   if ( ( z_length = strlen( z_fname ) ) >= ENVLIM )
   {
      strncpy( mcb.dfznm , z_fname, ENVLIM-1 );
      mcb.dfznm[ENVLIM-1] = '\0';
      mcb.dfznmll = ENVLIM - 1;         /* set length of path name      */
   }
   else
   {
      strcpy( mcb.dfznm , z_fname );    /* set path name                */
      mcb.dfznmll = z_length;           /* set length of path name      */
   }

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
   if(( mcb.dfgdfg = (uschar *)malloc( z_buf.st_size )) == NULL )
   {
      return( MEM_MALLOC );
   }

/*----------------------------------------------------------------------*
 *      Copy dictionary information to Fdict structure
 *----------------------------------------------------------------------*/
   if ( (fdictp->fdictname = (char *)malloc( strlen( z_fname ) + 1 )) != NULL )
      strcpy( fdictp->fdictname, z_fname );
   fdictp->dfzfd = mcb.dfzfd;
   fdictp->dfgdfg = mcb.dfgdfg;

/*----------------------------------------------------------------------*
 *       READ FUZOKU-GO GAKUSHU DICT
 *----------------------------------------------------------------------*/
   z_rt = _Kczread( mcb.kcbaddr, (short)FD, (short)0, (short)SHARE, (short)0 );
   if ( z_rt != SUCCESS ) {
      free( mcb.dfgdfg );
      return( z_rt );
   }

/*----------------------------------------------------------------------*
 *      RETUEN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
