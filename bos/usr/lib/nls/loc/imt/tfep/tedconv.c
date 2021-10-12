static char sccsid[] = "@(#)87  1.1  src/bos/usr/lib/nls/loc/imt/tfep/tedconv.c, libtw, bos411, 9428A410j 4/21/94 01:57:49";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: tedconv.c
 *
 *   ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include "ted.h"
#include "msgextrn.h"
#define buffer_len 80
unsigned char temp_buffer[buffer_len];
/* static iconv_t  iconv_flag;   */
iconv_t OpenIconv(unsigned int in_flag,
                  unsigned int out_flag)
{
unsigned char from_code[80];
unsigned char to_code[80];
static iconv_t  iconv_flag;

  if (in_flag == out_flag)
     return 0;

  strcpy(from_code,codesetinfo[in_flag].name);
  strcpy(to_code,codesetinfo[out_flag].name);

  iconv_flag = iconv_open(to_code,from_code);
  return(iconv_flag);
}

int CloseIconv(iconv_t iconv_flag)
{
  if (iconv_flag != 0)
     iconv_close(iconv_flag);
}


unsigned int StrCodeConvert ( iconv_t iconv_flag,
                              unsigned char *inbuf,
                              unsigned char *outbuf,
                              size_t *inbytesleft,
                              size_t *outbytesleft)
{
  int rtc ;

  rtc = 0;
  if (!inbuf) return ERROR;

  rtc = iconv( iconv_flag,&inbuf,inbytesleft,&outbuf,outbytesleft);

  return rtc ;

}


unsigned int Convert_Message (iconv_t iconv_flag,unsigned int out_flag)
{
  size_t        in_count,out_count;

  in_count = S_ALPHA_NUM_LEN-1;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_ALPHA_NUM_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_ALPHA_NUM_EUC,NULL,S_ALPHA_NUM_LEN-1);
  memcpy(S_ALPHA_NUM_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_CHINESE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_CHINESE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_CHINESE_EUC,NULL,S_CHINESE_LEN-1);
  memcpy(S_CHINESE_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_HALF_SIZE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_HALF_SIZE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_HALF_SIZE_EUC,NULL,S_HALF_SIZE_LEN-1);
  memcpy(S_HALF_SIZE_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_FULL_SIZE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_FULL_SIZE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_FULL_SIZE_EUC,NULL,S_FULL_SIZE_LEN-1);
  memcpy(S_FULL_SIZE_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_PHONETIC_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_PHONETIC_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_PHONETIC_EUC,NULL,S_PHONETIC_LEN-1);
  memcpy(S_PHONETIC_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_TSANG_JYE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_TSANG_JYE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_TSANG_JYE_EUC,NULL,S_TSANG_JYE_LEN-1);
  memcpy(S_TSANG_JYE_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_INTERNAL_CODE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_INTERNAL_CODE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_INTERNAL_CODE_EUC,NULL,S_INTERNAL_CODE_LEN-1);
  memcpy(S_INTERNAL_CODE_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_CAPS_LOCK_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_CAPS_LOCK_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_CAPS_LOCK_EUC,NULL,S_CAPS_LOCK_LEN-1);
  memcpy(S_CAPS_LOCK_EUC,temp_buffer,buffer_len - out_count);

  in_count = S_BLANK_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,S_BLANK_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(S_BLANK_EUC,NULL,S_BLANK_LEN-1);
  memcpy(S_BLANK_EUC,temp_buffer,buffer_len - out_count);

/*  S_CODE_FLAG_EUC[]=           */

  in_count = ALPHA_NUM_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,ALPHA_NUM_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(ALPHA_NUM_EUC,NULL,ALPHA_NUM_LEN-1);
  memcpy(ALPHA_NUM_EUC,temp_buffer,buffer_len - out_count);

  in_count = CHINESE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,CHINESE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(CHINESE_EUC,NULL,CHINESE_LEN-1);
  memcpy(CHINESE_EUC,temp_buffer,buffer_len - out_count);

  in_count = HALF_SIZE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,HALF_SIZE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(HALF_SIZE_EUC,NULL,HALF_SIZE_LEN-1);
  memcpy(HALF_SIZE_EUC,temp_buffer,buffer_len - out_count);

  in_count = FULL_SIZE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,FULL_SIZE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(FULL_SIZE_EUC,NULL,FULL_SIZE_LEN-1);
  memcpy(FULL_SIZE_EUC,temp_buffer,buffer_len - out_count);

  in_count = L_PHONETIC_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,PHONETIC_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(PHONETIC_EUC,NULL,L_PHONETIC_LEN-1);
  memcpy(PHONETIC_EUC,temp_buffer,buffer_len - out_count);

  in_count = TSANG_JYE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,TSANG_JYE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(TSANG_JYE_EUC,NULL,TSANG_JYE_LEN-1);
  memcpy(TSANG_JYE_EUC,temp_buffer,buffer_len - out_count);

  in_count = INTERNAL_CODE_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,INTERNAL_CODE_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(INTERNAL_CODE_EUC,NULL,INTERNAL_CODE_LEN-1);
  memcpy(INTERNAL_CODE_EUC,temp_buffer,buffer_len - out_count);

  in_count = CAPS_LOCK_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,CAPS_LOCK_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(CAPS_LOCK_EUC,NULL,CAPS_LOCK_LEN-1);
  memcpy(CAPS_LOCK_EUC,temp_buffer,buffer_len - out_count);

  in_count = BLANK_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,BLANK_EUC_1,temp_buffer,
                                        &in_count,&out_count);
  memset(BLANK_EUC,NULL,BLANK_LEN-1);
  memcpy(BLANK_EUC,temp_buffer,buffer_len - out_count);

  in_count = DB_BLANK_LEN;
  out_count = buffer_len;
  StrCodeConvert(iconv_flag,DB_BLANK_1,temp_buffer,
                                        &in_count,&out_count);
  memset(DB_BLANK,NULL,DB_BLANK_LEN-1);
  memcpy(DB_BLANK,temp_buffer,buffer_len - out_count);

/* CODE_FLAG_EUC[]= */

/*  S_CODE_FLAG_EUC[]=           */

   in_count = strlen(error1_str);
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,error1_str_1,temp_buffer,
                                        &in_count,&out_count);
   memset(error1_str,NULL,strlen(error1_str));
   memcpy(error1_str,temp_buffer,buffer_len - out_count);

   in_count = strlen(error2_str);
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,error2_str_1,temp_buffer,
                                        &in_count,&out_count);
   memset(error2_str,NULL,strlen(error2_str));
   memcpy(error2_str,temp_buffer,buffer_len - out_count);

/***********************************************************************/
/* message for Simpify Tsang-Jye                                       */
/***********************************************************************/

   in_count = Ph_title_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Ph_title_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Ph_title,NULL,Ph_title_LEN-1);
   memcpy(Ph_title,temp_buffer,buffer_len - out_count);

   in_count = Ph_line_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Ph_line_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Ph_line,NULL,Ph_line_LEN-1);
   memcpy(Ph_line,temp_buffer,buffer_len - out_count);

   in_count = Ph_digit_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Ph_digit_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Ph_digit,NULL,Ph_digit_LEN-1);
   memcpy(Ph_digit,temp_buffer,buffer_len - out_count);

   in_count = Ph_bottom1_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Ph_bottom1_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Ph_bottom1,NULL,Ph_bottom1_LEN-1);
   memcpy(Ph_bottom1,temp_buffer,buffer_len - out_count);

   in_count = Ph_bottom2_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Ph_bottom2_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Ph_bottom2,NULL,Ph_bottom2_LEN-1);
   memcpy(Ph_bottom2,temp_buffer,buffer_len - out_count);

/***********************************************************************/
/* message for Simpify Tsang-Jye                                       */
/***********************************************************************/

   in_count = Stj_title_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Stj_title_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Stj_title,NULL,Stj_title_LEN-1);
   memcpy(Stj_title,temp_buffer,buffer_len - out_count);

   in_count = Tj_title_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Tj_title_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Tj_title,NULL,Tj_title_LEN-1);
   memcpy(Tj_title,temp_buffer,buffer_len - out_count);

   in_count = Stj_line_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Stj_line_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Stj_line,NULL,Stj_line_LEN-1);
   memcpy(Stj_line,temp_buffer,buffer_len - out_count);

   in_count = Stj_digit_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Stj_digit_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Stj_digit,NULL,Stj_digit_LEN-1);
   memcpy(Stj_digit,temp_buffer,buffer_len - out_count);

   in_count = Stj_bottom1_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Stj_bottom1_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Stj_bottom1,NULL,Stj_bottom1_LEN-1);
   memcpy(Stj_bottom1,temp_buffer,buffer_len - out_count);

   in_count = Tj_bottom1_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Tj_bottom1_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Tj_bottom1,NULL,Tj_bottom1_LEN-1);
   memcpy(Tj_bottom1,temp_buffer,buffer_len - out_count);

   in_count = Stj_bottom2_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,Stj_bottom2_1,temp_buffer,
                                        &in_count,&out_count);
   memset(Stj_bottom2,NULL,Stj_bottom2_LEN-1);
   memcpy(Stj_bottom2,temp_buffer,buffer_len - out_count);

/***********************************************************************/
/* message for Stroke list box                                         */
/***********************************************************************/
   in_count = stroke_title_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stroke_title_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stroke_title,NULL,stroke_title_LEN-1);
   memcpy(stroke_title,temp_buffer,buffer_len - out_count);

   in_count = stroke_line_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stroke_line_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stroke_line,NULL,stroke_line_LEN-1);
   memcpy(stroke_line,temp_buffer,buffer_len - out_count);

   in_count = stroke_digit_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stroke_digit_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stroke_digit,NULL,stroke_digit_LEN-1);
   memcpy(stroke_digit,temp_buffer,buffer_len - out_count);

   in_count = stroke_bottom1_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stroke_bottom1_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stroke_bottom1,NULL,stroke_bottom1_LEN-1);
   memcpy(stroke_bottom1,temp_buffer,buffer_len - out_count);

   in_count = stroke_bottom2_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stroke_bottom2_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stroke_bottom2,NULL,stroke_bottom2_LEN-1);
   memcpy(stroke_bottom2,temp_buffer,buffer_len - out_count);

   in_count = strokechar_LEN;
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,strokechar_1,temp_buffer,
                                        &in_count,&out_count);
   memset(strokechar,NULL,strokechar_LEN-1);
   memcpy(strokechar,temp_buffer,buffer_len - out_count);

   max_ic_input_len = codesetinfo[out_flag].max_ic_input_len;
   strcpy(S_CODE_FLAG,codesetinfo[out_flag].s_status_line_code_flag);
   strcpy(CODE_FLAG,codesetinfo[out_flag].l_status_line_code_flag);

   in_count = strlen(stj_radical);
   out_count = buffer_len;
   StrCodeConvert(iconv_flag,stj_radical_1,temp_buffer,
                                        &in_count,&out_count);
   memset(stj_radical,NULL,strlen(stj_radical));
   memcpy(stj_radical,temp_buffer,buffer_len - out_count);

}
