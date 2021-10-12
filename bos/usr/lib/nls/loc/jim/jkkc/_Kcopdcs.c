static char sccsid[] = "@(#)71	1.2.2.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopdcs.c, libKJI, bos411, 9428A410j 5/18/93 22:55:34";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcopdcs
 *
 * DESCRIPTIVE NAME:  OPEN SYSTEM DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x0108(WSPOVER):   overflow of work space buffer
 *                    0x0181(SYS_OPEN):  error of open()
 *                    0x0281(SYS_CLOSE): error of close()
 *                    0x0981(SYS_SHMAT): error of shmat()
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */
#include   "dict.h"
#include   <unistd.h>

/************************************************************************ 
 *      INCLUDE FILES
 ************************************************************************/
#include <sys/types.h>                  /* define for lockf()           */
#include <sys/ipc.h>                    /* define for shmat()           */
#include <sys/shm.h>                    /* define for shmat()           */
#include <fcntl.h>                      /* define for shmat()           */
#include <sys/stat.h>                   /* define for fstat()           */

/************************************************************************ 
 *      DEFINITON OF LOCAL CONSTANTS
 ************************************************************************/
#define   Z_LC_VER     0x80
#define   Z_EXLEN        20
#define   Z_1K         1024
#define   Z_PKK_FORMAT 0x00
#define   Z_MKK_FORMAT 0x01
#define   Z_EMT_FORMAT 0x02
#define	  Z_SHMAT_CNT	  1

/************************************************************************
 *      SET THE CURRENT SDICTDATA
 ************************************************************************/
static          SDICTDATA       sdictp;
void            *SetCurrentSDICTDATA(SDICTDATA sp)  { sdictp = sp;    }
SDICTDATA       GetCurrentSDICTDATA()               { return(sdictp); }

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcopdcs( z_mcbptr, z_sysnm, z_file )
struct MCB   *z_mcbptr;                 /* pointer of MCB               */
char         *z_sysnm;           	/* System Dict Name		*/
short	     z_file;			/* Multi System Dict Number  	*/
{
/*----------------------------------------------------------------------* 
 *      DEFINITON OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
    char         *z_fname;              /* return value from getenv()   */
    short        z_sset;                /* return value from _Kcosset() */
    short	 z_ret;			/* return value			*/
    short	 z_first= ON;		/* First Open 			*/
    off_t	 z_sysize = 0;		/* System dict size		*/
    SDICTDATA    GetCurrentSDICTDATA(); /* Get current Sdictdata        */
    SDICTDATA    sdictdatap;            /* System dict information      */
    SDICTINFO    sdictp;                /* System dict information      */

/*----------------------------------------------------------------------* 
 *      Check whether the dictionary is already opened or not
 *----------------------------------------------------------------------*/
    sdictdatap = GetCurrentSDICTDATA();
    sdictp     = &(sdictdatap->sdictinfo[z_file]);

/*----------------------------------------------------------------------* 
 *      SET BASE POINTER
 *----------------------------------------------------------------------*/
    mcbptr1 = z_mcbptr;                 /* establish address'th to mcb  */
    if( sdictp->dsyfd >= 0 )
        z_first = OFF;

/*----------------------------------------------------------------------* 
 *      OPEN THE FILE REQUESTED BY KJ-MONITOR
 *----------------------------------------------------------------------*/
    if( z_first == ON ) { 		/*  FIRST IMOBJ 		*/
        z_fname = NULL;
        if(( mcb.dsyfd[z_file] = OpenFile(z_sysnm, O_RDONLY)) != -1) 
            z_fname  = z_sysnm;
        else
            return( SYS_OPEN );
        sdictp->dsyfd = mcb.dsyfd[z_file];

       /*---------------------------------------------------------------*
 	*      CHECK SYSTEM DICTIONARY
 	*---------------------------------------------------------------*/
        if(( z_ret = chk_sysdict(mcb.dsyfd[z_file], &z_sysize )) != SUCCESS )
	    return( z_ret );
    }
    else {				/*   SECOND IMOBJ 		*/
        mcb.dsyfd[z_file] = sdictp->dsyfd;
    }

/*----------------------------------------------------------------------* 
 *  SET SYSTEM DICTIONARY INFORMATION 
 *----------------------------------------------------------------------*/
    if(( z_sset = _Kcosset( z_mcbptr, z_sysnm, z_file )) != SUCCESS )
        return( z_sset );

/*----------------------------------------------------------------------* 
 *  ATTACH SHARED MEMORY SEGMENT          
 *----------------------------------------------------------------------*/
    if(( z_ret = sys_attach(z_mcbptr,z_sysnm,z_file,z_sysize,z_first)) != SUCCESS )
         return( z_ret );

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS ); 
}

chk_sysdict( fd, z_sysize )
int	fd;				/* System File descriptor	*/
off_t	*z_sysize;			/* Sysrem Dict Size		*/
{
    short        z_i, z_j, z_rec;       /* Loop counter                 */
    short        z_vcb;                 /* Version Check Bytes          */
    uschar       z_sver1[2];            /* System dict version 1  	*/
    uschar       z_sver2[2];            /* System dict version 2  	*/
    short	 z_record;		/* System dict record size	*/
    short	 z_format;		/* System dict format 		*/
    short	 z_length;		/* System dicr record length	*/
    struct stat  z_buf;                 /* infomation of fstat          */

/*----------------------------------------------------------------------* 
 *  GET VCB (Version Check Bytes)
 *----------------------------------------------------------------------*/
    if( lseek( fd, Z_LC_VER, 0 )  == -1 )
        return( SYS_LSEEK );
    if( read(  fd, z_sver1 , 2 )  == -1 )
        return( SYS_READ );

    z_vcb    = (( z_sver1[0] << 8 ) & 0xFF00 ) + ( z_sver1[1] & 0x00FF );
    z_format = (uchar)(( z_vcb & 0x0F00 ) >>  8 );
    z_length = (uchar)(( z_vcb & 0x00C0 ) >>  6 );

/*----------------------------------------------------------------------* 
 *  GET RECORD SIZE   
 *----------------------------------------------------------------------*/
    switch( z_length ) {
        case 0x00: z_record = 1*Z_1K; break;
        case 0x01: z_record = 2*Z_1K; break;
        case 0x02: z_record = 4*Z_1K; break;
        case 0x03: z_record = 8*Z_1K; break;
        defautl  : return( SYS_INCRR );
    }
/*----------------------------------------------------------------------* 
 *  GET FILE STATUS   
 *----------------------------------------------------------------------*/
    if( fstat( fd , &z_buf ) == -1 )
       return ( SYS_FSTAT );

/*----------------------------------------------------------------------* 
 *  CHECK THE SIZE WHETHER IT IS A MULTIPLE OF 2 KBYTE
 *----------------------------------------------------------------------*/
    if(((z_buf.st_size % z_record) != 0) || (z_buf.st_size < (z_record*2)))
        return( SYS_INCRR );

/*----------------------------------------------------------------------* 
 *  CHECK SYTEM DICTIONARY FORMAT ( MKK EMT Format )
 *----------------------------------------------------------------------*/
    if( z_format != Z_EMT_FORMAT )  
        return( SYS_INCRR );

/*----------------------------------------------------------------------* 
 *  CHECK THE VERSION NUMBER ON EACH BLOCK 
 *----------------------------------------------------------------------*/ 
    for(z_i=1; z_i < (z_buf.st_size / z_record); z_i+=REC_CONT) { 
	if( lseek( fd, z_i * z_record,0) == -1 ) return( SYS_LSEEK );
	if(  read( fd, z_sver2, 2 )      == -1 ) return( SYS_READ );
	if( CHPTTOSH( z_sver1 ) != CHPTTOSH( z_sver2 )) {
	    if( z_i == 1 )		/* Check Record Continue 	*/
                return( SYS_INCRR );	
	    else {
	        for(z_j=1, z_rec=z_i-1; z_j < REC_CONT; z_j++, z_rec--) {
	            if( lseek(fd,(z_rec*z_record),SEEK_SET) == -1 )
	                return( SYS_LSEEK );
	            if( read( fd, z_sver2, 2 ) == -1 )
	                return( SYS_READ );
      	            if( CHPTTOSH( z_sver1 ) == CHPTTOSH( z_sver2 )) 
	                break;
	            else
	                continue;
	        }
                if( z_j < REC_CONT )
	           continue;
	        else
	           return( SYS_INCRR );/* Check Record Continue 	*/
	       }
	}
        else
	    continue;
    }
    *z_sysize = z_buf.st_size;
/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
    return( SUCCESS );
}

sys_attach( z_mcbptr, z_sysnm, z_file, z_sysize, z_first )
struct MCB   *z_mcbptr;                 /* pointer of MCB               */
char         *z_sysnm;                  /* System dict name		*/
short	     z_file;			/* No. of Sdictinfo structure   */
off_t	     z_sysize;			/* System dict size		*/
short	     z_first;			/* First imobj or		*/
{
/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short _Kccldcs();

/*----------------------------------------------------------------------* 
 *      DEFINITON OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_length;               /* set length of path name      */
   short        z_rcldcs;		/* return calue from _Kccldcs() */
   short        z_shmcnt;               /* Count of shmat() Function    */
   SDICTDATA    GetCurrentSDICTDATA();  /* get current Sdictdata        */
   SDICTDATA    sdictdatap;             /* system dict information      */
   SDICTINFO    sdictp;                 /* system dict information      */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTER
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'th to mcb  */

/*----------------------------------------------------------------------* 
 *      Check whether the dictionary is already opened or not
 *----------------------------------------------------------------------*/
    sdictdatap = GetCurrentSDICTDATA();
    sdictp     = &(sdictdatap->sdictinfo[z_file]);

/*----------------------------------------------------------------------* 
 *      ATTACH SHARED MEMORY SEGMENT
 *----------------------------------------------------------------------*/
   if( z_first == ON ) {	/************  First imobj   ************/
       if(((z_shmcnt=sdictdatap->shmatcnt) < 0) || (z_shmcnt > SDICT_NUM))
           z_shmcnt = Z_SHMAT_CNT;

       if( z_file < z_shmcnt ) {
           if((long)( mcb.dsyseg[z_file] = 
	       shmat( mcb.dsyfd[z_file],0,(SHM_MAP | SHM_RDONLY)))!=-1) {
               mcb.sxesxe[z_file] = NULL;
           }
       }
       if(( z_file >= z_shmcnt ) || ((long)(mcb.dsyseg[z_file]) == -1 )) {
           if((mcb.dsyseg[z_file] = (uschar *)malloc(z_sysize)) == NULL) {
               if(( z_rcldcs = _Kccldcs( z_mcbptr )) != SUCCESS ) {
                   return( SYS_CLOSE );
	       }
               return( MEM_MALLOC );
           }
           mcb.sxesxe[z_file] = (struct SXE *)mcb.dsyseg[z_file]; 
           sdictp->sxesxe = (char *)mcb.sxesxe[z_file];
           if ( lseek( mcb.dsyfd[z_file], 0, 0 )  == -1 )
              return( SYS_LSEEK );
           if( read(mcb.dsyfd[z_file],mcb.dsyseg[z_file],z_sysize) == -1) {
               if(( z_rcldcs = _Kccldcs( z_mcbptr )) != SUCCESS ) {
                   return( SYS_CLOSE );
	       }
               return( SYS_READ );
           }
       }
       sdictp->dsyseg = mcb.dsyseg[z_file];
       if(( sdictp->sdictname = (char *)malloc(strlen(z_sysnm) + 1)) != NULL)
           strcpy( sdictp->sdictname, z_sysnm );
    }
    else {			/***********  Second imobj   ************/
       mcb.dsyfd[z_file]  = sdictp->dsyfd;
       mcb.dsyseg[z_file] = sdictp->dsyseg;
       mcb.sxesxe[z_file] = (struct SXE *)sdictp->sxesxe;
    }

/*----------------------------------------------------------------------* 
 *      SET INFORMATION OF SYSTEM DICTIONARY
 *----------------------------------------------------------------------*/
   mcb.dsynm[z_file] = NULL;
   mcb.dsynmll[z_file] = 0;

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS ); 
}
