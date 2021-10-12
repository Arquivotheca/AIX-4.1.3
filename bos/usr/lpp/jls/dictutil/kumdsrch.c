static char sccsid[] = "@(#)63	1.3  src/bos/usr/lpp/jls/dictutil/kumdsrch.c, cmdKJI, bos411, 9428A410j 7/7/93 13:28:44";
/*
 * COMPONENT_NAME: (cmdKJI) User Dictionary Utility
 *
 * FUNCTIONS: chk_exist, srch_mono, srch_poly, mora2pc
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kumdsrch.c
 *
 * DESCRIPTIVE NAME:    System Dictionary Check
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         chk_exist srch_mono srch_poly
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/* include Kanji Project. */
#include "kut.h"
#include "kumdict.h"


/* Copyright Identify. */
static char    *cprt1 = "5601-125 COPYRIGHT IBM CORP 1989           ";
static char    *cprt2 = "LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";


chk_exist( z_mcbptr, mora_len, mora_code )

MULSYS	*z_mcbptr;
ushort  mora_len;
uchar   *mora_code;
{
    ushort          z_ix, z_mx;
    ushort          mora_key;

    if (mora_len == 1)
    {
	if ((*mora_code < z_mcbptr->dict_mono_lkey) ||
	    (*mora_code > z_mcbptr->dict_mono_hkey))
	{
	    return (-1);
	}
	z_ix = *mora_code / 8;
	z_mx = *mora_code % 8;
	if ((z_mcbptr->dict_mono_ex[z_ix] & (1 << (7 - z_mx))) == 0)
	{
	    return (-1);
	}
    }
    else
    {
	mora_key = (ushort) (*mora_code * 256 + *(mora_code + 1));
	if ((mora_key < z_mcbptr->dict_poly_lkey) ||
	    (mora_key > z_mcbptr->dict_poly_hkey))
	{
	    return (-1);
	}
	z_ix = *mora_code / 8;
	z_mx = *mora_code % 8;
	if ((z_mcbptr->dict_poly_ex[z_ix] & (1 << (7 - z_mx))) == 0)
	{
	    return (-1);
	}
    }
    return (0);
}



srch_mono( z_mcbptr, data, mora_code, kj_code, kj_len )

MULSYS 	*z_mcbptr;
uchar   *data;
uchar   mora_code;
ushort  *kj_code;
ushort  kj_len;
{
    uchar          *save_ptr, *tmp_ptr;
    ushort          seisho[256], kana_code[2], offset, jdata_l, tg_l, df_l;
    ushort          entry_num, i, j, num, flag;


    save_ptr = data;

    /* skip VCB		 */
    data += 2;

    /* set E.Mora_C	 */
    if ((ushort) * data != 0)
    {
	entry_num = (ushort) * data;
	data++;
    }
    else
    {
	data++;
	entry_num = GETSHORT(data);
	data += 3;
    }

    i = 0;
    while (mora_code != (uchar) * (data + i * 2))
    {
	i++;
	if (i == entry_num)
	{
	    return (-1);
	}
    }
    tmp_ptr = (uchar *) (data + entry_num * 2 + i * 2);
    if (((ushort) (*(tmp_ptr + 1) & 0x80)) == 0)
    {
	return (-1);
    }
    offset = (ushort) * tmp_ptr +
	(ushort) (*(tmp_ptr + 1) & 0x3f) * 256;

    data = (uchar *) (save_ptr + offset);
    save_ptr = data;
    jdata_l = *(data++) - 1;

    for (i = 0, flag = 0; data < (uchar *) (save_ptr + jdata_l);)
    {
	if ((uchar) * data == 0x80)
	{
	    data++;
	}

	if (((*data == 0xbf) || (*data == 0xff)) && (*(data + 1) == 0xfe))
	{
	    if (kj_len == 1 &&  kj_code[0] == 0xfffe)
	    {
		return(0);
	    }
	    mora2pc(0, mora_code, kana_code, &num);
	    seisho[i] = kana_code[0];
	}
	else
	if (((*data == 0xbf) || (*data == 0xff)) && (*(data + 1) == 0xff))
	{
	    if (kj_len == 1 &&  kj_code[0] == 0xffff)
	    {
		return(0);
	    }
	    mora2pc(1, mora_code, kana_code, &num);
	    seisho[i] = kana_code[0];
	}
	else
	{
	    if (((*data | 0x80) < 0x81) || ((*data | 0x80) > 0xfc) ||
		(((*data | 0x80) > 0x9f) && ((*data | 0x80) < 0xe0)))
	    {
		seisho[i] = (ushort) ((*data | 0xc0) * 256 + *(data + 1));
	    }
	    else
	    {
		seisho[i] = (ushort) ((*data | 0x80) * 256 + *(data + 1));
	    }
	    num = 1;
	}

	if (num == 2)
	{
	    seisho[i + 1] = kana_code[1];
	}
	i += num;
	if ((*data & 0x80) == 0)
	{
	    data += 2;
	    continue;
	}

	if (kj_len == i)
	{
	    for (j = 0; j < kj_len; j++)
	    {
		if (seisho[j] != kj_code[j])
		{
		    flag = 1;
		    break;
		}
	    }
	    if (flag == 0)
	    {
		return (0);
	    }
	}
	data += 2;

	tg_l = (ushort) (((uchar) * data >> 6) & 0x03);
	df_l = (ushort) (((uchar) * data >> 4) & 0x03);

	data += z_mcbptr->add_info_l;
	data += tg_l;
	while (((*data & 0x80) != 0) && (df_l != 0))
	{
	    data += df_l;
	}
	data += df_l;
	i = 0;
	flag = 0;
    }

    return (-1);
}



srch_poly(z_mcbptr, data, mora_code, mora_len, kj_code, kj_len)
    MULSYS         *z_mcbptr;
    uchar          *data;
    uchar          *mora_code;
    ushort          mora_len;
    ushort         *kj_code;
    ushort          kj_len;
{
    uchar          *save_ptr, *tmp_ptr, mora_tmp[256];
    ushort          seisho[256], kana_code[2], offset = 0, jdata_l, tg_l, df_l;
    ushort          entry_num, i, j, k, num, flag, cmp_len = 0;


    save_ptr = data;
    if ((mora_len % 2) != 0)
    {
	memcpy(mora_tmp, mora_code, mora_len);
	mora_tmp[mora_len] = 0x00;
	mora_code = mora_tmp;
	mora_len++;
    }

    /* skip VCB		 */
    data += 2;

    /* Loop */
    while (offset == 0)
    {

	if (mora_len <= cmp_len)
	{
	    return (-1);
	}

	/* set E.Mora_C	 */
	if ((ushort)(*data) != 0)
	{
	    entry_num = (ushort)(*data);
	    data++;
	}
	else
	{
	    data++;
	    entry_num = GETSHORT(data);
	    data += 2;
	}

	for (i = 0; i < entry_num; i++)
	{
	    if (memcmp((char *) (mora_code + cmp_len),
		       (char *) (data + i * 2), 2) == 0)
	    {
		tmp_ptr = (uchar *) (data + entry_num * 2 + i * 2);
		if (mora_len == (cmp_len + 2))
		{
		    if ((*(tmp_ptr + 1) & 0x80) == 0x80)
		    {
			offset = (ushort) * tmp_ptr +
			    (ushort) (*(tmp_ptr + 1) & 0x3f) * 256;
			break;
		    }
		}
		else
		{
		    offset = (ushort) * tmp_ptr +
			    (ushort) (*(tmp_ptr + 1) & 0x3f) * 256;
		    break;
		}
	    }
	}
	if (offset == 0)
	{
	    return (-1);
	}

	if ((mora_len != (cmp_len + 2)) && (*(tmp_ptr + 1) & 0xc0) == 0xc0)
	{
	    if ((ushort) * (save_ptr + offset) != 0)
	    {
	        offset += (ushort) * (save_ptr + offset);
	    }
	    else
	    {
	        offset += GETSHORT((save_ptr + offset + 1));
	    }
	    data = (uchar *) (save_ptr + offset);
	    offset = 0;
	}
	else
	if ((*(tmp_ptr + 1) & 0x80) == 0)
	{
	    data = (uchar *) (save_ptr + offset);
	    offset = 0;
	}
	cmp_len += 2;
    }

    data = (uchar *) (save_ptr + offset);
    save_ptr = data;
    if(*data != 0){
	jdata_l = *(data++) - 1;
    }else{
	data++;
	jdata_l = GETSHORT(data) - 1;
	data += 2;
    }

    for (i = 0, flag = 0; data < (uchar *) (save_ptr + jdata_l);)
    {
	if ((uchar) * data == 0x80)
	{
	    data++;
	}

	if (((*data == 0xbf) || (*data == 0xff)) && (*(data + 1) == 0xfe))
	{
	    if (kj_len == 1 &&  kj_code[0] == 0xfffe)
	    {
		return(0);
	    }
	    for (j = 0, k = 0; (j < cmp_len) && (mora_code[j] != 0x00); j++)
	    {
		mora2pc(0, mora_code[j], kana_code, &num);
		seisho[i + j + k] = kana_code[0];
		if (num == 2)
		{
		    k++;
		    seisho[i + j + k] = kana_code[1];
		}
	    }
	    i += j + k;
	}
	else
	if (((*data == 0xbf) || (*data == 0xff)) && (*(data + 1) == 0xff))
	{
	    if (kj_len == 1 &&  kj_code[0] == 0xffff)
	    {
		return(0);
	    }
	    for (j = 0, k = 0; (j < cmp_len) && (mora_code[j] != 0x00); j++)
	    {
		mora2pc(1, mora_code[j], kana_code, &num);
		seisho[i + j + k] = kana_code[0];
		if (num == 2)
		{
		    k++;
		    seisho[i + j + k] = kana_code[1];
		}
	    }
	    i += j + k;
	}
	else
	{
	    if (((*data | 0x80) < 0x81) || ((*data | 0x80) > 0xfc) ||
		(((*data | 0x80) > 0x9f) && ((*data | 0x80) < 0xe0)))
	    {
		seisho[i] = (ushort) ((*data | 0xc0) * 256 + *(data + 1));
	    }
	    else
	    {
		seisho[i] = (ushort) ((*data | 0x80) * 256 + *(data + 1));
	    }
	    i++;
	}

	if ((*data & 0x80) == 0)
	{
	    data += 2;
	    continue;
	}

	if (kj_len == i)
	{
	    if (memcmp(seisho, kj_code, kj_len * 2) == 0)
	    {
		return (0);
	    }
	}
	data += 2;

	tg_l = (ushort) (((uchar) * data >> 6) & 0x03);
	df_l = (ushort) (((uchar) * data >> 4) & 0x03);

	data += z_mcbptr->add_info_l;
	data += tg_l;
	while (((*data & 0x80) != 0) && (df_l != 0))
	{
	    data += df_l;
	}
	data += df_l;
	i = 0;
	flag = 0;
    }

    return (-1);
}



static unsigned short m2pc[2][256] =
{				/* Hiragana */
 0x8140, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 0 */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x829f,	/* 1 */

 0x82a0, 0x82a1, 0x82a2, 0x82a3, 0x82a4, 0x82a5, 0x82a6, 0x82a7,
 0x82a8, 0x82a9, 0x82aa, 0x82ab, 0x82ac, 0x82ad, 0x82ae, 0x82af,	/* 2 */

 0x82b0, 0x82b1, 0x82b2, 0x82b3, 0x82b4, 0x82b5, 0x82b6, 0x82b7,
 0x82b8, 0x82b9, 0x82ba, 0x82bb, 0x82bc, 0x82bd, 0x82be, 0x82bf,	/* 3 */

 0x0000, 0x82c1, 0x82c2, 0x0000, 0x82c4, 0x82c5, 0x82c6, 0x82c7,
 0x82c8, 0x82c9, 0x82ca, 0x82cb, 0x82cc, 0x82cd, 0x82ce, 0x82cf,	/* 4 */

 0x82d0, 0x82d1, 0x82d2, 0x82d3, 0x82d4, 0x82d5, 0x82d6, 0x82d7,
 0x82d8, 0x82d9, 0x82da, 0x82db, 0x82dc, 0x82dd, 0x82de, 0x82df,	/* 5 */

 0x82e0, 0x82e1, 0x82e2, 0x82e3, 0x82e4, 0x82e5, 0x82e6, 0x82e7,
 0x82e8, 0x82e9, 0x82ea, 0x82eb, 0x82ec, 0x82ed, 0x82ee, 0x82a4,	/* 6 */

 0x82f0, 0x82f1, 0x0000, 0x0000, 0x815b, 0x0000, 0x0000, 0x8145,
 0x8192, 0x0000, 0x817c, 0x815e, 0x0000, 0x0000, 0x0000, 0x0000,	/* 7 */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 8 */

 0x82ab, 0x82ab, 0x82ab, 0x82ac, 0x82ac, 0x82ac, 0x82b5, 0x82b5,
 0x82b5, 0x82b6, 0x82b6, 0x82b6, 0x82bf, 0x82bf, 0x82bf, 0x0000,	/* 9 */

 0x0000, 0x0000, 0x82c9, 0x82c9, 0x82c9, 0x82d0, 0x82d0, 0x82d0,
 0x82d1, 0x82d1, 0x82d1, 0x82d2, 0x82d2, 0x82d2, 0x82dd, 0x82dd,	/* A */

 0x82dd, 0x82e8, 0x82e8, 0x82e8, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* B */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x824f,	/* C */

 0x8250, 0x8251, 0x8252, 0x8253, 0x8254, 0x8255, 0x8256, 0x8257,
 0x8258, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* D */

 0x8260, 0x8261, 0x8262, 0x8263, 0x8264, 0x8265, 0x8266, 0x8267,
 0x8268, 0x8269, 0x826a, 0x826b, 0x826c, 0x826d, 0x826e, 0x826f,	/* E */

 0x8270, 0x8271, 0x8272, 0x8273, 0x8274, 0x8275, 0x8276, 0x8277,
 0x8278, 0x8279, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* F */


/* Katakana */
 0x8140, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 0 */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8340,	/* 1 */

 0x8341, 0x8342, 0x8343, 0x8344, 0x8345, 0x8346, 0x8347, 0x8348,
 0x8349, 0x834a, 0x834b, 0x834c, 0x834d, 0x834e, 0x834f, 0x8350,	/* 2 */

 0x8351, 0x8352, 0x8353, 0x8354, 0x8355, 0x8356, 0x8357, 0x8358,
 0x8359, 0x835a, 0x835b, 0x835c, 0x835d, 0x835e, 0x835f, 0x8360,	/* 3 */

 0x0000, 0x8362, 0x8363, 0x0000, 0x8365, 0x8366, 0x8367, 0x8368,
 0x8369, 0x836a, 0x836b, 0x836c, 0x836d, 0x836e, 0x836f, 0x8370,	/* 4 */

 0x8371, 0x8372, 0x8373, 0x8374, 0x8375, 0x8376, 0x8377, 0x8378,
 0x8379, 0x837a, 0x837b, 0x837c, 0x837d, 0x837e, 0x8380, 0x8381,	/* 5 */

 0x8382, 0x8383, 0x8384, 0x8385, 0x8386, 0x8387, 0x8388, 0x8389,
 0x838a, 0x838b, 0x838c, 0x838d, 0x838e, 0x838f, 0x8390, 0x8345,	/* 6 */

 0x8392, 0x8393, 0x0000, 0x0000, 0x815b, 0x0000, 0x0000, 0x8145,
 0x8192, 0x0000, 0x817c, 0x815e, 0x0000, 0x0000, 0x0000, 0x0000,	/* 7 */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 8 */

 0x834c, 0x834c, 0x834c, 0x834d, 0x834d, 0x834d, 0x8356, 0x8356,
 0x8356, 0x8357, 0x8357, 0x8357, 0x8360, 0x8360, 0x8360, 0x0000,	/* 9 */

 0x0000, 0x0000, 0x836a, 0x836a, 0x836a, 0x8371, 0x8371, 0x8371,
 0x8372, 0x8372, 0x8372, 0x8373, 0x8373, 0x8373, 0x837e, 0x837e,	/* A */

 0x837e, 0x838a, 0x838a, 0x838a, 0x8394, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* B */

 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x824f,	/* C */

 0x8250, 0x8251, 0x8252, 0x8253, 0x8254, 0x8255, 0x8256, 0x8257,
 0x8258, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* D */

 0x8260, 0x8261, 0x8262, 0x8263, 0x8264, 0x8265, 0x8266, 0x8267,
 0x8268, 0x8269, 0x826a, 0x826b, 0x826c, 0x826d, 0x826e, 0x826f,	/* E */

 0x8270, 0x8271, 0x8272, 0x8273, 0x8274, 0x8275, 0x8276, 0x8277,
 0x8278, 0x8279, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000	/* F */
};

static unsigned short m2pc2[2][36] =
{
/* Hiragana */
 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3,
 0x82e5, 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3, 0x82e5, 0x0000,	/* 9 */

 0x0000, 0x0000, 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3, 0x82e5,
 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3, 0x82e5, 0x82e1, 0x82e3,	/* A */

 0x82e5, 0x82e1, 0x82e3, 0x82e5,/* B */

/* Katakana */
 0x8383, 0x8385, 0x8387, 0x8383, 0x8385, 0x8387, 0x8383, 0x8385,
 0x8387, 0x8383, 0x8385, 0x8387, 0x8383, 0x8385, 0x8387, 0x0000,	/* 9 */

 0x0000, 0x0000, 0x8383, 0x8385, 0x8387, 0x8383, 0x8385, 0x8387,
 0x8383, 0x8385, 0x8387, 0x8383, 0x8385, 0x8387, 0x8383, 0x8385,	/* A */

 0x8387, 0x8383, 0x8385, 0x8387	/* B */
};


mora2pc(type, mora_code, pc_code, num)
    ushort          type, pc_code[], *num;
    uchar           mora_code;
{

    pc_code[0] = m2pc[type][(ushort) mora_code];
    if (pc_code[0] == 0x0)
    {
	*num = 0;
	return;
    }
    if (mora_code >= 0x90 && mora_code <= 0xb4)
    {
	*num = 2;
	pc_code[1] = m2pc2[type][(ushort) (mora_code - 0x90)];
    }
    else
    {
	*num = 1;
    }

}
