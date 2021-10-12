/* @(#)58  1.6  src/bos/kernext/x25/jsfac.h, sysxx25, bos411, 9428A410j 1/31/91 16:59:04 */
#ifndef _H_JSFAC
#define _H_JSFAC
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS:
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

#include <sys/types.h>


#if defined(XDH)
#include "jsdefs.h"
#include "xdhx25.h"
#endif

#if defined(QLLC)
#include <x25/jsdefs.h>
#include <x25/xdhx25.h>
#endif

#if defined(API)
#include <x25/jsdefs.h>
#include <sys/x25user.h>
#endif

#if defined(APPS)
#include <x25/jsdefs.h>
#include <sys/x25user.h>
#endif


struct intl_x25fac_flag_to_code_t
{
  u_long    flag;
  u_char    code;
  u_char    type;
  unsigned  mask;
};

typedef struct intl_x25fac_flag_to_code_t x25fac_flag_to_code_t;


#ifndef X25FLG_RPOA
/* X25APIUSE ON */
/*****************************************************************************/
/* These are the values for the extended calling and called addr use flags   */
/*****************************************************************************/
#define X25FAC_ADDR_EXT_USE_ENTIRE_OSI_NSAP  (0)
#define X25FAC_ADDR_EXT_USE_PARTIAL_OSI_NSAP (1)
#define X25FAC_ADDR_EXT_USE_NON_OSI          (2)

struct cb_fac_struct
{
  u_long   flags ;				  /* Mask of X25FLG values   */

						  /* X25FLG_FACEXT activated */
  unsigned fac_ext_len;				  /* Length of fac_ext       */
  u_char  *fac_ext;				  /* For Non-X.25 facilities */

						  /* X25FLG_PSIZ activated   */
  u_char   psiz_clg;				  /* Calling packet size     */
						  /*  ISO8208 encoded (4-12) */
  u_char   psiz_cld;				  /* Called  packet size     */
						  /*  ISO8208 encoded (4-12) */

						  /* X25FLG_WSIZ activated   */
  u_char   wsiz_clg;				  /* Calling window size     */
						  /*            1-7 or 1-127 */
  u_char   wsiz_cld;				  /* Called  window size     */
						  /*            1-7 or 1-127 */

						  /* X25FLG_TCLS activated   */
  u_char   tcls_clg;				  /* Calling throughput class*/
						  /*  ISO8208 encoded (7-12) */
  u_char   tcls_cld;				  /* Called  throughput class*/
						  /*  ISO8208 encoded (7-12) */

						  /* X25FLG_RPOA activated   */
  unsigned rpoa_id_len;				  /* Number of RPOAs         */
  ushort  *rpoa_id;				  /* List of RPOAs           */

						  /* X25FLG_CUG           or */
						  /* X25FLG_OA_CUG        or */
						  /* X25FLG_BI_CUG activated */
  ushort   cug_id;				  /* Closed User Group Id    */

						  /* X25FLG_NUI_DATA         */
  unsigned nui_data_len;			  /* Number of bytes of NUI  */
  u_char  *nui_data;				  /* The NUI data            */

						  /* X25FLG_CI_SEG_CNT       */
  unsigned ci_seg_cnt_len;			  /* Number of bytes         */
  u_char  *ci_seg_cnt;				  /* The segment count       */

						  /* X25FLG_CI_MON_UNT       */
  unsigned ci_mon_unt_len;			  /* Number of bytes         */
  u_char  *ci_mon_unt;				  /* Format is PTT defined   */

						  /* X25FLG_CI_CALL_DUR      */
  unsigned ci_call_dur_len;			  /* Number of bytes         */
  u_char  *ci_call_dur;				  /* Format is PTT defined   */

						  /* X25FLG_CLAMN            */
  u_char   clamn;				  /* Reason for modification */
						  /*                         */
						  /*  0x7 distribution within*/
						  /*       hunt group        */
						  /*  0x1 DTE busy           */
						  /*  0x9 DTE out of order   */
						  /*  0xF Systematic DTE     */
						  /*       redirection       */

						  /* X25FLG_CALL_REDR        */
						  /* Originally called DTE   */
  u_char   call_redr_addr[X25_MAX_ASCII_ADDRESS_LENGTH];
  u_char   call_redr_reason;			  /* Reason for redirection  */

						  /* X25FLG_TRAN_DEL         */
  short    tran_del;				  /* Transit delay (ms)      */

						  /* X25FLG_CALLING_ADDR_EXT */
  u_char   calling_addr_ext_use;		  /* See X25FAC_ADDR_EXT_USE */
  char     calling_addr_ext[X25_MAX_EXT_ADDR_DIGITS+1];/* ASCIIZ             */

						  /* X25FLG_CALLED_ADDR_EXT  */
  u_char   called_addr_ext_use;			  /* See X25FAC_ADDR_EXT_USE */
  char     called_addr_ext[X25_MAX_EXT_ADDR_DIGITS+1];/* ASCIIZ              */

						  /* X25FLG_MIN_TCLS         */
  u_char   min_tcls_clg;			  /* Calling min throughput  */
						  /*                 (7-12)  */
  u_char   min_tcls_cld;			  /* Called  min throughput  */
						  /*                 (7-12)  */

						  /* X25FLG_END_TO_END_DEL   */
  unsigned end_to_end_del_len;			  /* Number of entries (1-3) */
  ushort   end_to_end_del[3];			  /* [0] = Cumulative delay  */
						  /* [1] = Desired delay     */
						  /* [2] = Maximum acceptable*/
};
/* X25APIUSE OFF */
typedef struct cb_fac_struct cb_fac_t;

/* X25APIUSE ON */
/*********************************************************************/
/*                                                                   */
/*   Structure Flag Definitions                                      */
/*                                                                   */
/*********************************************************************/
#define X25FLG_RPOA             0x00000001
#define X25FLG_PSIZ             0x00000002
#define X25FLG_WSIZ             0x00000004
#define X25FLG_TCLS             0x00000008
#define X25FLG_REV_CHRG         0x00000010
#define X25FLG_FASTSEL          0x00000020
#define X25FLG_FASTSEL_RSP      0x00000040
#define X25FLG_CUG              0x00000080
#define X25FLG_OA_CUG           0x00000100
#define X25FLG_BI_CUG           0x00000200
#define X25FLG_NUI_DATA         0x00000400
#define X25FLG_CI_SEG_CNT       0x00000800
#define X25FLG_CI_MON_UNT       0x00001000
#define X25FLG_CI_CALL_DUR      0x00002000
#define X25FLG_CI_REQUEST       0x00004000
#define X25FLG_CLAMN            0x00008000
#define X25FLG_CALL_REDR        0x00010000
#define X25FLG_TRAN_DEL         0x00020000
#define X25FLG_CALLING_ADDR_EXT 0x00040000
#define X25FLG_CALLED_ADDR_EXT  0x00080000
#define X25FLG_MIN_TCLS         0x00100000
#define X25FLG_END_TO_END_DEL   0x00200000
#define X25FLG_EXP_DATA         0x00400000
#define X25FLG_FACEXT           0x00800000

/* X25APIUSE OFF                                                             */
#endif
/*****************************************************************************/
/* Maximum length of the overflow area for an application calling            */
/* byte_stream to cb_fac                                                     */
/*****************************************************************************/
#define  X25_MAX_OVERFLOW_AREA        (255)

/*****************************************************************************/
/* This is the reserved first u_char for the options marker                  */
/*****************************************************************************/
#define X25FAC_FACILITIES_PUNCTUATION  0x00

#define X25FAC_INTRA_OPTIONS_MARK      0x00
#define X25FAC_CCITT_DTE_SPECIFIC_MARK 0x0F
#define X25FAC_INTER_OPTIONS_MARK      0xFF

#define X25FAC_SHIFT_EXTENSION         0xFF

/*****************************************************************************/
/* Encoded length depends on fac type - length = code + parm fields          */
/* No 4-byte or n-byte facilities are supported by this product              */
/* explicitly (Type C and Type D respectively)                               */
/*****************************************************************************/
#define X25FAC_TYPE_A_LENGTH 2
#define X25FAC_TYPE_B_LENGTH 3

#define X25FAC_TYPE_A_MASK (0x00)
#define X25FAC_TYPE_B_MASK (0x40)
#define X25FAC_TYPE_C_MASK (0x80)
#define X25FAC_TYPE_D_MASK (0xC0)
#define X25FAC_TYPE_MASK   (0xC0)

#define X25FAC_EXT_ADDR_USE_MASK (0xC0)
#define X25FAC_EXT_ADDR_USE_SHIFT (6)

/*****************************************************************************/
/* Supported limits for selected facilities                                  */
/*****************************************************************************/
#define X25FAC_TCLS_MIN 0x06                      /* Throughput class        */
#define X25FAC_TCLS_MAX 0x0B
#define X25FAC_WSIZ_MIN 0x01                      /* Window size limits      */
#define X25FAC_WSIZ_MAX 0x07
#define X25FAC_PSIZ_MIN 0x04                      /* Packet size limits      */
#define X25FAC_PSIZ_MAX 0x0A

/*****************************************************************************/
/* Facilities identification codes - as defined in CCITT 1984 X.25           */
/*****************************************************************************/
#define X25FAC_FASTSEL_CODE    0x01               /* Also reverse charge     */
#define X25FAC_TCLS_CODE       0x02
#define X25FAC_CUG_BASIC_CODE  0x03
#define X25FAC_CI_REQUEST_CODE 0x04
#define X25FAC_CUG_OAB_CODE    0x09
#define X25FAC_CLAMN_CODE      0x08
#define X25FAC_MIN_TCLS_CODE   0x0A
#define X25FAC_EXP_DATA_CODE   0x0B

#define X25FAC_PSIZ_CODE       0x42
#define X25FAC_WSIZ_CODE       0x43
#define X25FAC_RPOA_CODE       0x44
#define X25FAC_CUG_EXT_CODE    0x47
#define X25FAC_CUG_OAE_CODE    0x48
#define X25FAC_BI_CUG_CODE     0x41
#define X25FAC_TRAN_DEL_CODE   0x49

#define X25FAC_NUI_DATA_CODE         0xC6
#define X25FAC_CI_MON_UNT_CODE       0xC5
#define X25FAC_CI_SEG_CNT_CODE       0xC2
#define X25FAC_CI_CALL_DUR_CODE      0xC1
#define X25FAC_RPOAE_CODE            0xC4
#define X25FAC_CALL_REDR_CODE        0xC3
#define X25FAC_CALLING_ADDR_EXT_CODE 0xCB
#define X25FAC_CALLED_ADDR_EXT_CODE  0xC9
#define X25FAC_END_TO_END_DEL_CODE   0xCA
#define X25FAC_PUNCTUATION_MARKER    0x00

/*****************************************************************************/
/* The next block of parameters are ORed together to create one byte         */
/*****************************************************************************/
#define X25FAC_FASTSEL_PARM     0x80              /* Fast select (either)    */
#define X25FAC_FASTSEL_RSP_PARM 0x40              /* Fastsel with restriction*/
#define X25FAC_REV_CHRG_PARM    0x01              /* Reverse charging        */
#define X25FAC_EXP_DATA_PARM    0x01              /* Expeditied data         */

#define X25FAC_CI_REQUESTED_PARM     0x1
#define X25FAC_CI_NOT_REQUESTED_PARM 0x0

/*****************************************************************************/
/* Internal macros for use within the jsfac.c routines                       */
/*****************************************************************************/
#define X25_FACILITIES         (1)
#define AWKWARD_X25_FACILITIES (2)
#define CCITT_DTE_FACILITIES   (3)
#define OTHER_FACILITIES       (4)

#define X25FAC_STATE_X25       (1)
#define X25FAC_STATE_INTRA     (2)
#define X25FAC_STATE_INTER     (3)
#define X25FAC_STATE_CCITT_DTE (4)
#define X25FLG_FAC_UNKNOWN     0x0

/*****************************************************************************/
/*  Function        convert_binary_to_bcd                                    */
/*                                                                           */
/*  Prototype       void convert_binary_to_bcd(                              */
/*                    u_char *to;                                            */
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
/*****************************************************************************/
/*  Function        convert_bcd_to_binary                                    */
/*                                                                           */
/*  Prototype       unsigned convert_bcd_to_binary(                          */
/*                    u_char *from;                                          */
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
/*****************************************************************************/
/*  Function        convert_bcd_to_string                                    */
/*                                                                           */
/*  Prototype       void convert_bcd_to_string(                              */
/*                    u_char   *from;                                        */
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
/*****************************************************************************/
/*  Function        convert_string_to_bcd                                    */
/*                                                                           */
/*  Prototype       unsigned convert_string_to_bcd(                          */
/*                    u_char *to;                                            */
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
/*****************************************************************************/
/*  Function        gime_more_memory                                         */
/*                                                                           */
/*  Prototype       u_char *gime_more_memory(                                */
/*                    u_char **overflow_ptr;                                 */
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
/*****************************************************************************/
/*  Function        _x25_convert_byte_stream_to_cb_fac                       */
/*                                                                           */
/*  Prototype       int _x25_byte_stream_to_cb_fac(                          */
/*                    cb_fac_t *cb_fac_ptr,                                  */
/*                    byte     *overflow_area,                               */
/*                    byte     *fac_stream,                                  */
/*                    unsigned  fac_stream_length)                           */
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
/* Start of declarations for jsfac.c                                         */
#ifdef _NO_PROTO
unsigned _x25_convert_cb_fac_to_byte_stream();
int _x25_convert_byte_stream_to_cb_fac();
#else
extern unsigned _x25_convert_cb_fac_to_byte_stream(
  unsigned char *fac_stream,
  cb_fac_t *cb_fac_ptr,
  x25_devinfo_t *iocinfo_ptr);

extern int _x25_convert_byte_stream_to_cb_fac(
  cb_fac_t *cb_fac_ptr,
  unsigned char *overflow_area,
  unsigned char *fac_stream,
  unsigned fac_length);

#endif /* _NO_PROTO */
/* End of declarations for jsfac.c                                           */
#endif
