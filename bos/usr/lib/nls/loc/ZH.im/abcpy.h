/* @(#)39	1.1  src/bos/usr/lib/nls/loc/ZH.im/abcpy.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:09  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: HIBYTE
 *		LOBYTE
 *		MOD
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

#define MOD(x,y) x-x/y*y
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((UINT)(w) >> 8) & 0xFF))

#define SPECIAL_VALUE1  0x225
#define SPECIAL_VALUE2  0x226
#define SPECIAL_LEN     104
#define SPECIAL_POS     24

struct YJNDX {
	       BYTE head;
	       BYTE *pt;
	       WORD num;
	     };

#ifndef _PYML
#define _PYML
BYTE pymljp[]={
        "ZH0000"
        "SH0000"
        "CH0000"
	"B00000"
	"C00000"
	"D00000"
	"F00000"
	"G00000"
	"H00000"
	"J00000"
	"K00000"
	"L00000"
	"M00000"
	"N00000"
	"P00000"
	"Q00000"
	"R00000"
	"S00000"
	"T00000"
	"W00000"
	"X00000"
	"Y00000"
	"Z00000"
	};

BYTE pymla[] = {

	"A00000"
	"AI0000"
	"AN0000"
	"ANG000"
	"AO0000"
	};

BYTE pymlb[] = {

	"BA0000"
	"BAI000"
	"BAN000"
	"BANG00"
	"BAO000"
	"BEI000"
	"BEN000"
	"BENG00"
	"BI0000"
	"BIAN00"
	"BIAO00"
	"BIE000"
	"BIN000"
	"BING00"
	"BO0000"
	"BU0000"
	};

BYTE pymlc[] = {

	"CA0000"
	"CAI000"
	"CAN000"
	"CANG00"
	"CAO000"
	"CE0000"
	"CEN000"
	"CENG00"
	"CHA000"
	"CHAI00"
	"CHAN00"
	"CHANG0"
	"CHAO00"
	"CHE000"
	"CHEN00"
	"CHENG0"
	"CHI000"
	"CHONG0"
	"CHOU00"
	"CHU000"
	"CHUA00"
	"CHUAI0"
	"CHUAN0"
	"CHUANG"
	"CHUI00"
	"CHUN00"
	"CHUO00"
	"CI0000"
	"CONG00"
	"COU000"
	"CU0000"
	"CUAN00"
	"CUI000"
	"CUN000"
	"CUO000"
	};

BYTE pymld[] = {

	"DA0000"
	"DAI000"
	"DAN000"
	"DANG00"
	"DAO000"
	"DE0000"
	"DEI000"
	"DEN000"
	"DENG00"
	"DI0000"
	"DIA000"
	"DIAN00"
	"DIAO00"
	"DIE000"
	"DING00"
	"DIU000"
	"DONG00"
	"DOU000"
	"DU0000"
	"DUAN00"
	"DUI000"
	"DUN000"
	"DUO000"
	};

BYTE pymle[] = {

	"E00000"
	"EN0000"
	"ENG000"
	"ER0000"
	};

BYTE pymlf[] = {

	"FA0000"
	"FAN000"
	"FANG00"
	"FEI000"
	"FEN000"
	"FENG00"
	"FIAO00"
	"FO0000"
	"FOU000"
	"FU0000"
	};

BYTE pymlg[] = {

	"GA0000"
	"GAI000"
	"GAN000"
	"GANG00"
	"GAO000"
	"GE0000"
	"GEI000"
	"GEN000"
	"GENG00"
	"GONG00"
	"GOU000"
	"GU0000"
	"GUA000"
	"GUAI00"
	"GUAN00"
	"GUANG0"
	"GUI000"
	"GUN000"
	"GUO000"
	};

BYTE pymlh[] = {

	"HA0000"
	"HAI000"
	"HAN000"
	"HANG00"
	"HAO000"
	"HE0000"
	"HEI000"
	"HEN000"
	"HENG00"
	"HONG00"
	"HOU000"
	"HU0000"
	"HUA000"
	"HUAI00"
	"HUAN00"
	"HUANG0"
	"HUI000"
	"HUN000"
	"HUO000"
	};

BYTE pymlj[] = {

	"JI0000"
	"JIA000"
	"JIAN00"
	"JIANG0"
	"JIAO00"
	"JIE000"
	"JIN000"
	"JING00"
	"JIONG0"
	"JIU000"
	"JU0000"
	"JUAN00"
	"JUE000"
	"JUN000"
	};

BYTE pymlk[] = {

	"KA0000"
	"KAI000"
	"KAN000"
	"KANG00"
	"KAO000"
	"KE0000"
	"KEI000"
	"KEN000"
	"KENG00"
	"KONG00"
	"KOU000"
	"KU0000"
	"KUA000"
	"KUAI00"
	"KUAN00"
	"KUANG0"
	"KUI000"
	"KUN000"
	"KUO000"
	};

BYTE pymll[] = {

	"LA0000"
	"LAI000"
	"LAN000"
	"LANG00"
	"LAO000"
	"LE0000"
	"LEI000"
	"LENG00"
	"LI0000"
	"LIA000"
	"LIAN00"
	"LIANG0"
	"LIAO00"
	"LIE000"
	"LIN000"
	"LING00"
	"LIU000"
	"LO0000"
	"LONG00"
	"LOU000"
	"LU0000"
	"LUAN00"
	"LUE000"
	"LUN000"
	"LUO000"
	"LV0000"
	};

BYTE pymlm[] = {

	"MA0000"
	"MAI000"
	"MAN000"
	"MANG00"
	"MAO000"
	"ME0000"
	"MEI000"
	"MEN000"
	"MENG00"
	"MI0000"
	"MIAN00"
	"MIAO00"
	"MIE000"
	"MIN000"
	"MING00"
	"MIU000"
	"MO0000"
	"MOU000"
	"MU0000"
	};

BYTE pymln[] = {

	"NA0000"
	"NAI000"
	"NAN000"
	"NANG00"
	"NAO000"
	"NE0000"
	"NEI000"
	"NEN000"
	"NENG00"
	"NI0000"
	"NIAN00"
	"NIANG0"
	"NIAO00"
	"NIE000"
	"NIN000"
	"NING00"
	"NIU000"
	"NONG00"
	"NOU000"
	"NU0000"
	"NUAN00"
	"NUE000"
	"NUN000"
	"NUO000"
	"NV0000"
	};

BYTE pymlo[] = {

	"O00000"
	"OU0000"
	};

BYTE pymlp[] = {

	"PA0000"
	"PAI000"
	"PAN000"
	"PANG00"
	"PAO000"
	"PEI000"
	"PEN000"
	"PENG00"
	"PI0000"
	"PIAN00"
	"PIAO00"
	"PIE000"
	"PIN000"
	"PING00"
	"PO0000"
	"POU000"
	"PU0000"
	};

BYTE pymlq[] = {

	"QI0000"
	"QIA000"
	"QIAN00"
	"QIANG0"
	"QIAO00"
	"QIE000"
	"QIN000"
	"QING00"
	"QIONG0"
	"QIU000"
	"QU0000"
	"QUAN00"
	"QUE000"
	"QUN000"
	};

BYTE pymlr[] = {

	"RA0000"
	"RAN000"
	"RANG00"
	"RAO000"
	"RE0000"
	"REN000"
	"RENG00"
	"RI0000"
	"RONG00"
	"ROU000"
	"RU0000"
	"RUA000"
	"RUAN00"
	"RUI000"
	"RUN000"
	"RUO000"
	};

BYTE pymls[] = {

	"SA0000"
	"SAI000"
	"SAN000"
	"SANG00"
	"SAO000"
	"SE0000"
	"SEN000"
	"SENG00"
	"SHA000"
	"SHAI00"
	"SHAN00"
	"SHANG0"
	"SHAO00"
	"SHE000"
	"SHEI00"
	"SHEN00"
	"SHENG0"
	"SHI000"
	"SHOU00"
	"SHU000"
	"SHUA00"
	"SHUAI0"
	"SHUAN0"
	"SHUANG"
	"SHUI00"
	"SHUN00"
	"SHUO00"
	"SI0000"
	"SONG00"
	"SOU000"
	"SU0000"
	"SUAN00"
	"SUI000"
	"SUN000"
	"SUO000"
	};

BYTE pymlt[] = {

	"TA0000"
	"TAI000"
	"TAN000"
	"TANG00"
	"TAO000"
	"TE0000"
	"TENG00"
	"TI0000"
	"TIAN00"
	"TIAO00"
	"TIE000"
	"TING00"
	"TONG00"
	"TOU000"
	"TU0000"
	"TUAN00"
	"TUI000"
	"TUN000"
	"TUO000"
	};

BYTE pymlw[] = {

	"WA0000"
	"WAI000"
	"WAN000"
	"WANG00"
	"WEI000"
	"WEN000"
	"WENG00"
	"WO0000"
	"WU0000"
	};

BYTE pymlx[] = {

	"XI0000"
	"XIA000"
	"XIAN00"
	"XIANG0"
	"XIAO00"
	"XIE000"
	"XIN000"
	"XING00"
	"XIONG0"
	"XIU000"
	"XU0000"
	"XUAN00"
	"XUE000"
	"XUN000"
	};

BYTE pymly[] = {

	"YA0000"
	"YAN000"
	"YANG00"
	"YAO000"
	"YE0000"
	"YI0000"
	"YIN000"
	"YING00"
	"YO0000"
	"YONG00"
	"YOU000"
	"YU0000"
	"YUAN00"
	"YUE000"
	"YUN000"
	};

BYTE pymlz[] = {

	"ZA0000"
	"ZAI000"
	"ZAN000"
	"ZANG00"
	"ZAO000"
	"ZE0000"
	"ZEI000"
	"ZEN000"
	"ZENG00"
	"ZHA000"
	"ZHAI00"
	"ZHAN00"
	"ZHANG0"
	"ZHAO00"
	"ZHE000"
	"ZHEI00"
	"ZHEN00"
	"ZHENG0"
	"ZHI000"
	"ZHONG0"
	"ZHOU00"
	"ZHU000"
	"ZHUA00"
	"ZHUAI0"
	"ZHUAN0"
	"ZHUANG"
	"ZHUI00"
	"ZHUN00"
	"ZHUO00"
	"ZI0000"
	"ZONG00"
	"ZOU000"
	"ZU0000"
	"ZUAN00"
	"ZUI000"
	"ZUN000"
	"ZUO000"
	};

BYTE pymlfb[]={

	"AES000"
	"BAIKE0"
	"BAIWA0"
	"BE0000"
	"CAL000"
	"CEOK00"
	"CEOM00"
	"CEON00"
	"CEOR00"
	"CIS000"
	"DEM000"
	"DEO000"
	"DIM000"
	"DUG000"
	"DUL000"
	"EI0000"
	"EO0000"
	"EOL000"
	"EOM000"
	"EOS000"
	"FENWA0"
	"FUI000"
	"GAD000"
	"GEO000"
	"GEU000"
	"GIB000"
	"GO0000"
	"GONGLI"
	"HAL000"
	"HAOKE0"
	"HEM000"
	"HEUI00"
	"HO0000"
	"HOL000"
	"HWA000"
	"HWEONG"
	"I00000"
	"JIALUN"
	"JOU000"
	"KAL000"
	"KEG000"
	"KEM000"
	"KEO000"
	"KEOL00"
	"KEOP00"
	"KEOS00"
	"KEUM00"
	"KI0000"
	"KOS000"
	"KWEOK0"
	"KWI000"
	"LEM000"
	"LEN000"
	"LIWA00"
	"MANGMI"
	"MAS000"
	"MEO000"
	"MOL000"
	"MYEO00"
	"MYEON0"
	"MYEONG"
	"NEM000"
	"NEUS00"
	"NG0000"
	"NGAG00"
	"NGAI00"
	"NGAM00"
	"NUNG00"
	"OES000"
	"OL0000"
	"ON0000"
	"PAK000"
	"PEOL00"
	"PHAS00"
	"PHDENG"
	"PHOI00"
	"PHOS00"
	"PPUN00"
	"QIANKE"
	"QIANWA"
	"RAM000"
	"SAENG0"
	"SAL000"
	"SED000"
	"SEI000"
	"SEO000"
	"SEON00"
	"SHIKE0"
	"SHIWA0"
	"SO0000"
	"SOL000"
	"TAE000"
	"TAP000"
	"TEO000"
	"TEUL00"
	"TEUN00"
	"TIU000"
	"TOL000"
	"TON000"
	"WIE000"
	"YEN000"
	"YUG000"
	"ZAD000"
	"ZO0000"
	"GONGFEN"
	"MILIKLANM"
	};

struct YJNDX pyml_index[] = {{0, pymljp, 0x1},
		      {'A', pymla, 0x21},
		      {'B', pymlb, 0x26},
		      {'C', pymlc, 0x36},
		      {'D', pymld, 0x59},
		      {'E', pymle, 0x70},
		      {'F', pymlf, 0x74},
		      {'G', pymlg, 0x7e},
		      {'H', pymlh, 0x91},
		      {'J', pymlj, 0xa4},
		      {'K', pymlk, 0xb2},
		      {'L', pymll, 0xc5},
		      {'M', pymlm, 0xdf},
		      {'N', pymln, 0xf2},
		      {'O', pymlo, 0x10b},
		      {'P', pymlp, 0x10d},
		      {'Q', pymlq, 0x11e},
		      {'R', pymlr, 0x12c},
		      {'S', pymls, 0x13c},
		      {'T', pymlt, 0x15f},
		      {'W', pymlw, 0x172},
		      {'X', pymlx, 0x17b},
		      {'Y', pymly, 0x189},
		      {'Z', pymlz, 0x198},
		      {'Z'+1, pymlfb, 0x1bd},
		     };
#endif 


#define TRUE    1
#define FALSE   0
#define NUMBER  0x20
#define FUYIN   0x21
#define YUANYIN  0x22
#define SEPERATOR  0x27
#define FIRST_T    1
#define SECOND_T   2
#define THIRD_T    3
#define FORTH_T    4

/* about search strutagy */
#define BX_FLAG         8
#define JP_FLAG         4
#define QP_FLAG         2
#define YD_FLAG         1

/* about search lib */
#define BODY_START              0
#define KZK_BODY_START          0
#define KZK_BASE                0xa000l
#define MORE_THAN_5             23
#define TMMR_REAL_LENGTH        0x1800

/* mark for test*/
#define TEST                    0


struct SLBL{
    WORD value;
    BYTE head;
    WORD length;
    BYTE tune;
    BYTE bx1;
    WORD bx2;
    BYTE flag;
}sb;

struct N_SLBL{
    BYTE buffer[30];
    int length;
}neg;

#ifndef _DICREC
#define _DICREC
struct DICREC {
              long startp;
              long length;
};
#endif

#ifndef _FHEAD
#define _FHEAD
extern struct FILEHEAD {
		struct DICREC total;
		struct DICREC csm;
		struct DICREC dyj;
		struct DICREC jf;
                struct DICREC grp;
		BYTE idflag[8];
}head;
#endif

struct DYJ {
           BYTE csml;
           BYTE csmh;
           BYTE pd;
};


BYTE slbl_tab[]="ZH00\1"
"SH00\2"
"CH00\3"
"ING0\4"
"AI00\5"
"AN00\6"
"ANG0\7"
"AO00\x8"
"EI00\x9"
"EN00\xa"
"ENG0\xb"
"IA00\xc"
"IAN0\xd"
"IANG\xe"
"IAO0\xf"
"IE00\x10"
"IN00\x11"
"IU00\x12"
"ONG0\x13"
"OU00\x14"
"UA00\x15"
"UAI0\x16"
"UAN0\x17"
"UE00\x18"
"UN00\x19"
"UENG\x1a"
"UI00\x1b"
"UO00\x1c"
"UANG\x1d"
"ER00\x1e"
"IONG\x1f"
"VE00\x18"
"UEN0\x19"
"VEN0\x19"
"UEI0\x1b"
"IOU0\x12";


BYTE buffer[30];
WORD jp_index[32];
WORD qp_index[520];

BYTE cmp_head,cmp_state,cmp_bx,by_cchar_flag;
WORD cmp_yj,cmp_cisu;

/* about search lib */
LONG r_addr;
WORD out_svw_cnt,msx_area_cnt;
WORD search_start,search_end,kzk_search_start,kzk_search_end;
WORD item_length,kzk_item_length,last_item_name,item_addr,slib_addr;
BYTE word_lib_state;
WORD lib_w[0x800];
WORD kzk_lib_w[0x400];
BYTE word_source,xs_flag,sfx_attr,jiyi_pindu,system_info;
BYTE stack1_move_counter;
WORD extb_ps;
BYTE country_flag;
BYTE undo_p[6]={0};
