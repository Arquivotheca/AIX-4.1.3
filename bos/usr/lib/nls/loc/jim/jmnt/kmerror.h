/* @(#)80	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kmerror.h, libKJI, bos411, 9428A410j 6/6/91 14:33:19 */

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
 * MODULE NAME:         kmerror.h
 *
 * DESCRIPTIVE NAME:    Kanji Monitor Error Code Values.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
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
 * CHANGE ACTIVITY:     Sept. 21 1988 Satoshi Higuchi
 *                      Added error code "IMFCVOVF".
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
/*
 *      Kanji Monitor Error Code.
 */
#ifndef _kj_kmerr
#define _kj_kmerr
#define KM_BIAS  (   -3000    )/* KM Error Code Bias.                     */
#define KM_BIASW (   -300     )/* KM Warning Code Bias.                   */
#define KMSUCC   (      0     )/* Kanji Monitor Suucessful of Execution.  */
/*
 *      Warning Level Error Code.
 */
#define KMCROTW  (KM_BIASW-0  )/* Invalid cursor position.(Warning)       */
#define KMIVKPDW (KM_BIASW-1  )/* Invalid parameter is exist in profile.  */
#define KMIVSUPW (KM_BIASW-2  )/* Value of profile is not supported.      */
/*
 *      Fatal Level Error Code.
 */
#define KMCROTE  (KM_BIAS-0   )/* Invalid cursor position.                */
#define KMIFACE  (KM_BIAS-1   )/* DBCS input field is already active.     */
#define KMIFLENE (KM_BIAS-2   )/* Invalid DBCS input field length.        */
#define KMIFNAE  (KM_BIAS-3   )/* DBCS input field is not active.         */
#define KMINITE  (KM_BIAS-4   )/* Appoppriate field is already active.    */
#define KMINPRE  (KM_BIAS-5   )/* DBCS input field is not active.         */
#define KMIVCPE  (KM_BIAS-6   )/* Invalid conversion position.            */
#define KMIVCURE (KM_BIAS-7   )/* Invalid cursor position.                */
#define KMIVKCME (KM_BIAS-8   )/* Invalid code is exist in kjcvmap.       */
#define KMIVPSNE (KM_BIAS-9   )/* Invalid path name for profile.          */
#define KMIVRIE  (KM_BIAS-10  )/* Invalid cursor position.                */
#define KMMALOCE (KM_BIAS-11  )/* Memory allocation error.                */
#define KMMDEALE (KM_BIAS-12  )/* Memory deallocation error.              */
#define KMMXSTCE (KM_BIAS-13  )/* Invalid maxstc.                         */
#define KMMXSTRE (KM_BIAS-14  )/* Invalid maxstr.                         */
#define KMNOTDBE (KM_BIAS-15  )/* Invalid input code.(DBCS code exception)*/
#define KMNPSCDE (KM_BIAS-16  )/* Invalid pseudo code.                    */
#define KMNSFTCE (KM_BIAS-17  )/* Invalid effective shift code.           */
#define KMPROFOE (KM_BIAS-18  )/* Profile(KMPF) open error.               */
#define KMSGLCHE (KM_BIAS-19  )/* Unsupproted shift mode.                 */
                               /* (Single byte character mode)            */
#define KMTERME  (KM_BIAS-20  )/* DBCS input field is inactive.           */
#define KMTRMALE (KM_BIAS-21  )/* Trace memory allocation error.          */
/*
 *      Diagnosis Trace Error.
 */
#define TR_BIAS  (   -5000    )/* TR Error Code Bias.                     */
#define TRSUCC   (TR_BIAS-0   )/* Successful.(Trace file)                 */
#define TRFCLE   (TR_BIAS-1   )/* Trace file close error.                 */
#define TRFOPE   (TR_BIAS-2   )/* Trace file open error.                  */
#define TRFWTE   (TR_BIAS-3   )/* Trace file writing error.               */
/*
 *      Kanji Monitor Internal Error.
 */
#define IMSUCC   (      0     )/* Sccessful. Kanji Monitor (Return code)  */
#define IMFAIL   (     -1     )/* Failure of Execution.                   */
#define IM_BIAS  (   -3500    )/* IM Error Code Bias.                     */
#define IM_BIASW (   -350     )/* IM Error Code Bias.                     */
/*
 *      Internal Warning Level Error Code.
 */
#define IMALPHW  (IM_BIASW-0  )/* Warning for shift1(KCB).(Shift1 is Alpha*/
                               /* /Numeric shift.)                        */
#define IMHIRAW  (IM_BIASW-1  )/* Warning for shift1(KCB).(Shift1 is      */
                               /* Hiragana shift.)                        */
#define IMINSTW  (IM_BIASW-2  )/* Warning for Insert/Replace mode.(Still  */
                               /* insert mode.)                           */
#define IMKATAW  (IM_BIASW-3  )/* Warning for Shift1(KCB). (Shift1 is     */
                               /* Katakana shift.)                        */
#define IMNOSAVW (IM_BIASW-4  )/* Save data is not exist.                 */
#define IMTRUSTW (IM_BIASW-5  )/* Display string has been truncated.      */
                               /* (Warning)                               */
/*
 *      Internal Fatal Level Error Code.
 */
#define IMDGVK   (IM_BIAS-0   )/* Valid key is input in case of Diagnostic*/
                               /* mode.                                   */
#define IMINSI   (IM_BIAS-1   )/* Information for kmreset.(Insert/Replace */
                               /* reset.)                                 */
#define IMIVCODE (IM_BIAS-2   )/* Invalid pseudo Code Error.              */
#define IMIVMSGE (IM_BIAS-3   )/* Invalid message type.                   */
#define IMKLRSTI (IM_BIAS-4   )/* Information of keyboard lock reset.     */
                               /* (Successful.)                           */
#define IMKTNOK  (IM_BIAS-5   )/* Return code for kmktnc.(Successful.)    */
#define IMMSGRSE (IM_BIAS-6   )/* Invalid pseudo code input.              */
                               /* (Not customized)                        */
#define IMMSGRSI (IM_BIAS-7   )/* Information of message reset(Successful)*/
#define IMNCVCHE (IM_BIAS-8   )/* There is no converted characters.       */
#define IMNKLRSE (IM_BIAS-9   )/* Keyboard lock status can not reset.     */
#define IMNOTIFE (IM_BIAS-10  )/* Insufficient input field length.        */
#define IMNOTRGE (IM_BIAS-11  )/* Profile parameter is not customized for */
                               /* Registration of dictionary.             */
#define IMNTOTHE (IM_BIAS-12  )/* Another message has been already        */
                               /* displaied.                              */
#define IMRGIVSE (IM_BIAS-13  )/* Invalid string for Dictionary           */
                               /* Registration.                           */
#define IMRGRSTI (IM_BIAS-14  )/* Information of Dictionary Registration  */
                               /* reset.                                  */
#define IMRGSTE  (IM_BIAS-15  )/* Invalid pseudo code input.              */
                               /* (Not customized.)                       */
#define IMRGSTI  (IM_BIAS-16  )/* Information of effective pseudo code    */
                               /* input.                                  */
#define IMRMRSE  (IM_BIAS-17  )/* Invalid key is input at Registration    */
                               /* Message Reset Process.                  */
#define IMRMRSTI (IM_BIAS-18  )/* Information of Registration Message     */
                               /* Reset .(Successful.)                    */
#define IMSDCVME (IM_BIAS-19  )/* Invalid cnvmd flag.                     */
#define IMSDVNLE (IM_BIAS-20  )/* Invalid length parameter.               */
#define IMTRUSTE (IM_BIAS-21  )/* Display string has been truncated.      */
#define IMNTNUME (IM_BIAS-22  )/* Input code is not number.               */
#define IMKJCVOE (IM_BIAS-23  )/* Kanji Conversion Map Overflow.          */
#define IMKJCVLE (IM_BIAS-24  )/* Invalid Kanji Convertion Map Length.    */
#define IMFCVOVF (IM_BIAS-25  )/* Kjdataf etc. overflow                   */
#endif
