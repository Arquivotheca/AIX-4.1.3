static char sccsid[] = "@(#)80	1.1  src/htx/usr/lpp/htx/lib/hga/vramtest.c, tu_hga, htx410 6/2/94 11:37:14";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: s3vram_test
 *		test_next_meg
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
#include <sys/types.h>
#include "tu_type.h"

#include "hga_extern.h"
#include "s3regs.h"

#define S3_MIN_RAM 2
#define S3_928_MAX_RAM 4
#define S3_964_MAX_RAM 8
#define ONE_MEG (1024*1024)

#define s3_964VXP	1

/* hardwired this value, for now */
char  s3_device = s3_964VXP;

s3vram_test()
{
	int             rc = 0;
	unsigned int    vramAddr = 0;
	unsigned int    vramEndAddr = 0;
	unsigned int    i, j;
	volatile char  *testAddr;
	volatile char  *testEndAddr;
	int             max_ram;

	vramAddr = vramBaseAddress;
	/* Find the size of video memory by looking into the configuration
	   register.
	*/
	S3WriteIndex(S3ConfigReg1); 
	S3ReadData(i);  
	i = i >> 5;

	switch (i) {
	case 0:
		if (s3_device == s3_964VXP)
			max_ram = S3_928_MAX_RAM;
		else 
			max_ram = S3_928_MAX_RAM;
		break;
	case 4:
		max_ram = 2;
		break;
	case 6:
		max_ram = 1;
		break;
	default:
		max_ram = 2;
		break;
	} /* endswitch */

	S3WriteIndex(S3LinearAddrWindowPositionLow);
	S3ReadData(i); 
	vramAddr |= i << 16;

	S3WriteIndex(S3LinearAddrWindowPositionHigh);
	S3ReadData(i); 
	vramAddr |= i << 8;

	testAddr = (char *) vramAddr;
	*testAddr = 0x5a;
	for (i=S3_MIN_RAM;i<max_ram;i++ ) {
		/* Find end of video memory     */
		vramEndAddr = vramAddr + ONE_MEG * i;
		testEndAddr = (char *) vramEndAddr;
		*testEndAddr = 0xa5;
		if (*testAddr != 0x5a) {
			rc = -1;
			break;
		}
		*(testEndAddr+1) = 0x00;
		if (*testEndAddr != 0xa5)
			break;
	} /* endfor */
	switch (i) {
	case 1:
		break;
	case 2:
		break;
	case 4:
		break;
	default:
		rc = -1;
		break;
	} /* endswitch */

/*
	for (; (vramAddr<vramEndAddr) && (!rc); vramAddr += ONE_MEG)
		rc = test_next_meg(vramAddr);
*/
	test_next_meg(vramAddr);
#if 0
	for (j=0; j<i; j++) {
		rc = test_next_meg(vramAddr);
		vramAddr += ONE_MEG;
		if ( rc < 0 ) 
			break;
	}
#endif
	return rc;
}

test_next_meg(vramaddr)
ulong vramaddr;
{
	int i, rc = 0;
	ulong_t startaddr;
	ulong_t *write_addr, *read_addr;
	ulong_t pattern, data;

	pattern = BLACK;
	do {
		read_addr = write_addr = vramaddr;
		for(i=0; i < (ONE_MEG/sizeof(ulong_t)) ; i++) {
			*write_addr = pattern;
			data = *read_addr;	
			if ( data != pattern ) {
				rc = -1;
			}
			write_addr += 1;
			read_addr += 1;
		}
		pattern += 0x01010101;	/* Change to next color */	
	} while ( (pattern <= 0x09090909) && (rc == 0 ) );
	return rc;
}
