# @(#)63        1.4  src/bos/diag/util/udsense/labels.dsense.sh, dsaudsense, bos411, 9435D411a 9/1/94 17:25:17
#
#   COMPONENT_NAME: DSAUDSENSE
#
#   FUNCTIONS: Aix
#               Content
#               Number
#               elseif
#               if
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
labels.dsense    Sense Data file for Aix(T) Release 1, Icr1a, Icr1b, 2
                 Contains Only information that is non-classified as to IBM
                 Content
                 (c) IBM Corporation 1991

                 This file contains some miscellaneous errors that can only
                 be classified by error label.  Generally it is best to list
                 errors by resource type instead of label because the decode
                 for the same label may be different depending on resource type.
                 In some cases, however, there is no type to decode

The following is a list of routines included:

       MISC_ERR      Miscellaneous interrupts
   MACHINECHECK      Machine Check
       DSI_IOCC      Data Storage Interrupt I/O Channel Converter
       DSI_PROC      Data Storage Interrupt Processor

!MACHINECHECK
%                   Machine Check
%
* Local Variables
#set model_220m20 0

0 3 @         | 1) Memory Location Value = 0x%.2x
0 3  = 30     |    Slot Location = A and B   ^model_220m20
0 3  = 31     |    Slot Location = C and D   ^model_220m20
0 3  = 32     |    Slot Location = E and F   ^model_220m20
0 3  = 33     |    Slot Location = G and H   ^model_220m20
0 3  = 41     |    Slot Location = A and E
0 3  = 42     |    Slot Location = B and F
0 3  = 43     |    Slot Location = C and G
0 3  = 44     |    Slot Location = D and H
0 3  = 45     |    Slot Location = E and A
0 3  = 46     |    Slot Location = F and B
0 3  = 47     |    Slot Location = G and C
0 3  = 48     |    Slot Location = H and D

0 14 @ 32     | 2) Machine-check Error Status Register (MESR) = 0x%.8x

~if model_220m20
              %    Model 220/M20
0 14 ^ 80    ||      Error while in diagnostic mode.
0 14 ^ 40    ||      Error during processor Load or Store.
0 14 ^ 10    ||      Address Exception - Address out of range (Software).
0 14 ^ 8     ||      Attempted Store to ROS (Software).
0 14 ^ 4     ||      Uncorrectable ECC. (SIMM)

~else
              %    Non-Model 220/M20
0 14 ^ 80    ||      Error while in diagnostic mdoe.
0 14 ^ 40    ||      I or D cache reload request.
0 14 ^ 20    ||      D cache reload request (not on RSC0).
0 14 ^ 10    ||      Store Back Request.
0 15 ^ 80    ||      Memory Bus Error - RAS/CAS Parity.
              |        - Probably the Card not the SIMM.
0 15 ^ 40    ||      Memory Timeout.
              |        - Probably the Card not the SIMM.
0 15 ^ 20    ||      Memory Card Failure - not applicable tp RSC0.
              |        - Probably the Card not the SIMM.
0 15 ^ 10    ||      Address exception - Address out of range (Software).
0 15 ^  8    ||      Attempted store to ROS (Software).
0 15 ^  4    ||      Uncorrectable ECC due to Address Parity.
              |        - Not applicable to RS1 or RS.9.
              |        - Probably the SIMM not the Card.
0 15 ^  2    ||      Uncorrectable ECC
              |        - Probably the SIMM not the Card.

~endif

0 18 @ 32     | 3) Machine-check Error Address Register (MEAR) = 0x%.8x

              %    Interleave Address for 4 DCU Machines =\
0 21 ^  20    | 1\
0 21 !^ 20    | 0\
0 21 ^  10    |1
0 21 !^ 10    |0
              %    Interleave Address for 2 DCU Machines =\
0 21 ^  10    | 1\
0 21 !^ 10    | 0\
0 21 ^   8    |1
0 21 !^  8    |0

!MISC_ERR
%                   Miscellaneous Error Sense Data Analysis
   * Have a miscellaneous error associated with a miscellaneous interrupt
   0 0 @ 32  | 1) IOCC Number (Bus ID) = 0x%.8x
   0 0 ^ 80  |    Memory Space ^mem_space
   0 0 !^ 80 |    I/O Space
   0 3 ^ 80  ^iocc_add
   0 3 ^ 40  ^rt_comp
   0 3 ^ 20  ^bypass
   ~if (mem_space)
       ~if (iocc_add)
                 %    IOCC Access
       ~elseif (! rt_comp)
                 %    Bus Memory Access
       ~elseif ( rt_comp )
                 %    RT Compatibility mode
       ~endif
   ~else
       0 3 @ @io_addr
       io_addr := io_addr & 0x0f
                 %    I/O addr ext = 0x%.1x ~io_addr
   ~endif
   0 4 @ 32  | 2) Bus Status Register = 0x%.8x
   0 7 @    @arb_lev
   arb_lev := arb_lev >> 4
             %    Arb Level = 0x%.2x ~arb_lev
   0 7 @    @bmode
   bmode := bmode & 0x0f
   0 7 ^ 04  |    Burst Mode
             %    Burst Mode/Chrdy/SDR(1)/SDR(0) = 0x%.2x ~bmode

   0 8 @ 32  | 3) Misc Interrupt Reg = 0x%.8x
   0 8 ^ 80  ||    Channel Check was detected            $have_reason
   0 8 ^ 40  ||    I/O Bus timeout interrupt occurred    $have_reason
   ~if (! have_reason)
     0 8 ^ 20 ||  Ctrl- Alt - anything keyboard sequence interrupt occurred
   ~else
     0 8 ^ 20 |    Ctrl- Alt - anything keyboard sequence interrupt occurred
   ~endif

!DSI_COMMON
*                   Data Storage Interrupt Common Routines
%
#define io_operations
#define seg_valid
#define BUID
#define iocc_select
#define rt_compatible
#define bypass
#define extent
#define csr15

0 0 @ 32      | 1) Data Storage Interrupt Status Register (DSISR) = 0x%.8x
0 0  ^ 80    ||    An I/O Exception.
0 0  ^ 40    ||    End of selected PFT chain & translation of an attempted access is not found.
0 0  ^ 20    ||    Storage access is not permitted by the data-locking mechanism.
0 0  ^ 10    ||    A floating load or store instruction references an I/O segment.
              |      (For example, a segment whose Segment register's T bit equals 1).
0 0  ^ 8     ||    A storage access is not permitted by the page protection mechanism.
              |      (Described in  "Page Protection").
0 0  ^ 4     ||    An access causes a loop in the translation mechanism
              |      (For example, the PFT search has gone on for more than 127 attempts).
0 0  ^ 2      |    A store operation.
0 0 !^ 2      |    A load operation.
0 0  ^ 1     ||    A data storage access crosses a segment boundary.
              |      The first segment accessed had T = 0 & the segment crossed into has T = 1.
%
0 8 @ 32      | 2) Segment Register = 0x%.8x

0 8  ^ 80     |    Load/Store instruction is targeted to I/O address spaces. ^io_operations
0 8 !^ 80     |    Load/Store instruction is targeted to System Memory.
             ||      Segment Register is invalid since since I/O was not selected.

0 8 @16   @BUID
BUID := BUID >> 4
BUID := BUID & 0xff

              %    Bus Unit Identification (BUID) = 0x%.2x ~BUID

~if (BUID < 0x20) || (BUID > 0x23)
             %%      The BUID does not select the IOCC, so the Segment Register is invalid.

~elseif (io_operations)

0 11 ^ 80     ^iocc_select
0 11 ^ 40     ^rt_compatible
0 11 ^ 20     ^bypass
0 11 @ @extent
extent := extent & 0xf

0 8  ^ 40     |    Privileged Key: in user mode.
0 8 !^ 40     |    Privileged Key: the operating system is in control.

0 9  ^ 8      |    A command issued to an invalid device address will cause a Data Storage
              %      interrupt to be posted and a card selected feedback error code to be set
              %      in Channel Status register 15.
0 9 !^ 8      |    An I/O Load or Store instructions that does not receive a positive address
              |      response from the Micro Channel will be allowed to proceed anyway.

0 9 ^  4      |    If multiple I/O Bus cycles are required for a Load or Store instruction
              |      then the I/O Bus address will be incremented between cycles.
0 9 !^ 4      |    If multiple I/O Bus cycles are required for a Load or Store instruction
              |      then the I/O Bus address will not be incremented between cycles.

  ~if (iocc_select)
              %    The effective address mode is IOCC Control.
0 8  ^ 40    ||      The Privileged Key is supposed to indicate operating system control.

  ~else

    ~if (rt_compatible)
              %    The I/O effective address mode is RT Compatiblity.

    ~else
              %    The I/O effective address mode is Standard Bus.
              %    Extent = 0x%.2x
              %      This field is concatenated with effective address bits 4 to 31 to form
              %      a 32-bit I/O bus address when working is Standard Bus Mode.
    ~endif

    ~if (bypass)
              %    The IOCC bypasses TCW checking and memory mapping  and only direct bus
              %      access is possible.

    ~else
              %    The extended functions of authority checking, access validation, and system
              %      consistency are invoked.
    ~endif
  ~endif
~endif

!DSI_IOCC
%                   Data Storage Interrupt I/O Channel Converter
%

#include DSI_COMMON
%

0 12 @ @csr15
csr15 := csr15 >> 4
csr15 := csr15 & 0xf
              % 3) Channel Status Register 15 = 0x%.2x ~csr15

csr15 = 1    ||    Invalid Operation.
              |      An attempt was made to access a facility or device not authorized by the
              |      system supervisor or an attempt was made to access a bus address for
              |      which a TCW does not exist (except when the bypass bit is on).

csr15 = 3    ||    Limit Check.
              |      An attempt was made to access a bus I/O device not within the address
              |      range established by the limit registers.

csr15 = 5    ||    Authority Error.
              |      An attempt was made to access a bus or system memory page and the storage
              |      key in the TCW does not match the authority mask in Channel Status
              |      register 15.  It can also be set if a write operation is attempted to a
              |      read-only page in system memory.

csr15 = 6    ||    Page Fault.
              |      An attempt was made to access a page with TCW bits 30 and 31 set to
              |      B`01'.  This should occur in normal operation.

csr15 = 8    ||    Channel Check.
              |      A device responded with a channel check indication.  For example, a
              |      device might respond with a channel check for a write operation to that
              |      device where there is bad parity on the data or for other device detected
              |      errors during an operation to that device.  This error cannot be reported
              |      if a card selected feedback error is reported.  (The card selected
              |      feedback error takes precedence over channel check error).

csr15 = 9    ||    Data Parity.
              |      The IOCC detected bad parity on a Load operation from an I/O device.
              |      (However, in the case of a Load operation, a channel check error takes
              |      precedence over a data parity error.) This error code is also set if the
              |      IOCC detects bad data parity or an uncorrectable ECC error during a load
              |      of a TCW.

csr15 = a    ||    I/O Bus Error.
              |      Set if an error on the Micro Channel has been detected during transfer.
              |      The types of errors detected here are implementation dependent.

csr15 = b    ||    Card Selected Feedback Error.
              |      A device was addressed and did not respond by driving the `cd sfdbk'
              |      line, and the address check bit was on in the I/O Segment register.
              |      Conditions which could cause this to occur are the device is not present,
              |      the device is not seated in the card slot properly, the device is not
              |      enabled, or the device detects bad address parity and does not respond to
              |      that address.  This error code takes precedence over a channel check.

csr15 = c    ||    Error Correcting Code (ECC) Error.
              |      The IOCC received an uncorrectable ECC error response from the internal
              |      system bus during a Load or Store instruction that is mapped to system
              |      memory.  (This process is similar to a bus master operation).

csr15 = d    ||    System Address Error.
              |      The IOCC sent an address over the system bus and did not receive an
              |      address acknowledgement.  This can occur if the real page number in the
              |      address is invalid.  Software should make sure that the real page number
              |      in the TCW is valid.

csr15 = e    ||    TCW Reload Error.
              |      The IOCC detected a parity or uncorrectable ECC error during an indirect
              |      TCW reload (with the bypass bit off).

csr15 = f    ||    IOCC Error.
              |      The IOCC detected an internal error during a Load or Store instruction.
              |      This error only occurs in a TCW or tag table access or flush command.
              |      All other IOCC errors result in a check stop.

!DSI_PROC
%                   Data Storage Interrupt Processor
%

#include DSI_COMMON
%

0 12 @ 32     | 3) EXVAL = 0x%.8x

0 12 = 0000000e    ||    EFAULT - Stored to an invalid address.

0 12 = fffffffa    ||    Invalid address not in memory.
                    |      This is usually the result of a page fault.  You will get this if
                    |      you tried to access something that was paged out while interrupts
                    |      are disabled.

0 12 = 00000005    ||    I/O Error.
                    |      This is a hardware problem.  This means that you got an I/O error
                    |      trying to page in/out, or you tried to access a memory mapped file
                    |      and could not do it.

0 12 = 00000086    ||    Protection Exception.
                    |      This means that you tried to store to a location that is protected.
                    |      This is usually low kernel memory.
