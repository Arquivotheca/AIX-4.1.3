/* @(#)75	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmsys.h, libKJI, bos411, 9428A410j 7/23/92 00:17:50	*/
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
 * MODULE NAME:       _Kcsdct.h
 *
 * DESCRIPTIVE NAME:  Multi System dictionary
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       
 *
 ******************** END OF SPECIFICATIONS *****************************/

#include "dict.h"

struct	MULSYS {
/*----------------------------------------------------------------------*
 *      ADDITION SYSTEM DICTIONARY INFORMATION 
 *----------------------------------------------------------------------*/
uschar          frequency;              /* Frequency Information        */
uschar          format;                 /* Dictionary Format            */
uschar          rec_length;             /* Record Length                */
uschar          single_kanji;           /* Used Single Kanji Data       */
uschar          dict_version;           /* System Dictionary Version    */
short           record_size;            /* Record Size                  */

short           dict_index_s;           /* Index Record Size            */
short           dict_data_s;            /* Data  Record Size            */
short           dict_info_p;            /* Dictionary information offset*/
short           dict_info_l;            /* Dictionary information length*/
short           name_p;                 /* Name Area offset             */
short           name_l;                 /* name Area length             */
short           mono_p;                 /* Mono_E.mora offset           */
short           mono_l;                 /* Mono_E.mora length           */
short           poly_p;                 /* Poly_E.mora offset           */
short           poly_l;                 /* Poly_E.mora length           */
short           add_info_l;             /* Additional info index lengrh */
short           dict_mono_lkey;         /* Mono Lowest key  (Min E.Mora)*/
short           dict_mono_hkey;         /* Mono Highest key (Max E.Mora)*/
uschar          dict_mono_ex[32];       /* Mono E.Eora Exist Flag       */
usshort         dict_poly_lkey;         /* Poly Lowest key  (Min E.Mora)*/
usshort         dict_poly_hkey;         /* Poly Highest Key (Max E.Mora)*/
uschar          dict_poly_ex[32];       /* Poly E.Eora Exist Flag       */
short           dict_yml;               /* YOMI CODE Max length         */
short           dict_sml;               /* SEISHO CODE Max length       */
uschar		dict_name[32];		/* Dictionary Name		*/
short           mono_sr;                /* Mono entry start record No.  */
short           mono_dl;                /* Mono entry data  length      */
short           poly_sr;                /* Poly entry start record No.  */
short           poly_dl;                /* poly entry data  length      */
short           poly_ml;                /* Mora Length                  */
};
