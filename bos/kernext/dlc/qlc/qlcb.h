/* @(#)49	1.6  src/bos/kernext/dlc/qlc/qlcb.h, sysxdlcq, bos411, 9428A410j 11/3/93 16:25:49 */
#ifndef _H_QLCB
#define _H_QLCB
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

#define NORM_DATA (0x80000000)
#define XIDD_DATA (0x40000000)
#define NETD_DATA (0x10000000)

#include <x25/jsxbuf.h> 

/*****************************************************************************/
/* and typedef the mbuf_t to the qllc general buffer type                    */
/*****************************************************************************/
typedef mbuf_t gen_buffer_type;

struct read_ext_type
{
  unsigned int user_sap_correlator;
  unsigned int user_ls_correlator;
  unsigned int dlc_flags;
  /***************************************************************************/
  /*           dlc_flags has the following structure.....                    */
  /*                                                                         */
  /*  <---------------------------------32 bits-------------------------->   */
  /*  n  x  r  n                                                      o  r   */
  /*  o  i  e  e   .................................................. f  s   */
  /*  r  d  s  t                                                      l  p   */
  /*  m  d  v  d                                                      o  p   */
  /*                                                                         */
  /* Values for these flags are defined in this qlcb.h file.                 */
  /*                                                                         */
  /***************************************************************************/
  unsigned int dlh_length;      /* not used for Write                        */
};
typedef struct read_ext_type read_ext_type;

/*****************************************************************************/
/* The write data offset is returned at Station Started, and advises the     */
/* kernel user where the data written into an mbuf may start, ensuring that  */
/* qllc address and control fields, and packet data areas aren't overwritten.*/
/*****************************************************************************/
/*
#define WRITE_DATA_OFFSET \
  (sizeof(x25_packet_data_t) + \
  + OFFSETOF(user_data[0], qllc_body1_t) \
  )
*/
#define WRITE_DATA_OFFSET (sizeof(x25_packet_data_t))
/*****************************************************************************/
/* I don't think you need worry about the a and c fields. They are not       */
/* relevant any more for normal data, and you only need to skip over the     */
/* packet data.                                                              */
/*****************************************************************************/

#define MIN_BUF_SIZE  (X25_OFFSETOF_USER_DATA)
	                     /* This is a min buffer size that is used when  */
	                     /* getting a buffer which will be used as a     */
	                     /* call request, or other packet. The eventual  */
	                     /* buffer size will be enlarged using jsmbuf_   */
	                     /* guarantee_size when you know which facs/cud  */
	                     /* are to be included.                          */

/*****************************************************************************/
/* The following macros are concerned with the queueing and management of    */
/* buffers.                                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Macro:       QBM_GET_BUFFER                                               */
/* Description:                                                              */
/*   This macro gets a buffer and returns the buffer's address.              */
/*   NOTE: This used to call JSMBUF_MALLOC which in turn called m_get with   */
/*         the M_DONTWAIT option. This was not consistent with the other     */
/*         DLCs so I added a macro (JSMBUF_WMALLOC) that will use the        */
/*         M_WAIT option of m_get                                            */
/*                                                                           */
/* Type:        gen_buffer_type *                                            */
/* Parameters:  unsigned int length  (# bytes)                               */
/*****************************************************************************/
#define QBM_GET_BUFFER(length) \
  ((gen_buffer_type *)(JSMBUF_WMALLOC((unsigned)(length))))


/*****************************************************************************/
/* Macro:       QBM_FREE_BUFFER                                              */
/* Description:                                                              */
/*   This macro frees a buffer.                                              */
/*                                                                           */
/* Type:        none                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_FREE_BUFFER(buf_ptr) \
  JSMBUF_FREE(buf_ptr)

/*****************************************************************************/
/* Macro:      QBM_ENQUE_BUFFER                                              */
/* Description:                                                              */
/*   This macro queues the buffer on the end of a queue. A queue is          */
/*   implemented by a chain of mbufs.                                        */
/*   Mbufs are chained together by their m_act field.                        */
/*                                                                           */
/* Type:    none                                                             */
/* Parameters: gen_buffer_type *buf_ptr                                      */
/*             gen_buffer_type *queue_ptr                                    */
/*****************************************************************************/
#define QBM_ENQUE_BUFFER(buf_ptr,queue_ptr) \
   jsmlist_enq( \
     (jsmlist_t *)queue_ptr, \
     buf_ptr \
     )

/*****************************************************************************/
/* Macro:      QBM_DEQUE_BUFFER                                              */
/* Description:                                                              */
/*   This macro dequeues the buffer from the front of a queue. A queue is    */
/*   implemented by a chain of mbufs.                                        */
/*   Mbufs are chained together by their m_act field.                        */
/*                                                                           */
/* Type:       gen_buffer_type *                                             */
/* Parameters: gen_buffer_type *queue_ptr                                    */
/*****************************************************************************/
#define QBM_DEQUE_BUFFER(queue_ptr) \
    (gen_buffer_type *)jsmlist_deq((jsmlist_t *)queue_ptr)

/*****************************************************************************/
/* Macro:      QBM_REQUE_BUFFER                                              */
/* Description:                                                              */
/*   This macro requeues the buffer on the front of a queue. A queue is      */
/*   implemented by a chain of mbufs.                                        */
/*   Mbufs are chained together by their m_act field.                        */
/*                                                                           */
/* Type:       none                                                          */
/* Parameters: gen_buffer_type *queue_ptr                                    */
/*             gen_buffer_type *buf_ptr                                      */
/*****************************************************************************/
#define QBM_REQUE_BUFFER(buf_ptr,queue_ptr) \
    jsmlist_add_to_front((jsmlist_t *)queue_ptr,buf_ptr)

/*****************************************************************************/
/* QBM_IOMOVEIN                                                              */
/*****************************************************************************/
#define QBM_IOMOVEIN(buf_ptr,offset,uiop,resid) \
  JSMBUF_IOMOVEIN(buf_ptr,offset,uiop,resid)

/*****************************************************************************/
/* QBM_MBUF_TO_UIO_MOVE                                                      */
/* This macro calls the relevant JSMBUF macro.                               */
/* Function is to iomove data from an mbuf to a                              */
/* uio structure in user space.                                              */
/* Inputs:  buffer address                                                   */
/*          uio structure address                                            */
/* Outputs: none                                                             */
/* Return:  none                                                             */
/*****************************************************************************/
#define QBM_MBUF_TO_UIO_MOVE(buffer_ptr,uio_ptr) \
    JSMBUF_TO_UIO_MOVE(buffer_ptr,uio_ptr)

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/* The following macros are concerned with buffer contents.                  */
/* ------------------------------------------------------------------------- */
/*****************************************************************************/
/*****************************************************************************/
/* QLLC HEADER CONTENTS                                                      */
/*****************************************************************************/
#define QBM_SET_USER_SAP_CORRELATOR(buf_ptr,value) \
  JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.user_sap_correlator,x25_mbuf_t),\
    value \
    )

#define QBM_RETURN_USER_SAP_CORRELATOR(buf_ptr) \
  JSMBUF_READ_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.user_sap_correlator,x25_mbuf_t),\
    correlator_type \
    )

#define QBM_SET_USER_LS_CORRELATOR(buf_ptr,value) \
  JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.user_ls_correlator,x25_mbuf_t),\
    value \
    )

#define QBM_RETURN_USER_LS_CORRELATOR(buf_ptr) \
  JSMBUF_READ_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.user_ls_correlator,x25_mbuf_t),\
    correlator_type \
    )

/*****************************************************************************/
/* To read whether a dlc flag is set or not, use the following macro with    */
/* one of the flags defined                                                  */
/*****************************************************************************/
#define NORM (0x80000000)
#define XIDD (0x40000000)
#define NETD (0x10000000)
#define OFLO (0x00000002)
#define RSPP (0x00000001)

#define QBM_RETURN_DLC_FLAGS(buf_ptr) \
  (JSMBUF_READ_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.dlc_flags,x25_mbuf_t), \
    int \
    ) \
    )

#define QBM_SET_DLC_FLAGS(buf_ptr,value) \
  (JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.dlc_flags,x25_mbuf_t), \
    value \
    ) \
    )

#define QBM_SET_DLH_LENGTH(buf_ptr,value) \
  (JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.dlh_length,x25_mbuf_t), \
    value \
    ) \
    )

#define QBM_RETURN_DLH_LENGTH(buf_ptr) \
  JSMBUF_READ_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.dlh_length,x25_mbuf_t), \
    int \
    )

#define QBM_RETURN_NETID(buf_ptr) \
  JSMBUF_READ_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.netid,x25_mbuf_t), \
    unsigned short \
    )

#define QBM_RETURN_SESSION_ID(buf_ptr) \
  JSMBUF_READ_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.session_id,x25_mbuf_t), \
    unsigned short \
    )

#define QBM_RETURN_CALL_ID(buf_ptr) \
  JSMBUF_READ_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.call_id,x25_mbuf_t), \
    unsigned short \
    )

#define QBM_SET_NETID(buf_ptr,value) \
  JSMBUF_SET_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.netid,x25_mbuf_t), \
    value \
    )

#define QBM_SET_SESSION_ID(buf_ptr,value) \
  JSMBUF_SET_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.session_id,x25_mbuf_t), \
    value \
    )

#define QBM_SET_CALL_ID(buf_ptr,value) \
  JSMBUF_SET_HALF_WORD( \
    buf_ptr, \
    OFFSETOF(header.qllc_header.call_id,x25_mbuf_t), \
    value \
    )

/*****************************************************************************/
/* PACKET DATA CONTENTS                                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Macro:    QBM_RETURN_PACKET_TYPE                                          */
/* Description:                                                              */
/*   This macro returns the type of packet contained by the buffer.          */
/*   Returns byte set to one of #defined values in jsxbuf.h for various      */
/*   packet types e.g. PKT_CALL_REQ, PKT_CALL_ACCEPT, etc.                   */
/*      i.e. accesses buf_ptr->pd.packet_type                                */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type  *buf_ptr;                                   */
/*****************************************************************************/
#define QBM_RETURN_PACKET_TYPE(buf_ptr) \
  X25_READ_PACKET_TYPE(buf_ptr)

/*****************************************************************************/
/* Macro:    QBM_SET_PACKET_TYPE                                             */
/* Description:                                                              */
/*   This macro sets the type of packet contained by the buffer.             */
/*   Accepts byte set to one of #defined values in jsxbuf.h for various      */
/*   packet types e.g. PKT_CALL_REQ, PKT_CALL_ACCEPT, etc.                   */
/*      i.e. accesses buf_ptr->pd.packet_type                                */
/*                                                                           */
/* Type:        void                                                         */
/* Parameters:  gen_buffer_type  *buf_ptr;                                   */
/*              byte value;                                                  */
/*****************************************************************************/
#define QBM_SET_PACKET_TYPE(buf_ptr,value) \
  X25_SET_PACKET_TYPE(buf_ptr,value)

/*****************************************************************************/
/* Macro:    QBM_SET_FLAGS                                                   */
/* Description:                                                              */
/*      This macro sets the flags field in the packet data structure in the  */
/*      buffer.                                                              */
/*      i.e. it sets                                                         */
/*      (buf_ptr)->pd.flags  to value                                        */
/*                                                                           */
/* Type:       bool                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             byte value;                                                   */
/*****************************************************************************/
#define QBM_SET_FLAGS(buf_ptr,value) \
  X25_SET_FLAGS(buf_ptr,value)

/*****************************************************************************/
/* Macro:    QBM_RETURN_Q_BIT                                                */
/* Description:                                                              */
/*      This macro returns the setting of the Q-bit in the buffer.           */
/*      i.e. it returns                                                      */
/*      (buf_ptr)->pd.flags & X25_Q_BIT                                      */
/*                                                                           */
/* Type:       bool                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_Q_BIT(buf_ptr) \
  (X25_READ_FLAGS(buf_ptr) & X25_Q_BIT)

/*****************************************************************************/
/* Macro:    QBM_SET_Q_BIT                                                   */
/* Description:                                                              */
/*      This macro sets the Q-bit in the buffer.                             */
/*      i.e. it accomplishes                                                 */
/*      (buf_ptr)->pd.flags = (buf_ptr)->pd.flags | X25_Q_BIT                */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_SET_Q_BIT(buf_ptr) \
  X25_SET_FLAGS(buf_ptr,(X25_READ_FLAGS(buf_ptr) | X25_Q_BIT))

/*****************************************************************************/
/* Macro:    QBM_CLEAR_Q_BIT                                                 */
/* Description:                                                              */
/*      This macro clears the Q-bit in the buffer.                           */
/*      i.e. it accomplishes                                                 */
/*      (buf_ptr)->pd.flags = (buf_ptr)->pd.flags & ~X25_Q_BIT               */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_CLEAR_Q_BIT(buf_ptr) \
  X25_SET_FLAGS(buf_ptr,(X25_READ_FLAGS(buf_ptr) & ~X25_Q_BIT))

/*****************************************************************************/
/* Macro:    QBM_RETURN_D_BIT                                                */
/* Description:                                                              */
/*      This macro returns the setting of the D-bit in a buffer.             */
/*      i.e. it returns                                                      */
/*      ((buf_ptr)->pd.flags) & X25_D_BIT                                    */
/*                                                                           */
/* Type:       bool                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_D_BIT(buf_ptr) \
  (bool)(X25_READ_FLAGS(buf_ptr) & X25_D_BIT)

/*****************************************************************************/
/* Macro:    QBM_RETURN_CAUSE                                                */
/* Description:                                                              */
/*      This macro returns the cause code in the buffer.                     */
/*      i.e. it returns                                                      */
/*      ((buf_ptr)->pd.cause)                                                */
/*                                                                           */
/* Type:       byte                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_CAUSE(buf_ptr) \
  (byte)(X25_READ_CAUSE(buf_ptr))

/*****************************************************************************/
/* Macro:    QBM_SET_CAUSE                                                   */
/* Description:                                                              */
/*      This macro sets the cause code in the buffer.                        */
/*      i.e. it sets                                                         */
/*      ((buf_ptr)->pd.cause)                                                */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             byte value;                                                   */
/*****************************************************************************/
#define QBM_SET_CAUSE(buf_ptr,value) \
  (X25_SET_CAUSE(buf_ptr,value))

/*****************************************************************************/
/* Macro:    QBM_RETURN_DIAGNOSTIC                                           */
/* Description:                                                              */
/*      This macro returns the diagnostic in the buffer.                     */
/*      i.e. it returns                                                      */
/*      ((buf_ptr)->pd.diagnostic)                                           */
/*                                                                           */
/* Type:       byte                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_DIAGNOSTIC(buf_ptr) \
  (byte)(X25_READ_DIAGNOSTIC(buf_ptr))

/*****************************************************************************/
/* Macro:    QBM_SET_DIAGNOSTIC                                              */
/* Description:                                                              */
/*      This macro sets the diagnostic in the buffer.                        */
/*      i.e. it sets                                                         */
/*      ((buf_ptr)->pd.diagnostic)                                           */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             byte value;                                                   */
/*****************************************************************************/
#define QBM_SET_DIAGNOSTIC(buf_ptr,value) \
  (X25_SET_DIAGNOSTIC(buf_ptr,value))

/*****************************************************************************/
/* CALL DATA CONTENTS                                                        */
/*****************************************************************************/
/*****************************************************************************/
/* Macro:    QBM_RETURN_CALLED_ADDRESS                                       */
/* Description:                                                              */
/*      This macro gets the called address out                               */
/*      of a buffer with a call_data structure in it                         */
/*       i.e. accesses (buffer_ptr)->body.cd.called_address                  */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             char *called_address                                          */
/*****************************************************************************/
#define QBM_RETURN_CALLED_ADDRESS(buf_ptr,called_address) \
  X25_READ_CALLED_ADDRESS(buf_ptr,called_address)

/*****************************************************************************/
/* Macro:    QBM_SET_CALLED_ADDRESS                                          */
/* Description:                                                              */
/*      This macro sets the called address in                                */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.called_address                  */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             char *called_address                                          */
/*****************************************************************************/
#define QBM_SET_CALLED_ADDRESS(buf_ptr,called_address) \
  X25_SET_CALLED_ADDRESS(buf_ptr,called_address)

/*****************************************************************************/
/* Macro:    QBM_RETURN_CALLING_ADDRESS                                      */
/* Description:                                                              */
/*      This macro gets the calling address out                              */
/*      of a buffer with a call_data structure in it                         */
/*       i.e. accesses (buffer_ptr)->body.cd.calling_address                 */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             char *calling_address                                         */
/*****************************************************************************/
#define QBM_RETURN_CALLING_ADDRESS(buf_ptr,calling_address) \
  X25_READ_CALLING_ADDRESS(buf_ptr,calling_address)

/*****************************************************************************/
/* Macro:    QBM_SET_CALLING_ADDRESS                                         */
/* Description:                                                              */
/*      This macro sets the calling address in                               */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.calling_address                 */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             char *calling_address                                         */
/*****************************************************************************/
#define QBM_SET_CALLING_ADDRESS(buf_ptr,calling_address) \
  X25_SET_CALLING_ADDRESS(buf_ptr,calling_address)

/*****************************************************************************/
/* Macro:    QBM_RETURN_FACILITIES_LENGTH                                    */
/* Description:                                                              */
/*      This macro gets the facilities length out                            */
/*      of a buffer with a call_data structure in it                         */
/*       i.e. accesses (buffer_ptr)->body.cd.optional_data                   */
/*                                                                           */
/* Type:       unsigned short                                                */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_FACILITIES_LENGTH(buf_ptr) \
  X25_READ_FACILITIES_LENGTH(buf_ptr)

/*****************************************************************************/
/* Macro:    QBM_SET_FACILITIES_LENGTH                                       */
/* Description:                                                              */
/*      This macro sets the facilities length in                             */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.optional_data                   */
/*                                                                           */
/* Type:       unsigned short                                                */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             byte length;                                                  */
/*****************************************************************************/
#define QBM_SET_FACILITIES_LENGTH(buf_ptr,length) \
  X25_SET_FACILITIES_LENGTH(buf_ptr,length)

/*****************************************************************************/
/* Macro:    QBM_RETURN_CUD                                                  */
/* Description:                                                              */
/*      This macro returns the call user data in                             */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.optional_data but allows for    */
/*                                 length of facilities....                  */
/*                                                                           */
/* Type:       byte       -  relies on fact that QLLC CUD is always 1 byte   */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_CUD(buf_ptr) \
  JSMBUF_READ_BYTE(buf_ptr, \
    X25_OFFSETOF_FAC_CUD_DATA + X25_READ_FACILITIES_LENGTH(buf_ptr), \
    byte \
    )

/*****************************************************************************/
/* Macro:    QBM_RETURN_CUD1                                                 */
/* Description:                                                              */
/*      This macro returns specified byte of call user data                  */
/*      Defect 110313                                                        */
/*                                                                           */
/* Type:       byte       -                                                  */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_CUD1(buf_ptr,ii) \
  JSMBUF_READ_BYTE(buf_ptr, \
    X25_OFFSETOF_FAC_CUD_DATA + X25_READ_FACILITIES_LENGTH(buf_ptr) +ii, \
    byte \
    )

/*****************************************************************************/
/* Macro:    QBM_RETURN_CUD_LENGTH                                           */
/* Description:                                                              */
/*      This macro returns the length of call user data in                   */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.call_user_data_length           */
/*                                                                           */
/* Type:       ushort     -  relies on fact that QLLC CUD is always 1 byte   */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*****************************************************************************/
#define QBM_RETURN_CUD_LENGTH(buf_ptr) \
  X25_READ_CUD_LENGTH(buf_ptr)

/*****************************************************************************/
/* Macro:    QBM_SET_CUD_LENGTH                                              */
/* Description:                                                              */
/*      This macro sets the length of call user data in                      */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.call_user_data_length           */
/*                                                                           */
/* Type:       void                                                          */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             ushort value;                                                 */
/*****************************************************************************/
#define QBM_SET_CUD_LENGTH(buf_ptr,value) \
  X25_SET_CUD_LENGTH(buf_ptr,value)

/*****************************************************************************/
/* Macro:    QBM_SET_CUD                                                     */
/* Description:                                                              */
/*      This macro sets the call user data in                                */
/*      a buffer with a call_data structure in it                            */
/*       i.e. accesses (buffer_ptr)->body.cd.optional_data but allows for    */
/*                                 length of facilities....                  */
/*                                                                           */
/* Type:       void       -  relies on fact that QLLC CUD is always 1 byte   */
/* Parameters: gen_buffer_type *buf_ptr;                                     */
/*             byte cud;                                                     */
/*****************************************************************************/
#define QBM_SET_CUD(buf_ptr,cud) \
  JSMBUF_SET_BYTE(buf_ptr, \
    X25_OFFSETOF_FAC_CUD_DATA + X25_READ_FACILITIES_LENGTH(buf_ptr), \
    cud \
    )

/*****************************************************************************/
/* BUFFER BODY CONTENTS                                                      */
/*****************************************************************************/
/*****************************************************************************/
/* Macro:       QBM_RETURN_QLLC_ADDRESS_FIELD()                              */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_body.address_field                                    */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_RETURN_QLLC_ADDRESS_FIELD(buf_ptr) \
  JSMBUF_READ_BYTE( \
    buf_ptr, \
    OFFSETOF(body.qllc_body.address_field, x25_mbuf_t), \
    byte \
    )

/*****************************************************************************/
/* Macro:       QBM_SET_QLLC_ADDRESS_FIELD()                                 */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_body.address_field                                    */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_SET_QLLC_ADDRESS_FIELD(buf_ptr,value) \
  JSMBUF_SET_BYTE( \
    buf_ptr, \
    OFFSETOF(body.qllc_body.address_field, x25_mbuf_t), \
    value \
    )

/*****************************************************************************/
/* Macro:       QBM_RETURN_QLLC_CONTROL_FIELD()                              */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_body.control_field                                    */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_RETURN_QLLC_CONTROL_FIELD(buf_ptr) \
  JSMBUF_READ_BYTE( \
    buf_ptr, \
    OFFSETOF(body.qllc_body.control_field, x25_mbuf_t), \
    byte \
    )

/*****************************************************************************/
/* Macro:       QBM_SET_QLLC_CONTROL_FIELD()                                 */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_body.control_field                                    */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_SET_QLLC_CONTROL_FIELD(buf_ptr,value) \
  JSMBUF_SET_BYTE( \
    buf_ptr, \
    OFFSETOF(body.qllc_body.control_field, x25_mbuf_t), \
    value \
    )

/*****************************************************************************/
/* MACROS TO SET FIELDS IN NETD BUFFERS                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Macro:       QBM_SET_NETD_FLAGS_FIELD()                                   */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_netd.netd_type_flags                                  */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_SET_NETD_FLAGS_FIELD(buf_ptr,value) \
  JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(body.qllc_netd.netd_type_flags, x25_mbuf_t), \
    value \
    )
/*****************************************************************************/
/* Macro:       QBM_SET_NETD_DATA_LENGTH()                                   */
/* Description:                                                              */
/*                                                                           */
/* Uses OFFSETOF macro in jsdefs.h. This gives you the offset of a member    */
/* into a struct of given type, so the type becomes gen_buffer_type, and the */
/* member is body.qllc_netd.netd_type_flags                                  */
/*                                                                           */
/* Type:        byte                                                         */
/* Parameters:  gen_buffer_type *buf_ptr                                     */
/*****************************************************************************/
#define QBM_SET_NETD_DATA_LENGTH(buf_ptr,value) \
  JSMBUF_SET_WORD( \
    buf_ptr, \
    OFFSETOF(body.qllc_netd.data_length, x25_mbuf_t), \
    value \
    )

/*****************************************************************************/
/* THERE IS AN INCONSISTENCY HERE IN THE WAY QLLC TREATS DATA AREAS IN       */
/* BUFFERS. IT CAN READ BLOCKS, AND SET BYTES...MAY NEED TO BE MADE MORE     */
/* RATIONAL.                                                                 */
/*****************************************************************************/
/*****************************************************************************/
/* i.e. return data area macro should be read byte and read block pair       */
/*****************************************************************************/

/*****************************************************************************/
/* Macro:    QBM_RETURN_DATA_AREA                                            */
/*   This macro returns the data area from a buffer.                         */
/*   The data area used by QLLC is always the qllc_body structure given in   */
/*   jsxbuf.h. The write_data_offset returned on Station Started result      */
/*   ensures that the kernel user never overwrites the address and control   */
/*   fields, or the reserved area before the qllc_body, which is used for    */
/*   packet_data.                                                            */
/*   It does not allow for m_buf header info as this is already taken into   */
/*   account by the kernel user.                                             */
/*                                                                           */
/*   The data area is copied from the buffer to the empty area whose address */
/*   is passed as parameter.                                                 */
/*****************************************************************************/
#define QBM_RETURN_BLOCK(buf_ptr,offset,data_area,length) \
  JSMBUF_READ_BLOCK( \
    buf_ptr, \
    offset, \
    data_area, \
    length \
    )



/*****************************************************************************/
/* Macro:    QBM_SET_BYTE                                                    */
/*   This macro sets one byte of data in a buffer.                           */
/*   The data area used by QLLC is always the qllc_body structure given in   */
/*   jsxbuf.h, unless the buffer is in call format.                          */
/*   The write_data_offset returned on Station Started result                */
/*   ensures that the kernel user never overwrites the address and control   */
/*   fields, or the reserved area before the qllc_body, which is used for    */
/*   packet_data.                                                            */
/*   It does not allow for m_buf header info as this is already taken into   */
/*   account by the kernel user.                                             */
/*                                                                           */
/*   The data area is copied from the buffer to the empty area whose address */
/*   address is passed as parameter.                                         */
/*****************************************************************************/
#define QBM_SET_BYTE(buf_ptr,offset,value) \
  JSMBUF_SET_BYTE( \
    buf_ptr, \
    offset, \
    value \
    )

/*****************************************************************************/
/* Macro:    QBM_SET_BLOCK                                                   */
/*   This macro sets a block of data in a buffer.                            */
/*   The data area used by QLLC is always the qllc_body structure given in   */
/*   jsxbuf.h. The write_data_offset returned on Station Started result      */
/*   ensures that the kernel user never overwrites the address and control   */
/*   fields, or the reserved area before the qllc_body, which is used for    */
/*   packet_data.                                                            */
/*   It does not allow for m_buf header info as this is already taken into   */
/*   account by the kernel user.                                             */
/*                                                                           */
/*   The data area is copied from the buffer to the empty area whose address */
/*   address is passed as parameter.                                         */
/*****************************************************************************/
#define QBM_SET_BLOCK(buf_ptr,offset,from,len) \
  JSMBUF_SET_BLOCK( \
    buf_ptr, \
    offset, \
    from, \
    len \
    )

/*****************************************************************************/
/*  FUNCTION HEADERS                                                         */
/*****************************************************************************/

/* Start of declarations for qlcb.c                                          */
#ifdef _NO_PROTO

/*****************************************************************************/
/* Function     qbm_return_x25_called_address                                */
/*                                                                           */
/* Description  This procedure gets the called address out of a buffer       */
/*              and fills in a structure of type x25_address_type            */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*              In filling in the x25_address_type structure it computes     */
/*              the length of the address.                                   */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty x25_address_type structure.                 */
/*                                                                           */
/*****************************************************************************/
void qbm_return_x25_called_address();

/*****************************************************************************/
/* Function     qbm_return_x25_calling_address                               */
/*                                                                           */
/* Description  This procedure gets the calling address out of a buffer      */
/*              and fills in a structure of type x25_address_type            */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*              In filling in the x25_address_type structure it computes     */
/*              the length of the address.                                   */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty x25_address_type structure.                 */
/*                                                                           */
/*****************************************************************************/
void qbm_return_x25_calling_address();

/*****************************************************************************/
/* Function     qbm_return_qllu                                              */
/*                                                                           */
/* Description  This procedure gets the qllu out of a buffer                 */
/*              and fills in a structure of type qllc_qllu_type              */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty qllc_qllu_type structure.                   */
/*                                                                           */
/*****************************************************************************/
void qbm_return_qllu();

/*****************************************************************************/
/* Function     qbm_return_cud                                               */
/*                                                                           */
/* Description  This procedure gets the cud out of a buffer                  */
/*              and fills in a structure of type x25_cud_type                */
/*              which is provided by the caller (address passed as           */
/*              input parameter).                                            */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters   address of buffer                                            */
/*              address of empty x25_cud_type structure.                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void qbm_return_cud();

#else
extern void qbm_return_x25_called_address(
  gen_buffer_type *buffer_ptr,
  char            *x25_address);

extern void qbm_return_x25_calling_address(
  gen_buffer_type *buffer_ptr,
  char            *x25_address);

extern void qbm_return_qllu(
  gen_buffer_type *buffer_ptr,
  struct qllc_qllu_type *qllu_ptr);

extern void qbm_return_cud(
  gen_buffer_type *buffer_ptr,
  x25_cud_type *cud_ptr);

#endif /* _NO_PROTO */
/* End of declarations for qlcb.c                                            */

#endif


