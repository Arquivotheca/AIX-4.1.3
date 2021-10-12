/* @(#)33	1.1  src/bos/usr/ccs/lib/libc/des_crnl.h, libcdes, bos410, 9415B410a 4/26/90 22:16:06 */
/* 
 * COMPONENT_NAME: (LIBCDES) Data Encryption Standard Library
 * 
 * FUNCTIONS: DES_FAILED 
 *
 * ORIGINS: 24 
 *
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

 /*
 * des_crypt.h, des library routine interface
 */

#define DES_DIRMASK (1 << 0)
#define DES_ENCRYPT (0*DES_DIRMASK)	/* Encrypt */
#define DES_DECRYPT (1*DES_DIRMASK)	/* Decrypt */
#define DES_DEVMASK (1 << 1)
#define	DES_HW (0*DES_DEVMASK)	/* Use hardware device */ 
#define DES_SW (1*DES_DEVMASK)	/* Use software device */
#define DESERR_NOHWDEVICE 1	/* succeeded, but hw device not available */

#define DES_FAILED(err) \
	((err) > DESERR_NOHWDEVICE)

extern cbc_crypt();
extern ecb_crypt();

#ifndef _KERNEL
extern void
des_setparity();
#endif
