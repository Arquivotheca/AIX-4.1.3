static char sccsid[] = "@(#)47  1.15  src/bos/usr/ccs/lib/libc/crypt.c, libcs, bos411, 9428A410j 11/22/93 11:26:22";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: crypt, _crypt, encrypt, _encrypt, setkey, _setkey
 *
 * ORIGINS: 3 26 27 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 *
 *(c)Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 *ALL RIGHTS RESERVED
 */

#ifdef _DES_
#ifdef _THREAD_SAFE
#define _setkey_r	__setkey_r
#define _crypt_r	__crypt_r
#define _encrypt_r	__encrypt_r
#else /* _THREAD_SAFE */
#define _setkey		__setkey
#define _crypt		__crypt
#define _encrypt	__encrypt
#endif /* _THREAD_SAFE */
#endif /* _DES_ */

#ifdef _THREAD_SAFE
#include <crypt.h>
#define NULL 0
#endif /* _THREAD_SAFE */

#ifndef _DES_
#include <sys/errno.h>
#endif /* _DES_ */

/*
 * This program implements a data encryption algorithm to _encrypt passwords.
 */

static int tr();

typedef char C16[16];
typedef char C24[24];
typedef char C28[28];
typedef char C32[32];
typedef char C48[48];
typedef char C64[64];

/* Moves n bytes from b to a, if a and b are well behaved */
/* Most compilers will be fooled into doing block moves */
#define Move(a,b,n) {struct foo {char x[n];}; \
			*((struct foo *)a) = *((struct foo *)b); }

/* Xor's the n bytes at a with the n bytes at b */
#define Xor(a,b,n) { \
	char *p=(char *)a, *q=(char *)b, *pe=(&((char *)a)[n]); \
	while (p<pe) *p++ ^= *q++;}

/* Initial permutation, */
static C64 IP = {
	57,49,41,33,25,17, 9, 1,
	59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5,
	63,55,47,39,31,23,15, 7,
	56,48,40,32,24,16, 8, 0,
	58,50,42,34,26,18,10, 2,
	60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6};

/* * Final permutation, FP = IP^(-1) */
static C64 FP = {
	39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30,
	37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28,
	35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26,
	33, 1,41, 9,49,17,57,25,
	32, 0,40, 8,48,16,56,24};

/* Permuted-choice 1 from the key bits to yield C and D.
 Note that bits 8,16... are left out: They are intended for a parity check. */
static C28 PC1_C = {
	56,48,40,32,24,16, 8,
	 0,57,49,41,33,25,17,
	 9, 1,58,50,42,34,26,
	18,10, 2,59,51,43,35};

static C28 PC1_D = {
	62,54,46,38,30,22,14,
	 6,61,53,45,37,29,21,
	13, 5,60,52,44,36,28,
	20,12, 4,27,19,11, 3};

/* Sequence of shifts used for the key schedule. */
static C16 shifts = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

/* Permuted-choice 2, to pick out the bits from
   the CD array that generate the key schedule. */
static C24 PC2_C = {
	13,16,10,23, 0, 4,
	 2,27,14, 5,20, 9,
	22,18,11, 3,25, 7,
	15, 6,26,19,12, 1};

static C24 PC2_D = {
	40,51,30,36,46,54,
	29,39,50,44,32,47,
	43,48,38,55,33,52,
	45,41,49,35,28,31};

/* The E bit-selection table. */
static C48 E, e = {
	31, 0, 1, 2, 3, 4,
	 3, 4, 5, 6, 7, 8,
	 7, 8, 9,10,11,12,
	11,12,13,14,15,16,
	15,16,17,18,19,20,
	19,20,21,22,23,24,
	23,24,25,26,27,28,
	27,28,29,30,31, 0};

#ifndef _THREAD_SAFE
/* The key schedule. Generated from the key. */
static C48 KS[16];
#endif /* _THREAD_SAFE */

/*
 * NAME:	_setkey
 *                                                                    
 * FUNCTION:	Set a key into the crypt 'machine'.
 *                                                                    
 * NOTES:
 */  

/* Set up the key schedule from the key. */

#ifndef _DES_
static
#endif /* _DES_ */
#ifdef _THREAD_SAFE
int
_setkey_r(const char *key, CRYPTD *cd)
#else /* _THREAD_SAFE */
void
_setkey(const char *key)
#endif /* _THREAD_SAFE */
{
	int i;
	/* The C and D arrays used to calculate the key schedule. */
	C28 C,D;

#ifdef _THREAD_SAFE
	char *E;
	char (*KS)[48];
	if (cd == (CRYPTD *)NULL)
		return -1;
	E = cd->E;
	KS = cd->KS;
#endif /* _THREAD_SAFE */

	/* Generate C and D by permuting the key.  The low order bit of each
	   8-bit char is not used, so C and D are only 28 bits apiece. */
	Move(C,PC1_C,28); tr(C,key,28);
	Move(D,PC1_D,28); tr(D,key,28);

	/* To generate Ki, rotate C and D according
	   to schedule and pick up a permutation using PC2. */
	for (i=0; i<16; i++) {
		/* rotate. */
		int k;
		for (k=0; k<shifts[i]; k++) {
			int j,t1,t2;
			t1=C[0]; t2=D[0];
			for (j=0; j<27; j++) {C[j]=C[j+1]; D[j]=D[j+1];}
			C[27]=t1; D[27]=t2;}

		/* get Ki. Note C and D are concatenated. */
		for (k=0; k<24; k++) {
			KS[i][k]=PC2_C[k]; 
			KS[i][k+24]=PC2_D[k]-28;}
		tr(KS[i],C,24); tr(&KS[i][24],D,24);}

	Move(E,e,48);}

/* The 8 selection functions. For some reason, they give a 0-origin
   index, unlike everything else. (Until Lee changed the others). */
static C64 S[8] = {{
	14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	 0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	 4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13},

	{15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	 3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	 0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9},

	{10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	 1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12},

	{7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	 3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14},

	{2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	 4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3},

	{12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	 9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	 4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13},

	{ 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	 1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	 6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12},

	{13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	 1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	 7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	 2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11}};

/* P is a permutation on the selected combination of the current L and key. */
static C32 P = {
	15, 6,19,20, 28,11,27,16,
	 0,14,22,25, 4,17,30, 9,
	 1, 7,23,13, 31,26, 2, 8,
	18,12,29, 5, 21,10, 3,24};

#define L work
#define R (&work[32])

/*
 * NAME:	_encrypt
 *                                                                    
 * FUNCTION:	Encrypt/decrypt a block.
 *                                                                    
 * NOTES:
 */  

#ifdef RETURN
#undef RETURN
#endif /* RETURN */

#ifdef _THREAD_SAFE
#define RETURN(__a)	return(__a)
#else /* _THREAD_SAFE */
#define RETURN(_a)	return
#endif /* _THREAD_SAFE */

/* The payoff: _encrypt a block. */

#ifndef _DES_
static
#endif /* _DES_ */
#ifdef _THREAD_SAFE
int
_encrypt_r(char block[64], int edflag, CRYPTD *cd)
#else /* _THREAD_SAFE */
void
_encrypt(char block[64], int edflag)
#endif /* _THREAD_SAFE */
{
	int j,i,ii;
	/* Work is current block, divided into 2 halves. */
	/* PreS is combination of the key and the input, before selection. */
	C64 work; C48 preS; C32 tempL,f;

#ifdef _THREAD_SAFE
	char *E;
	char (*KS)[48];
	if (cd == (CRYPTD *)NULL)
		RETURN(-1);
	E = cd->E;
	KS = cd->KS;
#endif /* _THREAD_SAFE */


#ifndef _DES_
	/**********
	** LIBC VERSION: if edflag is set (do decrypt) set errno and return
	**********/
	if (edflag) {
		errno=ENOSYS;
		RETURN(-1);
	}
#endif /* _DES_ */

	/* First, permute the bits in the input */

	Move(work,IP,64); tr(work,block,64);

	/* Perform an encryption operation 16 times. */
	for (ii=0; ii<16; ii++) {
		/* Set direction */
#ifdef _DES_
		if (edflag)
			i = 15-ii;
		else
#endif /* _DES_ */
			i = ii;

		/* Save the R array, which will be the new L. */
		Move(tempL,R,32);

		/* Expand R to 48 bits using the E selector;
		   exclusive-or with the current key bits. */
		Move(preS,E,48); tr(preS,R,48); Xor(preS,KS[i],48);

		/* The pre-select bits are now considered in 8 groups of 
		 * 6 bits each.  The 8 selection functions map these
		 * 6-bit quantities into 4-bit quantities and the results
		 * permuted to make an f(R, K).
		 * The indexing into the selection functions is peculiar;
		 * it could be simplified by rewriting the tables. */
		for (j=0; j<8; j++) {
			char *p = &preS[6*j];
			int k = 0;
			if (*p++) k|=0x20; 
			if (*p++) k|=0x08;
			if (*p++) k|=0x04;
			if (*p++) k|=0x02;
			if (*p++) k|=0x01;
			if (*p)   k|=0x10;
			k = S[j][k];
			p = &f[4*(j+1)];
			*--p = (k&1)!=0;
			*--p = (k&2)!=0;
			*--p = (k&4)!=0;
			*--p = (k&8)!=0;}

		/* The new R is L ^ f(R, K).
		   The f here has to be permuted first, though. */
		Move(R,P,32); tr(R,f,32); Xor(R,L,32);

		/* Finally, the new L (the original R) is copied back. */
		Move(L,tempL,32);}

	/* The output L and R are reversed. */
	{C32 t; Move(t,L,32); Move(L,R,32); Move(R,t,32);}

	/* The final output gets the inverse permutation of the original. */
	Move(block,FP,64); tr(block,work,64);

	RETURN(0);
}

/*
 * NAME:	_crypt
 *                                                                    
 * FUNCTION:	Encrypt a password.
 *                                                                    
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:	the encrypted password
 */  
#ifndef _DES_
static
#endif /* _DES_ */
#ifdef _THREAD_SAFE
char *
_crypt_r(const char *pw, const char *salt, CRYPTD *cd)
#else /* _THREAD_SAFE */
char *
_crypt(const char *pw, const char *salt)
#endif /* _THREAD_SAFE */
{
	int i, j, c;
	int temp;
#ifndef _THREAD_SAFE
	static char block[66], iobuf[16];
#else /* _THREAD_SAFE */
	char *E;
	char *block;
	char *iobuf;
	if (cd == (CRYPTD *)NULL)
		return(0);
	E = cd->E;
	block = cd->block;
	iobuf = cd->iobuf;
#endif /* _THREAD_SAFE */

	for(i=0; i < 66; i++)
		block[i] = 0;
	for(i=0; (c= *pw) && i < 64; pw++) {
		for(j=0; j < 7; j++, i++)
			block[i] = (c>>(6-j)) & 01;
		i++;
	}
	
#ifdef _THREAD_SAFE
	_setkey_r(block, cd);
#else /* _THREAD_SAFE */
	_setkey(block);
#endif /* _THREAD_SAFE */
	
	for(i=0; i < 66; i++)
		block[i] = 0;

	for(i=0; i < 2; i++) {
		c = *salt++;
		iobuf[i] = c;
		if(c > 'Z')
			c -= 6;
		if(c > '9')
			c -= 7;
		c -= '.';
		for(j=0; j < 6; j++) {
			if((c>>j) & 01) {
				int k=6*i+j;
				temp = E[k];
				E[k] = E[k+24];
				E[k+24] = temp;
			}
		}
	}
	
	for(i=0; i < 25; i++)
#ifdef _THREAD_SAFE
		_encrypt_r(block,0,cd);
#else /* _THREAD_SAFE */
		_encrypt(block,0);
#endif /* _THREAD_SAFE */
	
	for(i=0; i < 11; i++) {
		c = 0;
		for(j=0; j < 6; j++) {
			c <<= 1;
			c |= block[6*i+j];
		}
		c += '.';
		if(c > '9')
			c += 7;
		if(c > 'Z')
			c += 6;
		iobuf[i+2] = c;
	}
	iobuf[i+2] = 0;
	if(iobuf[1] == 0)
		iobuf[1] = iobuf[0];
	return(iobuf);
}

static 
tr(char *dst,char *tab,int cnt)
{
	char *rdst=dst, *rtab=tab, *rend=rdst+cnt;
	while (rdst<rend) {*rdst = rtab[*rdst]; rdst++;}
}

#ifndef _DES_
static void __load_crypt_functions(void);
static __setup=0;

#ifdef _THREAD_SAFE
extern char *__crypt_r();
extern int __encrypt_r();
extern int __setkey_r();
static char *(*crypt_function)() = _crypt_r;
static int (*setkey_function)() = _setkey_r;
static int (*encrypt_function)() = _encrypt_r;
extern struct rec_mutex _crypt_rmutex;
#else /* _THREAD_SAFE */
extern char *__crypt();
extern void __encrypt();
extern void __setkey();
static char *(*crypt_function)() = _crypt;
static void (*setkey_function)() = _setkey;
static void (*encrypt_function)() = _encrypt;
#endif /* _THREAD_SAFE */

/*
 * NAME:	crypt
 *                                                                    
 * FUNCTION:	If libdes is found, then load and run __crypt from libdes,
 * 		otherwise run _crypt from libc.
 *                                                                    
 * NOTES:
 */  

#ifdef _THREAD_SAFE
char *
crypt_r(const char *pw, const char *salt, CRYPTD *cd)
#else /* _THREAD_SAFE */
char *
crypt(const char *pw, const char *salt)
#endif /* _THREAD_SAFE */
{

#ifdef _THREAD_SAFE
	_rec_mutex_lock(&_crypt_rmutex);
#endif /* _THREAD_SAFE */
	if (!__setup) {
		__setup=1;
		__load_crypt_functions();
	}
#ifdef _THREAD_SAFE
	_rec_mutex_unlock(&_crypt_rmutex);
	return( (*crypt_function)(pw, salt,cd) );
#else /* _THREAD_SAFE */
	return( (*crypt_function)(pw, salt) );
#endif /* _THREAD_SAFE */
}

/*
 * NAME:	setkey
 *                                                                    
 * FUNCTION:	If libdes is found, then load and run __setkey from libdes,
 * 		otherwise run _setkey from libc.
 *
 * NOTES:
 */  

#ifdef _THREAD_SAFE
int
setkey_r(const char *key, CRYPTD *cd)
#else /* _THREAD_SAFE */
void
setkey(const char *key)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	_rec_mutex_lock(&_crypt_rmutex);
#endif /* _THREAD_SAFE */
	if (!__setup) {
		__setup=1;
		__load_crypt_functions();
	}
#ifdef _THREAD_SAFE
	_rec_mutex_unlock(&_crypt_rmutex);
	(*setkey_function)(key, cd);
#else /* _THREAD_SAFE */
	(*setkey_function)(key);

#endif /* _THREAD_SAFE */
}

/*
 * NAME:	encrypt
 *                                                                    
 * FUNCTION:	If libdes is found, then load and run __encrypt from libdes,
 * 		otherwise run _encrypt from libc.
 *
 *                                                                    
 * NOTES:
 */  

#ifdef _THREAD_SAFE
int
encrypt_r(char block[64], int edflag, CRYPTD *cd)
#else /* _THREAD_SAFE */
void
encrypt(char block[64], int edflag)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	_rec_mutex_lock(&_crypt_rmutex);
#endif /* _THREAD_SAFE */
	if (!__setup) {
		__setup=1;
		__load_crypt_functions();
	}
#ifdef _THREAD_SAFE
	_rec_mutex_unlock(&_crypt_rmutex);
	(*encrypt_function)(block, edflag, cd);
#else /* _THREAD_SAFE */
	(*encrypt_function)(block, edflag);
#endif /* _THREAD_SAFE */
}

static
void
__load_crypt_functions()
{

	int *load_rc;
	int loadbind_rc;

	/**********
	  try to load libdes, if this fails, then just use the ones
	  in libc
	**********/
#ifdef _THREAD_SAFE
	load_rc = load("libdes_r", 1 ,"/usr/lib");
#else /* _THREAD_SAFE */
	load_rc = load("libdes", 1 ,"/usr/lib");
#endif /* _THREAD_SAFE */
	if (load_rc == (int *) 0)
		return;

	/**********
	  Bind in any exported symbols from libdes, If this fails, unload
	  libdes and use the functions in libc
	**********/
#ifdef _THREAD_SAFE
	loadbind_rc = loadbind(0, _crypt_r, load_rc);
#else /* _THREAD_SAFE */
	loadbind_rc = loadbind(0, _crypt, load_rc);
#endif /* _THREAD_SAFE */
	if (loadbind_rc == -1) {
		(void) unload(load_rc);
		return;
	}

	/**********
	  libdes is loaded, set up the function pointers and return
	**********/
#ifdef _THREAD_SAFE
	crypt_function = __crypt_r;
	setkey_function = __setkey_r;
	encrypt_function = __encrypt_r;
#else /* _THREAD_SAFE */
	crypt_function = __crypt;
	setkey_function = __setkey;
	encrypt_function = __encrypt;
#endif /* _THREAD_SAFE */

	return;

}
#endif /* _DES_ */
