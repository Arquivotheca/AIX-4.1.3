static char sccsid[] = "@(#)78	1.1  src/htx/usr/lpp/htx/lib/hga/palette.c, tu_hga, htx410 6/2/94 11:37:09";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: init_cursor_bt485
 *		s3_cursor_size
 *		s3_load_palette
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>

#include "hga_extern.h"
#include "colors.h"
#include "s3regs.h"
#include "Bt485.h"
#include "screen.h"

char	user_palette[256*3];

s3_load_palette(table)
char *table;
{
	int curDACIndex;
	char *s;

	s = user_palette;
	if ( table == NULL )
		table = colors;

	/* Disable the video */
	S3WriteRegister(S3AttrControllerIndex, 0x00);
	S3WriteRegister(S3DACWriteIndexRegister, 0x00);
	for(curDACIndex=0; curDACIndex<(256*3); curDACIndex++) {
		*s++ = *table;
		S3WriteRegister(S3DACDataRegister, *table);
		table++;
	}

	/* Re-enable video */
	S3WriteRegister(S3AttrControllerIndex, 0x20);
	s3_cursor_size(CURSOR_FULL);
	s3_cursor_size(CURSOR_NORMAL);
	return;
}

s3_cursor_size(int new_size)
{
	static cursor_shape = CURSOR_OFF;
	int     old_size;

	old_size = cursor_shape;
	cursor_shape = new_size;
	init_cursor_bt485(new_size);

	return old_size;
}

init_cursor_bt485(int new_size)
{
	char    u;
	int     i;
	char 	cursor_data;
/*
	int     cursor_data;
*/
	int     control_reg;
	static last_load = CURSOR_OFF;

	Bt485_register_read(cmd_reg_2, control_reg);

	/* Check if in interlace mode   */
	S3WriteRegister (S3CRTControlIndex, 0x42); 
	S3ReadRegister(S3CRTControlData, u);
	if (u & 0x20)
		control_reg |= 0x08;              /* If so, tell DAC         */
	else
		control_reg &= ~0x08;             /* If so, tell DAC         */

	if ((last_load != new_size) && (new_size != CURSOR_OFF)) {
		last_load = new_size;
		/* Start at cursor location 0, plane 0  */
		S3WriteRegister( S3DACWriteIndexRegister, 0x00 )
		/* Access cursor RAM array Data         */
		S3WriteRegister (S3CRTControlIndex, 0x55);                 
		S3ReadRegister (S3CRTControlData, u);
		u &= ~0x01;
		u |= 0x02;
		S3WriteRegister( S3CRTControlData, u);

		for (i=0;i<128;i++) {
			cursor_data = 0x00;
			switch (new_size) {
			case CURSOR_FULL:
				if ((i==24) || (i==20) || (i==16) ||
				    (i==12) || (i== 8) || (i== 4) || (i== 0))
					cursor_data = 0xff;
			case CURSOR_HALF:
				if ((i==52) || (i==48) || (i==44) ||
				    (i==40) || (i==36) || (i==32) || (i==28))
					cursor_data = 0xff;
			case CURSOR_NORMAL:
				if ((i==56) || (i==60))
					cursor_data = 0xff;
				break;
			} /* endswitch */
			/* Set cursor shape data                */
			S3WriteRegister( S3DACCursorData, cursor_data);
		}
		for (i=0;i<128;i++)
			/* Set all of plane 1 to 0xff           */
			S3WriteRegister( S3DACCursorData, 0xff);
		/* Restore access to Palette data       */
		S3WriteRegister(S3CRTControlIndex, 0x55);                 
		S3ReadRegister (S3CRTControlData, u);
		u &= ~0x03;
		S3WriteRegister(S3CRTControlData, u);
	} /* endif */

	if (new_size == CURSOR_OFF)
		control_reg &= ~0x02;
	else
		control_reg |= 0x02;

	Bt485_register_write(cmd_reg_2, control_reg);

	return;
}
