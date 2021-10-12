/* @(#)84	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/kmpfdef.h, libKJI, bos411, 9428A410j 7/23/92 03:26:01 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _kj_kmpfdef
#define _kj_kmpfdef
/*
 *      Kanji Profile Read/Write Permission.
 */
#define M_KPFPRH (     '%'    )/* Symbol for No interactive customzing in */
                               /* Kanji Monitor Profile.                  */
#define M_KISPRH (    0x01    )/* Constant for unchanged flag.            */
                               /* (Kanji Monitor Profile save data.)      */
/*
 *      Replace/Insert Mode Change Key.
 */
#define K_INSERT (      1     )/* Insert key.                             */
#define K_RESET  (      2     )/* reset key.                              */
/*
 *      Decide Input Key for Kanji Number,Dictionary Registration.
 */
#define K_DAACT  (    0x01    )/* Action Enable.                          */
#define K_DAENT  (    0x02    )/* Enter  Enable.                          */
#define K_DACR   (    0x04    )/* CR     Enable.                          */
/*
 *      Cancel Input Key for All Candidate,Kanji Number,Dictionary
 +      Registratin,and Message Display.
 */
#define K_REACT  (    0x01    )/* Action Enable.                          */
#define K_REENT  (    0x02    )/* Enter  Enable.                          */
#define K_RECR   (    0x04    )/* Cr     Enable.                          */
#define K_RERES  (    0x08    )/* Reset  Enable.                          */
/*
 *      Initial Conversion Mode Switch.
 */
#define K_FUKUGO (  K_WLCON   )/* Word level conversion.                  */
#define K_TANBUN (  K_SPLCON  )/* Single phrase level conversion.         */
#define K_RENBUN (  K_MPBCON  )/* Multi Phase level batch conversion.     */
/*
 *      Kanji Number Input Mode.
 */
#define K_KJIS   (      1     )/* JIS      Code Mode.                     */
#define K_KEBC   (      2     )/* EBCDIC   Code Mode.                     */
#define K_KNO    (      3     )/* KANJI NO Code Mode.                     */
#define K_KPCNO  (      4     )/* PC KANJI Code Mode.                     */
/*
 *      Auxiliary Area Display Location.
 */
#define K_AUXNEA (      0     )/* Near cursor.                            */
#define K_AUXCEN (      1     )/* Center.                                 */
#define K_AUXUPL (      2     )/* Upper left.                             */
#define K_AUXUPR (      3     )/* Upper left.                             */
#define K_AUXLOL (      4     )/* Lower left.                             */
#define K_AUXLOR (      5     )/* Lower right.                            */
/*
 *      Caution Bell Enable/Disable.
 */
#define K_BEEPOF (      0     )/* Beep flag is OFF.                       */
#define K_BEEPON (      1     )/* Beep flag is ON.                        */
/*
 *      Interactive Registration Enable/Disable.
 */
#define K_DICOK  (      1     )/* Interactive registration is possible.   */
#define K_DICNG  (      2     )/* Interactive registration is impossible. */
/*
 *      Input FIeld Mix Enable/Disable.
 */
#define K_MIXNG  (      0     )/* Input Field is DBCS String Only.        */
#define K_MIXOK  (      1     )/* Input FIeld is Contains DBCS & ANK.     */
/*
 *      Initial Keyboard Status is Rmaji/Kana or Not.
 */
#define K_ROMOFF (      0     )/* Keyboard Code not conversion.           */
#define K_ROMON  (      1     )/* Initial Keyboard Status is Romaji/Kana  */
                               /* Mode.                                   */
/*
 *      Initial Romaji/Kana Mode.
 */
#define K_KATA   (  K_ST1KAT  )/* Katakana.                               */
#define K_KANA   (  K_ST1HIR  )/* Hiragana.                               */
/*
 *      Keyboard Lock Enable/Disable Mode.
 */
#define K_KBLOFF (      0     )/* Keyboard lock Disable.                  */
#define K_KBLON  (      1     )/* Keybaord Lock Enable.                   */
/*
 *      Cursor Move Range Infield or Contains Outfield.
 */
#define K_CURIN  (      0     )/* Cursor cannot go out of range.          */
#define K_CUROUT (      1     )/* Cursor can go out of range.             */
/*
 *      Katakana Conversion Enable/Disable Mode.
 */
#define K_KANAOF (      0     )/* Disable Katakana Conversion.            */
#define K_KANAON (      1     )/* Enable  Katakana Conversion.            */
/*
 *      AlphaNumeric Conversion Enable/Disable.
 */
#define K_ALPOFF (      0     )/* Alpha Numeric conversion is Disable.    */
#define K_ALPON  (      1     )/* Alpha Numeric conversion is Enable.     */
/*
 *      Dictionary Learning Enable/DIsable.
 */
#define K_LEAOFF (      0     )/* Learning Dictionary Disable.            */
#define K_LEAON  (      1     )/* Learning Dictionary Enable.             */
/*
 *      Kanji Number Conversion Enable/Disable.
 */
#define K_KJNMOF (      0     )/* Kanji Number Conversion Disable.        */
#define K_KJNMON (      1     )/* Kanji Number COnvertion Enable.         */
/*
 *      Application Return Condition.
 */
#define K_PFKOFF (      0     )/* Auxiliayr Area is'nt Busy &             */
                               /* Control  Sequence Arrive  from Input    */
                               /* Data When Return to Application.        */
#define K_PFKON  (      1     )/* Control  Sequence Arrive  from Input    */
                               /* Data then Allways Return to Application.*/



/* #(B) 1987.12.15. Flying Conversion Add */
#define K_CIKUJI (      0     )/* Flying Conversion.                      */
#define K_IKKATU (      1     )/* Ikkatsu Conversion.                     */
/* #(E) 1987.12.15. Flying Conversion Add */


#define K_JIS78  (      0     )
#define K_JIS83  (      1     )

#define	K_SHNUMDEF  (	1     )	/* A default number for using mapped files*/
#define	K_UDLOADOFF (	0     )	/* User dictionary is never loaded 	*/
				/* after _Jopen()  			*/
#define	K_UDLOADON  (	1     ) /* User dictionary is loaded 		*/
				/* if it has been updated		*/
#define	K_RESETOFF  (	0     )	/* ESC key does not reset input mode 	*/
#define	K_RESETON   (	1     )	/* ESC key resets input mode 		*/

#endif
