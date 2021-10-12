static char sccsid[] = "src/bos/diag/tu/tok/rw_pos.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:40";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: read_pos, write_pos
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Read/Write POS Register

Module Name :  rw_pos.c
SCCS ID     :  1.4

Current Date:  6/20/91, 10:14:45
Newest Delta:  1/19/90, 16:49:37

Functions to read/write POS registers.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/tokuser.h>
#include <errno.h>

#include "toktst.h"

#ifdef debugg
extern void detrace();
#endif

/******************************************************************************

read_pos

Reads specified POS register, "pos_reg", and places value in variable
pointed to by "pos_val_p".

IF successful
THEN RETURNs 0
ELSE RETURNs error for pos read

******************************************************************************/

int read_pos (fdes, pos_reg, pos_val_p, tucb_ptr)
   int fdes;
   unsigned char pos_reg;
   unsigned char *pos_val_p;
   TUTYPE *tucb_ptr;
   {
	struct TOK_POS_REG tr_pos_s;
	int cc;
	extern int errno;

	extern int mktu_rc();
	struct htx_data *htx_sp;
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	tr_pos_s.opcode = TOK_READ;
	tr_pos_s.pos_reg = pos_reg;
	tr_pos_s.pos_val = 0;
	/*
	 * changed cmd parm from CIO_POS_ACC to that below
	 * since device driver switched to this macro 08/10/89.
	 */
	cc = ioctl(fdes, TOK_ACCESS_POS, &tr_pos_s);
	if (cc < 0)
	   {
		if (htx_sp != NULL)
			htx_sp->bad_others++;

		if ((errno == EIO) || (errno == ENOMSG))
		   {
			if (tr_pos_s.status == TOK_NOT_DIAG_MODE)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					NOT_DIAG_ERR));

			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS_RD_ERR + pos_reg));
		   }
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }

	*pos_val_p = tr_pos_s.pos_val;
	if (htx_sp != NULL)
		htx_sp->good_others++;

	return(0);
   }

/******************************************************************************

write_pos

Writes specified POS register, "pos_reg", with value "pos_val".

IF successful
THEN RETURNs 0
ELSE RETURNs error for pos write

******************************************************************************/

int write_pos (fdes, pos_reg, pos_val, tucb_ptr)
   int fdes;
   unsigned char pos_reg;
   unsigned char pos_val;
   TUTYPE *tucb_ptr;
   {
	struct TOK_POS_REG tr_pos_s;
	int cc;

	extern int mktu_rc();
	struct htx_data *htx_sp;
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	tr_pos_s.opcode = TOK_WRITE;
	tr_pos_s.pos_reg = pos_reg;
	tr_pos_s.pos_val = pos_val;
	/*
	 * changed cmd parm from CIO_POS_ACC to that below
	 * since device driver switched to this macro 08/10/89.
	 */
	cc = ioctl(fdes, TOK_ACCESS_POS, &tr_pos_s);
	if (cc < 0)
	   {
		if (htx_sp != NULL)
			htx_sp->bad_others++;

		if ((errno == EIO) || (errno == ENOMSG))
		   {
			if (tr_pos_s.status == TOK_NOT_DIAG_MODE)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					NOT_DIAG_ERR));

			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				POS_WR_ERR + pos_reg));
		   }
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }

	if (htx_sp != NULL)
		htx_sp->good_others++;

	return(0);
   }
