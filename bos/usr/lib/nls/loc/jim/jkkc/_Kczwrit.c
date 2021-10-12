static char sccsid[] = "@(#)13	1.7.1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kczwrit.c, libKJI, bos41J 3/2/95 02:08:41";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kczwrit
 *
 * DESCRIPTIVE NAME:  WRITE DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0584(FZK_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0684(FZK_READ):   error of read()
 *                    0x0782(USR_WRITE):  error of write()
 *                    0x0784(FZK_WRITE):  error of write()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0884(FZK_LOCKF):  error of lockf()
 *                    0x0a10(UPDATING):   being updated
 *                    0x0b10(RQ_RECOVER): request of recovery
 *                    0x7fff(UERROR):     unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#include   "_Kcmac.h"                   /* Kkc macro definition		*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kczwrit( z_kcbptr, z_catego, z_rrn, z_mode )

struct  KCB     *z_kcbptr;              /* pointer of KCB               */
short   z_catego;                       /* category type                */
short   z_rrn;                          /* rrelative record number      */
short   z_mode;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
#ifdef _AIX
   extern   short   _Kcxstat();
#endif

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#ifdef _AIX
#include <fcntl.h>                      /* include for open()           */
#include <sys/lockf.h>                  /* include for lockf()          */
#include <sys/errno.h>                  /* include for errno            */
#endif

#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_IQ           0x01
#define   Z_SET_ON       0xf2
#define   Z_SET_OFF      0x00

#define   Z_SX_OFFSET     130
#define   Z_SD_OFFSET    2048
#define   Z_UX_OFFSET    (1024*7)
#define   Z_UD_OFFSET       0
#define   Z_MRU_OFFSET      0
#define   Z_FD_OFFSET       0

#define   Z_SD_RECORD    2048
#define   Z_UX_RECORD    1024
#define   Z_UD_RECORD    1024
#define   Z_MRU_RECORD   7168
#define   Z_FD_RECORD     200

#define   Z_INIT            0

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_offset;               /* set offset                   */
   short        z_record;               /* set recored size             */
   short        z_ditype;               /* set dictionary type          */
   short        z_i;                    /* loop counter                 */
   char         *z_bufnam;              /* set buffer name              */
   int          z_fildes;               /* set file descripter          */
   short        z_stat;                 /* biffer of statas of usrdct   */
   short        z_rc;
   int          fd;                     /* file descriptor              */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish KCB pointer        */
   mcbptr1 = ( struct MCB *)kcb.myarea; /* get MCB pointer              */

   {
       UDICTINFO dictp;
       dictp = GetCurrentUDICTINFO();

       if ( dictp->dusfd >= 0 &&
           mcbptr1->uxeuxe != (struct UXE *)dictp->uxeuxe ) {
           RefreshMcbUdict( mcbptr1, dictp );
       }
   }
/*----------------------------------------------------------------------*
 *      CHEK CATEGORY OF DICTIONARY
 *----------------------------------------------------------------------*/
   switch ( z_catego )                  /* check the category code and  */
   {                                    /* set file descripter, & etc.  */
      case UX :   for( z_i = 0;
                       ((uschar *)mcb.uxeuxe + z_i ) < mcb.udeude; z_i++ )
                  {
                     *( (uschar *)mcb.uxeuxe + z_i ) 
                                       = *( (uschar *)kcb.uxeuxe + z_i );
                  }
                  z_fildes = mcb.dusfd;
                  z_offset = Z_UX_OFFSET;
                  z_record = Z_UX_RECORD;
                  z_bufnam = ( char *)mcb.uxeuxe;
                  z_ditype = USR_ERROR;
                  break;

      case UD :   for( z_i = 0; z_i < Z_UD_RECORD; z_i++ )
                     *( mcb.udeude + (z_rrn-8) * Z_UD_RECORD + z_i )
                                                 = *( kcb.udeude + z_i );
                  z_fildes = mcb.dusfd;
                  z_offset = Z_UD_OFFSET;
                  z_record = Z_UD_RECORD;
                  z_bufnam = ( char *)mcb.udeude + (z_rrn-8) * Z_UD_RECORD;
                  z_ditype = USR_ERROR;
                  EMORAto7bit(z_bufnam);
                  break;

#ifdef _AIX
      case MRU: z_fildes = mcb.dusfd;
                z_offset = Z_MRU_OFFSET;
                z_record = Z_MRU_RECORD;
                z_bufnam = ( char *)mcb.mdemde;
                z_ditype = USR_ERROR;
		EMORAto7bit(z_bufnam);
                break;

      case FD : z_fildes = mcb.dfzfd;
                z_offset = Z_FD_OFFSET;
                z_record = Z_FD_RECORD;
                z_bufnam = ( char *)mcb.dfgdfg;
                z_ditype = FZK_ERROR;
                break;
#else
#   if defined(_AGC) || defined(_GSW)
      case MRU: return ( SUCCESS );

      case FD : return ( SUCCESS );
#   endif
#endif

      default : return( UERROR );
   }

#ifdef _AIX
/*----------------------------------------------------------------------*
 *      LOCK DICTIONARY FILES
 *----------------------------------------------------------------------*/
   lseek( z_fildes, Z_INIT, 0 );

   switch( z_mode )
   {
      case EXCLUSIVE: if ( lockf( z_fildes, F_TLOCK, 0 ) == -1 )
                         return( LOCKF_ERROR * 0x0100 + z_ditype );
                      break; 

      case SHARE:     if ( lockf( z_fildes, F_ULOCK, 0 ) == -1 )
                         return( LOCKF_ERROR * 0x0100 + z_ditype );
                      break; 

      default:        return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      CALUCULATE PHYSICAL LOCATION, AND WRITE TO DICTIONARY   
 *----------------------------------------------------------------------*/
   if ( z_catego == MRU || z_catego == UX || z_catego == UD )
   {
       z_stat = _Kcxstat( mcbptr1, Z_IQ );
       switch( z_stat )
       {
          case SUCCESS:
          case MRUBROKEN:
			   break;

          case UPDATING:
          case RQ_RECOVER:
          case USR_LSEEK:
          case USR_LOCKF:
          case USR_READ:
                           return( z_stat );
          default:         return( UERROR );
      }

      z_stat = _Kcxstat( mcbptr1, Z_SET_ON );

      switch( z_stat )
      {
         case SUCCESS:    break;

         case USR_LSEEK:
         case USR_LOCKF:
         case USR_WRITE:
                          return( z_stat );

         default:         return( UERROR );
      } 
   }

   z_rc = SUCCESS;

/*----------------------------------------------------------------------*
 *      Get file descriptor to write
 *----------------------------------------------------------------------*/
   if ( z_catego == MRU || z_catego == UX || z_catego == UD )
   {
      if ( ( fd = OpenUdict( O_RDWR ) ) == -1 )
         z_rc = LSEEK_ERROR * 0x0100 + z_ditype;
   }
   else
   {  FDICTINFO fdictp;
      fdictp = GetCurrentFDICTINFO();
      if ( ( fdictp->dfzfd < 0 ) || ( fdictp->fdictname == NULL ) )
         z_rc = LSEEK_ERROR * 0x0100 + z_ditype;
      else
         if ( ( fd = OpenFile( fdictp->fdictname, O_RDWR ) ) == -1 )
            z_rc = LSEEK_ERROR * 0x0100 + z_ditype;
   }

   if( lseek( fd, z_offset + ( z_rrn * z_record ), 0 ) != -1 )
   {
      errno = 0;
      if(( write( fd, z_bufnam, z_record ) != -1 )||(errno == EBADF ))
      {
         if ( z_catego == MRU || z_catego == UX || z_catego == UD )
         {
            z_stat = _Kcxstat( mcbptr1, Z_SET_OFF );

            switch( z_stat )
            {
               case SUCCESS:    break;
      
               case USR_LSEEK:
               case USR_LOCKF:
               case USR_WRITE:
                                return( z_stat );

               default:         return( UERROR );
            } 
         }
      }
      else
         z_rc = WRITE_ERROR * 0x0100 + z_ditype; 
   }
   else
      z_rc = LSEEK_ERROR * 0x0100 + z_ditype; 

   close ( fd );

/*----------------------------------------------------------------------*
 *      UNLOCK DICTIONARY FILES
 *----------------------------------------------------------------------*/
   lseek( z_fildes, Z_INIT, 0 );

   if ( lockf( z_fildes, F_ULOCK, 0 ) == -1 )
      z_rc = LOCKF_ERROR * 0x0100 + z_ditype;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rc ); 
#endif
}


/*----------------------------------------------------------------------*
 *	conversion table for EMORA to 7 bit code
 *----------------------------------------------------------------------*/
static	uschar	EMORAtoOLD[0x100] = {
/*      0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */

/*0*/ 0x00,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
/*1*/ 0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
/*2*/ 0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
/*3*/ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
/*4*/ 0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
/*5*/ 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
/*6*/ 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
/*7*/ 0x70,0x71,0x72,0x73,0x74,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x7f,

/*8*/ 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
/*9*/ 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
/*a*/ 0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
/*b*/ 0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
/*c*/ 0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
/*d*/ 0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
/*e*/ 0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,
/*f*/ 0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};


/*----------------------------------------------------------------------*
 *	convert alphanumeric yomi (kana) from EMORA to 7 bit code
 *----------------------------------------------------------------------*/
EMORAto7bit
(
uschar *mrudata		/* MRU area (7168 bytes) on memory		*/
)
{
	uschar	*last = mrudata + GETSHORT(mrudata);
	int	len;

	mrudata += 2;
	while(mrudata < last){

		/* process kana */
		len = *mrudata++;
		len--;
		if(*mrudata == (M_E_SHIFT | ESC_ALPHA)){
			*mrudata++ = ESC_ALPHA;
			for(len--; len > 0; len--){
				*mrudata++ = EMORAtoOLD[*mrudata];
			}
		}else{
			mrudata += len;
		}

		/* skip kanji */
		len = *mrudata;
		mrudata += len;
	}
	return 0;
}
