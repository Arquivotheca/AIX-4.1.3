static char sccsid[] = "@(#)38	1.1  src/bos/diag/tu/iop/ioptu_16.c, tu_iop, bos411, 9428A410j 4/14/94 10:43:12";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_16()
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
#include "ioptu.h"

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/*  The first time here, TU 16 reads and saves values in     */
	/*  the LCD registers                                        */
	/*  It then writes *U*U*U to LCD and returns.                */
	/*  Subsequent times here it writes VAS to LCD and returns.  */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int	ioptu_16( fildes, First, save, tucb_ptr)
int	fildes;
int	*First;
char	*save;
TUTYPE	*tucb_ptr;
{					 /*	LCD_TEST	*/

	int	i,j,k;
	char	VAS_string[] = "VAS ";
        int err = 0 ;                       /* return code    */
        char data[32] ;                     /* returned data  */
        char comp[32] ;
	
	if (*First == 1) {
						/* read and save LCD */
		err = get_nvram(fildes,LCD_STR_OLD,data,
					MV_WORD,8,tucb_ptr);
		if ( err != 0 ) return(err) ;
		sleep(1);

		for (i=0; i<32; i++) {
			*(save+i)= data[i] ;
		} /* end for i=0 ...	*/

				/* writes 16 'U*' to LCD	*/
		for (k=0; k<16; k++) {
			data[(2*k)] = 'U' ;
			data[(2*k)+1] = '*' ;
		} /* end for k=0 ...	*/

		err = set_nvram(fildes,LCD_STR_NEW,data,MV_WORD,8,tucb_ptr);
		if ( err != 0 ) return(err) ;
		sleep(1);

		*First = 0 ;	/* only one access	*/

	} 			/* endif First == 1	*/
	else	{		/* First == 0	*/
				/* writes a circulating 'VAS'	*/
		for (j=0; j<4; j++) {
			for (k=0; k<32; k++) {
				data[k] = VAS_string[(k+j)%4];
			} /* end for k=0  */

			err = set_nvram(fildes,LCD_STR_NEW,data,
				MV_WORD,8,tucb_ptr);
			if ( err != 0 ) return(err) ;

			/* wait for string to be displayed */
			for(k=0; k<10; ++k) { /* wait 10 times max */
				err = get_nvram(fildes,LCD_STR_OLD,comp,
					MV_WORD,8,tucb_ptr);
				if ( err != 0 ) return(err) ;
				sleep(1);
				if(!strncmp(data,comp,32))
					break;
			} /* end for k */
			if ( k >= 10 )
				return(LCD_TEST_FAILURE);

		} /* end for j=0; ..	*/
	}
	return err ;				 /* end TU 16 */
} /* ioptu_16 *.c */
