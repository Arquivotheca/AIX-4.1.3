static char sccsid[] = "@(#)62	1.1  src/bos/usr/lpp/jls/dictutil/kumdinfo.c, cmdKJI, bos411, 9428A410j 7/22/92 23:32:56";
/*
 * COMPONENT_NAME: User Dictionary Utility 
 *
 * FUNCTIONS: rd_sdic, set_sdic, setinfo, strmcpy, strmcpyr
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kumdinfo.c
 *
 * DESCRIPTIVE NAME:    System Dictionary Information Set and
 *                      Dictionary Data Load
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            rd_sdic set_sdic set_info strmcpy strmcpyr
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         rd_sdic set_sdic
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/* include Standard. */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

/* include Kanji Project. */
#include "kut.h"
#include "kumdict.h"

/* Copyright Identify. */
static char    *cprt1 = "5601-125 COPYRIGHT IBM CORP 1989           ";
static char    *cprt2 = "LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";


/* Definiton of Local Constants */
#define   Z_EXLEN        20
#define   Z_LC_VER   0x0080
#define   Z_1K         1024
#define CHPTTOSH(adr) \
( (((unsigned char *)(adr))[0]<<8) +((unsigned char *)(adr))[1])
#define  ENVLIM		80
#define	 REC_CONT	16	/* Skip Record Number            */
#define  Z_INF_OFFSET	0x82	/* Offset of Informatio area	 */
#define  Z_INF_SIZE  	0x74	/* Size of Informatio area	 */



rd_sdic( mora_len, z_mcbptr, sdcbpt )

ushort 	mora_len;
MULSYS 	*z_mcbptr;	/* pointer of MCB               		*/
SDCB    *sdcbpt;        /* Pointer to System Dictionry                  */
{
    char	   *dcptr;	/* the image of system dictionary	*/
    unsigned long   dict_size;
    unsigned long   dict_entry;

    dcptr = sdcbpt->dcptr;
    if ( dcptr == NULL )
	return (-1);

    if ( mora_len == 1 ) {
	dict_size = z_mcbptr->mora_dl;
	dict_entry = z_mcbptr->record_size * z_mcbptr->mora_sr;
    }
    else {
	dict_size = z_mcbptr->poly_dl * z_mcbptr->record_size;
	dict_entry = z_mcbptr->record_size * z_mcbptr->poly_sr;
    }

    sdcbpt->rdptr = &dcptr[dict_entry];

    return (0);
}



set_sdic( z_mcbptr, sdcbpt )

MULSYS 	*z_mcbptr;	/* pointer of MCB               		*/
SDCB    *sdcbpt;        /* Pointer to System Dictionry                  */
{
    char	   *dcptr;	/* the image of system dictionary	*/
    char           *z_envnm;	/* return value from getenv()   	*/
    char            z_fnmwk[ENVLIM + Z_EXLEN];	/* temporaly work area  */
    short           z_length;	/* set length of path name      	*/
    short           z_rcldcs;
    short           z_i, z_j, z_rec;	/* loop counter                 */
    uchar           z_sver1[2];	/* system dictionary version 1  	*/
    uchar           z_sver2[2];	/* system dictionary version 2  	*/
    struct stat     z_buf;	/* infomation of fstat          	*/
    short           z_vcb;	/* Version Check Bytes		 	*/
    short           z_sset;	/* return value from setinfo() 		*/

    dcptr = sdcbpt->dcptr;
    if ( dcptr == NULL ) {
	return (-1);
    }

    /*------------------------------------------------------------------*
     *      SET VCB INFORMATION
     *------------------------------------------------------------------*/
    /*----- get VCB in INDEX area --------------------------------------*/
    memcpy( &z_sver1, &dcptr[Z_LC_VER], 2);

    /*----- copy VCB from char type area to short type area ------------*/
    z_vcb = ((z_sver1[0] << 8) & 0xFF00) + (z_sver1[1] & 0x00FF);

    /*----- get FREQUENCY ----------------------------------------------*/
    z_mcbptr->frequency = (uchar) ((z_vcb & 0xF000) >> 12);

    /*----- get FORMAT -------------------------------------------------*/
    z_mcbptr->format = (uchar) ((z_vcb & 0x0F00) >> 8);

    /*----- get RECORD LENGTH ------------------------------------------*/
    z_mcbptr->rec_length = (uchar) ((z_vcb & 0x00C0) >> 6);

    /*----- get SINGLE-KANJI DICTIONARY --------------------------------*/
    z_mcbptr->single_kanji = (uchar) ((z_vcb & 0x0010) >> 4);

    /*----- get DICTIONARY VERSION -------------------------------------*/
    z_mcbptr->dict_version = (uchar) (z_vcb & 0x000F);

    /*------------------------------------------------------------------*
     *      SET RECORD SIZE   
     *------------------------------------------------------------------*/
    switch ( z_mcbptr->rec_length ) {
    case 0x00: z_mcbptr->record_size = 1 * Z_1K; break;
    case 0x01: z_mcbptr->record_size = 2 * Z_1K; break;
    case 0x02: z_mcbptr->record_size = 4 * Z_1K; break;
    case 0x03: z_mcbptr->record_size = 8 * Z_1K; break;
    default:   return (-1);
    }

    /*------------------------------------------------------------------*
     *      CHECK THE SIZE WHETHER IT IS A MULTIPLE OF 2 KBYTE
     *------------------------------------------------------------------*/
    if (((sdcbpt->st_size % z_mcbptr->record_size) != 0) ||
	(sdcbpt->st_size < (z_mcbptr->record_size * 2))) {
	return (-1);
    }

    /*------------------------------------------------------------------*
     *      CHECK SYTEM DICTIONARY FORMAT ( MKK EMT Format )
     *------------------------------------------------------------------*/
    if ( z_mcbptr->format != 0x02 ) {
	return (-1);
    }

    /*------------------------------------------------------------------*
     *      CHECK THE VERSION NUMBER ON EACH BLOCK
     *------------------------------------------------------------------*/
    for ( z_i = 1; z_i < (sdcbpt->st_size / z_mcbptr->record_size);
		z_i += REC_CONT) {

	/*----- read version number ------------------------------------*/
    	memcpy( &z_sver2, &dcptr[(z_i * z_mcbptr->record_size)], 2);

	/*----- compare version number ---------------------------------*/
	if ( CHPTTOSH(z_sver1) != CHPTTOSH(z_sver2) )
	    return (-1);
	else
	    continue;
    }
    /*------------------------------------------------------------------*
     *      SET SYSTEM DICTIONARY INFORMATION 
     *------------------------------------------------------------------*/
    if ((z_sset = setinfo( z_mcbptr, sdcbpt )) != 0) {
	return ( z_sset );
    }

    /*------------------------------------------------------------------* 
     *      RETURN
     *------------------------------------------------------------------*/
    return (0);
}



setinfo( z_mcbptr, sdcbpt )

MULSYS	*z_mcbptr;	/* pointer of MCB               		*/
SDCB    *sdcbpt;        /* Pointer to System Dictionry                  */
{
    char       *dcptr;		/* the image of system dictionary	*/
    uchar      *z_buf;			/* Read Data Buffer 		*/
    uchar       z_get[2];		/* Read Short Data  		*/
    short       z_i;
    uchar  	z_sver2[2];             /* system dictionary version 2  */
    uchar      *strmcpyr();
    short   	z_index_size;           /* Index size                   */

    dcptr = sdcbpt->dcptr;
    if ( dcptr == NULL )
	return (-1);

    /*------------------------------------------------------------------*
     *    GET INDEX RECORD SIZE
     *------------------------------------------------------------------*/
    memcpy( z_sver2, &dcptr[130], 2 );
    z_index_size = (((z_sver2[1] << 8) & 0xFF00)+(z_sver2[0] & 0x00FF)) *
                    z_mcbptr->record_size;

    /*------------------------------------------------------------------*
     *    MALLOC OF INDEX SIZE
     *------------------------------------------------------------------*/
    if(( z_buf = malloc( z_index_size )) == NULL )
            return( -1 );

    /*------------------------------------------------------------------*
     *    GET INFORMATION AERA DATA             
     *------------------------------------------------------------------*/
    memcpy( z_buf, dcptr, z_index_size );
    z_i = Z_INF_OFFSET;

    z_mcbptr->dict_index_s = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 0, z_i+ 1));
    z_mcbptr->dict_data_s  = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 2, z_i+ 3));
    z_mcbptr->dict_info_p  = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 4, z_i+ 5));
    z_mcbptr->dict_info_l  = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 6, z_i+ 7));
    z_mcbptr->name_p       = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 8, z_i+ 9));
    z_mcbptr->name_l       = GETSHORT(strmcpyr(z_get, z_buf, z_i+10, z_i+11));
    z_mcbptr->mono_p       = GETSHORT(strmcpyr(z_get, z_buf, z_i+12, z_i+13));
    z_mcbptr->mono_l       = GETSHORT(strmcpyr(z_get, z_buf, z_i+14, z_i+15));
    z_mcbptr->poly_p       = GETSHORT(strmcpyr(z_get, z_buf, z_i+16, z_i+17));
    z_mcbptr->poly_l       = GETSHORT(strmcpyr(z_get, z_buf, z_i+18, z_i+19));

    z_i = z_mcbptr->dict_info_p;
    z_mcbptr->add_info_l   = GETSHORT(strmcpyr(z_get, z_buf, z_i+ 0, z_i+ 1));

    z_mcbptr->dict_mono_lkey = ((short) z_buf[z_i+2]) & 0xFFFF;
    z_mcbptr->dict_mono_hkey = ((short) z_buf[z_i+3]) & 0xFFFF;

    strmcpy(z_mcbptr->dict_mono_ex, z_buf, z_i+4, z_i+35);

    z_mcbptr->dict_poly_lkey = (short) (((z_buf[z_i+36] << 8) & 0xFF00)
					+ (z_buf[z_i+37] & 0xFF));
    z_mcbptr->dict_poly_hkey = (short) (((z_buf[z_i+38] << 8) & 0xFF00)
					+ (z_buf[z_i+39] & 0xFF));

    strmcpy(z_mcbptr->dict_poly_ex, z_buf, z_i+40, z_i+71);
    z_mcbptr->dict_yml = ((short) z_buf[z_i+72]) & 0xFFFF;
    z_mcbptr->dict_sml = ((short) z_buf[z_i+73]) & 0xFFFF;

    z_i = z_mcbptr->mono_p;
    z_mcbptr->mora_sr = GETSHORT(strmcpyr(z_get, z_buf, z_i+0, z_i+1));
    z_mcbptr->mora_dl = GETSHORT(strmcpyr(z_get, z_buf, z_i+2, z_i+3));

    z_i = z_mcbptr->poly_p;
    z_mcbptr->poly_sr = GETSHORT(strmcpyr(z_get, z_buf, z_i+0, z_i+1));
    z_mcbptr->poly_dl = GETSHORT(strmcpyr(z_get, z_buf, z_i+2, z_i+3));
    z_mcbptr->poly_ml = ((short) z_buf[z_i+4]) & 0xFFFF;

    free( z_buf );
    return (0);
}

strmcpy(file, org, start, end)
    uchar          *file, *org;
    short           start, end;
{
    int             z_i;

    z_i = start;
    while (z_i--)
	*org++;
    for (z_i = 0; z_i < (end - start + 1); z_i++)
	file[z_i] = *org++;
}

uchar *
strmcpyr(file, org, start, end)
    uchar          *org, *file;
    short           start, end;
{
    int             z_i;

    z_i = start;
    while (z_i--)
	*org++;
    strncpy(file, org, end - start + 1);
    return (file);
}
