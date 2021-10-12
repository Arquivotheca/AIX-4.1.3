/* @(#)01	1.2  src/bos/usr/bin/panel20/w_dismsg.h, cmdhia, bos411, 9428A410j 7/21/92 10:25:22 */

/*
 * COMPONENT_NAME: (CMDHIA) Messages include file
 *
 * FUNCTIONS: Defines the masks for all the arguments which can be
 *            inserted into a message
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define W_MSGVI1		0x0001	/* Mask for int variable 1         */
#define W_MSGVI2		0x0002	/* Mask for int variable 2         */
#define W_MSGVL1		0x0004	/* Mask for long int variable 1    */
#define W_MSGVL2		0x0008	/* Mask for long int variable 2    */
#define W_MSGVC1		0x0010	/* Mask for char string variable 1 */
#define W_MSGVC2		0x0020	/* Mask for char string variable 2 */
#define W_MSGVC3		0x0040	/* Mask for char string variable 3 */
#define W_MSGVCH1		0x0080	/* Mask for char variable 1        */
