/* @(#)18	1.2  src/bos/kernext/dlc/qlc/qlcextcb.h, sysxdlcq, bos411, 9428A410j 11/2/93 10:04:48 */
#ifndef _H_QLCEXTCB
#define _H_QLCEXTCB
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
/* This header file contains the protocol specific data structures for the   */
/* QLLC interface. It should be used in conjunction with the gdlextcb.h file */
/* which provides the generic DLC structures.                                */
/*****************************************************************************/
#define QLLC_MAX_SNA_FACS_LENGTH  (128)

/*****************************************************************************/
/* The parameter passed to the QUERY_SAP ioctl has a protocol specific data  */
/* area which consists of a cio_stats_t structure as defined in comio.h.     */
/*****************************************************************************/
struct qlc_query_sap_psd
{
  char   x25_data[sizeof(cio_stats_t)];
};

/*****************************************************************************/
/* The parameter passed to the START_LS ioctl has a protocol specific data   */
/* area which consists of the X.25 facilities, as well as the Support Level  */
/* and Listen Name to be used in conjunction with the link station being     */
/* started.                                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Before defining the Start structure, a facilities type is required.       */
/* The X.25 facilities requested when a Call is made (i.e. on Start) are     */
/* passed to QLLC in the structure defined by sna_facilities_type.           */
/*****************************************************************************/
struct sna_facilities_type
{
  unsigned facs: 1;          /* x.25 facilities:              0=no, 1=yes  */
  unsigned revc: 1;          /* reverse charging:             0=no, 1=yes  */
  unsigned rpoa: 1;          /* RPOA required:                0=no, 1=yes  */
  unsigned psiz: 1;          /* default packet size:          0=yes,1=no   */
  unsigned wsiz: 1;          /* default window size:          0=yes,1=no   */
  unsigned tcls: 1;          /* default throughput class:     0=yes,1=no   */
  unsigned cug : 1;          /* closed user group:            0=no, 1=yes  */
  unsigned cugo: 1;          /* closed user group/out. access 0=no, 1=yes  */
  unsigned cud : 1;          /* call user data                0=no, 1=yes  */
  unsigned res2: 1;          /* bit not used any more                      */
  unsigned nui : 1;          /* network user identification   0=no, 1=yes  */
  unsigned     :21;          /* end of bit field, next field word-aligned  */
  /***********************************************/
  /* the following encoding scheme is used for   */
  /* specifying packet sizes:                    */
  /*      0x06 = 64 octets                       */
  /*      0x07 = 128 octets                      */
  /*      0x08 = 256 octets                      */
  /*      0x09 = 512 octets                      */
  /*      0x0A = 1024 octets                     */
  /*      0x0B = 2048 octets                     */
  /*      0x0C = 4096 octets                     */
  /***********************************************/
  unsigned char  recipient_tx_psiz;    /* size of received packets           */
  unsigned char  originator_tx_psiz;   /* size of transmit packets           */

  unsigned char recipient_tx_wsiz;    /* window size for received packets    */
  unsigned char originator_tx_wsiz;   /* window size for transmit packets    */
  /***********************************************/
  /* the following encoding scheme is used for   */
  /* specifying throughput classes:              */
  /*      0x07 = 1200 bits/s.                    */
  /*      0x08 = 2400 bits/s.                    */
  /*      0x09 = 4800 bits/s.                    */
  /*      0x0A = 9600 bits/s.                    */
  /*      0x0B = 19200 bits/s.                   */
  /*      0x0C = 48000 bits/s.                   */
  /***********************************************/
  unsigned char recipient_tx_tcls;   /* throughput class for received data   */
  unsigned char originator_tx_tcls;   /* throughput class for transmit data  */

  unsigned short  reserved;  /* this field not used any more               */ 
  /**********************************************************/
  /* CUG index is four ASCII decimal digits specifying the  */
  /* closed user group selected for this call               */
  /* IDs less than four digits should have zeros            */
  /* prefixed to start                                      */
  /**********************************************************/
  unsigned short  cug_index;
  unsigned short  rpoa_id_count;               /* number of rpoa identifiers */
  unsigned short  rpoa_id[30];
  unsigned int    nui_length;                          /* no of bytes of nui */
  char            nui_data[109];
  unsigned int    cud_length;                          /* no of bytes of cud */
  char            cud_data[16];
};


/*****************************************************************************/
/* And the QLC_START_PSD protocol specific data area is defined as follows.  */
/*****************************************************************************/
struct qlc_start_psd
{
  char                        listen_name[8];
  unsigned short              support_level;
  struct sna_facilities_type  facilities;
};

/*****************************************************************************/
/* QLLC uses a control block definition for Start_LS which is a structure    */
/* that includes the generic start control block, from gdlextcb.h, and also  */
/* contains protocol specific fields which occupy the area left at the end   */
/* of the generic parameter block for such protocol dependencies.            */
/*****************************************************************************/
struct qlc_sls_arg
{
  struct dlc_sls_arg    dlc;   
  struct qlc_start_psd  psd;
};


#endif
