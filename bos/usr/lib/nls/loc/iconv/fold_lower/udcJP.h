/* @(#)61	1.6  src/bos/usr/lib/nls/loc/iconv/fold_lower/udcJP.h, cmdiconv, bos411, 9428A410j 11/13/91 22:40:43
 *
 * COMPONENT_NAME: (CMDICONV)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __udcJP_h
#define __udcJP_h

#define ASCII	1
#define KANA	2
#define JIS	3
#define UDC	4
#define IBMSEL	5
#define INVALID 6

static int	index[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 2, 2, 2
	};


static unsigned short	ibmsel_pctoudc[] = {
	0x7921, 0x7922, 0x7923, 0x7924, 0x7925, 0x7926, 0x7927, 0x7928,
	0x7929, 0x792a, 0x792b, 0x792c, 0x792d, 0x792e, 0x792f, 0x7930,
	0x7931, 0x7932, 0x7933, 0x7934, 0x0000, 0x7936, 0x7937, 0x7938,
	0x7939, 0x793a, 0x793b, 0x0000, 0x793d, 0x793e, 0x793f, 0x7940,
	0x7941, 0x7942, 0x7943, 0x7944, 0x7945, 0x7946, 0x7947, 0x7948,
	0x7949, 0x794a, 0x794b, 0x794c, 0x794d, 0x794e, 0x794f, 0x7950,
	0x7951, 0x7952, 0x7953, 0x7954, 0x7955, 0x7956, 0x7957, 0x7958,
	0x7959, 0x795a, 0x795b, 0x795c, 0x795d, 0x795e, 0x795f, 0x7960,
	0x7961, 0x7962, 0x7963, 0x7964, 0x7965, 0x7966, 0x7967, 0x7968,
	0x7969, 0x796a, 0x796b, 0x796c, 0x796d, 0x796e, 0x796f, 0x7970,
	0x7971, 0x7972, 0x7973, 0x7974, 0x7975, 0x7976, 0x7977, 0x7978,
	0x7979, 0x797a, 0x797b, 0x797c, 0x797d, 0x797e,
	0x7a21, 0x7a22, 0x7a23, 0x7a24, 0x7a25, 0x7a26, 0x7a27, 0x7a28,
	0x7a29, 0x7a2a, 0x7a2b, 0x7a2c, 0x7a2d, 0x7a2e, 0x7a2f, 0x7a30,
	0x7a31, 0x7a32, 0x7a33, 0x7a34, 0x7a35, 0x7a36, 0x7a37, 0x7a38,
	0x7a39, 0x7a3a, 0x7a3b, 0x7a3c, 0x7a3d, 0x7a3e, 0x7a3f, 0x7a40,
	0x7a41, 0x7a42, 0x7a43, 0x7a44, 0x7a45, 0x7a46, 0x7a47, 0x7a48,
	0x7a49, 0x7a4a, 0x7a4b, 0x7a4c, 0x7a4d, 0x7a4e, 0x7a4f, 0x7a50,
	0x7a51, 0x7a52, 0x7a53, 0x7a54, 0x7a55, 0x7a56, 0x7a57, 0x7a58,
	0x7a59, 0x7a5a, 0x7a5b, 0x7a5c, 0x7a5d, 0x7a5e, 0x7a5f, 0x7a60,
	0x7a61, 0x7a62, 0x7a63, 0x7a64, 0x7a65, 0x7a66, 0x7a67, 0x7a68,
	0x7a69, 0x7a6a, 0x7a6b, 0x7a6c, 0x7a6d, 0x7a6e, 0x7a6f, 0x7a70,
	0x7a71, 0x7a72, 0x7a73, 0x7a74, 0x7a75, 0x7a76, 0x7a77, 0x7a78,
	0x7a79, 0x7a7a, 0x7a7b, 0x7a7c, 0x7a7d, 0x7a7e,
	0x7b21, 0x7b22, 0x7b23, 0x7b24, 0x7b25, 0x7b26, 0x7b27, 0x7b28,
	0x7b29, 0x7b2a, 0x7b2b, 0x7b2c, 0x7b2d, 0x7b2e, 0x7b2f, 0x7b30,
	0x7b31, 0x7b32, 0x7b33, 0x7b34, 0x7b35, 0x7b36, 0x7b37, 0x7b38,
	0x7b39, 0x7b3a, 0x7b3b, 0x7b3c, 0x7b3d, 0x7b3e, 0x7b3f, 0x7b40,
	0x7b41, 0x7b42, 0x7b43, 0x7b44, 0x7b45, 0x7b46, 0x7b47, 0x7b48,
	0x7b49, 0x7b4a, 0x7b4b, 0x7b4c, 0x7b4d, 0x7b4e, 0x7b4f, 0x7b50,
	0x7b51, 0x7b52, 0x7b53, 0x7b54, 0x7b55, 0x7b56, 0x7b57, 0x7b58,
	0x7b59, 0x7b5a, 0x7b5b, 0x7b5c, 0x7b5d, 0x7b5e, 0x7b5f, 0x7b60,
	0x7b61, 0x7b62, 0x7b63, 0x7b64, 0x7b65, 0x7b66, 0x7b67, 0x7b68,
	0x7b69, 0x7b6a, 0x7b6b, 0x7b6c, 0x7b6d, 0x7b6e, 0x7b6f, 0x7b70,
	0x7b71, 0x7b72, 0x7b73, 0x7b74, 0x7b75, 0x7b76, 0x7b77, 0x7b78,
	0x7b79, 0x7b7a, 0x7b7b, 0x7b7c, 0x7b7d, 0x7b7e,
	0x7c21, 0x7c22, 0x7c23, 0x7c24, 0x7c25, 0x7c26, 0x7c27, 0x7c28,
	0x7c29, 0x7c2a, 0x7c2b, 0x7c2c, 0x7c2d, 0x7c2e, 0x7c2f, 0x7c30,
	0x7c31, 0x7c32, 0x7c33, 0x7c34, 0x7c35, 0x7c36, 0x7c37, 0x7c38,
	0x7c39, 0x7c3a, 0x7c3b, 0x7c3c, 0x7c3d, 0x7c3e, 0x7c3f, 0x7c40,
	0x7c41, 0x7c42, 0x7c43, 0x7c44, 0x7c45, 0x7c46, 0x7c47, 0x7c48,
	0x7c49, 0x7c4a, 0x7c4b, 0x7c4c, 0x7c4d, 0x7c4e, 0x7c4f, 0x7c50,
	0x7c51, 0x7c52, 0x7c53, 0x7c54, 0x7c55, 0x7c56, 0x7c57, 0x7c58,
	0x7c59, 0x7c5a, 0x7c5b, 0x7c5c, 0x7c5d, 0x7c5e, 0x7c5f, 0x7c60,
	0x7c61, 0x7c62, 0x7c63, 0x7c64, 0x7c65, 0x7c66, 0x7c67, 0x7c68,
	0x7c69, 0x7c6a, 0x7c6b, 0x7c6c, 0x7c6d, 0x7c6e, 0x7c6f, 0x7c70,
	0x7c71, 0x7c72, 0x7c73, 0x7c74, 0x7c75, 0x7c76, 0x7c77, 0x7c78,
	0x7c79, 0x7c7a, 0x7c7b, 0x7c7c, 0x7c7d, 0x7c7e,
	0x7d21, 0x7d22, 0x7d23, 0x7d24, 0x7d25, 0x7d26, 0x7d27, 0x7d28,
	0x7d29, 0x7d2a, 0x7d2b, 0x7d2c, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};


static unsigned short	ibmsel_udctopc[] = {
	0xfa40, 0xfa41, 0xfa42, 0xfa43, 0xfa44, 0xfa45, 0xfa46, 0xfa47,
	0xfa48, 0xfa49, 0xfa4a, 0xfa4b, 0xfa4c, 0xfa4d, 0xfa4e, 0xfa4f,
	0xfa50, 0xfa51, 0xfa52, 0xfa53, 0x0000, 0xfa55, 0xfa56, 0xfa57,
	0xfa58, 0xfa59, 0xfa5a, 0x0000, 0xfa5c, 0xfa5d, 0xfa5e, 0xfa5f,
	0xfa60, 0xfa61, 0xfa62, 0xfa63, 0xfa64, 0xfa65, 0xfa66, 0xfa67,
	0xfa68, 0xfa69, 0xfa6a, 0xfa6b, 0xfa6c, 0xfa6d, 0xfa6e, 0xfa6f,
	0xfa70, 0xfa71, 0xfa72, 0xfa73, 0xfa74, 0xfa75, 0xfa76, 0xfa77,
	0xfa78, 0xfa79, 0xfa7a, 0xfa7b, 0xfa7c, 0xfa7d, 0xfa7e, 0xfa80,
	0xfa81, 0xfa82, 0xfa83, 0xfa84, 0xfa85, 0xfa86, 0xfa87, 0xfa88,
	0xfa89, 0xfa8a, 0xfa8b, 0xfa8c, 0xfa8d, 0xfa8e, 0xfa8f, 0xfa90,
	0xfa91, 0xfa92, 0xfa93, 0xfa94, 0xfa95, 0xfa96, 0xfa97, 0xfa98,
	0xfa99, 0xfa9a, 0xfa9b, 0xfa9c, 0xfa9d, 0xfa9e,
	0xfa9f, 0xfaa0, 0xfaa1, 0xfaa2, 0xfaa3, 0xfaa4, 0xfaa5, 0xfaa6,
	0xfaa7, 0xfaa8, 0xfaa9, 0xfaaa, 0xfaab, 0xfaac, 0xfaad, 0xfaae,
	0xfaaf, 0xfab0, 0xfab1, 0xfab2, 0xfab3, 0xfab4, 0xfab5, 0xfab6,
	0xfab7, 0xfab8, 0xfab9, 0xfaba, 0xfabb, 0xfabc, 0xfabd, 0xfabe,
	0xfabf, 0xfac0, 0xfac1, 0xfac2, 0xfac3, 0xfac4, 0xfac5, 0xfac6,
	0xfac7, 0xfac8, 0xfac9, 0xfaca, 0xfacb, 0xfacc, 0xfacd, 0xface,
	0xfacf, 0xfad0, 0xfad1, 0xfad2, 0xfad3, 0xfad4, 0xfad5, 0xfad6,
	0xfad7, 0xfad8, 0xfad9, 0xfada, 0xfadb, 0xfadc, 0xfadd, 0xfade,
	0xfadf, 0xfae0, 0xfae1, 0xfae2, 0xfae3, 0xfae4, 0xfae5, 0xfae6,
	0xfae7, 0xfae8, 0xfae9, 0xfaea, 0xfaeb, 0xfaec, 0xfaed, 0xfaee,
	0xfaef, 0xfaf0, 0xfaf1, 0xfaf2, 0xfaf3, 0xfaf4, 0xfaf5, 0xfaf6,
	0xfaf7, 0xfaf8, 0xfaf9, 0xfafa, 0xfafb, 0xfafc,
	0xfb40, 0xfb41, 0xfb42, 0xfb43, 0xfb44, 0xfb45, 0xfb46, 0xfb47,
	0xfb48, 0xfb49, 0xfb4a, 0xfb4b, 0xfb4c, 0xfb4d, 0xfb4e, 0xfb4f,
	0xfb50, 0xfb51, 0xfb52, 0xfb53, 0xfb54, 0xfb55, 0xfb56, 0xfb57,
	0xfb58, 0xfb59, 0xfb5a, 0xfb5b, 0xfb5c, 0xfb5d, 0xfb5e, 0xfb5f,
	0xfb60, 0xfb61, 0xfb62, 0xfb63, 0xfb64, 0xfb65, 0xfb66, 0xfb67,
	0xfb68, 0xfb69, 0xfb6a, 0xfb6b, 0xfb6c, 0xfb6d, 0xfb6e, 0xfb6f,
	0xfb70, 0xfb71, 0xfb72, 0xfb73, 0xfb74, 0xfb75, 0xfb76, 0xfb77,
	0xfb78, 0xfb79, 0xfb7a, 0xfb7b, 0xfb7c, 0xfb7d, 0xfb7e, 0xfb80,
	0xfb81, 0xfb82, 0xfb83, 0xfb84, 0xfb85, 0xfb86, 0xfb87, 0xfb88,
	0xfb89, 0xfb8a, 0xfb8b, 0xfb8c, 0xfb8d, 0xfb8e, 0xfb8f, 0xfb90,
	0xfb91, 0xfb92, 0xfb93, 0xfb94, 0xfb95, 0xfb96, 0xfb97, 0xfb98,
	0xfb99, 0xfb9a, 0xfb9b, 0xfb9c, 0xfb9d, 0xfb9e,
	0xfb9f, 0xfba0, 0xfba1, 0xfba2, 0xfba3, 0xfba4, 0xfba5, 0xfba6,
	0xfba7, 0xfba8, 0xfba9, 0xfbaa, 0xfbab, 0xfbac, 0xfbad, 0xfbae,
	0xfbaf, 0xfbb0, 0xfbb1, 0xfbb2, 0xfbb3, 0xfbb4, 0xfbb5, 0xfbb6,
	0xfbb7, 0xfbb8, 0xfbb9, 0xfbba, 0xfbbb, 0xfbbc, 0xfbbd, 0xfbbe,
	0xfbbf, 0xfbc0, 0xfbc1, 0xfbc2, 0xfbc3, 0xfbc4, 0xfbc5, 0xfbc6,
	0xfbc7, 0xfbc8, 0xfbc9, 0xfbca, 0xfbcb, 0xfbcc, 0xfbcd, 0xfbce,
	0xfbcf, 0xfbd0, 0xfbd1, 0xfbd2, 0xfbd3, 0xfbd4, 0xfbd5, 0xfbd6,
	0xfbd7, 0xfbd8, 0xfbd9, 0xfbda, 0xfbdb, 0xfbdc, 0xfbdd, 0xfbde,
	0xfbdf, 0xfbe0, 0xfbe1, 0xfbe2, 0xfbe3, 0xfbe4, 0xfbe5, 0xfbe6,
	0xfbe7, 0xfbe8, 0xfbe9, 0xfbea, 0xfbeb, 0xfbec, 0xfbed, 0xfbee,
	0xfbef, 0xfbf0, 0xfbf1, 0xfbf2, 0xfbf3, 0xfbf4, 0xfbf5, 0xfbf6,
	0xfbf7, 0xfbf8, 0xfbf9, 0xfbfa, 0xfbfb, 0xfbfc,
	0xfc40, 0xfc41, 0xfc42, 0xfc43, 0xfc44, 0xfc45, 0xfc46, 0xfc47,
	0xfc48, 0xfc49, 0xfc4a, 0xfc4b, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};


static unsigned short	ibmsel_udctoeuc[] = {
	0xf3a1, 0xf3a2, 0xf3a3, 0xf3a4, 0xf3a5, 0xf3a6, 0xf3a7, 0xf3a8,
	0xf3a9, 0xf3aa, 0xf3ab, 0xf3ac, 0xf3ad, 0xf3ae, 0xf3af, 0xf3b0,
	0xf3b1, 0xf3b2, 0xf3b3, 0xf3b4, 0xf4fe, 0xa2c3, 0xf3b5, 0xf3b6,
	0xf3b7, 0xf3b8, 0xf3b9, 0xf4fe, 0xd4e3, 0xdcdf, 0xe4e9, 0xe3f8,
	0xd9a1, 0xb1bb, 0xf4a1, 0xc2ad, 0xc3fc, 0xe4d0, 0xc2bf, 0xbcf4,
	0xb0a9, 0xb0c8, 0xf4a2, 0xb0d2, 0xb0d4, 0xb0e3, 0xb0ee, 0xb1a7,
	0xb1a3, 0xb1ac, 0xb1a9, 0xb1be, 0xb1df, 0xb1d8, 0xb1c8, 0xb1d7,
	0xb1e3, 0xb1f4, 0xb1e1, 0xb2a3, 0xf4a3, 0xb2bb, 0xb2e6, 0xb2ed,
	0xb2f5, 0xb2fc, 0xf4a4, 0xb3b5, 0xb3d8, 0xb3db, 0xb3e5, 0xb3ee,
	0xb3fb, 0xf4a5, 0xf4a6, 0xb4c0, 0xb4c7, 0xb4d0, 0xb4de, 0xf4a7,
	0xb5aa, 0xf4a8, 0xb5af, 0xb5c4, 0xb5e8, 0xf4a9, 0xb7c2, 0xb7e4,
	0xb7e8, 0xb7e7, 0xf4aa, 0xf4ab, 0xf4ac, 0xb8ce,
	0xb8e1, 0xb8f5, 0xb8f7, 0xb8f8, 0xb8fc, 0xb9af, 0xb9b7, 0xbabe,
	0xbadb, 0xcdaa, 0xbae1, 0xf4ae, 0xbaeb, 0xbbb3, 0xbbb8, 0xf4af,
	0xbbca, 0xf4b0, 0xf4b1, 0xbbd0, 0xbbde, 0xbbf4, 0xbbf5, 0xbbf9,
	0xbce4, 0xbced, 0xbcfe, 0xf4b2, 0xbdc2, 0xbde7, 0xf4b3, 0xbdf0,
	0xbeb0, 0xbeac, 0xf4b4, 0xbeb3, 0xbebd, 0xbecd, 0xbec9, 0xbee4,
	0xbfa8, 0xbfc9, 0xc0c4, 0xc0e4, 0xc0f4, 0xc1a6, 0xf4b5, 0xc1f5,
	0xc1fc, 0xf4b6, 0xc1f8, 0xc2ab, 0xc2a1, 0xc2a5, 0xf4b7, 0xc2b8,
	0xc2ba, 0xf4b8, 0xc2c4, 0xc2d2, 0xc2d7, 0xc2db, 0xc2de, 0xc2ed,
	0xc2f0, 0xf4b9, 0xc3a1, 0xc3b5, 0xc3c9, 0xc3b9, 0xf4ba, 0xc3d8,
	0xc3fe, 0xf4bb, 0xc4cc, 0xf4bc, 0xc4d9, 0xc4ea, 0xc4fd, 0xf4bd,
	0xc5a7, 0xc5b5, 0xc5b6, 0xf4be, 0xc5d5, 0xc6b8, 0xc6d7, 0xc6e0,
	0xc6ea, 0xc6e3, 0xc7a1, 0xc7ab, 0xc7c7, 0xc7c3,
	0xc7cb, 0xc7cf, 0xc7d9, 0xf4bf, 0xf4c0, 0xc7e6, 0xc7ee, 0xc7fc,
	0xc7eb, 0xc7f0, 0xc8b1, 0xc8e5, 0xc8f8, 0xc9a6, 0xc9ab, 0xc9ad,
	0xf4c2, 0xc9ca, 0xc9d3, 0xc9e9, 0xc9e3, 0xc9fc, 0xc9f4, 0xc9f5,
	0xf4c4, 0xcab3, 0xcabd, 0xcaef, 0xcaf1, 0xcbae, 0xf4c5, 0xcbca,
	0xcbe6, 0xcbea, 0xcbf0, 0xcbf4, 0xcbee, 0xcca5, 0xcbf9, 0xccab,
	0xccae, 0xccad, 0xccb2, 0xccc2, 0xccd0, 0xccd9, 0xf4c6, 0xcdbb,
	0xf4c7, 0xcebb, 0xf4c8, 0xceba, 0xcec3, 0xf4c9, 0xcef2, 0xb3dd,
	0xcfd5, 0xcfe2, 0xcfe9, 0xcfed, 0xf4cb, 0xf4cc, 0xf4cd, 0xf4ce,
	0xd0e5, 0xf4cf, 0xd0e9, 0xd1e8, 0xf4d0, 0xf4d1, 0xd1ec, 0xd2bb,
	0xf4d3, 0xd3e1, 0xd3e8, 0xd4a7, 0xf4d4, 0xf4d5, 0xd4d4, 0xd4f2,
	0xd5ae, 0xf4d6, 0xd7de, 0xf4d7, 0xd8a2, 0xd8b7, 0xd8c1, 0xd8d1,
	0xd8f4, 0xd9c6, 0xd9c8, 0xd9d1, 0xf4d8, 0xf4d9,
	0xf4da, 0xf4db, 0xf4dc, 0xdcd3, 0xddc8, 0xddd4, 0xddea, 0xddfa,
	0xdea4, 0xdeb0, 0xf4de, 0xdeb5, 0xdecb, 0xf4df, 0xdfb9, 0xf4e0,
	0xdfc3, 0xf4e1, 0xf4e2, 0xe0d9, 0xf4e3, 0xf4e4, 0xe1e2, 0xf4e5,
	0xf4e6, 0xf4e7, 0xe2c7, 0xe3a8, 0xe3a6, 0xe3a9, 0xe3af, 0xe3b0,
	0xe3aa, 0xe3ab, 0xe3bc, 0xe3c1, 0xe3bf, 0xe3d5, 0xe3d8, 0xe3d6,
	0xe3df, 0xe3e3, 0xe3e1, 0xe3d4, 0xe3e9, 0xe4a6, 0xe3f1, 0xe3f2,
	0xe4cb, 0xe4c1, 0xe4c3, 0xe4be, 0xf4e8, 0xe4c0, 0xe4c7, 0xe4bf,
	0xe4e0, 0xe4de, 0xe4d1, 0xf4e9, 0xe4dc, 0xe4d2, 0xe4db, 0xe4d4,
	0xe4fa, 0xe4ef, 0xe5b3, 0xe5bf, 0xe5c9, 0xe5d0, 0xe5e2, 0xe5ea,
	0xe5eb, 0xf4ea, 0xf4eb, 0xf4ec, 0xe6e8, 0xe6ef, 0xe7ac, 0xf4ed,
	0xe7ae, 0xf4ee, 0xe7b1, 0xf4ef, 0xe7b2, 0xe8b1, 0xe8b6, 0xf4f1,
	0xf4f2, 0xe8dd, 0xf4f3, 0xf4f4, 0xe9d1, 0xf4f5,
	0xe9ed, 0xeacd, 0xf4f6, 0xeadb, 0xeae6, 0xeaea, 0xeba5, 0xebfb,
	0xebfa, 0xf4f7, 0xecd6, 0xf4f8, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe,
	0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe, 0xf4fe
	};


static unsigned short	ibmsel_euctoudc[] = {
	0x7921, 0x7922, 0x7923, 0x7924, 0x7925, 0x7926, 0x7927, 0x7928,
	0x7929, 0x792a, 0x792b, 0x792c, 0x792d, 0x792e, 0x792f, 0x7930,
	0x7931, 0x7932, 0x7933, 0x7934, 0x7937, 0x7938, 0x7939, 0x793a,
	0x793b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x7943, 0x794b, 0x795d, 0x7963, 0x796a, 0x796b, 0x7970, 0x7972,
	0x7976, 0x797b, 0x797c, 0x797d, 0x0000, 0x7a2c, 0x7a30, 0x7a32,
	0x7a33, 0x7a3c, 0x7a3f, 0x7a43, 0x7a4f, 0x7a52, 0x7a57, 0x7a5a,
	0x7a62, 0x7a67, 0x7a6a, 0x7a6c, 0x7a70, 0x7a74, 0x7b24, 0x7b25,
	0x0000, 0x7b31, 0x0000, 0x7b39, 0x7b3f, 0x7b4f, 0x7b51, 0x7b53,
	0x7b56, 0x0000, 0x7b5d, 0x7b5e, 0x7b5f, 0x7b60, 0x7b62, 0x7b65,
	0x7b66, 0x0000, 0x7b69, 0x7b6d, 0x7b6e, 0x7b72, 0x7b74, 0x7b7d,
	0x7b7e, 0x7c21, 0x7c22, 0x7c23, 0x0000, 0x7c2b, 0x7c2e, 0x7c30,
	0x7c32, 0x7c33, 0x7c35, 0x7c36, 0x7c38, 0x7c39, 0x7c3a, 0x7c55,
	0x7c5c, 0x7c6a, 0x7c6b, 0x7c6c, 0x7c70, 0x7c72, 0x7c74, 0x0000,
	0x7c78, 0x7c79, 0x7c7b, 0x7c7c, 0x7c7e, 0x7d23, 0x7d2a, 0x7d2c,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};


static unsigned short	ibmsel0212_euctoudc[][2] = {
	{0xa2c3, 0x7936}, {0xb0a9, 0x7949}, {0xb0c8, 0x794a}, {0xb0d2, 0x794c},
	{0xb0d4, 0x794d}, {0xb0e3, 0x794e}, {0xb0ee, 0x794f}, {0xb1a3, 0x7951},
	{0xb1a7, 0x7950}, {0xb1a9, 0x7953}, {0xb1ac, 0x7952}, {0xb1bb, 0x7942},
	{0xb1be, 0x7954}, {0xb1c8, 0x7957}, {0xb1d7, 0x7958}, {0xb1d8, 0x7956},
	{0xb1df, 0x7955}, {0xb1e1, 0x795b}, {0xb1e3, 0x7959}, {0xb1f4, 0x795a},
	{0xb2a3, 0x795c}, {0xb2bb, 0x795e}, {0xb2e6, 0x795f}, {0xb2ed, 0x7960},
	{0xb2f5, 0x7961}, {0xb2fc, 0x7962}, {0xb3b5, 0x7964}, {0xb3d8, 0x7965},
	{0xb3db, 0x7966}, {0xb3dd, 0x7b58}, {0xb3e5, 0x7967}, {0xb3ee, 0x7968},
	{0xb3fb, 0x7969}, {0xb4c0, 0x796c}, {0xb4c7, 0x796d}, {0xb4d0, 0x796e},
	{0xb4de, 0x796f}, {0xb5aa, 0x7971}, {0xb5af, 0x7973}, {0xb5c4, 0x7974},
	{0xb5e8, 0x7975}, {0xb7c2, 0x7977}, {0xb7e4, 0x7978}, {0xb7e7, 0x797a},
	{0xb7e8, 0x7979}, {0xb8ce, 0x797e}, {0xb8e1, 0x7a21}, {0xb8f5, 0x7a22},
	{0xb8f7, 0x7a23}, {0xb8f8, 0x7a24}, {0xb8fc, 0x7a25}, {0xb9af, 0x7a26},
	{0xb9b7, 0x7a27}, {0xbabe, 0x7a28}, {0xbadb, 0x7a29}, {0xbae1, 0x7a2b},
	{0xbaeb, 0x7a2d}, {0xbbb3, 0x7a2e}, {0xbbb8, 0x7a2f}, {0xbbca, 0x7a31},
	{0xbbd0, 0x7a34}, {0xbbde, 0x7a35}, {0xbbf4, 0x7a36}, {0xbbf5, 0x7a37},
	{0xbbf9, 0x7a38}, {0xbce4, 0x7a39}, {0xbced, 0x7a3a}, {0xbcf4, 0x7948},
	{0xbcfe, 0x7a3b}, {0xbdc2, 0x7a3d}, {0xbde7, 0x7a3e}, {0xbdf0, 0x7a40},
	{0xbeac, 0x7a42}, {0xbeb0, 0x7a41}, {0xbeb3, 0x7a44}, {0xbebd, 0x7a45},
	{0xbec9, 0x7a47}, {0xbecd, 0x7a46}, {0xbee4, 0x7a48}, {0xbfa8, 0x7a49},
	{0xbfc9, 0x7a4a}, {0xc0c4, 0x7a4b}, {0xc0e4, 0x7a4c}, {0xc0f4, 0x7a4d},
	{0xc1a6, 0x7a4e}, {0xc1f5, 0x7a50}, {0xc1f8, 0x7a53}, {0xc1fc, 0x7a51},
	{0xc2a1, 0x7a55}, {0xc2a5, 0x7a56}, {0xc2ab, 0x7a54}, {0xc2ad, 0x7944},
	{0xc2b8, 0x7a58}, {0xc2ba, 0x7a59}, {0xc2bf, 0x7947}, {0xc2c4, 0x7a5b},
	{0xc2d2, 0x7a5c}, {0xc2d7, 0x7a5d}, {0xc2db, 0x7a5e}, {0xc2de, 0x7a5f},
	{0xc2ed, 0x7a60}, {0xc2f0, 0x7a61}, {0xc3a1, 0x7a63}, {0xc3b5, 0x7a64},
	{0xc3b9, 0x7a66}, {0xc3c9, 0x7a65}, {0xc3d8, 0x7a68}, {0xc3fc, 0x7945},
	{0xc3fe, 0x7a69}, {0xc4cc, 0x7a6b}, {0xc4d9, 0x7a6d}, {0xc4ea, 0x7a6e},
	{0xc4fd, 0x7a6f}, {0xc5a7, 0x7a71}, {0xc5b5, 0x7a72}, {0xc5b6, 0x7a73},
	{0xc5d5, 0x7a75}, {0xc6b8, 0x7a76}, {0xc6d7, 0x7a77}, {0xc6e0, 0x7a78},
	{0xc6e3, 0x7a7a}, {0xc6ea, 0x7a79}, {0xc7a1, 0x7a7b}, {0xc7ab, 0x7a7c},
	{0xc7c3, 0x7a7e}, {0xc7c7, 0x7a7d}, {0xc7cb, 0x7b21}, {0xc7cf, 0x7b22},
	{0xc7d9, 0x7b23}, {0xc7e6, 0x7b26}, {0xc7eb, 0x7b29}, {0xc7ee, 0x7b27},
	{0xc7f0, 0x7b2a}, {0xc7fc, 0x7b28}, {0xc8b1, 0x7b2b}, {0xc8e5, 0x7b2c},
	{0xc8f8, 0x7b2d}, {0xc9a6, 0x7b2e}, {0xc9ab, 0x7b2f}, {0xc9ad, 0x7b30},
	{0xc9ca, 0x7b32}, {0xc9d3, 0x7b33}, {0xc9e3, 0x7b35}, {0xc9e9, 0x7b34},
	{0xc9f4, 0x7b37}, {0xc9f5, 0x7b38}, {0xc9fc, 0x7b36}, {0xcab3, 0x7b3a},
	{0xcabd, 0x7b3b}, {0xcaef, 0x7b3c}, {0xcaf1, 0x7b3d}, {0xcbae, 0x7b3e},
	{0xcbca, 0x7b40}, {0xcbe6, 0x7b41}, {0xcbea, 0x7b42}, {0xcbee, 0x7b45},
	{0xcbf0, 0x7b43}, {0xcbf4, 0x7b44}, {0xcbf9, 0x7b47}, {0xcca5, 0x7b46},
	{0xccab, 0x7b48}, {0xccad, 0x7b4a}, {0xccae, 0x7b49}, {0xccb2, 0x7b4b},
	{0xccc2, 0x7b4c}, {0xccd0, 0x7b4d}, {0xccd9, 0x7b4e}, {0xcdaa, 0x7a2a},
	{0xcdbb, 0x7b50}, {0xceba, 0x7b54}, {0xcebb, 0x7b52}, {0xcec3, 0x7b55},
	{0xcef2, 0x7b57}, {0xcfd5, 0x7b59}, {0xcfe2, 0x7b5a}, {0xcfe9, 0x7b5b},
	{0xcfed, 0x7b5c}, {0xd0e5, 0x7b61}, {0xd0e9, 0x7b63}, {0xd1e8, 0x7b64},
	{0xd1ec, 0x7b67}, {0xd2bb, 0x7b68}, {0xd3e1, 0x7b6a}, {0xd3e8, 0x7b6b},
	{0xd4a7, 0x7b6c}, {0xd4d4, 0x7b6f}, {0xd4e3, 0x793d}, {0xd4f2, 0x7b70},
	{0xd5ae, 0x7b71}, {0xd7de, 0x7b73}, {0xd8a2, 0x7b75}, {0xd8b7, 0x7b76},
	{0xd8c1, 0x7b77}, {0xd8d1, 0x7b78}, {0xd8f4, 0x7b79}, {0xd9a1, 0x7941},
	{0xd9c6, 0x7b7a}, {0xd9c8, 0x7b7b}, {0xd9d1, 0x7b7c}, {0xdcd3, 0x7c24},
	{0xdcdf, 0x793e}, {0xddc8, 0x7c25}, {0xddd4, 0x7c26}, {0xddea, 0x7c27},
	{0xddfa, 0x7c28}, {0xdea4, 0x7c29}, {0xdeb0, 0x7c2a}, {0xdeb5, 0x7c2c},
	{0xdecb, 0x7c2d}, {0xdfb9, 0x7c2f}, {0xdfc3, 0x7c31}, {0xe0d9, 0x7c34},
	{0xe1e2, 0x7c37}, {0xe2c7, 0x7c3b}, {0xe3a6, 0x7c3d}, {0xe3a8, 0x7c3c},
	{0xe3a9, 0x7c3e}, {0xe3aa, 0x7c41}, {0xe3ab, 0x7c42}, {0xe3af, 0x7c3f},
	{0xe3b0, 0x7c40}, {0xe3bc, 0x7c43}, {0xe3bf, 0x7c45}, {0xe3c1, 0x7c44},
	{0xe3d4, 0x7c4c}, {0xe3d5, 0x7c46}, {0xe3d6, 0x7c48}, {0xe3d8, 0x7c47},
	{0xe3df, 0x7c49}, {0xe3e1, 0x7c4b}, {0xe3e3, 0x7c4a}, {0xe3e9, 0x7c4d},
	{0xe3f1, 0x7c4f}, {0xe3f2, 0x7c50}, {0xe3f8, 0x7940}, {0xe4a6, 0x7c4e},
	{0xe4be, 0x7c54}, {0xe4bf, 0x7c58}, {0xe4c0, 0x7c56}, {0xe4c1, 0x7c52},
	{0xe4c3, 0x7c53}, {0xe4c7, 0x7c57}, {0xe4cb, 0x7c51}, {0xe4d0, 0x7946},
	{0xe4d1, 0x7c5b}, {0xe4d2, 0x7c5e}, {0xe4d4, 0x7c60}, {0xe4db, 0x7c5f},
	{0xe4dc, 0x7c5d}, {0xe4de, 0x7c5a}, {0xe4e0, 0x7c59}, {0xe4e9, 0x793f},
	{0xe4ef, 0x7c62}, {0xe4fa, 0x7c61}, {0xe5b3, 0x7c63}, {0xe5bf, 0x7c64},
	{0xe5c9, 0x7c65}, {0xe5d0, 0x7c66}, {0xe5e2, 0x7c67}, {0xe5ea, 0x7c68},
	{0xe5eb, 0x7c69}, {0xe6e8, 0x7c6d}, {0xe6ef, 0x7c6e}, {0xe7ac, 0x7c6f},
	{0xe7ae, 0x7c71}, {0xe7b1, 0x7c73}, {0xe7b2, 0x7c75}, {0xe8b1, 0x7c76},
	{0xe8b6, 0x7c77}, {0xe8dd, 0x7c7a}, {0xe9d1, 0x7c7d}, {0xe9ed, 0x7d21},
	{0xeacd, 0x7d22}, {0xeadb, 0x7d24}, {0xeae6, 0x7d25}, {0xeaea, 0x7d26},
	{0xeba5, 0x7d27}, {0xebfa, 0x7d29}, {0xebfb, 0x7d28}, {0xecd6, 0x7d2b},
	};

#endif
