static char sccsid[] = "@(#)39	1.1  src/bos/diag/tu/iop/ioptu_17.c, tu_iop, bos411, 9428A410j 4/14/94 10:43:28";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_17()
 *
 *   ORIGINS: 27, 83
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/*  TU 17 restores the LCD values if saved in TU16.        */
	/*  TU16 must have been executed before this TU or rc = 20 */
	/*  will be returned.                                      */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int ioptu_17(fildes, First, save, tucb_ptr)
int	fildes;
int	*First;
char	*save;
TUTYPE	*tucb_ptr;
{					 /*	LCD_TEST	*/

	int	i;
        int err = 0 ;                       /* return code    */
        char data[32] ;

	if (*First ==1)
		return(err = 20) ;

	/* put orig value back */
	for (i=0; i<32; i++)
		data[i] = *(save+i) ; 

	err = set_nvram(fildes,LCD_STR_NEW,data,MV_WORD,8,tucb_ptr);

	*First = 1 ;		/* to restart	*/

	return err ;
} /* ioptu_17 *.c */
