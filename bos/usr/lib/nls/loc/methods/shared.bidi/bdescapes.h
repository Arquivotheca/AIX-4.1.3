/* @(#)35	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/bdescapes.h, cfgnls, bos411, 9439B411a 9/27/94 14:46:35 */
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: *		BDAutoPush
 *		BDDisableAutoPush
 *		BDDisableKeys
 *		BDEnableAutoPush
 *		BDEnableKeys
 *		BDImplicitOn
 *		BDLatinLang
 *		BDLatinLtr
 *		BDNLRtl
 *		BDNaturalLang
 *		BDNumArabicOn
 *		BDNumBilingualOn
 *		BDNumCsdOff
 *		BDNumHinduOn
 *		BDRestoreEnv
 *		BDSaveEnv
 *		BDSetNonullsMode
 *		BDSetNullsMode
 *		BDShapeAixMode
 *		BDShapeHostMode
 *		BDSymmetricSwapOff
 *		BDSymmetricSwapOn
 *		BDTurnOffBidi
 *		BDTurnOnBidi
 *		BDVisualOn
 *		send_escape
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef	 _BDESCAPES_
#define  _BDESCAPES_

#include <fcntl.h>

/*... bidi ESC sequneces  ...  */
#define        BIDI_DISABLE_BD_KEYS_ESC_SEQ  "\033[?82h"
#define        BIDI_ENABLE_BD_KEYS_ESC_SEQ   "\033[?82l"
#define        BIDI_SAVE_BD_ENV_ESC_SEQ      "\033[?83h"
#define        BIDI_RESTORE_BD_ENV_ESC_SEQ   "\033[?83l"
#define	       BIDI_RTL_ESC_SEQ	             "\033[?35h"
#define	       BIDI_LTR_ESC_SEQ	             "\033[?35l"
#define	       BIDI_TURN_ON_ESC_SEQ          "\033[?86h"
#define	       BIDI_TURN_OFF_ESC_SEQ         "\033[?86l"
#define	       BIDI_TOGGLE_AUTOPUSH_ESC_SEQ  "\033[?87h"
#define	       BIDI_ENABLE_AUTOPUSH_ESC_SEQ  "\033[?88h"
#define	       BIDI_DISABLE_AUTOPUSH_ESC_SEQ "\033[?88l"

#define        BIDI_NL_RTL_ESC_SEQ           "\033[3;1 S"
#define        BIDI_LATIN_LTR_ESC_SEQ        "\033[0;1 S" 
#define        BIDI_SYMMETRIC_ON_ESC_SEQ     "\033[3 ]"
#define        BIDI_SYMMETRIC_OFF_ESC_SEQ    "\033[15 ]"
#define        BIDI_IMPLICIT_ON_ESC_SEQ      "\033[8l"
#define        BIDI_VISUAL_ON_ESC_SEQ        "\033[8h"
#define        BIDI_NUM_CSD_OFF_ESC_SEQ      "\033[18 ]"
#define        BIDI_NUM_ARABIC_ESC_SEQ       "\033[1 ]"
#define        BIDI_NUM_HINDU_ESC_SEQ        "\033[2 ]"
#define        BIDI_NUM_BILINGUAL_ESC_SEQ    "\033[20 ]"
#define        BIDI_SET_DEFAULTS_ESC_SEQ     "\033[0 ]"
#define        BIDI_HOST_SHAPING_ESC_SEQ     "\033[13 ]"
#define        BIDI_AIX_SHAPING_ESC_SEQ      "\033[14 ]"
#define        BIDI_SET_NONULLS_ESC_SEQ      "\033[23 ]"
#define        BIDI_SET_NULLS_ESC_SEQ        "\033[24 ]"

/* macros to set bidi escapes */
#define  send_escape(fid,esc)  \
            if (fid<0) write(open("/dev/tty",O_WRONLY),esc,strlen(esc)); \
            else write (fid,esc,strlen(esc))

#define BDTurnOnBidi(fid)       send_escape(fid,BIDI_TURN_ON_ESC_SEQ)
#define BDTurnOffBidi(fid)      send_escape(fid,BIDI_TURN_OFF_ESC_SEQ)
#define BDSetDefaults(fid)      send_escape(fid,BIDI_SET_DEFAULTS_ESC_SEQ)
#define BDDisableKeys(fid)      send_escape(fid,BIDI_DISABLE_BD_KEYS_ESC_SEQ)
#define BDEnableKeys(fid)       send_escape(fid,BIDI_ENABLE_BD_KEYS_ESC_SEQ)
#define BDSaveEnv(fid)          send_escape(fid,BIDI_SAVE_BD_ENV_ESC_SEQ)
#define BDRestoreEnv(fid)       send_escape(fid,BIDI_RESTORE_BD_ENV_ESC_SEQ)
#define BDLatinLtr(fid)         send_escape(fid,BIDI_LATIN_LTR_ESC_SEQ)
#define BDNLRtl(fid)            send_escape(fid,BIDI_NL_RTL_ESC_SEQ)
#define BDNaturalLang(fid)      send_escape(fid,BIDI_RTL_ESC_SEQ)
#define BDLatinLang(fid)        send_escape(fid,BIDI_LTR_ESC_SEQ)
#define BDAutoPush(fid)         send_escape(fid,BIDI_TOGGLE_AUTOPUSH_ESC_SEQ)
#define BDDisableAutoPush(fid)  send_escape(fid,BIDI_DISABLE_AUTOPUSH_ESC_SEQ)
#define BDEnableAutoPush(fid)   send_escape(fid,BIDI_ENABLE_AUTOPUSH_ESC_SEQ)
#define BDSymmetricSwapOn(fid)  send_escape(fid,BIDI_SYMMETRIC_ON_ESC_SEQ)
#define BDSymmetricSwapOff(fid) send_escape(fid,BIDI_SYMMETRIC_OFF_ESC_SEQ)
#define BDNumCsdOff(fid)        send_escape(fid,BIDI_NUM_CSD_OFF_ESC_SEQ)
#define BDImplicitOn(fid)       send_escape(fid,BIDI_IMPLICIT_ON_ESC_SEQ)
#define BDVisualOn(fid)         send_escape(fid,BIDI_VISUAL_ON_ESC_SEQ)
#define BDNumArabicOn(fid)      send_escape(fid,BIDI_NUM_ARABIC_ESC_SEQ)
#define BDNumHinduOn(fid)       send_escape(fid,BIDI_NUM_HINDU_ESC_SEQ)
#define BDNumBilingualOn(fid)   send_escape(fid,BIDI_NUM_BILINGUAL_ESC_SEQ)
#define BDShapeHostMode(fid)    send_escape(fid,BIDI_HOST_SHAPING_ESC_SEQ)
#define BDShapeAixMode(fid)     send_escape(fid,BIDI_AIX_SHAPING_ESC_SEQ)
#define BDSetNonullsMode(fid)   send_escape(fid,BIDI_SET_NONULLS_ESC_SEQ)
#define BDSetNullsMode(fid)     send_escape(fid,BIDI_SET_NULLS_ESC_SEQ)
#endif /* _BDESCAPES_ */
