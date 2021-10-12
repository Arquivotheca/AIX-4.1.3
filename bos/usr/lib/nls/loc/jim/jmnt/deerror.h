/* @(#)60	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/deerror.h, libKJI, bos411, 9428A410j 6/6/91 14:28:18 */

/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         deerror.h
 *
 * DESCRIPTIVE NAME:    DBCS Editor Error Code Values.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
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

#ifndef _kj_deerr
#define _kj_deerr
#define DESUCC   (      0     )/* Successful of Exectution.               */
                               /* (DBCS Editor Normal Return).            */
#define DE_BIAS  (   -1000    )/* DBCS Editor Error Code Bias.            */
#define DE_BIASW (   -100     )/* DBCS Editor Warning Code Bias.          */
/*
 *      Warning Level Error Code.
 */
#define DEEVCNTW (DE_BIASW-0  )/* Event continuation.                     */
#define DEEVIQW  (DE_BIASW-1  )/* Event Continuation Queue is Empty.      */
#define DEFLEVW  (DE_BIASW-2  )/* Event Continuation Queue is Empty.      */
#define DEIVFNCW (DE_BIASW-3  )/* Invalid Function Code.                  */
#define DEIVSTLW (DE_BIASW-4  )/* Invalid Length of the Initial String.   */
#define DEIVRIW  (DE_BIASW-5  )/* Invalid Cursor Operation Mode.          */
/*
 *      Fatal   Level Error Code.
 */
#define DEIFACE  (DE_BIAS-0   )/* DBCS Input Field is Already Active.     */
#define DEIFDFE  (DE_BIAS-1   )/* DBCS Input Field has been Already       */
                               /* Defined.                                */
#define DEIFNAE  (DE_BIAS-2   )/* DBCS Input Field is not active.         */
#define DEIFNDE  (DE_BIAS-3   )/* The Specified Field has not been        */
                               /* Defined.                                */
#define DEIVBCE  (DE_BIAS-4   )/* Invalid Background Color Index.         */
#define DEIVCNVE (DE_BIAS-5   )/* Invalid Conversion Mode.                */
#define DEIVCRSE (DE_BIAS-6   )/* Invalid Cursor Position.                */
#define DEIVDCPE (DE_BIAS-7   )/* Invalid DBCS Editor Pointer.            */
#define DEIVDSPE (DE_BIAS-8   )/* Invalid Display-Adapter.                */
#define DEIVDTLE (DE_BIAS-9   )/* Invalid Data Length.                    */
#define DEIVFLDE (DE_BIAS-10  )/* Invalid File Descripter.                */
#define DEIVFLNE (DE_BIAS-11  )/* Invalid Field No.                       */
#define DEIVFNTE (DE_BIAS-12  )/* Invalid Font No.                        */
#define DEIVFTPE (DE_BIAS-13  )/* Invalid Font Type.                      */
#define DEIVMIXE (DE_BIAS-14  )/* Invalid MIX Mode.                       */
#define DEIVMXFE (DE_BIAS-15  )/* Invalid Maxmum Number of Fields.        */
#define DEIVTCE  (DE_BIAS-16  )/* Invalid Text Color Index.               */
#define DEIVXYE  (DE_BIAS-17  )/* Invalid Coordinate.                     */
#define DEMALOCE (DE_BIAS-18  )/* Memory Allocation Error.                */
#define DENPSCE  (DE_BIAS-19  )/* Invalid Pseudo Code Type.               */
#define DEPRFACE (DE_BIAS-20  )/* Profile Access Error.                   */
#define DERSFTE  (DE_BIAS-21  )/* Reset Font Error.                       */
#define DERSIFE  (DE_BIAS-22  )/* The Specified Field has not been Defined*/
#define DEFNTLDE (DE_BIAS-23  )/* Invalid Font Loading Error.             */
#define DEIVRCE  (DE_BIAS-24  )/* Invalid Reverse Color Index.            */
#define DEIVEVDE (DE_BIAS-25  )/* Invalid Event Data Input Mode.          */
#define DEIVTYPE (DE_BIAS-26  )/* Invalid Pseudo Code Type.               */
#define DEIVSTRE (DE_BIAS-27  )/* Invalid String Length(Too Long).        */
#define DEIVCHRE (DE_BIAS-28  )/* Invalid Data Contains in String.        */
#define DEIVDURE (DE_BIAS-29  )/* Invalid the beep Duration.              */
#define DEIVFRQE (DE_BIAS-30  )/* Invalid the beep Frequency.             */
#define DEIVFPNE (DE_BIAS-31  )/* Invalid Font File Path Name.            */
#define DEIVFPLE (DE_BIAS-32  )/* Invalid Length of Font File Path Name.  */
#define DEFNTSGE (DE_BIAS-33  )/* Invalid Number of Segment Number.       */
#define DEFNTSZE (DE_BIAS-34  )/* Invalid Font Height or Width.           */
#define DERSSHE  (DE_BIAS-35  )/* DECB input field is not active.         */

/*
 *      DBCS Editor GSL Error Return Code.
 *
 */
#define DE_BASE  (  GS_BASE   )/* GSL Error Number Offset.                */
#endif
