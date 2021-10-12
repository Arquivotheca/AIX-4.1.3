/* @(#)61       1.4  src/bos/usr/lib/pios/fmtrs/piofpt.h, cmdpios, bos411, 9428A410j 2/22/90 06:01:50 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
*  piofpt.h   - Definitions for a Printer Formatter                            *
*******************************************************************************/



/*******************************************************************************
*                         Integer and String Variables                         *
*******************************************************************************/

/* Initialize the printer (for each print file)? */
#define Init_printer (*(_Init_printer + piomode))  /* variable name */
int *_Init_printer;   /* pointer to the variable */

/* Restore the Printer at the End of the Print Job? */
#define Restoreprinter (*(_Restoreprinter + piomode))  /* variable name */
int *_Restoreprinter;   /* pointer to the variable */

/* Paper Source Drawer (1: Upper   2: Lower    3: Envelope) */
#define Paper_source (*(_Paper_source + piomode))  /* variable name */
int *_Paper_source;   /* pointer to the variable */

/* Issue Form Feed Between Copies & At Job End (!: no;  +: yes) */
#define Do_formfeed (*(_Do_formfeed + piomode))  /* variable name */
int *_Do_formfeed;   /* pointer to the variable */



/*******************************************************************************
*                               String Constants                               *
*******************************************************************************/

/* Path Name of Font File To Be Downloaded (must include download commands) */
#define DOWNLOAD_FONT  "mF"

/* Smallest legal sheetfeeder drawer number */
#define LOWER          "wl"

/* Largest legal sheetfeeder drawer number */
#define UPPER          "wu"

/* Command To Initialize the Printer */
#define INIT_CMD       "ci"

/* Command To Restore the Printer at Job End */
#define REST_CMD       "cr"

/* ASCII Control Code to Advance the Paper to Top of Next Page (FF) */
#define FF_CMD         "af"



/*******************************************************************************
*                    Table of Variables and Their Attributes                   *
*******************************************************************************/

struct attrparms attrtable[] = {
                                                              /*
 name      data type       lookup table     addr of pointer
------   --------------   --------------   ----------------   */
"_j"   , VAR_INT        , NULL           , (union dtypes *) &_Init_printer   ,
"_u"   , VAR_INT        , NULL           , (union dtypes *) &_Paper_source   ,
"_J"   , VAR_INT        , NULL           , (union dtypes *) &_Restoreprinter ,
"_Z"   , VAR_INT        , NULL           , (union dtypes *) &_Do_formfeed    ,
NULL   , 0              , NULL           , NULL

};



/*******************************************************************************
*                                                                              *
*                   String of Options (Flags) for piogetopt()                  *
*                                                                              *
*   These are all the available formatter command line flags.  These flags     *
*   should be duplicated in the %f[] segment of the pipeline defined in the    *
*   formatter database.                                                        *
*                                                                              *
*******************************************************************************/

#define OPTSTRING  "j:u:J:Z:"

