/* @(#)24	1.1  src/bos/usr/bin/virscan/vsutil.h, cmdsvir, bos411, 9428A410j 4/3/91 19:22:56 */
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: NONE
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

void mask(register byte *addr, register int count);
void pr_hex(byte *hex, int len);
void pr_masked_hex(byte *masked_hex, register int len, boolean is_complex_sig);
char *get_line_from_sig_file(char *buffer, int length, FILE *file_ptr);
void dump_bsect(byte *buffer, int size);

#if BOOTS
void beep_until_key_pressed(void);
#endif

void pr_then_clear_line(char *msg);
void clear_line_if_required(void);
void outp_progress_line(char *msgbuf);
int  mycmp(byte *masked_signature,
           byte *test_addr,
           register int len,
           int *mismatched_bytes_count, /* Only valid if function rv == 0 */
           struct sigdats *sig);
void convert_message(char *message);
