static char sccsid[] = "@(#)93	1.1  src/bos/usr/ccs/lib/libc/POWER/atof_tab.c, libccnv, bos411, 9428A410j 5/8/91 10:10:24";
/*
 * COMPONENT_NAME: LIBCCNV atof
 *
 * FUNCTIONS: checknan and checknf functions.
 *            Tables for atof, strtod, atoff, strtof.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct fp { 
	long int i[2]; 
};

struct fp atof_pow1[128][2] = {
	0x3FF00000, 0x00000000,   0x00000000, 0x00000000,      /*  10^0    */
	0x40240000, 0x00000000,   0x00000000, 0x00000000,      /*  10^1    */
	0x40590000, 0x00000000,   0x00000000, 0x00000000,      /*  10^2    */
	0x408F4000, 0x00000000,   0x00000000, 0x00000000,      /*  10^3    */
	0x40C38800, 0x00000000,   0x00000000, 0x00000000,      /*  10^4    */
	0x40F86A00, 0x00000000,   0x00000000, 0x00000000,      /*  10^5    */
	0x412E8480, 0x00000000,   0x00000000, 0x00000000,      /*  10^6    */
	0x416312D0, 0x00000000,   0x00000000, 0x00000000,      /*  10^7    */
	0x4197D784, 0x00000000,   0x00000000, 0x00000000,      /*  10^8    */
	0x41CDCD65, 0x00000000,   0x00000000, 0x00000000,      /*  10^9    */
	0x4202A05F, 0x20000000,   0x00000000, 0x00000000,      /*  10^10   */
	0x42374876, 0xe8000000,   0x00000000, 0x00000000,      /*  10^11   */
	0x426D1A94, 0xa2000000,   0x00000000, 0x00000000,      /*  10^12   */
	0x42A2309C, 0xe5400000,   0x00000000, 0x00000000,      /*  10^13   */
	0x42D6BCC4, 0x1e900000,   0x00000000, 0x00000000,      /*  10^14   */
	0x430C6BF5, 0x26340000,   0x00000000, 0x00000000,      /*  10^15   */
	0x4341C379, 0x37e08000,   0x00000000, 0x00000000,      /*  10^16   */
	0x43763457, 0x85d8a000,   0x00000000, 0x00000000,      /*  10^17   */
	0x43ABC16D, 0x674ec800,   0x00000000, 0x00000000,      /*  10^18   */
	0x43E158E4, 0x60913d00,   0x00000000, 0x00000000,      /*  10^19   */
	0x4415AF1D, 0x78b58c40,   0x00000000, 0x00000000,      /*  10^20   */
	0x444B1AE4, 0xd6e2ef50,   0x00000000, 0x00000000,      /*  10^21   */
	0x4480F0CF, 0x064dd592,   0x00000000, 0x00000000,      /*  10^22   */
	0x44B52D02, 0xc7e14af6,   0x41600000, 0x00000000,      /*  10^23   */
	0x44EA7843, 0x79d99db4,   0x41700000, 0x00000000,      /*  10^24   */
	0x45208B2A, 0x2c280290,   0x41D28000, 0x00000000,      /*  10^25   */
	0x4554ADF4, 0xb7320334,   0x42072000, 0x00000000,      /*  10^26   */
	0x4589D971, 0xe4fe8401,   0x423CE800, 0x00000000,      /*  10^27   */
	0x45C027E7, 0x2f1f1281,   0x42584400, 0x00000000,      /*  10^28   */
	0x45F431E0, 0xfae6d721,   0x429F2A80, 0x00000000,      /*  10^29   */
	0x46293E59, 0x39a08ce9,   0x42DB7A90, 0x00000000,      /*  10^30   */
	0x465F8DEF, 0x8808b024,   0x42F4B268, 0x00000000,      /*  10^31   */
	0x4693B8B5, 0xb5056e16,   0x434677C0, 0x80000000,      /*  10^32   */
	0x46C8A6E3, 0x2246c99c,   0x43682B61, 0x40000000,      /*  10^33   */
	0x46FED09B, 0xead87c03,   0x439E3639, 0x90000000,      /*  10^34   */
	0x47334261, 0x72c74d82,   0x43C5C3C7, 0xf4000000,      /*  10^35   */
	0x476812F9, 0xcf7920e2,   0x4416CD2E, 0x7c400000,      /*  10^36   */
	0x479E17B8, 0x4357691b,   0x443900F4, 0x36a00000,      /*  10^37   */
	0x47D2CED3, 0x2a16a1b1,   0x445E8262, 0x88900000,      /*  10^38   */
	0x48078287, 0xf49c4a1d,   0x44A988BE, 0xcaad0000,      /*  10^39   */
	0x483D6329, 0xf1c35ca4,   0x44E7F577, 0x3eac2000,      /*  10^40   */
	0x48725DFA, 0x371a19e6,   0x452EF96A, 0x872b9400,      /*  10^41   */
	0x48A6F578, 0xc4e0a060,   0x4556B7C5, 0x28f67900,      /*  10^42   */
	0x48DCB2D6, 0xf618c878,   0x458C65B6, 0x73341740,      /*  10^43   */
	0x4911EFC6, 0x59cf7d4b,   0x45C1BF92, 0x08008e88,      /*  10^44   */
	0x49466BB7, 0xf0435c9e,   0x45EC5EED, 0x14016454,      /*  10^45   */
	0x497C06A5, 0xec5433c6,   0x45EBB542, 0xc80deb48,      /*  10^46   */
	0x49B18427, 0xb3b4a05b,   0x46691514, 0x9bd08b31,      /*  10^47   */
	0x49E5E531, 0xa0a1c872,   0x46975A59, 0xc2c4adfd,      /*  10^48   */
	0x4A1B5E7E, 0x08ca3a8f,   0x46BA61E0, 0x66ebb2f9,      /*  10^49   */
	0x4A511B0E, 0xc57e6499,   0x47043E96, 0x2029a7ee,      /*  10^50   */
	0x4A8561D2, 0x76ddfdc0,   0x46F4E3BA, 0x83411e91,      /*  10^51   */
	0x4ABABA47, 0x14957d30,   0x472A1CA9, 0x24116636,      /*  10^52   */
	0x4AF0B46C, 0x6cdd6e3e,   0x476051E9, 0xb68adfe2,      /*  10^53   */
	0x4B24E187, 0x8814c9cd,   0x47D14666, 0x4242d97e,      /*  10^54   */
	0x4B5A19E9, 0x6a19fc40,   0x480D97FF, 0xd2d38fdd,      /*  10^55   */
	0x4B905031, 0xe2503da8,   0x48427EFF, 0xe3c439ea,      /*  10^56   */
	0x4BC4643E, 0x5ae44d12,   0x48771EBF, 0xdcb54865,      /*  10^57   */
	0x4BF97D4D, 0xf19d6057,   0x4899CCDF, 0xa7c534fc,      /*  10^58   */
	0x4C2FDCA1, 0x6e04b86d,   0x48C04017, 0x91b6823b,      /*  10^59   */
	0x4C63E9E4, 0xe4c2f344,   0x4902280E, 0xbb121165,      /*  10^60   */
	0x4C98E45E, 0x1df3b015,   0x4936B212, 0x69d695be,      /*  10^61   */
	0x4CCF1D75, 0xa5709c1a,   0x49762F4B, 0x82261d97,      /*  10^62   */
	0x4D037269, 0x87666190,   0x49B5DD8F, 0x3157d27e,      /*  10^63   */
	0x3FF00000, 0x00000000,    0x00000000, 0x00000000,   /* 10^-0   */
	0x3FB99999, 0x99999999,    0x3C633333, 0x33333333,   /* 10^-1   */
	0x3F847AE1, 0x47ae147a,    0x3C3C28F5, 0xc28f5c29,   /* 10^-2   */
	0x3F50624D, 0xd2f1a9fb,    0x3C0CED91, 0x6872b021,   /* 10^-3   */
	0x3F1A36E2, 0xeb1c432c,    0x3BC4AF4F, 0x0d844d01,   /* 10^-4   */
	0x3EE4F8B5, 0x88e368f0,    0x3B908C3F, 0x3e0370ce,   /* 10^-5   */
	0x3EB0C6F7, 0xa0b5ed8d,    0x3B4B5A63, 0xf9a49c2c,   /* 10^-6   */
	0x3E7AD7F2, 0x9abcaf48,    0x3B15E1E9, 0x9483b023,   /* 10^-7   */
	0x3E45798E, 0xe2308c39,    0x3AFBF3F7, 0x0834acdb,   /* 10^-8   */
	0x3E112E0B, 0xe826d694,    0x3AC65CC5, 0xa02a23e2,   /* 10^-9   */
	0x3DDB7CDF, 0xd9d7bdba,    0x3A86FAD5, 0xcd10396a,   /* 10^-10  */
	0x3DA5FD7F, 0xe1796495,    0x3A47F7BC, 0x7b4d28aa,   /* 10^-11  */
	0x3D719799, 0x812dea11,    0x39F97F27, 0xf0f6e886,   /* 10^-12  */
	0x3D3C25C2, 0x68497681,    0x39E84CA1, 0x9697c81b,   /* 10^-13  */
	0x3D06849B, 0x86a12b9b,    0x394EA709, 0x09833de7,   /* 10^-14  */
	0x3CD203AF, 0x9ee75615,    0x3983643E, 0x74dc0530,   /* 10^-15  */
	0x3C9CD2B2, 0x97d889bc,    0x3925B4C2, 0xebe68799,   /* 10^-16  */
	0x3C670EF5, 0x4646d496,    0x39112426, 0xfbfae7eb,   /* 10^-17  */
	0x3C32725D, 0xd1d243ab,    0x38E41CEB, 0xfcc8b989,   /* 10^-18  */
	0x3BFD83C9, 0x4fb6d2ac,    0x388A52B3, 0x1e9e3d07,   /* 10^-19  */
	0x3BC79CA1, 0x0c924223,    0x38675447, 0xa5d8e536,   /* 10^-20  */
	0x3B92E3B4, 0x0a0e9b4f,    0x383F769F, 0xb7e0b75e,   /* 10^-21  */
	0x3B5E3920, 0x10175ee5,    0x3802C54C, 0x931a2c4b,   /* 10^-22  */
	0x3B282DB3, 0x4012b251,    0x37C13BAD, 0xb829e079,   /* 10^-23  */
	0x3AF357C2, 0x99a88ea7,    0x379A9624, 0x9354b394,   /* 10^-24  */
	0x3ABEF2D0, 0xf5da7dd8,    0x376544EA, 0x0f76f610,   /* 10^-25  */
	0x3A88C240, 0xc4aecb13,    0x37376A54, 0xd92bf80d,   /* 10^-26  */
	0x3A53CE9A, 0x36f23c0f,    0x370921DD, 0x7a89933d,   /* 10^-27  */
	0x3A1FB0F6, 0xbe506019,    0x36B06C5E, 0x54eb70c4,   /* 10^-28  */
	0x39E95A5E, 0xfea6b347,    0x3689F04B, 0x7722c09d,   /* 10^-29  */
	0x39B4484B, 0xfeebc29f,    0x3660C684, 0x960de6a5,   /* 10^-30  */
	0x398039D6, 0x6589687f,    0x3633D203, 0xab3e521e,   /* 10^-31  */
	0x3949F623, 0xd5a8a732,    0x35F2E99F, 0x7863b696,   /* 10^-32  */
	0x3914C4E9, 0x77ba1f5b,    0x35C587B2, 0xc6b62bab,   /* 10^-33  */
	0x38E09D87, 0x92fb4c49,    0x3585A5EA, 0xd789df78,   /* 10^-34  */
	0x38AA95A5, 0xb7f87a0e,    0x355E1E55, 0x793b192d,   /* 10^-35  */
	0x38754484, 0x932d2e72,    0x351696EF, 0x285e8eaf,   /* 10^-36  */
	0x3841039D, 0x428a8b8e,    0x34F5D5F9, 0x435905df,   /* 10^-37  */
	0x380B38FB, 0x9daa78e4,    0x34A2ACB7, 0x3de9ac65,   /* 10^-38  */
	0x37D5C72F, 0xb1552d83,    0x347BBD5F, 0x64baf050,   /* 10^-39  */
	0x37A16C26, 0x2777579c,    0x34463119, 0x1d6259da,   /* 10^-40  */
	0x376BE03D, 0x0bf225c6,    0x341E8DAD, 0xb11b7b15,   /* 10^-41  */
	0x37364CFD, 0xa3281e38,    0x33E87157, 0xc0e2c8dd,   /* 10^-42  */
	0x3701D731, 0x4f534b60,    0x33B38DDF, 0xcd823a4b,   /* 10^-43  */
	0x36CC8B82, 0x18854567,    0x33682C65, 0xc4d3edbc,   /* 10^-44  */
	0x3696D601, 0xad376ab9,    0x331A27AC, 0x0f72f8c0,   /* 10^-45  */
	0x366244CE, 0x242c5560,    0x331C372A, 0xce584c13,   /* 10^-46  */
	0x362D3AE3, 0x6d13bbce,    0x32BAFAAB, 0x8f01e6e1,   /* 10^-47  */
	0x35F7624F, 0x8a762fd8,    0x32859556, 0x0c018581,   /* 10^-48  */
	0x35C2B50C, 0x6ec4f313,    0x32656EEF, 0x38009bcd,   /* 10^-49  */
	0x358DEE7A, 0x4ad4b81e,    0x323DF258, 0xf99a163e,   /* 10^-50  */
	0x3557F1FB, 0x6f10934b,    0x320E5B7A, 0x614811cb,   /* 10^-51  */
	0x352327FC, 0x58da0f6f,    0x31DEAF95, 0x1aa00e3c,   /* 10^-52  */
	0x34EEA660, 0x8e29b24c,    0x31977F54, 0xf7667d2d,   /* 10^-53  */
	0x34B8851A, 0x0b548ea3,    0x316932AA, 0x5f8530f1,   /* 10^-54  */
	0x34839DAE, 0x6f76d883,    0x30EEAAA3, 0x26eb4b43,   /* 10^-55  */
	0x344F62B0, 0xb257c0d1,    0x30F4BBBB, 0x5b8bc3c3,   /* 10^-56  */
	0x34191BC0, 0x8eac9a41,    0x30B45F92, 0x2c12d2d2,   /* 10^-57  */
	0x33E41633, 0xa556e1cd,    0x309B596D, 0xab3ababa,   /* 10^-58  */
	0x33B011C2, 0xeaabe7d7,    0x306C478A, 0xef622efc,   /* 10^-59  */
	0x3379B604, 0xaaaca626,    0x300B6379, 0x2f412cb0,   /* 10^-60  */
	0x3344919D, 0x5556eb51,    0x2FF8AD7E, 0xa30d08f0,   /* 10^-61  */
	0x3310747D, 0xdddf22a7,    0x2FCA2465, 0x4f3da0c0,   /* 10^-62  */
	0x32DA53FC, 0x9631d10c,    0x2F803A3B, 0xb1fc3467    /* 10^-63  */
};


struct fp atof_pospow2[5][2] = {
	0x3FF00000, 0x00000000,   0x00000000, 0x00000000,      /*  10^0    */
	0x4D384F03, 0xe93ff9f4,   0x49EB54F2, 0xfdadc71e,      /*  10^64   */
	0x5A827748, 0xf9301d31,   0x57337F19, 0xbccdb0db,      /*  10^128  */
	0x67CC0E1E, 0xf1a724ea,   0x6475AA16, 0xef894fd2,      /*  10^192  */
	0x75154FDD, 0x7f73bf3b,   0x71CA3776, 0xee406e64       /*  10^256  */
};

struct fp atof_negpow2[6][2] = {
	0x3FF00000, 0x00000000,    0x00000000, 0x00000000,   /* 10^-0    */
	0x32A50FFD, 0x44f4a73d,    0x2F3A53F2, 0x398d747b,   /* 10^-64   */
	0x255BBA08, 0xcf8c979c,    0x220282B1, 0xf2cfdb41,   /* 10^-128  */
	0x18123FF0, 0x6eea8479,    0x14C019ED, 0x8c1a8d19,   /* 10^-192  */
	0x0AC80628, 0x64ac6f43,    0x07539FA9, 0x11155ff0,   /* 10^-256  */
};


int atof_digit[] = { 

/*	 0	 1	 2	 3	 4	 5	 6	 7  */

/* 0*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/* 10*/	 0,	 0,   	 0,   	 0,   	 0,   	 0,   	 0,	 0,
/* 20*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/* 30*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/* 40*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/* 50*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/* 60*/	 1,   	 1,   	 1,   	 1,   	 1,   	 1,   	 1,   	 1,   
/* 70*/	 1,   	 1,   	 0,	 0,	 0,	 0,	 0,	 0,
/*100*/	 0,	 0,   	 0,   	 0,   	 0,   	 0,   	 0,   	 0,
/*110*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*120*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*130*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*140*/	 0,	 0,   	 0,   	 0,   	 0,   	 0,   	 0,   	 0,
/*150*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*160*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*170*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*200*/	 0,	 0,	 0,	 0,	 0,	 0,	 0, 	 0,
/*210*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*220*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*230*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*240*/	 0,	 0,   	 0,   	 0,   	 0,   	 0,   	 0,   	 0,
/*250*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*260*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*270*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*300*/	 0,	 0,	 0,	 0,	 0,	 0,	 0, 	 0,
/*310*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*320*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*330*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*340*/	 0,	 0,   	 0,   	 0,   	 0,   	 0,   	 0,   	 0,
/*350*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*360*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*370*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*400*/	 0,	 0,	 0,	 0,	 0,	 0,	 0, 	 0
};
/*
 * NAME: checknf
 *                                                                    
 * FUNCTION: check to see in string is infinity
 *                                                                    
 * NOTES:
 *	The check for infinity is case insensitive
 *
 * RETURNS: 3 for inf string;
 *          8 for infinity string;
 *          0 otherwise.
 *
 */

int checknf(char *bp)
{
	bp++;
	if ((*bp == 'n') || (*bp == 'N'))
	{   bp++;
	    if ((*bp == 'f') || (*bp == 'F'))
	    {   
	    /* if here, we have inf.  Scan some more to check for infinity. */
		if (
			((*++bp == 'i') || (*bp == 'I')) &&
			((*++bp == 'n') || (*bp == 'N')) &&
			((*++bp == 'i') || (*bp == 'I')) &&
			((*++bp == 't') || (*bp == 'T')) &&
			((*++bp == 'y') || (*bp == 'Y'))
	       	    )
			return (8);
		else 	return (3);
	    }
	}

	return (0);
}

/*
 * NAME: checknan
 *                                                                    
 * FUNCTION: check to see in string is NaNQ or NaNS
 *                                                                    
 * NOTES:
 *	The check for NaN is case insensitive
 *
 * RETURNS: 1 for NaNQ string;
 *          2 for NaNS string;
 *          3 for NaN  string:
 *          0 otherwise
 *
 */
int checknan(char *bp)
{
	bp++;
	if ((*bp == 'a') || (*bp == 'A'))
	{	bp++;
		if ((*bp == 'n') || (*bp == 'N'))
		{	bp++;
		/* if here we have a NaN; now just classify it */
			if ((*bp == 'q') || (*bp == 'Q')) {
				return (1);		/* NaNQ */
			}
			else if ((*bp == 's') || (*bp == 'S')) {
				return (2);		/* explicit NaNS */
			}
			else return (3);		/* implicit NaNS */
		}
	}

	return(0);
}
