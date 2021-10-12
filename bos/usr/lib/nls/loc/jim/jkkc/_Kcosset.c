static char sccsid[] = "@(#)78	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcosset.c, libKJI, bos411, 9428A410j 7/23/92 00:26:10";
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
 * MODULE NAME:       _Kcosset
 *
 * DESCRIPTIVE NAME:  SET OF SYSTEM DICTIONARY INFORMATION.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *
 ******************** END OF SPECIFICATIONS *****************************/

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

#define    Z_LC_VER     0x80
#define    Z_INF_OFFSET	0x82		/* Offset of Informatio area	*/
#define    Z_1K         1024

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcosset( z_mcbptr, z_sysnm, z_file )
struct MCB   *z_mcbptr;                 /* pointer of MCB               */
char         *z_sysnm;           	/* System Dict Name		*/
short	     z_file;			/* Multi System Dict Number	*/
{
/*----------------------------------------------------------------------*
 *    INCLUDE FILE                          
 *----------------------------------------------------------------------*/
#include "_Kcmcb.h"			/* Monitor Control Block (MCB)  */
#define _sys mcb.mulsys[z_file]

/*----------------------------------------------------------------------*
 *    DEFINITON OF LOCAL CONSTANTS          
 *----------------------------------------------------------------------*/
	uschar	*z_buf;			/* Read Data Buffer 		*/
	uschar	z_get[2];		/* Read Short Data  		*/
   	uschar  z_sver1[2];             /* system dictionary version 1  */
   	uschar  z_sver2[2];             /* system dictionary version 2  */
	short	z_i;			/* loop counter			*/
	short	z_dioff;		/* Dictionary Information Offset*/
	short   z_ioff;			/* Index Area Offset		*/
	short   z_moff;			/* Mono E.Mora Offset           */
	short	z_poff;			/* Poly E.Mora Offset           */
   	short   z_vcb;                  /* Version Check Bytes          */
	short	z_index_size;		/* Index size			*/

	/*--------------------------------------------------------------*
	 *    SET BASE POINTER                      
	 *--------------------------------------------------------------*/
	mcbptr1 = z_mcbptr;		/* Establish address'th to mcb  */

	/*--------------------------------------------------------------*
	 *    GET VERSION INFORMATION AERA DATA             
	 *--------------------------------------------------------------*/
        if( lseek( mcb.dsyfd[z_file], Z_LC_VER, 0 )  == -1 )
            return( SYS_LSEEK );

        if( read(  mcb.dsyfd[z_file], z_sver1 , 2 )  == -1 )
            return( SYS_READ );

        z_vcb = (( z_sver1[0] << 8 ) & 0xFF00 ) + ( z_sver1[1] & 0x00FF );
        mcb.mulsys[z_file].frequency    = (uschar)((z_vcb&0xF000) >> 12 );
        mcb.mulsys[z_file].format       = (uschar)((z_vcb&0x0F00) >>  8 );
        mcb.mulsys[z_file].rec_length   = (uschar)((z_vcb&0x00C0) >>  6 );
        mcb.mulsys[z_file].single_kanji = (uschar)((z_vcb&0x0010) >>  4 );
        mcb.mulsys[z_file].dict_version = (uschar)( z_vcb&0x000F);

	/*--------------------------------------------------------------*
 	 *      SET RECORD SIZE   
 	 *--------------------------------------------------------------*/
    	switch( mcb.mulsys[z_file].rec_length ) {
            case 0x00: mcb.mulsys[z_file].record_size = 1*Z_1K; break;
            case 0x01: mcb.mulsys[z_file].record_size = 2*Z_1K; break;
            case 0x02: mcb.mulsys[z_file].record_size = 4*Z_1K; break;
            case 0x03: mcb.mulsys[z_file].record_size = 8*Z_1K; break;
            defautl  : return( SYS_INCRR );
        }

	/*--------------------------------------------------------------*
	 *    GET INDEX RECORD SIZE 
	 *--------------------------------------------------------------*/
	if( lseek( mcb.dsyfd[z_file], Z_INF_OFFSET, 0 ) == -1 )
	        return( SYS_LSEEK );
	if( read(  mcb.dsyfd[z_file], z_sver2, 2 ) == -1 )
	        return( SYS_READ );
	z_index_size = (((z_sver2[1] << 8) & 0xFF00)+(z_sver2[0] & 0x00FF)) *
			mcb.mulsys[z_file].record_size; 

	/*--------------------------------------------------------------*
	 *    MALLOC OF INDEX SIZE
	 *--------------------------------------------------------------*/
	if(( z_buf = malloc( z_index_size )) == NULL )
		return( MEM_MALLOC );

	/*--------------------------------------------------------------*
	 *    GET INFORMATION AERA DATA             
	 *--------------------------------------------------------------*/
	if( lseek( mcb.dsyfd[z_file], 0, 0 ) == -1 ) {
		free( z_buf ); 
		return( SYS_LSEEK );
	}
	if( read(  mcb.dsyfd[z_file], z_buf, z_index_size ) == -1 ) {
		free( z_buf );
	        return( SYS_READ );
	}

	/*--------------------------------------------------------------*
	 *    SET INFORMATION AERA DATA             
	 *--------------------------------------------------------------*/
	z_ioff = Z_INF_OFFSET; 
	_sys.dict_index_s = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+ 0,z_ioff+ 1)); 
	_sys.dict_data_s  = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+ 2,z_ioff+ 3)); 
	_sys.dict_info_p  = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+ 4,z_ioff+ 5)); 
	_sys.dict_info_l  = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+ 6,z_ioff+ 7)); 
	_sys.name_p       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+ 8,z_ioff+ 9)); 
	_sys.name_l       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+10,z_ioff+11)); 
	_sys.mono_p       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+12,z_ioff+13)); 
	_sys.mono_l       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+14,z_ioff+15)); 
	_sys.poly_p       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+16,z_ioff+17)); 
	_sys.poly_l       = GETSHORT(strmcpyr(z_get,z_buf,z_ioff+18,z_ioff+19)); 

	z_dioff             = _sys.dict_info_p;
	_sys.add_info_l     = GETSHORT(strmcpyr(z_get,z_buf,z_dioff+0,z_dioff+1)); 
	_sys.dict_mono_lkey = ((short)z_buf[z_dioff+2] ) & 0xFFFF; 
	_sys.dict_mono_hkey = ((short)z_buf[z_dioff+3] ) & 0xFFFF; 
	strmcpy( _sys.dict_mono_ex, z_buf, z_dioff+4, z_dioff+35 );
	_sys.dict_poly_lkey = (short)(((z_buf[z_dioff+36]<< 8)&0xFF00)+
	                               (z_buf[z_dioff+37]&0xFF)); 
	_sys.dict_poly_hkey = (short)(((z_buf[z_dioff+38]<< 8)&0xFF00)+
				       (z_buf[z_dioff+39]&0xFF)); 
	strmcpy( _sys.dict_poly_ex, z_buf, z_dioff+40, z_dioff+71 );
	_sys.dict_yml       = ((short)z_buf[z_dioff+72] ) & 0xFFFF; 
	_sys.dict_sml       = ((short)z_buf[z_dioff+73] ) & 0xFFFF; 

	if(( _sys.name_p ) && ( _sys.name_l )) {
	    if(( _sys.name_l > 0 ) && ( _sys.name_l <= 0x20 ))
	        strmcpy( _sys.dict_name, z_buf, _sys.name_p, _sys.name_p + _sys.name_l );
	    else if( _sys.name_l > 0x20 )
	        strmcpy( _sys.dict_name, z_buf, _sys.name_p, _sys.name_p + 0x20 );
	}

	z_moff = _sys.mono_p;
	z_poff = _sys.poly_p;
	if( z_moff ) {
	    _sys.mono_sr = GETSHORT(strmcpyr(z_get,z_buf,z_moff+0,z_moff+1)); 
	    _sys.mono_dl = GETSHORT(strmcpyr(z_get,z_buf,z_moff+2,z_moff+3)); 
	}
	else {
	    _sys.mono_sr = _sys.mono_dl = 0; 
	}
	if( z_poff ) {
	    _sys.poly_sr = GETSHORT(strmcpyr(z_get,z_buf,z_poff+0,z_poff+1)); 
	    _sys.poly_dl = GETSHORT(strmcpyr(z_get,z_buf,z_poff+2,z_poff+3)); 
	    _sys.poly_ml = ((short)z_buf[z_poff+4] ) & 0xFFFF; 
	}
	else {
	    _sys.poly_sr = _sys.poly_dl = _sys.poly_ml = 0;
	}
	free( z_buf );
	return( SUCCESS );
}

strmcpy( file, org, start, end )
uschar	*file, *org;
short	start, end;
{
	int	z_i;

	z_i = start;
	while( z_i-- ) *org++;
	for( z_i = 0; z_i < ( end - start + 1 ); z_i++ )
	    file[z_i] = *org++;
}

strmcpyr( file, org, start, end )
uschar	*org, *file;
short	start, end;
{
	int	z_i;

	z_i = start;
	while( z_i-- ) *org++;
	strncpy( file, org, end - start + 1 );
	return( file );
}
