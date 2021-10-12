/* @(#)01       1.3  src/bos/usr/include/uc_conv.h, libiconv, bos411, 9428A410j 4/5/94 08:35:52
 *
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		Definitions for UCS Toolkit
 *			codeset conversion functions.
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

#ifndef __UC_CONV
#define __UC_CONV

#include <sys/types.h>
#include <uc_convP.h>

/* Map option ---------------------------------------------------------------*/

#define	MAP_NONE	0		/* No override                       */
#define	MAP_IRV		1		/* International Reference Version   */

/* Substitution option ------------------------------------------------------*/

#define NO_SUBSTITUTION		0	/* Don't apply substitution          */
#define SUBSTITUTE_TO_UNICODE	1	/* Apply only TO Unicode             */
#define SUBSTITUTE_FROM_UNICODE	2	/* Apply only FROM Unicode           */
#define SUBSTITUTE_BOTH_WAYS	3	/* Apply both TO & FROM              */

/* Function prototypes  -----------------------------------------------------*/

extern	int		UCCINIT (	/* Initialize UCS conversion         */
	uchar_t		*cs_name,	/* MBCS codeset specification        */
	int		map,		/* Map option                        */
	int		sub,		/* Substitution option               */
	UniChar		sub_uni,	/* Substitution character in UCS     */
	uchar_t		*subchar,	/* Substitution character in MBCS    */
	_uc_ch_t	**ch);		/* Reterns conversion handle         */

extern	int		UCCM2U (	/* MBCS to UCS conversion            */
	_uc_ch_t	*ch,		/* Conversion handle                 */
	uchar_t		*in_buf,	/* Input buffer                      */
	size_t		*in_size,	/* #of bytes of in_buf / processed   */
	UniChar		*out_buf,	/* Output buffer                     */
	size_t		*out_size,	/* #of UniChar of out_buf / output   */
	size_t		*subs);		/* #of non-identical conversions     */

extern	int		UCCU2M (	/* UCS to MBCS conversion            */
	_uc_ch_t	*ch,		/* Conversion handle                 */
	UniChar		*in_buf,	/* Input buffer                      */
	size_t		*in_size,	/* #of UniChar of in_buf / processed */
	uchar_t		*out_buf,	/* Output buffer                     */
	size_t		*out_size,	/* #of bytes of out_buf / output     */
	size_t		*subs);		/* #of non-identical conversions     */

extern	int		UCCTERM (	/* Terminate UCS conversion          */
	_uc_ch_t	*ch);		/* Conversion handle                 */

/* Error status codes for the function return value -------------------------*/

#define UC_NO_ERRORS			0
#define	UC_INVALID_HANDLE		1
#define UC_INVALID_OPTION		2
#define UC_INVALID_SUBCHAR		3
#define UC_INVALID_TABLE		4
#define UC_TABLE_NOT_AVAILABLE		5
#define	UC_NOT_ENOUGH_SPACE		6
#define UC_BUFFER_FULL			7
#define UC_INVALID_CHAR_FOUND		8
#define UC_INPUT_CHAR_TRUNCATED		9
#define UC_OTHER_ERRORS			100

#endif /*!__UC_CONV*/
