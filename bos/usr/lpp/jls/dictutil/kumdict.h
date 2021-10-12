/* @(#)kumdict.h	1.1  com/cmd/KJI/dictutil,3.1,9021 10/10/89 20:45:09 */

/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: header file
 *
 * ORIGINS: IBM
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kumdict.h
 *
 * DESCRIPTIVE NAME:    System Dictionary (Ver3.0) Information Table
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
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Macro.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         NA.
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

#define SDICT_NUM	16	/* System Dict Max Count	 */
#define SDICT_LEN	80	/* System Dict Max Length	 */

typedef struct mulsys MULSYS;
struct mulsys
{
    /*----------------------------------------------------------------------*
     *      ADDITION SYSTEM DICTIONARY INFORMATION 
     *----------------------------------------------------------------------*/
    uchar           frequency;		/* Frequency Information        */
    uchar           format;		/* Dictionary Format            */
    uchar           rec_length;		/* Record Length                */
    uchar           single_kanji;	/* Used Single Kanji Data       */
    uchar           dict_version;	/* System Dictionary Version    */
    ushort          record_size;	/* Record Size                  */

    short           dict_index_s;	/* Index Record Size            */
    short           dict_data_s;	/* Data  Record Size            */
    short           dict_info_p;	/* Dictionary information offset */
    short           dict_info_l;	/* Dictionary information lengrh */
    short           name_p;		/* Name Area offset             */
    short           name_l;		/* Name Area length             */
    short           mono_p;		/* Mono_E.mora offset           */
    short           mono_l;		/* Mono_E.mora length           */
    short           poly_p;		/* Poly_E.mora offset           */
    short           poly_l;		/* Poly_E.mora length           */
    short           add_info_l;		/* Additional info index lengrh */
    short           dict_mono_lkey;	/* Mono Lowest key  (Min E.Mora) */
    short           dict_mono_hkey;	/* Mono Highest key (Max E.Mora) */
    uchar           dict_mono_ex[32];	/* Mono E.Eora Exist Flag       */
    ushort          dict_poly_lkey;	/* Poly Lowest key  (Min E.Mora) */
    ushort          dict_poly_hkey;	/* Poly Highest Key (Max E.Mora) */
    uchar           dict_poly_ex[32];	/* Poly E.Eora Exist Flag       */
    short           dict_yml;		/* YOMI CODE Max length         */
    short           dict_sml;		/* SEISHO CODE Max length       */
    short           mora_sr;		/* Mora entry start record No.  */
    short           mora_dl;		/* Mora entry data length       */
    short           poly_sr;		/* Poly entry start record No.  */
    short           poly_dl;		/* Poly entry data length       */
    short           poly_ml;		/* Mora Length                  */
};
