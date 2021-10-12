/* static char sccsid[] = "@(#)91  1.2  src/bos/diag/tu/kbd/kbd_io.c, tu_kbd, bos411, 9428A410j 5/10/94 14:20:22"; */
/*
 *
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS:   wr_byte, rd_iocc, wr_iocc, 
 *              rd_byte, wr_word 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*                                                                         */
/*       If the mfg mode in the tu control block (tucb) is set to be       */
/*       invoked by HTX then TU program will look at variables in tu       */

/*       uses the predefined values.                                       */
/*                                                                         */
/***************************************************************************/

#include "tu_type.h"

/****************************************************************************/
/* FUNCTION: wr_byte

   DESCRIPTION: Uses the machine device driver to write ONE BYTE to
		the specified address

****************************************************************************/

int wr_byte(int fdes, unsigned char *pdata,unsigned int addr)
{
	MACH_DD_IO iob;
	int rc;

	iob.md_data = pdata;
	iob.md_incr = MV_BYTE;
	iob.md_size = sizeof(*pdata);
	iob.md_addr = addr;
	rc = ioctl(fdes, MIOBUSPUT, &iob);
	return (rc);
}

/****************************************************************************/
/* FUNCTION: wr_word

   DESCRIPTION: Uses the machine device driver to write a WORD from
                pdata to the specified address

****************************************************************************/

int wr_word(fdes, pdata, addr)
int fdes;
char *pdata;
unsigned int addr;
{
        MACH_DD_IO iob;
        int rc;

        iob.md_data = (char *)pdata;
        iob.md_incr = MV_WORD;
        iob.md_size = sizeof(*pdata);
        iob.md_addr = addr;
        rc = ioctl(fdes, MIOBUSPUT, &iob);
        return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_byte

   DESCRIPTION: Uses the machine device driver to read ONE BYTE to
		the specified address

****************************************************************************/

int rd_byte(int fdes, unsigned char *pdata,unsigned int addr)
{
	MACH_DD_IO iob;
	int rc;

	iob.md_data = pdata;
	iob.md_incr = MV_BYTE;
	iob.md_size = sizeof(*pdata);
	iob.md_addr = addr;
	rc = ioctl(fdes, MIOBUSGET, &iob);
	return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_iocc 

   DESCRIPTION: Uses the machine device driver macro to read the
		POS register 

****************************************************************************/
int rd_iocc(fdes, pdata, addr)
int fdes;
unsigned char *pdata;
unsigned int addr;
{
	MACH_DD_IO iob;
	int rc;

	iob.md_data = pdata;
	iob.md_incr = MV_BYTE;
	iob.md_size = sizeof(*pdata);
	iob.md_addr = POSREG(addr, POS_SLOT);    /* mdio.h macro for IOCC address */
	rc = ioctl(fdes, MIOCCGET, &iob);
	return (rc);
}


/****************************************************************************/
/* FUNCTION: wr_iocc 

   DESCRIPTION: Uses the machine device driver macro to write to the
		POS register 

****************************************************************************/

int wr_iocc(fdes, pdata, addr)
int fdes;
unsigned char *pdata;
unsigned int addr;
{
	MACH_DD_IO iob;
	int rc;

	iob.md_data = pdata;
	iob.md_incr = MV_BYTE;
	iob.md_size = sizeof(*pdata);
	iob.md_addr = POSREG(addr, POS_SLOT);    /* mdio.h macro for IOCC address */
	rc = ioctl(fdes, MIOCCPUT, &iob);
	return (rc);
}

