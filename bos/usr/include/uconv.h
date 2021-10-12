/* @(#)26  1.1  src/bos/usr/include/uconv.h, libiconv, bos411, 9428A410j  11/2/93  11:23:59
 *
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_UCONV_H
#define	_UCONV_H

#include <sys/types.h>
#include <sys/errno.h>
#include <iconv.h>

/*
 *	Conversion descriptor.
 */

typedef	struct _UconvObject	*UconvObject;

struct	_UconvObject {
	_LC_core_iconv_t	core;
	int			(*uconv_from_ucs)(UconvObject,
				UniChar**, size_t*, void**, size_t*, size_t*);
	int			(*uconv_to_ucs)(UconvObject,
				void**, size_t*, UniChar**, size_t*, size_t*);
	void			*ch;	/* Conversion handle */
};

/*
 *	Prototypes
 */

extern	int	UniCreateUconvObject (	/* Initialize UCS conversion         */
		UniChar*,		/*   UCS conversion name             */
		UconvObject*);		/*   Conversion object being returned*/

extern	int	UniUconvToUcs (		/* MBS->UCS conversion               */
		UconvObject,		/*   Conversion object handle        */
		void**,			/*   Input buffer                    */
		size_t*,		/*   #of bytes left in input buffer  */
		UniChar**,		/*   UCS output buffer               */
		size_t*,		/*   #of UniChars left in UCS buffer */
		size_t*);		/*   #of non-identical conversions   */

extern	int	UniUconvFromUcs (	/* UCS->MBS conversion               */
		UconvObject,		/*   Conversion object handle        */
		UniChar**,		/*   UCS input buffer                */
		size_t*,		/*   #of UniChars left in UCS buffer */
		void**,			/*   Output buffer                   */
		size_t*,		/*   #of bytes left in output buffer */
		size_t*);		/*   #of non-identical conversions   */

extern	int	UniUconvFreeUconvObject(/* Close UCS conversion object       */
		UconvObject);		/*   Conversion object handle        */

/*
 *	Return Value Definition.
 */

#define	UCONV_EMFILE	EMFILE		/* {OPEN_MAX} files are open         */
#define	UCONV_ENFILE	ENFILE		/* Too many files                    */
#define	UCONV_ENOMEM	ENOMEM		/* Insufficient storage space        */
#define	UCONV_EINVAL	EINVAL		/* Invalid input parameter           */
#define	UCONV_EILSEQ	EILSEQ		/* Invalid input codepoint           */
#define	UCONV_E2BIG	E2BIG		/* Lack of space in output buffer    */
#define	UCONV_EBADF	EBADF		/* Invalid conversion object         */

#endif/*!_UCONV_H*/
/* End of uconv.h -----------------------------------------------------------*/
