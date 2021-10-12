/* @(#)89  1.5  src/bos/kernext/x25/jsxbuf.h, sysxx25, bos411, 9428A410j 1/12/94 13:06:49 */
#ifndef _H_JSXBUF
#define _H_JSXBUF
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

/*****************************************************************************/
/*  Discussion            X.25      Buffer Formats                           */
/*                                                                           */
/*                              Buffer Overview                              */
/*                              ---------------                              */
/*                                                                           */
/*                                                                           */
/* Mbufs provide a scheme for buffer management that is flexible and fast.   */
/* For the definition of the structure of mbufs, see the header file         */
/* <sys/mbuf.h>.  Mbufs come in different sizes with space for a limited     */
/* amount of data storage.  Pointers inside the mbuf headers point to the    */
/* start of the data fields.                                                 */
/*                                                                           */
/* Mbufs may be connected together using the link pointers inside the mbuf   */
/* headers.  The data areas in these connected mbufs should be considered as */
/* being the same data area.  Data may be split anywhere to be continued in  */
/* the next mbuf in the chain.                                               */
/*                                                                           */
/* Thus an Mbuf can split the data section anywhere it wants to.  No         */
/* guarantees can be made that any section of the data area is not split     */
/* across an Mbuf boundary.  For example, if an application called strcpy    */
/* on the address of a string that is split across multiple mbufs, the       */
/* strcpy would work on data contained within a single mbuf, but would fail  */
/* on data that is split across multiple mbufs.                              */
/*                                                                           */
/* To solve this a series of macros have been written to access the          */
/* contents of an Mbuf sequence.                                             */
/*                                                                           */
/*                               Data Contents                               */
/*                               -------------                               */
/*                                                                           */
/*                                                                           */
/* The following section defines the contents of the data section of the     */
/* Mbufs.  It ignores where the Mbuf sequence decides to split the buffers.  */
/*                                                                           */
/* Mbufs are shared between Top, QLLC and Gp.  In order to avoid copying     */
/* and reformatting of data, we need to adopt a common format of buffer.     */
/*                                                                           */
/* The buffer format can be split into three parts                           */
/*                                                                           */
/*    1) A component dependent header                                        */
/*                                                                           */
/*    2) A fixed section                                                     */
/*                                                                           */
/*    3) A variable section, whose format depends on the various selectors   */
/*    in the fixed section.                                                  */
/*                                                                           */
/*                                                                           */
/*                        Component dependent header                         */
/*                        ~~~~~~~~~~~~~~~~~~~~~~~~~~                         */
/*                                                                           */
/*                                                                           */
/* Gp                                                                        */
/* ++                                                                        */
/*                                                                           */
/* Gp's header is as follows                                                 */
/*                                                                           */
/*                                 Gp header                                 */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³             Stage             ³          Return Code          ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³           Session Id          ³            Channel            ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                            Protocol                           ³ 11 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 12 ³                            Top Info                           ³ 27 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 28 ³                            Z_Struct                           ³ 47 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*       Note:  The Protocol section is used for other purposes within Gp.   */
/*       However, the only exposed use for it is to hold the protocol of     */
/*       the session.                                                        */
/*                                                                           */
/* The Top Info is as formatted as follows                                   */
/*                                                                           */
/*                                 Top Info                                  */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                  Channel of sleeping process                  ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³                      Address of caller rc                     ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                        Write Identifier                       ³ 11 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 12 ³                             Flags                             ³ 15 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/* Top                                                                       */
/* +++                                                                       */
/*                                                                           */
/* Top's format is as follows                                                */
/*                                                                           */
/*                                Top header                                 */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                            Reserved                           ³ 47 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/* QLLC                                                                      */
/* ++++                                                                      */
/* QLLC's format is as follows                                               */
/*                                                                           */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                    User SAP Correlator                        ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³                    User LS Correlator                         ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                        DLC Flags                              ³ 11 ³*/
/*³    ³N³X³ ³N³                                                   ³O³R³    ³*/
/*³    ³O³I³ ³E³                                                   ³F³S³    ³*/
/*³    ³R³D³ ³T³                                                   ³L³P³    ³*/
/*³    ³M³D³ ³D³                                                   ³O³P³    ³*/
/*ÃÄÄÄÄÅÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÅÄÄÄÄ´*/
/*³ 12 ³                DLH Length (DLC Header Length)                 ³ 15 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 16 ³           Netid               ³            Call_ID            ³ 20 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*                               Fixed section                               */
/*                               ~~~~~~~~~~~~~                               */
/*                                                                           */
/*                                                                           */
/* This section is the same for ALL buffers.  It is as follows               */
/*                                                                           */
/*                       Fixed Section for all buffers                       */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³  Packet Type  ³     Cause     ³   Diagnostic  ³     Flags     ³ 3  ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/* The packet type describes what type of packet this is. For Gp buffers,    */
/* this field is ignored as the stage provides more detailed information.    */
/*                                                                           */
/* The flags field define the values of the D, M and Q bits. This is a bit   */
/* mask using X25_D_BIT, X25_M_BIT and X25_Q_BIT.                            */
/*                                                                           */
/*                             Variable Section                              */
/*                             ~~~~~~~~~~~~~~~~                              */
/*                                                                           */
/*                                                                           */
/* This section is of variable length.  It holds the user data and other     */
/* variable format blocks.                                                   */
/*                                                                           */
/* Common variable sections                                                  */
/* ++++++++++++++++++++++++                                                  */
/*                                                                           */
/* There are some common variable sections.  These are                       */
/*                                                                           */
/*   Call                                                                    */
/*   ----                                                                    */
/*                                                                           */
/*                       Call buffer variable section                        */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                        Calling address                        ³ 19 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 20 ³                         Called address                        ³ 39 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 40 ³       Facilities length       ³        User Data length       ³ 43 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 44 ³             Facilities followed by call user data             ³ *  ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*   Data                                                                    */
/*   ----                                                                    */
/*                                                                           */
/*                                                                           */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                           User Data                           ³ *  ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/* Gp variable sections                                                      */
/* ++++++++++++++++++++                                                      */
/*                                                                           */
/*   Link Status Body                                                        */
/*   ----------------                                                        */
/*                                                                           */
/*                       Link Status variable section                        */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³ Packet level connection status³ Frame level connection status ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³        Hardware status        ³ No of Virtual circuits active ³ 7  ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*   Task Body                                                               */
/*   ---------                                                               */
/*                                                                           */
/*                           Task variable section                           */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                         Length of RCM                         ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³                    Pointer to the RCM data                    ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                        Length of Bladon                       ³ 11 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 12 ³                   Pointer to the Bladon data                  ³ 15 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*   Statistics body                                                         */
/*   ---------------                                                         */
/*                                                                           */
/*                        Statistics variable section                        */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³                  Reserved for internal Gp use                 ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³                        Statistics data                        ³ 319³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*   Csm body                                                                */
/*   --------                                                                */
/*                                                                           */
/* C&SM buffers are in one of three types.  Which type the buffer is in is   */
/* decided by the alert number.                                              */
/*                                                                           */
/*                                CSM Type 1                                 */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³  alert number ³      line     ³      lcn      ³   timer tick  ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³      mode     ³     state     ³      alert dependent data     ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                      alert dependent data                     ³ 15 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 16 ³ local, remote ³    PVC, SVC   ³     Calling address (BCD)     ³ 19 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 20 ³                  Calling address (continued)                  ³ 23 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 24 ³  Calling address (continued)  ³      Called address (BCD)     ³ 27 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 28 ³                   Called address (continued)                  ³ 31 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 32 ³   Called address (continued)  ³     cause     ³   diagnostic  ³ 35 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*                                CSM Type 2                                 */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³  alert number ³      line     ³      lcn      ³   timer tick  ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³      mode     ³     state     ³      alert dependent data     ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                      alert dependent data                     ³ 15 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 16 ³ local, remote ³    PVC, SVC   ³     Calling address (BCD)     ³ 19 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 20 ³                  Calling address (continued)                  ³ 23 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 24 ³  Calling address (continued)  ³      Called address (BCD)     ³ 27 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 28 ³                   Called address (continued)                  ³ 31 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 32 ³   Called address (continued)  ³            Timeout            ³ 35 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 36 ³    Retries    ³                    Reserved                   ³ 39 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/*                                CSM Type 3                                 */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³  alert number ³      line     ³      lcn      ³   timer tick  ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³      mode     ³     state     ³      alert dependent data     ³ 7  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 8  ³                      alert dependent data                     ³ 15 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 16 ³ local, remote ³    PVC, SVC   ³     Calling address (BCD)     ³ 19 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 20 ³                  Calling address (continued)                  ³ 23 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 24 ³  Calling address (continued)  ³      Called address (BCD)     ³ 27 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 28 ³                   Called address (continued)                  ³ 31 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 32 ³   Called address (continued)  ³   Diagnostic  ³  Explanation  ³ 35 ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 36 ³          Explanation          ³            Reserved           ³ 39 ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*                                                                           */
/* Top variable sections                                                     */
/* +++++++++++++++++++++                                                     */
/*                                                                           */
/* Not known                                                                 */
/*                                                                           */
/* QLLC variable sections                                                    */
/* ++++++++++++++++++++++                                                    */
/*                                                                           */
/*                           QLLC variable section                           */
/*ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄ¿*/
/*³ 0  ³ Address_Field ³ Control_Field ³           User Data           ³ 3  ³*/
/*ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄ´*/
/*³ 4  ³                     User Data (Continued)                     ³ *  ³*/
/*ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÙ*/
/*                                                                           */
/*       Note:  Address and Control fields are one byte each                 */
/*                                                                           */
/*                                                                           */
/*                               Buffer access                               */
/*                               -------------                               */
/*                                                                           */
/*                                                                           */
/* The buffer formats are VERY likely to change as new requirements and new  */
/* sections arrive.  The following macros have been defined to allow the     */
/* users of this buffer format to be protected from change ...               */
/*                                                                           */
/*                               Common Macros                               */
/*                               ~~~~~~~~~~~~~                               */
/*                                                                           */
/*                                                                           */
/* These macros may be used by all components.                               */
/*                                                                           */
/*    *  byte X25_READ_PACKET_TYPE(buffer_ptr)                               */
/*                                                                           */
/*    *  void X25_SET_PACKET_TYPE(buffer_ptr,packet_type)                    */
/*                                                                           */
/*    *  byte X25_READ_CAUSE(buffer_ptr)                                     */
/*                                                                           */
/*    *  void X25_SET_CAUSE(buffer_ptr , cause)                              */
/*                                                                           */
/*    *  byte X25_READ_DIAGNOSTIC(buffer_ptr)                                */
/*                                                                           */
/*    *  void X25_SET_DIAGNOSTIC(buffer_ptr , diagnostic)                    */
/*                                                                           */
/*    *  byte X25_READ_FLAGS(buffer_ptr)                                     */
/*                                                                           */
/*    *  void X25_SET_FLAGS(buffer_ptr)                                      */
/*                                                                           */
/*    *  void X25_READ_CALLING_ADDRESS(buffer_ptr , calling_address)         */
/*                                                                           */
/*    This copies the calling address from the buffer into the area pointed  */
/*    to by calling_address.                                                 */
/*                                                                           */
/*    *  void X25_SET_CALLING_ADDRESS(buffer_ptr , calling_address)          */
/*                                                                           */
/*    This copies from the calling address into the buffer.                  */
/*                                                                           */
/*    *  void X25_READ_CALLED_ADDRESS(buffer_ptr , called_address)           */
/*                                                                           */
/*    *  void X25_SET_CALLED_ADDRESS(buffer_ptr , called_address)            */
/*                                                                           */
/*    *  ushort X25_READ_FACILITIES_LENGTH(buffer_ptr)                       */
/*                                                                           */
/*    *  void X25_SET_FACILITIES_LENGTH(buffer_ptr , length)                 */
/*                                                                           */
/*    *  ushort X25_READ_CUD_LENGTH(buffer_ptr)                              */
/*                                                                           */
/*    *  void X25_SET_CUD_LENGTH(buffer_ptr , length)                        */
/*                                                                           */
/*                                                                           */
/* There are no macros provided to set or read the user data in either call  */
/* packets or data packets.  To do this , you should use JSMBUF_READ_BLOCK   */
/* and JSMBUF_SET_BLOCK with                                                 */
/*                                                                           */
/*    *  X25_OFFSETOF_USER_DATA                                              */
/*                                                                           */
/*    This is the offset from the start of the buffer to where the user      */
/*    data starts                                                            */
/*                                                                           */
/*    *  X25_OFFSETOF_FAC_CUD_DATA                                           */
/*                                                                           */
/*    This is the offset from the start of the buffer (which is in call      */
/*    format) to where the facilities and call user data should be stored    */
/*    in the variable section of the call buffer.                            */
/*                                                                           */
/* For example, if you want to copy the call user data from an mbuf pointed  */
/* to by mbuf_ptr into a byte array called cud, you could use the following  */
/*                                                                           */
/*    unsigned facilities_length=X25_READ_FACILITIES_LENGTH(mbuf_ptr);       */
/*    unsigned cud_length=X25_READ_CUD_LENGTH(mbuf_ptr);                     */
/*                                                                           */
/*    JSMBUF_READ_BLOCK(                                                     */
/*      mbuf_ptr,                                                            */
/*      X25_OFFSETOF_FAC_CUD_DATA+facilities_length,                         */
/*      cud,                                                                 */
/*      cud_length);                                                         */
/*                                                                           */
/*****************************************************************************/
#include <sys/types.h>

#ifdef QLLC
#include <x25/jsdefs.h>
#include <x25/jsmbuf.h>
#include <x25/crddefs.h>
#include <x25/crdcsm.h>
#else
#include "jsdefs.h"
#include "jsmbuf.h"
#include "crddefs.h"
#include "crdcsm.h"
#endif


/*****************************************************************************/
/* First, the correlator types....                                           */
/*****************************************************************************/
/* X25USER ON                                                                */

typedef ushort x25_session_id_t;                     
#define X25_INVALID_SESSION_ID ((x25_session_id_t)~0)

/*****************************************************************************/
/* The following macros define the different bits in the flag field of the   */
/* x25_packet_data_t structure                                               */
/*****************************************************************************/
#define X25_Q_BIT (0x1)
#define X25_D_BIT (0x2)
#define X25_M_BIT (0x4)

/*****************************************************************************/
/* The following macros define the possible values of the packet_type field  */
/* in the x25_packet_data_t structure                                        */
/*****************************************************************************/
#define PKT_CALL_REQ       (0)
#define PKT_INCOMING_CALL  (1)
#define PKT_CALL_ACCEPT    (2)
#define PKT_CALL_CONNECTED (3)
#define PKT_CLEAR_REQ      (4)
#define PKT_CLEAR_IND      (5)
#define PKT_CLEAR_CONFIRM  (6)
#define PKT_RESET_REQ      (7)
#define PKT_RESET_IND      (8)
#define PKT_RESET_CONFIRM  (9)
#define PKT_DATA           (10)
#define PKT_RR             (11)
#define PKT_D_BIT_ACK      (11)
#define PKT_INT            (12)
#define PKT_INT_CONFIRM    (13)
#define PKT_MONITOR        (14)

/*****************************************************************************/
/* The x25_packet_data structure contains information about the X.25 packets */
/* being sent or received.                                                   */
/*****************************************************************************/
struct x25_packet_data
{
  uchar packet_type;                              /* PKT_DATA  etc           */
  uchar cause;                                    /* Reason for reset / clear*/
  uchar diagnostic;
  uchar flags;                                    /* M, Q, D bits            */
};
typedef struct x25_packet_data x25_packet_data_t;

#define X25_VARIABLE_LENGTH_ARRAY (1)             /* Eye-catcher in decls.   */
/*****************************************************************************/
/* The x25_call_data structure is used to pass information about X.25 call   */
/* setup and termination packets                                             */
/* calling_address, called_address are null terminated ASCII strings         */
/*****************************************************************************/
struct x25_call_data
{
  char     calling_address[X25_MAX_ASCII_ADDRESS_LENGTH];
  char     called_address[X25_MAX_ASCII_ADDRESS_LENGTH];
  ushort   facilities_length;
  ushort   user_data_length;
  uchar    optional_data[X25_VARIABLE_LENGTH_ARRAY];
};
typedef struct x25_call_data x25_call_data_t;

/* X25USER OFF                                                               */

/*****************************************************************************/
/* First, lets define the various headers involved with the gp buffer        */
/*****************************************************************************/
struct intl_top_info_t
{
  void_ptr_t        caller_rc;                    /* 4  bytes                */
  unsigned          flags;                        /* 4  bytes                */
  void_ptr_t        stat_q_entry;                 /* 4  bytes                */
  int               call_id;                      /* 4  bytes                */
  pid_t             pid;                          /* 4  bytes                */
};
typedef struct intl_top_info_t top_info_t;

struct intl_gp_mbuf_header_t
{
  ushort           stage;                         /* Operation to perform    */
  ushort           rc;                            /* Result of the operation */
  x25_session_id_t session;                       /* Session to send on      */
  ushort           channel;                       /* What channel to assgn.  */
  union
  {
    struct
    {
      ushort       current_offset;                /* How far thru. sequence  */
      ushort       packet_size;                   /* Size of pkt. on session */
    } send_data;
    char           packet_header[4];              /* First bytes on a read   */
    uchar          protocol;                      /* Type of session to open */
  } opts;
  top_info_t       top_info;                      /* Top's info. to be saved */
  z_struct_t       z_struct;                      /* Card's info to be saved */
};
typedef struct intl_gp_mbuf_header_t gp_mbuf_header_t;

/* X25USER ON                                                                */
/*****************************************************************************/
/* The following macros define the possible values of the protocol field     */
/* in the x25_start_data structure                                           */
/*****************************************************************************/

#define X25_PROTOCOL_ELLC    (0)
#define X25_PROTOCOL_QLLC_80 (1)
#define X25_PROTOCOL_QLLC_84 (2)
#define X25_PROTOCOL_TCPIP   (3)
#define X25_PROTOCOL_YBTS    (4)
#define X25_PROTOCOL_ISO8208 (5)
/* X25USER OFF                                                               */

struct intl_xdh_mbuf_header_t
{
  unsigned no_header_required;
};
typedef struct intl_xdh_mbuf_header_t xdh_mbuf_header_t;

/*****************************************************************************/
/* The QLLC mbuf header is the same as the dlc_io_ext defined for GDLC       */
/* Device Managers, with the addition of netid and call_id fields for QLLC   */
/* routing and call acceptance/reject correlator notification.               */
/*****************************************************************************/
struct intl_qllc_mbuf_header_t
{
  unsigned int user_sap_correlator;
  unsigned int user_ls_correlator;
  struct
  {
    unsigned norm :1;           /* indicates normal data type                */
    unsigned xidd :1;           /* indicates xid data type                   */
    unsigned res1 :1;
    unsigned netd :1;           /* indicates netd data type                  */
	                        /* (so not used for Write)                   */
    unsigned res2 :26;
    unsigned oflo :1;           /* not used for Write                        */
    unsigned rspp :1;           /* not used for Write                        */
  } dlc_flags;
  unsigned int dlh_length;      /* not used for Write                        */
  unsigned short netid;         /* used for QLLC routing                     */
  unsigned short session_id;    /* used for session checking                 */
  unsigned short call_id;       /* only used on incoming calls               */
};
typedef struct intl_qllc_mbuf_header_t qllc_mbuf_header_t;

union intl_x25_mbuf_header_t
{
  gp_mbuf_header_t   gp_header;
  xdh_mbuf_header_t  xdh_header;
  qllc_mbuf_header_t qllc_header;
};
typedef union intl_x25_mbuf_header_t x25_mbuf_header_t;

/*****************************************************************************/
/* Now for the two QLLC bodies.                                              */
/*****************************************************************************/

/*****************************************************************************/
/* The first body is defined to allow easy manipulation of the QLLC Address  */
/* and Control fields, which precede the data. To read/write these fields it */
/* will be possible to use "offsetof".                                       */
/*****************************************************************************/
/*****************************************************************************/
/* Define the address and control field types so that code can simply use    */
/* sizeof to compute required buffer sizes.                                  */
/*****************************************************************************/
typedef uchar address_field_t;
typedef uchar control_field_t;

struct intl_qllc_body1_t
{
  address_field_t address_field;
  control_field_t control_field;
  char user_data[X25_VARIABLE_LENGTH_ARRAY];
};
typedef struct intl_qllc_body1_t qllc_body1_t;

/*****************************************************************************/
/* The second QLLC body is for network data buffers which carry transit delay*/
/* info and charging info to SNA.                                            */
/*****************************************************************************/
/*****************************************************************************/
/* Values are defined for the netd_type_flags field.                         */
/*****************************************************************************/
#define QLLC_CINF 0x80000000;
#define QLLC_TDSI 0x40000000;

struct intl_qllc_body2_t
{
  unsigned int netd_type_flags;
  unsigned int data_length;
  char data[X25_VARIABLE_LENGTH_ARRAY];
};
typedef struct intl_qllc_body2_t qllc_body2_t;

/*****************************************************************************/
/* Now for some Gp bodies ....                                               */
/* Most of the gp formats are similar to the call / data format, but there   */
/* are a few specialist ones.                                                */
/* These are link status, microcode download and statistics                  */
/*****************************************************************************/
struct intl_gp_link_status_body_t
{
  crd_link_status_t link_status;
};
typedef struct intl_gp_link_status_body_t gp_link_status_body_t;

struct intl_gp_task_body_t
{
  /***************************************************************************/
  /* If downloading bladon, fill in both the RCM and the bladon information  */
  /* If downloading the diagnostic tasks, just fill in the rcm section       */
  /* with the diagnostic task information                                    */
  /***************************************************************************/
  uchar    *rcm_ptr;                              /* RCM task                */
  unsigned rcm_length;                            /* Length of RCM task      */
  uchar    *bladon_ptr;                           /* Bladon task             */
  unsigned bladon_length;                         /* Length of bladon task   */
};
typedef struct intl_gp_task_body_t gp_task_body_t;

struct intl_gp_stats_body_t
{
  unsigned stats_index;                           /* statistic to consider   */
  unsigned stats_value[80];                       /* All the stats values    */
};
typedef struct intl_gp_stats_body_t gp_stats_body_t;

union intl_x25_mbuf_body_t
{
  x25_call_data_t       cd;
  char                  user_data[X25_VARIABLE_LENGTH_ARRAY];
  qllc_body1_t          qllc_body;
  qllc_body2_t          qllc_netd;
  gp_link_status_body_t gp_link_status;
  gp_task_body_t        gp_task;
  gp_stats_body_t       gp_stats;
  crd_csm_t             gp_csm;
};
typedef union intl_x25_mbuf_body_t x25_mbuf_body_t;

/*****************************************************************************/
/* The x25_mbuf_t structure is the generic structure for all of the          */
/* buffers in the x.25 subsystem.  This is the format of the Mbuf body.      */
/* This is superimposed on the data section of an Mbuf.                      */
/* WARNING: this is a variable length structure ... do not use sizeof on     */
/* it as it will not reflect the length of the optional_data section         */
/*****************************************************************************/
struct intl_x25_mbuf_t
{
  x25_mbuf_header_t header;                       /* LLC's headers           */
  x25_packet_data_t pd;                           /* Fixed section of Mbuf   */
  x25_mbuf_body_t   body;                         /* Variable data           */
};
typedef struct intl_x25_mbuf_t x25_mbuf_t;

/*****************************************************************************/
/* The following declaration is for external consumption only.  It will be   */
/* incorporated into an external header file in time.                        */
/*****************************************************************************/
/* X25USER ON                                                                */

#define X25_RESERVED_HEADER_LENGTH (sizeof(x25_mbuf_header_t))
struct x25_buffer
{
  char reserved[X25_RESERVED_HEADER_LENGTH];
  struct x25_packet_data pd;
  union
  {
    struct x25_call_data cd;
    char user_data[X25_VARIABLE_LENGTH_ARRAY];
  } body;
};

/* X25USER OFF                                                               */
/*****************************************************************************/
/* Finally, the most useful bit of it all... we must provide some macros to  */
/* access the common sections of the Mbufs.  Although the header is of       */
/* variable size, we can still access the body without too much problem      */
/* The macros assume that the first parameter passed is a valid Mbuf of      */
/* sufficient length.  Use JSMBUF_GUARANTEE_SIZE if the length is not        */
/* big enough                                                                */
/*****************************************************************************/
#define X25_READ_PACKET_TYPE(p) \
  (JSMBUF_READ_BYTE(p,OFFSETOF(pd.packet_type,x25_mbuf_t),uchar))

#define X25_SET_PACKET_TYPE(p,value) \
  (JSMBUF_SET_BYTE(p,OFFSETOF(pd.packet_type,x25_mbuf_t),value))

#define X25_READ_CAUSE(p) \
  (JSMBUF_READ_BYTE(p,OFFSETOF(pd.cause,x25_mbuf_t),byte))

#define X25_SET_CAUSE(p,value) \
  (JSMBUF_SET_BYTE(p,OFFSETOF(pd.cause,x25_mbuf_t),value))

#define X25_READ_DIAGNOSTIC(p) \
  (JSMBUF_READ_BYTE(p,OFFSETOF(pd.diagnostic,x25_mbuf_t),byte))

#define X25_SET_DIAGNOSTIC(p,value) \
  (JSMBUF_SET_BYTE(p,OFFSETOF(pd.diagnostic,x25_mbuf_t),value))

#define X25_READ_FLAGS(p) \
  (JSMBUF_READ_BYTE(p,OFFSETOF(pd.flags,x25_mbuf_t),byte))

#define X25_SET_FLAGS(p,value) \
  (JSMBUF_SET_BYTE(p,OFFSETOF(pd.flags,x25_mbuf_t),value))

/*****************************************************************************/
/* As the user data in a data packet is of variable length, we cannot        */
/* provide an easy way of getting at the user data.  Instead, you must use   */
/* the JSMBUF_READ_BLOCK or JSMBUF_SET_BLOCK to copy the data                */
/*****************************************************************************/
#define X25_OFFSETOF_USER_DATA (OFFSETOF(body.user_data[0],x25_mbuf_t))

/*****************************************************************************/
/* Now the macros to set up the contents of the call_data structure.  There  */
/* are macros for the calling, called address, facilities length, cud length */
/* and an OFFSETOF macro for the facilties / call user data block            */
/* N.B. the arguments passed to the calling and called address macros must   */
/* be at least X25_MAX_ASCII_ADDRESS_LENGTH long.  This routine ignored the  */
/* zero terminator and just copies that many bytes!                          */
/*****************************************************************************/
#define X25_READ_CALLING_ADDRESS(p,to)\
  ((JSMBUF_READ_BLOCK(\
    (p),\
    OFFSETOF(body.cd.calling_address[0],x25_mbuf_t),\
    (to),\
    (unsigned)X25_MAX_ASCII_ADDRESS_LENGTH)))
#define X25_SET_CALLING_ADDRESS(p,from)\
  ((JSMBUF_SET_BLOCK(\
    (p),\
    OFFSETOF(body.cd.calling_address[0],x25_mbuf_t),\
    (from),\
    (unsigned)X25_MAX_ASCII_ADDRESS_LENGTH)))

#define X25_READ_CALLED_ADDRESS(p,to)\
  ((JSMBUF_READ_BLOCK(\
    (p),\
    OFFSETOF(body.cd.called_address[0],x25_mbuf_t),\
    (to),\
    (unsigned)X25_MAX_ASCII_ADDRESS_LENGTH)))
#define X25_SET_CALLED_ADDRESS(p,from)\
  ((JSMBUF_SET_BLOCK(\
    (p),\
    OFFSETOF(body.cd.called_address[0],x25_mbuf_t),\
    (from),\
    (unsigned)X25_MAX_ASCII_ADDRESS_LENGTH)))

#define X25_READ_FACILITIES_LENGTH(p)\
  ((JSMBUF_READ_HALF_WORD(\
    (p),\
    OFFSETOF(body.cd.facilities_length,x25_mbuf_t),\
    ushort)))
#define X25_SET_FACILITIES_LENGTH(p,value)\
  ((JSMBUF_SET_HALF_WORD(\
    (p),\
    OFFSETOF(body.cd.facilities_length,x25_mbuf_t),\
    value)))
#define X25_READ_CUD_LENGTH(p)\
  ((JSMBUF_READ_HALF_WORD(\
    (p),\
    OFFSETOF(body.cd.user_data_length,x25_mbuf_t),\
    ushort)))
#define X25_SET_CUD_LENGTH(p,value)\
  ((JSMBUF_SET_HALF_WORD(\
    (p),\
    OFFSETOF(body.cd.user_data_length,x25_mbuf_t),\
    value)))
#define X25_OFFSETOF_FAC_CUD_DATA\
  (OFFSETOF(body.cd.optional_data[0],x25_mbuf_t))

/* Start of declarations for jsxbuf.h                                        */
/* End of declarations for jsxbuf.h                                          */

#endif
