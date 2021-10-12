/* @(#)72	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/kjextern.h, libKJI, bos411, 9428A410j 6/6/91 14:31:35 */

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
 * MODULE NAME:         kjextern.h
 *
 * DESCRIPTIVE NAME:    Defines Kanji Project External Reference
 *                      Linkage.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
 *                      DBCS  Monitor V1.0
 *                      Kanji Monitor V1.0
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
#ifndef lint
extern  int     deacif_ ();     /* Activate Input Field.                  */
extern  int     declos_ ();     /* Close DBCS Editor.                     */
extern  int     dedaif_ ();     /* Deactive Input Field.                  */
extern  int     dedfif_ ();     /* Define Input Field.                    */
extern  int     deerif_ ();     /* Erase Input Field.                     */
extern  int     deeviq_ ();     /* Event Inquier.                         */
extern  int     deevwt_ ();     /* Event Wait.                            */
extern  int     deflev_ ();     /* Flash Event.                           */
extern  int     deopen_ ();     /* DBCS Editor Open.                      */
extern  int     derdrw_ ();     /* Redraw Input Field.                    */
extern  int     dersif_ ();     /* Reset Input Field Shift Status.        */
extern  int     derssh_ ();     /* Reset Shift Status.                    */
extern  int     desmpl_ ();     /* Sample DBCS Input Data.                */
extern  int     destrk_ ();     /* Shift Status Tracking.                 */
extern  int     desvft_ ();     /* Save Font Attribute Information.       */
extern  int     desvlg_ ();     /* Save Logical Attribute Information.    */
extern  int     desvln_ ();     /* Save Line Attribute Information.       */
extern  int     destbp_ ();     /* Beep Frequency/Time Change.            */

extern  int     _Eax1ap ();     /* Auxiliary Area No.1 Create/Delete      */
extern  int     _Eax1cr ();     /* Auxiliary Area No.1 Cursor Display     */
extern  int     _Eax1ec ();     /* Auxiliary Area No.1 Redraw.            */
extern  int     _Eax2ap ();     /* Auxiliary Area No.2 Create/Delete      */
extern  int     _Eax2cr ();     /* Auxiliary Area No.2 Cursor Display     */
extern  int     _Eax2ec ();     /* Auxiliary Area No.2 Redraw.            */
extern  int     _Eax3ap ();     /* Auxiliary Area No.3 Create/Delete      */
extern  int     _Eax3cr ();     /* Auxiliary Area No.3 Cursor Display     */
extern  int     _Eax3ec ();     /* Auxiliary Area No.3 Redraw.            */
extern  int     _Eax4ap ();     /* Auxiliary Area No.4 Create/Delete      */
extern  int     _Eax4cr ();     /* Auxiliary Area No.4 Cursor Display     */
extern  int     _Eax4ec ();     /* Auxiliary Area No.4 Redraw.            */
extern  int     _Eaxap  ();     /* Auxiliary Area Generate/Delete.        */
extern  int     _Eaxec  ();     /* Auxiliary Area Redraw.                 */
extern  int     _Ebeep  ();     /* Caution Bell.                          */
extern  int     _Ecmd   ();     /* Execute Trigger Command.               */
extern  void    _Econv  ();     /* Psuedo Code Conversion.                */
extern  int     _Ecur   ();     /* Cursor Echo & Move.                    */
extern  int     _Eecac  ();     /* Input Field Redraw.                    */
extern  int     _Eeccr  ();     /* Cursor Echo.                           */
extern  int     _Eecho  ();     /* Echo Input Field.                      */
extern  int     _Eerfd  ();     /* Erase Specified Field.                 */
extern  int     _Eerrgsl();     /* GSL Error Interface.                   */
extern  int     _Eloadft();     /* Load Geometric Text.                   */
extern  int     _Equery ();     /* Query Hardware Information.            */
extern  int     _Ersft  ();     /* Recover User Environment.              */
extern  int     _Esetcol();     /* Set Geometric Text Color.              */

extern  int     _Jclos  ();     /* Close Kanji Monitor.                   */
extern  int     _Jclr   ();     /* Kanji Internal Work Area Clear.        */
extern  int     _Jcrst  ();     /* Cursor Force Set.                      */
extern  int     _Jinit  ();     /* Input Field Initialize.                */
extern  int     _Jinpr  ();     /* Data Input Processing.                 */
extern  int     _Jopen  ();     /* Kanji Monitor Open.                    */
extern  int     _Jshrs  ();     /* Forced Shift Status Reset.             */
extern  int     _Jterm  ();     /* Input Field Terminate.                 */
extern  int     _Jtrak  ();     /* Shift Status Tracking.                 */

extern  int     _MCN_rs ();     /* UnConversional Indicator Reset.        */
extern  int     _MC_rtn ();     /* Cursor Control  Routine.               */
extern  int     _MD_rtn ();     /* Code Conversion Routine.               */
extern  int     _ME_01  ();     /* First Input Mode Processing.           */
extern  int     _ME_02  ();     /* Continuous Mode Processing.            */
extern  int     _ME_03  ();     /* Back Space Mode Processing.            */
extern  int     _ME_04  ();     /* Erase Eof Mode Processing.             */
extern  int     _ME_05  ();     /* Erase Input Mode Processing.           */
extern  int     _ME_06  ();     /* Delete Mode Processing.                */
extern  int     _ME_07  ();     /* Character Editing Routine.             */
extern  int     _ME_09  ();     /* Editing Back Space Mode Processing.    */
extern  int     _ME_0a  ();     /* All Candidate Number Input Processing. */
extern  int     _ME_0b  ();     /* Kanji Number Input Processing.         */
extern  int     _ME_0c  ();     /* Kanji Number Mode Delete Processing.   */
extern  int     _ME_0d  ();     /* Conversion Mode Switch Processing.     */
extern  int     _ME_0e  ();     /* Kanji Number Back Space Processing.    */
extern  int     _ME_rtn ();     /* Editing General Routine.               */
extern  int     _MKL_rs ();     /* Keybaord Lock Reset Processing.        */
extern  int     _MK_a2  ();     /* Kanji Number Input Processing.         */
extern  int     _MK_b4a ();     /* Hiragana/Katakana Conversion A.        */
extern  int     _MK_b4b ();     /* Hiragana/Katakana Conversion B.        */
extern  int     _MK_c1  ();     /* First Conversion.                      */
extern  int     _MK_c3  ();     /* Next Conversion.                       */
extern  int     _MK_e3  ();     /* All Candidate Conversion.              */
extern  int     _MK_e4  ();     /* All Candidate Data Generate.           */
extern  int     _MK_rtn ();     /* KKC Interface Routine.                 */
extern  int     _MMSG_rs();     /* Message Control  Routine.              */
extern  int     _MM_09  ();     /* Dictionary Registration Yomi Prompt.   */
extern  int     _MM_rtn ();     /* Mode Switching Routine.                */
extern  int     _MRG_a  ();     /* Registration Yomi Get.                 */
extern  int     _MRG_b  ();     /* Registration DBCS Get.                 */
extern  int     _MRM_rs ();     /* Dictionary Registration Message Reset. */
extern  int     _MR_rtn ();     /* Kana/Kanji Conversion Interface Rtn.   */
extern  int     _MS_rtn ();     /* Shift Status Set Routine.              */
extern  int     _Macaxst();     /* All Candidate Auxiliary Area.          */
extern  int     _Macifst();     /* All Candidate Input Field.             */
extern  int     _Maddch ();     /* Display Character Add.                 */
extern  int     _Maxcrmv();     /* Auxiliary Area Cursor Move.            */
extern  int     _Maxmst ();     /* Auxiliary Area Message Set.            */
extern  int     _Mbsins ();     /* Back Space in Insert Editing Mode.     */
extern  int     _Mbsrepn();     /* Back Space in Replace Editing(No Save).*/
extern  int     _Mbsrepr();     /* Back Space in Replace Editing(Saved).  */
extern  int     _Mckbk  ();     /* Candidate Phrase  Check.               */
extern  int     _Mcrmv  ();     /* Phrase Cursor Move.                    */
extern  int     _Mdagend();     /* Diagnosis Trace End.                   */
extern  int     _Mdagmsg();     /* Diagnosis Trace Message Set.           */
extern  int     _Mdagst ();     /* Diagnosis Trace Start.                 */
extern  int     _Mdaha  ();     /* Dakuten/Handakuten Processing.         */
extern  int     _Mdelins();     /* Delete Character in Insert Mode.       */
extern  int     _Mdelrep();     /* Delete Character in Delete Mode.       */
extern  int     _Mdisv  ();     /* Input Information Save.                */
extern  int     _Mecho  ();     /* String Data Set Kanji Control  Block.  */
extern  int     _Mexchng();     /* Exchnage String.                       */
extern  int     _Mfmrst ();     /* Auxiliary Area & Input FIeld           */
                                /* Message Information Recover.           */
extern  unsigned char   _Mgetchm();
                                /* Scan DBCS Character Mode.              */
extern  int     _Mhrktsw();     /* Hiragana/Katakana Conversion.          */
extern  int     _Mhtdc  ();     /* Sigle Byte Character Sets Data String  */
                                /* Interchange for Double Byte Character  */
                                /* Sets Data String.                      */
extern  int     _Mifmst ();     /* Input Field Information Set.           */
extern  int     _Mindset();     /* Indicator Information Set.             */
extern  int     _Minsch ();     /* Insert Character in Kanji Control      */
                                /* Block.                                 */
extern  int     _Minssv ();     /* Insert Cahracter in Kanji Control      */
                                /* Block,with Current DBCS String Save    */
                                /* in Background Save Area.               */
extern  int     _Mkanagt();     /* Get Kana Data From Kanji Conversion    */
                                /* Map,with Kana Information.             */
extern  int     _Mkanasd();     /* Signle Byte 'kana' String Convert to   */
                                /* DBCS String.                           */
extern  int     _Mkcflcn();     /* First Candidates & Last Candidate      */
                                /* Processing.                            */
extern  int     _Mkcnxpr();     /* Kana/Kanji Control  Block Information  */
                                /* Set Kanji Monitor Internal Save Area.  */
extern  int     _Mkjgrst();     /* Kanji Monitor Internal Save Information*/
                                /* Set Kana/Kanji Control  Block.         */
extern  int     _Mkkcclr();     /* Initialize Kanji Monitor Internal Save */
                                /* Area.                                  */
extern  char    _Mktec  ();     /* Single Byte 'kana' Data Convert to     */
                                /* Single Byte 'Alphanumeric'(Romaji Kana-*/
                                /* Conversion).                           */
extern  int     _Mktnc  ();     /* Kanji Number(JIS Segment Number) Proce-*/
                                /* ssing.                                 */
extern  unsigned char   _Mlfrtc ();
                                /* Get DBCS Character Class               */
extern  int     _Mlock  ();     /* Keyboard Lock Flag Set.                */
extern  int     _Mmvch  ();     /* Move Character String.                 */
extern  int     _Mnumgt ();     /* Single Byte 'Numberic' code & 'Keytop  */
                                /* Numeric code(in Kana/Hiragana Mode)'   */
                                /* Convert to DBCS Numeric Code.          */
extern  int     _Mnxprps();     /* Kana/Kanji Conversion Backend Process. */
extern  int     _Mnxtopn();     /* Kana/Kanji Interface Reinitialize.     */
extern  int     _Mnxtpre();     /* Kana/Kanji Conversion Interface Init.  */
extern  int     _Mregrs ();     /* Dictionary Regsitration Mode Reset.    */
extern  int     _Mrepch ();     /* Replace DBCS One Character.            */
extern  int     _Mreset ();     /* Mode Reset Routine.                    */
extern  int     _Mrscvym();     /* Converted DBCS One Phrase Recover      */
                                /* Yomi.                                  */
extern  int     _Mrsstrh();     /* Input Field Attribute Set.             */
extern  int     _Mrstym ();     /* Converted DBCS String Specified Area   */
                                /* Recover Yomi.                          */
extern  int     _Msetch ();     /* Input Field Redraw Area Set.           */
extern  int     _Mstlcvl();     /* Terminate Specified Conversion Area    */
                                /* and Last Conveted DBCS String Data     */
                                /* Learning Dictionary.                   */
extern  int     _Mstrl  ();     /* Get Input Field Available Data Length. */
extern  int     _Mymstl ();     /* Terminate Yomi Conversion Data.        */
extern  int     _Myomic ();     /* Kana DBCS String Convert to 7bit yomi  */
                                /* code for MKK.                          */



/* #(B) 1987.12.15. Flying Conversion Add */
extern  int     _Mflypro();/* Flying Conversin Processing.              */
extern  int     _Mflycnv();/* Flying Conversion.                        */
extern  int     _Midecid();/* Promary decide management fly conversion. */
extern  int     _Mansave();/* Save management fly conversion.           */
extern  int     _Mflyrst();/* Reset fly conversion area.                */
extern  int     _MK_c1f ();/* First Conversion of Flying Conversion.    */
extern  int     _Msglop ();/* Open Single Candidates.                   */
extern  int     _Msglfw ();/* Forward Single Candidates.                */
extern  int     _Msglbw ();/* Backward Single Candidates.               */
/* #(E) 1987.12.15. Flying Conversion Add */


extern  int     _Rkc    ();     /* Romaji Kana Conversion Routine.        */
extern  int     _Traced ();     /* Diagnosis Trace(Data Type).            */
extern  int     _Tracef ();     /* Diagnosis Trace Data Output.           */
extern  int     _Tracep ();     /* Diagnosis Trace(Pointer Type).         */

#endif

