/* @(#)88  1.3  src/bos/kernext/dlc/qlc/qlcvfac.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:24:20 */
#ifndef _H_QLCVFAC
#define _H_QLCVFAC
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
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
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* SNA FACILITIES DEFINITIONS                                                */
/*****************************************************************************/

/*****************************************************************************/
/* x25_pkt_size_type is supposed to enumerate the valid packet sizes, i.e.   */
/* 16, 32, 64, 128, 256, 512, 1024. However, this doesn't seem to be possible*/
/* in C, so we can only fake it by defining x25_pkt_size_type as unsigned int*/
/*****************************************************************************/
typedef unsigned int x25_pkt_size_type;

/*****************************************************************************/
/* CCITT_TO_X25_PSIZE_MAP converts a valid packet size code to the pkt size  */
/* it represents. The map is:                                                */
/* {0x04->16, 0x05->32, 0x06->64, 0x07->128 0x08->256, 0x09->512, 0x0A->1024}*/
/* If the parameter is not in the domain of the map, zero is returned.       */
/*****************************************************************************/
#define CCITT_TO_X25_PSIZE_MAP(p) ((p)>0x03 && (p)<0x0B) ? 0x01<<(p) : 0

/*****************************************************************************/
/* sna_facilities_type describes the protocol dependent part of the          */
/* start link station configuration area. It is imbedded within the          */
/* link station config area which is passed by a user to QLLC.               */
/*****************************************************************************/
/* moved to qlcextcb.h on 8th November 1989 */

/******************************************************************************
** SNA FACILITIES FUNCTION HEADERS
******************************************************************************/
/*
x25_pkt_size_type sna_fac_originator_tx_pkt_size();
x25_facilities_type *sna_fac_x25_format();
*/
/******************************************************************************
** X.25 FACILITIES DEFINITIONS
******************************************************************************/

/******************************************************************************
** In the SEDL design, x25_facilities_type was defined as a set of
** x25_facilities_element_types. This doesn't fit neatly into the C code.
** Ideally, x25_facilities_type would be represented as a structure containing
** a length field and an array of 63 bytes to hold
** the encoded facilities (the maximum length of facilities supported by 1980
** X.25 is 63 bytes). Like this:
**
**     typedef struct {
**         byte length;               /- the length of facilities info       -/
**         byte facils[MAX_FAC_LEN];  /- the encoded facilities array        -/
**     }   x25_facilities_type;
**
** However, due to the RT's propensity to word-align all structures, we have to
** use a byte array. The first byte of the array holds the facilities length.
** The second byte is a spacer to skip the user data length field in the X.25
** Call Buffer returned by VRM DD. Encoded facilities start at the 3rd byte of
** the array (i.e. offset 2).
******************************************************************************/

/******************************************************************************
** The maximum length of facilities which is supported by the 1980 X.25
** recommendation is 63 bytes
******************************************************************************/
#define MAX_FAC_LEN (63)

/******************************************************************************
** The parameter code for a reverse charge request (which is constant) is also
** #define'd
******************************************************************************/
#define REVC_PARM 0x01

/******************************************************************************
** An enumerated type is defined to represent the facilities codes which are
** supported by QLLC, or may be encountered by QLLC. The National Options Mark
** (0x00) is only used as a delimiter, as we know that all facilities following
** this mark are not supported. The facilities class codes are included as an
** aid to parsing incoming X.25 facilities requests. They will determine how
** many bytes of facilities parameters should follow the facility code.
******************************************************************************/
enum x25_facilities_code_type {
    /* national options mark (unsupported) */
    fc_nat_opts = 0x00,
    /* supported facility codes */
    fc_revc     = 0x01,
    fc_tcls     = 0x02,
    fc_cug      = 0x03,
    fc_psiz     = 0x42,
    fc_wsiz     = 0x43,
    fc_rpoa     = 0x44,
    /* facility code class coding */
    fc_class_a  = 0x00,
    fc_class_b  = 0x40,
    fc_class_c  = 0x80,
    fc_class_d  = 0xC0
};

/******************************************************************************
** Define constants for the offset from start of the facilities type to the
** length field, and to the start of real encoded facilities.
**
** x25_facilities_type used to be defined as as a byte array big enough to
** hold the maximum facilities length plus two bytes header. However we never
** need to allocate storage for facilities. All we ever use is a pointer. So
** to make pointer arithmetic much more straightforward, x25_facilities_type
** is typedef'd to unsigned char.
******************************************************************************/
#define FAC_LENGTH (0)
#define FAC_START (2)
typedef byte x25_facilities_type;

/******************************************************************************
** X.25 FACILITIES CLEAR
** Function:    X25_FAC_CLEAR
**
** Description: Completely empties an X.25 facilities request. Used to clear
**              facilities on an incoming call before accepting that call.
**
** Parameter:   f               - a pointer to the X.25 facilities requests
**
** Returns:     no return value
**
******************************************************************************/
#define X25_FAC_CLEAR(f) (f)[FAC_LENGTH] = 0


/******************************************************************************
** X.25 FACILITIES FUNCTION HEADERS
******************************************************************************/
/* Start of declarations for qlcvfac.c                                       */
#ifdef _NO_PROTO

/* bool x25_fac_non_default_pkt_size();        */
/* x25_pkt_size_type x25_fac_get_orig_tx_pkt_size();   */
/* x25_pkt_size_type x25_fac_get_recip_tx_pkt_size();   */
/* x25_facilities_type *x25_fac_alter_pkt_size();         */

void  qvm_convert_sna_facs_to_cb_fac();

#else

extern void  qvm_convert_sna_facs_to_cb_fac(
  struct sna_facilities_type  *sna_facs,
  cb_fac_t *cb_facs);

/* End of declarations for qlcvfac.c                                         */
#endif   /* _NO_PROTO */

#endif

