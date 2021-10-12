/*
 * $Log: pfm.h,v $
 * Revision 1.1  1991/04/01  21:26:25  ddj
 * Initial revision
 *
 * $EndLog$
 */
/*
 * ========================================================================== 
 * Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts
 * 
 * All Rights Reserved
 * 
 * All Apollo source code software programs, object code software programs,
 * documentation and copies thereof shall contain the copyright notice above
 * and this permission notice.  Apollo Computer Inc. reserves all rights,
 * title and interest with respect to copying, modification or the
 * distribution of such software programs and associated documentation,
 * except those rights specifically granted by Apollo in a Product Software
 * Program License or Source Code License between Apollo and Licensee.
 * Without this License, such software programs may not be used, copied,
 * modified or distributed in source or object code form.  Further, the
 * copyright notice must appear on the media, the supporting documentation
 * and packaging.  A Source Code License does not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * 
 * Apollo disclaims all warranties, express or implied, with respect to
 * the Software Programs including the implied warranties of merchantability
 * and fitness, for a particular purpose.  In no event shall Apollo be liable
 * for any special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits whether in an
 * action of contract or tort, arising out of or in connection with the
 * use or performance of such software programs.
 * ========================================================================== 
 */

#ifndef pfm_included
#define pfm_included

/*
 * Process Fault Manager definitions -- *OLD VERSION*.  See "ppfm.h" for latest
 * version.  This version retained for compatibility (esp. of "pfm_$..." macros
 * below).  New applications should use "ppfm.h" and the new-style "pfm_$"
 * macros defined there.
 */

#ifdef apollo

#ifdef __STDC__
#  include <ppfm.h>
#else
#  ifdef DSEE
#    include "sys/ins/pfm.ins.c"
#  else
#    include "/sys/ins/pfm.ins.c"
#  endif
#  define pfm__init(junk)
#  define Volatile
#endif

#else

#define pfm__cleanup(crec) \
    pfm____cleanup(&(crec))

#define pfm__rls_cleanup(crec, st) \
    pfm___rls_cleanup(&(crec), &st)

#define pfm__reset_cleanup(crec, st) \
    pfm___reset_cleanup(&(crec), &st)

#include <ppfm.h>

#endif

/*
 * The following are here only to deal with an interim, botched naming scheme
 * that will eventually go away.  Do *not* use these names.
 */

#ifndef apollo
    /* Non-Apollo */
#define pfm__p_cleanup          pfm____cleanup
#define pfm__p_rls_cleanup      pfm___rls_cleanup
#define pfm__p_reset_cleanup    pfm___reset_cleanup
#else
#ifdef __STDC__
    /* Apollo/ANSI */
#define pfm__p_cleanup          pfm__cleanup
#define pfm__p_rls_cleanup      pfm__rls_cleanup
#define pfm__p_reset_cleanup    pfm__reset_cleanup
#else
    /* Apollo/non-ANSI */
#define pfm__p_cleanup          "pfm__p_cleanup not allowed"
#define pfm__p_rls_cleanup      "pfm__p_rls_cleanup not allowed"
#define pfm__p_reset_cleanup    "pfm__p_reset_cleanup not allowed"
#endif
#endif

#endif
