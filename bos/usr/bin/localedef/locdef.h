/* @(#)99	1.7  src/bos/usr/bin/localedef/locdef.h, cmdnls, bos411, 9428A410j 3/10/94 10:58:11 */
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __H_LOCDEF
#define __H_LOCDEF
typedef struct {
    char *nlldate;
    char *nltmisc;
    char *nltstr;
    char *nltunits;
    char *nlyear;
} _LC_aix31_t;

/*
 *  predefined weights: IGNORE, UNDEFINED, SUB_STRING, ELLIPSIS_WEIGHT, and
 *  FWD_REF.
 *
 *  UNDEFINED, ELLIPSIS_WEIGHT, and FWD_REF are replaced with appropriate
 *  values.  IGNORE and SUB_STRING weights are passed through to the final
 *  locale object.
 *
 *  At some point, if we need more collation weights, SUB_STRING,
 *  ELLIPSIS_WEIGHT, and FWD_REF could probably be changed from
 *  (USHRT_MAX-n) to (n).  Since IGNORE maps to ffff, it too could
 *  be changed to an unused value < 255.
 */

#define IGNORE    -1
#define UNDEFINED 0
#define SUB_STRING _SUB_STRING
#define ELLIPSIS_WEIGHT (USHRT_MAX - 2)
#define FWD_REF (USHRT_MAX - 3)

/*  _SUB_STRING is defined as (SHRT_MAX - 1) in localedef.h             */


/* Values used as parameters to sem_existing_symbol() */

#define FATAL	0	/* Fatal error if specified char is undefined	*/
#define SKIP	1	/* Ignore characters and range statements	*/
			/* containing undefined characters.		*/

#endif
