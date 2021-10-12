/* @(#)14       1.2.1.1  src/bos/diag/util/ufd/fd_sa.h, dsaufd, bos411, 9428A410j 3/6/92 15:06:11 */
#ifndef _H_FD_SA
#define _H_FD_SA

/*
 * COMPONENT_NAME: TUDKST
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define DRIVE_0    0
#define DRIVE_1    1

#define RFD_0_35_HH  "/dev/rfd0.36"      /*   3.5  inch Drive 0 hi density */
#define RFD_0_35_H   "/dev/rfd0.18"       /*   3.5  inch Drive 0 hi density */
#define RFD_0_35_L   "/dev/rfd0.9"       /*   3.5  inch Drive 0 low densit */
#define RFD_1_525_H  "/dev/rfd1.15"     /*   5.25 inch Drive 1 hi denisty */
#define RFD_1_525_L  "/dev/rfd1.9"      /*   5.25 inch Drive 1 lo density */


#define AIX_ERROR            -1
#define A_LINE               160
#define A_SCREEN  (A_LINE * 20)
#define ANSWERED_YES          1

#define DEFAULT_FAILURES      10

#define FIRST_STAGE          1
#define SECOND_STAGE         2
#define THIRD_STAGE          3
#define FOURTH_STAGE         4

#endif

/* end _H_FD_SA  */
