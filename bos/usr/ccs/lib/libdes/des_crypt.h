/* @(#)16	1.3  src/des/libdes/des_crypt.h, libdes, des411, 9427A410a 1/17/94 14:43:36 */
/* 
 * COMPONENT_NAME: (LIBDES) Data Encryption Standard Library
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

#ifndef _H_LIBDES_DES_CRYPT
#define _H_LIBDES_DES_CRYPT

#define DES_MAXDATA 8192	/* max bytes encrypted in one call */
#define DES_DIRMASK (1 << 0)
#define DES_ENCRYPT (0*DES_DIRMASK)	/* Encrypt */
#define DES_DECRYPT (1*DES_DIRMASK)	/* Decrypt */


#define DES_DEVMASK (1 << 1)
#define	DES_HW (0*DES_DEVMASK)	/* Use hardware device */ 
#define DES_SW (1*DES_DEVMASK)	/* Use software device */


#define DESERR_NONE 0	/* succeeded */
#define DESERR_NOHWDEVICE 1	/* succeeded, but hw device not available */
#define DESERR_HWERROR 2	/* failed, hardware/driver error */
#define DESERR_BADPARAM 3	/* failed, bad parameter to call */

#define DES_FAILED(err) \
	((err) > DESERR_NOHWDEVICE)

/*
 * cbc_crypt()
 * ecb_crypt()
 *
 * Encrypt (or decrypt) len bytes of a buffer buf.
 * The length must be a multiple of eight.
 * The key should have odd parity in the low bit of each byte.
 * ivec is the input vector, and is updated to the new one (cbc only).
 * The mode is created by oring together the appropriate parameters.
 * DESERR_NOHWDEVICE is returned if DES_HW was specified but
 * there was no hardware to do it on (the data will still be
 * encrypted though, in software).
 */


/*
 * Cipher Block Chaining mode
 */
extern cbc_crypt(/* key, buf, len, mode, ivec */); /*
	char *key;	
	char *buf;
	unsigned len;
	unsigned mode;
	char *ivec;	
*/ 


/*
 * Electronic Code Book mode
 */
extern ecb_crypt(/* key, buf, len, mode */); /*
	char *key;	
	char *buf;
	unsigned len;
	unsigned mode;
*/


#ifndef _KERNEL
/* 
 * Set des parity for a key.
 * DES parity is odd and in the low bit of each byte
 */
extern void
des_setparity(/* key */); /*
	char *key;	
*/
#endif

#endif /*_H_LIBDES_DES_CRYPT*/
