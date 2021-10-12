static char sccsid[] = "@(#)57  1.7.1.1  src/bos/kernext/x25/jsfac.c, sysxx25, bos411, 9428A410j 4/1/94 11:48:58";
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS: convert_binary_to_bcd, convert_bcd_to_binary,
 *            convert_string_to_bcd, convert_bcd_to_string, gime_more_memory,
 *            _x25_convert_cb_fac_to_byte_stream,
 *            _x25_convert_byte_stream_to_cb_fac
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */

#if defined (_KERNEL)
#include <sys/malloc.h>
#else
#include <stdlib.h>
#endif

#include <string.h>

#ifdef XDH
#include "jsfac.h"
#else
#include <x25/jsfac.h>
#endif

#define UNSIGNED_2 ((unsigned)2)


/*****************************************************************************/
/*  Discussion                   Facilities                                  */
/*                                                                           */
/*                             Facilities format                             */
/*                             -----------------                             */
/*                                                                           */
/* Facilities are laid out as follows.                                       */
/*                                                                           */
/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³                         X.25 facilities                         ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0x0                               ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0x0                               ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³        non-X.25 facilities provided by the local network        ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0x0                               ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0xFF                              ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³        non-X.25 facilities provided by the remote network       ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0x0                               ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                               0x0F                              ³        */
/*ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³                 CCITT-specified DTE facilities                  ³        */
/*ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/* If any section is not required, both it and the preceeding punctuation    */
/* (i.e. the 0x0 0xnn) can be missed out.                                    */
/* Within each section, the facilites format is defined as a series of       */
/* facility codes, followed by a number of bytes of arguments.  The number   */
/* of bytes of arguments are defined by the format of the facility code.     */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ¿        */
/*³0³     Class     ³                                               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/* The class can have one of the following values                            */
/*                                                                           */
/* 00     Class A.  This has a single byte parameter field                   */
/* 01     Class B.  This has two bytes as a parameter                        */
/* 10     Class C.  This has three bytes as a parameter                      */
/* 11     Class D. The next byte defines how long the parameter is           */
/*                                                                           */
/* There is one special facility code, 0xFF, which is reserved for           */
/* extension of the facility codes.  The octet following this one indicates  */
/* an extended facility code having the format A, B, C or D. Repetition of   */
/* the facility code 0xFF is permitted resulting in additional extensions.   */
/*                                                                           */
/*                              X.25 facilities                              */
/*                              ~~~~~~~~~~~~~~~                              */
/*                                                                           */
/*                                                                           */
/* packet size                                                               */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x42                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³           Reserved           ³    Transmission packet size    ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³           Reserved           ³      Receive packet size       ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_PSIZ                                                        */
/*               Packet Size selection                                       */
/*                                                                           */
/*                                                                           */
/*        psiz_clg                                                           */
/*               Indicates the requested size for packets transmitted from   */
/*               the Calling DTE. Supported values are                       */
/*               0x04 = 16 octets                                            */
/*               0x05 = 32 octets                                            */
/*               0x06 = 64 octets                                            */
/*               0x07 = 128 octets                                           */
/*               0x08 = 256 octets                                           */
/*               0x09 = 512 octets                                           */
/*               0x0A = 1024 octets                                          */
/*               0x0B = 2048 octets                                          */
/*               0x0C = 4096 octets                                          */
/*        psiz_cld                                                           */
/*               Requested size for packets transmitted from the Called DTE. */
/*               Supported values are the same as for psiz_clg.              */
/*                                                                           */
/* window size                                                               */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x43                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³           Reserved           ³    Transmission window size    ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³           Reserved           ³      Receive window size       ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_WSIZ                                                        */
/*               Window Size selection                                       */
/*                                                                           */
/*                                                                           */
/*        wsiz_clg                                                           */
/*               Requested size for the window for packets transmitted by    */
/*               the Calling DTE. Values are in the range from 0x01 to 0x07  */
/*               inclusive                                                   */
/*        wsiz_cld                                                           */
/*               Requested size for the window for packets to be             */
/*               transmitted by the Called DTE. Values are in the range      */
/*               from 0x01 to 0x07 inclusive                                 */
/*                                                                           */
/* Throughput class                                                          */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x02                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³   Outgoing Throughput class  ³   Incoming Throughput Class    ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_TCLS                                                        */
/*               Throughput class required                                   */
/*                                                                           */
/*                                                                           */
/*        tcls_clg                                                           */
/*               Throughput Class requested for data to be sent by the       */
/*               Calling DTE. Supported values are                           */
/*               0x07 = 1200 bit/s                                           */
/*               0x08 = 2400 bit/s                                           */
/*               0x09 = 4800 bit/s                                           */
/*               0x0A = 9600 bit/s                                           */
/*               0x0B = 19200 bit/s                                          */
/*               0x0C = 48000 bit/s                                          */
/*        tcls_cld                                                           */
/*               Throughput Class request for data sent from the Called DTE. */
/*               Supported values are the same as for tcls_clg.              */
/*                                                                           */
/* Closed user group selection                                               */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x03                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of CUG   ³      Second BCD digit of CUG   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        Or                                                                 */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x47                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of CUG   ³      Second BCD digit of CUG   ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Third BCD digit of CUG   ³      Fourth BCD digit of CUG   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CUG                                                         */
/*               Closed User Group selection required                        */
/*                                                                           */
/*                                                                           */
/*        cug_id Indicates the value of a closed user group.                 */
/*                                                                           */
/* Cug with outgoing access                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x09                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of CUG   ³      Second BCD digit of CUG   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        Or                                                                 */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x48                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of CUG   ³      Second BCD digit of CUG   ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Third BCD digit of CUG   ³      Fourth BCD digit of CUG   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_OA_CUG                                                      */
/*               Closed User Group with outgoing access                      */
/*                                                                           */
/*                                                                           */
/*        cug_id Indicates the value of a closed user group.                 */
/*                                                                           */
/* Bilateral closed user group selection                                     */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x41                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of CUG   ³      Second BCD digit of CUG   ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Third BCD digit of CUG   ³      Fourth BCD digit of CUG   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_BI_CUG                                                      */
/*               Bilateral Closed User Group selection required              */
/*                                                                           */
/*                                                                           */
/*        cug_id Indicates the value of a closed user group.                 */
/*                                                                           */
/* Reverse Charging and Fast Select                                          */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x01                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ´        */
/*³1³      A        ³                                       ³   B   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        A can be one of                                                    */
/*                                                                           */
/*        00     Fast select not selected                                    */
/*        01     Fast select not selected                                    */
/*        10     Fast select requested with no restriction on response       */
/*        11     Fast select requested with restriction on response          */
/*                                                                           */
/*        B can be one of                                                    */
/*                                                                           */
/*        0      No reverse charging requested                               */
/*        1      Reverse charging requested                                  */
/*                                                                           */
/*                                                                           */
/*        X25FLG_FASTSEL                                                     */
/*               Fast Select                                                 */
/*        X25FLG_FASTSEL_RSP                                                 */
/*               Indicates whether a restricted response is required when    */
/*               X25FLG_FASTSEL is also requested.                           */
/*        X25FLG_REV_CHRG                                                    */
/*               Reverse Charge required                                     */
/*                                                                           */
/* Network user identification                                               */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC6                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³             Length of Network User Identification             ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                                                               ³        */
/*ÃÄ´                  Network User Identification                  ³        */
/*³*³                                                               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_NUI_DATA                                                    */
/*               Network User Identification                                 */
/*                                                                           */
/*                                                                           */
/*        nui_data                                                           */
/*               Network user identification data in format identified by    */
/*               the network administrator.                                  */
/*        nui_data_len                                                       */
/*               The number of bytes given in nui_data                       */
/*                                                                           */
/* Charging information request                                              */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x04                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ´        */
/*³1³                                                       ³   A   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        A can be one of                                                    */
/*                                                                           */
/*        0      Charging information not requested                          */
/*        1      Charging information requested                              */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CI_REQUEST                                                  */
/*               Charging Information - Requesting Service                   */
/*                                                                           */
/* Charging (monetary unit)                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC5                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                Length of charging information                 ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                                                               ³        */
/*ÃÄ´                    Charging Identification                    ³        */
/*³*³                                                               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CI_MON_UNT                                                  */
/*               Charging information - Monetary Unit                        */
/*                                                                           */
/*                                                                           */
/*        ci_mon_unt                                                         */
/*               Charging information - monetary unit data                   */
/*        ci_mon_unt_len                                                     */
/*               The number of bytes                                         */
/*                                                                           */
/* Charging (segment count)                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC2                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                Length of charging information                 ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                                                               ³        */
/*ÃÄ´                    Charging Identification                    ³        */
/*³*³                                                               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CI_SEG_CNT                                                  */
/*               Charging information - Segment Count                        */
/*                                                                           */
/*                                                                           */
/*        ci_seg_cnt                                                         */
/*               Charging information - segment count data.                  */
/*        ci_seg_cnt_len                                                     */
/*               Length                                                      */
/*                                                                           */
/* Charging (call duration)                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC1                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                Length of charging information                 ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                                                               ³        */
/*ÃÄ´                    Charging Identification                    ³        */
/*³*³                                                               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CI_CALL_DUR                                                 */
/*               Charging information - Call Duration                        */
/*                                                                           */
/*                                                                           */
/*        ci_call_dur                                                        */
/*               Charging information - call duration data                   */
/*        ci_call_dur_len                                                    */
/*               The number of bytes of charging information                 */
/*                                                                           */
/* RPOA selection                                                            */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x44                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³     First BCD digit of RPOA  ³      Second BCD digit of RPOA  ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Third BCD digit of RPOA  ³      Fourth BCD digit of RPOA  ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        Or                                                                 */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC4                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                Length of RPOA information                     ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³   First BCD digit of RPOA #1 ³    Second BCD digit of RPOA #1 ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³3³   Third BCD digit of RPOA #1 ³    Fourth BCD digit of RPOA #1 ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³*³   First BCD digit of RPOA #n ³    Second BCD digit of RPOA #n ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³*³   Third BCD digit of RPOA #n ³    Fourth BCD digit of RPOA #n ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_RPOA                                                        */
/*               Recognised Private Operating Agency selection required      */
/*                                                                           */
/*                                                                           */
/*        rpoa_id                                                            */
/*               Indicates the requested RPOA transit network.               */
/*               Each RPOA is stored as an unsigned short.                   */
/*        rpoa_id_len                                                        */
/*               The number of RPOAs pointed to by rpoa_id.                  */
/*                                                                           */
/* Called line address modified notification (CLAMN)                         */
/*        One of                                                             */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x08                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³   0   ³                       ³               A               ³        */
/*ÀÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        A can have one of the following values                             */
/*                                                                           */
/*        0x7    Call distribution within a hunt group                       */
/*        0x1    Call redirection due to original DTE busy                   */
/*        0x9    Call redirection due to original DTE out of order           */
/*        0x0F   Call redirection due to prior request from originally       */
/*               called DTE for systematic redirection                       */
/*                                                                           */
/*        Or                                                                 */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x08                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³   1   ³                          B                            ³        */
/*ÀÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        B is passed from the remote DTE giving a reason for the            */
/*        redirection.                                                       */
/*                                                                           */
/*        X25FLG_CLAMN                                                       */
/*               Called Line Address Modified Notification                   */
/*                                                                           */
/*                                                                           */
/*        clamn  The reason for called line address modified notification.   */
/*               This is the value of the byte at offset 1, i.e. either A    */
/*               or B|0x80.                                                  */
/*                                                                           */
/* Call redirection notification                                             */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC3                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³               Length of redirection information               ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                               A                               ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³3³                              ³    Length of called address    ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³4³                                                               ³        */
/*ÃÄ´                        Called Address                         ³        */
/*³*³                            (BCD)                              ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        A is                                                               */
/*                                                                           */
/*        0x1    Call redirection due to original DTE busy                   */
/*        0x9    Call redirection due to original DTE out of order           */
/*        0x0F   Call redirection due to prior request from originally       */
/*               called DTE for systematic redirection                       */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CALL_REDR                                                   */
/*               Call Redirection Notification                               */
/*                                                                           */
/*                                                                           */
/*        call_redr_reason                                                   */
/*               Contains reason for call redirection.  This is byte A.      */
/*        call_redr_address                                                  */
/*               An ASCIIZ string of the original called DTE address         */
/*                                                                           */
/* Transit delay selection and indication                                    */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x49                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                                                               ³        */
/*ÃÄ´                 Transit delay in milliseconds                 ³        */
/*³2³                 ( in binary, hi byte first )                  ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_TRAN_DEL                                                    */
/*               Transit Delay Selection and Notification                    */
/*                                                                           */
/*                                                                           */
/*        tran_del                                                           */
/*               Transit delay in milliseconds                               */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                      CCITT-specified DTE facilities                       */
/*                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                       */
/*                                                                           */
/*                                                                           */
/* Calling address extension                                                 */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xCB                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                   Number of bytes following                   ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Use       ³      Length of calling extension address      ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*³3³                                                               ³        */
/*ÃÄ´                    Calling extension address                  ³        */
/*³*³                            (BCD)                              ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        where Use can be one of                                            */
/*                                                                           */
/*        00     To carry an entire calling OSI NSAP address                 */
/*        01     To carry a partial calling OSI NSAP address                 */
/*        10     To carry a non-OSI calling address                          */
/*        11     Reserved                                                    */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CALLING_ADDR_EXT                                            */
/*               Calling address extension                                   */
/*                                                                           */
/*                                                                           */
/*        calling_addr_ext_use                                               */
/*               Values as above for Use.                                    */
/*        calling_addr_ext                                                   */
/*               Up to 40 decimal digits containing the calling address      */
/*               extension. The address extension is stored in ASCIIZ        */
/*               format.                                                     */
/*                                                                           */
/* called address extension                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0xC9                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                   Number of bytes following                   ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³     Use       ³      Length of called extension address       ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³3³                                                               ³        */
/*ÃÄ´                    Called extension address                   ³        */
/*³*³                            (BCD)                              ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_CALLED_ADDR_EXT                                             */
/*               called address extension                                    */
/*                                                                           */
/*                                                                           */
/*        called_addr_ext_use                                                */
/*               Values as above for Use.                                    */
/*        called_addr_ext                                                    */
/*               Up to 40 decimal digits containing the called address       */
/*               extension. The address extension is stored in ASCIIZ        */
/*               format.                                                     */
/*                                                                           */
/* Minimum throughput class                                                  */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x0A                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³  Calling minimum throughput  ³   Called miniumum throughput   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_MIN_TCLS                                                    */
/*               Quality of Service Negotiation - Minimum throughput class   */
/*                                                                           */
/*                                                                           */
/*        min_tcls_clg                                                       */
/*               Throughput class requested for data to be sent by the       */
/*               Calling DTE. Supported values are                           */
/*               0x07 = 1200 bit/s                                           */
/*               0x08 = 2400 bit/s                                           */
/*               0x09 = 4800 bit/s                                           */
/*               0x0A = 9600 bit/s                                           */
/*               0x0B = 19200 bit/s                                          */
/*               0x0C = 48000 bit/s                                          */
/*        min_tcls_cld                                                       */
/*               Throughput class request for data sent from the Called DTE. */
/*               Supported values are the same as for tcls_clg.              */
/*                                                                           */
/* End-to-end transmit delay facility                                        */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x49                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³1³                 Length of the following area                  ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³2³                                                               ³        */
/*ÃÄ´           Cumulative transit delay in milliseconds            ³        */
/*³3³                 ( in binary, hi byte first )                  ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³4³                                                               ³        */
/*ÃÄ´          Requested end-to-end delay in milliseconds           ³        */
/*³5³                 ( in binary, hi byte first )                  ³        */
/*ÃÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´        */
/*³6³                                                               ³        */
/*ÃÄ´       Maximum acceptable transit delay in milliseconds        ³        */
/*³7³                 ( in binary, hi byte first )                  ³        */
/*ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*                                                                           */
/*        X25FLG_END_TO_END_DEL                                              */
/*               Quality of Service Negotiation - End-to-end transit delay   */
/*                                                                           */
/*                                                                           */
/*        end_to_end_del                                                     */
/*               Specifies cumulative, requested end-to-end and maximum      */
/*               acceptable transit delays.                                  */
/*        end_to_end_del_len                                                 */
/*               The number of values in the stream at end_to_end_del.       */
/*               This can be one of 1, 2 or 3, as the requested end-to-end   */
/*               delay and maximum acceptable transit delay are optional.    */
/*                                                                           */
/* Expedited data negotiation                                                */
/*                                                                           */
/*                                                                           */
/*ÚÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿        */
/*³0³                             0x0B                              ³        */
/*ÃÄÅÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ´        */
/*³1³                                                       ³   A   ³        */
/*ÀÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ        */
/*                                                                           */
/*        A can be one of                                                    */
/*                                                                           */
/*        0      No use of expedited data                                    */
/*        1      Use of expedited data                                       */
/*                                                                           */
/*                                                                           */
/*        X25FLG_EXP_DATA                                                    */
/*               Expedited Data Negotiation                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                            Non-X.25 facilities                            */
/*                            ~~~~~~~~~~~~~~~~~~~                            */
/*                                                                           */
/*                                                                           */
/* Any    All other facilities are stored in the facilities extension        */
/*        structure.  This include national options.                         */
/*                                                                           */
/*        X25FLG_FACEXT                                                      */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*        fac_ext_len                                                        */
/*               The length of an array which contains further information   */
/*               on facilities                                               */
/*        fac_ext                                                            */
/*               Pointer to extra facility information provided by the user  */
/*               or network. It is provided to allow extra facilities which  */
/*               the main cb_fac structure does not cater for. The elements  */
/*               of fac_ext are copied directly into the facility field.     */
/*                                                                           */
/*               When the information is network/remote DTE provided, it is  */
/*               the responsibility of the user application to interpret     */
/*               the field. Facility markers must be used in fac_ext if      */
/*               such facilities are required.                               */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*  Function        convert_binary_to_bcd                                    */
/*                                                                           */
/*  Prototype       void convert_binary_to_bcd(                              */
/*                    uchar *to;                                             */
/*                    unsigned from;                                         */
/*                    unsigned length)                                       */
/*                                                                           */
/*  Description     Converts between a binary number and its BCD format.     */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      to          A pointer to where to store the BCD string               */
/*      from        The number to convert from                               */
/*      length      The number of bytes of BCD to generate                   */
/*****************************************************************************/
#ifdef _NO_PROTO
static void convert_binary_to_bcd(to,from,length)
unsigned char *to;
unsigned from;
unsigned length;
#else
static void convert_binary_to_bcd(
  unsigned char *to,
  unsigned from,
  unsigned length)
#endif
{
  while (length!=0)
  {
    to[length-1]=(((from/10)%10)<<4)+from%10;
    from /= 100;
    --length;
  };
}

/*****************************************************************************/
/*  Function        convert_bcd_to_binary                                    */
/*                                                                           */
/*  Prototype       unsigned convert_bcd_to_binary(                          */
/*                    unsigned char *from;                                   */
/*                    unsigned length)                                       */
/*                                                                           */
/*  Description     Converts between a bcd number and a binary one           */
/*                                                                           */
/*  Return Code                                                              */
/*                  The binary number stored in BCD format at from.          */
/*                                                                           */
/*  Parameters                                                               */
/*      from        A pointer to where the BCD string is stored              */
/*      length      The number of bytes of BCD to read                       */
/*****************************************************************************/
#ifdef _NO_PROTO
static unsigned convert_bcd_to_binary(from,length)
unsigned char *from;
unsigned length;
#else
static unsigned convert_bcd_to_binary(
  unsigned char *from,
  unsigned length)
#endif
{
  unsigned to=0;
  unsigned v1;
  unsigned v2;

  while (length!=0)
  {
    v1= *from&0xF0;
    v2= *from&0x0F;
    to=(to*100)+((v1>>4)*10) + v2;
    ++from;
    --length;
  };
  return to;
}

/*****************************************************************************/
/*  Function        convert_string_to_bcd                                    */
/*                                                                           */
/*  Prototype       unsigned convert_string_to_bcd(                          */
/*                    unsigned char *to;                                     */
/*                    char  *from)                                           */
/*                                                                           */
/*  Description     Converts between a string and a bcd string               */
/*                                                                           */
/*  Return Code                                                              */
/*                  The number of bytes that the bcd string occupies.        */
/*                                                                           */
/*  Parameters                                                               */
/*      to          A pointer to store the BCD string.  If the number        */
/*                  of characters in the string is odd, this is padded with  */
/*                  a 0, e.g. "123" is converted into 0x12 0x30.             */
/*      from        The string to read from.  This is in ASCIIZ format       */
/*                  and MUST consist of all digits.  No check is performed   */
/*                  that this is so.                                         */
/*****************************************************************************/
#ifdef _NO_PROTO
static unsigned convert_string_to_bcd(to,from)
unsigned char *to;
char  *from;
#else
static unsigned convert_string_to_bcd(
  unsigned char *to,
  char  *from)
#endif
{
  unsigned char *to_ptr=to;

  while (*from)
  {
    *to= (*from++&0x0F)<<4;
    if (*from)
      *to |= (*from++&0x0F);
    ++to;
  }
  return to-to_ptr;
}

/*****************************************************************************/
/*  Function        convert_bcd_to_string                                    */
/*                                                                           */
/*  Prototype       void convert_bcd_to_string(                              */
/*                    unsigned char   *from;                                 */
/*                    char    *to;                                           */
/*                    unsigned length)                                       */
/*                                                                           */
/*  Description     Converts between a BCD string and an                     */
/*                  ASCIIZ representation of it, e.g.                        */
/*                  0x1234 into "1234".                                      */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      from        The BCD is stored here.                                  */
/*      to          A pointer to where to store the output string            */
/*      length      The number of characters to convert                      */
/*****************************************************************************/
#ifdef _NO_PROTO
static void convert_bcd_to_string(from,to,length)
  unsigned char   *from;
  char    *to;
  unsigned length;
#else
static void convert_bcd_to_string(
  unsigned char   *from,
  char    *to,
  unsigned length)
#endif
{
  unsigned v1;
  unsigned v2;

  while (length!=0)
  {
    v1= *from&0xF0;
    v2= *from&0x0F;
    *to++  = (v1>>4)+'0';
    if (--length)
    {
      *to++=(v2&0x0F)+'0';
      ++from;
      --length;
    }
  }
  *to++='\0';
}

/*****************************************************************************/
/*  Function        gime_more_memory                                         */
/*                                                                           */
/*  Prototype       unsigned char *gime_more_memory(                         */
/*                    unsigned char **overflow_ptr;                          */
/*                    unsigned length)                                       */
/*                                                                           */
/*  Description     Returns a pointer to a section of free memory.           */
/*                  This is either allocated off the heap or from a section  */
/*                  of storage hanging off *overflow_ptr.  If *overflow_ptr  */
/*                  is NULL, it will be allocated from the heap.  Otherwise  */
/*                  it is allocated from *overflow_ptr to                    */
/*                  *overflow_ptr+length. *overflow_ptr is then adjusted to  */
/*                  point after the allocated area.                          */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to a free section of memory.                   */
/*                                                                           */
/*  Parameters                                                               */
/*      overflow_ptr                                                         */
/*                  A pointer to a pointer to some free memory.  If it       */
/*                  is a pointer to NULL, the memory is allocated off the    */
/*                  heap. Otherwise, *overflow_ptr is adjusted to skip over  */
/*                  the newly allocated memory.                              */
/*      length      The number of bytes to allocate.  This is rounded up     */
/*                  to an even number before allocation so that alignment    */
/*                  requirements for RPOAs are satisfied.                    */
/*****************************************************************************/
#ifdef _NO_PROTO
static unsigned char *gime_more_memory(overflow_ptr,length)
  unsigned char **overflow_ptr;
  unsigned length;
#else
static unsigned char *gime_more_memory(
  unsigned char **overflow_ptr,
  unsigned length)
#endif
{
  unsigned char *return_p;

  if (*overflow_ptr==NULL)                        /* Memory off the heap ?   */
#if !defined(_KERNEL)
    return_p=malloc(length);
#else
    return_p=xmalloc(length,(unsigned)4, kernel_heap);
#endif
  else
  {
    length+=(length&1);                           /* Make length even        */
    return_p= *overflow_ptr;
    *overflow_ptr+=length;                        /* Skip over allocated area*/
  }
  return return_p;
}

/*****************************************************************************/
/* This table is the main conversion table between the flags that are passed */
/* in and the facility codes that are passed out.  It is also used in the    */
/* oposite direction.  The third field divides the facilities up into        */
/* four types.                                                               */
/* 1) X25_FACILITIES.  These are standard run of the mill easy-to-handle     */
/*                     facilities                                            */
/* 2) AWKWARD_X25_FACILITIES.  Some facilities have nasty formats.  These    */
/*                     are marked thus.                                      */
/* 3) OTHER_FACILITIES These are punctuation markers and unknown facilities  */
/* 4) CCITT_DTE_FACILITIES These are the DTE specific facility codes.        */
/*****************************************************************************/
static x25fac_flag_to_code_t flag_to_fac_code[]=
{
  { X25FLG_RPOA,        X25FAC_RPOA_CODE, X25_FACILITIES },
  { X25FLG_RPOA,        X25FAC_RPOAE_CODE, X25_FACILITIES },
  { X25FLG_PSIZ,        X25FAC_PSIZ_CODE, X25_FACILITIES },
  { X25FLG_WSIZ,        X25FAC_WSIZ_CODE, X25_FACILITIES },
  { X25FLG_TCLS,        X25FAC_TCLS_CODE, X25_FACILITIES },
  { X25FLG_FASTSEL,     X25FAC_FASTSEL_CODE, AWKWARD_X25_FACILITIES, X25FAC_FASTSEL_PARM },
  { X25FLG_FASTSEL_RSP, X25FAC_FASTSEL_CODE, AWKWARD_X25_FACILITIES, X25FAC_FASTSEL_RSP_PARM },
  { X25FLG_REV_CHRG,    X25FAC_FASTSEL_CODE, AWKWARD_X25_FACILITIES, X25FAC_REV_CHRG_PARM },
  { X25FLG_CUG,         X25FAC_CUG_BASIC_CODE, X25_FACILITIES  },
  { X25FLG_CUG,         X25FAC_CUG_EXT_CODE, X25_FACILITIES },
  { X25FLG_OA_CUG,      X25FAC_CUG_OAB_CODE, X25_FACILITIES },
  { X25FLG_OA_CUG,      X25FAC_CUG_OAE_CODE, X25_FACILITIES },
  { X25FLG_BI_CUG,      X25FAC_BI_CUG_CODE, X25_FACILITIES },
  { X25FLG_NUI_DATA,    X25FAC_NUI_DATA_CODE, X25_FACILITIES },
  { X25FLG_CI_SEG_CNT,  X25FAC_CI_SEG_CNT_CODE, X25_FACILITIES },
  { X25FLG_CI_MON_UNT,  X25FAC_CI_MON_UNT_CODE, X25_FACILITIES },
  { X25FLG_CI_CALL_DUR, X25FAC_CI_CALL_DUR_CODE, X25_FACILITIES },
  { X25FLG_CI_REQUEST,  X25FAC_CI_REQUEST_CODE,X25_FACILITIES },
  { X25FLG_CLAMN,       X25FAC_CLAMN_CODE,X25_FACILITIES },
  { X25FLG_CALL_REDR,   X25FAC_CALL_REDR_CODE,X25_FACILITIES },
  { X25FLG_TRAN_DEL,    X25FAC_TRAN_DEL_CODE,X25_FACILITIES },

  { X25FLG_FACEXT,      X25FAC_PUNCTUATION_MARKER, OTHER_FACILITIES },

  { X25FLG_CALLING_ADDR_EXT, X25FAC_CALLING_ADDR_EXT_CODE,CCITT_DTE_FACILITIES },
  { X25FLG_CALLED_ADDR_EXT,  X25FAC_CALLED_ADDR_EXT_CODE,CCITT_DTE_FACILITIES },
  { X25FLG_MIN_TCLS,         X25FAC_MIN_TCLS_CODE,CCITT_DTE_FACILITIES },
  { X25FLG_END_TO_END_DEL,   X25FAC_END_TO_END_DEL_CODE,CCITT_DTE_FACILITIES },
  { X25FLG_EXP_DATA,         X25FAC_EXP_DATA_CODE,CCITT_DTE_FACILITIES },

  { X25FLG_FACEXT,      X25FAC_PUNCTUATION_MARKER, CCITT_DTE_FACILITIES }
};

/*****************************************************************************/
/*  Function        _x25_convert_cb_fac_to_byte_stream                       */
/*                                                                           */
/*  Prototype       int _x25_convert_cb_fac_to_byte_stream(                  */
/*                    byte      *fac_stream,                                 */
/*                    cb_fac_t  *cb_fac_ptr,                                 */
/*                    x25_ddi_t *ddi_ptr)                                    */
/*                                                                           */
/*  Description     Converts between a cb_fac structure and                  */
/*                  the X.25 facilities byte stream that should be           */
/*                  sent in the packet.                                      */
/*                                                                           */
/*                                                                           */
/*                  No checking is performed on the facilities information.  */
/*                  If the parameters are not within the ISO defined         */
/*                  limits, the results are undefined.                       */
/*                                                                           */
/*  Return Code                                                              */
/*                  The length of the facilities stored in fac_stream.       */
/*                                                                           */
/*  Parameters                                                               */
/*      fac_stream  A pointer to an array of bytes (at least                 */
/*                  X25_MAX_FACILITIES_LENGTH long) that will be updated to  */
/*                  contain the facilites.  The length of the byte stream    */
/*                  in this area is returned from the function.              */
/*      cb_fac_ptr  A pointer to the generic facilities structure.           */
/*      ddi_ptr     A pointer to the ddi.  This is used to decide            */
/*                  1) Whether to use extended or basic format for CUG       */
/*****************************************************************************/
#ifdef _NO_PROTO
unsigned _x25_convert_cb_fac_to_byte_stream(fac_stream,cb_fac_ptr,iocinfo_ptr)
  unsigned char *fac_stream;
  cb_fac_t *cb_fac_ptr;
  x25_devinfo_t *iocinfo_ptr;
#else
unsigned _x25_convert_cb_fac_to_byte_stream(
  unsigned char *fac_stream,
  cb_fac_t *cb_fac_ptr,
  x25_devinfo_t *iocinfo_ptr)
#endif
{
  unsigned parm_len;
  unsigned len=0;
  ulong    flag_value;
  int      i,j;
  int      fastsel_len= -1;
  unsigned char    mask;
  bool     found_any_dte_specific=0;
  unsigned char    fac_code;
  unsigned char    fac_type;
  unsigned ext_copy;
  ulong    cb_flags=cb_fac_ptr->flags;

#ifdef _UNIT_TEST
  ulong    sub_flags= *(int *)iocinfo_ptr;
#else
  ulong    sub_flags=iocinfo_ptr->un.x25.supported_facilities;
#endif
  bool     done_fac_ext=FALSE;
  int      fac_ext_index=0;
  int      current_fac_len;

  for (i=0;i<sizeof(flag_to_fac_code)/sizeof(flag_to_fac_code[0]);++i)
  {
    if ((cb_flags&flag_to_fac_code[i].flag)==0)   /* This facility field set */
      continue;
    fac_code=flag_to_fac_code[i].code;            /* The X.25 facility code  */
    flag_value=flag_to_fac_code[i].flag;          /* The flag                */
    cb_flags&=~flag_value;                        /* Don't parse it again    */
    mask=flag_to_fac_code[i].mask;                /* Mask for bit operations */
    fac_type=flag_to_fac_code[i].type;            /* X25_, CCITT_DTE_ etc.   */

    switch(fac_type)
    {
    case X25_FACILITIES:                          /* Simple facilities, fill */
      fac_stream[len++]=fac_code;
      break;
    case CCITT_DTE_FACILITIES:
      if (found_any_dte_specific==FALSE &&
	(sub_flags & X25_FAC_MARK_0F))
      {
	fac_stream[len++]=X25FAC_FACILITIES_PUNCTUATION;/* Add punctuation   */
	fac_stream[len++]=X25FAC_CCITT_DTE_SPECIFIC_MARK;
	found_any_dte_specific=TRUE;
      }
      fac_stream[len++]=fac_code;                 /* Fill in code            */
      break;
    case OTHER_FACILITIES:
      break;
    case AWKWARD_X25_FACILITIES:
    default:
      break;
    }

    switch(flag_value)
    {
    case X25FLG_FASTSEL:                          /* Awkward facilities      */
    case X25FLG_FASTSEL_RSP:
    case X25FLG_REV_CHRG:
      if (sub_flags & X25_FAC_REV_CHRG)
      {
	/*********************************************************************/
	/* Fast select and reverse charging share the same facility code.    */
	/* Thus, if you have more than one in the list, you need to use the  */
	/* same elements.  This is done by remembering the fastselect offset */
	/* in fastsel_len so that we can stamp all over it if we need to     */
	/* The facility code has to be filled in explicitly as these are     */
	/* awkward facilities                                                */
	/*********************************************************************/
	if (fastsel_len==-1)
	{
	  fac_stream[len++] = X25FAC_FASTSEL_CODE;
	  fastsel_len=len;
	  fac_stream[len++] = 0;
	}
	fac_stream[fastsel_len]|=mask;
      }
      break;

    case X25FLG_FACEXT:                           /* User defined facilities */
      /***********************************************************************/
      /* If we are now on the second try at doing the extended facilities    */
      /* Just copy the remaining facilities out to the packet                */
      /***********************************************************************/
      if (done_fac_ext)
      {
	for(ext_copy=fac_ext_index;ext_copy<cb_fac_ptr->fac_ext_len;++ext_copy)
	  fac_stream[len++] = cb_fac_ptr->fac_ext[ext_copy];
      }
      else
      {
	done_fac_ext=TRUE;                        /* We've done the facext   */
	for (ext_copy=fac_ext_index;ext_copy<cb_fac_ptr->fac_ext_len;)
	{
	  fac_code=cb_fac_ptr->fac_ext[ext_copy++];
	  /*******************************************************************/
	  /* Have we got a CCITT DTE specific user defined facilities in this*/
	  /* block ? If so, we must break out and handle them next loop      */
	  /* round                                                           */
	  /*******************************************************************/
	  if (fac_code==X25FAC_FACILITIES_PUNCTUATION)
	  {
	    fac_code=cb_fac_ptr->fac_ext[ext_copy++];
	    if (fac_code==X25FAC_CCITT_DTE_SPECIFIC_MARK)
	    {
	      cb_flags|=X25FLG_FACEXT;
	      fac_ext_index=ext_copy;
	      break;
	    }
	    else
	    {
	      fac_stream[len++]=X25FAC_FACILITIES_PUNCTUATION;
	      fac_stream[len++]=fac_code;
	    }
	  }
	  else
	  {
	    /*****************************************************************/
	    /* Extended facilties can have Facility Shifts in them.          */
	    /*****************************************************************/
	    switch(fac_code)
	    {
	    case X25FAC_SHIFT_EXTENSION:
	      fac_stream[len++]=X25FAC_SHIFT_EXTENSION;
	      while (cb_fac_ptr->fac_ext[ext_copy]==X25FAC_SHIFT_EXTENSION)
	      {
	        fac_stream[len++]=X25FAC_SHIFT_EXTENSION;
	        ++ext_copy;
	      };
	      /*fac_code=fac_stream[ext_copy++];*/
	      fac_code=cb_fac_ptr->fac_ext[ext_copy++];
	      break;
	    }

	    fac_stream[len++]=fac_code;           /* Copy code into bin      */
	    switch(fac_code&X25FAC_TYPE_MASK)
	    {
	    case X25FAC_TYPE_A_MASK:
	      current_fac_len=1;
	      break;
	    case X25FAC_TYPE_B_MASK:
	      current_fac_len=2;
	      break;
	    case X25FAC_TYPE_C_MASK:
	      current_fac_len=3;
	      break;
	    case X25FAC_TYPE_D_MASK:              /* Length stored in stream */
	      current_fac_len=cb_fac_ptr->fac_ext[ext_copy++];
	      fac_stream[len++]=current_fac_len;
	      break;
	    }

	    /*****************************************************************/
	    /* The facilities length is stored in current_fac_len            */
	    /*****************************************************************/
#if !defined (_KERNEL)
	    memcpy(&fac_stream[len],&cb_fac_ptr->fac_ext[ext_copy],current_fac_len);
#else
	    bcopy(&cb_fac_ptr->fac_ext[ext_copy],&fac_stream[len],current_fac_len);
#endif
	    ext_copy+=current_fac_len;
	    len+=current_fac_len;
	    /* break; */
	  }
	}
	/*********************************************************************/
	/* Next time round, we should not do the facilities again!           */
	/*********************************************************************/
	fac_ext_index=ext_copy;
      }
      break;

    case X25FLG_WSIZ:                             /* Window size             */
      if (sub_flags & X25_FAC_WSIZ)
      {
	fac_stream[len++]=cb_fac_ptr->wsiz_cld;
	fac_stream[len++]=cb_fac_ptr->wsiz_clg;
      }
      else
	--len;
      break;

    case X25FLG_PSIZ:                             /* Packet size             */
      if (sub_flags & X25_FAC_PSIZ)
      {
	fac_stream[len++]=cb_fac_ptr->psiz_cld;
	fac_stream[len++]=cb_fac_ptr->psiz_clg;
      }
      else
	--len;
      break;

    case X25FLG_TCLS:                             /* Throughput class        */
      if (sub_flags & X25_FAC_TCLS)
      {
	fac_stream[len++]=(cb_fac_ptr->tcls_cld << 4) | cb_fac_ptr->tcls_clg;
      }
      else
	--len;
      break;

    case X25FLG_BI_CUG:                           /* Bilateral CUG           */
      if (sub_flags & X25_FAC_BI_CUG)
      {
	convert_binary_to_bcd(&fac_stream[len],(unsigned)cb_fac_ptr->cug_id,UNSIGNED_2);
	len+=2;
      }
      else
	--len;
      break;

    case X25FLG_CUG:                              /* Cugs basic and out-bound*/
      if (sub_flags & X25_FAC_CUG)
      {
	parm_len=1;
      }
      else if (sub_flags & X25_FAC_EXT_CUG)
      {
	parm_len=2;
	fac_stream[len-1]=X25FAC_CUG_EXT_CODE;
      }
      else
      {
	parm_len=0;
	--len;
      }
      if (parm_len!=0)
      {
	convert_binary_to_bcd(&fac_stream[len],(unsigned)cb_fac_ptr->cug_id,parm_len);
	len+=parm_len;
      }
      break;

    case X25FLG_OA_CUG:
      if (sub_flags & X25_FAC_OA_CUG)
      {
	parm_len=1;
      }
      else if (sub_flags & X25_FAC_EXT_OA_CUG)
      {
	parm_len=2;
	fac_stream[len-1]=X25FAC_CUG_OAE_CODE;
      }
      else
      {
	parm_len=0;
	--len;
      }
      if (parm_len!=0)
      {
	convert_binary_to_bcd(&fac_stream[len],(unsigned)cb_fac_ptr->cug_id,parm_len);
	len+=parm_len;
      }
      break;

    case X25FLG_RPOA:
      if (cb_fac_ptr->rpoa_id_len!=1)             /* Need extended format ?  */
      {
	if (sub_flags & X25_FAC_EXT_RPOA)
	{
	  fac_stream[len-1]=X25FAC_RPOAE_CODE;
	  fac_stream[len++]=cb_fac_ptr->rpoa_id_len*2;
	  for (j=0;j<cb_fac_ptr->rpoa_id_len;j++)     /* Copy over           */
	  {
	    convert_binary_to_bcd(
	      &fac_stream[len],
	      (unsigned)cb_fac_ptr->rpoa_id[j],
	      UNSIGNED_2);
	    len+=2;
	  }
	}
	else
	  --len;
      }
      else if (sub_flags & X25_FAC_RPOA)
      {
	convert_binary_to_bcd(
	  &fac_stream[len],
	  (unsigned)cb_fac_ptr->rpoa_id[0],
	  UNSIGNED_2);
	len+=2;
      }
      else
	--len;
      break;

    case X25FLG_NUI_DATA:
      if (sub_flags & X25_FAC_NUI_DATA)
      {
	fac_stream[len++]=cb_fac_ptr->nui_data_len;
#if !defined(_KERNEL)
	memcpy(&fac_stream[len],cb_fac_ptr->nui_data,cb_fac_ptr->nui_data_len);
#else
	bcopy(cb_fac_ptr->nui_data,&fac_stream[len],cb_fac_ptr->nui_data_len);
#endif
	len+=cb_fac_ptr->nui_data_len;
      }
      else
	--len;
      break;

    case X25FLG_CI_MON_UNT:
      if (sub_flags & X25_FAC_CI_MON_UNT)
      {
	fac_stream[len++]=cb_fac_ptr->ci_mon_unt_len;
#if !defined(_KERNEL)
	memcpy(&fac_stream[len],cb_fac_ptr->ci_mon_unt,cb_fac_ptr->ci_mon_unt_len);
#else
	bcopy(cb_fac_ptr->ci_mon_unt,&fac_stream[len],cb_fac_ptr->ci_mon_unt_len);
#endif
	len+=cb_fac_ptr->ci_mon_unt_len;
      }
      else
	--len;
      break;

    case X25FLG_CI_SEG_CNT:
      if (sub_flags & X25_FAC_CI_SEG_CNT)
      {
	fac_stream[len++]=cb_fac_ptr->ci_seg_cnt_len;
#if !defined(_KERNEL)
	memcpy(&fac_stream[len],cb_fac_ptr->ci_seg_cnt,cb_fac_ptr->ci_seg_cnt_len);
#else
	bcopy(cb_fac_ptr->ci_seg_cnt,&fac_stream[len],cb_fac_ptr->ci_seg_cnt_len);
#endif
	len+=cb_fac_ptr->ci_seg_cnt_len;
      }
      else
	--len;
      break;

    case X25FLG_CI_REQUEST:
      if (sub_flags & X25_FAC_CI_REQUEST)
      {
	fac_stream[len++]=X25FAC_CI_REQUESTED_PARM;
      }
      else
	--len;
      break;

    case X25FLG_CI_CALL_DUR:
      if (sub_flags & X25_FAC_CI_CALL_DUR)
      {
	fac_stream[len++]=cb_fac_ptr->ci_call_dur_len;
#if !defined(_KERNEL)
	memcpy(&fac_stream[len],cb_fac_ptr->ci_call_dur,cb_fac_ptr->ci_call_dur_len);
#else
	bcopy(cb_fac_ptr->ci_call_dur,&fac_stream[len],cb_fac_ptr->ci_call_dur_len);
#endif
	len+=cb_fac_ptr->ci_call_dur_len;
      }
      else
	--len;
      break;

    case X25FLG_CLAMN:
      if (sub_flags & X25_FAC_CLAMN)
      {
	fac_stream[len++]=cb_fac_ptr->clamn;
      }
      else
	--len;
      break;

    case X25FLG_TRAN_DEL:
      if (sub_flags & X25_FAC_TRAN_DEL)
      {
	fac_stream[len++]=(cb_fac_ptr->tran_del>>8);
	fac_stream[len++]=(cb_fac_ptr->tran_del&0xFF);
      }
      else
	--len;
      break;

    case X25FLG_CALLING_ADDR_EXT:
      if (sub_flags & X25_FAC_MARK_0F)
      {
	parm_len=strlen(cb_fac_ptr->calling_addr_ext);
	fac_stream[len++] = (parm_len+3)/2;
	fac_stream[len++] = parm_len +
	  (cb_fac_ptr->calling_addr_ext_use<<X25FAC_EXT_ADDR_USE_SHIFT);
	len+=convert_string_to_bcd(&fac_stream[len],cb_fac_ptr->calling_addr_ext);
      }
      else
	--len;
      break;

    case X25FLG_CALLED_ADDR_EXT:
      if (sub_flags & X25_FAC_MARK_0F)
      {
	parm_len=strlen(cb_fac_ptr->called_addr_ext);
	fac_stream[len++] = (parm_len+3)/2;
	fac_stream[len++] = parm_len +
	  (cb_fac_ptr->called_addr_ext_use<<X25FAC_EXT_ADDR_USE_SHIFT);
	len+=convert_string_to_bcd(&fac_stream[len],cb_fac_ptr->called_addr_ext);
      }
      else
	--len;
      break;

    case X25FLG_MIN_TCLS:
      if (sub_flags & X25_FAC_MARK_0F)
      {
	fac_stream[len++]=(cb_fac_ptr->min_tcls_cld<<4) | cb_fac_ptr->min_tcls_clg;
      }
      else
	--len;
      break;

    case X25FLG_CALL_REDR:
      if (sub_flags & X25_FAC_CALL_REDR)
      {
	fac_stream[len+1]=cb_fac_ptr->call_redr_reason;
	fac_stream[len+2]=strlen(cb_fac_ptr->call_redr_addr);
	parm_len=convert_string_to_bcd(&fac_stream[len+3],cb_fac_ptr->call_redr_addr);
	parm_len+=2;
	fac_stream[len++]=parm_len;
	len+=parm_len;
      }
      else
	--len;
      break;

    case X25FLG_END_TO_END_DEL:
      if (sub_flags & X25_FAC_MARK_0F)
      {
	fac_stream[len++]=cb_fac_ptr->end_to_end_del_len*2;
	for (j=0;j<cb_fac_ptr->end_to_end_del_len;++j)
	{
	  fac_stream[len++] = (cb_fac_ptr->end_to_end_del[j]>>8);
	  fac_stream[len++] = (cb_fac_ptr->end_to_end_del[j]&0xFF);
	}
      }
      else
	--len;
      break;

    case X25FLG_EXP_DATA:
      if (sub_flags & X25_FAC_MARK_0F)
      {
	fac_stream[len++]=X25FAC_EXP_DATA_PARM;
      }
      else
	--len;
      break;
    }
  }
  return(len);
}

/*****************************************************************************/
/*  Function        _x25_convert_byte_stream_to_cb_fac                       */
/*                                                                           */
/*  Description     Converts between the X.25 facilities byte stream         */
/*                  that has arrived in a packet and a                       */
/*                  cb_fac_ptr structure.                                    */
/*                                                                           */
/*                                                                           */
/*                  No checking is performed on the facilities stream.       */
/*                  Any errors in the stream or unknown facilities will be   */
/*                  accumulated in the fac_ext structure.                    */
/*                                                                           */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      cb_fac_ptr  A pointer to the generic facilities structure.  On       */
/*                  input, the following fields should be filled in          */
/*                  flags                                                    */
/*                         Should be set to the flags that caller wishes to  */
/*                         parse. Thus, if the caller want to test to see    */
/*                         if reverse charging is set, then set the flags    */
/*                         value to X25FLG_REV_CHRG. If the caller wants to  */
/*                         parse all of the facilities, the flags value      */
/*                                                                           */
/*                  On return from the function, the flags value is set to   */
/*                  indicate which facilities, that were requested to be     */
/*                  looked for, have been found.                             */
/*                  For example, if                                          */
/*                  the flags value was set to X25FLG_CALLED_ADDR_EXT        */
/*                  before the call and there was a called address           */
/*                  extension facility in the packet, the flags, on return,  */
/*                  will be set to X25FLG_CALLED_ADDR_EXT and the            */
/*                  called_addr_ext field will be set to point to            */
/*                  an area of memory that holds an ASCIIZ representation    */
/*                  of the extended called address.                          */
/*      overflow_area                                                        */
/*                  This is used to hold any extra data that needs to be     */
/*                  referenced by the cb_fac_ptr.                            */
/*                  If this is set to NULL, the memory required              */
/*                  is claimed using malloc().                               */
/*                  If this is non-NULL it must point to an                  */
/*                  area of memory at least X25_MAX_OVERFLOW_AREA long,      */
/*                  ( the maximum number of bytes that may be required for   */
/*                  dynamic store ). overflow_area should be word-aligned.   */
/*                  In the example above, when the facilities were parsed    */
/*                  the called_addr_ext pointed to an area                   */
/*                  of memory containing the ASCIIZ extended called          */
/*                  address. This address would be located in the            */
/*                  overflow_area if non-NULL or on the heap if NULL.        */
/*      fac_stream  A pointer to an array of bytes that contains the         */
/*                  incoming packet's facilities.                            */
/*      fac_length  The length of the byte stream at fac_stream              */
/*****************************************************************************/
#ifdef _NO_PROTO
int _x25_convert_byte_stream_to_cb_fac(cb_fac_ptr,overflow_area,fac_stream,fac_length)
  cb_fac_t *cb_fac_ptr;
  unsigned char *overflow_area;
  unsigned char *fac_stream;
  unsigned fac_length;
#else
int _x25_convert_byte_stream_to_cb_fac(
  cb_fac_t *cb_fac_ptr,
  unsigned char *overflow_area,
  unsigned char *fac_stream,
  unsigned fac_length)
#endif
{
  unsigned char    bin[X25_MAX_FACILITIES_LENGTH];     /* unrecognised facilities     */
  unsigned bin_len=0;                          /* Length of unrecognised facs */
  unsigned parm_len;
  bool     known_fac_code;
  int      i,j;
  ulong    flag_value;
  ulong    new_flag_value=0x0;
  unsigned char    fac_code;
  unsigned char    fac_type;
  unsigned char    x25fac_state=X25FAC_STATE_X25;
  unsigned char    binfac_state=X25FAC_STATE_X25;
  unsigned current_fac_len;

#ifdef DEBUG
  outputf(
    "_x25_byte_stream_to_cb_fac called flag_value=%x fac_length=%d\n",
    cb_fac_ptr->flags,
    fac_length);
#endif
  for (i=0;i<fac_length;)
  {
    fac_code = fac_stream[i++];
    known_fac_code=FALSE;
    for (j=0;j<sizeof(flag_to_fac_code)/sizeof(flag_to_fac_code[0]);++j)
    {
      if (fac_code==flag_to_fac_code[j].code)
      {
	known_fac_code=TRUE;
	flag_value=flag_to_fac_code[j].flag;
	fac_type=flag_to_fac_code[j].type;
	break;
      }
    }

    if (known_fac_code==FALSE)
    {
      flag_value=X25FLG_FAC_UNKNOWN;
      fac_type=OTHER_FACILITIES;
      current_fac_len=0;
    }
    else
    {
      switch(fac_code&X25FAC_TYPE_MASK)
      {
      case X25FAC_TYPE_A_MASK:
	current_fac_len=1;
	break;
      case X25FAC_TYPE_B_MASK:
	current_fac_len=2;
	break;
      case X25FAC_TYPE_C_MASK:
	current_fac_len=3;
	break;
      case X25FAC_TYPE_D_MASK:
	current_fac_len=fac_stream[i++];
	break;
      }

      switch(fac_type)
      {
      case X25_FACILITIES:
	if (x25fac_state!=X25FAC_STATE_X25)
	  flag_value=X25FLG_FAC_UNKNOWN;
	else if ((flag_value&cb_fac_ptr->flags)==0)
	{
	  i+=current_fac_len;
	  continue;
	}
	else
	  new_flag_value|=flag_value;
	break;
	
      case CCITT_DTE_FACILITIES:
	if (x25fac_state!=X25FAC_STATE_CCITT_DTE)
	  flag_value=X25FLG_FAC_UNKNOWN;
	else if ((flag_value&cb_fac_ptr->flags)==0)
	{
	  i+=current_fac_len;
	  continue;
	}
	else
	  new_flag_value|=flag_value;
	break;
	
      case AWKWARD_X25_FACILITIES:
	if (x25fac_state!=X25FAC_STATE_X25)
	  flag_value=X25FLG_FAC_UNKNOWN;
	break;
      case OTHER_FACILITIES:
	break;
      }

      if (flag_value==X25FLG_FAC_UNKNOWN &&
	((fac_code&X25FAC_TYPE_MASK)==X25FAC_TYPE_D_MASK))
	--i;
    }

    switch(flag_value)
    {
    case X25FLG_FASTSEL:
      if (fac_stream[i]&X25FAC_FASTSEL_PARM)
	new_flag_value|=X25FLG_FASTSEL;
      if (fac_stream[i]&X25FAC_FASTSEL_RSP_PARM)
	new_flag_value|=X25FLG_FASTSEL_RSP;
      if (fac_stream[i]&X25FAC_REV_CHRG_PARM)
	new_flag_value|=X25FLG_REV_CHRG;
      i+=current_fac_len;
      break;

    case X25FLG_PSIZ:
      cb_fac_ptr->psiz_cld = fac_stream[i++];
      cb_fac_ptr->psiz_clg = fac_stream[i++];
      break;

    case X25FLG_WSIZ:
      cb_fac_ptr->wsiz_cld = fac_stream[i++];
      cb_fac_ptr->wsiz_clg = fac_stream[i++];
      break;

    case X25FLG_CUG:
    case X25FLG_OA_CUG:
    case X25FLG_BI_CUG:
      cb_fac_ptr->cug_id=convert_bcd_to_binary(&fac_stream[i],current_fac_len);
      i+=current_fac_len;
      break;

    case X25FLG_RPOA:
      parm_len=1;
      if (fac_code==X25FAC_RPOAE_CODE)
      {
	parm_len=current_fac_len/2;
      }
      cb_fac_ptr->rpoa_id=(ushort *)gime_more_memory(&overflow_area,parm_len*2);
      if (cb_fac_ptr->rpoa_id==NULL)
	return -1;
      for (j=0;j<parm_len;++j)
	cb_fac_ptr->rpoa_id[j]=convert_bcd_to_binary(&fac_stream[i+j*2],UNSIGNED_2);
      cb_fac_ptr->rpoa_id_len=parm_len;
      i+=current_fac_len;
      break;

    case X25FLG_TCLS:
      cb_fac_ptr->tcls_cld = (fac_stream[i] & 0xF0) >>4;
      cb_fac_ptr->tcls_clg = (fac_stream[i++] & 0x0F);
      break;

    case X25FLG_CI_REQUEST:
      if (fac_stream[i]==0)
	new_flag_value&=~X25FLG_CI_REQUEST;
      i+=current_fac_len;
      break;

    case X25FLG_CLAMN:
      cb_fac_ptr->clamn=fac_stream[i++];
      break;

    case X25FLG_TRAN_DEL:
      cb_fac_ptr->tran_del=fac_stream[i++]>>8;
      cb_fac_ptr->tran_del+=fac_stream[i++];
      break;

    case X25FLG_CALLING_ADDR_EXT:
      parm_len=fac_stream[i];
      cb_fac_ptr->calling_addr_ext_use=parm_len>>X25FAC_EXT_ADDR_USE_SHIFT;
      parm_len&=~X25FAC_EXT_ADDR_USE_MASK;
      convert_bcd_to_string(
	&fac_stream[i+1],
	cb_fac_ptr->calling_addr_ext,
	parm_len);
      i+=current_fac_len;
      break;

    case X25FLG_CALLED_ADDR_EXT:
      parm_len=fac_stream[i];
      cb_fac_ptr->called_addr_ext_use=parm_len>>X25FAC_EXT_ADDR_USE_SHIFT;
      parm_len&=~X25FAC_EXT_ADDR_USE_MASK;
      convert_bcd_to_string(
	&fac_stream[i+1],
	cb_fac_ptr->called_addr_ext,
	parm_len);
      i+=current_fac_len;
      break;

    case X25FLG_MIN_TCLS:
      cb_fac_ptr->min_tcls_cld = (fac_stream[i] & 0xF0) >>4;
      cb_fac_ptr->min_tcls_clg = (fac_stream[i++] & 0x0F);
      break;

    case X25FLG_END_TO_END_DEL:
      cb_fac_ptr->end_to_end_del_len=current_fac_len/2;
      for (j=0;j<cb_fac_ptr->end_to_end_del_len;++j)
      {
	cb_fac_ptr->end_to_end_del[j]=
	  (fac_stream[i]<<8)+fac_stream[i+1];
	i+=2;
      }
      break;

    case X25FLG_CALL_REDR:
      cb_fac_ptr->call_redr_reason=fac_stream[i];
      parm_len=fac_stream[i+1];
      convert_bcd_to_string(
	&fac_stream[i+2],
	cb_fac_ptr->call_redr_addr,
	parm_len);
      i+=current_fac_len;
      break;

    case X25FLG_EXP_DATA:
      if (fac_stream[i]==0)
	new_flag_value&=~X25FLG_EXP_DATA;
      i+=current_fac_len;
      break;

    case X25FLG_FACEXT:
      switch(fac_stream[i])
      {
      case X25FAC_INTRA_OPTIONS_MARK:
	x25fac_state=X25FAC_STATE_INTRA;
	bin[bin_len++]=fac_code;
	bin[bin_len++]=fac_stream[i];
	break;
      case X25FAC_CCITT_DTE_SPECIFIC_MARK:
	x25fac_state=X25FAC_STATE_CCITT_DTE;
	break;
      case X25FAC_INTER_OPTIONS_MARK:
	x25fac_state=X25FAC_STATE_INTER;
	bin[bin_len++]=fac_code;
	bin[bin_len++]=fac_stream[i];
	break;
      default:
	bin[bin_len++]=fac_code;
	bin[bin_len++]=fac_stream[i];
	break;
      }
      ++i;
      break;

    case X25FLG_NUI_DATA:
      cb_fac_ptr->nui_data=gime_more_memory(&overflow_area,current_fac_len);
      if (cb_fac_ptr->nui_data==NULL)
	return -1;
#if !defined (_KERNEL)
      memcpy(cb_fac_ptr->nui_data,&fac_stream[i],current_fac_len);
#else
      bcopy(&fac_stream[i],cb_fac_ptr->nui_data,current_fac_len);
#endif
      cb_fac_ptr->nui_data_len=current_fac_len;
      i+=current_fac_len;
      break;

    case X25FLG_CI_MON_UNT:
      cb_fac_ptr->ci_mon_unt=gime_more_memory(&overflow_area,current_fac_len);
      if (cb_fac_ptr->ci_mon_unt==NULL)
	return -1;
#if !defined (_KERNEL)
      memcpy(cb_fac_ptr->ci_mon_unt,&fac_stream[i],current_fac_len);
#else
      bcopy(&fac_stream[i],cb_fac_ptr->ci_mon_unt,current_fac_len);
#endif
      cb_fac_ptr->ci_mon_unt_len=current_fac_len;
      i+=current_fac_len;
      break;

    case X25FLG_CI_SEG_CNT:
      cb_fac_ptr->ci_seg_cnt=gime_more_memory(&overflow_area,current_fac_len);
      if (cb_fac_ptr->ci_seg_cnt==NULL)
	return -1;
#if !defined (_KERNEL)
      memcpy(cb_fac_ptr->ci_seg_cnt,&fac_stream[i],current_fac_len);
#else
      bcopy(&fac_stream[i],cb_fac_ptr->ci_seg_cnt,current_fac_len);
#endif
      cb_fac_ptr->ci_seg_cnt_len=current_fac_len;
      i+=current_fac_len;
      break;

    case X25FLG_CI_CALL_DUR:
      cb_fac_ptr->ci_call_dur=gime_more_memory(&overflow_area,current_fac_len);
      if (cb_fac_ptr->ci_call_dur==NULL)
	return -1;
#if !defined (_KERNEL)
      memcpy(cb_fac_ptr->ci_call_dur,&fac_stream[i],current_fac_len);
#else
      bcopy(&fac_stream[i],cb_fac_ptr->ci_call_dur,current_fac_len);
#endif
      cb_fac_ptr->ci_call_dur_len=current_fac_len;
      i+=current_fac_len;
      break;

    case X25FLG_FAC_UNKNOWN:
      /***********************************************************************/
      /* If we have an unknown facility and we are in CCITT DTE specific     */
      /* section of the facilities, the facilities extension should have     */
      /* the CCITT specific marker added to it                               */
      /***********************************************************************/
      if (x25fac_state==X25FAC_STATE_CCITT_DTE &&
	binfac_state!=X25FAC_STATE_CCITT_DTE)
      {
	bin[bin_len++]=X25FAC_FACILITIES_PUNCTUATION;
	bin[bin_len++]=X25FAC_CCITT_DTE_SPECIFIC_MARK;
      }

      switch(fac_code)
      {
      case X25FAC_SHIFT_EXTENSION:
	bin[bin_len++]=X25FAC_SHIFT_EXTENSION;
	while (fac_stream[i]==X25FAC_SHIFT_EXTENSION)
	{
	  bin[bin_len++]=X25FAC_SHIFT_EXTENSION;
	  ++i;
	};
	fac_code=fac_stream[i];
	++i;
	break;
      }

      bin[bin_len++]=fac_code;                    /* Copy code into bin      */
      switch(fac_code&X25FAC_TYPE_MASK)
      {
      case X25FAC_TYPE_A_MASK:
	current_fac_len=1;
	break;
      case X25FAC_TYPE_B_MASK:
	current_fac_len=2;
	break;
      case X25FAC_TYPE_C_MASK:
	current_fac_len=3;
	break;
      case X25FAC_TYPE_D_MASK:
	current_fac_len=fac_stream[i];
	bin[bin_len++]=current_fac_len;
	++i;
	break;
      }
#if !defined (_KERNEL)
      memcpy(&bin[bin_len],&fac_stream[i],current_fac_len);
#else
      bcopy(&fac_stream[i],&bin[bin_len],current_fac_len);
#endif
      i+=current_fac_len;
      bin_len+=current_fac_len;
      break;
    }
  }

  if (bin_len ==0)
    new_flag_value &=~X25FLG_FACEXT;
  else
  {
    new_flag_value |= X25FLG_FACEXT;
    cb_fac_ptr->fac_ext=gime_more_memory(&overflow_area,bin_len);
    if (cb_fac_ptr->fac_ext==NULL)
      return -1;
#if !defined (_KERNEL)
    memcpy(cb_fac_ptr->fac_ext,bin,bin_len);
#else
    bcopy(bin,cb_fac_ptr->fac_ext,bin_len);
#endif
    cb_fac_ptr->fac_ext_len = bin_len;
  }

  cb_fac_ptr->flags &= new_flag_value;

#if defined(_KERNEL) && defined(DEBUG)
  outputf(
    "_x25_byte_stream_to_cb_fac flag_value=%x ret_flag_value=%x\n",
    new_flag_value,
    cb_fac_ptr->flags);
#endif

  return(0);
}
