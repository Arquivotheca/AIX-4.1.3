# @(#)62        1.4.1.1  src/bos/diag/util/udsense/rtypes.dsense.sh, dsaudsense, bos411, 9435D411a 9/1/94 17:27:22
#
#   COMPONENT_NAME: DSAUDSENSE
#
#   FUNCTIONS: Address
#               C93A
#               Command
#               DCE
#               DTE
#               Disconnected
#               Drive
#               Error
#               FRMR
#               Information
#               N
#               Number
#               P
#               PVC
#               Receive
#               SABM
#               SVC
#               Send
#               T1
#               T4
#               Tape
#               Unused
#               VPD
#               address
#               available
#               block
#               call
#               check
#               d
#               disconnection
#               drive
#               elseif
#               error
#               event
#               expired
#               file
#               format
#               found
#               frame
#               hardfiles
#               if
#               initiator
#               large
#               level
#               limit
#               line
#               line_st
#               list
#               long
#               mismatch
#               mode
#               modulo
#               off
#               operation
#               parameter
#               protected
#               ready
#               remote
#               sent
#               timeout
#               tracks
#               unknown
#               up
#               use
#               user
#               usr
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
rtypes.dsense    Sense Data file
                 (c) IBM Corporation 1991, 1992, 1993

The following is a list of routines included

 x25           -- X.25
 TokenRng      -- Token Ring
 tokenring     -- Token Ring
 ethernet      -- Ethernet

 fd            -- Diskette Drive

 150mb         -- 150mb 1/4 Tape Drive
 8mm           -- 2.3gb 8mm Tape drive
 8mm5gb        -- 5gb 8mm Tape drive
 525mb         -- 525mb 1/4" Tape Drive
 1200mb-c      -- 1.2gb 1/4" Tape Drive
 4mm2gb        -- 2.0gb 4mm
 4mm2gb2       -- 2.0gb 4mm
 4mm4gb        -- 4.0gb 4mm
 4mm4gb2       -- 4.0gb 4mm
 9trk          -- 9 Track Tape Drive
 ost           -- Template for other SCSI tape drive assuming 8mm5bg tape

 cdrom         -- CDROM
 cdrom1        -- CDROM
 enhcdrom      -- Enhanced CDROM
 enhcdrom2     -- Enhanced CDROM
 enhcdrom3     -- Enhanced CDROM
 osomd         -- Rewriteable CDROM

 badisk        -- Bus Attached Hard Drive
 serdasda      -- Serial DASD Adapter
 serdasdc      -- Serial DASD Controller
 serdasdd      -- Serial DASD 857 Mb F Drive
 serdasdd1     -- Serial DASD 857 Mb C Drive
 serdasdd2     -- Serial DASD 471 Mb F Drive
 serdasdd3     -- Serial DASD 471 Mb C Drive
 serdasdd4     -- Serial DASD 1.07 Gb F Drive
 serdasdd5     -- Serial DASD 1.07 Gb C Drive

 200mb         -- 200 Mb drive
 270mb         -- 270 Mb drive (Quantum)
 270mb2        -- 270 Mb drive (Satsuma)
 320mb         -- 320 Mb drive
 360mb         -- 360 Mb drive (Satsuma)
 355mb         -- 355 Mb drive
 400mb         -- 400 Mb drive
 540mb         -- 540 Mb drive
 540mb2        -- 540 Mb drive (Satsuma)
 540mb3        -- 540 Mb drive (Quantum)
 540mb4        -- 540 Mb drive (Maxtor)
 670mb         -- 670 Mb drive
 730mb2        -- 730 Mb drive (Satsuma)
 857mb         -- 857 Mb drive
 1000mb        -- 1 GB drive
 1000mb2       -- 1 GB drive
 1000mbde      -- 1 GB drive (DE)
 1200mb        -- 1.2 Gb drive
 1370mb        -- 1.37 Gb drive

 1100mb        -- 1.1 GB drive
 1100mb16bit   -- 1.1 GB drive (Wide)
 1100mb16bitde -- 1.1 GB drive (Wide/DE)
 2000mb        -- 2 GB drive
 2000mbde      -- 2 GB Drive (DE)
 2000mb16bit   -- 2 GB drive (Wide)
 2000mb16bitde -- 2 GB drive (Wide/DE)
 2200mb        -- 2.2 GB drive
 2200mb16bit   -- 2.2 GB drive (Wide)
 2200mb16bitde -- 2.2 GB drive (Wide/DE)
 4500mb16bit   -- 4.5 GB drive (Wide)
 4500mb16bitde -- 4.5 GB drive (Wide/DE)
 osdisk        -- General Information about OEM disk drives

 hscsi         -- SCSI Adapter Device Driver information
 vscsi         -- SCSI Adapter Device Driver Info for Wide Adapter.
 8efc          -- SCSI Adapter Device Driver Info for Wide Adapter.
 pscsi         -- Integrated SCSI Controller for 7011 & SF
 8fba          -- NCR720 based Integrated SCSI Controller.
 ncr810        -- NCR810 based Integrated SCSI Controller.

 scsi_int_gen  -- General Decode information for Integrated SCSI devices
 scsi_gen      -- General Decode information for SCSI devices
 scsi_cmnd     -- General Decode for a SCSI command decode of intergrated SCSI
 scsi_hf       -- Particular Information for SCSI hardfiles (all types)
 quantum       -- General Decode information for Quantum family of drives
 satsuma       -- General Decode information for Satsuma family of drives


!x25
* X.25 Information
*
#set call_details 0
#set alert_1 0
#set alert_2 0
#set alert_3 0
#set transfer_on 0
%                      X.25 Sense Information Analysis
0  0  @       | 1) Alert Header Information (alert %d) @alert_no
0  0  = 05    ||    X-5 Alert: Modem failure detected in X.25 subsystem
              |        DCD and/or DSR failure detected or
              |        Watchdog timer expires -- no clocks or
              |        No cable on XIOSTART
0  0  = 07    ||    X-7 Alert: Local auto call unit not responding
              |       Unable to establish XID ie. 107(DSR) and 109(DCD)
              |         remain off after specified time.  Or
              |       XID established and the network forces VX32 OFF after
              |       failing to establish VC with 60 secs
0  0  = 08    ||    X-8 Alert: X.21 network connection not established
              |       Is an X21 failure in X.25 subsytem.
              |       DCE fault ie R=0 I=OFF after network timeout or
              |       DCE not ready ie I=OFF and R=010101.. for >= 24 intervals

0  0  = 09    ||    X-9 Alert: FRMR Frame received.  Case W & !X
0  0  = 0a    ||    X-10 Alert: FRMR Frame received. Case X & W
0  0  = 0b    ||    X-11 Alert: FRMR Frame received. Case Y
0  0  = 0c    ||    X-12 Alert: FRMR Frame received. Case Z
0  0  = 0d    ||    X-13 Alert: FRMR Frame transmitted. Case W & !X
0  0  = 0e    ||    X-14 Alert: FRMR Frame transmitted. Case X & W
0  0  = 0f    ||    X-15 Alert: FRMR Frame transmitted. Case Y
0  0  = 10    ||    X-16 Alert: FRMR Frame transmitted. Case Z
0  0  = 11    ||    X-17 Alert: Frame retry limit (N2) reached
               |      The local station has retried a command frame the max
               |      number of times without receiving an appropriate response.
0  0  = 12    ||    X-18 Alert: Disconnect frame received:
               |      An unexpected DISC command frame was received either during
               |      link setup or during information transfer
0  0  = 13    ||    X-19 Alert:Disconnect mode frame rcved during link activation
               |      The local station sent SABM to initialize the link and
               |      received a DM frame.
0  0  = 15    ||    X-21 Alert: DTE (concerns CLEAR_INDICATION packet) $alert_1
               |      A CLEAR_INDICATION packet containing a DCE-Originated
               |      cause code and a diagnostic was received by the DTE.
0  0  = 16    ||    X-22 Alert: DTE (concerns RESTART_INDICATION packet) $alert_1
               |      A RESTART_INDICATION packet containing a DEC-Originated
               |      cause code and a diagnostic was received by the DTE
0  0  = 17    ||    X-23 Alert: DTE (concerns RESET_REQUEST packet) $alert_1
               |      A RESET_REQUEST packet containing a DEC-Originated
               |      cause code and a diagnostic was received by the DTE
0  0  = 18    ||    X-24 Alert: DTE (concerns CLEAR_REQUEST packet) $alert_1
               |      A CLEAR_REQUEST packet containing a DEC-Originated
               |      cause code and a diagnostic was received by the DTE
0  0  = 19    ||    X-25 Alert: DTE (concerns RESTART_REQUEST packet) $alert_1
               |      A RESTART_REQUEST packet containing a DEC-Originated
               |      cause code and a diagnostic was received by the DTE
0  0  = 1a    ||    X-26 Alert: DTE (T20 expired before RESTART_CONFIRMATION)
               |      Time-limit T20 expired at the DTE prior to receipt of a
               |      RESTART_CONFIRMATION packet following transfer of a
               |      RESTART_REQUEST $alert_2
0  0  = 1b    ||    X-27 Alert: DTE (T22 expired before RESET_CONFIRMATION)
               |      Time-limit T22 expired at the DTE prior to receipt of a
               |      RESET_CONFIRMATION packet following transfer of a
               |      RESET_REQUEST $alert_2
0  0  = 1c    ||    X-28 Alert: DTE
               |      Time-limit T21 expired at the DTE prior to receipt of a
               |      CALL_CONNECTED or CLEAR_INDICATION packet following xfer
               |      of a CALL_REQUEST packet $alert_2
0  0  = 1d    ||    X-29 Alert: DTE (T23 expired before CLEAR_CONFIRMATION)
               |      Time-limit T23 expired at the DTE prior to receipt of a
               |      CLEAR_CONFIRMATION packet following transfer of a
               |      CLEAR_REQUEST $alert_2
0  0  = 1e    ||    X-30 Alert: DTE (protocol violation on part of DTE)
               |      A DIAGNOSTIC packet, indicating a protocol violation on
               |      the part of the DTE was received by the DTE $alert_3
0  0  = 1f    ||    X-31 Alert: DTE (RESET_INDICATION received by DTE) $alert_1
               |      A RESET_INDICATION packet containing a DCE originated
               |      cause code and a diagnostic was received by the DTE
0  0  = 20    ||    X-32 Alert: DCE (CLEAR_INDICATION sent by DCE) $alert_1
               |      A CLEAR_INDICATION packet containing a DCE originated
               |      cause code and a diagnostic was sent by the DCE
0  0  = 21    ||    X-33 Alert: DCE (RESET_INDICATION sent by DCE) $alert_1
               |      A RESET_INDICATION packet containing a DCE originated
               |      cause code and a diagnostic was sent by the DCE
0  0  = 22    ||    X-34 Alert: DCE (RESTART_INDICATION sent by DCE) $alert_1
               |      A RESTART_INDICATION packet containing a DCE originated
               |      cause code and a diagnostic was sent by the DCE
0  0  = 23    ||    X-35 Alert: DCE (RESTART_REQUEST received by DCE) $alert_1
               |      A RESTART_REQUEST packet containing cause code and a
               |      non-zero diagnostic code received by the DCE
0  0  = 24    ||    X-36 Alert: DCE(T10 exp. before rcpt of RESTART_CONFIRMATION)
               |      Time limit T10 expired at the DCE prior to receipt of a
               |      RESTART_CONFIRMATION packet following transfer of a
               |      RESTART_INDICATION packet. $alert_2
0  0  = 25    ||    X-37 Alert: DCE (T12 exp. before rcpt of RESET_CONFIRMATION)
               |      Time limit T12 expired at the DTE prior to receipt of a
               |      RESET_CONFIRMATION packet following transfer of a
               |      RESET_INDICATION packet. $alert_2
0  0  = 26    ||    X-38 Alert: DCE (T11 exp. )
               |      Time limit T11 expired at the DTE prior to receipt of a
               |      CALL_ACCEPTED or CLEAR_REQUEST packet following transfer
               |      of an INCOMING_CALL packet. $alert_2
0  0  = 26    ||    X-39 Alert: DCE (T13 exp. )
               |      Time limit T13 expired at the DCE prior to receipt of a
               |      CLEAR_CONFIRMATION packet following transfer of a
               |      CLEAR_INDICATION packet. $alert_2

0  1  @       |    Line Number (%d decimal)
0  2  @ 16    |    LCN Number  (0x%.2x)
0  2  = ffff  |       LCN Number is for phys/frame
0  4  @ 16    |    Clock Reference Tick count/secs (%d in decimal)

              % 2) Alert Reference Code Information
0  6  @       |    Mode of the layer  (0x%.2x)
0  6  = 00    |      DTE
0  6  = 01    |      DCE
0  6  = 03    |      DCE

* State defines
0  7  @       |    State of the layer (0x%.2x)
~if ((alert_no == 5) || (alert_no == 8))
              %      link_desc->lev1_updown state =\
  0 7 = 00                                         | Disconnected
  0 7 = 01                                         | Connected
  0 7 = 02                                         | Connection requested
             |                                       (CNX_REQUESTED)
  0 7 = 03                                         | Disconnection requested
             |                                       (CNX_REQUESTED)
~elseif (alert_no == 7)
              %      acu_state  =\
  0 7 = 00                       | Modem in off state (ACU_OFF)
  0 7 = 01                       | Modem available (ACU_ON)
  0 7 = 02                       | Modem trying to go on line (ACU_CALL)
  0 7 = 03                       | Modem in ans mode wait for call (ACU_W_CALL)
  0 7 = 04                       | Modem answering incoming call (ACU_IN_CALL)
  0 7 = 05                       | Modem on line and in data mode (ACU_DATA)
~elseif ((alert_no >= 9) && (alert_no <= 19))
*  State for Frame level alerts
              %      link_desc->line_st (Main line state) =
  0 7 = 00    |         0 - Passive mode link connection awaiting SABM,
              |             CCIT Link set-up, (OFF)
  0 7 = 01    |         1 - Active link mode connection SABM sent state,
              |             CCIT Link set-up, (SS)
  0 7 = 02    |         2 - FRMR sent state, (FS)
  0 7 = 03    |         3 - Connected, SABM/UA exchange done,
              |             CCIT information transfer phase, (ON) ^transfer_on
  0 7 = 04    |         4 - Break State, (BRK)
  0 7 = 05    |         5 - DISC sent state, CCITT link disconnection (DS)
  0 7 = 06    |         6 - CCITT link Disconnected phase,
              |             DISC/UA exchange done, (DISC)
  0 7 = 07    |         7 - Waiting for XID state, (WX)
~elseif ((alert_no >= 21) && (alert_no <= 39))
*  State for Packet level alerts
              %      Packet level state =
  0 7 = 01    |        ST1_DISC    -- Line disconnected, CCITT none
  0 7 = 02    |        ST2_RESTART -- Waiting on restart request, CCITT r2
  0 7 = 03    |        ST3_READY   -- Packet level ready, CCITT r1-p1
  0 7 = 04    |        ST4_CALL    -- Waiting on call request, CCITT r1-p2
  0 7 = 05    |        ST5_TRANS   -- Data transfer state, CCITT r1-p4-d1
  0 7 = 06    |        ST6_CLEAR   -- Waiting on a clear request, CCITT r1-p6
  0 7 = 07    |        ST7_RESET  -- Waiting on a reset request, CCITT r1-p4-d2
  0 7 = 08    |        ST8_CR_RESET -- Waiting for user reset confirmation
              |            CCITT r1-p4-d1 for network, CCITT r1-p4-d3 for user
  0 7 = 09    |        ST9_CF_CALL -- Waiting for user confirmation call
              |            CCITT r1-p3
  0 7 = 0a    |        ST3_READY   -- Packet level ready r1.
              |            Used for instance if restart confirmation rxd in
              |            packet level ready state.
  0 7 = 0c    |        Data transfer state. Used for instance in cases of call,
              |            call accept, clear confirmation being received in
              |            states ST5_TRANS or ST8_CF_RESET
  0 7 = 0d    |        Clear collistion state
~endif

              % 3) Further Alert Information \
0  8  @ 32    |(In Hex  %.8x \
0  12 @ 32    |%.8x \
0  16 @ 16    |%.4x)

* Misc[0] Defines

~if ((alert_no == 5) || (alert_no == 7) || (alert_no == 8))
              %    Cable id =\
  0 8 = 00                   | 0 - V24 cable attached, (CABLE_V24)
  0 8 = 01                   | 1 - V35 cable attached, (CABLE_V35)
  0 8 = 02                   | 2 - X21 cable attached, (CABLE_X21)
  0 8 = 03                   | 3 - No or unidentified cable attached
~elseif ((alert_no >= 9) && (alert_no <= 19))
  ~if (transfer_on)
              %    link_desc->send_st Transmit State =
    0 8 = 00  |      0 - Normal Send (NS)
    0 8 = 01  |      1 - Remote Busy,(RB)
    0 8 = 02  |      2 - T1 (time out) (T1)
  ~endif
~elseif ((alert_no >= 21) && (alert_no <= 39))
              %    Logical channel structure =
  0 8 = 4c    |      'L' - Line control LCID - 0
  0 8 = 50    |      'P' - PVC (TYPE_PERMANENT)
  0 8 = 49    |      'I' - Incoming SVC (TYPE_INCOMING)
  0 8 = 42    |      'B' - Bidirectional (2-way) SVC (TYPE_BIDIRECT)
  0 8 = 4f    |      'O' - Outgoing SVC (TYPE_OUTGOING)
  0 8 = 55    |      'U' - Unused (TYPE_UNUSED)
~endif

* Misc[1] Defines
~if ((alert_no == 5) || (alert_no == 8))
              %    Status of X21 =\
  0 9 = 00                       | 0 - Disconnected (X21_OFF) ^x21_off
  0 9 = 01                       | 1 - Connecting; waiting for stable R line
              |                        (X21_WAIT_R_STABLE)
  0 9 = 02                       | 2 - Connecting; waiting having assert C line
              |                        (X21_WAIT_AFTER_C)
  0 9 = 03                       | 3 - Connected, (X21_DATA_XFER)

  ~if ((alert_no == 5) && (! x21_off))
              %   WARNING-- In this alert was expecting status of X21 to be off
  ~endif
~elseif (alert_no == 7)
              %    ACU error status
  0 9 = 00                   | No error (ACU_ER_NO)
  0 9 = ff                   | Call failure indication (ACU_ER_CFI)
  0 9 = fe                   | Delayed call error (ACU_ER_DLC)
  0 9 = fd                   | Invalid command (ACU_ER_INV)
  0 9 = fc                   | Call aborted due to timeout (ACU_ER_TIME)
  0 9 = fb                   | Call aborted by user XIOACU abort (ACU_ER_ABORT)
  0 9 = fa                   | Carrier detect off (ACU_ER_CDOFF)
  0 9 = f9                   | Clear-to-send off (ACU_ER_CTS)
  0 9 = f8                   | Data-set-ready off (ACU_ER_DSR)
  0 9 = f7                   | Other error (ACU_ER_OTHER)
  0 9 = f6                   | Modem test indicator off (ACU_ER_TST)
  0 9 = f5                   | Link level establishment timeout (ACU_ER_SS)

~elseif ((alert_no >= 9) && (alert_no <= 19))
  ~if (transfer_on)
              %    link_desc->rcve_st Receive State =
     0 9 = 00 |      0 - Normal Receive (NS)
     0 9 = 01 |      1 - Reject state,(RJ)
  ~endif
~elseif ((alert_no >= 21) && (alert_no <= 39))
              %    Event packet level is processing =
  0 9 = 01    |      01 - Physical level is up (LEV1_CONNECT)
  0 9 = 02    |      02 - Physical level is down (LEV1_DISCONNECT)
  0 9 = 03    |      03 - Frame level is up (LEV2_CONNECT)
  0 9 = 04    |      04 - Frame level is down (LEV2_DISCONNECT)
  0 9 = 05    |      05 - Info frame received from frame level (FRAME_RECEIVED)
  0 9 = 06    |      06 - Flow control event (FR_FLOW_CTL)
  0 9 = 07    |      07 - T20 timeout after restart request sent, (T20_RESTART)
  0 9 = 08    |      08 - T21 timeout after call request sent, (T21_CALL)
  0 9 = 09    |      09 - T22 timeout after reset request sent, (T22_RESET)
  0 9 = 0a    |      10 - T23 timeout after clear request sent, (T23_CLEAR)
  0 9 = 0b    |      11 - T28 timeout after registration rqst sent (T28_REGIST)
  0 9 = 0e    |      14 - Data packet received from line (DATS_PK)
  0 9 = 0f    |      15 - RR packet received from line (RR_PK)
  0 9 = 10    |      16 - RNR packet received from line (RNR_PK)
  0 9 = 11    |      17 - Interrupt packet received from line
  0 9 = 12    |      18 - Interrupt confirmation packet received from line
  0 9 = 13    |      19 - Incoming call packet received from line
  0 9 = 14    |      20 - Call connected packet received from line
  0 9 = 15    |      21 - Clear indication packet received from line (CLEAR_PK)
  0 9 = 16    |      22 - Clear confirmation packet received from line
  0 9 = 17    |      23 - Reset indication packet received from line (RESET_PK)
  0 9 = 18    |      24 - Reset confirmation packet received from line
  0 9 = 19    |      25 - Diagnostic packet received from line
  0 9 = 1a    |      26 - Registration request packet recved from line (REG_PK)
  0 9 = 1b    |      27 - Registration confirmation packet received from line
  0 9 = 1c    |      28 - Restart indication packet received from line (REST_PK)
  0 9 = 1d    |      29 - Restart confirm. packer recvd from line (CF_REST_PK)
  0 9 = 1e    |      30 - Packet received with format error (ERROR_PK)
  0 9 = 1f    |      31 - Diagnostic package with time-out restart rcv from line
  0 9 = 20    |      32 - Diagnostic time-out restart received from line
  0 9 = 21    |      33 - Diagnostic time-out call received from line
  0 9 = 22    |      34 - Diagnostic time-out reset received from line
  0 9 = 23    |      35 - Diagnostic time-out clear received from line
  0 9 = 28    |      40 - Call request packet received from user (CALL_RQ)
  0 9 = 29    |      41 - User is reading incoming call packet (ACCEPT_CALL)
  0 9 = 2a    |      42 - Call accepted packet received from user (CALL_CF)
  0 9 = 2b    |      43 - Clear request packet received from user (CLEAR_RQ)
  0 9 = 2c    |      44 - Long clear request packet received from usr (FS_CL_RQ)
  0 9 = 2d    |      45 - Flow control event from adaptor (DATA_ACPT)
  0 9 = 2e    |      46 - Data packet received from user (DATA_SEND)
  0 9 = 2f    |      47 - Interrupt packet received from user (IT_SENT)
  0 9 = 30    |      48 - Reset request packet received from user (RESET_RQ)
  0 9 = 31    |      49 - Reset confirmation packet received from usr (RESET_CF)
  0 9 = 32    |      50 - User request for PVC (PVC_RQ)
  0 9 = 33    |      51 - Interrupt confirmation packet received from user
  0 9 = 34    |      52 - Registration packet received from user
  0 9 = 3c    |      60 - ISO timer T24 expired (T24_RR)
  0 9 = 3d    |      61 - ISO timer T25 expired (T25_DATA)
  0 9 = 3e    |      62 - ISO timer T26 expired (T26_IT)
~endif

* Misc[2]
~if ((alert_no >= 9) && (alert_no <= 19))
              %    link_desc->dustbin =
  0 10 = 00   |      0 - Receive buffers available
  0 10 = 01   |      1 - Receive buffer shortage, RNR sent
  0 10 = 03   |      3 - Sev buf shortage, rcved INFO frames discarded
              %          (Set after RNR sent to remote(LOCAL Busy))
~endif

* Misc[3]
~if ((alert_no >= 9) && (alert_no <= 19))
              %    Frame level event being processed =
  0 11 = 08   |      08 - Timer timed out event (TIME_OUT)
  0 11 = 09   |      09 - Busy timeout, RNR (TO_BUSY)
  0 11 = 0a   |      10 - Ignore the frame (FR_IGN)
  0 11 = 0b   |      11 - Receiver frame ready (FR_RR)
  0 11 = 0c   |      12 - Receiver frame not ready (FR_RNR)
  0 11 = 0d   |      13 - Reject frame (FR_REJ)
  0 11 = 0e   |      14 - Information or data frame (FR_INFO)
  0 11 = 0f   |      15 - SABM frame (FR_SABM)
  0 11 = 10   |      16 - SARM frame (FR_DM)
  0 11 = 11   |      17 - Disconnect frame (FR_DISC)
  0 11 = 12   |      18 - Un-numbered acknowledgement frame (FR_UA)
  0 11 = 13   |      19 - Frame reject frame FRMR (FR_FRMR)
  0 11 = 14   |      20 - The N(r) in the frame is incorrect (BAD_NR)
  0 11 = 15   |      21 - Frame type is unknown (FR_UNK)
  0 11 = 16   |      22 - Received an XID frame (FR_XID)
  0 11 = 17   |      23 - Received fram was too large (BAD_LENGTH)
  0 11 = 18   |      24 - Never received, used for LMS diag (FR_DUST)
~endif

* Misc[4]
~if ((alert_no >= 9) && (alert_no <= 19))
              %    Frame modulo(lin_desc->mod_count)=
  0 12 = 08   |      modulo 8 (MOD8) ^MOD8
  0 12 = 80   |      modulo 128 (MOD128)
~endif

* Misc[5,6 for alert 17]
~if (alert_no == 17)
              %    Frame that was retried N2 times =
  0 13 = 01   |      1 - FRMR (CF_FR_N2)
  0 13 = 02   |      2 - DISC (CF_DISC_N2)
  0 13 = 03   |      3 - SABM (CF_SS_N2)
  0 13 = 04   |      4 - T1 (CF_T1_N2)
  0 13 = 05   |      5 - T4 (CF_T4_N2)
  0 13 = 06   |      6 - Same INFO frame (CF_INFO_N2)

  0 14 @      |    Value of N2 = %d (in decimal)
~endif



* Misc[5-9]
~if ((alert_no >= 9) && (alert_no <= 16))
              %    FRMR information (\
  0 13 @      | %.2x\
  0 14 @      | %.2x\
  0 15 @      | %.2x\
  ~if (! MOD8)
    016 @     | %.2x\
    017 @     | %.2x\
  ~endif
              %)
~endif

~if (alert_1 || alert_2 || alert_3)
              % 4) Address details for packet alerts
 0  18 @      |    Direction (0x%.2x)
 0  18 = 00   |         Local initiated
 0  18 = 01   |         Remote initiated
 0  19 @      |    VC Type (0x%.2x)
 0  19 = 00   |         PVC
 0  19 = 01   |         SVC
              %    Calling Address, packed BCD (\
 0  20 @ 32   |%.8x \
 0  24 @ 32   |%.8x )
              %    Called Address,  packed BCD (\
 0  28 @ 32   |%.8x \
 1  0  @ 32   |%.8x )
~endif

 1  4  @ 32   | 5) Additional Alert Specific Information (0x%.8x)
~if (alert_1)
   1  4 @     |    Cause (0x%.2x) @cause
   1  5 @     |    Diagnostic code from packet (0x%.2x) @diagc
   ~if ((diagc >= 0) && (diagc <= 15))
              %      General or no additional information diag code category
     diagc = 01 |        Invalid P(S)
     diagc = 02 |        Invalid (PR)
   ~elseif ((diagc >= 16) && (diagc <= 31))
              %      Packet type invalid
    diagc =  11 |      For state r1
    diagc =  12 |      For state r2
    diagc =  13 |      For state r3
    diagc =  14 |      For state p1
    diagc =  15 |      For state p2
    diagc =  16 |      For state p3
    diagc =  17 |      For state p4
    diagc =  18 |      For state p5
    diagc =  19 |      For state p6
    diagc =  1a |      For state p7
    diagc =  1b |      For state d1
    diagc =  1c |      For state d2
    diagc =  1d |      For state d3
   ~elseif ((diagc >= 32) && (diagc <= 47))
              %      Packet not allowed diag code category
    diagc =  21 |      Unidentifiable Packet
    diagc =  22 |      Call on one way logic channel
    diagc =  23 |      Invalid packet type on a permanent virtual circuit
    diagc =  24 |      Packet on unassigned logic channel
    diagc =  25 |      Reject not subscribed to
    diagc =  26 |      Packet too short
    diagc =  27 |      Packet too long
    diagc =  28 |      Invalid general format indentifier
    diagc =  29 |      Restart or registration packet with nonzero in bits
                |      1 to 4 of octet 1, or bits 1 to 7 of octet 2
    diagc =  2a |      Packet type not compatible with facility
    diagc =  2b |      Unauthorized interrupt confirmation
    diagc =  2c |      Unauthorized interrupt
    diagc =  2d |      Unauthorized reject
   ~elseif ((diagc >= 48) && (diagc <= 63))
              %      Time expired
    diagc =  31 |      For incoming call
    diagc =  32 |      For clear indication
    diagc =  33 |      For reset indication
    diagc =  34 |      For restart indication
   ~elseif ((diagc >= 64) && (diagc <= 79))
              %      Call set up, call clearing or registration problem
    diagc =  41 |      Facility/registration code not allowed
    diagc =  42 |      Facility parameter not allowed
    diagc =  43 |      Invalid called address
    diagc =  44 |      Invalid calling address
    diagc =  45 |      Invalid facility/registration length
    diagc =  46 |      Incoming call barred
    diagc =  47 |      No logical channel available
    diagc =  48 |      Call collision
    diagc =  49 |      Duplicate facility requested
    diagc =  4a |      Non zero address length
    diagc =  4b |      Non zero facility length
    diagc =  4c |      Facility not provided when expected
    diagc =  4d |      Invalid CCITT-specified DTE facility
   ~elseif ((diagc >= 80) && (diagc <= 95))
              %      Miscellaneous
    diagc =  51 |      Improper diag code from DTE
    diagc =  52 |      Not aligned octet
    diagc =  53 |      Inconsistent Q bit setting
   ~elseif ((diagc >= 96) && (diagc <= 111))
              %      Not assigned
   ~elseif ((diagc >= 112) && (diagc <= 127))
              %      International problem
    diagc =  71 |      Remote network problem
    diagc =  72 |      International protocol problem
    diagc =  73 |      International link out of order
    diagc =  74 |      International link busy
    diagc =  75 |      Transit network facility problem
    diagc =  76 |      Remote network facility problem
    diagc =  77 |      International routing problem
    diagc =  78 |      Temporary routing problem
    diagc =  79 |      Unknown called DNIC
    diagc =  7a |      Maintenance action
   ~elseif ((diagc >= 80) && (diagc <= 255))
              %      Network specific diagnostic information
   ~endif
              %      NOTE:Specific information above about diag codes are
              %           based on standards and may not reflect actual protocol
~elseif (alert_2)
   1  4 @ 16  |    Timeout in secs  (%d)
   1  6       |         Retry count (%d decimal)
~elseif (alert_3)
   1  4 @     |    Diagnostic code from packet (0x%.2x)
   0 10 @ @exp_length
   ~if (exp_length > 0)
              %    Explanation codes from packet (\
      1 4 @   |0x%.2x \
      ~if (exp_length > 1)
         1 5 @   |0x%.2x \
      ~endif
      ~if (exp_length > 2)
         1 6 @   |0x%.2x \
      ~endif
              %)
   ~endif
~endif

!TokenRng
* Alternate entry for tokenring -- Calls tokenring
#include tokenring

!tokenring
* Token Ring Sense Data Analysis
%                      Token Ring Sense Data Analysis
%
                    % 1) Slot Information
0  8  @ 16          |    Adapter Slot = %d
1  28 @ 32          |    Token Ring Adapter Slot = %d
%
0  0  @ 32          | 2) Adapter State (0x%.8x)
0  0  =  0ACA DEAD  ||    Adapter is dead            $have_state
0  0  =  0ACA FFFF  |    Adapter is opening         $have_state
0  0  =  0ACA 0000  |    Adapter is closed          $have_state
0  0  =  0ACA 0001  |    Close phase 0              $have_state
0  0  =  0ACA 0002  |    Close cmd pending          $have_state
0  0  =  0ACA 0010  |    Reset phase 0              $have_state
0  0  =  0ACA 0011  |    Reset phase 1              $have_state
0  0  =  0ACA 0012  |    Reset phase 2              $have_state
0  0  =  0ACA 0020  |    Init phase 0               $have_state
0  0  =  0ACA 0021  |    Init phase 1               $have_state
0  0  =  0ACA 0030  |    Open phase 0               $have_state
0  0  =  0ACA 0031  |    Open cmd pending           $have_state
0  0  =  0ACA 0032  |    Adapter is open            $have_state
0  0  =  0ACA 0040  |    Kill pending               $have_state

~if (! have_state)
                    %    PROBLEM: Adapter was not found in any valid state.
~endif

%
                    % 3) Counters
0 12  @ 32          |    Number of cycles through network recovery = %d
0 16  @ 32          |    Number of software errors                 = %d
0 20  @ 32          |    Number of hardware errors                 = %d
0 24  @ 32          |    Number of microchannel errors             = %d
%
0  4  @  32         | 4) Network Recovery Mode Status (0x%.8x)
0  4  =  A440 0001  |    Paradise - ok
0  4  =  A440 0002  |    Reactivating-chaos
0  4  =  A440 0003  |    Recovering
0  4  =  A440 0004  |    Recovering - Limbo
0  4  =  A440 0005  |    SCB cmd pending
0  4  =  A440 0006  |    SCB cmd pending
0  4  =  A440 0007  |    Trace pending
0  4  =  A440 0008  |    Limbo kill pending
0  4  =  A440 FFFF  |    Fatal error
%
0 10  @  16         | 5) Reason for network recovery taking place if it did (0x%.4x)
0 10  =       0102  ||    Token ring adapter check
0 10  =       0104  |    Ring status
0 10  =       0108  ||    Tok implied force
0 10  =       0110  ||    Tok cmd failed
0 10  =       0204  ||    Tok ring adapter microcode error
0 10  =       0400  ||    Tok auto remove
0 10  =       0800  ||    Tok lobe wire fault

%
0 28  @ 16          | 6) Ring Status (0x%.4x)
0 28  ^       8000  ||    Signal loss
0 28  ^       4000  |     Hard error
0 28  ^       2000  |     Soft error
0 28  ^       0080  ||    Counter overflow
0 28  ^       0100  |    Remove received
0 28  ^       0400  |    Auto remove
0 28  ^       0800  ||    Lobe wire fault

%
0 30  @ 16          | 7) Last logged adapter error if any (0x%.4x)
0 30  =       0010  |    Init failed      $told_lae
0 30  =       0030  |    Reset failed     $told_lae
~if (! told_lae)
                    %    Value may be value from a pioread failure or
                    %    status from a bad open
~endif
%
1  0  @ 32          | 8) Device Driver State (0x%.8x)
1  0  =  0100 0000  |    Tx routine 0
1  0  =  0101 0001  |    Tx routine 1
1  0  =  0101 0002  |    Tx routine 2
1  0  =  0101 0003  |    Tx routine 3
1  0  =  0101 0004  |    Tx routine 4
1  0  =  0101 0005  |    Tx routine 5
1  0  =  0101 0006  |    Tx routine 6
1  0  =  0102 0000  |    Tx move list 0
1  0  =  0103 0000  |    Tx chain undo
1  0  =  0201 0000  |    Tok receive 0
1  0  =  0200 0001  |    Tok receive 1
1  0  =  0202 0000  |    Tok rec frame 0
1  0  =  0202 0001  |    Tok rec frame 1
1  0  =  0202 0002  |    Tok rec frame 2
1  0  =  0203 0000  |    Load receive chain
1  0  =  0204 0000  |    Read receive list
1  0  =  0205 0000  |    Clear receive list
1  0  =  0301 0000  |    Enter limbo mode
1  0  =  0302 0000  |    Limbo cycle 0
1  0  =  0302 0001  |    Limbo cycle 1
1  0  =  0302 0002  |    Limbo cycle 2
1  0  =  0302 0003  |    Limbo cycle 3
1  0  =  0302 0004  |    Limbo cycle 4
1  0  =  0302 0005  |    Limbo cycle 5
1  0  =  0302 0006  |    Limbo cycle 6
1  0  =  0303 0000  |    Limbo kill 0
1  0  =  0303 0001  |    Limbo kill 1
1  0  =  0303 0002  |    Limbo kill 2
1  0  =  0303 0003  |    Limbo kill 3
1  0  =  0304 0000  |    Limbo ergress
1  0  =  0305 0000  |    Limbo bug 0
1  0  =  0305 0001  |    Limbo bug 1
1  0  =  0305 0002  |    Limbo bug 2
1  0  =  0305 0003  |    Limbo bug 3
1  0  =  0305 0004  |    Limbo bug 4
1  0  =  0305 0005  |    Limbo bug 5
1  0  =  0305 0006  |    Limbo bug 6
1  0  =  0305 0007  |    Limbo bug 7
1  0  =  0305 0008  |    Limbo bug 8
1  0  =  0401 0000  |    SLIH 0
1  0  =  0401 0001  |    SLIH 1
1  0  =  0401 0002  |    SLIH 2
1  0  =  0401 0003  |    SLIH 3
1  0  =  0401 0004  |    SLIH 4
1  0  =  0401 0005  |    SLIH 5
1  0  =  0401 0006  |    SLIH 6
1  0  =  0401 0007  |    SLIH 7
1  0  =  0401 0008  |    SLIH 8
1  0  =  0401 0009  |    SLIH 9
1  0  =  0501 0000  |    OFLV CMD 0
1  0  =  0501 0001  |    OFLV CMD 1
1  0  =  0502 0000  |    OFLV ADAPTER
1  0  =  0503 0000  |    OFLV IOCTL 0
1  0  =  0503 0001  |    OFLV IOCTL 1
1  0  =  0503 0002  |    OFLV IOCTL 2
1  0  =  0503 0003  |    OFLV IOCTL 3
1  0  =  0504 0000  |    OFLV 0
1  0  =  0504 0001  |    OFLV 1
1  0  =  0505 0000  |    OFLV RSP 0
1  0  =  0505 0001  |    OFLV RSP 1
1  0  =  0505 0002  |    OFLV RSP 2
1  0  =  0505 0003  |    OFLV RSP 3
1  0  =  0505 0004  |    OFLV RSP 4
1  0  =  0601 0000  |    Receive mbuf timer
1  0  =  0701 0000  |    Reset adapter
1  0  =  0702 0000  |    Reset phase 0
1  0  =  0702 0001  |    Reset phase 1
1  0  =  0702 0002  |    Reset phase 2
1  0  =  0702 0003  |    Reset phase 3
1  0  =  0703 0000  |    Initialize adpater
1  0  =  0704 0000  |    Init phase 0
1  0  =  0704 0001  |    Init phase 0
1  0  =  0704 0002  |    Init phase 0
1  0  =  0705 0000  |    Init phase 1
1  0  =  0705 0001  |    Init phase 1
1  0  =  0705 0002  |    Init phase 1
1  0  =  0705 0003  |    Init phase 1
1  0  =  0705 0004  |    Init phase 1
1  0  =  0705 0005  |    Init phase 1
1  0  =  0706 0000  |    Open Adapter
1  0  =  0707 0000  |    Open adapter phase 0
1  0  =  0708 0000  |    Open adapter pending
1  0  =  0708 0001  |    Open adapter pending
1  0  =  0708 0002  |    Open adapter pending
1  0  =  0708 0003  |    Open adapter pending
1  0  =  0709 0000  |    Open adapter timeout
1  0  =  0709 0001  |    Open adapter timeout
1  0  =  0709 0002  |    Open adapter timeout
1  0  =  070A 0000  |    OFLV BRINGUP 0
1  0  =  070A 0001  |    OFLV BRINGUP 1
1  0  =  070A 0002  |    OFLV BRINGUP 2
1  0  =  070A 0003  |    OFLV BRINGUP 3
1  0  =  070A 0004  |    OFLV BRINGUP 4
1  0  =  070A 0005  |    OFLV BRINGUP 5
1  0  =  070A 0006  |    OFLV BRINGUP 6
1  0  =  070A 0007  |    OFLV BRINGUP 7
1  0  =  070A 0008  |    OFLV BRINGUP 8
1  0  =  070B 0001  |    Tok xmatt failure
1  0  =  070B 0002  |    Mem attach failure
1  0  =  070B 0003  |    Mem detach failure
1  0  =  070B 0004  |    D master failed
1  0  =  070B 0005  |    D clear failed
1  0  =  070B 0006  |    D complete failed
1  0  =  070B 0007  |    D init failed
1  0  =  070B 0008  |    D umask failed
1  0  =  070C 0000  |    Deactivate
1  0  =  070C 0001  |    Deactivate
1  0  =  070C 0002  |    Deactivate
1  0  =  070C 0003  |    Deactivate
1  0  =  0A01 0000  |    Download IOCTL MC 0
1  0  =  0A01 0001  |    Download IOCTL MC 1
1  0  =  0A01 0002  |    Download IOCTL MC 2
1  0  =  0A01 0003  |    Download IOCTL MC 3
1  0  =  0A02 0000  |    Download LDR IMAGE
1  0  =  0A03 0000  |    Download init microcode 0
1  0  =  0A03 0001  |    Download init microcode 1
1  0  =  0A03 0002  |    Download init microcode 2
1  0  =  0A03 0003  |    Download init microcode 3
1  0  =  0A03 0004  |    Download init microcode 4
1  0  =  0A03 0005  |    Download init microcode 5
1  0  =  0A03 0006  |    Download init microcode 6
1  0  =  0A03 0007  |    Download init microcode 7
1  0  =  0A03 0008  |    Download init microcode 8
1  0  =  0A03 0009  |    Download init microcode 9
1  0  =  0A03 000A  |    Download init microcode A
1  0  =  0A03 000B  |    Download init microcode B
1  0  =  0B01 0000  |    PIO read 0
1  0  =  0B01 0001  |    PIO read 1
1  0  =  0B02 0000  |    PIO write
%
                    % 9) Device Driver Recorded Status
1  4  @ 32          |    a) Current Pio attachment key = %d
1  8  @ 32          |    b) Number of PIO errors       = %d
1 12  @ 32          |    c) Last Pio expection status  = %d
* See /usr/include/sys/exept.h
1 12  = 0000 0080   ||       Floating Point Exception
1 12  = 0000 0081   ||       Invalid Op-code
1 12  = 0000 0082   ||       Privileged op in user mode
1 12  = 0000 0083   ||       Trap Instruction
1 12  = 0000 0084   ||       Alignment
1 12  = 0000 0085   ||       Invalid Address
1 12  = 0000 0086   ||       Protection
1 12  = 0000 0087   ||       Synchronous I/O
1 12  = 0000 0088   ||       I/O exception from IOCC
1 12  = 0000 0089   ||       I/O exception from SLA
1 12  = 0000 008a   ||       I/O exception from SCU
1 12  = 0000 0100   ||       Machine dependent exceptions
1 12  = 0000 0101   ||       I-fetch from the I/O segment
1 12  = 0000 0102   ||       I-fetch from special segment
1 12  = 0000 0103   ||       Data Lock
1 12  = 0000 0104   ||       Floating point load/store from/to IO segment
1 12  = 0000 0105   ||       Data straddles boundary between differing segs
1 12  = 0000 0106   ||       Too long or loop in hash chain
1 12  = 0000 0107   ||       Data storage interrupt
1 12  = 0000 0108   ||       Instuction storage interrupt
*      DEFINITION CAN BE FOUND IN /USR/INCLUDE/SYS/M EXCEPT.H
1 16  @ 32          |    Last Adapter Check Code block = 0x%.8x \
1 20  @ 32          | 0x%.8x
%
1  24 @ 16          | 10) Transmit Status (0x%.4x)
1  24 ^  0400       ||     TRANSMISSION ERROR
1  24 ^  1000       |     END   TX  FRAME
1  24 ^  2000       |     START TX FRAME
1  24 ^  8000       |     COMMAND COMPLETE
%
                    % 11) Hardware VPD Information
2  0  @ 16          |     Pos ID for adapter           = 0x%.4x
2  2  @ 16          |     Pos Reg 2, 3                 = 0x%.4x
2  4  @ 16          |     Pos Reg 4, 5                 = 0x%.4x
2  6  @ 16          |     Pos Reg 6, 7                 = 0x%.4x
2  8  @ 32          |     Tr network address           = 0x%.8x \
2 12  @ 16          | 0x%.4x
2 14  @ 32          |     Tr VPD address upper         = 0x%.8x \
2 18  @ 16          | 0x%.4x
2 20  @ 32          |     Tr VPD Ros Level             = 0x%.8x \
2 24  @ 16          | 0x%.4x

%
                    % 12) Last logged Token Ring Information
2 26  @ 16          |     %.4x \
2 28  @ 16          | %.4x \
2 30  @ 16          | %.4x \
3 0   @ 16          | %.4x \
3 2   @ 16          | %.4x \
3 4   @ 16          | %.4x \
3 6   @ 16          | %.4x \
3 8   @ 16          | %.4x

3 10  @ 16          |     %.4x \
3 12  @ 16          | %.4x \
3 14  @ 16          | %.4x \
3 16  @ 16          | %.4x \
3 18  @ 16          | %.4x \
3 20  @ 16          | %.4x \
3 22  @ 16          | %.4x \
3 24  @ 16          | %.4x

3 26  @ 16          |     %.4x \
3 28  @ 16          | %.4x \
3 30  @ 16          | %.4x \
4 0   @ 16          | %.4x \
4 2   @ 16          | %.4x \
4 4   @ 16          | %.4x

%
                    % 13) Token Ring alert data
4  6  @ 16          |     %.4x \
4  8  @ 16          | %.4x \
4 10  @ 16          | %.4x \
4 12  @ 16          | %.4x \
4 14  @ 16          | %.4x \
4 16  @ 16          | %.4x \
4 18  @ 16          | %.4x \
4 20  @ 16          | %.4x

4 22  @ 16          |     %.4x \
4 24  @ 16          | %.4x \
4 26  @ 16          | %.4x \
4 28  @ 16          | %.4x \
4 30  @ 16          | %.4x \
5  0  @ 16          | %.4x \
5  2  @ 16          | %.4x \
5  4  @ 16          | %.4x

5  6  @ 16          |     %.4x
!ethernet
* Sense information for the ethernet
%                        Ethernet Sense Data Analysis
%
0  0  @ 32          | 1) Error Number (0x%.8x)
0  0  = 00000004    |    Bad Address
0  0  = 00000087    |    I/O Exception
0  0  = 00000084    |    I/O Alignment Problem
0  0  = 0000004A    |    No buffer space available or no TCWs
0  0  = 00000003    |    Hard Failure or transmit carrier sense was lost
0  0  = 00000000    |    Operation Ok. Transmit completed but with an error

0  4  @             | 2) Command (0x%.2x)
0  4  = 00         ||    A parity error has occurred ^parity_error
0  4  = 10          |    Pack Received Mask
0  4  = 30          |    Out of resources mask
0  4  = 20          |    Abort reception mask
0  4  = 08          |    Adapter receive error
0  4  = 18          |    Adapter receive error
0  4  = 28          |    Adapter receive error
0  4  = 02          |    Single Packet Transmitted Error
0  4  = 06          |    Transmission error mask

~if parity_error
  0  5  @            | 3) Status register (0x%.2x)
  0  5  = c0         |    Parity Error
~endif

                    % 4) POS Register Information
0 6   @ 16          |       Card ID   = 0x%.4x
0 6  != F58E        |    WARNING Expected Card ID to be F58E
0 8   @ 16          |    Pos Regs 2,3 = 0x%.4x
0 10  @ 16          |    Pos Regs 4,5 = 0x%.4x
0 12  @ 16          |    Pos Regs 6,7 = 0x%.4x

                    % 5) Ethernet address in use (in hex)
0 14  @ 16          |    %.4x \
0 16  @ 16          |%.4x \
0 18  @ 16          |%.4x

                    % 6) Ethernet VPD address (in hex)
0 20  @ 16          |    %.4x \
0 22  @ 16          |%.4x \
0 24  @ 16          |%.4x


                    % 7) Ethernet Ros level from VPD (in hex)
0 26  @ 16          |    %.4x \
0 28  @ 16          |%.4x

0 30  @ 16          | 8) Ethernet VPD ROS Length (0x%.4x)

1 0   @ 16          | 9) Firmware Version Number (0x%.4x)
1 2   @ 32          |10) Exec Command in Progress Flag (0x%.8x)

* Will consider 6,7,8,9,10 To be reserved
1 11  @             |11) Adapter State (0x%.2x)
* Check the or out, may not be correct on next line
1 11  = 00          |    Adapter not started or parity error
1 11  = 01          |    Adapter reset is on wait 1/2 second
1 11  = 02          |    Adapter reset is off wait 1  second
1 11  = 03          |    Adapter self tests running
1 11  = 04          |    Get first byte of Execute mailbox byte
1 11  = 05          |    Get second byte of execute mailbox byte
1 11  = 06          |    Get third byte of execute mailbox byte
1 11  = 07          |    Get fourth byte of execute mailbox byte
1 11  = 08          |    Execute mailbox config commands started
1 11  = 09          |    Adapter started and acknowledged
* Don't really know if its 0a or 10
1 11  = 0a          |    Adapter close in progress
1 11  = 0b          |    One TX or RX abort command processed

1 12  @ 32          |12) Slot adapter is in (0x%.8x)
1 16  @ 16          |13) Adapter type field displacement (0x%.8x)
1 18  @ 16          |13) Net ID offset (0x%.4x)


!fd
* Sense information for Diskette drives -- See /usr/include/sys/fd.h
%               Floppy Disk Device Driver Sense Data Analysis
%
#define miss_data
%                  Diskette Interface Sense Data Analysis
%
0  0  @ 24    | 1) Drive Status Information (0x%.6x)
0  0  ^ 01    |    No drive selected (this bit always set until R2)
0  0  ^ 02    |    Drive 0 selected
0  0  ^ 04    |    Drive 1 selected
0  0  ^ 08    |    250 kbps transfer rate for data
0  0  ^ 10    |    300 kbps transfer rate for data
0  0  ^ 20    |    500 kbps transfer rate for data

0  1  ^ 02    |    1.44 Megabyte drive
0  1  ^ 08    |    1.2  Megabyte drive
0  1  ^ 10    |    Retries enabled
0  1  ^ 20    |    Device driver timer expired

0  2  ^ 02    |    Double-sided diskette
0  2  ^ 08    |    9  sectors per track
0  2  ^ 10    |    15 sectors per track
0  2  ^ 20    |    18 sectors per track
0  2  ^ 40    |    40 cylinders
0  2  ^ 80    |    80 cylinders


* Further explanation of floppdy drive status can be found referencing the
* National Semiconducter pub RRD-B20M78 relating to the DP8473 Floppy Disk
*  Controller
0  3  @ 16    | 2) Command Phase (0x%.4x)

0  5  @       | 3) Main Status Register of Controller (0x%.2x)
0  5  ^ 80    |    Controller Ready
0  5  ^ 40    |    Next byte expected to be read from diskette controller
0  5 !^ 40    |    Next byte expected to be written to diskette controller
0  5  ^ 20    |    Non-DMA Mode
0  5  ^ 10    |    Command is in process
0  5  ^ 08    |    Drive 3 is seeking
0  5  ^ 04    |    Drive 2 is sseeking
0  5  ^ 02    |    Drive 1 is sseeking
0  5  ^ 01    |    Drive 0 is sseeking

0  6  @       | 4) Disk Changed Register of Controller (0x%.2x)
0  6  ^ 80    |    Diskette Change indicated
0  6 !^ 80    |    Diskette Change not indicated


0  7  @  32   | 5) Status Registers (0x%.8x)
* D7-D6 are interrupt code. 00= Normal, 01 =Abnormal, 10=Invalid cmd,
*                           11= rdy change
0 7   @       |    @int_code
int_code := int_code >> 6
int_code = 00 |    Cmd terminated normally
int_code = 01 |    Cmd terminated abnormally. Execution not complete
int_code = 02 |    Drive received an invalid command
int_code = 03 |    Ready status changed during polling mode

0  7  ^ 20    |    Seek or recalibrate completed
0  7  ^ 10    ||    No track 0 occurred after a recalibrate
0  7  ^ 04    |    Head 0 at end of execution phase
0  7  !^ 04   |    Head 1 at end of execution phase
0  7 @        |    @sel_cd
sel_cd := sel_cd & 03
sel_cd = 00   |    Selected drive is 0
sel_cd = 01   |    Selected drive is 1
sel_cd = 02   |    Selected drive is 2
sel_cd = 03   |    Selected drive is 3


0  8  ^ 80    |    All of track transfered with out terminal count activated
0  8  ^ 20    |    CRC Error $was_crc
0  8  ^ 10    |    Over Run -- Data not take from controller quickly enough
              |               during a data transfer
0  8  ^ 04    ||    Can' return data for a cmd
              |             a) Controller can't find specified sector but
              |                at least an address mark was found.
              |                (Possible media error)
              |          or b) CRC errors in address field occur during
              |                Read ID cmnd
              |          or c) Starting sector can't be found in a read
              |                track cmd
0  8  ^ 02    ||    Write Protected Media
0  8  ^ 01    ||    Address mark missing $was_addm

0  9  ^ 40    ||    Wrong Type of Address mark detected
              |       Deleted address mark found when not expected
              |       Or a regular mark found when a delete mark expected
0  9  ^ 20    ||    Data CRC Error $crc_data
~if (crc_data && (! was_crc))
              %    WARNING. CRC error field not set above. This CRC report
              %    may not be valid
~endif
0  9  ^ 10    ||    On wrong track
              |     Desired sector not found and a track number on a sector
              |        of the current track is not that recorded in track reg
0  9  ^ 08    |    Scan Equal -- Scan command equal condition satisfied
0  9  ^ 04    ||    Scan Not Satisified -- Sector scan condition not satisf.
0  9  ^ 02    ||    Bad Track --
              |        Sector not found and a track number found that does
              |        not match the track register and recorded track
              |        number is 99 (May be a media prob)
0  9  ^ 01    ||    Data field Address Mark missing -- $miss_data
              |        Can't find the data address mark during a read/scan
~if (miss_data && (! was_addm))
              %    WARNING. Missing address mark bit not set.  This report
              %    may not be valid
~endif
0 10  ^ 40    |    Media is write protected
0 10  ^ 10    |    Track 0
0 10  ^ 04    |    Head 0
0 10  !^ 04   |    Head 1

0 10 @        @    drive_id
drive_id :=  drive_id >> 6
drive_id = 00 |    Drive 0
drive_id = 01 |    Drive 1
drive_id = 02 |    Drive 2
drive_id = 03 |    Drive 3

%
0  11  @      |   Cylinder              = %d (decimal)
0  12  @      |   Head                  = %d (decimal)
0  13  @      |   Sector                = %d (decimal)
0  14  @      |   # bytes/sector        = %d (decimal)
0  15  @      |   Head Settle Time      = %d (milliseconds)
0  16  @ 32   |   Motor Speed           = %d (milliseconds)
0  20  @ 32   |   Mbytes read           = %d (decimal - since last config)
0  24  @ 32   |   Mbytes written        = %d (decimal - since last config)

!150mb
* Sense information for 1/4 inch tape drives

%               1/4 Inch 150mb Tape Drive Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_tape 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @       | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  @      @err_class
                  0 20  @      @err_cd
                  0 20  ^ 70   @extended_sense
* Note, if extended information, error class and code come from byte 1,0
                 ~if extended_sense
                     1  0  @      @err_class
                     1  0  @      @err_cd
                  ~endif
                  err_class := err_class & 0x70
                  err_cd    := err_cd    & 0x1f

err_class @  |    a) Error Class (0x%.2x)
err_class =  00    |       Drive Error
err_class =  10    |       Target Error
err_class =  20    |       System related error
err_class =  30    |       Vendor unique error conditions
err_class =  70    |       Extended Sense Status Block ^extended_sense

err_cd @     |    b) Error Code (0x%.2x)

err_cd =  00 |       No sense -- Drive detected no errors during last cmd
             |         or drive reserved for other initiator.
err_cd =  04 ||       Hardware Err -- Drive detected failure
             |         with capstan, head servo, media sensor or tape cartridge
err_cd =  09 ||       Media not loaded --
             |         Tape not in drive, door open, or tape not loaded
err_cd =  0A ||       Insufficient Capacity -- Not enough room left on tape
err_cd =  11 ||       Uncorrectable Write Data error -- 16 retries failed
             |         to write or passed physical end of media
err_cd =  14 ||       No record found -- There is no data on the tape
err_cd =  17 ||       Write protected -- Tape is read only, write attempted
err_cd =  19 ||       Bad Block Found -- Couldn't read block (had 24 retries)
err_cd =  1C ||       File Mark Detected -- File mark found during read
             |         Outstanding read terminated. Tape at end of file mark
err_cd =  1D ||       Compare Error -- Miscompare on verify
err_cd =  20 ||       Invalid Command -- Last command passed was invalid
             |         or had illegal or inappropriate parameters
err_cd =  30 ||       Unit Attention -- Cartridge has been changed and load
             |         command given or drive was reset. However a new
             |         command was given.
err_cd =  33 ||       Append Error -- Write command given but end of recording
             |         area was not detected on previous read
err_cd =  34 ||       Read End-of-Media -- Read past end of recorded area was
             |         attempted or a Read after a write encountered.

~if (! extended_sense)
   ~if (valid_ibytes)
      0 21 @ 24  |     c)  Information Bytes (0x%.6x)
   ~endif
~else
             % 7) Extended Sense Information
  0 21 @     |    a) Segment Number = 0x%.2x
             %    b) Flags
  0 22 ^ 80  |         File Mark Detected
  0 22 ^ 40  |         End of Media Detected
  0 22 ^ 20  |         Illegal Length Indicator -- Logical Block len!= reqstd

   ~if (valid_ibytes)
     0 23 @ 32 |    c) Information Bytes (0x%.8x)
   ~endif
  1  1  @    |    d) Extended Error Code (0x%.2x)
  1  1  = 01 ||       Append Error -- Write cmd issued when LEOT was not
             |         detected in the proceeding Read operation.
  1  1  = 02 ||       Bad Command Block -- Some bits in the cmd block are
             |         invalid or the command itself is invalid
  1  1  = 03 ||       Bad Parameter Block -- Some parameter(s) of the cmd
             |         are not valid
  1  1  = 04 ||       Bus Parity Error -- Parity error in date xfer during wrt
  1  1  = 06 ||       Capstan Servor Error -- Err in drive motor or tape
             |         with too high torque
  1  1  = 08 ||       Compare error on verify -- Drive detected miscompare
             |         during verify operation
  1  1  = 0b ||       File Mark Detected -- A file mark detected on tape
             |         during a read when data should have been read
             |         Outstanding read terminated. Tape at end of file mark
  1  1  = 0c ||       Head Servo Error -- Error in head motor
  1  1  = 0d ||       Illegal Command -- Unknown/illegal command issued
  1  1  = 0f ||       Illegal Length -- Length of the read block is not
             |         equal to the requested length
  1  1  = 10 ||       Inappropriate Request --
             |         Attempt made to backspace filemark blocks
             |         or write attempt to wrong media or format type.
  1  1  = 12 ||       No cartridge in drive -- Drive does not have cartridge
             |         inserted or cartridge was removed before an
             |         unload command.
  1  1  = 13 ||       Not loaded -- Drive has not been logically loaded
             |         with a load command or automatic load
  1  1  = 15 ||       No data on tape -- Tape is completely blank
  1  1  = 16 ||       Read after write -- Read command issued after a write
  1  1  = 17 ||       Read EOM Logical -- Logical end of media detected during
             |         a read command
  1  1  = 18 ||       Read EOM Physical -- Physical end of media detected
             |         during a Read command
  1  1  = 19 ||       Reservation Conflict -- Drive reserved for another
             |         initiator
  1  1  = 1a ||       Sensor Error -- Media or hardware error detected
             |         in the sensor system
  1  1  = 1b ||       Tape Runout --
  1  1  = 1c ||       Unit Attention --
             |         Cartridge inserted into drive or bus reset performed
             |         and then a command was given that did not take
             |         this in to account
  1  1  = 1d ||       Write EOM Warning -- Tape went beyond psuedo early warning
             |         on the last track of tape during a write
  1  1  = 1e ||       Write EOM -- Tape has reached physical end of media
             |         on the last track of tape during a write
  1  1  = 1f ||       Write Protected -- Tape cartridge is write protected
  1  1  = 20 ||       16 Rewrites -- A physical block rewritten 16 times
             |          without success. A permanent write check.
  1  1  = 22 ||       24 ReReads -- A physical block has been reread 24 times
             |         without success. A permanent read check.
  1  1  = 2f ||       Space Reverse Position --
             |         Space reverse command unable to locate correct block
  1  1  = 30 ||       Space Reverse Iteration --
             |         Space Reverse command used too many iterative calls
             |          to internal functions
  1  1  = 31 ||       Selftest-buffer size -- Error running self test
  1  1  = 32 ||       Selftest-rereads -- Error running self test
  1  1  = 33 ||       Selftest-rewrites -- Error running self test
  1  1  = 34 ||       Selftest-buffer low -- Error running self test
  1  1  = 35 ||       Selftest-buffer high -- Error running self test
  1  1  = 36 ||       Selftest-Unspeced fatal -- Error running self test
  1  1  = 37 ||       Selftest-timeout -- Error running self test
  1  1  = 38 ||       Selftest-buffer -- Error running self test
  1  1  = 39 ||       Selftest-drive control -- Error running self test
  1  1  = 3A ||       Selftest-EPROM Verify -- Error running self test
  1  1  = 3b ||       Selftest-EPROM Error -- Error running self test
  1  1  = 3c ||       Selftest-External RAM -- Error running self test
  1  1  = 3d ||       Selftest-SCSI CNTRL -- Error running self test
  1  1  = 3e ||       Selftest-Spurious Interrupt -- Error running self test
  1  1  = 3f ||       Selftest-Stack Overflow -- Error running self test
  1  1  = 41 ||       24 re-reads, variable --
             |         Variable block has been re-read 24 times unsuccessfully.
             |         A permanent read check
  1  1  = 42 ||       Space Reverse into BOT -- Beginning of tape was detected
             |         during a space reverse command
  1  1  = 45 ||       Append failure -- 16 rewrites failed during an append
             |         sequence. Permanent data check
  1  1  = 46 ||       Illegal Command w/vad --
             |         The command format (fixed or variable block) doesn't
             |         match what is written on the tape.
  1  2 @ 24  |    e) Block Counter           = %d (decimal)
  1  5 @ 16  |    f) File Mark Counter       = %d (decimal)
  1  7 @ 16  |    g) Underrun Counter        = %d (decimal)
  1  9 @ 16  |    h) Recoverable err Counter = %d (decimal)
  1 11 @ 8   |    i) Marginal Counter        = %d (decimal)
  1 12 @ 8   |    j) Remaining Blocks        = %d (decimal)
  1 13 @ 16  |    k) ECC Correction Counter  = %d (decimal)
1 13 > 00 00 |       Note. ECC Correction applied implies ECC being used.
             |       ECC is not supported for this tape drive.

* Note, can someday implement a soft error check as follows:
* if (Block Counter / Recoverable err ratio) on a read
* is < 1388x then the error rate is too high
* On a write if it is < 0x014f then the error rate is too high

%
~endif

!8mm
* Sense information for 8mm tape drives
%                     8mm Tape Drive Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_tape 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @     | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  ^ 70   @extended_sense

~if (! extended_sense) && (request_sense_cmnd)
             %     WARNING:SCSI CMND was request sense, but sense
             %     returned was not valid extended_sense information.
             %     The values will be translated as such but may be wrong
~endif

0 21  @    |    a) Segment Number = 0x%.2x
0 21 != 00 |       Warning Segment Number != 0x00, but documentation says
           |       it should be 0x00.
0 22  @    |    b) Flags / Sense Key Info (0x%.2x)
0 22  ^ 80 |       File Mark Detected
0 22  ^ 40 |       End of Media Detected
0 22  ^ 20 |       Illegal Length Indicator -- Logical Block len!= requested
             0 22 @   @sen_k
             sen_k := sen_k & 0x0f
sen_k = 00 |       No Sense -- There is no specific information.
           |        Sense request may be requested due to flags above.
sen_k = 01 |       Recovered Error -- This should not be seen
sen_k = 02 ||       Not Ready -- Cassette not inserted or device not loaded
sen_k = 02 ^not_ready

* Determine if additional Sense Code is valid
1 0  = 04  |  $have_sensecode
~if not_ready
  1  1 = 00  |          Note: Tape is not present
  1  1 = 01  |          Note: Tape is not present. Presume rewinding or
             |           reloading
~endif
sen_k = 03 ||       Medium Error -- Unrecovered data error.
            |        Most likely a flaw in the tape or a formatter write
            |        parity error on data operation.
sen_k = 04 ||       Hardware Error -- Power up selftest failed.
           |        Internal data parity err detected, servo failure,
           |        sense err or tape cassette failure. Unrecoverable
sen_k = 05 ||       Illegal Request -- Illegal parameter detected in cmd
           |        descriptor block or in additional parms supplied as
           |        data.  Can occur from append error inappropriate result
sen_k = 06 ||       Unit Attention Condition --
           |        Could be due to drive being reset, mode select parms
           |        changed by another initaior or Tape cassette changed
sen_k = 07 ||       Data Protect -- Trying to write to write protected tape
sen_k = 08 ||       Blank Check -- Logical end of media detected
           |        while reading, verifying or spacing.
sen_k = 09 ||       Vendor Unique -- A Tape Mark Detect error
sen_k = 0a ||       Copy Aborted -- Not used
sen_k = 0b ||       Aborted Command -- Device aborted command.
           |        An initiator detected err during command execution,
           |        message reject, or SCSI bus parity error detected by
           |        the drive will cause this condition
sen_k = 0c |       Equal -- Not Used
sen_k = 0d ||       Volume Overflow -- Last write or write filemark cmd
           |        reached physical end of tape and the data or filemark
           |        may remain in the buffer
sen_k = 0e |       Miscompare -- Not Used
sen_k = 0f |       Reserved   -- For Future Use

1  7  @ 24 |    c) Additional Flags (0x%.6x)
1  7  ^ 80 ||       Power Fail -- Device just completed SCSI Reset or POR
1  7  ^ 40 ||       SCSI Bus Parity Err -- Detected during data phase.
           |        Residual count contains # of blocks not written
           |        (unless variable mode when block is not written)
1  7  ^ 20 ||       Format Buffer Parity Err -- Internal buffer parity err
           |         in write formatter
1  7  ^ 10 ||       Media Error (generally means a retry count exhausted)
           |        One of the following:
           |         --Write retry exhausted (>12 retries or 40 cons bad
           |            blocks and reposition not able to wrt 11 in 5 trcks
           |         --Read data retry exhausted (>10 repositions)
           |         --Write filemark retry exhausted ( > 18 tracks )
           |         --Formatter parity err on data op
1  7  ^ 08 ||       Error counter overflow -- Not an error condition
1  7  ^ 04 ||       Tape Motion Error -- Servo tracking retries > 9
1  7  ^ 02 ||       Tape Not Present
1  7  ^ 01 |       Beginning of Tape (Not an error condition)

1  8  ^ 80 ||       Transfer abort Error -- Device failed to pause data
           |        in preparation for disconnect.
1  8  ^ 40 ||       Tape Mark Detect Err
           |        Dev could not find tape marks in the correct number.
           |        A recoverable error being a Mismatch between # of
           |        filemarks spaced and physical ID block.
           |        Residue indicates number of blocks missing
1  8  ^ 20 |       Write Protect - Cassette is write protected (information)
1  8  ^ 10 ||       File mark error -- Permanent write error on write filemark
           |        ( >18 tracks needed to write at least 1 block/track)
1  8  ^ 08 ||       Under run error -- Can't continue data flow.
           |        Hardware or microcode error possible
1  8  ^ 04 ||       Write Error 1 --  greater than 12 write retries
1  8  ^ 02 ||       Servo System Error -- Servo detected an err.
           |         Will unload cassette
1  8  ^ 01 ||       Formatter Error -- Data formatter detected an error.
1  9  ^ 02 ||       Write Splice Error B -- Blank tape found (tape motion err)
           |        while looking for a new track
1  9  ^ 01 ||       Write Splice Error O -- Overshoot of track on splicing
           |        from existing tracks (tape motion error)
           %    d) Counters and Other Numeric Information
0 23 !@ 32 |       Residue                 = %d (decimal)
           |         (Number of blocks not transfered)
0 27 @     |       Additional Sense Length = %d (decimal)
0 31 @     |       Underrun/Overrun Count  = %d (decimal)
1 4  @ 24  |       Read/write Data Err cnt = %d (decimal) @softerr
1 11 !@ 24 |       Remaining Tape Blocks   = %d (decimal) @remaining

1 14 @  8  |       Tracking Retry Counter  = %d (decimal)
1 15 @  8  |       Read/Write Retry        = %d (decimal)
           |         Incremented when 40 consecutive blocks bad on write
           |         Or a gap in block sequence encountered on read

           % 7) Calculations Assuming a 2.3 GB Tape
                maxtape := 2292769
                * Compute tape used and tape used as fraction
                tapeused := maxtape - remaining
                perused := ( tapeused * 100) / maxtape
                perfrac := ((( tapeused * 100) % maxtape ) * 100) / maxtape

                * Compute soft err rate as percent of tape used
                 ~if( tapeused > 0 )
                    pererrs := (softerr * 100 )/ tapeused
                    fractp  := (((softerr * 100) % tapeused) * 100) / tapeused
                 ~else
                    pererrs := 0
                    fractp  := 0
                 ~endif
           %    Computations:
           %      maxtape = %d ~maxtape
           %      tapeused = maxtape - remaining = %d - %d = %d ~maxtape remaining tapeused


           %      %d blocks or (%d.%.2d %%) of the tape used \ ~tapeused perused perfrac
           %had %d Soft errors ~softerr
           %      Error Percentage = %d.%.2d ~pererrs fractp

~if (read_cmnd)
     ~if (pererrs >= 1)
           %      Calculated error percentage exceeds max (1%%) allowed on read
     ~endif
~elseif (write_cmnd)
     ~if (pererrs >= 1)
           %      Calculated error percentage exceeds max (2%%) allowed on write
     ~endif
~elseif (request_sense_cmnd)
     ~if (pererrs >= 1)
           %%     Soft errors likely exceed max allowed
            %     Soft errors must be < 1%% on a read and < 2%% on a write
     ~endif
~endif


!8mm5gb

* Sense information for 8mm tape drives
%                5.0Gb 8mm Tape Drive Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_tape 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @     | 6) Error Status Information
0 20       |    a) Error Code status (0x%.2x)

                  0 20  ^ 80   $valid_ibytes
0 20 ^ 70  |       Error information is associated with current cmd
0 20 ^ 71  |       Error information is associated with a previous cmd
                  0 20  ^ 70   @this_cmnd
                  0 20  ^ 71   @prev_cmnd

~if ( ((! this_cmnd) && (! prev_cmnd)) && ( request_sense_cmnd ))
             %     WARNING:SCSI CMND was request sense, but sense
             %     returned was not valid extended_sense information.
             %     The values will be translated as such but may be wrong
~endif

0 21  @    |    b) Segment Number = 0x%.2x
0 21 != 00 |       Warning Segment Number != 0x00, but documentation says
           |       it should be 0x00.
0 22  @    |    c) EOM/ILI  / Sense Key Info (0x%.2x)
0 22  ^ 80 |       File Mark Detected
0 22  ^ 40 |       EOM -- Logical beginning or end of tape or LEOT
           |              encountered during write
0 22  ^ 20 |       Illegal Length Indicator -- Logical Block len!= requested
1  7  @ 24 |    d) Additional Flags (0x%.6x)
1  7  ^ 80 ||       Power Fail -- Device just completed SCSI Reset or POR
1  7  ^ 40 ||       SCSI Bus Parity Err -- Detected during data phase.
           |        Residual count contains # of blocks not written
           |        (unless variable mode when block is not written)
1  7  ^ 20 ||       Format Buffer Parity Err -- Internal buffer parity err
           |         in write formatter
1  7  ^ 10 ||       Media Error (generally means a retry count exhausted)
           |        One of the following:
           |         --Write retry exhausted (>12 retries or 40 cons bad
           |            blocks and reposition not able to wrt 11 in 5 trcks
           |         --Read data retry exhausted (>10 repositions)
           |         --Write filemark retry exhausted ( > 18 tracks )
           |         --Formatter parity err on data op
1  7  ^ 08 ||       Error counter overflow -- Not an error condition
1  7  ^ 04 ||       Tape Motion Error -- Servo tracking retries > 9
1  7  ^ 02 ||       Tape Not Present
1  7  ^ 01 |       Beginning of Tape (Not an error condition)

1  8  ^ 80 ||       Transfer abort Error -- Device failed to pause data
           |        in preparation for disconnect.
1  8  ^ 40 ||       Tape Mark Detect Err
           |        Dev could not find tape marks in the correct number.
           |        A recoverable error being a Mismatch between # of
           |        filemarks spaced and physical ID block.
           |        Residue indicates number of blocks missing
1  8  ^ 20 |       Write Protect - Cassette is write protected (information)
1  8  ^ 10 ||       File mark error -- Permanent write error on write filemark
           |        ( >18 tracks needed to write at least 1 block/track)
1  8  ^ 08 ||       Under run error -- Can't continue data flow.
           |        Hardware or microcode error possible
1  8  ^ 04 ||       Write Error 1 --  greater than 12 write retries
1  8  ^ 02 ||       Servo System Error -- Servo detected an err.
           |            Will unload cassette
1  8  ^ 01 ||       Formatter Error -- Data formatter detected an error.
1  9  ^ 10 ||       Drive needs cleaning
1  9  ^ 08 |        Drive was cleaned
1  9  ^ 04 |        Physical end of tape detected
1  9  ^ 02 ||       Write Splice Error B -- Blank tape found (tape motion err)
           |        while looking for a new track
1  9  ^ 01 ||       Write Splice Error O -- Overshoot of track on splicing
           |        from existing tracks (tape motion error)


             0 22 @   @sen_k
             sen_k := sen_k & 0x0f
           % 7) Error Code Information
0 22       |    a) Sense Key (0x%.2x) ~sen_k
sen_k = 00 |       No Sense -- There is no specific information.
           |           Sense request may be requested due to flags above.
sen_k = 01 |       Recovered Error --
sen_k = 02 ||       Not Ready -- Cassette not inserted or device not loaded
sen_k = 03 ||       Medium Error -- Unrecovered data error.
            |         Most likely a flaw in the tape or a formatter write
            |         parity error on data operation.
sen_k = 04 ||       Hardware Error --
           |       Power up selftest failed,
           |          internal data parity err detected, servo failure,
           |          sense err or tape cassette failure.
sen_k = 05 ||       Illegal Request -- Illegal parameter detected in cmd
           |          descriptor block or in additional parms supplied as
           |          data.  Can occur from append error or inappropriate result
sen_k = 06 ||       Unit Attention Condition --
           |          Could be due to drive being reset, mode select parms
           |          changed by another initaior or Tape cassette changed
sen_k = 07 ||       Data Protect -- Trying to write to write protected tape
sen_k = 08 ||       Blank Check -- Logical end of media detected
           |          while reading, verifying or spacing.
sen_k = 09 ||       Vendor Unique -- Positioning error
           |          Actual position is unknown or unexpected.
sen_k = 0a ||       Copy Aborted -- Not used
sen_k = 0b ||       Aborted Command -- Device aborted command.
           |          An initiator detected err during command execution,
           |          message reject, or SCSI bus parity error detected by
           |          the drive will cause this condition
sen_k = 0c |       Equal -- Not Used
sen_k = 0d ||       Volume Overflow -- Last write or write filemark cmd
           |          reached physical end of tape and the data or filemark
           |          may remain in the buffer
sen_k = 0e |       Miscompare -- Not Used
sen_k = 0f |       Reserved   -- For Future Use

1 0 @ 16   |    b) Additional Sense Code/Qualifier  (0x%.4x)
1 0 = 00 00  |       No additional sense information
1 0 = 00 01  |       File Mark detected
1 0 = 00 02  ||       End of medium detected
1 0 = 00 04  ||       Beginning of medium detected
1 0 = 00 05  ||       End of data detected
1 0 = 03 02  ||       Excessive write errors
1 0 = 04 00  ||       Not ready, cause unknown
1 0 = 04 01  ||       Not ready, in process of getting ready
1 0 = 08 01  ||       Communication timeout
1 0 = 08 02  ||       Communication parity error
1 0 = 09 00  ||       Track following error
1 0 = 0c 00  ||       Hard write error, retry limited exceeded
1 0 = 10 00  ||       ID CRC error
1 0 = 11 00  ||       Hard read error
1 0 = 11 01  ||       Hard read error, retry limit exceeded
1 0 = 11 02  ||       Error too long to correct
1 0 = 11 03  ||       Multiple read errors
1 0 = 14 00  ||       Media error detected by read
1 0 = 15 00  ||       Random positioning error
1 0 = 15 01  ||       Mechanical positioning error
1 0 = 15 02  ||       Postitioning error dectected by read
1 0 = 1a 00  ||       Invalid parameter list length in CDB
1 0 = 20 00  ||       Illegal operation code
1 0 = 21 00  ||       Logical block address out of range
1 0 = 24 00  ||       Invalid field in CDB
1 0 = 25 00  ||       Logical unit not supported
1 0 = 26 00  ||       Invalid Field in Parameter List
1 0 = 26 01  ||       Parameter not supported
1 0 = 26 02  ||       Parameter value invalid
1 0 = 27 00  ||       Tape is write protected
1 0 = 28 00  ||       Not ready to ready transition, media may have chnged
1 0 = 29 00  ||       Power on, SCSI, or bus device reset
1 0 = 2a 01  ||       Mode select parameters have changed
1 0 = 30 00  ||       Incompatible medium installed
1 0 = 30 01  ||       Cannot read medium, Unknown format
1 0 = 30 02  ||       Cannot read medium, Incompatible format
1 0 = 31 00  ||       Medium format is corrupted
1 0 = 3a 00  ||       Medimu is not present
1 0 = 3b 02  ||       Tape postion error at end of medium
1 0 = 3d 00  ||       Invalid bits set in Identify Message
1 0 = 3f 01  ||       New microcode was loaded
1 0 = 43 00  ||       Message error
1 0 = 44 00  ||       Internal target failure
1 0 = 47 00  ||       SCSI parity error
1 0 = 48 00  ||       Initiator detected error message received
1 0 = 49 00  ||       Invalid message error
1 0 = 4e 00  ||       Overlapped commands attempted
1 0 = 50 01  ||       Write append position error
1 0 = 53 02  ||       Media removal prevented
1 0 = 5a 01  ||       Operator medium removal request
1 0 = 70 00  ||       Compressed/uncompressed boundary
1 0 = 70 10  ||       Uncompressed/compressed boundary
1 0 = 81 00  ||       Command has a variable/fixed mode mismatch
1 0 = 84 00  ||       Tried to change mode parameter while away from LBOT

01 16 @    |    c) Fault Symptom Code               (0x%.2x)
01 16 = 02   ||       Invalid position for WRITE
01 16 = 03   ||       Tape is write protected for WRITE
01 16 = 04   ||       LEOT encountered on current WRITE
01 16 = 05   ||       Operation has aborted (as requested)
01 16 = 06   ||       LEOT encountered on the last WRITE
01 16 = 08   ||       Compression data integrity check failed.
01 16 = 09   ||       Detected LEOT during READ
01 16 = 0a   ||       Length mismatch on READ
01 16 = 0b   ||       Uncorrectable block on READ
01 16 = 0c   ||       EOD encountered on READ
01 16 = 0d   ||       Filemark encountered during a READ
01 16 = 0e   ||       Illegal condition for READ
01 16 = 0f   ||       READ issued at blank tape
01 16 = 10   ||       READ operation has aborted (as requested)
01 16 = 11   ||       Too many permanent READ errors, cannot sync
01 16 = 14   ||       PEOT encountered on READ
01 16 = 15   ||       Bad filemark encountered during a READ
01 16 = 16   ||       Medium error detected during a READ
01 16 = 17   ||       Hardware error during a READ
01 16 = 18   ||       READ decompression failed - HW error
01 16 = 19   ||       READ cdecompression CRC failed.
01 16 = 1a   ||       Hit compressed -> noncompressed boundary.
01 16 = 1b   ||       Hit non-compressed -> compressed boundary.
01 16 = 1c   ||       Unknown or incompatible format
01 16 = 26   ||       Not at a legal place to Write File Mark
01 16 = 27   ||       Tape is write protected for Write File Mark
01 16 = 28   ||       LEOT encountered during Write File Mark
01 16 = 32   ||       Filemark detected during SPACE/LOCATE
01 16 = 33   ||       EOD encountered on SPACE/LOCATE
01 16 = 34   ||       PEOT encountered on SPACE/LOCATE
01 16 = 35   ||       PBOT encountered on SPACE/LOCATE
01 16 = 36   ||       Format error during a SPACE/LOCATE
01 16 = 37   ||       Uncorrectable block during a SPACE/LOCATE
01 16 = 38   ||       Medium error during a SPACE/LOCATE
01 16 = 3a   ||       Bad filemark during SPACE (2.3 GB mode only)
01 16 = 3b   ||       SPACE/LOCATE has aborted (as requested)
01 16 = 3d   ||       SPACE/LCOATE has aborted (As requested)
01 16 = 4b   ||       Illegal position for ERASE
01 16 = 4c   ||       Tape is write protected for ERASE
01 16 = 4e   ||       ERASE has aborted (as requested)
01 16 = 58   ||       Hardware error during SEND DIAGNOSTIC
01 16 = 5a   ||       Hardware error during SEND DIAGNOSTIC
01 16 = 61   ||       Header in wrong format when loading microcode
01 16 = 62   ||       Servo load image is invalid
01 16 = 63   ||       Bank 0 of control load image is not valid
01 16 = 64   ||       Bank 1 of control load image is not valid
01 16 = 65   ||       EEPROM load image is not valid
01 16 = 66   ||       New code load won't run with old boot code
01 16 = 67   ||       Cannot program one of the memories
01 16 = 68   ||       Hardware error while loading new firmware
01 16 = 69   ||       CRC in load image was not correct
01 16 = 6d   ||       Read buffer command failed.
01 16 = 8c   ||       Software hang, system is in an unknown state.
01 16 = 95   ||       WRITE failure after retry limit exceeded
01 16 = 96   ||       Write filemark failure after retry limit exceeded
01 16 = 97   ||       Write EOD failure after retry limit exceeded
01 16 = 98   ||       Fill error, invalid BRT
01 16 = 99   ||       Fill error, buffer empty
01 16 = 9a   ||       Deformatter intrp timeout on search
01 16 = 9c   ||       Formatter intrp timeout on write
01 16 = 9d   ||       Permanent write error, write recovery fail
01 16 = 9e   ||       Permanent write error, rewrite threshild
01 16 = 9f   ||       Servo zone readback check failure
01 16 = a1   ||       Head sync error during WRITE
01 16 = a2   ||       Underrun error during WRITE
01 16 = a3   ||       IPORT WRITE buffer parity error
01 16 = a4   ||       DPORT WRITE parity error
01 16 = a5   ||       PPORT WRITE parity error
01 16 = a6   ||       IPORT READ parity error
01 16 = a7   ||       DPORT READ parity error
01 16 = a8   ||       PPORT READ parity error
01 16 = ab   ||       Servo timed out
01 16 = ac   ||       Servo software error
01 16 = ad   ||       Servo hardware error
01 16 = ae   ||       Not tracking
01 16 = af   ||       EOT encountered during a motion command
01 16 = b0   ||       Not tracking -- loss of PLL
01 16 = b3   ||       LBOT WRITE failure
01 16 = b4   ||       LBOT ATM write failure
01 16 = b5   ||       Read manager could not read LBOT
01 16 = b6   ||       EOT encountered during buffer flush
01 16 = c0   ||       Power-on reset occurred.
01 16 = c1   ||       Tape may have been changed
01 16 = c2   ||       Mode Select parameters have changed
01 16 = c3   ||       New microcode was loaded
01 16 = c4   ||       Operator requested media removal
01 16 = c5   ||       Incompatible media was rejected
01 16 = c6   ||       Not ready, cause not known.
01 16 = c7   ||       Not readym, in process of becoming ready
01 16 = c9   ||       Command requires a tape and none is loaded
01 16 = cc   ||       Parameter list length error in CDB
01 16 = cd   ||       Illegal Operation Code
01 16 = ce   ||       Invalid field or reserved bits set in CDB
01 16 = cf   ||       This LUN is not supported
01 16 = d0   ||       Invalid field in Parameter list (Mode Data)
01 16 = d1   ||       Illegal bit set in Indentify Message
01 16 = d2   ||       Media removal is prevented
01 16 = d3   ||       Command has mode mismatch (variable/fixed)
01 16 = d4   ||       Illegal transfer length in CDB
01 16 = d6   ||       Tried to change Mode Parms and not at LBOT
01 16 = d7   ||       Can't read medium, incompatible format
01 16 = d8   ||       Overlapped commands attempted, bad ITL nexus.
01 16 = d9   ||       Logical block out of range
01 16 = da   ||       Illegal bits set in ID message
01 16 = e0   ||       Aborted in CDB phase, parity or other error
01 16 = e1   ||       Aborted prior to data phase, bad message
01 16 = e2   ||       Aborted in data phase, init. detected error
01 16 = e3   ||       Aborted in data phase, bad message
01 16 = e4   ||       Aborted after data phase, bad message
01 16 = e5   ||       Aborted after data phase, other error
01 16 = e6   ||       ABORT caused by SCSI Bus Parity Error
01 16 = e7   ||       ABORT sent by initiator has been completed.
01 16 = e8   ||       The tape drive requires cleaning
01 16 = e9   ||       The tape drive has been cleaned.
01 16 = ea   ||       Invalid mode (2.3 GB) for data compression
01 16 = eb   ||       Download in progress
01 16 = fa   ||       Serial number invalid or blank
01 16 = fc   ||       Head sync value in EEPROM out of range
01 16 = fd   ||       EEPROM contains meaningless info

           % 8) Counters and Other Numeric Information
0 23 !@ 32 |    Residue                 = %d (decimal)
           |      (Number of blocks not transfered)
0 27 @     |    Additional Sense Length = %d (decimal)
0 31 @     |    Underrun/Overrun Count  = %d (decimal)
1 4  @ 24  |    Read/write Data Err cnt = %d (decimal) @softerr
1 11 !@ 24 |    Remaining Tape Blocks   = %d (decimal) @remaining

1 14 @  8  |    Tracking Retry Counter  = %d (decimal)
1 15 @  8  |    Read/Write Retry        = %d (decimal)
           |      Incremented when 40 consecutive blocks bad on write
           |      Or a gap in block sequence encountered on read

           % 9) Calculations Assuming a 5.0 GB Tape
                 maxtape := 4827968
                 * Compute tape used and tape used as fraction
                 tapeused := maxtape - remaining
                 perused := ( tapeused * 100) / maxtape
                 perfrac := ((( tapeused * 100) % maxtape ) * 100) / maxtape

                 * Compute soft err rate as percent of tape used
                 ~if( tapeused > 0 )
                    pererrs := (softerr * 100 )/ tapeused
                    fractp  := (((softerr * 100) % tapeused) * 100) / tapeused
                 ~else
                    pererrs := 0
                    fractp  := 0
                 ~endif
           %    Computations:
           %      maxtape = %d ~maxtape
           %      tapeused = maxtape - remaining = %d - %d = %d ~maxtape remaining tapeused

           %      %d blocks or (%d.%.2d %%) of the tape used \ ~tapeused perused perfrac
           %had %d Soft errors ~softerr
           %      Error Percentage = %d.%.2d ~pererrs fractp

           ~if (read_cmnd)
                ~if (pererrs >= 1)
                      %      Calculated error percentage exceeds max (1%%) allowed on read
                ~endif
           ~elseif (write_cmnd)
                ~if (pererrs >= 1)
                      %      Calculated error percentage exceeds max (2%%) allowed on write
                ~endif
           ~elseif (request_sense_cmnd)
                ~if (pererrs >= 1)
                     %%      Soft errors likely exceed max allowed
                      %      Soft errors must be < 1%% on a read and < 2%% on a write
                ~endif
           ~endif

           * 10) Calculations Assuming a 2.3 GB Tape
                 maxtape := 2293760
                 * Compute tape used and tape used as fraction
                 tapeused := maxtape - remaining
                 perused := ( tapeused * 100) / maxtape
                 perfrac := ((( tapeused * 100) % maxtape ) * 100) / maxtape

                 * Compute soft err rate as percent of tape used
                 ~if( tapeused > 0 )
                    pererrs := (softerr * 100 )/ tapeused
                    fractp  := (((softerr * 100) % tapeused) * 100) / tapeused
                 ~else
                    pererrs := 0
                    fractp  := 0
                 ~endif
           ~if( tapeused > 0 )           
                 %10) Calculations Assuming a 2.3 GB Tape
                 %    Computations:
                 %      maxtape = %d ~maxtape
                 %      tapeused = maxtape - remaining = %d - %d = %d ~maxtape remaining tapeused

                 %      %d blocks or (%d.%.2d %%) of the tape used \ ~tapeused perused perfrac
                 %had %d Soft errors ~softerr
                 %      Error Percentage = %d.%.2d ~pererrs fractp
                 ~if (read_cmnd)
                    ~if (pererrs >= 1)
                       %      Calculated error percentage exceeds max (1%%) allowed on read
                    ~endif
                 ~elseif (write_cmnd)
                      ~if (pererrs >= 1)
                            %      Calculated error percentage exceeds max (2%%) allowed on write
                      ~endif
                 ~elseif (request_sense_cmnd)
                      ~if (pererrs >= 1)
                           %%      Soft errors likely exceed max allowed
                            %      Soft errors must be < 1%% on a read and < 2%% on a write
                      ~endif
                 ~endif
           ~endif


!525mb
* Sense information for 525mb tape drives
%                     525mb Tape Drive Sense Data Analysis
%
#set is_525mb 1
#set is_scsi_tape 1
#include 525_1200_tape


!1200mb-c
* Sense information for 1200mb tape drives (-c stands for Cartridge)
%                     1200mb Tape Drive Sense Data Analysis
%
#set is_1200mbc 1
#set is_scsi_tape 1
#include 525_1200_tape

!525_1200_tape
* Sense information for the 525mb and 1.2gb tape drives.

* Include general SCSI information
#include scsi_gen

* Local Variables
#define valid_info
#define sen_key

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field is set.
0 20  ^ 80  ^valid_info

0 22 @   @sen_key
sen_key := sen_key & 0x0f

0 22  @       |    a) Flags / Sense Key Info (0x%.2x)
0 22  ^ 80    |       FM - File Mark Detected
0 22  ^ 40    |       EOM - End of Media Detected
0 22  ^ 20    |       ILI - Illegal Length Indicator -- Requested logical block
              |             length does not match logical block length on media.
              %       Sense Key (0x%.2x) ~sen_key

sen_key = 00  |       No Sense - There is no specific information.
              |         Sense request may be requested due to flags above.
sen_key = 01 ||       Recovered Error - Occurred on the read or write operation
              |         on the tape. Only occurs if the PER bit of the Error
              |         Recovery Page in the Mode Select command is set to 1.
sen_key = 02 ||       Not Ready - Cassette not inserted or device not loaded.
sen_key = 03 ||       Media Error - Unrecoverable data error occurred.
              |         No record found, bad block found, or cartridge failures.
sen_key = 04 ||       Hardware Error - Power up self-test failed.
              |         Internal parity error detected, servo failure, component failures.
              |         Some Tape Cartridge failures may cause the error condition.
sen_key = 05 ||       Illegal Request - Illegal parameter detected.
              |         Occurred in the Command Descriptor Block or in additional parameters
              |         supplied as data. Can occur from append error or inappropriate request.
sen_key = 06 ||       Unit Attention
              |         Indicates that the cartridge has been changed and a Load command given
              |         or the drive may have been reset (by the bus device reset message)
              |         since the last issued command.
sen_key = 07 ||       Data Protect - Trying to write to write protected tape.
sen_key = 08 ||       Blank Check - Logical end of partition is detected.
sen_key = 0b ||       Aborted Command - Device aborted command.
              |         The initiator may recover by trying the command again. SCSI bus parity
              |         errors will cause this error condition.
sen_key = 0d ||       Volume Overflow
              |         This condition can occur if additional data blocks are appended after
              |         the drive has reported EOM and there is not sufficient space left.
sen_key = 0e ||       Miscompare - Used by the Verify command.
              |         Indicates that source data did not match the data read from the tape.

              %    b) Counters and Other Numeric Information

~if (valid_info)
  0 23 !@ 32  |       Residue = %d (decimal).
~else
              %       Information field does not contain valid information.
~endif

0 27 @        |       Additional Sense Length = %d (decimal)
1 2  @ 24     |       Block Counter = %d (decimal)
1 5  @ 16     |       File Mark Counter = %d (decimal)
1 7  @ 16     |       Underrun Counter = %d (decimal)
1 9  @ 16     |       Recoverable Error Counter = %d (decimal)
1 11 @        |       Current Track ID = %d (decimal)
1 13 @ 16     |       ECC Correction Counter = %d (decimal)

1 1  @        |    c) Extended Error Code (EXERCD) = 0x%.2x
1 1  = 01    ||       INVALID WRITE AFTER READ
              |         Write command has been issued when LEOP was not detected in the
              |         preceeding READ command.
1 1  = 02    ||       INVALID FIELD IN CDB
              |         Command block has an invalid field or unsupported LUN.
1 1  = 03    ||       INVALID FIELD IN PARM
              |         Parameter field has an invalid field or unsupported parameter.
1 1  = 04    ||       SCSI PARITY ERROR
              |         Parity error detected in CDB, data, message, or parameters.
1 1  = 06    ||       CAPSTAN SERVO ERROR
              |         Error in drive motor or cartridge slow, fast, or stuck.
1 1  = 08    ||       COMPARE ERROR ON VERIFY - Data miscompare during Verify.
1 1  = 0b    ||       FILEMARK DETECTED - During a read type operation.
1 1  = 0c    ||       HEAD SERVO ERROR
              |         No tape edge found during write. Drive hardware error or media too
              |         worn detect signal at tape edge.
1 1  = 0d    ||       INVALID COMMAND OP CODE
1 1  = 0f    ||       ILLEGAL LENGTH - A transfer length mismatch occurred during a Read.
1 1  = 12    ||       NO CARTRIDGE PRESENT - Cartridge was removed without doing an Unload cmd.
1 1  = 13    ||       CARTRIDGE NOT LOADED
              |         The drive has not been logically loaded with a Load command or
              |         autoload function.
1 1  = 15    ||       NO DATA FOUND DETECTED
              |         No data detected on tape during read or space operation.
1 1  = 16    ||       COMMAND OUT OF SEQUENCE
              |         Read command was issued after a Write command. Recover Buffered Data
              |         command issued after non-Hard Write error.
1 1  = 17    ||       LEOP DURING READ/SPACE - Logical end of partition detected.
1 1  = 18    ||       PEOP DURING READ/SPACE - Physical end of partition detected.
1 1  = 19    ||       RESERVATION CONFLICT - Drive reserved for another intiator.
1 1  = 1a    ||       SENSE ERROR
              |         Sensing problem with hardware or tape has holes or scratches to allow
              |         light to come through.
1 1  = 1b    ||       TAPE RUNNOUT
              |         Drive has detected the ends of the tape without detecting load point
              |         or early warning holes. Tape may be run off the spool.

~if (is_525mb)
  1 1 = 1c   ||       UNIT ATTENTION
~endif

1 1  = 1d    ||       PEW DURING WRITE
              |         Tape has past Psuedo Early Warning on the last track during a Write
              |         command.
1 1  = 1e    ||       WRITE TO PEOT
              |         Tape has reached physical end of partition on last track during a
              |         Write command.
1 1  = 1f    ||       WRITE PROTECTED.
1 1  = 20    ||       WRITE RETRIES EXHAUSTED.
1 1  = 22    ||       READ RETRIES EXHAUSTED.
1 1  = 24    ||       ILLEGAL TERMINATION OF WRITE
              |         Read error caused if the was terminated incorrectly during the Write
              |         operation.  All available data was read.
1 1  = 36    ||       INT RAM ERROR - Error detected by selftest.
              |         RAM error or Illegal Parameter error.
1 1  = 38    ||       BUFFER ERROR
              |         Errors occurred with internal data buffer, possible parity error.
              |         Buffer error during selftest. DMA counter miscompares.
1 1  = 39    ||       DRIVE CNTRL ERROR - Detected by selftest.
1 1  = 3a    ||       EEPROM ERROR - Detected by selftest.
1 1  = 3b    ||       EPROM ERROR - Detected by selftest.
1 1  = 3c    ||       EXT RAM ERROR - Detected by selftest.
1 1  = 3d    ||       SCSI CNTRL ERROR - Detected by selftest.
1 1  = 42    ||       PBOP DURING SPACE - Physical beginning of partition detected.
1 1  = 45    ||       WRITE APPEND ERROR
              |         Write error while appending data, possible uncompleted frame.
1 1  = 4a    ||       RECOVER END OF BUFFER
              |         End of buffer was detected before all requested data was transferred.
1 1  = 4b    ||       LEOP AFTER EW READ/SPACE
              |         Logical end of partition was detected after the Psuedo Early Warning
              |         point during a read or space operation.
1 1  = 50    ||       INCOMPATIBLE MEDIA TYPE
              |         Attempt to use wrong media for a selected format. This condition can
              |         occur on both a read or write operation to tape.
1 1  = 51    ||       INCOMPATIBLE TAPE FORMAT
              |         Attempted to utilize tape format which is different than what is
              |         recorded on the tape. This error condition will occur if you append
              |         data to the end of the data area in a format that is different than
              |         what is currently on the tape.
1 1  = 52    ||       MODE PARAMETERS CHANGED
              |         Mode Select parameters have changed since the drive was last used by
              |         this indicator.
1 1  = 53    ||       PARM LENGTH ERROR - Parameter block length error.
1 1  = 54    ||       SPACE REVERSE ERROR
              |         An unrecoverable read error while spacing reverse. Tape position is
              |         unknown.
1 1  = 55    ||       CARTRIDGE REMOVED - Removed after command was accepted.
1 1  = 56    ||       OVERLAPPED COMMANDS - Commands attempted from multiple initiators.
1 1  = 57    ||       RECOVERED ERROR - Occurred on a read or write operation.
              |         This will occur when PER bit in Mode Select command is set.
1 1  = 58    ||       EDC CONTROLLER ERROR - Selftest EDC Controller error.
1 1  = 59    ||       REWIND ERROR - Selftest rewind error.
1 1  = 5a    ||       WIND ERROR - Selftest wind error.
1 1  = 5b    ||       CPU ERROR - Selftest CPU error.
1 1  = 5c    ||       WRITE ERROR - Selftest Write error.
1 1  = 5d    ||       READ ERROR - Selftest Read error.
1 1  = 5f    ||       WRITE/ERASE FAULT - Wrt or erase signal active at an inappropriate time.

~if (is_1200mbc)

  1 1  = 60  ||       UNKNOWN TAPE FORMAT
              |         Attempted to utilize a tape format which is recorded in an Unknown
              |         Format. This error condition will only occur while attempting to read
              |         the tape.
  1 1  = 61  ||       UNIT ATTENTION: POWER UP
              |         The drive has just completed a power up condition.
  1 1  = 62  ||       UNIT ATTENTION: SCSI RESET
              |         The drive has just completed a SCSI RESET operation.
  1 1  = 63  ||       UNIT ATTENTION: CODE UPDATE
              |         The drive microcode level has been changed since last used by
              |         this Initiator.
  1 1  = 64  ||       UNIT ATTENTION: TAPED CHANGED
              |         The tape cartridge has been changed by the operator.
  1 1  = 65  ||       INVALID MICROCODE LOAD
              |         The device has detected an invalid microcode load as received by
              |         the host.
  1 1  = 66  ||       RETENTION DURING DOWNLOAD
              |         Tape retensioning in progress while a Write Buffer Mode 5 command
              |         request was received by the drive.
~endif

              %    d) Other information
1 12 ^ 1     ||       The drive needs to be cleaned.
1 15 @        |       Internal Drive Status = 0x%.2x = %d (decimal)


!4mm2gb2
#include 4mm2gb

!4mm2gb
* Sense information for 2gb 4mm tape drives
%                     2gb 4mm Tape Drive Sense Data Analysis
%
#set is_4mm2gb 1
#set is_scsi_tape 1
#include 4mm_drives

!4mm4gb2
#include 4mm4gb

!4mm4gb
* Sense information for 4gb 4mm tape drives
%                     4gb 4mm Tape Drive Sense Data Analysis
%
#set is_4mm4gb 1
#set is_scsi_tape 1
#include 4mm_drives

!4mm_drives
* Include General SCSI Decode Information
#include scsi_gen

#define is_4mm2gb
#define is_4mm4gb

* Local Variables
#define valid_info
#define e_code
#define sen_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define ASC
#define ASCQ
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field is set.
0 20  ^ 80  ^valid_info

0 20 @ @temp_byte
e_code := temp_byte & 0x7f

0 22 @   @sen_key
sen_key := sen_key & 0x0f

e_code = 70   |    a) Error information is associated with current command.
e_code = 71   |    a) Error information is associated with previous command.


0 22  @       |    b) Flags / Sense Key Info (0x%.2x)
0 22  ^ 80    |       FMK - File Mark Detected
0 22  ^ 40    |       EOM - End of Media Detected
0 22  ^ 20    |       ILI - Illegal Length Indicator -- Requested logical block
              |             length does not match logical block length on media.
              %       Sense Key (0x%.2x) ~sen_key

sen_key = 00  |       No Sense - There is no specific information.
              |         This could be the case if sense is requested because of
              |         file mark, end of media detected, or illegal length.
sen_key = 01 ||       Recovered Error - Occurred on the read or write operation.
              |         Only reported if the PER bit in the Mode Select command is set enabled.
sen_key = 02 ||       Not Ready - Cassette not inserted or device not loaded.
sen_key = 03 ||       Media Error - Unrecoverable data error occurred.
              |         Most likely a flaw in the tape or a formatter write parity
              |         error on a data operation.
sen_key = 04 ||       Hardware Error
              |         Power up self-test failed, internal parity error detected, servo
              |         failure, sensor error or tape cartridge failure.
sen_key = 05 ||       Illegal Request
              |         Illegal parameter detected in the Command Descriptor Block or in
              |         additional parameters supplied as data. Can occur from append error or
              |         inappropriate request.
sen_key = 06 ||       Unit Attention
sen_key = 07 ||       Data Protect
              |         Requested to write a write protected cartridge or to a non-data grade
              |         data cartidge, command was not performed.
sen_key = 08 ||       Blank Check - Logical end of media (end of recorded area)
              |         is detected while reading, verifying, or spacing.
sen_key = 0b ||       Aborted Command - Device aborted command.
              |         An Initiator Detected Error during command execution, Message Reject
              |         or SCSI Bus Parity Error detected by the device during command or
              |         data out phase will cause this.
sen_key = 0d ||       Volume Overflow
              |         This condition indicates that the last Write or Write Filemark command
              |         reached Physical EOT & that data or filemarks may remain in the buffer.
sen_key = 0e ||       Miscompare

              %    c) Other Numeric Information

~if (valid_info)
  0 23 !@ 32  |       Residue = %d (decimal).
~else
              %       Information field does not contain valid information.
~endif

0 27 @        |       Additional Sense Length = %d (decimal)
1 2  @        |       FRU Code = 0x%.2x
1 2  = 00     |         No failure
1 2  = 01    ||         Controller PCA failure
1 2  = 02    ||         Mechanism failure

1 0  @ 16     |    d) Additional Error Codes (ASC/ASCQ) = 0x%.4x

1 0  @   @ASC
1 1  @   @ASCQ

1 0 = 0000    |       NO SENSE - No additional sense information.
1 0 = 0001   ||       FILEMARK DETECTED - during Read or Space.
1 0 = 0002   ||       END-OF-PARTITION/MEDIUM DETECTED
              |         Encountered unepectedly during a Read, Space, Write, or Write FM.
1 0 = 0003   ||       SETMARK DETECTED - during a Read or Space.
1 0 = 0004   ||       BEGINNING-OF-PARTITION/MEDIUM DETECTED
              |         A Space command encountered BOP/BOT unexpectedly.
1 0 = 0005   ||       END OF DATA DETECTED - Read or Space encountered end of data.
1 0 = 0302   ||       EXCESSIVE WRITE ERRORS
              |         Media error.
1 0 = 0400   ||       NOT READY, TAPE BEING LOADED
              |         Load or Unload is received while an Unload with Immediate is occurring
              |         or the Unload button was pressed.
1 0 = 0401   ||       NOT READY, TAPE BEING LOADED
              |         Media access cmd received while a Load with Immediate or autoload was
              |         occurring.
1 0 = 0402   ||       NOT READY, INITIALIZING REQUIRED
              |         Command is received and the tape is logically Unloaded. Load command
              |         is required.
1 0 = 0403   ||       NOT READY, PROCESS OF BECOMING READY
              |         Media access cmd is received while an immediate reported load (or
              |         autoload) is occurring.
1 0 = 0800   ||       COMMUNICATIONS FAILURE
              |         An err occurs at the mechanism interface. Applicable only to self-test.
1 0 = 0900   ||       TRACK FOLLOWING ERROR - Read fails due to a fatal positioning error.
              |         Could be tape or drive problem.
1 0 = 0c00   ||       WRITE ERROR - Failed to write data or tape marks.
              |         Could be a tape or drive error.
1 0 = 1100   ||       UNRECOVERED READ ERROR - Failed to read tape.
              |         Most likely bad tape but could be drive error.
1 0 = 1403   ||       END OF DATA NOT FOUND - Blank tape while reading.
              |         Probably corrupted format.
1 0 = 1500   ||       MECHANICAL POSITIONING ERROR
              |         A mechanical retry successfully completed the media access command.
1 0 = 1501   ||       MECHANICAL POSITIONING ERROR
              |         Hardware Error.
1 0 = 1700   ||       RECOVERED DATA WITH NO ERROR CORRECTION
              |         A soft error occurred but completed the media access command.
1 0 = 1701   ||       RECOVERED DATA WITH RETRIES
              |         A soft error retry successfully completed the Write or Write FM cmd.
1 0 = 1800   ||       RECOVERED DATA WITH ERROR CORRECTION
              |         C3 ECC was used to complete the Read or Space command successfully.
1 0 = 1801   ||       RECOVERED DATA WITH ERROR CORRECTION
              |         Recovered error.
1 0 = 1a00   ||       PARAMETER LIST LENGTH ERROR
              |         A Mode Select parameter list contains an incomplete mode parameter
              |         header, mode block descriptor or mode page.
1 0 = 2000   ||       INVALID COMMAND OPERATION CODE
              |         Drive does not recognize the opcode of the command received.
1 0 = 2100   ||       LOGICAL BLOCK ADDRESS OUT OF RANGE
              |         Error occurs if Logical Block Address is out of range.
1 0 = 2101   ||       INVALID ELEMENT ADDRESS
1 0 = 2400   ||       INVALID FIELD IN CDB
              |         Detected during pre-execution checks of received command.
1 0 = 2500   ||       LOGICAL UNIT NOT SUPPORTED
              |         An identify message is received with LUN not zero.
1 0 = 2600   ||       INVALID FIELD IN PARAMETER LIST
1 0 = 2601   ||       PARAMETER NOT SUPPORTED
1 0 = 2602   ||       PARAMETER VALUE INVALID
1 0 = 2603   ||       THRESHOLD PARAMETERS NOT SUPPORTED
1 0 = 2700   ||       WRITE PROTECTED - Detetected during a Write, Write FM, or Erase commands.
1 0 = 2800   ||       NOT READY TO READY TRANSITION
              |         Set following a tape load to indicate that the tape may have changed.
1 0 = 2900   ||       POWER-ON OR BUS RESET OCCURRED - Hard reset to drive occurred.
              |         Power on reset, SCSI Reset signal or Bus device reset occurred.
1 0 = 2a01   ||       MODE PARAMETERS CHANGED
              |         Set for all hosts following a Mode Select command, other than the
              |         host that sent the command.
1 0 = 3000   ||       INCOMPATIBLE MEDIUM INSTALLED
              |         Set when writing was requested to a non-data grade data cartridge.
1 0 = 3001   ||       UNKNOWN FORMAT
1 0 = 3002   ||       CANNOT READ MEDIA, INCOMPATIBLE FORMAT
              |         Read or Space of unknown tape format, not DDS.
1 0 = 3003   ||       CLEANING CARTRIDGE INSTALLED
1 0 = 3004   ||       DIAGNOSTIC CARTRIDGE INSTALLED
1 0 = 30c3   ||       COMPRESSED DATA FORMAT CORRUPTION
1 0 = 30c4   ||       BLOCK ACCESS TABLE CORRUPTION - RD
1 0 = 30c5   ||       BLOCK ACCESS TABLE CORRUPTION - SP
1 0 = 3100   ||       MEDIA FORMAT CORRUPTED - Incorrect DDS format.
1 0 = 3101   ||       FORMAT COMMAND FAILED
              |         A formatting operation induced via a Mode Select command sending a
              |         Media Partitions page failed.
1 0 = 3300   ||       TAPE LENGTH ERROR
              |         Tape too short to format with requested partition size.
1 0 = 3700   ||       ROUNDED PARAMETER
              |         Mode Select mode parameter was rounded to match drive sensitivity.
1 0 = 3a00   ||       MEDIUM NOT PRESENT
              |         Media access command sent when no media is in the drive.
1 0 = 3b00   ||       SEQUENTIAL POSITIONING ERROR
              |         Current command failed to complete successfully or logical position has
              |         been lost. Tape may be positioned on other side of bad groups of data.
1 0 = 3b01   ||       TAPE POSITIONING ERROR AT BOM
              |         Load or Locate/Mode Sense command fails to read DDS format system
              |         area on tape.
1 0 = 3b08   ||       LOST TAPE POSITION DURING SPACE   
1 0 = 3b0d   ||       DESTINATION ELEMENT FULL
1 0 = 3b0e   ||       SOURCE ELEMENT EMPTY
1 0 = 3d00   ||       INVALID BITS IN IDENTIFY MESSAGE - Illegal identify message.
1 0 = 3e00   ||       LOGICAL UNIT NOT SELF CONFIGURED YET
              |         Power on test and self configuration is not completed. Media access
              |         commands cannot be attempted.
1 0 = 3f01   ||       MICROCODE HAS BEEN CHANGED
              |         Microcode download has successfully been changed.

~if ( is_4mm2gb )
  ~if (ASC == 0x40)
             %%       DIAGNOSTIC FAILURE ON COMPONENT 0x%.2x ~ASCQ
  ~endif
~else
  ~if (ASC == 0x40)
             %%       DIAGNOSTIC TEST FAILED
              %         Selftest detects an error or a command is prohibited because
              %         of a diagnostic failure.
  ~endif
~endif

1 0 = 4300   ||       MESSAGE ERROR
              |         Parity errors on inbound or outbound messages exceeds the limit.
1 0 = 4400   ||       INTERNAL TARGET FAILURE
              |         Internal drive hardware or firmware error that should never occur.
1 0 = 4480   ||       COMPRESSION HARDWARE FAULT
1 0 = 4481   ||       PORT A PARITY ERROR
1 0 = 4482   ||       PORT B PARITY ERROR
1 0 = 4490   ||       ERROR DURING COMPRESSION
1 0 = 4491   ||       PORT B CONTROL PROBLEM
1 0 = 4492   ||       COMPRESSION LSI PAUSE PROBLEM
1 0 = 4493   ||       ERROR DURING BUFFER SWAP
1 0 = 4494   ||       CMPN CHIP NOT PAUSED WHEN EXPECTED
1 0 = 4495   ||       FIFO NOT EMPTY WHEN EXPECTED
1 0 = 4496   ||       CMPN CHIP NOT FLUSHED WHEN EXPECTED
1 0 = 44ad   ||       CAPSTAN SERVO ERROR
1 0 = 44ae   ||       TAPE CYLINDER ERROR
1 0 = 44af   ||       TAPE REEL ERROR
1 0 = 44b0   ||       UNKOWN MECHANICAL ERROR
1 0 = 44b1   ||       THE TAPE IS CUT
1 0 = 44b2   ||       THE TAPE IS JAMMED
1 0 = 44b3   ||       GAIN ADJUSTMENT FAILURE
1 0 = 44b4   ||       TAPE PROCESS INTERNAL ERROR
1 0 = 4500   ||       SELECT OR RESELECT FAILURE
              |         Drive fails to reselect the host to complete an operation.
1 0 = 4700   ||       SCSI PARITY ERROR - Detected by drive in unexpected SCSI state.
              |         This error should be treated as a Firmware error and should not occur.
1 0 = 4800   ||       INITIATOR DETECTED ERROR MESSAGE RECEIVED
              |         The previous phase was invalid.
1 0 = 4900   ||       INVALID MESSAGE ERROR
              |         Host and drive do not recognize each others message.
1 0 = 4a00   ||       COMMAND PHASE ERROR
              |         Too many parity errors occur during an attempted Command phase.
1 0 = 4b00   ||       DATA PHASE ERROR
              |         Too many parity errors occur during the Data-In and Data-Out phases.
1 0 = 4e00   ||       OVERLAPPED COMMANDS ATTEMPTED
              |         Host selects a command when a command is already pending for that host.
1 0 = 5000   ||       WRITE APPEND ERROR - Append area is unreadable.
1 0 = 5100   ||       ERASE FAILURE - Erase cmd fails to erase specified area.
1 0 = 5101   ||       WRITE APPEND POSITION ERROR
              |         Erase command was issued at an invalid location on tape.
1 0 = 5200   ||       CARTRIDGE FAULT
              |         Tape broke or invalid combination of identification holes.
1 0 = 5300   ||       MEDIA LOAD OR EJECT FAILURE.
1 0 = 5301   ||       UNLOAD TAPE FAILURE - Unload operation failed.
1 0 = 5302   ||       MEDIUM REMOVAL PREVENTED
              |         Unload command is received while a tape is present but not loaded and
              |         medium removal has been prevented.
1 0 = 5a01   ||       EJECT BUTTON PUSHED
1 0 = 5a02   ||       OPERATOR SELECTED WRITE PROTECT
              |         A Write operation (data, marks, erase) is requested for a write
              |         protected cartridge.
1 0 = 5b00   ||       LOG EXCEPTION
1 0 = 5b01   ||       THRESHOLD CONDITION MET
1 0 = 5b02   ||       LOG COUNTER AT MAXIMUM

~if (ASC == 0x70)
             %%       DECOMP EXCEPTION SHORT ALGOTIRTH ID 0x%.2x ~ASCQ
              %         During a Read command the unit detected a change of
              %         recorded data item type (record or entity) which it is
              %         not configured to return to the host without issuing a
              %         warning about the change.
~endif

1 0 = 8280   ||       HUMIDITY TOO HIGH
              |         Drive is operating in humidity which exceeds the specified operating
              |         limits.
1 0 = 8281   ||       DRYNESS
              |         Optionally posted to all hosts when the humidity falls to a
              |         permissible level.
1 0 = 8282   ||       DRIVE REQUIRES CLEANING
              |         Or media has a high error rate and should be replaced.
1 0 = 8283   ||       BAD MICROCODE DETECTED - During microcode download process.
1 0 = 9103   ||       GROUP CORRUPTED

              %    e) Sense Key Specific Information

1 3   ^ 80  ^SKSV
* If Sense Key Specific Valid
~if (SKSV)

  1 3 ^ 40       ^CD
  1 3 ^ 8        ^BPV
  1 3 @          @temp_byte
  bit_ptr := (temp_byte & 0x7)
  1 4 @ 16       @field_ptr

  * If in the Command Descriptor Block else Data Parameter
  ~if (CD)
              %       Byte 0x%.8x of the Command Descriptor Block ~field_ptr
  ~else
              %       Byte 0x%.8x of the parameter list ~field_ptr
  ~endif

  * if Field Pointer Value is valid
  ~if (BPV)
              %       Bit %d of the specified byte ~bit_ptr
  ~endif

~else
  1 4 @      ||       Internal Drive Error Code = 0x%.2x

~endif


!9trk
* Sense information for 9 Track tape drives

%                  9 Track Tape Drive Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_tape 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @     | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  ^ 70   $err_class_okay
~if (! err_class_okay)
             %    WARNING Error Status not valid sense return.
             %    Will translate as if it were but results may be wrong
~endif
0 20  = x0   |    Information from a current error
0 20  = x1   |    Information from deferred or already acknowledge error

0 21 != 0    |    WARNING Segment != 0 as expected
0 22 @       | 7) Flags
0 22  ^ 80   |    Current command has a read filemark
0 22  ^ 40   |    EOM (End of tape or beginning of tape)
0 22  ^ 20   ||    ILI -- Illegal length indicator
             |      Requested logical block length != length of data on tape
1  6  ^ 80   |    Command Rejected
1  6  ^ 40   |    Reset Acknowledge
1  6  ^ 20   |    Transfer Abort Acknowledge
1  6  ^ 10   |    Hard Error Encountered
1  6  ^ 08   |    Unexpected EOF/BOT/Runaway
1  6  ^ 04   |    Data Correction Occurred
1  6  ^ 02   |    Immediate Report Message
1  6  ^ 01   |    Transparent Status Message

1  8  ^ 80   |    Beginning of Tape
1  8  ^ 40   |    End of File (EOF)
1  8  ^ 20   |    End of Tape (EOT)
1  8  ^ 10   |    Early End of Tape
1  8  ^ 08   |    10 feet beyond End of Tape
1  8  ^ 04   |    Tape Runaway
1  8  ^ 02   |    Position Lost

0 22  @      |    Sense Key(0x%.2x)
0 22  = x0   |    No Sense            $no_sense
0 22  = x1   ||    Recovered Error    $recovered_error
0 22  = x2   ||    Not Ready -- Media cannot be accessed  $not_ready
             |     Tape may not be online, or tape drive may be performing
             |     another function (diagnostics,rewinding,loading, door open)
0 22  = x3   ||    Medium Error -- Retries exhausted $medium_error
0 22  = x4   ||    Hardware Error -- Hardware failure or media problems $hw_err
0 22  = x5   ||    Illegal Request -- Command or parms not valid $ill_req
0 22  = x6   ||    Unit Attention -- State of tape load/unload not known $ua
0 22  = x7   ||    Data Protect -- Write enable ring not on tape $data_prot
0 22  = x8   ||    Blank Check -- Inter-record gap too large $blank_check
0 22  = x9   |    Unused Sense Key
0 22  = xa   ||    Copy Aborted -- Not implemented
0 22  = xb   ||    Aborted command -- Tape is aborting or is asked to abort cmd
0 22  = xb   $aborted_cmnd
0 22  = xc   |    Equal -- Not implemented
0 22  = xd   ||    Volume Overflow  -- Tape position 10 ft past EOT $overflow
0 22  = xe   |    Miscompare -- Not implemented
0 22  = xf   |    Reserved sense key

0 23  @ 32   |  8) Information Bytes( 0x%.8x)
             %  9) Additional Sense Information (Command Specific)
0 27  @      |     Length of valid additional data = %d
0 28  @ 16   |      Additional Data 0x%.4x \
0 30  @ 16   | 0x%.4x \
1 0   @ 16   | 0x%.4x

             % 10) Decoded Error Information
~if no_sense
1 6 = 00 00   |    No Sense
1 6 = 00 02   |    End of Media (EOM) detected
1 6 = 00 04   |    Beginning of Media (EOM) detected (back space into BOT)
1 6 = 00 88   ||    Transfer buffer empty -- Cant retrieve record from buffer
~elseif recovered_error
1 6 = 00 00   ||   Recovered Error
~elseif not_ready
1 6 = 00 00   ||    Not Ready
1 6 = 04 00   ||    Tape Unit Not Ready
1 6 = 04 01   ||    Volume Not Ready
1 6 = 04 02   ||    Volume Not Installed
1 6 = 04 80   ||    Tape unit is not Offline
1 6 = 04 81   ||    Tape is loaded preventing access to test
1 6 = 04 82   ||    Front door or top cover is open
1 6 = 04 83   ||    Tape unit is in Diagnostic Mode
1 6 = 04 84   ||    Tape unit is not in Diagnostic Mode
1 6 = 31 00   ||    Illegal format on write
1 6 = 38 0C   ||    Not at Beginning of Tape (BOT) for a Write ID
~elseif medium_error
1 6 = 00 00   ||     Hardware Error
1 6 = 11 00   ||     Unrecovered Read
1 6 = 1f 00   ||     Unrecovered Write
1 6 = 30 00   ||     Cannot read from tape (unknown format)
1 6 = 89 00   ||     Tape positioning or servo error
~elseif hw_err
1 6 = 00 00   ||     Hardware Error
1 6 = 08 00   ||     LUN communication failure
1 6 = 40 00   ||     Diagnostic failed
1 6 = 42 00   ||     Power-on failure
1 6 = 44 00   ||     Internal error on controller
1 6 = 55 00   ||     Tapue Unit reported eroror
1 6 = 89 00   ||     Tape positioning or servo error
1 6 = 8a 00   ||     Buffer controller error
1 6 = 44 97   ||     Data written to the buffer did not match requested length
1 6 = 91 00   ||     Erroneous DPR reset
1 6 = 92 00   ||     Erroneous SPIFI reset
1 6 = 11 00   ||     Unrecognized read
1 6 = 1f 00   ||     Unrecognized write
1 6 = 47 00   ||     Parity Error detected
~elseif ill_req
1 6 = 00 00   ||     Illegal request
1 6 = 20 00   ||     Illegal command during copy
1 6 = 20 01   ||     Illegal function for device type
1 6 = 20 02   ||     Copy
1 6 = 20 03   ||     Copy
1 6 = 20 04   ||     Copy
1 6 = 25 00   ||     Illegal LUN
1 6 = 26 00   ||     Illegal field in parameter list
1 6 = 26 06   ||     Tape Density error
1 6 = 26 83   ||     Verify immediate is not supported
1 6 = 26 85   ||     Request test is not remotely accessible
1 6 = 26 86   ||     Illegal Mode Select/Sense Length
1 6 = 26 87   ||     Requested write length exceeds maximum
1 6 = 26 88   ||     Verify Immediate is not supported
1 6 = 26 90   ||     Bad page selected in Read Log
1 6 = 26 94   ||     Unsupported Mode Sense/Select Page
1 6 = 34 00   ||     Illegal Command Descriptor Block
1 6 = 34 01   ||     Illegal command operation code
1 6 = 34 04   ||     Reserved field used
1 6 = 34 06   ||     Illegal mode select parameter
1 6 = 34 07   ||     Fixed bit set in Variable Mode
1 6 = 34 8e   ||     Byte compare mode not supported
1 6 = 34 8f   ||     SILI bit set while in fixed mode
1 6 = 34 93   ||     Immediate bit set when not in buffered mode
1 6 = 34 95   ||     Illegal mode length
1 6 = 34 96   ||     Illegal field in mode is detected
1 6 = 34 04   ||     Nonzero reserved field
1 6 = 34 f0   ||     Buffer is empty -- cant receive a record from buffer
1 6 = 34 f1   ||     Buffer is full -- cant put a record in buffer
1 6 = 34 f2   ||     Block header illegal for NVRAM load
1 6 = 3f f3   ||     Record length or checksum error on NVRAM load
1 6 = 3f f4   ||     Illegal target ID
1 6 = 40 00   ||     Nested sequencing error
~elseif ua
1 6 = 00 00   ||     Unit attention
1 6 = 28 00   ||     Tape position moved by the operator or tape unloaded
1 6 = 28 80   ||     Tape unit went from online to offline and back but tape not moved
1 6 = 29 00   ||     Power on reset, bus device reset
1 6 = 29 01   ||     Interface only reset
1 6 = 29 86   ||     Test cancelled by reset
~elseif data_prot
1 6 = 00 00   ||     Data Protect
1 6 = 27 00   ||     Write Protected
~elseif blank_check
1 6 = 00 00   ||     Blank Check
1 6 = 00 05   ||     End of Data (EOD)
~elseif aborted_cmnd
1 6 = 00 00   ||     Aborted Command
1 6 = 37 00   ||     Parity error
1 6 = 39 00   ||     Message-out error
1 6 = 39 02   ||     Illegal message caused an abort
1 6 = 4e 00   ||     Protocol or reselection timeout error
1 6 = ff 00   ||     Copy-target went to incorrect phase or sent unexpected status
~elseif overflow
1 6 = xx x0   ||     Volume overflow
~endif
              % 11) Counters
1 9  @ 24     |     Data Record Byte count = %d (decimal)
1 12 @        |                Retry count = %d (decimal)
1 13 @ 16     |       Back reference count = %d (decimal)
1 15 @        | 12) Last Error causing a retry (0x%.2x)
1 15 = 01 |  No tape is loaded
1 15 = 02 |  Tape unit is not online
1 15 = 03 |  Tape unit is not offline
1 15 = 04 |  Tape unit is write protected
1 15 = 05 |  Tape loaded prevents access to test
1 15 = 06 |  Front door or top cover is open
1 15 = 07 |  Controller is currently in the diagnostics mode
1 15 = 08 |  Controller is currently not in the diagnositics mode
1 15 = 09 |  Tape unit not streaming
1 15 = 0a |  Cannot read tape with unidentified format
1 15 = 0b |  Cannot write tape with unidentified format
1 15 = 0d |  Tape already at BOT when backspack cmd issued
1 15 = 0f |  Tape 10 feet past EOT
1 15 = 10 |  Illegal Command received
1 15 = 11 |  Illegal parm for a command
1 15 = 12 |  Illegal TEST or INFO number
1 15 = 13 |  Test not remotely accessible
1 15 = 14 |  Test aborted by reset
1 15 = 15 |  User defined sequence full, cant add test
1 15 = 16 |  Requested density not available
1 15 = 17 |  Illegal processor selected in a test
1 15 = 18 |  Requested write rec length > max
1 15 = 1c |  Buffer is empty, cant retrieve record from buffer
1 15 = 1d |  Buffer full, cant place a record in buffer
1 15 = 1e |  Block header or tape format not valid for NVRAM load
1 15 = 1f |  Record length or checksum error on NVRAM

1 15 = 20 |  Buffer overrun
1 15 = 21 |  Gap detected before end of data on read
1 15 = 22 |  Three or more tracks in error on read
1 15 = 23 |  Two tracks in error on read
1 15 = 24 |  Single track in error on read (NRZI only)
1 15 = 25 |  CRC error on read
1 15 = 26 |  ACRC error on read
1 15 = 28 |  Residual error on read
1 15 = 28 |  Syndrome detected single track in error on read
1 15 = 29 |  Formatter CRC error on read
1 15 = 2a |  Unknown formatter error on read
1 15 = 2b |  Data block timeout
1 15 = 2c |  Block detect error
1 15 = 2d |  End block detected
1 15 = 2e |  Bad gap after ID
1 15 = 2f |  Gap check timeout
1 15 = 30 |  Short gap after block
1 15 = 31 |  Block overrun
1 15 = 32 |  False ID block detected
1 15 = 33 |  Bad tape mark read
1 15 = 34 |  Hitch into a block failed
1 15 = 35 |  Hitch into a gap failed
1 15 = 39 |  NRZI error
1 15 = 3a |  Tracks with gain too low during autocol
1 15 = 3b |  Tracks with gain too high during autocol
1 15 = 3c |  Tracks with gain too low and too high during autocol
1 15 = 3f |  Tape runaway during diagnostic test

1 15 = 40 |  Buffer underrun
1 15 = 41 |  Gap detected before end of data on write
1 15 = 42 |  Three or more tracks in error on write
1 15 = 43 |  Two tracks in error on write
1 15 = 44 |  One track in error on write
1 15 = 45 |  CRC error on write
1 15 = 46 |  ACRC error on write
1 15 = 47 |  Residual error on write
1 15 = 48 |  Syndrome detected single track in error on write
1 15 = 49 |  Formater CRC error on write
1 15 = 4a |  Unkown formatter error on write
1 15 = 4b |  Data block timeout
1 15 = 4c |  Data block detect error
1 15 = 4d |  End data block detect error
1 15 = 4e |  Bad gap after ID
1 15 = 4f |  Gap check timeout
1 15 = 50 |  Erase verify error
1 15 = 51 |  PE density ID detect Error
1 15 = 52 |  PE density ID verify Error
1 15 = 53 |  GCR density ID detect Error
1 15 = 54 |  GCR density ID verify Error
1 15 = 55 |  GCR ARA burst detect  Error
1 15 = 56 |  GCR ARA burst verify  Error
1 15 = 57 |  GCR ARA ID burst detect  Error
1 15 = 58 |  GCR ARA ID burst verify  Error
1 15 = 59 |  Tape mark detect error
1 15 = 5a |  Tape mark verify error
1 15 = 5b |  Bad pregap on write
1 15 = 5c |  Buffer data parity error during write record
1 15 = 5d |  No block detected during write record verify
1 15 = 5e |  No block detected during write tape mark verify
1 15 = 5f |  No block detected during write ID verify

1 15 = 60 |  Tension shutdown
1 15 = 61 |  Tape speed out of specifications
1 15 = 62 |  Tape ramping error
1 15 = 63 |  Servo unresponsive error
1 15 = 6e |  No reel found
1 15 = 6f |  Hub lock failure
1 15 = 70 |  Reel will not seat
1 15 = 71 |  Reel inverted
1 15 = 72 |  Tape stuck to reel
1 15 = 73 |  Tape stuck in path
1 15 = 74 |  Unable to establish tension
1 15 = 75 |  Tape eject timeout
1 15 = 76 |  Door open abort
1 15 = 77 |  Failed to read the ID after a rewind
1 15 = 78 |  No BOT marker detected
1 15 = 79 |  Operator reset abort of tape operation
1 15 = 7a |  Host reset abort of tape operation
1 15 = 7d |  Last block not found
1 15 = 7e |  Gap recapture postion error
1 15 = 7f |  Block recapture postion error

1 15 = 80 |  Reel size detector failure
1 15 = 83 |  Unable to thread tape into tape path
1 15 = 84 |  Open loop motor control error
1 15 = 85 |  Gap timer circuitry check failed

1 15 = a0 |  Interface data parity error
1 15 = a1 |  Tape unit data parity error
1 15 = a2 |  Byte count mismatch on read
1 15 = a3 |  Prior error reject
1 15 = a4 |  Write stoped at EOT
1 15 = a5 |  Zero byte record read or requested
1 15 = a6 |  Final report not valid
1 15 = a7 |  Tape runawau during manual commands
1 15 = a8 |  Tape position synchronization mismatch
1 15 = a9 |  Physical data record too small to deblock
1 15 = aa |  Illegal pointer found during deblocking of physical records
1 15 = ab |  Access table contents were illegal
1 15 = ac |  Access table contents were incomplete
1 15 = ad |  Improper byte ocunt sum of acces table entries
1 15 = bf |  Fatal error encountered

1 15 = c0 |  Command is not supported
1 15 = c2 |  Illegal field in the cmd descriptor block
1 15 = c3 |  Illegal mode select parameter
1 15 = c4 |  Mode length is not on a legal boundary
1 15 = c5 |  Fixed bit set, but tape not in fixed mode
1 15 = c6 |  Microprocessor halted
1 15 = c7 |  Byte compare bit is not supportedd
1 15 = c8 |  Front panel cancel occurred
1 15 = c9 |  The SILI bit is set in fixed mode
1 15 = ca |  A rewind was requested while offline
1 15 = cb |  Check log occurred correctly
1 15 = cc |  Parity error was detected
1 15 = cd |  Illegal log was requested
1 15 = ce |  Power on has occurred
1 15 = cf |  The tape was changed
1 15 = d0 |  A spurious reset occurred
1 15 = d1 |  A spurious SCSI chip interrupt occurred
1 15 = d2 |  Requested write length was too big
1 15 = d3 |  Verify immediate is not supported
1 15 = d4 |  Illegal mesage caused an abort
1 15 = d5 |  Illegal LUN was detected
1 15 = d6 |  A diagnostic failed
1 15 = d7 |  Immediate bit set when not in immediate response mode
1 15 = d8 |  An supported mode page was requested
1 15 = d9 |  An illegal mode length was requested
1 15 = da |  An illegal field in the mode was detectged
1 15 = db |  There were nonzero reserved fields
1 15 = dc |  The data written to buffer did not match requested length
1 15 = dd |  The power on self test failed
1 15 = de |  There was a bus protocol error
1 15 = df |  There was a failure to reselect (timeout)
1 15 = e0 |  Copy lost data reset
1 15 = f1 |  Tape unit went from online to offline to online with no position cng


!3490e

* Sense information for the 3490E autoloading tape drive
%                  3490E Autoloading Tape Drive Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_tape 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @      | 6) Error Status Information
0 20        |    a) Error Code status (0x%.2x)

0 20  ^ 80  $valid_ibytes
0 20 ^ 70   |       Error information is associated with current cmd
0 20 ^ 71   |       Error information is associated with a previous cmd
0 20 ^ 70   @this_cmnd
0 20 ^ 71   @prev_cmnd

~if ( ((! this_cmnd) && (! prev_cmnd)) && ( request_sense_cmnd ))
            %     WARNING:SCSI CMND was request sense, but sense
            %     returned was not valid extended_sense information.
            %     The values will be translated as such but may be wrong
~endif

0 22  @     |    c) EOM/ILI  / Sense Key Info (0x%.2x)
0 22  ^ 80  |       File Mark Detected
0 22  ^ 40  |       EOM -- Logical beginning or end of tape or LEOT
            |              encountered during write
0 22  ^ 20  |       Illegal Length Indicator -- Logical Block len!= requested
0 22 @      @sen_k
            sen_k := sen_k & 0x0f
            %    d) Sense Key (0x%.2x) ~sen_k
0 23 !@ 32  |    e) Information Bytes = 0x%.8x
0 27 @      |    f) Additional Sense Length = %d (decimal)

            % 7) Library Error and Informational Sense
1 10 @      |    a) Sense Byte 0 = 0x%.2x
1 10  ^ 80  |         Command Reject
1 10  ^ 40  |         Intervention Required
1 10  ^ 20  |         Bus Out Check
1 10  ^ 10  |         Equipment Check
1 10  ^ 08  |         Data Check
1 10  ^ 04  |         Overrun
1 10  ^ 02  |         Deferred Unit Check
1 10  ^ 01  |         Assigned Elsewhere
1 11 @      |    b) Sense Byte 1 0x%.2x
1 11  ^ 80  |         Locate Failure
1 11  ^ 40  |         Drive On-line to CU
1 11  ^ 20  |         Reserved
1 11  ^ 10  |         Sequence Error
1 11  ^ 08  |         Beginning of Tape
1 11  ^ 04  |         Write Mode
1 11  ^ 02  |         Write Protect
1 11  ^ 01  |         Not Capable
1 12 @      |    c) Sense Byte 2 0x%.2x
            %       Reporting Channel Path @ch_path
            chpath := chpath & 0xe0
chpath = 20 |         Channel Interface A
chpath = 40 |         Channel Interface B
chpath = 60 |         Channel Interface C
chpath = 80 |         Channel Interface D
            %       Reporting Channel Interface
1 12  ^ 10  |         CU 1
1 12 !^ 10  |         CU 0
            %       Reporting Control Unit
1 12  ^ 80  |         CU 1
1 12 !^ 80  |         CU 0
1 12  ^ 04  |       Automatic Cartridge Loader Active
1 12  ^ 02  |       Tape Synchronous Mode
1 12  ^ 01  |       Tape Positioning

1 13 @      |    d) Error Recovery Action Code (ERA) = 0x%.2x
1 13 = 00  ||         Unsolicited Sense
1 13 = 21  ||         Data Streaming Not Operational
1 13 = 22  ||         Path Equipment Check
1 13 = 23  ||         Read Data Check
1 13 = 24  ||         Load Display Check
1 13 = 25  ||         Write Data Check
1 13 = 26  ||         Data Check (Read Opposite)
1 13 = 27  ||         Command Reject
1 13 = 28  ||         Write ID Mark Check
1 13 = 29  ||         Function Incompatible
1 13 = 2a  ||         Unsolicited Environmental Data
1 13 = 2b  ||         Environmental Data Present
1 13 = 2c  ||         Permanent Equipment Check
1 13 = 2d  ||         Data Security Erase Failure
1 13 = 2e  ||         Not Capable (BOT Error)
1 13 = 30  ||         Write Protected
1 13 = 31  ||         Tape Void
1 13 = 32  ||         Tension Loss
1 13 = 33  ||         Load Failure
1 13 = 34  ||         Unload Failure
1 13 = 35  ||         Drive Equipment Check
1 13 = 36  ||         End of Data
1 13 = 37  ||         Tape Length Error
1 13 = 38  ||         Physical End of Volume
1 13 = 39  ||         Backward at BOT
1 13 = 3a  ||         Drive Switched Not Ready
1 13 = 3b  ||         Manual Rewind/Unload
1 13 = 40  ||         Overrun
1 13 = 41  ||         Record Sequnce Number
1 13 = 42  ||         Degraded Mode
1 13 = 43  ||         Drive Not Ready
1 13 = 44  ||         Locate Block Unsuccessful
1 13 = 45  ||         Drive Assigned Elsewhere
1 13 = 46  ||         Drive Not On-line
1 13 = 47  ||         Volume Fenced
1 13 = 48  ||         Log Sense Data and Retry Request
1 13 = 49  ||         Bus Out Check
1 13 = 4a  ||         Control Unit ERP Failed
1 13 = 4b  ||         Control Unit and Drive Incompatible
1 13 = 4c  ||         Recovered Check-1 Failure
1 13 = 4d  ||         Resetting Event
            |           If resetting event notification is not supported, the program logs
            |           a temporary OBR record.
1 13 = 4e  ||         Maximum Block Size Exceeded
1 13 = 50  ||         Read Bufferred Log (Overflow)
1 13 = 51  ||         Read Bufferred Log (EOV)
1 13 = 52  ||         End of Volume Complete
1 13 = 53  ||         Global Command Intercept
1 13 = 54  ||         Channel Interface Recovery - Temp
1 13 = 55  ||         Channel Interface Recovery - Perm
1 13 = 56  ||         Channel Protocol Error
1 13 = 57  ||         Attention Intercept
1 13 = 5b  ||         Format 3480 XF Incompatible
1 13 = 5c  ||         Format 3480-2 XF Incompatible
1 13 = 5e  ||         Compaction Algorythm Incompatible
1 13 = 60  ||         Libray Attachment Facility Equipment Check
1 13 = 62  ||         Libray Manager Offline to Subsystem
1 13 = 63  ||         Control Unit and Libray Manager Incompatible
1 13 = 64  ||         Library VOLSER in Use
1 13 = 65  ||         Library Volume Reserved
1 13 = 66  ||         Library VOLSER Not in Library
1 13 = 67  ||         Library Catagory Empty
1 13 = 68  ||         Library Order Sequence Check
1 13 = 69  ||         Library Output Stations Full
1 13 = 6b  ||         Library Volume Misplaced
1 13 = 6c  ||         Library Misplaced Volume Found
1 13 = 6d  ||         Library Drive Not Uploaded
1 13 = 6e  ||         Library Inaccessible Volume Restored
1 13 = 6f  ||         Library Vision Failure
1 13 = 70  ||         Library Manager Equipment Check
1 13 = 71  ||         Library Equipment Check
1 13 = 72  ||         Library Not Capable - Manual Mode
1 13 = 73  ||         Library Intervention Required
1 13 = 74  ||         Library Informational Data
1 13 = 75  ||         Library Volume Inaccessible
1 13 = 76  ||         Library All Cells Full
1 13 = 77  ||         Library Duplicate VOLSER Ejected
1 13 = 78  ||         Library Duplicate VOLSER Left in Input Station
1 13 = 79  ||         Library Unreadable or Invalid VOLSER Left in Input Station
1 13 = 7a  ||         Library Read Library Statistics
1 13 = 7b  ||         Library Volume Manually Ejected
1 13 = 7c  ||         Library Out of Cleaner Volumes
1 13 = 7f  ||         Library Catagory in Use
1 13 = 80  ||         Library Unexpected Volume Ejected
1 13 = 81  ||         Library I/O Station Door Open
1 13 = 82  ||         Library Manager Program Exception
1 13 = 83  ||         Library Drive Exception
1 13 = 84  ||         Library Drive Failure
1 13 = 85  ||         Library Environmental Alert
1 13 = 86  ||         Library All Catagories Reserved
1 13 = 87  ||         Library Duplicate Volume Add Requested
1 13 = 88  ||         Damaged Cartrisge Ejected

1 14 @ 24   |    e) Channel Logical Block Number = 0x%.6x
1 17 @      |    f) Format of Sense or Log Data = 0x%.2x
1 17 = 19   |         Forced Error Logging Sense
1 17 = 20   |         Error and Informational Sense
1 17 = 21   |         Buffered Log Data (Not Extended Log Format)
1 17 = 22   |         End of Volume Sense
1 17 = 23   |         Library and Error and Informational Sense
1 17 = 24   |         VOLID Mask Information Sense
1 17 = 30   |         Buffered Log Data (Extended Log Format)
1 18 @      |    g) Library Error Modifier Byte = 0x%.2x
1 18 = 27  ||         Command Reject
1 18 = 29  ||         Function Incompatible
1 18 = 63  ||         Control Unit and Libray Manager Incompatible
1 18 = 64  ||         Library VOLSER in Use
1 18 = 74  ||         Library Informational Data
1 18 = 81  ||         Library I/O Station Door Open
1 18 = 83  ||         Library Drive Exception
1 18 = 84  ||         Library Drive Failure

1 19 @ 16   |    h) Library Error Code = 0x%.4x
1 21 @ 16   |    i) Library Other Code = 0x%.4x

1 23 @ 24   @volser1
1 26 @ 24   @volser2
            %    j) VOLSER = 0x%.4x%.4x ~volser1 volser2
1 29 @      |    k) Library Manager Software EC Level = 0x%.2x
1 30 @      |    l) Library Subsystem ID = 0x%.2x
 
1 31 @      @seqnum1
2 00 @ 16   @seqnum2
            %    m) Library Sequence Number = 0x%.2x%.4x ~seqnum1 seqnum2
2 02 @      |    n) Format 19/20 Byte 24 = 0x%.2x @nibble
2 02  ^ 80  |         Channel Adapter A Installed
2 02  ^ 40  |         Channel Adapter B Installed
2 02  ^ 20  |         Channel Adapter C Installed
2 02  ^ 10  |         Channel Adapter D Installed
            %         Data Transfer Mode:
            nibble := nibble & 0x0f
nibble = 00 |           Unthrottled Interlock
nibble = 01 |           1.5MB Interlock
nibble = 05 |           2.0MB Streaming
nibble = 06 |           3.0MB Streaming
nibble = 07 |           4.5MB Streaming
nibble = 0e |           Serial 9.0MB
nibble = 0f |           Serial 4.5MB
2 03 @      |    o) Format 19/20 Byte 25 = 0x%.2x @nibble
2 03  ^ 80  |         Dual CU Communication
2 03  ^ 40  |         Reserved
2 03  ^ 20  |         Library Interface Online
2 03  ^ 10  |         Library Installed
2 03  ^ 08  |         IDRC Allowed
2 03  ^ 04  |         IDRC Installed
2 03  ^ 02  |         Upgrade Buffer
2 03  ^ 01  |         ACL Feature
2 04 @      |    p) Format 19/20 Byte 26 (Control Unit Microcode EC Level = 0x%.2x)
            %    q) Subsystem Type:
2 05 @      @nibble
            stype := nibble & 0xf0
stype = 01  |         3490 Model D41/D42
stype = 02  |         3490 Model A10/A20/B20/B40
stype = 03  |         TA-90
stype = 04  |         3490 Model A01/A02/B02/B04
stype = 05  |         3480 Model A11/B11
stype = 06  |         3490 Model D31/D32
stype = 07  |         3480 Model A22/B22
stype = 09  |         3490 Model C10/C11/C22
            snumh := nibble & 0x0f
2 06 @ 16   @snuml
            %    r) Control Unit Serial Number = 0x%.1x%.4x ~snumh snuml
2 07 @      @nibble
            lda := nibble & 0xf0
            lda := lda >> 4
            %    s) Logical Drive Address = 0x%.1x ~lda
            pda := nibble & 0x0f
            %    t) Physical Drive Address = 0x%.1x ~pda


!cdrom
#include cdrom1

!cdrom1
* Sense information for CD ROM device

%                         CD ROM Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_cd 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @       | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  ^ 70   $err_class_okay
~if (! err_class_okay)
             %    WARNING Error Status not valid sense return.
             %    Will translate as if it were but results may be wrong
~endif

0 22 @       |    Sense Key (0x%.2x)
0 22 = x0    |    No Sense
0 22 = x1    ||    Recovered Error
0 22 = x2    ||    Not Ready -- Drive not ready (may be no disc)
0 22 = x3    ||    Medium Error -- Disc defective
0 22 = x4    ||    Hardware Error --Unrecovered hardware error
0 22 = x5    ||    Illegal Request -- Command or parms not valid
0 22 = x6    ||    Unit Attention --
             |      a) Caddy Eject executed and media was exchanged OR
             |      b) CD-ROM drive reset by power-on, reset, or SCSI reset
             |      c) Mode Select cmnd issued and block length in block
             |         desc. or error recovery parms modified

1 0 @        |    Additional Sense Code (0x%.2x)
1 0  = 00    |    No Sense
1 0  = 04    ||    Not Ready -- Drive not ready (may be no or wrong disc)
1 0  = x3    ||    Medium Error -- Disc defective
1 0  = 11    ||    Unrecovered Error -- Unrecovered data read error
1 0  = 12    ||    No address --
             |       CD-ROM HEADER address of MARK ERROR object block does
             |       not exist
1 0  = 15    ||    Seek -- Seek not complete within specified time limit
1 0  = 17    ||    Recovered Error (with retries)
1 0  = 18    ||    Recovered Error (with ECC)
1 0  = 20    ||    Invalid command Op Code
1 0  = 21    ||    Illegal Logical Block Address
             |     Improper block address or subcode Q address in CDB
             |     OR disc last position is exceeded
1 0  = 24    ||    Illegal field in Command Descriptor Block
1 0  = 26    ||    Invalid field in parameter list
1 0  = 28    ||    Medium changed -- Caddy eject executed and CD disc chnged
1 0  = 29    ||    Power On Reset or Bus Deveice Reset
1 0  = 2A    ||    Mode Select parameter Change
1 0  = 30    ||    Incompatible cartridge
             |     May have caused
             |     a) Tracking servo failure
             |     b) Spindle servo failure
             |     c) Reading (subcode Q data = audio level) failure
             |     d) Track jump execution function failure
             |     e) Memory check failure
             |     f) Reading (header address = CD-ROM level) failure OR
             |     g) CD-ROM data error dectection/correction function failure
1 0  = 42    ||    Self Test Diag failure
1 0  = 44    ||    Internal Controller Error
1 0  = 45    ||    Select/reselect fail
1 0  = 47    ||    SCSI Interface Parity Error
1 0  = 49    ||    Inappropriate/illegal message received in Data/IN Out phase
1 0  = 88    ||    Not digital audio track -- Illegal request
1 0  = 89    ||    Not CD-ROM data track -- Illegal request
1 0  = 8a    ||    Not Audio Play state -- Illegal request


!enhcdrom2
* enhcdrom2 and enhcdrom3 are same as enhcdrom.
#include enhcdrom

!enhcdrom3
#include enhcdrom

!enhcdrom
%                         Enhanced CD ROM Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_cd 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @        | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  ^ 70   $err_class_okay
~if (! err_class_okay)
             %    WARNING Error Status not valid sense return.
             %    Will translate as if it were but results may be wrong
~endif

0 22 @        | 7) Sense Key (0x%.2x)
0 22 = 00     |      NO SENSE - No specific sense key.
0 22 = 01    ||      RECOVERED ERROR
              |        The last command completed successfully with some recovery action
              |        performed by the drive.
0 22 = 02    ||      NOT READY - Drive not ready.
0 22 = 03    ||      MEDIUM ERROR
              |        The command terminated with a non-recovered error condition that was
              |        probably caused by a flaw in the medium. This sense key may also be
              |        returned if the drive is unable to distinguish between a flaw in the
              |        medium and a specific hardware failure.
0 22 = 04    ||      HARDWARE ERROR
              |        The drive detected a non-recoverable hardware failure while performing
              |        a command or during a self test.
0 22 = 05    ||      ILLEGAL REQUEST
              |        There was an illegal parameter in the command descriptor block or in
              |        the additional parameters supplied as data for some commands.
0 22 = 06    ||      UNIT ATTENTION
              |        The removable medium may have been changed or the drive has been reset.
0 22 = 08    ||      BLANK CHECK
              |        The drive encountered blank medium or format-defined end of data
              |        indication while reading.
0 22 = 0b    ||      ABORTED COMMAND - The drive aborted the command.
              |        The initiator can recover execution of the cmd by issuing the cmd again.

1 0 @ 16      | 8) Additional Sense Code (0x%.4x)
1 0  = 0000   |      No additional sense information.
1 0  = 0006  ||      I/O process terminated.
1 0  = 0011  ||      Audio play operation in progress.
1 0  = 0012  ||      Audio play operation paused.
1 0  = 0013  ||      Audio play operation successfully completed.
1 0  = 0014  ||      Audio play operation stopped due to error.
1 0  = 0015  ||      No current audio status to return.
1 0  = 0200  ||      No seek complete.
1 0  = 0400  ||      Logical unit not ready, cause not reportable.
1 0  = 0401  ||      Logical unit is in process of becoming ready.
1 0  = 0402  ||      Logical unit not ready, initializing command required.
1 0  = 0403  ||      Logical unit not ready, manual intervention required.
1 0  = 0500  ||      Logical unit does not respond to selection.
1 0  = 0600  ||      No reference position found.
1 0  = 0700  ||      Multiple peripheral devices selected.
1 0  = 0800  ||      Logical unit communication failure.
1 0  = 0801  ||      Logical unit communication time-out.
1 0  = 0802  ||      Logical unit communication parity error.
1 0  = 0900  ||      Track following error.
1 0  = 0901  ||      Tracking servo failure.
1 0  = 0902  ||      Focus servo failure.
1 0  = 0903  ||      Spindle servo failure.
1 0  = 0a00  ||      Error log overflow.
1 0  = 1100  ||      Unrecovered read error.
1 0  = 1105  ||      L-EC uncorrectable error.
1 0  = 1106  ||      CIRC unrecovered error.
1 0  = 1400  ||      Recorded entity not found.
1 0  = 1401  ||      Record not found.
1 0  = 1500  ||      Random positioning error.
1 0  = 1501  ||      Mechanical positioning error.
1 0  = 1502  ||      Positioning error detected by read of medium.
1 0  = 1700  ||      Recovered data with no error correction applied.
1 0  = 1701  ||      Recovered data with retires.
1 0  = 1702  ||      Recovered data with positive head offset.
1 0  = 1703  ||      Recovered data with negative head offset.
1 0  = 1704  ||      Recovered data with retries and/or CIRC applied.
1 0  = 1705  ||      Recovered data using previous sector ID.
1 0  = 1800  ||      Recovered data with error correction applied.
1 0  = 1801  ||      Recovered data with error correction and retries applied.
1 0  = 1802  ||      Recovered data - data auto-reallocated.
1 0  = 1803  ||      Recovered data with with CIRC.
1 0  = 1804  ||      Recovered data with with LEC.
1 0  = 1805  ||      Recovered data - recommend reassignment.
1 0  = 1a00  ||      Parameter list length error.
1 0  = 1b00  ||      Synchronous data transfer error.
1 0  = 2000  ||      Invalid command operation code.
1 0  = 2100  ||      Logical block address out of range.
1 0  = 2400  ||      Invalid field in CDB.
1 0  = 2500  ||      Logical unit not supported.
1 0  = 2600  ||      Invalid field in parameter list.
1 0  = 2601  ||      Parameter not supported.
1 0  = 2602  ||      Parameter value invalid.
1 0  = 2603  ||      Threshold parameters not supported.
1 0  = 2800  ||      Not ready to ready transition (medium may have changed).
1 0  = 2900  ||      Power on, reset, or bus device reset occurred.
1 0  = 2a00  ||      Parameters changed.
1 0  = 2a01  ||      Mode parameters changed.
1 0  = 2a02  ||      Log parameters changed.
1 0  = 2b00  ||      Copy cannot execute since host cannot disconnect.
1 0  = 2c00  ||      Command sequence error.
1 0  = 2f00  ||      Commands cleared by another initiator.
1 0  = 3000  ||      Incompatible medium installed.
1 0  = 3001  ||      Cannot read medium - unknown format.
1 0  = 3002  ||      Cannot read medium - incompatible format.
1 0  = 3700  ||      Rounded parameter.
1 0  = 3900  ||      Saving parameters not supported.
1 0  = 3a00  ||      Medium not present.
1 0  = 3d00  ||      Invalid bits in identify message.
1 0  = 3e00  ||      Logical unit has not self-configured yet.
1 0  = 3f00  ||      Target operating conditions have changed.
1 0  = 3f01  ||      Microcode has been changed.
1 0  = 3f02  ||      Changed operating definition.
1 0  = 3f03  ||      Inquiry data has changed.
1 0  = 4080  ||      Diagnostic failure on TC9220F.
1 0  = 4081  ||      Diagnostic failure on memories.
1 0  = 4082  ||      Diagnostic failure on CD-ROM ECC circuit.
1 0  = 4083  ||      Diagnostic failure on gate-array.
1 0  = 4084  ||      Diagnostic failure on internal SCSI controller.
1 0  = 4300  ||      Message error.
1 0  = 4400  ||      Internal target failure.
1 0  = 4500  ||      Select or reselect failure.
1 0  = 4600  ||      Unsuccessful soft reset.
1 0  = 4700  ||      SCSI parity error.
1 0  = 4800  ||      Initiator detected error message received.
1 0  = 4900  ||      Invalid message error.
1 0  = 4a00  ||      Command phase error.
1 0  = 4b00  ||      Data phase error.
1 0  = 4c00  ||      Logical unit failed self-configuration.
1 0  = 4e00  ||      Overlapped command attempted.
1 0  = 5300  ||      Media load or eject failed.
1 0  = 5302  ||      Medium removal prevented.
1 0  = 5700  ||      Unable to recover table-of-contents.
1 0  = 5a00  ||      Operator request or state change input (unspecified).
1 0  = 5a01  ||      Operator medium removal request.
1 0  = 5b00  ||      Log exception.
1 0  = 5b01  ||      Threshold condition met.
1 0  = 5b02  ||      Log counter at maximum.
1 0  = 5b03  ||      Log list codes exhausted.
1 0  = 6300  ||      End of user area encountered on this track.
1 0  = 6400  ||      Illegal mode for this track.

!osomd
%                         Rewriteable CD ROM Sense Data Analysis
%
* Include General SCSI Decode Information
#set is_scsi_rwcd 1

#include scsi_gen

* Decode byte 0,20 Composed of  bit 7 = valid information bytes
*                                 6-4 = Error Class
*                                 4-0 = Error Code
0 20 @        | 6) Error Status Information (0x%.2x)
                  0 20  ^ 80   $valid_ibytes
                  0 20  ^ 70   $err_class_okay
~if (! err_class_okay)
             %    WARNING Error Status not valid sense return.
             %    Will translate as if it were but results may be wrong
~endif

0 22 @        | 7) Sense Key (0x%.2x) @sense_key
0 22 = 00     |      NO SENSE - No specific sense key.
0 22 = 01    ||      RECOVERED ERROR
              |        The last command completed successfully with some recovery action
              |        performed by the drive.
0 22 = 02    ||      NOT READY - Drive not ready.
0 22 = 03    ||      MEDIUM ERROR
              |        The command terminated with a non-recovered error condition that was
              |        probably caused by a flaw in the medium. This sense key may also be
              |        returned if the drive is unable to distinguish between a flaw in the
              |        medium and a specific hardware failure.
0 22 = 04    ||      HARDWARE ERROR
              |        The drive detected a non-recoverable hardware failure while performing
              |        a command or during a self test.
0 22 = 05    ||      ILLEGAL REQUEST
              |        There was an illegal parameter in the command descriptor block or in
              |        the additional parameters supplied as data for some commands.
0 22 = 06    ||      UNIT ATTENTION
              |        The removable medium may have been changed or the drive has been reset.
0 22 = 08    ||      BLANK CHECK
              |        The drive encountered blank medium or format-defined end of data
              |        indication while reading.
0 22 = 0b    ||      ABORTED COMMAND - The drive aborted the command.
              |        The initiator can recover execution of the cmd by issuing the cmd again.

1 0 @ 16      | 8) Additional Sense Code and Qualifier (0x%.4x)
1 0  = 0000   |      No additional sense information
1 0  = 0100  ||      No index/sector signal (No sector mark found)
1 0  = 0200  ||      No seek complete
              |        Occurs when the time required to move the actuator to the new
              |        location exceeds the specified time required to complete the operation.
1 0  = 0400  ||      Logical unit not ready, cause not reportable
1 0  = 0900  ||      Track following error
              |        Occurs when the positioning of the actuator cannot be maintained
              |        over a given track.  If the focus is dropped due to defect on media
              |        or any other drive fault occurs, this error is also reported.
1 0  = 0903  ||      Spindle servo error
1 0  = 1000  ||      ID CRC error
              |        A read error was detected in the ID field of the sector during read
              |        or write operation.  If the host data transfer is slow in write 
              |        operation, this additional sense code may be set with recoverd sense
              |        key.  In this case write operation itself has succeeded with error
              |        recovery procedure.  IC CRC error is also reported when a pseudo sector
              |        mark is detected.
1 0  = 1100  ||      Unrecoverable read error
1 0  = 1500  ||      Seek positioning error
              |        Occurs when the positioning of the actuator to a new location
              |        failed to reach that new location.
1 0  = 1600  ||      Data sync mark error
              |        Occurs when when the sync field at the beginning of the data field
              |        cannot be detected.
1 0  = 1800  ||      Recovered read data with ECC applied
1 0  = 1900  ||      Defect list error
1 0  = 1901  ||      Defect list not available
              |        Occurs when defect list reading is attempted when no defect list
              |        is on the media.  Sense key for this code is No Sense (0).
1 0  = 1a00  ||      Parameter list length error
1 0  = 1c00  ||      Primary Defect List not found
1 0  = 2000  ||      Invalid command operation code
1 0  = 2100  ||      Logical block address out of range
1 0  = 2400  ||      Invalid field in CDB
              |        This code includes the Unload Cartridge request with cartridge locked.
1 0  = 2500  ||      Illegal LUN
1 0  = 2600  ||      Invalid field in parameter list
1 0  = 2700  ||      Write protected
1 0  = 2800  ||      Not ready to ready transition (Medium may have changed)
              |        This is not an error condition.  This additional sense code indicates
              |        that a change was made to the media.  The change to the media could
              |        have resulted from not ready to ready transition.
1 0  = 2900  ||      Power on reset or Bus Device Reset ocurred
              |        This is not an error condition.  The target was reset.
1 0  = 2a01  ||      Mode Select parameters changed
              |        This is not an error condition.
1 0  = 3000  ||      Incompatible medium installed
1 0  = 3100  ||      Medium format corrupted
              |        A Format operation was interrupted prior to completion.  The Format Unit
              |        command should be re-issued.
1 0  = 3200  ||      No defect spare location available
1 0  = 3201  ||      Automatic reassignment error
              |        Automatic Read/Write reassignment failed 3 times on the same operation.
1 0  = 3900  ||      Saving parameters not supported
1 0  = 3a00  ||      Medium not present
1 0  = 4300  ||      Message error
              |        Occurs when an inappropriate or unexpected message reject is received
              |        from the initiator or the initiator rejects a message twice.
1 0  = 4400  ||      Internal controller error / Miscellaneous error
1 0  = 4500  ||      Select/Re-select failure
              |        Occurs when the initiator fails to respond twice to a re-selection
              |        within 250ms after gaining bus arbitration.
1 0  = 4700  ||      SCSI parity error
1 0  = 4800  ||      Initiator detected error message received
              |        Occurs when the initiator detects an error, sends a message to retry,
              |        detects the error again, and sends the retry message a second time.
              |        The target then sets the Check Condition status with this sense code.
1 0  = 4900  ||      Invalid message error
1 0  = 4e00  ||      Overlap command attempted
              |        Occurs when the initiator sent a command while the target was
              |        executing another command from this initiator.
1 0  = 5100  ||      Erase failure
              |        Laser power calibration failure.  Target cannot write data on this
              |        medium.
1 0  = 5300  ||      Media load/eject failure
1 0  = 5302  ||      Medium removal prevented
              |        While at least one Prevent/Allow Medium Removal command is in effect,
              |        a SCSI command that would eject medium was attempted and failed.
1 0  = 5a01  ||      Operator medium removal request
              |        While medium removal is not allowed by Prevent/Allow Medium Removal 
              |        command or jumper, Eject button was pressed by operator.

              % 9) Additional Information
~if (valid_ibytes)
  0 23 @ 32     |        Logical Block address where error was caused = 0x%.8x
~endif

0 27 @        |        Additional Sense Length = %d (decimal)

* If Reassign Blocks command failed:
0 04 @    @cmnd
~if (cmnd == 07)
  0 28 @ 32 |            Logical Block Address where Reassign Blocks failed = 0x%.8x
~endif

1 3   ^ 80  ^FPV
* If FPV bit is set
~if (FPV)
  1 3 ^ 40       ^CD
  1 3 ^ 8        ^BPV
  1 3 @          @temp_byte
  bit_ptr := (temp_byte & 0x7)
  1 4 @ 16       @field_ptr

  * If in the Command Descriptor Block else Data Parameter
  ~if (CD)
              %          Illegal Request Sense Key issued in CDB byte 0x%.8x ~field_ptr
  ~else
              %          Illegal Request Sense Key issued in Data phase byte 0x%.8x ~field_ptr
  ~endif

  * if Field Pointer Value is valid
  ~if (BPV)
              %          Illegal Request Sense Key in bit %d of the specified byte ~bit_ptr
  ~endif
~endif

~if ((sense_key == 0x01) || (sense_key == 0x03) || (sense_key == 0x04))
  1 12 @ 24   |          Track location of error = %d
  1 15 @      |          Sector location of error = %d
  1 17 @      |          Number of recovery actions taken = %d
~endif


!badisk
* Sense information for Bus Attached Attached Hardfiles
%                 Bus Attached Hardfile Sense Data Analysis
%
             % 1) Location of error information

0  0  @  32  |    Segment count in which error occurred        = 0x%.8x
0  4  @  32  |    Byte count within segment where err occurred = 0x%.8x
%
0  8  @      | 2) Word count of status block = %d
0  9  @      | 3) Last Command (0x%.2x)
0  9  = 01   |      Read Data
0  9  = E1   |      Read Data
0  9  = 02   |      Write Data
0  9  = E2   |      Write Data
0  9  = 03   |      Read Verify
0  9  = E3   |      Read Verify
0  9  = 04   |      Write Verify
0  9  = E4   |      Write Verify
0  9  = 05   |      Seek
0  9  = E5   |      Seek
0  9  = 06   |      Park Head
0  9  = E6   |      Park Head
0  9  = 08   |      Get Device Status
0  9  = E8   |      Get Device Status
0  9  = 09   |      Get Device Configuration
0  9  = E9   |      Get Device Configuration
0  9  = 0A   |      Get POS Information
0  9  = EA   |      Get POS Information
0  9  = 0B   |      Translate RBA
0  9  = EB   |      Translate RBA
0  9  = 10   |      Write Attachment Buffer
0  9  = F0   |      Write Attachment Buffer
0  9  = 11   |      Read  Attachment Buffer
0  9  = F1   |      Read  Attachment Buffer
0  9  = 12   |      Run Diagnostic Test
0  9  = F2   |      Run Diagnostic Test
0  9  = 14   |      Get Diagnostic Status Block
0  9  = F4   |      Get Diagnostic Status Block
0  9  = 15   |      Get MFG Header
0  9  = F5   |      Get MFG Header
0  9  = 16   |      Format Unit
0  9  = F6   |      Format Unit
0  9  = 17   |      Get Command Complete Status
0  9  = F7   |      Get Command Complete Status
0 10  @      | 4) Command Status (0x%.2x)
0 10  = 01   ||      Command Completed with Success
0 10  = 03   ||      Command Completed with Success with ECC Applied
0 10  = 05   ||      Command Completed with Success with Retries
0 10  = 06   ||      Format Command partially complete
0 10  = 07   ||      Command Completed with Success, ECC and Retries needed
0 10  = 08   ||      Command Completed with Warning
0 10  = 09   ||      Abort Complete
0 10  = 0A   ||      Reset Complete
0 10  = 0B   ||      Data Transfer Ready
0 10  = 0C   ||      Command Completed with Failure
0 10  = 0D   ||      DMA Error
0 10  = 0E   ||      Command Block Error
0 10  = 0F   ||      Attention Error
0 11  @      | 5) Command Error Value (0x%.4x)
0 11  = 00   |      No Error
0 11  = 01   ||      Invalid Parameter in Command Block
0 11  = 03   ||      Command Not supported
0 11  = 04   ||      Command Aborted
0 11  = 06   ||      Command Rejected (Attachment diagnostic failure)
0 11  = 07   ||      Format Rejected (Sequence error)
0 11  = 08   ||      Format error (Primary map unusable or defective)
0 11  = 09   ||      Format error (Secondary map unusable or defective)
0 11  = 0A   ||      Format error (diagnose error)
0 11  = 0B   ||      Format Warning (secondary map overflow)
0 11  = 0D   ||      Format error (host checksum error)
0 11  = 0F   ||      Format Warning (Push table overflow)
0 11  = 10   ||      Format Warning (more than 10 pushes in a cylinder)
0 11  = 12   ||      Format Warning (verify error(s) found)
0 11  = 13   ||      Invalid device for command

0  9  @      | 6) Device Status (0x%.2x)
0  9  ^ e0   |      Last Device is controlling attachment
0  9  ^ 1f   |      Last Device is disk

0 12  ^ 10   |      Device Ready
0 12 !^ 10   |      Device Not Ready
0 12  ^ 08   |      Device Selected
0 12 !^ 08   |      Device Not Selected
0 12  ^ 04   ||      Write fault
0 12 !^ 04   |      No Write fault
0 12  ^ 02   |      Track 0 flag
0 12 !^ 02   |      No track 0 flag
0 12  ^ 01   |      Seek or command complete
0 12 !^ 01   |      no seek or command not complete
%
0 13 @       | 7) Device Error Info (0x%.2x)
0 13 =  00   |      No error
0 13 =  01   ||      Seek fault
0 13 =  02   ||      Interface Fault
0 13 =  03   ||      Block not found (could not find ID)
0 13 =  04   ||      Block not found (AM not found)
0 13 =  05   ||      Data ECC error (hard error)
0 13 =  06   ||      ID CRC error
0 13 =  07   ||      RBA out of range
0 13 =  09   ||      Defective block (not used)
0 13 =  0B   ||      Selection error
0 13 =  0D   ||      Write fault
0 13 =  0E   ||      Read fault
0 13 =  0F   ||      No index or sector pulse
0 13 =  10   ||      Device not ready
0 13 =  11   ||      Seek error
0 13 =  12   ||      Bad format (not used)
0 13 =  13   ||      Volume overflow (not used)
0 13 =  14   ||      No data AM found
0 13 =  15   ||      no ID AM or ID ECC error found
0 13 =  18   ||      no ID found on track (ID search)
%
             % 8) Block Information
0 14 @  16   |    Blocks left to process = 0x%.4x
0 18 @  16   |    Last relative block processed = 0x%.4x\
0 16 @  16   |0x%.4x
0 20 @  16   |    Blocks requiring error recovery = 0x%.4x

!serdasda
#define SRN
%                     Serial DASD Adapter
%
0 0 @ 16 @SRN
              % 1) SRN # = 0x%.2x = %d (decimal) ~SRN  ~SRN
SRN = 0101   ||    Motor Error -> Exchange the FRUs for new FRUs.
SRN = 0111   ||    Servo Error -> Exchange the FRUs for new FRUs.
SRN = 0121   ||    FCM Error -> Exchange the FRU for a new FRU.
SRN = 0131   ||    PRML -> Exchange the FRU for a new FRU.
SRN = 0141   ||    Arm Electronics Error -> Exchange the FRUs for new FRUs.
SRN = 01c8   ||    Data overrun or underrun -> This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 01fe   ||    FCM code not known or unexpected disk-drive message.
              |      This might be caused by a microcode problem.  See -- Heading 'MICMAI'
              |      unknown -- before exchanging any FRUs.
SRN = 01ff   ||    A download of the system microcode has failed.
              |      Run diagnostics to the subsystem in System Verification mode.  If no
              |      error is found, see -- Heading 'MICMAI' unknown -- to check the microcode
              |      level. If no valid code level has been downloaded, the microcode package
              |      might have been corrupted. Reload the 9333 microcode from diskette.
SRN = 0281   ||    Sector has been reassigned or rewriten by the automatic media maintenance.
SRN = 0282   ||    An excessive number of soft data checks has been detected.
              |      Exchange the disk drive electronics card for a new one, then run the
              |      Certify service aid. If any data checks are detected, exchange the disk
              |      enclosure for a new one.
SRN = 0283   ||    Data check threshold exceeded on a single sector.
              |      This problem is normally solved by the automatic media maintenance
              |      procedures on the using system. Run the Certify service aid to check that
              |      no media defects remain.
SRN = 0284   ||    A Reassign Block command has failed.
              |      An attempt has been made to reassign downstream data checks, but the
              |      threshold has been exceeded. Exchange the FRUs for new FRUs.
SRN = 0285   ||    An excessive number of hard data checks has been detected.
              |      Exchange both FRUs at the same time. Do not use the disk drive electronics
              |      card again, although it might appear to be working correctly.
SRN = 0286   ||    Internal media maintenance has detected an excessive number of media errors.
              |      Exchange the FRUs for new FRUs.
SRN = 0287   ||    A read or write of a reserved area failed.
              |      Exchange the FRUs for new FRUs.
SRN = 0288   ||    An internal media error prevented a Reassign Blocks command completing.
              |      Exchange the FRUs for new FRUs.
SRN = 0291   ||    Random soft data check threshold exceeded.
              |      Exchange the FRUs for new FRUs and run the Certify service aid.
SRN = 02ff   ||    Invalid format 2 error. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0301   ||    Controller DMA Channel Error -> Exchange the FRUs for new FRUs.
SRN = 0321   ||    Controller-to-disk-drive serial-link line fault.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = 0322   ||    Controller-to-disk-drive serial-link order time-out.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = 0323   ||    Controller serial-link hardware error.
              |      Exchange the FRUs for new FRUs.
SRN = 0324   ||    Controller-to-disk-drive permanent line fault.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = 0351   ||    Controller-to-disk-drive serial-link time-out.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = 0353   ||    Disk drive serial-link hardware error -> Exchange the FRUs for new FRUs.
SRN = 0380   ||    Controller-to-adapter communication error -> Exchange the FRUs for new FRUs.
SRN = 0381   ||    Controller processor Check-1 -> Exchange the FRUs for new FRUs.
SRN = 0382   ||    Controller microcode Check-1.
              |      This might be caused by by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0391   ||    No spare sectors available -> Exchange both FRUs at the same time.
              |      Do not use the disk drive electronics card again, although it might
              |      appear to be working correctly.
SRN = 03a0   ||    Degraded mode - not 512-byte format or the Format Unit command failed.
              |      Run the Format service aid to this disk drive.
SRN = 03a1   ||    Degraded mode - a Reassign Block command has failed.
              |      Run the disk drive test System Verification mode. If the test fails with
              |      the same SRN, Exchange the FRUs for new FRUs.
SRN = 03a3   ||    Degraded mode - the disk drive read/write tests have failed.
              |      Exchange the FRUs for new FRUs.
SRN = 03a4   ||    Degraded mode - the disk drive POST has failed.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = 03a5   ||    Degraded mode - configuration sector data unreadable.
              |      Exchange the FRUs for new FRUs.
SRN = 03a6   ||    Degraded mode - motor stopped -> Exchange the FRUs for new FRUs.
SRN = 03a7   ||    Degraded mode - Controller does not work with the disk drive.
              |      You need to load microcode that works with the disk drive, contact your
              |      support center.
SRN = 03c2   ||    Controller-to-disk-drive serial link order time-out.
              |      Exchange the FRUs for new FRUs.
SRN = 03c3   ||    Controller-to-disk-drive serial link - Message Sent time-out.
              |      Exchange the FRUs for new FRUs.
SRN = 03f0   ||    Undefined disk drive error.
              |      If the disk drive module Power Good or Ready light is off or the Check
              |      light is on, go to -- Heading 'START' unknown --.
SRN = 03fd   ||    DMA error. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 03fe   ||    Invalid software/microcode condition detected.
              |      This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 03ff   ||    No valid sense data. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0401   ||    Invalid queue control, mailbox, or message detected by the controller.
              |      This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0402   ||    Command to an active tag.
              |      This might be caused by a microcode or device driver problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0403   ||    Download code with adapter not quiet.
              |      This might be caused by a microcode or device driver problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0404   ||    Illegal adapter command.
              |      This might be caused by a microcode or device driver problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging any FRUs.
SRN = 0405   ||    The controller is not responding.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise, ensure that the external serial link cable is
              |      connected correctly. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = 0407   ||    Mailbox timeout.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise, exchange the FRUs for new FRUs.
SRN = 0408   ||    Open link or controller is powered down.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise, ensure that the external serial link cable is
              |      connected correctly. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = 040a   ||    Unexpected SCSI status -> Exchange the FRUs for new FRUs.
SRN = 040b   ||    Unexpected SCSI status -> Exchange the FRU for a new FRU.
SRN = 040c   ||    Data DMA has failed -> Exchange the FRUs for new FRUs.
SRN = 040d   ||    The mailbox DMA has failed.
              |      Run diagnostics to the base system in Problem Determination mode.
              |      If no error is found, exchange the FRU for a new FRU.
SRN = 040e   ||    Invalid mailbox pointer.
              |      Run diagnostics to the base system in Problem Determination mode.
              |      If no error is found, exchange the FRU for a new FRU.
SRN = 040f   ||    DMA count error -> Exchange the FRUs for new FRUs.
SRN = 0410   ||    The controller address is out of range.
              |      Run diagnostics to the base system in Problem Determination mode.
              |      If no error is found, exchange the FRU for a new FRU.
SRN = 0411   ||    Quiesce timeout.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise, exchange the FRUs for new FRUs.
SRN = 0413   ||    The download adapter-microcode checksum has failed.
              |      Exchange the FRUs for new FRUs.
SRN = 0414   ||    Message to an inactive tag.
              |      Run diagnostics in System Verification mode to each controller that is
              |      attached to this adapter. If no error is found, exchange the FRU for a
              |      new FRU.
SRN = 0415   ||    Message from wrong link.
              |      Run diagnostics in System Verification mode to each controller that is
              |      attached to this adapter. If no error is found, exchange the FRU for a
              |      new FRU.
SRN = 0416   ||    Invalid link message.
              |      Run diagnostics in System Verification mode to each controller that is
              |      attached to this adapter. If no error is found, exchange the FRU for a
              |      new FRU.
SRN = 0417   ||    Irrecoverable link error.
              |      Ensure that the external serial link cable is correctly installed. If the
              |      problem remains, exchange the FRUs in the order listed.
SRN = 0418   ||    Invalid adapter parameter. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging the FRU.
SRN = 0419   ||    Invalid tag. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown -- before exchanging the FRU.
SRN = 041d   ||    DMA disabled -> Exchange the FRUs for new FRUs.
SRN = 0421   ||    Mailbox terminated by internal reset.
              |      Run diagnostics to the controller in 'System Verification' mode. If no
              |      trouble is found scan the error log for the last 24 hours. If another
              |      occurrence of this SRN is found then check that the local serial link
              |      cable is secure. If the cable is secure then replace all the listed FRUs.
SRN = 0422   ||    Adapter hardware error -> exchange the FRUs for new FRUs.
SRN = 0424   ||    DMA error.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found exchange the FRU for a new FRU.
SRN = 0480   ||    Write-data parity check.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found exchange the FRU for a new FRU.
SRN = 0481   ||    Invalid access.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found exchange the FRU for a new FRU.
SRN = 0484   ||    The subsystem detected a power failure.
              |      If the Bulk-Power Present light is off, go to -- Heading 'START'
              |      unknown --. If the light is on, a short power interruption might have
              |      been detected. If the installation has been having power interference
              |      for example, from decreased mainline voltage and thunderstorms), take no
              |      action. Go to -- Heading 'FIXMAP' unknown --. If the installation has no
              |      power interference, exchange the FRUs for new FRUs.
SRN = 048a   ||    A command has failed to complete.
              |      This might be caused by a microcode problem. See -- Heading 'MICMAIN'
              |      unknown -- before exchanging the FRUs.
SRN = 04fe   ||    Invalid adapter or controller status has been received.
              |      Run diagnostics to the adapter and controller in System Verification
              |      mode. If no error is found, the problem might be in the microcode.
              |      See -- Heading 'MICMAIN' unknown -- before exchanging the FRUs.
SRN = 04ff   ||    Adapter type-1 error -> Exchange the FRU for a new FRU.
SRN = 0500   ||    Time-out is waiting for Queue Full status to clear.
              |      Exchange the FRUs for new FRUs.
SRN = 0501   ||    Device reserved via the alternative adapter path.
              |      If the controller has two adapter connectors connected, the controller
              |      might be reserved to the alternative path. Run the disk drive tests in
              |      System Verification mode. If only one adapter connector is connected,
              |      exchange the FRU for a new FRU.
SRN = 0502   ||    Device driver detected hardware error.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found exchange the FRU for a new FRU.
SRN = 0503   ||    Host system operation time-out.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRUs for new FRUs.
SRN = 0504   ||    Channel Ready time-out.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found exchange the FRU for a new FRU.
SRN = 0505   ||    Invalid Fenced-Out status.
              |      If the controller is attached to more than one host system path, go to
              |      "Fencing" before replacing any FRUs.
SRN = 0802   ||    Disk drive hardware error -> Exchange the FRU for a new FRU.
SRN = 0803   ||    Disk drive media defect -> Exchange the FRUs for new FRUs.
              |      Do not attempt to replace the disk drive electronics card.
SRN = 0804   ||    Microcode mismatch -> See "Microcode Maintenance" before replacing any FRUs.
SRN = 0805   ||    No spare sectors available -> Exchange the FRU for a new FRU.
              |      Do not attempt to replace the disk drive electronics card.
SRN = 0806   ||    Error threshold exceeded.
              |      If the disk drive is still operational, ask the customer to backup the
              |      data before you exchange the FRU for a new FRU.
SRN = 08fe   ||    Invalid op-code, LBA or parameter.
              |      See "Microcode Maintenance" before replacing any FRUs.
SRN = a001   ||    Problem is smoke, a burning smell, or a noise problem.
              |      Customer problem determination procedures indicated the above. Go to
              |      -- Heading 'START' unknown --.
SRN = a002   ||    Power is on at the rack but off at the controller.
              |      Go to -- Heading 'START' unknown --.
SRN = a003   ||    Power is on at the 9333 but the controller has not detected power on.
              |      Go to -- Heading 'START' unknown --.
SRN = a004   ||    A disk drive module has failed read/write tests.
              |      Go to -- Heading 'START' unknown --.
SRN = a005   ||    A Controller has failed POST -> Exchange the FRU for a new FRU.
              |      If the problem remains, go to -- Heading 'START' unknown --.
SRN = a006   ||    The Bulk-Power Present light does not go off when switch is set to Standby.
              |      Exchange the FRUs for new FRUs. If the problem remains, go to
              |      -- Heading 'START' unknown --.
SRN = a010   ||    A disk drive module has no power -> Go to -- Heading 'START' unknown --.
SRN = a011   ||    The disk drive POST has failed -> Go to -- Heading 'START' unknown --.
SRN = a012   ||    A disk drive has failed to become ready.
              |      Go to -- Heading 'START' unknown --.
SRN = a013   ||    A disk drive has failed to turn off.
              |      Go to -- Heading 'START' unknown --.
SRN = c001   ||    Power Good at the controller, but no Power Good at the disk drive.
              |      Go to -- Heading 'START' unknown --.
SRN = c002   ||    Disk drive Check - the disk drive Check light is on.
              |      Go to -- Heading 'START' unknown --.
SRN = c003   ||    Disk drive not ready - the disk drive Check light and Ready light are off.
              |      Go to -- Heading 'START' unknown --.
SRN = c004   ||    The read/write verification test have failed.
              |      Go to -- Heading 'START' unknown --.
SRN = c005   ||    Cannot configure the disk drive.
              |      If the disk drive module Power Good or Ready light is off, or the Check
              |      light is on, or the reference display indicates the failing compartment,
              |      go to -- Heading 'START' unknown --. Otherwise, exchange the FRU for a
              |      new FRU.
SRN = c011   ||    Power is on at the rack, but off at the controller.
              |      Go to -- Heading 'START' unknown --.
SRN = c012   ||    An irrecoverable error has been detected by the controller.
              |      Go to -- Heading 'START' unknown --.
SRN = c013   ||    Cannot configure the controller.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise, ensure that the external serial link cable is
              |      connected correctly. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = c021   ||    Cannot configure the adapter -> Exchange the FRU for a new FRU.
SRN = c100   ||    Switch card hardware error. Code L indicates that switch 0 has failed.
              |      Code H indicates that switch 1 has failed. Exchange the FRUs for new FRUs.
SRN = c101   ||    Switch card configuration error.
              |      Go to MAP 2310 step 1 before exchanging any FRUs.
SRN = d001   ||    The adapter POST has failed -> Exchange the FRU for a new FRU.
SRN = d002   ||    OPENX to the adapter has failed -> Exchange the FRU for a new FRU.
SRN = d003   ||    OPENX to the adapter has failed -> Exchange the FRU for a new FRU.
SRN = d004   ||    OPENX to the adapter has failed.
              |      If the disk drive module Power Good or Ready light is off, or the Check
              |      light is on, or the reference display indicates the failing compartment,
              |      go to -- Heading 'START' unknown --. Otherwise, exchange the FRUs for
              |      new FRUs.
SRN = d101   ||    The disk drive POST has failed -> Exchange the FRUs for new FRUs.
SRN = d301   ||    The controller POST has failed -> Exchange the FRU for a new FRU.
SRN = d310   ||    Check reading sense -> Exchange the FRU for a new FRU.
SRN = d320   ||    Permanent Queue Full from the controller -> Exchange the FRU for a new FRU.
SRN = d330   ||    Controller fault - invalid reservation -> Exchange the FRU for a new FRU.
SRN = d340   ||    Controller fault - reservation is not resettable.
              |      Exchange the FRU for a new FRU.
SRN = d350   ||    Invalid sense data. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown --.
SRN = d360   ||    Controller has failed to quiesce -> Exchange the FRUs for new FRUs.
SRN = d370   ||    Controller cannot quiesce the drive.
              |      If the disk drive module Power Good or Ready light is off, or the Check
              |      light is on, go to -- Heading 'START' unknown --. Otherwise, exchange the
              |      FRUs for new FRUs.
SRN = d400   ||    Unexpected async status -> Exchange the FRU for a new FRU.
SRN = d401   ||    Unexpected special status -> Exchange the FRU for a new FRU.
SRN = d402   ||    Command to an active tag -> Exchange the FRU for a new FRU.
SRN = d403   ||    Unexpected adapter status -> Exchange the FRU for a new FRU.
SRN = d404   ||    Illegal adapter command. This might be caused by a microcode problem.
              |      See -- Heading 'MICMAI' unknown --.
SRN = d405   ||    The controller is not responding -> Exchange the FRUs for new FRUs.
SRN = d406   ||    Unexpected unit-powered-on status.
              |      Check that all controllers and disk drives that are attached to this
              |      adapter are powered on and ready. If they are not, go to -- Heading
              |      'START' unknown --. Otherwise exchange the FRU for a new FRU.
SRN = d407   ||    Time-out.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise ensure that the external serial link cable is
              |      connected correctly. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = d408   ||    Adapter to controller open serial link.
              |      If the reference display does not show a dash, go to -- Heading 'START'
              |      unknown --. Otherwise ensure that the external serial link cable is
              |      connected correctly. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = d409   ||    Unexpected adapter status -> Exchange the FRU for a new FRUs.
SRN = d40a   ||    Unexpected SCSI status -> Exchange the FRUs for new FRUs.
SRN = d40b   ||    Unexpected special status -> Exchange the FRUs for new FRUs.
SRN = d40c   ||    The data DMA has failed.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d40d   ||    The mailbox DMA has failed.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d40e   ||    Invalid mailbox pointer.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d40f   ||    DMA count error -> Exchange the FRU for  a new FRUs.
SRN = d410   ||    The controller address is out of range.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d411   ||    Quiesce controller time-out -> Exchange the FRUs for new FRUs.
SRN = d412   ||    A recoverable link error was found while running diagnostics.
              |      Ensure that the external serial link cable is connected correctly. Rerun
              |      the diagnostic test. If the problem remains, exchange the FRUs for new
              |      FRUs.
SRN = d413   ||    Dowbload microcode checksum failure - unexpected adapter status.
              |      Exchange the FRU for a new FRU.
SRN = d414   ||    Message to inactive tag -> Exchange the FRU for a new FRU.
SRN = d415   ||    Message on wrong link.
              |      Run diagnostics to each controller attached to this adapter in System
              |      Verification mode. If no error is found, exchange the FRU for a new FRU.
SRN = d416   ||    Invalid link message.
              |      Run diagnostics to each controller attached to this adapter in System
              |      Verification mode. If no error is found, exchange the FRU for a new FRU.
SRN = d417   ||    Irrecoverable link error.
              |      Ensure that the external serial link cable is connected correctly. If the
              |      problem remains, exchange the FRUs for new FRUs.
SRN = d418   ||    Invalid adapter parameters -> Exchange the FRU for a new FRU.
SRN = d419   ||    An unexpected 'Invalid Tag' status has been received.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d41d   ||    Unexpected 'DMA Disabled' status.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d420   ||    Unexpected 'Write Buffer Started' status -> Exchange the FRU for a new FRUs.
SRN = d421   ||    Mailbox terminated by internal reset -> Exchange the FRUs for new FRUs.
SRN = d422   ||    Adapter hardware error -> Exchange the FRU for a new FRU.
SRN = d424   ||    Unexpected 'Adapter Trace DMA Error' status.
              |      Exchange the FRU for a new FRU.
SRN = d425   ||    System DMA hung.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRUs.
SRN = d426   ||    An unexpected 'Trace Superseded' status has been received.
              |      Exchange the FRU for a new FRU.
SRN = d427   ||    An unexpected 'Previous Trace Dump Busy' status has been received.
              |      Exchange the FRU for a new FRU.
SRN = d4fe   ||    Adapter failure - invalid adapter status.
              |      This might be caused by a microcode problem.  See -- Heading 'MICMAI'
              |      unknown --.
SRN = d510   ||    An adapter command has timed out -> Exchange the FRU for a new FRU.
SRN = d550   ||    Invalid 'Last Tag' -> Exchange the FRU for a new FRU.
SRN = d570   ||    Invalid SCSI status -> Exchange the FRU for a new FRU.
SRN = d580   ||    I/O bus error.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = d601   ||    The system is not able to communicate with the adapter.
              |      Run diagnostics to the base system in Problem Determination mode. If no
              |      error is found, exchange the FRU for a new FRU.
SRN = dffe   ||    Invalid parameters have been sent to the ATU.
              |      You have a diagnostic error. Call your support center for assistance.
SRN = dfff   ||    Invalid return code from ATU.
              |      You have a diagnostic error. Call your support center for assistance.
%
              % 2) Validity Information
0 2 ^ 01      |    PQS 5 register data valid
0 2 ^ 02      |    Alert register valid
0 2 ^ 04      |    System DMA return code valid
0 2 ^ 08      |    Channel status register valid
%
              % 3) POS 5 Register
0 3 ^ 01     ||    Channel check
0 3 ^ 02     ||    Extended status available
0 3 ^ 04     ||    Type 1 error
0 3 ^ 08     ||    Invalid access
0 3 ^ 10     ||    Write data parity error
0 3 ^ 11     ||    Invalid status
%
              % 4) Alert Register
0 6 = 00     ||    Controler SCSI status
0 6 = 01     ||    Special Controller Status
0 6 = 02     ||    Command to an already active tag
0 6 = 03     ||    Download to an active tag not quiet
0 6 = 04     ||    Illegal adapter command
0 6 = 05     ||    Controller not responding/link not operational
0 6 = 06     ||    Unit powered on or reset occurred
0 6 = 07     ||    Timeout (only for query device)
0 6 = 08     ||    Link/controller is powered down or disconnected
0 6 = 09     ||    Tag purged after a reset operation from system
0 6 = 0a     ||    Unexpected SCSI status
0 6 = 0b     ||    Unexpected special status
0 6 = 0c     ||    Data DMA failed/adapter detected error
0 6 = 0d     ||    Mailbox DMA failed/adapter detected error
0 6 = 0e     ||    Invalid mailbox pointer/adapter detected error
0 6 = 0f     ||    DMA count error/mismatch
0 6 = 10     ||    Out of range controller
0 6 = 11     ||    Quiesce time out/controller did not respond
0 6 = 12     ||    Recoverable link error by retry
0 6 = 13     ||    Download microcode checksum failure
0 6 = 14     ||    Message to inactive tag
0 6 = 15     ||    Message on wrong link
0 6 = 16     ||    Invalid link message
0 6 = 17     ||    Unrecoverable link error
0 6 = 18     ||    Invalid adapter parameters
0 6 = 19     ||    Invalid tag
0 6 = 1a     ||    Not used
0 6 = 1b     ||    Not used
0 6 = 1c     ||    Not used
0 6 = 1d     ||    DMA disable, retrying
0 6 = 1e     ||    Not used
0 6 = 1f     ||    Not used
0 6 = 20     ||    Write buffer started
0 6 = 21     ||    Mailbox terminated by an internal reset
0 6 = 22     ||    Adapter hardware error
0 6 = 23     ||    Not used
0 6 = 24     ||    Trace data DMA error
0 6 = 25     ||    System DMA hung
0 6 = 26     ||    Trace event not changed, dump outstanding
0 6 = 27     ||    Not used
%
0 8 @ 16 @temp
              % 5) DMA Kernel Service RC = 0x%.4x = %d (decimal) ~temp ~temp
%
0 10 @ 16 @temp
              % 6) Channel Status Register = 0x%.4x = %d (decimal) ~temp ~temp

!serdasdc
%                     Serial DASD Controller
#set is_serdasd 1
#include scsi_gen

1 8 @  16     | 6) Unit Error Code (0x%.4x)

1 8 =  0000   |    No Error condition e.g. Unit Attention
1 8 =  0101  ||    Motor Speed not good.
              |    Motor Error.
              |    TMS detected motor error.
              |    POST - TMS320 failure.
1 8 =  0111  ||    Actuator over current
              |    Demodulator unsafe, (write gate)
              |    Demodulator unsafe, (read gate)
              |    Missing servo bits
              |    Index pulse + write gate
              |    Guard band + read gate
              |    Write inhibit + write gate
              |    Data MUX + write gate
              |    SID pulse + write gate
              |    TMS lost
              |    TMS detected error other than motor
              |    Seek error
              |    Not ready error, motor stopped
              |    Not ready error, powering off
              |    Post - Seek test failure
              |    Post - Track offset test failure
              |    Post - safety CCTS test failure
1 8 =  0121  ||    Head address parity error + write gate
              |    Head address parity error + read gate
              |    IPC error
              |    Unresetable level 1 interrupt
              |    Missing level 1 interrupt
              |    Post - CMPhilo A or PRML failure
1 8 =  0131  ||    Read gate + write gate
              |    PRML error
              |    WD sector + write gate
              |    AE not selected + write gate
              |    AE not selected + read gate
              |    Unresetable level 0 interrupt
              |    Missing level 0 interrupt
              |    Post - WD10c00 failure
1 8 =  0141  ||    AE disconnected
              |    AE error chip 2
              |    AE error chip 1
              |    Head or AE change + WR gate
1 8 =  01c8  ||    Data overruns/underruns
1 8 =  01fe  ||    FCM detected error
1 8 =  0281  ||    Sector auto-reallocated or re-written
1 8 =  0282  ||    Sector exceeded reassign threshold, with ARRE off.
              |    Auto reassign failed having started.
1 8 =  0283  ||    Exceeded error thresholding limit.
1 8 =  0284  ||    A different sector has been reassigned during Manual Reassign
1 8 =  0285  ||    Unrecoverable read error, without an equipment error
1 8 =  0286  ||    Sector switch threshold exceeded during reassign
1 8 =  0287  ||    Reserved area bad, (read or write)
1 8 =  0288  ||    Hard media error mid auto-reassign
              |    Hard media error mid manual-reassign (even if switched sector)
              |    Unreassignable hard error during verify phase of manual reassign
1 8 =  0290  ||    Reassign target sector switch required in manual, ARRE disabled
              |    Hard media error during verify phase of auto-reassign
              |    Hard media error during verify phase of manual-reassign
1 8 =  0291  ||    Error thresholding analysis gives random sectors
1 8 =  02ff  ||    Medium type error detected but none located (code bug)
1 8 =  0301  ||    DMA Bus parity
              |    DMA Uncorrectable ECC while fetching from DRAM
              |    Timer parity error
1 8 =  0321  ||    Ctrl-DASD LINK Controller Retry limit exceeded.
              |    Ctrl-DASD LINK Packet rejected (controller end)
              |    Ctrl-DASD LINK Invalid retry status
              |    Ctrl-DASD LINK invalid state (detected by controller)
1 8 =  0322  ||    Ctrl-DASD LINK Remote node disabled (DASD end)
              |    Ctrl-DASD LINK Link reset failed
              |    Ctrl-DASD LINK Timeout without waiting for DASD to go to enabled
1 8 =  0323  ||    Ctrl-DASD LINK Hardware error (controller end)
1 8 =  0324  ||    Ctrl-DASD LINK Permanent line fault
1 8 =  0351  ||    Ctrl-DASD LINK Timeout waiting for DASD to go to disabled
              |    Ctrl-DASD LINK Packet rejected (DASD end)
1 8 =  0353  ||    Ctrl-DASD LINK Hardware error (DASD end)
1 8 =  0380  ||    Controller-Adapter communications fault
1 8 =  0381  ||    Spinnaker type 1 error (hardware checker)
1 8 =  0382  ||    Spinnaker type 1 error (microcode insane point)
1 8 =  0391  ||    No defect spare data sectors available whilst auto-reassigning
              |    No defect spare data sectors available whilst system reassigning
1 8 =  03a0  ||    Degraded DASD - Not 512 byte format or Format failed
1 8 =  03a1  ||    Degraded DASD - Previous reassign failed
1 8 =  03a3  ||    Degraded DASD - Read write verification
1 8 =  03a4  ||    Degraded DASD - DASD POST had failed
1 8 =  03a5  ||    Degraded DASD - Unreadable configuration sector
1 8 =  03a6  ||    Degraded DASD - Motor stopped
1 8 =  03a7  ||    Degraded DASD - Unsupported DASD
1 8 =  03c2  ||    DASD link order timeout
1 8 =  03c3  ||    DASD link message sent timeout
1 8 =  03f0  ||    Undefined DASD failure
1 8 =  03fd  ||    DMA Illegal address (probably code bug)
              |    DMA interlock error (probably code bug)
1 8 =  03fe  ||    Invalid field in CDB
              |    Invalid field in parameter list
              |    Overlapped commands attempted
              |    Invalid command
              |    LBA out of range
              |    Parameter list length error
1 8 =  03ff  ||    Controller type error but none located

!serdasdd
%                     Serial DASD 857Mb F Hardfile Sense Data Analysis
#include serdasd_all

!serdasdd1
%                     Serial DASD 857Mb C Hardfile Sense Data Analysis
#include serdasd_all

!serdasdd2
%                     Serial DASD 471Mb F Hardfile Sense Data Analysis
#include serdasd_all

!serdasdd3
%                     Serial DASD 471Mb C Hardfile Sense Data Analysis
#include serdasd_all

!serdasdd4
%                     Serial DASD 1.07Gb F Hardfile Sense Data Analysis
#include serdasd_all

!serdasdd5
%                     Serial DASD 1.07Gb C Hardfile Sense Data Analysis
#include serdasd_all

!serdasd_all
* Sense information for all the Serial DASD hardfiles.

* PROCESS GENERAL SERIAL INFORMATION

#set is_serdasd 1
#include scsi_gen

* Local variables
#define valid_info
#define sense_key
#define SKSV
#define action_code
#define dasd_status
#define msg_code
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
~endif

0 27  @       |    c) Additional Sense Length = %d (decimal)

0 28 @ 32     |    d) Logical Block Address not reassigned 0x%.8x
              |         If a Reassigns Blocks command then this value is the LBA
              |         from the defect descriptor blk that was not reassigned.
              |         SCSI defines this as the 1st decriptor not reassigned,
              |         but Harrier only supports 1 block per Reassign command.

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE
              |         There is no sense key info to be reported for the unit
              |         This code occurs for a successfully completed command

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
  1 0 = 1500 ||         Random positioning error (recoverable)
  1 0 = 1700 ||         Data recovered without ECC
  1 0 = 1705 ||         Data recovered using previous sector ID
  1 0 = 1801 ||         Data recovered without ECC and retries
  1 0 = 1802 ||         Data recovered and data auto-reallocated
  1 0 = 3200 ||         No defect spare location avail. (during auto reassign)
  1 0 = 3201 ||         Defect list update failed
  1 0 = 5b01 ||         Threshold conditions met
  1 0 = 8400 ||         Internal DASD failure (recovered)
  1 0 = 8401 ||         DASD timeout detected
  1 0 = 8402 ||         Internal DASD failure recovered with a TMS reset
  1 0 = 8500 ||         Reassignment recommended, excessive recovery required.
              |           Bytes 3-6 define the LBA to be reassigned.

~elseif (sense_key == 2)
             ||       NOT READY
  1 0 = 0400 ||         Not ready, cause not reported
  1 0 = 0401 ||         Not ready, device is powering on
  1 0 = 0402 ||         Not ready, Start unit command required
  1 0 = 0403 ||         Not ready, Manual intervention required
  1 0 = 0404 ||         Not ready, Format in progress
  1 0 = 3100 ||         Medium format corrupted
  1 0 = 3101 ||         Format command failed - Format Unit required
  1 0 = 4080 ||         Internal DASD checkout failure
  1 0 = 4081 ||         DASD Read/Write test failure
  1 0 = 4c00 ||         LUN failed self configuration
  1 0 = 8200 ||         Command rejected, controller not quiet

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
  1 0 = 1100 ||         Unrecovered read error
  1 0 = 1401 ||         Record not found
  1 0 = 1600 ||         Data sync error
  1 0 = 1900 ||         Defect list error
  1 0 = 8501 ||         Requested block not reassigned, alternate LBA has been reassigned
  1 0 = 8502 ||         Requested block not reassigned.
              |           Alternate LBA defined in bytes 3-6 should be reassigned.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
  1 0 = 0200 ||         No seek complete
  1 0 = 0800 ||         Controller - DASD link error
  1 0 = 0801 ||         Controller - DASD link timeout
  1 0 = 0802 ||         Controller - DASD link CRC error
  1 0 = 0900 ||         Track following error
  1 0 = 1104 ||         Unrecovered read error - auto-reallocation failed
  1 0 = 1500 ||         Random positioning error (unrecoverable)
  1 0 = 1502 ||         Positioning error detected by incorrect ID
  1 0 = 3200 ||         No defect spare location avail (during Reassign Blk Cmd)
  1 0 = 4400 ||         Internal controller failure
  1 0 = 4700 ||         Adapter link CRC threshold error
  1 0 = 8300 ||         Reserved area of disk bad (both copies)
  1 0 = 8400 ||         Internal DASD failure (unrecovered)

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
  1 0 = 1a00 ||         Parameter list length error
  1 0 = 2000 ||         Invalid command
  1 0 = 2100 ||         LBA out of range
  1 0 = 2400 ||         Invalid field in CDB
  1 0 = 2500 ||         Unsupported LUN
  1 0 = 2600 ||         Invalid field in parameter list

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
  1 0 = 2800 ||         Not ready to ready transition
  1 0 = 2900 ||         Power on or link total reset occurred
  1 0 = 2a01 ||         Mode select parameters changed by another Host Adapter
  1 0 = 3f00 ||         Operating conditions have changed
  1 0 = 3f01 ||         Microcode has been changed (ctrl or DASD)
  1 0 = 3f03 ||         Inquiry data changed.
  1 0 = 8100 ||         Ready following a type 1 check

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND
  1 0 = 4e00 ||         Overlapped commands attempted
  1 0 = 8000 ||         Command aborted due to Early Power off Warning

~else
             ||       NOT IMPLEMENTED
              |         Undefined sense key value.
~endif

1 3 @ 24      |    f) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)
  ~endif
~endif

1 6 @         | 7) Host Action Code (0x%.4x)
1 6 ^  f0     |    Send a message to the operator.
1 6 ^  70     |    Write a sense record to the error log

1 6 @ @temp_byte
action_code := temp_byte & 0xf

action_code = 0  ||    No error recovery action required
action_code = 01 ||    Retry not recommended
action_code = 02 ||    Retry the failing operation once
action_code = 03 ||    Retry the failing operation 10 times
action_code = 04 ||    Reset controller and retry the failing operation
action_code = 05 ||    Quiesce the controller and retry the failing operation
action_code = 06 ||    Reset the DASD and retry the failing operation
action_code = 07 ||    Quiesce the DASD and retry the failing operation
action_code = 08 ||    Verify the Controller and DASD
action_code = 09 ||    Recommend reassignment of block (ESOFT)
action_code = 0a ||    Request reassignment of block (EMEDIA)
action_code = 0b ||    Absolute reset the DASD and retry the failing operation
action_code = 0c ||    Verify the DASD
action_code = 0d ||    Reassignment recommended by reassign code
action_code = 0e ||    Not used
action_code = 0f ||    Reset the controller and retry (no valid sense)

1 7 @         | 8) Controller Microcode Error (0x%.4x)

1 7 = 01     ||    Format Unit, invalid field in command descriptor block
1 7 = 02     ||    Format Unit, invalid field in parameter list
1 7 = 03     ||    Format Unit, Not ready format in progress
1 7 = 04     ||    Command processing, overlapped commands attempted
1 7 = 05     ||    Command processing, reset occurred
1 7 = 06     ||    Command processing, Microcode has changed
1 7 = 07     ||    Command processing, Mode select params changed by other host adapter
1 7 = 08     ||    Command processing, Not ready to ready transition (FMTUA)
1 7 = 09     ||    Command processing, Not ready to ready transition (PKCHUA)
1 7 = 0a     ||    Command validation, Invalid command
1 7 = 0b     ||    Command validation, Invalid field in CDB (R/W commands)
1 7 = 0c     ||    Command validation, Invalid field in CDB (Non-R/W commands)
1 7 = 0d     ||    Command validation, Invalid field in CDB (Non-R/W commands)
1 7 = 0e     ||    Command validation, LBA out of range
1 7 = 0f     ||    Request sense, Unsupported LUN
1 7 = 10     ||    Read degraded, Not ready start unit command required
1 7 = 11     ||    Read degraded, LUN failed self-configuration
1 7 = 12     ||    Read degraded, Medium format corrupted
1 7 = 13     ||    Mode select, Invalid field in parameter list
1 7 = 14     ||    Mode sense, Invalid field in command descriptor block
1 7 = 15     ||    Read capacity, LBA out of range
1 7 = 16     ||    Read capacity, Invalid field in command descriptor block
1 7 = 17     ||    Start/stop unit, Invalid field in command descriptor block
1 7 = 18     ||    Start/stop unit, Not ready DASD is powering on
1 7 = 19     ||    Start/stop unit, Not ready start unit command required
1 7 = 1a     ||    Send diagnostics, Invalid field in parameter list
1 7 = 1b     ||    Send diagnostics, Invalid field in command descriptor block
1 7 = 1c     ||    Send diagnostics, Parameter list length error
1 7 = 1d     ||    Write VPD, Invalid field in parameter block
1 7 = 1e     ||    Write buffer, Invalid field in command descriptor block
1 7 = 1f     ||    Auto reassign, Reassign recommended
1 7 = 20     ||    Auto reassign, failed having started
1 7 = 21     ||    Manual reassign, Alternate LBA should be reassigned
1 7 = 22     ||    DASD download failure, not ready start unit command required
1 7 = 23     ||    Reassign blocks, LBA out of range
1 7 = 24     ||    Reassign blocks, Invalid field in parameter list
1 7 = 25     ||    Reassign blocks, Reassigned downstream sector
1 7 = 26     ||    Reassign blocks, No defect spare location available
1 7 = 27     ||    Reassign blocks, Defect list update failed
1 7 = 28     ||    Ctrl-DASD LINK controller Retry limit exceeded
1 7 = 29     ||    Ctrl-DASD LINK Permanent line fault
1 7 = 2a     ||    Ctrl-DASD LINK Remote node disabled (DASD end)
1 7 = 2b     ||    Ctrl-DASD LINK Link reset failed
1 7 = 2c     ||    Ctrl-DASD LINK Hardware error (controller end)
1 7 = 2d     ||    Ctrl-DASD LINK Hardware error (DASD end)
1 7 = 2e     ||    Ctrl-DASD LINK Packet rejected (controller end)
1 7 = 2f     ||    Ctrl-DASD LINK Packet rejected (DASD end)
1 7 = 30     ||    Ctrl-DASD LINK Invalid retry status
1 7 = 31     ||    Ctrl-DASD LINK Timeout waiting for DASD to go to disabled
1 7 = 32     ||    Ctrl-DASD LINK Timeout waiting for DASD to go to ready
1 7 = 33     ||    Ctrl-DASD LINK Invalid link state at controller end
1 7 = 34     ||    Controller analysis, Internal DASD error
1 7 = 35     ||    Device analysis, Internal DASD error
1 7 = 36     ||    Error recovery, Data recovered without ECC
1 7 = 37     ||    Device analysis, DASD returned illegal order
1 7 = 38     ||    Device analysis, Not ready manual intervention is required
1 7 = 39     ||    Device analysis, Not ready DASD is powering on
1 7 = 3a     ||    Device analysis, Not ready start unit command is required
1 7 = 3b     ||    ECC recovery, Data recovered with ECC and retries
1 7 = 3c     ||    ID recovery, Data recovered using previous sector id
1 7 = 3d     ||    ID recovery, Record not found
1 7 = 3e     ||    Seek error recovery, Random positioning error
1 7 = 3f     ||    Internal DASD failure
1 7 = 40     ||    DASD hardware error recovery, Internal DASD error
1 7 = 41     ||    Trace stop, invalid field in command descriptor block
1 7 = 42     ||    Reading reserved area, Reserved area of disk bad
1 7 = 43     ||    Writing reserved area, Reserved area of disk bad
1 7 = 44     ||    Reading critical area, LUN failed self configuration
1 7 = 45     ||    Data check recovery, Unrecovered read error
1 7 = 46     ||    Command processing, ready following a type 1 check (software)
1 7 = 47     ||    Command processing, ready following a type 1 check (hardware)
1 7 = 48     ||    Error thresholding, threshold conditions met
1 7 = 49     ||    Send diagnostics, Internal DASD checkout failure
1 7 = 4a     ||    Send diagnostics, Start unit command required
1 7 = 4b     ||    Send diagnostics, Not ready DASD r/w test failure
1 7 = 4c     ||    Send diagnostics, Invalid command
1 7 = 4d     ||    Receive diagnostics, invalid field in command
1 7 = 4e     ||    Write buffer, Invalid command
1 7 = 4f     ||    Write buffer, Overlapped commands attempted
1 7 = 50     ||    Write buffer, Invalid field in parameter list
1 7 = 51     ||    Write buffer, Invalid field in CDB. (download)
1 7 = 52     ||    Error recovery, Data sync error
1 7 = 53     ||    ID recovery, LBA out of range
1 7 = 54     ||    Format unit, format command failed
1 7 = 55     ||    Copy and reassign, medium format corrupted
1 7 = 56     ||    Idle task, not ready device is powering on
1 7 = 57     ||    Idle task, not ready cause not reportable e.g. DASD powered off
1 7 = 58     ||    Device initialisation, Not ready cause not reportable
1 7 = 59     ||    Device initialisation, Not ready start unit required
1 7 = 5a     ||    Device initialisation, Not ready medium format corrupted
1 7 = 5b     ||    Device initialisation, format command failed - format unit required
1 7 = 5c     ||    Device initialisation, Internal DASD checkout failure
1 7 = 5d     ||    Device initialisation, DASD r/w test failure
1 7 = 5e     ||    Device initialisation, LUN failed self configuration
1 7 = 5f     ||    Device initialisation, DASD incorrectly formatted
1 7 = 60     ||    Seek request, no seek complete
1 7 = 61     ||    Internal DASD failure found during a non read/write order
1 7 = 62     ||    No response from the DASD
1 7 = 63     ||    Internal DASD failure found during general error processing
1 7 = 64     ||    Random positioning error found during general error processing
1 7 = 65     ||    Extend order failed
1 7 = 66     ||    Manual Reassign did not start, too many soft errors
1 7 = 67     ||    Request Sense, Inquiry data changed
1 7 = 68     ||    Timeout while sending link order
1 7 = 69     ||    Controller/Adapter communications fault occurred
1 7 = 6a     ||    ID deduction nonsense LBAs or bad physicals during ID sorting
1 7 = 6b     ||    Unsupported DASD
1 7 = 6c     ||    Auto reassigned/re-written
1 7 = 6d     ||    Unexpected DASD message received
1 7 = 6e     ||    Recovered DASD timeout
1 7 = 6f     ||    Internal DASD failure recovered with a TMS reset
1 7 = f9     ||    Error thresholding, random sector data check threshold exceeded
1 7 = fa     ||    Manual reassign, could not write to DTBR in reserved area
1 7 = fb     ||    Manual reassign, Hard error prevented reassign starting
1 7 = fc     ||    Manual reassign, failed having started

1 8 @  16     | 9) Unit Error Code (0x%.4x)

1 8 =  0000   |    No Error condition e.g. Unit Attention
1 8 =  0101  ||    Motor Speed not good.
              |    Motor Error.
              |    TMS detected motor error.
              |    POST - TMS320 failure.
1 8 =  0111  ||    Actuator over current
              |    Demodulator unsafe, (write gate)
              |    Demodulator unsafe, (read gate)
              |    Missing servo bits
              |    Index pulse + write gate
              |    Guard band + read gate
              |    Write inhibit + write gate
              |    Data MUX + write gate
              |    SID pulse + write gate
              |    TMS lost
              |    TMS detected error other than motor
              |    Seek error
              |    Not ready error, motor stopped
              |    Not ready error, powering off
              |    Post - Seek test failure
              |    Post - Track offset test failure
              |    Post - safety CCTS test failure
1 8 =  0121  ||    Head address parity error + write gate
              |    Head address parity error + read gate
              |    IPC error
              |    Unresetable level 1 interrupt
              |    Missing level 1 interrupt
              |    Post - CMPhilo A or PRML failure
1 8 =  0131  ||    Read gate + write gate
              |    PRML error
              |    WD sector + write gate
              |    AE not selected + write gate
              |    AE not selected + read gate
              |    Unresetable level 0 interrupt
              |    Missing level 0 interrupt
              |    Post - WD10c00 failure
1 8 =  0141  ||    AE disconnected
              |    AE error chip 2
              |    AE error chip 1
              |    Head or AE change + WR gate
1 8 =  01c8  ||    Data overruns/underruns
1 8 =  01fe  ||    FCM detected error
1 8 =  0281  ||    Sector auto-reallocated or re-written
1 8 =  0282  ||    Sector exceeded reassign threshold, with ARRE off.
              |    Auto reassign failed having started.
1 8 =  0283  ||    Exceeded error thresholding limit.
1 8 =  0284  ||    A different sector has been reassigned during Manual Reassign
1 8 =  0285  ||    Unrecoverable read error, without an equipment error
1 8 =  0286  ||    Sector switch threshold exceeded during reassign
1 8 =  0287  ||    Reserved area bad, (read or write)
1 8 =  0288  ||    Hard media error mid auto-reassign
              |    Hard media error mid manual-reassign (even if switched sector)
              |    Unreassignable hard error during verify phase of manual reassign
1 8 =  0290  ||    Reassign target sector switch required in manual, ARRE disabled
              |    Hard media error during verify phase of auto-reassign
              |    Hard media error during verify phase of manual-reassign
1 8 =  0291  ||    Error thresholding analysis gives random sectors
1 8 =  02ff  ||    Medium type error detected but none located (code bug)
1 8 =  0301  ||    DMA Bus parity
              |    DMA Uncorrectable ECC while fetching from DRAM
              |    Timer parity error
1 8 =  0321  ||    Ctrl-DASD LINK Controller Retry limit exceeded.
              |    Ctrl-DASD LINK Packet rejected (controller end)
              |    Ctrl-DASD LINK Invalid retry status
              |    Ctrl-DASD LINK invalid state (detected by controller)
1 8 =  0322  ||    Ctrl-DASD LINK Remote node disabled (DASD end)
              |    Ctrl-DASD LINK Link reset failed
              |    Ctrl-DASD LINK Timeout without waiting for DASD to go to enabled
1 8 =  0323  ||    Ctrl-DASD LINK Hardware error (controller end)
1 8 =  0324  ||    Ctrl-DASD LINK Permanent line fault
1 8 =  0351  ||    Ctrl-DASD LINK Timeout waiting for DASD to go to disabled
              |    Ctrl-DASD LINK Packet rejected (DASD end)
1 8 =  0353  ||    Ctrl-DASD LINK Hardware error (DASD end)
1 8 =  0380  ||    Controller-Adapter communications fault
1 8 =  0381  ||    Spinnaker type 1 error (hardware checker)
1 8 =  0382  ||    Spinnaker type 1 error (microcode insane point)
1 8 =  0391  ||    No defect spare data sectors available whilst auto-reassigning
              |    No defect spare data sectors available whilst system reassigning
1 8 =  03a0  ||    Degraded DASD - Not 512 byte format or Format failed
1 8 =  03a1  ||    Degraded DASD - Previous reassign failed
1 8 =  03a3  ||    Degraded DASD - Read write verification
1 8 =  03a4  ||    Degraded DASD - DASD POST had failed
1 8 =  03a5  ||    Degraded DASD - Unreadable configuration sector
1 8 =  03a6  ||    Degraded DASD - Motor stopped
1 8 =  03a7  ||    Degraded DASD - Unsupported DASD
1 8 =  03c2  ||    DASD link order timeout
1 8 =  03c3  ||    DASD link message sent timeout
1 8 =  03f0  ||    Undefined DASD failure
1 8 =  03fd  ||    DMA Illegal address (probably code bug)
              |    DMA interlock error (probably code bug)
1 8 =  03fe  ||    Invalid field in CDB
              |    Invalid field in parameter list
              |    Overlapped commands attempted
              |    Invalid command
              |    LBA out of range
              |    Parameter list length error
1 8 =  03ff  ||    Controller type error but none located

1 16 @        | 10) DASD status byte (0x%.4x)
1 16 @ @temp_byte
dasd_status := temp_byte & 0xe0
dasd_status := dasd_status >> 5

* If bits 7-5 of DASD status are 100 or 001
~if (dasd_status == 0x4) || (dasd_status == 0x1)

             %%    DASD Equipment Check or Seek error
  1 10 @     ||    PRML Error Status (0x%.4x)
  1 10 ^ 80   |      NOT USED
  1 10 ^ 40   |      NOT USED
  1 10 ^ 20   |      Read code error
  1 10 ^ 10   |      Write code error
  1 10 ^ 08   |      Index error
  1 10 ^ 04   |      Byte count lost
  1 10 ^ 02   |      VFO out of capture range
  1 10 ^ 01   |      Sync byte not found

  1 11 @     ||    Safety Status 0 (0x%.4x)
  1 11 ^ 80   |      AE disconnected
  1 11 ^ 40   |      AE error chip 2
  1 11 ^ 20   |      AE error chip 1
  1 11 ^ 10   |      Head or AE change
  1 11 ^ 08   |      SID pulse
  1 11 ^ 04   |      Actuator over current
  1 11 ^ 02   |      Motor error
  1 11 ^ 01   |      Speed not good

  1 12 @     ||    Safety Status 1 (0x%.4x)
  1 12 ^ 80   |      Drive fault
  1 12 ^ 40   |      NOT USED
  1 12 ^ 20   |      Read gate
  1 12 ^ 10   |      Write gate
  1 12 ^ 08   |      Missing servo bits
  1 12 ^ 04   |      Demod unsafe
  1 12 ^ 02   |      Index pulse
  1 12 ^ 01   |      Guardband

  1 13 @     ||    Safety Status 2 (0x%.4x)
  1 13 ^ 80   |      NOT USED
  1 13 ^ 40   |      Even parity error
  1 13 ^ 20   |      PRML error
  1 13 ^ 10   |      WD sector
  1 13 ^ 08   |      Data Mux
  1 13 ^ 04   |      AE not selected
  1 13 ^ 02   |      Write inhibit
  1 13 ^ 01   |      TMS lost

  1 14 @     ||    Safety RAS 0 (0x%.4x)
  1 14 ^ 80   |      NOT USED
  1 14 ^ 40   |      NOT USED
  1 14 ^ 20   |      NOT USED
  1 14 ^ 10   |      NOT USED
  1 14 ^ 08   |      Actuator write inhibit
  1 14 ^ 04   |      ID or home
  1 14 ^ 02   |      Missing SID
  1 14 ^ 01   |      AWI when read

  1 15 @     ||    FCM Microcode detected error (0x%.4x)
  1 15 = 01  ||      No data sync found
  1 15 = 02  ||      ECC check
  1 15 = 03  ||      ECC check during settle
  1 15 = 04  ||      10C00 parity error detected
  1 15 = 05  ||      Track switch required
  1 15 = 06  ||      No sector found.
              |        Due to 2.2 rev timeout or LBA + offset byte calculation from ID field
              |        read.
  1 15 = 07  ||      Seek error detected
              |        Cylinder byte in ID field mismatch with low byte of current cylinder)
  1 15 = 08  ||      Target sector not on this track
  1 15 = 09  ||      Operation aborted
  1 15 = 0a  ||      Condition not met for condition read or write
  1 15 = 0b  ||      Software TMS timeout (usually on command acceptance)
  1 15 = 0c  ||      TMS timeout by 8051 timeout (usually on command completion status)
  1 15 = 0d  ||      TMS command rejected. Unexpected command acceptance status
  1 15 = 0e  ||      TMS unexpected command completion status
  1 15 = 0f  ||      Soft TMS failure
  1 15 = 10  ||      Hard TMS failure
  1 15 = 11  ||      Track switch bad completion
  1 15 = 12  ||      Drive fault detected
  1 15 = 13  ||      10C00 sequencer load error.
              |        Caused by checksum failure on the requested load or invalid load request
  1 15 = 14  ||      RAM code checksum failure
  1 15 = 15  ||      IPC timeout.
              |        Set during a code download or write RAM order when an expected data
              |        packet has not arrived within 5 ms.
  1 15 = 16  ||      Write RAM excess data received
  1 15 = 17  ||      ID field no sync found error
  1 15 = 18  ||      ID field CRC check
  1 15 = e1  ||      SCSI CMPHILO POST1 failure
  1 15 = e2  ||      WD 10C00 POST1 failure
  1 15 = e3  ||      TMS BATs handshake failure
  1 15 = e4  ||      Recalibrate failure or no auto motor start.
              |        If POST_FAILURE set (in DASD_STATUS byte returned with a
              |          Read status order) Then Recalibrate failure.
              |        If POST_FAILURE clear Then
              |          Good completion of POST1 with no auto motor start.
  1 15 = e5  ||      Seek test POST2 failure
  1 15 = e6  ||      Safety Logic POST2 failure
  1 15 = e7  ||      Track offset test POST2 failure
  1 15 = e8  ||      Good completion of POST2 after motor start
  1 15 = ef  ||      File is probably unformatted.
              |        This code is set if the seek of POST2 fails to successfully read any
              |        ID field from a track.

  1 16 @      |    DASD Status = 0x%.4x (see Serial Redwing specification for details)

  1 17 @     ||    Interface Protocol Chip (IPC) Exceptions
  1 17 ^ 80   |      NOT USED
  1 17 ^ 40   |      NOT USED
  1 17 ^ 20   |      NRZ0 error
  1 17 ^ 10   |      Data overrun
  1 17 ^ 08   |      Data under run
  1 17 ^ 04   |      Data under run, sector unchanged (no write)
  1 17 ^ 02   |      Write gate error
  1 17 ^ 01   |      IPC parity error (an OR of all IPC parity checks)

  1 18 @ 16  ||    TMS Status (0x%.4x)
 1 18 ^ 8000 ||      Status Message
 1 18 ^ 4000 ||      Hard failure
 1 18 ^ 2000 ||      Function Busy (Set to zero when function completed)
 1 18 ^ 1000 ||      Soft failure

  1 18 @ 16   |    @msg_code
  msg_code := msg_code >> 4
  msg_code := msg_code & 0xff

  msg_code = 00 ||      Command/order complete
  msg_code = 10 ||      PES interrupt missing
  msg_code = 20 ||      Settle timeout
  msg_code = 45 ||      Retract failed
  msg_code = 47 ||      Carriage latch/actuator stuck at ID
  msg_code = 48 ||      Requested seek track number is too high
  msg_code = 51 ||      Motor locked up
  msg_code = 52 ||      Motor start accepted
  msg_code = 54 ||      Motor rotor stuck
  msg_code = 57 ||      Demod sync failed
  msg_code = 59 ||      Motor cold start timeout
  msg_code = 5e ||      Motor hot start timeout
  msg_code = 62 ||      Motor soft phase error
  msg_code = 63 ||      Motor stop complete
  msg_code = 64 ||      Motor stop accepted
  msg_code = 65 ||      Motor soft velocity error
  msg_code = 66 ||      Demod failed or locked in wrong palce
  msg_code = 6b ||      Motor demod sync erratic
  msg_code = 6d ||      Current sense circuit failed
  msg_code = 6e ||      Motor open circuit
  msg_code = 6f ||      Motor short circuit
  msg_code = 70 ||      Seek timeout
  msg_code = 73 ||      Demodulator became unsafe
  msg_code = 7a ||      Motor stuck or stalled during start
  msg_code = 7f ||      Motor interrupt failed
  msg_code = 81 ||      Manufacturing mode set up
  msg_code = 84 ||      Actuator ID guard band violation
  msg_code = 85 ||      Actuator OD guard band violation
  msg_code = 86 ||      Shift head accepted
  msg_code = 90 ||      Reject command
  msg_code = a2 ||      Recalibrate accepted
  msg_code = b0 ||      Actuator maximum velocity exceeded
  msg_code = d0 ||      3 consecutive PES rejections
  msg_code = fa ||      Command accepted
  msg_code = fe ||      Recalibrate complete

* If bits 7-5 of DASD status are 011.
~elseif (dasd_status == 0x3)

             %%    DASD Medium error
  1 12 @ 32   |    Physical block address (0x%.8x)
  1 12 @      |      Cylinder high address  = 0x%.2x
  1 13 @      |      Cylinder low address   = 0x%.2x
  1 14 @      |      Logical head address   = 0x%.2x
  1 15 @      |      Physical sector number = 0x%.2x
  1 16 @      |    DASD status (0x%.2x)

* If bits 7-5 of DASD status are 111.
~elseif (dasd_status == 0x7)

             %%    Controller Equipment Check Error

  1 10 @     ||    Controller to DASD Serial Link Status
  1 10 ^ 80   |      No frames
  1 10 ^ 40   |      Line fault

  1 10 @      |    @temp_byte
  temp_byte := temp_byte >> 4
  temp_byte := temp_byte & 0x3
              %    Link state = %d ~temp_byte

  1 10 @      |    @temp_byte
  temp_byte := temp_byte >> 2
  temp_byte := temp_byte & 0x3
              %    Transmit PSN = %d ~temp_byte

  1 10 @      |    @temp_byte
  temp_byte := temp_byte & 0x3
              %    Receive PSN = %d ~temp_byte

  1 11 @     ||    Controller to DASD Serial Link Error Status
  1 11 ^ 80   |      Illegal frame
  1 11 ^ 40   |      Protocol error
  1 11 ^ 20   |      CRC check
  1 11 ^ 10   |      Sequence error
  1 11 ^ 08   |      Packet reject
  1 11 ^ 04   |      ACK time-out
  1 11 ^ 02   |      No frames latched
  1 11 ^ 01   |      Line fault latched

  1 12 @     ||    Controller to DASD Serial Link HDW Error Status
  1 12 ^ 80   |      Serialise parity check
  1 12 ^ 40   |      Pointer/counter parity check
  1 12 ^ 20   |      Link FSM parity check
  1 12 ^ 10   |      Packet buffer data parity check
  1 12 ^ 08   |      Packet status data parity check
  1 12 ^ 04   |      DMA FSM detected error
  1 12 ^ 02   |      NOT USED
  1 12 ^ 01   |      NOT USED

  1 17 @     ||    DMA Error Register
  1 17 ^ 80   |      Illegal address
  1 17 ^ 40   |      DMA bus parity
  1 17 ^ 20   |      Uncorrectable error
  1 17 ^ 10   |      Interlock error

  1 18 @     ||    Timer Control Register
  1 18 ^ 80   |      Run
  1 18 ^ 40   |      Interrupt enable
  1 18 ^ 20   |      Zero
  1 18`^ 10   |      Error

~endif

!200mb
%                     200 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf

* The 200Mb documentation does not specify how to use ILI (incorrect_len)
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Command Specific Information: 0x%.8x

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE - There is no sense key information to be reported for the
              |         logical unit.

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
              |         The last command was completed successfully with some recovery action
              |         performed by the file.

~elseif (sense_key == 2)
             ||       The logical unit addressed cannot be addressed.

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
              |         The command terminated with a nonrecoverable error condition caused
              |         by a flaw in the media or by an error in the recorded data.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
              |         A file detected an unrecoverable hardware error while performing a
              |        command or during a diagnostic test.

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
              |         There was an illegal parameter in the command descriptor block or
              |         additional parameter supplied as data. If an invalid parameter is
              |         found in the CDB, then the command is terminated without altering
              |         the medium. If an invalid parameter is found in parameters supplied
              |         as data, then the file might hasve altered the medium.

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
              |         Indicates that the file entered in the Unit Attention Condition.

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND - The file aborted the command.

~elseif (sense_key == 0xe)
             ||       MISCOMPARE

~else
             ||       NOT IMPLEMENTED - Undefined sense key value.
~endif

1 0 = 0000    |       No additional sense information.
1 0 = 0100   ||       No index or sector.
1 0 = 0200   ||       No seek complete.
1 0 = 0300   ||       Write fault.
1 0 = 0400   ||       Drive not ready. Cause not reportable.
1 0 = 0401   ||       Drive not ready. In process of becoming ready.
1 0 = 0402   ||       Drive not ready. Initializing command required. (Start Motor)
1 0 = 0404   ||       Format in progress.
1 0 = 0800   ||       Communication failure.
1 0 = 0801   ||       Communication time out.
1 0 = 0802   ||       Communication parity error.
1 0 = 0900   ||       Track following error.
1 0 = 1000   ||       ID CRC error.
1 0 = 1100   ||       UNRECOVERED read error.
1 0 = 1401   ||       Record not found.
1 0 = 1500   ||       Seek positioning error.
1 0 = 1600   ||       Data synchronization mark error.
1 0 = 1700   ||       Recovered read data without ECC applied.
1 0 = 1800   ||       Recovered read data with ECC applied.
1 0 = 1900   ||       Defect list error.
              |         Occurs when a data error is detected while reading the manufacturing
              |         defect list or while reading or writing the grown defect list.
1 0 = 1901   ||       Defect list not available.
1 0 = 1a00   ||       Parameter list length error.
              |         The number of parameters supplied is not equal to the value the
              |         command allows.
1 0 = 1b00   ||       Synchronous data transfer error.
1 0 = 1c00   ||       Defect list not found.
1 0 = 1d00   ||       Miscompare during verify operation.
1 0 = 2000   ||       Invalid command operation code.
              |         This code is also returned when an unsupported cmd code is received.
1 0 = 2100   ||       Logical block address out of range.
1 0 = 2400   ||       Invalid field in CDB.
1 0 = 2500   ||       Unsupportted LUN. The file supports LUN 0 only.
1 0 = 2600   ||       Invalid field in the parameter list.
1 0 = 2601   ||       Parameter not supported.
1 0 = 2602   ||       Parameter value invalid.
1 0 = 2900   ||       Power on reset or Bus device reset occurred.
1 0 = 2a00   ||       Mode Select parameters changed.
1 0 = 3100   ||       Medium format corrupted.
              |         A format operation was interrupted (power down, reset) prior to
              |         completion of a Format Unit command. The Format Unit command should be
              |         re-issued and must complete successfully for this error condition to
              |         be removed.
1 0 = 3101   ||       Format command failed.
1 0 = 3200   ||       No defect spare location available.
              |         The Reassign Block command can not procedd the process because all
              |         available spare sectors have been used, or it will exceed
              |         Implementation Limitation of Defect Handling of the file.
1 0 = 3d00   ||       Invalid bits in identify message.
1 0 = 3f01   ||       Microcode has been changed.
1 0 = 4080   ||       Diagnostic failure on RAM.
1 0 = 4200   ||       Power On or Diagnostic Error.
1 0 = 4300   ||       Message reject error.
              |         An inappropriate or unexpected message reject is received from the
              |         initiator or the initiator rejects a message twice.
1 0 = 4400   ||       Internal controller error. The control microprocessor detects incorrect
              |         status or receives an illegal request from the device electronics.
1 0 = 4500   ||       Select/Re-select failed.
              |         The intiator failed to respond to a re-selection within 250 ms after
              |         the file gained bus arbitration. The re-selection is attempted a 2nd
              |         time before setting select/re-select failed status.
1 0 = 4700   ||       SCSI parity error.
1 0 = 4800   ||       Initiator detected error message received.
              |         The initiator detected an error, sent a message to retry, detected
              |         the error again, and sent the retry message a 2nd time. The file set
              |         Check Condition Status with Initiator Detected Status.
1 0 = 4900   ||       Inappropriate/illegal message.
              |         The initiator sent a message that either is not supported or is not
              |         in a logical sequence.
1 0 = 4a00   ||       Command phase error.
1 0 = 4b00   ||       Data phase error.
1 0 = 4c00   ||       LUN failed self-configuration.
1 0 = 4e00   ||       Overlapped commands attempted.
1 0 = 8001   ||       Timeout hang during read/write.
1 0 = 800f   ||       Bank RAM code error.
1 0 = 8100   ||       Overlay read fail.
1 0 = 8101   ||       Invalid overlay version.
1 0 = 8102   ||       Improper overlay sector.
1 0 = 810f   ||       Invalid overlay requested.

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Field Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif
  ~endif
~endif

              % 7) Error segment information
4 20  @  32   |     Segment count in which err occurred      = 0x%.8x
4 24  @  32   |     Byte count within seg where err occurred = 0x%.8x


!270mb
* Sense information for the 270 Mb hardfile
%                     270 MB Hardfile Sense Data Analysis
%
#include quantum


!270mb2
* Sense information for the 270 Mb hardfile
%                     270 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include satsuma

!quantum
* Quantum family of drives
* Do particulars for hardfiles
#define servo_err
#define illegal_request
#define hardware_error
#define aborted_cmd
#define medium_error
#define recovered_error
#define not_ready
#set tms_status 0
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   $lba_valid

0 20  = 70    |    Error for the current command
0 20  = f0    |    Error for the current command
0 20  = 71    |    Deferred error
0 20  = f1    |    Deferred error

0 22  @       |    a) Sense Key (0x%.4x)  @sense_key
0 22  = 00    |        No Sense
              |       --There is no sense key info to be reported for the unit
0 22  = 01   ||       Recovered error
              |       --The last cmnd completed successfully with recovery
              |         action performed by the target
0 22  = 02   ||       Not Ready
              |       --The logical unit can not be accessed
0 22  = 03   ||       Medium Error     $medium_error
              |       --The command terminated with a non-recoverable
              |         error condition caused by a flaw in the media or
              |         an error in the recorded data
0 22  = 04   ||       Hardware Error    $hardware_error
              |       --The target detected a non-recoverable hardware error
              |         while performing a command or during a diagnostic test
0 22  = 05   ||       Illegal Request  $illegal_request
              |       --There was an illegal parameter in the Command
              |         Descriptor Block or additional parameter supplied as data
0 22  = 06   ||       Unit Attention
              |       --Reported after an attention causing event
              |         Generally drive was reset
0 22  = 07   ||       Data Protect -- Write operation to write protect device
0 22  = 0a   ||       Compare or Copy cmd aborted
0 22  = 0b   ||       Aborted Command  $aborted_cmd
              |       --Target aborted the command
0 22  = 0d   ||       Volume Overflow
0 22  = 0e   ||       Miscompare on data


0 23 @ 32 |    b) Logic Block Address = 0x%.8x  or %d decimal @lba ~lba lba
~if lba_valid
             %       LBA is valid
~else
             %       NOTE LBA is not precisely valid for the error
~endif


0 27 @       |    c) Additional Sense Length = %d

1  0 @ 16    |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (aborted_cmd)
  1 0 = 2900 ||        SIC error
~else
  1 0 = 0000 |         None
  1 0 = 29   ||        Reset
~endif
1 0 = 0100 ||        No disk index or sector signal found
1 0 = 02   ||        No seek complete
1 0 = 0300 ||        Write fault
1 0 = 0301 ||        Wedge detected and write gate active
1 0 = 0400  |        Not ready
1 0 = 06   ||        Prediction update activity error or recalibrate failure
1 0 = 0900 ||        Bump timeout
1 0 = 0904 ||        Servo err -- sync --
1 0 = 0905 ||        Servo err -- address mark
1 0 = 0906 ||        Bad track number data err
1 0 = 0907 ||        Servo err -- defect
1 0 = 0908 ||        Bump err
1 0 = 1000 ||        ID Field CRC Error
~if (medium_error)
  1 0 = 11   ||        Unrecovered read error
~else
1 0 = 1100 ||        Data Field ECC error, recovered on last retry
~endif
1 0 = 1101 ||        CRC/Continue marker
1 0 = 1200 ||        ID Field Address Mark not found
1 0 = 1201 ||        ID Field Address Mark not found, internal continue
1 0 = 1300 ||        Data Address Mark not found
1 0 = 1400 ||        Record not found
1 0 = 1401 ||        Record not found or bad block mark set
1 0 = 1500 ||        Random positioning error (Seek Error)
1 0 = 1501 ||        Incorrect track error
1 0 = 1503 ||        Servo Addr Mark errs during Settle/Ontrack
1 0 = 1504 ||        Seek timeout
1 0 = 1505 ||        Servo Addr Mark errs during seek ISR
1 0 = 16   ||        Data synchrnonization mark error
1 0 = 1701 ||        Data recovered with ECC
1 0 = 1708 ||        Err concerning CRC/continue marker
1 0 = 1709 ||        Data recovered with ECC
1 0 = 1800 ||        Data recovered with ECC. Two equal syndromes
1 0 = 1801 ||        Data recovered with ECC and retries
1 0 = 1900 ||        Defect list error
1 0 = 1a   ||        Length error in parameter list
1 0 = 1b   ||        Error during synchronous transfer
1 0 = 1c   ||        Defect list not found
1 0 = 1d00 ||        Miscompare on a verify
1 0 = 2000 |         Command not valid
1 0 = 2400 |         Invalid bits in Command Descriptor Block
1 0 = 2500 |         Has invalid logic unit
1 0 = 2600 |         Has invalid field in parameters
1 0 = 2A   ||        Mode Select parameters changed by another initiator
1 0 = 3100 ||        SIC error
1 0 = 3101 ||        Format Unit cmd error
1 0 = 3200 ||        No defect spare location available
1 0 = 3201 ||        All alternate sectors used
1 0 = 4000 ||        Diagnostic failure (RAM error)
1 0 = 42   ||        Controller Code Problem
1 0 = 4200  |        Checksum -- Internal ROM
1 0 = 4201  |        Checksum -- Resident code
1 0 = 4202  |        Resident and overlay code compatibility
1 0 = 4203  |        Resident Code and ROM compatibility
1 0 = 4204  |        ROM and overlay code compatibility
1 0 = 4205  |        Checksum -- Overlay code
1 0 = 4206  |        Diskware vector table
1 0 = 4300  |        Err in Message -- Reject
1 0 = 4500 ||        Never saw initiator issue reselect
1 0 = 4700 ||        SCSI Parity Error
1 0 = 4800 ||        Initiator detected error message received
1 0 = 4A00  |        SIC error in FIFO load
1 0 = 4900  |        SIC error in FIFO unload
1 0 = 4B00  |        FIFO Full Predicated -- SIC Err
1 0 = 4E00  |        Overlapped commands attempted
1 0 = 6600 ||        SIC error
1 0 = 8000 ||        System sector error
1 0 = 8100 ||        System sector read error
~if (hardware_error)
  1 0 = 82   ||        Missing P1 or P2 digital signal
  1 0 = 83   ||        P1 or P2 analog signal is bad
~else
  1 0 = 8200 ||        Diskware read error
~endif
1 0 = 8400 ||        Sequencer format table write error
1 0 = 8500 ||        Message reject
1 0 = 8600 ||        Sequencer error
1 0 = 8601 ||        Sequencer error during recovery
1 0 = 8602 ||        Miscompare during read,write ID
1 0 = 8700 ||        Controller code internal error
1 0 = 8800 ||        Air lock stuck in closed position
1 0 = 8900 ||        Head amplifier failure           '
~if (illegal_request)
  1 0 = 8A00  |        Head number given is not valid
  1 0 = 8B00  |        Cylinder given is not valid
  1 0 = 8C00  |        Invalid initiator keeps selecting drive
  1 0 = 8D00  |        Bytes per block or sector not correct
  1 0 = 8F00  |        Invalid sector
  1 0 = AB    |        Type of defect list is not supported
~else
  1 0 = 8A00 ||        Head ID error
  1 0 = AB00 ||        Defect Data Read error
~endif
1 0 = 9000 ||        Error in synchronous acknowledge
1 0 = 9100 ||        Error in synchronous acknowledge
1 0 = 9200 ||        Error in FIFO Load
1 0 = 9300 ||        Error in FIFO Predicted Full
1 0 = 9400 ||        SIC error
1 0 = 9500 ||        Timeout on disk controller or sequencer
1 0 = 9600 ||        Bump recovery
1 0 = 9700 ||        Overrun/underrun
1 0 = 9800 ||        Settling timeout
1 0 = 9900 ||        Bump retry timeout
1 0 = 9A00 ||        Attempted drive reselect
1 0 = 9B00  |        Sync msg period or offset is bad
1 0 = 9C00  |        Select cmd during disconnect by same initiator
1 0 = 9D00 ||        Motor speed low error
1 0 = 9E00 ||        Motor error during spin up
1 0 = 9E01 ||        Motor speed out of range
1 0 = 9F   ||        Checksum error -- ROM
1 0 = A0   ||        Checksum error -- External PROM
1 0 = A1   ||        Error in sequencer rollover reg
1 0 = A2   ||        Error with External RAM
1 0 = A300 ||        Cant read sector to reassign blocks
1 0 = A4   ||        Thermistor failure
1 0 = A6   ||        A and B servors can not be detected
1 0 = A7   ||        Timed out with off track condition
1 0 = A8   ||        Phantom interrupt
1 0 = AA00 ||        Reallocate related error
1 0 = AC00 ||        Air lock stuck in open position
1 0 = AD   ||        Missing servo interrupt
1 0 = AE   ||        Error adjusting circle size
1 0 = B0   ||        Drive recalibration
1 0 = B1   ||        Drive speed low error
1 0 = C0   ||        SIC error
1 0 = C200 ||        SIC error
1 0 = FF00 ||        Autowrite request, but host channel not enabled
1 0 = FF01 ||        ID not found in format track list
1 0 = FF02 ||        Descriptor list of format track bad

1  2 @ 8     |    f) Field Replaceable Unit Code (0x%.2x)

1 2 = 00     |       None

1 3 ^ 80    ^SKSV
~if (! SKSV)
                %       Specific Information Not Used
~elseif ((recovered_error || medium_error) || (hardware_error))
     1  4 @ 16  |       Retry count (%d decimal)
~elseif not_ready
     1  4 @ 16  |       Progress indicator (%d decimal) out of 65536
~elseif illegal_request
     1  3 ^ 40  ^C_D
     ~if (C_D)
                %       Illegal parameter(s) in command block
     ~else
                %       Illegal parameter(s) in descriptor block
     ~endif

     1  3 ^ 08  ^BPV
     1 3 @          @temp_byte
     bit_pointer := (temp_byte & 0x07)
     1 4 @  16      @field_pointer
     ~if (BPV == 1)
                %       First illegal bit is bit %d of byte %d (decimal) ~bit_pointer field_pointer
     ~endif
~endif



!320mb
* Sense information for the 320 Mb hardfile
%                     320 MB Hardfile Sense Data Analysis
%
#set is_320mb 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf

!355mb
* Sense information for the 355 Mb hardfile
%                     355 MB Hardfile Sense Data Analysis
%
#set is_355mb 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf


!360mb
* Sense information for the 360 Mb hardfile
%                     360 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include satsuma


!400mb
* Sense information for the 400 Mb hardfile
%                     400 MB Hardfile Sense Data Analysis
%
#set is_400mb 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf

!540mb
%                     540 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV_FPV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
  0 23 @ 32   |    b) Information Bytes: 0x%.8x
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Command Specific Information: 0x%.8x

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE - There is no sense key information to be reported. This code
              |         occurs for a successfully completed command.

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
              |         The last command was completed successfully, but with some recovery
              |         action performed by the disk.

~elseif (sense_key == 2)
             ||       NOT READY - The disk drive cannot be accessed.
              |         Operator intervention may be required.

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
              |         The command terminated with a nonrecoverable error condition which was
              |         probably caused by a flaw in the media or by an error in the recorded
              |         data.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
              |         A nonrecoverable hardware error (e.g. disk drive failure, parity
              |         error, etc.) was detected while the disk drive was performing the
              |         command, or while the disk drive was performing a self-test operation.

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
              |         There was an illegal parameter in the command or in the additional
              |         required parameters supplied as data for some related commands. If the
              |         error is detected in the CDB, the disk drive does not alter the media.

~elseif (sense_key == 6)
             ||       UNIT ATTENTION - The disk drive has been reset.
              |         This error is reported the 1st time any cmd is issued after the cond-
              |         ition is detected and the requested cmd is not performed. This cond-
              |         ition is cleared when the next cmd that is not an INQUIRY cmd is
              |         issued by the same initiator. UNIT ATTENTION is reported to all SCSI
              |         devices that subsequently issue a cmd to the disk drive.

~elseif (sense_key == 0x7)
             ||       DATA PROTECT
              |         A write operation was attempted on a write protected device.

~elseif (sense_key == 0x9)
             ||       VENDOR UNIQUE - A vendor unique error condition occurred.
              |         This code is currently not returned by the disk drive.

~elseif (sense_key == 0xa)
             ||       COPY/COMPARE ABORTED
              |         A COPY or COMPARE command was aborted because an error condition was
              |         detected on the source and/or destination device. This code is not
              |         returned by the disk drive.

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND - The disk drive aborted the command.
              |         The initiator may recover by trying to execute the cmd again.

~elseif (sense_key == 0xd)
             ||       VOLUME OVERFLOW

~elseif (sense_key == 0xe)
             ||       MISCOMPARE

~else
             ||       NOT IMPLEMENTED - Undefined sense key value.
~endif

1 0 = 00      |       NO ADDITIONAL SENSE INFORMATION - The disk drive has no
              |         additional sense available for the previous command.
1 0 = 01     ||       NO INDEX/SECTOR SIGNAL
1 0 = 02     ||       NO SEEK COMPLETE. The disk drive could not complete a seek operation.
1 0 = 03     ||       PERIPHERAL DEVICE WRITE FAULT.
              |         The disk drive determined that a fault occurred during a WRITE
              |         operation.
1 0 = 04     ||       LOGICAL UNIT NOT READY. The disk drive is not ready.
1 0 = 05     ||       LOGICAL UNIT DOES NOT RESPOND TO SELECTION.
1 0 = 06     ||       NO REFERENCE POSITION FOUND.
              |         The disk drive could not rezero the positioner.
1 0 = 08     ||       LUN COMMUNICATION ERROR.
1 0 = 0c     ||       WRITE ERROR (AUTO REALLOCATION INVOLVED).
1 0 = 10     ||       ID CRC OR ECC ERROR.
              |         The sector ID field could not be read without a CRC error.
1 0 = 11     ||       UNRECOVERED READ ERROR.
              |         A block could not be read after the number of retyr attempts specified
              |         in the MODE SELECT command.
1 0 = 12     ||       ADDRESS MARK NOT FOUND FOR ID FIELD.
              |         The disk drive could not locate the address mark for a sector header.
1 0 = 13     ||       ADDRESS MARK NOT FOUND FOR A DATA FIELD.
              |         The disk drive could not locate the address mark for sector data area.
1 0 = 14     ||       RECORDED ENTITY NOT FOUND - The block sequence is improper.
              |         A block is missing, or the block cannot be read.
1 0 = 15     ||       RANDOM POSITIONING ERROR.
              |         A miscompare occurred between the cylinder address of the data header
              |         and the address specified in the CDB of the command.
1 0 = 16     ||       DATA SYNC MARK ERROR.
1 0 = 17     ||       RECOVERED DATA WITH NO ERROR CORRECTION APPLIED.
              |         The disk drive encountered an err which was recovered using
              |         retries, not including ECC, while reading the media.
1 0 = 18     ||       RECOVERED DATA WITH ERROR CORRECTION APPLIED.
              |         The disk drive encountered an error which was recovered using ECC
              |         correction while reading the media.
1 0 = 19     ||       DEFECT LIST ERROR.
              |         The disk drive encountered an error while accessing one of the defect
              |         lists.
1 0 = 1a     ||       PARAMETER LIST LENGTH ERROR.
              |         The parameter list length specified in the CDB by the initiator is
              |         too large for the disk drive.
1 0 = 1b     ||       SYNCHRONOUS DATA TRANSFER ERROR.
1 0 = 1c     ||       DEFECT LIST NOT FOUND.
              |         The disk drive could not locate the primary defect list (P list).
1 0 = 1d     ||       MISCOMPARE DURING VERIFY OPERATION.
              |         One or more bytes did not compare when the VERIFY or the WRITE AND
              |         VERIFY command was issued.
1 0 = 20     ||       INVALID COMMAND OPERATION CODE.
              |         The initiator issued a command that cannot be executed or is not
              |         applicable.
1 0 = 21     ||       LOGICAL BLOCK ADDRESS OUT OF RANGE. The addressed block is not valid.
1 0 = 22     ||       ILLEGAL FUNCTION FOR DEVICE TYPE.
              |         The disk drive is unable to perform the requested function.
1 0 = 24     ||       INVALID FIELD IN CDB.
              |         A field in the CDB is reserved and contains a value other than zero,
              |         or the value in the field is incorrect.
1 0 = 25     ||       LOGICAL UNIT NOT SUPPORTED.
              |         The LUN specified in the CDB or the SCSI IDENTIFY message is not zero.
1 0 = 26     ||       INVALID FIELD IN PARAMETER LIST.
              |         A field in the parameter list is reserved and contains a value other
              |         than zero, or the value in the field is incorrect.
1 0 = 27     ||       WRITE PROTECT.
              |         The disk is write protected. The outstanding WRITE command is aborted.
1 0 = 28     ||       NOT READY TO READY TRANSITION.
              |         The disk drive has detected a NOT READY condition followed by a READY
              |         condition.
1 0 = 29     ||       POWER ON, RESET, OR BUS DEVICE RESET OCCURRED.
              |         The disk drive has been reset by a SCSI BUS RESET, BUS DEVICE
              |         RESET message, or POWER ON/RESET condition.
1 0 = 2a     ||       PARAMETERS CHANGED.
              |         The MODE SELECT parameters for this device have been changed by
              |         another initiator and may affect current operations.
1 0 = 2f     ||       COMMANDS CLEARED BY ANOTHER INITIATOR.
1 0 = 31     ||       MEDIUM FORMAT CORRUPTED. The FORMAT UNIT command failed to complete.
1 0 = 32     ||       NO DEFECT SPARE LOCATION AVAILABLE.
              |         There are no remaining alternate tracks on the addressed disk drive.
              |         This error condition may occurr during the processing of a FORMAT UNIT
              |         or REASSIGN BLOCK command.
1 0 = 3f     ||       CHANGED OPERATION DEFINITION.
1 0 = 40     ||       DIAGNOSTIC FAILURE ON COMPONENT.
              |         The disk drive detected a RAM error during a SEND DIAGNOSTIC test
              |         operation.
1 0 = 41     ||       DATA PATH FAILURE.
1 0 = 42     ||       POWER ON FAILURE.
1 0 = 43     ||       MESSAGE ERROR.
              |         The initiator responded with a MESSAGE REJECT message to a message
              |         sent by the disk drive.
1 0 = 44     ||       INTERNAL TARGET FAILURE.
              |         The SCSI firmware detected an internal firmware or hardware error and
              |         was unable to complete the current command.
1 0 = 45     ||       SELECT/RESELECT FAILURE.
              |         The SCSI firmware detected a timeout error while attempting a
              |         reselection.
1 0 = 46     ||       SOFT RESET FAILURE.
1 0 = 47     ||       SCSI PARITY ERROR. A parity error occurred on the SCSI bus.
              |         The disk drive was unable to recover the data.
1 0 = 48     ||       INITIATOR DETECTED ERROR MESSAGE RECEIVED.
              |         The initiator sent an INITIATOR DETECTED ERROR message and the disk
              |         drive was unable to recover from the error.
1 0 = 49     ||       INVALID MESSAGE ERROR.
              |         The initiator sent an inappropriate or illegal SCSI message to the
              |         disk drive.
1 0 = 4c     ||       OVERLAY UPLOAD ERROR.
1 0 = 4e     ||       OVERLAPPED COMMANDS ATTEMPTED.
1 0 = 4f     ||       COMMAND SEQUENCE ERROR.
1 0 = 5c     ||       SPINDLES NOT SYNCHRONIZED.
1 0 = b9     ||       CHECK ATTENTION ERROR.

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV_FPV

* If Sense Key Specific Valid
~if (SKSV_FPV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Field Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif
  ~endif
~endif

              % 7) Error segment information
4 20  @  32   |     Segment count in which err occurred      = 0x%.8x
4 24  @  32   |     Byte count within seg where err occurred = 0x%.8x


!540mb2
* Sense information for the 540 Mb hardfile
%                     540 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include satsuma

!540mb3
* Sense information for the 540 Mb hardfile
%                     540 MB Hardfile Sense Data Analysis
%
#include quantum


!540mb4
* Sense information for the 540 Mb hardfile
%                     540 MB Hardfile Sense Data Analysis
%
* Do particulars for hardfiles
#define servo_err
#define illegal_request
#define hardware_error
#define aborted_cmd
#define recovered_error
#define medium_error
#define not_ready
#set tms_status 0
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   $lba_valid

0 20  = 70    |    Error for the current command
0 20  = f0    |    Error for the current command
0 20  = 71    |    Deferred error
0 20  = f1    |    Deferred error

0 22  @       |    a) Sense Key (0x%.4x)  @sense_key
0 22  = 00    |        No Sense
              |       --There is no sense key info to be reported for the unit
0 22  = 01   ||       Recovered error
              |       --The last cmnd completed successfully with recovery
              |         action performed by the target
0 22  = 02   ||       Not Ready
              |       --The logical unit can not be accessed
0 22  = 03   ||       Medium Error
              |       --The command terminated with a non-recoverable
              |         error condition caused by a flaw in the media or
              |         an error in the recorded data
0 22  = 04   ||       Hardware Error    $hardware_error
              |       --The target detected a non-recoverable hardware error
              |         while performing a command or during a diagnostic test
0 22  = 05   ||       Illegal Request  $illegal_request
              |       --There was an illegal parameter in the Command
              |         Descriptor Block or additional parameter supplied as data
0 22  = 06   ||       Unit Attention
              |       --Reported after an attention causing event
              |         Generally drive was reset
0 22  = 07   ||       Data Protect -- Write operation to write protect device
0 22  = 0a   ||       Compare or Copy cmd aborted
0 22  = 0b   ||       Aborted Command  $aborted_cmd
              |       --Target aborted the command
0 22  = 0d   ||       Volume Overflow
0 22  = 0e   ||       Miscompare on data


0 23 @ 32 |    b) Logic Block Address = 0x%.8x  or %d decimal @lba ~lba lba
~if lba_valid
             %       LBA is valid
~else
             %       NOTE LBA is not precisely valid for the error
~endif


0 27 @       |    c) Additional Sense Length = %d

1  0 @ 16    |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

1 0 = 00   |         No additional sense information
1 0 = 01   ||        No disk index or sector signal found
1 0 = 02   ||        No seek complete
1 0 = 03   ||        Write fault
1 0 = 04   ||        Not ready
1 0 = 05   ||        No Select -- Can not select drive
1 0 = 06   ||        Could not reposition drive to zero
1 0 = 08   ||        Communication error with Logical Unit
1 0 = 0C   ||        Write error (involved auto reallocation)
1 0 = 10   ||        ID Field CRC error
1 0 = 11   ||        Unrecovered read error
1 0 = 12   ||        ID Field Address Mark not found
1 0 = 13   ||        Data Address Mark not found
1 0 = 14   ||        Record not found
1 0 = 15   ||        Random positioning error (Seek Error)
1 0 = 16   ||        Data synchronization mark error
1 0 = 17   ||        Data recovered without ECC
1 0 = 18   ||        Data recovered with ECC
1 0 = 19   ||        Defect list error
1 0 = 1a   ||        Parameter list length error
1 0 = 1b   ||        Synchronous transfer error
1 0 = 1c   ||        Defect list not found
1 0 = 1d   ||        Miscompare on a verify
1 0 = 20   ||        Invalid command
1 0 = 21   ||        Logical Block Address out of range
1 0 = 22   ||        Illegal function for device type
1 0 = 24   ||        Illegal field in Command descriptor block
1 0 = 25   ||        Unsupported Logical Unit Number
1 0 = 26   ||        Invalid field in parameter list
1 0 = 27   ||        Write protected device
1 0 = 28   ||        Not ready to ready transition
1 0 = 29   ||        Power on, Reset, or Bus Device Reset occurred
1 0 = 2A   ||        Mode Select parameters changed by another initiator
1 0 = 2F   ||        Another initiator cleard parameters
1 0 = 31   ||        Medium format corrupted
1 0 = 32   ||        No defect spare location available
1 0 = 3F   ||        Operation definition changed
1 0 = 40   ||        Diagnostic failure (RAM error)
1 0 = 41   ||        Data Path failure
1 0 = 42   ||        Power on failure
1 0 = 43   ||        Err in Message -- Reject
1 0 = 44   ||        Internal Failure with target device
1 0 = 45   ||        Reselection Failure
1 0 = 46   ||        Soft Reset Failure
1 0 = 47   ||        SCSI Parity Error
1 0 = 48   ||        Initiator detected error message received
1 0 = 49   ||        Invalid error message received
1 0 = 4C   ||        Overlay upload error
1 0 = 4E   ||        Overlapped commands attempted
1 0 = 4F   ||        Command sequence error
1 0 = 5C   ||        Spindles not resynchronized
1 0 = B9   ||        Check Attention Error

  1 0 = 4100 ||        Data Path failure
  1 0 = 4a00 ||        Command phase error
  1 0 = 4b00 ||        Data Phase error

1  2 @ 8     |    f) Field Replaceable Unit Code (0x%.2x)

1 2 = 00     |       None

1 3 ^ 80    ^SKSV
~if (! SKSV)
                %       Specific Information Not Used
~elseif ((recovered_error || medium_error) || (hardware_error))
     1  4 @ 16  |       Retry count (%d decimal)
~elseif not_ready
     1  4 @ 16  |       Progress indicator (%d decimal) out of 65536
~elseif illegal_request
     1  3 ^ 40  ^C_D
     ~if (C_D)
                %       Illegal parameter(s) in command block
     ~else
                %       Illegal parameter(s) in descriptor block
     ~endif

     1  3 ^ 08  ^BPV
     1 3 @          @temp_byte
     bit_pointer := (temp_byte & 0x07)
     1 4 @  16      @field_pointer
     ~if (BPV == 1)
                %       First illegal bit is bit %d of byte %d (decimal) ~bit_pointer field_pointer
     ~endif
~endif


!670mb
* Sense information for the 670 Mb hardfile
%                     670 MB Hardfile Sense Data Analysis
%
#set is_670mb 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf


!730mb2
* Sense information for the 730 Mb hardfile
%                     730 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include satsuma


!857mb
* Sense information for the 857 Mb hardfile
%                     857 MB Hardfile Sense Data Analysis
%
#set is_857mb 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf

!1000mb
%                     1000 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1000_1200mb

!1000mbde
%                     1000 MB (DE) Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1000_1200mb

!1200mb
%                     1200 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1000_1200mb


!1000_1200mb
* Sense information for the 1000 and 1200 Mb hardfile

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
 ~if (incorrect_len)
  0 23 !@ 32  |    b) Incorrect Length, off by %d decimal bytes
 ~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
 ~endif
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Logical Block Address not reassigned 0x%.8x
              |       If a Reassigns Blocks command then this value is the 1st LBA
              |       from the defect descriptor block that was not reassigned.
              |       If an Auto-Reallocation fails then this value is the LBA
              |       that was not reassigned.

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE
              |         There is no sense key info to be reported for the unit
              |         This code occurs for a successfully completed command

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
  1 0 = 0100 ||         No Index/Sector Signal
              |           Fake and Extra index.
  1 0 = 0200 ||         No Seek Complete
              |           Servo error: Seek timeout.
              |           Servo error, Recalibrate timeout.
              |           Three consecutive missing Servo IDs detected by Path.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics error.
              |           Sector overrun error.
              |           IP write inhibit error.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the PATH.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by the
              |             Servo Processor.
  1 0 = 0c01 ||         Write Error Recovered With Auto Reallocation
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1401 ||         Record Not Found
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found), Recommend Reassignment.
  1 0 = 1406 ||         Record Not Found - Data Auto-Reallocated
              |           No sector found error (ID no sync. found) Data Auto Reallocated.
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           Seek positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           No Data sync. found.
              |           Data sync. error detected while outside of write band.
  1 0 = 1601 ||         Data Synchronization Mark Error - Data Rewritten
              |           No Data sync. found, Data Rewritten.
  1 0 = 1602 ||         Data Synchronization Mark Error - Recommend Rewrite
              |           No Data sync. found, Recommend Rewrite.
  1 0 = 1603 ||         Data Synchronization Mark Error - Data Auto Reallocated
              |           No Data sync. found, Data Auto Reallocated.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1701 ||         Recovered Data with Retries
              |           ECC check corrected without using ECC correction.
              |           ECC Error Detected while outside of write band corrected without ECC.
  1 0 = 1702 ||         Recovered Data with Positive Head Offset
  1 0 = 1703 ||         Recovered Data with Negative Head Offset
  1 0 = 1705 ||         Recovered Data using previous sector ID
              |           Data recovered using No ID Recovery.
  1 0 = 1706 ||         Recovered Data without ECC - Data Auto-Reallocated
  1 0 = 1707 ||         Recovered Data without ECC - Recommended Reassignment
  1 0 = 1708 ||         Recovered Data without ECC - Recommend Rewrite
  1 0 = 1709 ||         Recovered Data without ECC - Data Rewritten
  1 0 = 1801 ||         Recovered Data with Error Correction and Retries Applied
              |           Data correction applied to Drive data for a Data ECC check.
              |           ECC Error detected while outside of write band corrected with ECC.
  1 0 = 1802 ||         Recovered Data - Data Auto-Reallocated
              |           Recovered data with ECC, Auto Reallocated.
  1 0 = 1805 ||         Recovered Data - Recommend Reassignment
              |           Recovered data with ECC, Recommend Reassignment.
  1 0 = 1806 ||         Recovered Data with ECC - Recommend Rewrite
  1 0 = 1807 ||         Recovered Data with ECC - Data Rewritten.
  1 0 = 1c01 ||         Primary Defect List Not Found
              |           Requested P List does match returned list format (READ DEFECT DATA
              |           only).
  1 0 = 1c02 ||         Grown Defect List Not Found
              |           Requested G List does match returned list format (READ DEFECT DATA
              |           only).
  1 0 = 1f00 ||         Partial Defect List Transfered
              |           Defect list longer then 64k, 64k of data returned (READ DEFECT DATA
              |           only).
  1 0 = 4400 ||         Internal Target Failure
              |           Servo Error: Invalid Servo Status Received by the Interface
              |             Processor.
              |           Invalid SP Command Sequence.
              |           Illegal Head Requested.
              |           A servo command is already active.
              |           IP Servo Sanity Error.
              |           Path Servo Sanity Error.
              |           Servo error: Target Cylinder out of Range.
              |           Servo error: Command not accepted while in Retract.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error: Offset out of range.
              |           Servo error: Loss of interupts.
              |           Interupt: Occured with no Interupt bits set.
              |           Motor Speed Error.
              |           Temporary loss of Motor Synchronization.
              |           SCSI Reset Error.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold Reached on Recovered Error.
              |           Media Problem, Recommend Device Replacement.
              |           Hardware Problem, Recommend Device Replacement.

~elseif (sense_key == 2)
             ||       NOT READY
  1 0 = 0400 ||         Logical Unit Not Ready, Cause Not Reportable
              |           Motor is Stuck, Cannot be started.
              |           Motor timeout error.
              |           Motor Thermal Shutdown.
  1 0 = 0401 ||         Logical Unit is in the process of becoming ready
              |           Unavailable while Start Motor active.
              |           Unavailable while Spinup active.
  1 0 = 0402 ||         Logical Unit Not Ready, Initializing command required
              |           Degraded Mode/Motor not running.
              |           Self init Reset, Option Pin Low W/O Auto Motor Start.
  1 0 = 0404 ||         Logical Unit Not Ready, Format in Progress
              |           Unavailable while Format active.
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Degraded Mode/Reassign Block unsuccessful after pushdown started.
  1 0 = 3101 ||         Format Command Failed
              |           Degraded Mode/Format unsuccessful.
  1 0 = 4080 ||         Diagnostic Failure
              |           Degraded Mode/Bringup not successful.
  1 0 = 4085 ||         Diagnostic Failure
              |           Degraded Mode/RAM Microcode not loaded. Download incomplete.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self init Reset, Option Pin High W/O Auto Motor Start
  1 0 = 4c00 ||         Logical Unti Failed Self-Configuration
              |           Degraded Mode/Configuration not loaded.
              |           Degraded Mode/RAM Microcode not loaded.

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
  1 0 = 0c02 ||         Write Error - Auto Reallocation Failed
              |           Recovered Write error, Auto Reallocate failed.
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1100 ||         Unrecoverable Read Error
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           Data ECC Check.
              |           Data ECC Check detected while outside of write band.
  1 0 = 1104 ||         Unrecoverable Read Error - Auto Reallocate Failed
              |           Recoverable Read Error, Auto reallocate failed because of unreadable
              |             data.
  1 0 = 110b ||         Unreoverable Read Error - Recommend Reassignment
  1 0 = 1400 ||         Recorded Entity Not Found
              |           Track characterization failure. Unable to determine sector LBA due
              |             to adjacent read ID failures, with one sector defective.
              |           Reassign (pushdown not started) or Log Sense.
  1 0 = 1401 ||         Record Not Found
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found).
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           No Data sync. found.
              |           Data sync error detected while outside of the write band.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list (READ DEFECT DATA only).
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (READ DEFECT DATA only).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Unrecovered Read Error of Customer Data during Reassign after
              |           pushdown started.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold on Medium Error
              |           Media Problem, Recommend Device Replacement.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
  1 0 = 0100 ||         No Index/Sector Signal
              |           No sector pulse found.
              |           Fake and Extra index.
              |           Write with No Sector Pulses.
  1 0 = 0200 ||         No Seek Complete
              |           Servo processor did not finish command in time.
              |           Servo error: Seek timeout.
              |           Servo error: Recalibrate timeout.
              |           Three consecutive missing Servo IDs detected by Path.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics Not Ready
              |           Arm Electronics error.
              |           Sector overrun error.
              |           IP write inhibit error.
              |           Microjog Write Inhibit.
              |           Invalid Encoded Bit Stream.
              |           IP Retract Error.
              |           Write Gate not detected during Write.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the PATH.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by Servo
              |             Processor.
  1 0 = 1100 ||         Unrecovered Read Error in Reserved Area
              |           Data ECC Check (Reserved Area).
  1 0 = 1400 ||         Recorded Entity Not Found
              |           No Sector Found caused by hardware fault or software.
  1 0 = 1401 ||         Record Not Found - Reaserve Area
              |           No sector found error (Reserved Area).
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           See positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error in Reserved Area
              |           No Data sync. found (Reserved Area).
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list.
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (used by Format Unit and Reassign Block
              |             commands).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Unrecovered Hardware or Reserved area Data error during reassign
              |             after pushdown started.
  1 0 = 3200 ||         No Defect Spare Location Available
              |           GLIST full. Cannot add more entries.
              |           Entire track of defective errors.
              |           No spare sectors remaining.
  1 0 = 3201 ||         Defect list update failure
  1 0 = 4080 ||         Diagnostic Failure
              |           Microcode Check Sum error detected during ROS Test.
              |           Microcode Check Sum error detected during RAM Test.
              |           Servo data not present in CSR.
              |           Reserved area sector valid check failed.
              |           Configuration Sector valid check failed.
              |           Configuration Sector uploaded but Check Sum error.
              |           Reserved area sector version check failed.
  1 0 = 4085 ||         Diagnostic Failure
              |           Microcode Check Sum error during download of Microcode.
              |           Microcode Check Sum error during upload of Microcode.
  1 0 = 4090 ||         Diagnostic Failure
              |           Servo Data Verify Failure.
              |           BATS#2 Error: Seek test failure.
              |           BATS#2 Error: Head Offset test failure.
  1 0 = 40a0 ||         Diagnostic Failure
              |           BATS#2 Error. Read write test failure.
              |           BATS#2 Error. ECC/CRC test failure.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self init Reset, W Auto Motor Start.
  1 0 = 40c0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor ROS and Interface Processor RAM.
              |           Mismatch between the Interface Processor RAM and DE.
              |           Mismatch between the Interface Processor ROS and RAM.
  1 0 = 40d0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor ROS and DE.
              |           Mismatch between the Interface Processor ROS and DE.
  1 0 = 4400 ||         Internal Target Failure
              |           Mismatch between the Interface Processor ROS and Servo Processor ROS.
              |           Microcode error detected while trying to start SCSI data transfer.
              |           Buffer Controller Chip Channel A Error.
              |           SCSI Controller Chip internal parity error.
              |           Reassign could not find the target LBA.
              |           Servo Error: Invalid Servo Status Received by Interface Processor.
              |           Sanity Error during Read Capacity execution.
              |           Target unexpectedly went Bus Free.
              |           SCSI interrupt invalid (Bus Free).
              |           SP interrupt on but SP Status Valid bit is off.
              |           Format Track parameter error (number of sectors and number of IDs
              |             do not match).
              |           Invalid SP Command Sequence.
              |           Illegal Head Requested.
              |           A servo command is already active.
              |           IP Servo Sanity Error.
              |           Path Servo Sanity Error.
              |           Buffer too small to do a format or reassign.
              |           Servo error: Target Cylinder out of range.
              |           Servo error: Command not accepted while in Retract.
              |           Invalid Update Attempted. Read bit not selected.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error, Offset out of range.
              |           Servo error, Loss of interrupts.
              |           SP lost.
              |           SCSI Reset Disabled.
              |           Interrupt Occured with no interrupt bits set.
              |           Motor Speed Error.
              |           PRML Register Write Invalid.
              |           Temporary loss of Motor Synchronization.
              |           SCSI Reset Error.
              |           Buffer Controller Chip Sequence Error.
              |           Buffer Controller Chip Error.
              |           Invalid UEC.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold on Harware Error
              |           Hardware Problem, Recommend Device Replacement.

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
  1 0 = 1a00 ||         Parameter List Length Error
              |           Command parameter list length error.
  1 0 = 2000 ||         Invalid Command Operation Code
  1 0 = 2100 ||         Logical Block Address out of Range
              |           Invalid LBA.
  1 0 = 2400 ||         Invalid Field in CDB
              |           CDB invalid.
              |           Data length error on Read Long or Write Long.
              |           Invalid Buffer ID in Write Buffer Command.
  1 0 = 2500 ||         Logical Unit Not Supported
              |           Invalid LUN.
  1 0 = 2600 ||         Invalid Field in Parameter List
              |           Command parameter data invalid.
              |           Interface Processor ROS and RAM mismatch during Write Buffer Command.
              |           Invalid field in Parameter Data. See Field Ptr Value.
              |           Invalid LBA in Reassign Command when Reassign degraded.
              |           Servo Processor ROS and interface Processor RAM mismatch during
              |             Write Buffer Command.
              |           Interface Processor RAM and DE mismatch during Write Buffer Command.
  1 0 = 3d00 ||         Invalid Bits in Identify Message
              |           Reserved bits in identify msg are non-zero (Bus Free).

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
  1 0 = 2800 ||         Not Ready to Ready Transition (Medium may have changed) Unit Formatted.
              |           Unit Attention/Not Ready to Ready Transition (Format Completed).
  1 0 = 2900 ||         Power On, Reset, or bus device reset occurred.
              |           Unit Attention/POR.
              |           Unit Attention/Self Initiated Reset.
  1 0 = 2a01 ||         Mode Parameters Changed.
              |           Unit Attention/Mode Select Parameters have changed.
  1 0 = 2f00 ||         Commands Cleared by Another Initiator.
              |           Unit Attention/Command cleared by another initiator.
  1 0 = 3f01 ||         Microcode has been changed.
              |           Unit Attention/Write Buffer.
  1 0 = 5c01 ||         Spindles Synchronized.
              |           Unit Attention/Spindle Synchronized.
  1 0 = 5c02 ||         Spindles Not Synchronized.
              |           Unit Attention/Spindles not Synchronized.

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND
  1 0 = 1b00 ||         Synchronous Data Transfer Error.
              |           Synchronous transfer error. Extra pulses on synchronous transfer.
  1 0 = 2500 ||         Logical Unit Not Supported.
              |           Different LUN addressed (identify message) from first selected.
              |             (Bus Free)
  1 0 = 4300 ||         Message Error
              |           Message Reject message. (Bus Free)
              |           Attention dropped too late. (Bus Free)
              |           Message parity error received when no message sent by Target.
              |             (Bus Free)
  1 0 = 4400 ||         Internal Target Failure
              |           Cannot resume the operation (Data Transfer).
  1 0 = 4500 ||         Select or Reselect Failure
              |           Reselection timeout. (Bus Free)
  1 0 = 4700 ||         SCSI Parity Error
              |           Unrecovered SCSI parity error detected by Target during a command
              |             or data phase.
              |           Unrecovered SCSI parity error detected by the Target during a
              |             MESSAGE OUT phase. (Bus Free)
              |           Unrecovered SCSI parity error detected by the Initiator (Message
              |             Parity Error Message). (Bus Free)
  1 0 = 4800 ||         Initiator Detected Error Message Received
              |           Initiator Detected Error for other than STATUS or linked COMMAND
              |             COMPLETE phase.
              |           Initiator Detected Error message for STATUS or Linked COMMAND
              |             COMPLETE phase. (Bus Free)
  1 0 = 4900 ||         Invalid Message Error
              |           Invalid message or attention dropped before all bytes of an extended
              |             message are transferred. (Bus Free)
  1 0 = 4e00 ||         Overlapped Commands Attempted
              |           Invalid Initiator Connection.

~elseif (sense_key == 0xe)
             ||       MISCOMPARE
  1 0 = 1d00 ||         Miscompare During Verify Operation
              |           Miscompare during byte by byte verify.

~else
             ||       NOT IMPLEMENTED
              |         Undefined sense key value.
~endif

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

              %       Error in the Mode Select Command

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Bit Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif

  ~endif
~endif

1 8 @  16     | 7) Unit Error Code (0x%.4x)

1 8 =  0000   |    No Error
1 8 =  0101  ||    Degraded Mode/Motor not running
1 8 =  0102  ||    Unavailable while Start Motor active
1 8 =  0103  ||    Unavailable while Spinup active
1 8 =  0104  ||    Unavailable while Format active
1 8 =  0105  ||    Synchronous transferr error, Extra pulses on sync xfer
1 8 =  0106  ||    Requested P List does match returned list format (READ DEFECT DATA only)
1 8 =  0107  ||    Requested G List does match returned list format (READ DEFECT DATA only)
1 8 =  010a  ||    Defect list longer than 64k, 64k of data returned (READ DEFECT DATA only)
1 8 =  0111  ||    Degraded Mode/Reassign Block unsucessful after pushdown start
1 8 =  0112  ||    Degraded Mode/Format unsuccessful
1 8 =  0113  ||    Degraded Mode/Configuration not loaded
1 8 =  0114  ||    Degraded Mode/RAM Microcode not loaded
1 8 =  0115  ||    Degraded Mode/RAM Microcode not loaded. Download incomplete.
1 8 =  0120  ||    Microcode Check Sum err detected during download of Microcode
1 8 =  0121  ||    Mismatch between the interface Processor ROS and Servo Processor ROS.
1 8 =  0122  ||    Degraded Mode/Bringup not successful
1 8 =  0123  ||    Microcode err detected while trying to start SCSI data transfer
1 8 =  0124  ||    Mismatch between the Servo Processor ROS and DE.
1 8 =  0125  ||    Mismatch between the Servo Processor ROS and Interface Processor RAM.
1 8 =  0126  ||    Mismatch between the interface Processor RAM and DE.
1 8 =  0127  ||    Buffer Controller Chip Channel A Error - Parity error during transfer in.
1 8 =  0128  ||    Buffer Controller Chip Channel A Error - Parity error during transfer out.
1 8 =  0129  ||    Buffer Controller Chip Channel A Error - Programmed I/O Parity error.
1 8 =  012a  ||    Buffer Controller Chip Channel A Error - Unexpected error.
1 8 =  012c  ||    SCSI Controller Chip internal parity error
1 8 =  012d  ||    Cannot resume the operation (data transfer)
1 8 =  012e  ||    Mismatch between the Interface Processor ROS and RAM
1 8 =  012f  ||    Mismatch between the Interface Processor ROS and DE
1 8 =  0130  ||    Invalid op code
1 8 =  0131  ||    Invalid Logical Block Address (LBA)
1 8 =  0132  ||    Command Descriptor Block (CDB) Invalid.
1 8 =  0133  ||    Invalid Logical Unit Number (LUN)
1 8 =  0134  ||    Command parameter data invalid
1 8 =  0135  ||    Command parameter list length error
1 8 =  0136  ||    Interface Processor ROS and RAM mismatch during Write Buffer Command
1 8 =  0137  ||    Data length error on Read Long or Write Long
1 8 =  0138  ||    Invalid field in Parameter Data, See Field Pointer Value
1 8 =  0139  ||    Invalid LBA in Reassign Command when Reassign degraded.
1 8 =  013a  ||    Invalid Buffer ID in Write Buffer Command
1 8 =  013b  ||    Servo Processor ROS and Interface Processor RAM mismatch.
              |      During Write Buffer Command.
1 8 =  013c  ||    Interface Processor RAM and DE mismatch during Write Buffer Command.
1 8 =  0140  ||    Unit Attention/Not Ready to Ready Transition, Format Complete.
1 8 =  0141  ||    Unit Attention/POR.
1 8 =  0142  ||    Unit Attention/Mode Select Parameters have changed.
1 8 =  0143  ||    Unit Attention/Write Buffer.
1 8 =  0144  ||    Unit Attention/Command cleared by another initiator.
1 8 =  0145  ||    Unit Attention/Self Initiated Reset.
1 8 =  0147  ||    Unit Attention Spindles not Synchronized
1 8 =  0148  ||    Unit Attention/Spindles Synchronized
1 8 =  0150  ||    Microcode Check Sum error detected during ROS Test.
1 8 =  0151  ||    Microcode Check Sum error detected during RAM Test.
1 8 =  0152  ||    Microcode Check Sum error during upload of Microcode.
1 8 =  0153  ||    Motor Syncronization lost, motor speed maintained.
1 8 =  0156  ||    GLIST full, Cannot add more entries.
1 8 =  0157  ||    Entire track of defective sectors.
1 8 =  0158  ||    No sector pulse found
1 8 =  0159  ||    Defect list update failure
1 8 =  015a  ||    Motor is Stuck. Cannot be started.
1 8 =  015c  ||    Reassign could not find the target LBA.
1 8 =  015d  ||    No Sector Found caused by hardware fault or software.
1 8 =  015e  ||    No spare sectors remaining.
1 8 =  015f  ||    Error in Primary Defect list.
1 8 =  0160  ||    Initiator detected error other than Status or linked COMMAND COMPLETE phase.
1 8 =  0161  ||    Unrecovered SCSI parity error detected by target during a cmd or data phase.
1 8 =  0162  ||    Invalid initiator Connection.
1 8 =  0163  ||    Recommend Replacement due to Reassign Rate Analysis.
1 8 =  0164  ||    Recommend Replacement due to Hardware Error Rate Analysis
1 8 =  0165  ||    Error in Primary Defect list (READ DEFECT DATA only).
1 8 =  0166  ||    Error in Grown Defect list (READ DEFECT DATA only).
1 8 =  016a  ||    Servo Error: Invalid Servo Status received by the Interface Processor.
1 8 =  016b  ||    Arm Electronics Not Ready.
1 8 =  016c  ||    Sanity Error during Read Capacity execution.
1 8 =  016d  ||    Target unexpectedly went Bus Free.
1 8 =  016e  ||    Servo Data not present in CSR.
1 8 =  016f  ||    Servo Data Verify Failure.
1 8 =  0171  ||    Different LUN addressed (Identity msg) from first selected.
1 8 =  0172  ||    Message Reject message (Bus Free)
1 8 =  0173  ||    Reselection timeout (Bus Free)
1 8 =  0174  ||    Unrecovered SCSI parity error detected by Target during a MESSAGE OUT phase
              |      (Bus Free)
1 8 =  0175  ||    Initiator Detected Error message for STATUS or linked COMMAND COMPLETE phase
              |      (Bus Free)
1 8 =  0176  ||    Invalid message or attention dropped
              |      Before all bytes of an extended message are transferred (Bus Free)
1 8 =  0177  ||    Attention dropped too late (Bus Free)
1 8 =  0178  ||    Msg parity err received when no msg sent by target
1 8 =  0179  ||    Rsvd bits in Identity message are non zero. (Bus Free)
1 8 =  017a  ||    Unrecovered SCSI parity error detected by the initiator
              |      (Message Parity Error Message). (Bus Free)
1 8 =  017b  ||    SCSI interrupt invalid (Bus Free).
1 8 =  017e  ||    Media Problem, Recommend Device Replacement.
1 8 =  017f  ||    Hardware Problem, Recommend Device Replacement.
1 8 =  0180  ||    SP interrupt on but SP Status Valid bit is off.
1 8 =  0181  ||    Error in Grown Defect list (used by Format Unit & Reassign Block commands).
1 8 =  0182  ||    Format Track parm err (# of sectors and ids dont match)
1 8 =  0183  ||    Seek positioning error (ID miscompare)
1 8 =  0184  ||    Invalid SP Command Sequence
1 8 =  0185  ||    Illegal head requested
1 8 =  0186  ||    A servo command is already active
1 8 =  0187  ||    IP Servo Sanity Error
1 8 =  0188  ||    Path Servo Sanity Error
1 8 =  0189  ||    Reserved area sector valid check failed.
1 8 =  018a  ||    Servo processor did not finish command in time
1 8 =  018b  ||    Motor timeout error
1 8 =  018c  ||    Configuration sector valid check failed
1 8 =  018d  ||    Configuration sector uploaded but Check Sum error.
1 8 =  018e  ||    Reserved area sector version check failed.
1 8 =  018f  ||    Buffer too small to do a format or reassign.
1 8 =  0190  ||    Self init reset, invalid input:
              |    -- Option Pin Low W/O Auto Motor Start
              |    -- W Auto Motor Start
              |    -- Option Pin High W/O Auto Motor Start
1 8 =  0191  ||    Track characterization failure. Unable to determine
              |    -- sector LBA due to adjacent read ID failures, with one
              |    -- sector defective. Reassign (pushdown not started) or
              |    -- Log Sense.
1 8 =  0192  ||    Miscompare during byte by byte verify.
1 8 =  0193  ||    BATS#2 Error. Read write test failure.
1 8 =  0194  ||    BATS#2 Error. ECC/CRC test failure.
1 8 =  0195  ||    BATS#2 Error: Seek test failure.
1 8 =  0196  ||    BATS#2 Error: Head Offset Test failure.
1 8 =  0197  ||    Self Init Reset, No task available:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  0198  ||    Self Init Reset, Cause Unknown:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  0199  ||    Self Init Reset, SCSI Ctrl Chip Reset Unsuccessful:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019a  ||    Self Init Reset, Buffer Ctrl Chip Reset Unsuccessful:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019b  ||    Self Init Reset, Zero Divide Error:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019c  ||    Self Init Reset, Control Store Address Fault:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019d  ||    Self Init Reset, Unused Op Code.
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019e  ||    Motor Thermal Shutdown.
1 8 =  019f  ||    Self Init Reset, Invalid Queue Operator:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  01a1  ||    Servo error: Loss of interrupts from the Path.
1 8 =  01a2  ||    Servo error: Settle timeout.
1 8 =  01a3  ||    Servo error: Coarse offtrack.
1 8 =  01a4  ||    Servo error: 3 consecutive missing Servo IDs detected by Servo Processor.
1 8 =  01a5  ||    Servo error: Unexpected Guardband detected.
1 8 =  01a6  ||    Servo error: Settle overshoot.
1 8 =  01a7  ||    Servo error: Seek timeout.
1 8 =  01a8  ||    Servo error: Target Cylinder out of Range.
1 8 =  01a9  ||    Servo error: Command not accepted while in Retract.
1 8 =  01aa  ||    Invalid update attempted, Read bit not selected.
1 8 =  01ab  ||    Servo error: Maximun velocity exceeded.
1 8 =  01ac  ||    Servo error: Head number out of range.
1 8 =  01ad  ||    Servo error: Reference track is unreadable.
1 8 =  01ae  ||    Servo error: Invalid command.
1 8 =  01af  ||    Servo error: Velocity too high at settle hand off.
1 8 =  01b0  ||    Servo error, Offset out of range.
1 8 =  01b1  ||    Servo error, Recalibrate timeout: OD Guardband not found. ID Guardband Seen.
1 8 =  01b2  ||    Servo error, Recalibrate timeout: OD Guardband not found. Data band Seen.
1 8 =  01b3  ||    Servo error, Recalibrate timeout: OD Guardband not found.
              |      ID and Data band Seen.
1 8 =  01b4  ||    Servo error, Recalibrate timeout: OD Guardband not found.
              |      ID and Data band not Seen.
1 8 =  01b5  ||    Servo error, Recalibrate timeout
              |      While looking for 6 consecutive outside diameter guardband sectors.
1 8 =  01b6  ||    Servo error, Recalibrate timeout
              |      While looking for 6 consecutive data bands.
1 8 =  01b7  ||    Servo error, Recalibrate timeout
              |      While looking for the 1st data band track type zero.
1 8 =  01b8  ||    Servo error, Loss of interupts, motor start bit inactive
1 8 =  01b9  ||    Servo error, Loss of interupts, Servo Processor Lost.
1 8 =  01ba  ||    Servo error, Loss of interupts, Interface Processor initiated retract.
1 8 =  01bb  ||    Servo error, Loss of interupts, Servo Processor initiated retract.
1 8 =  01bc  ||    Servo error, Loss of interupts, PATH reset.
1 8 =  01bd  ||    Servo error, Loss of interupts, Bad Coherence.
1 8 =  01be  ||    Servo error, Loss of interupts, Conversion Too Long.
1 8 =  01bf  ||    Servo error, Loss of interupts,  PATH Demod Conversion in progress.
1 8 =  01c0  ||    Three consecutive missing Servo IDs detected by Path.
1 8 =  01c1  ||    Arm electronics error.
1 8 =  01c2  ||    Fake and Extra Index.
1 8 =  01c3  ||    SP lost.
1 8 =  01c4  ||    Sector overrun error.
1 8 =  01c5  ||    IP write inhibit error.
1 8 =  01c6  ||    SCSI Reset disabled.
1 8 =  01c7  ||    Microjog write inhibit.
1 8 =  01c8  ||    Interupt occured with no interupt bits set.
1 8 =  01c9  ||    Write with no sector pulses.
1 8 =  01ca  ||    Invalid encoded bit stream.
1 8 =  01cb  ||    Motor speed error.
1 8 =  01cc  ||    PRML Register Write Invalid.
1 8 =  01cd  ||    IP retract error.
1 8 =  01ce  ||    Temporary loss of Motor Synchronization.
1 8 =  01cf  ||    SCSI Reset Error.
1 8 =  01d0  ||    No sector found error (ID no sync. found).
1 8 =  01d1  ||    No data sync found
1 8 =  01d2  ||    Data ECC check
1 8 =  01d3  ||    Data correction applied to drive data for data ECC check
1 8 =  01d4  ||    ECC check corrected without using ECC correction
1 8 =  01d5  ||    Data Sync err detected while outside of the write band.
1 8 =  01d6  ||    Data ECC check detected while outside of the write band
1 8 =  01d7  ||    ECC error detected while outside of the write band corrected with ECC.
1 8 =  01d8  ||    ECC error detected while outside of the write band corrected without ECC.
1 8 =  01d9  ||    Data recovered using positive offsets.
1 8 =  01da  ||    Data recovered using negative offsets.
1 8 =  01db  ||    Data recovered using No ID recovery.
1 8 =  01f0  ||    Buffer Controller Chip Sequence Error - Check Sum error when loading.
1 8 =  01f1  ||    Buffer Controller Chip Sequence Error - Not stopped when loading.
1 8 =  01f2  ||    Buffer Controller Chip Error - Invalid interupt error.
1 8 =  01f3  ||    Buffer Controller Chip Error - Invalid read SEQSTOP.
1 8 =  01f4  ||    Buffer Controller Chip Error - Parity error on read.
1 8 =  01f5  ||    Buffer Controller Chip Error - Parity error on write.
1 8 =  01f6  ||    Write gate not detected during write.
1 8 =  01f7  ||    Buffer Controller Chip Error - Non parity channel error.
1 8 =  01f8  ||    Buffer Controller Chip Err - Channel parity error on read.
1 8 =  01f9  ||    Buffer Controller Chip Err - Channel parity error on write
1 8 =  01fa  ||    Buffer Controller Chip Err -  Buffer access not ready error.
1 8 =  01fb  ||    Buffer Controller Chip Error
              |      Channel B was busy before the start of a data transfer.
1 8 =  01fc  ||    Buffer Controller Chip Error
              |      Channel error during n transfer from the Data buffer to the Control Store
              |      RAM (CSR).
1 8 =  01fd  ||    Buffer Controller Chip Error
              |      Channel error during an transfer from the Control Store RAM to the Data
              |      buffer.
1 8 =  01fe  ||    Buffer Controller Chip Error
              |      Transfer from the Data buffer to Control Store RAM (CSR) never completed.
1 8 =  01ff  ||    Buffer Controller Chip Error
              |      Transfer from Control Store RAM (CSR) to Data buffer never completed.
1 8 =  02    ||    Software sanity error. Wrong reporting mechanism used.
1 8 =  0f    ||    Invalid UEC

1 8 @ @temp_byte
temp_byte := temp_byte & 0xf0

* If UEC high order nibble is 3 then display message
~if (temp_byte == 0x30)
             %%    Thermal Asperity Detected during error.
~endif

* If correct length and either RECOVERED ERROR or MEDIUM ERROR or HARDWARE ERROR
~if (! incorrect_len)
  ~if ((sense_key == 1) || (sense_key == 3) || (sense_key == 4))

              % 8) Physical Block Address (physical location of the error)
  1 12 @ 16   |      Cylinder = 0x%.4x
  1 14 @      |          Head = 0x%.2x
  1 15 @      |        Sector = 0x%.2x

  ~endif
~endif

              % 9) Error segment information
4 20  @  32   |     Segment count in which err occurred      = 0x%.8x
4 24  @  32   |     Byte count within seg where err occurred = 0x%.8x


!1000mb2
%                     1000 MB Hardfile Sense Data Analysis 
%
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
 ~if (incorrect_len)
  0 23 !@ 32  |    b) Incorrect Length, off by %d decimal bytes
 ~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
 ~endif
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Logical Block Address not reassigned 0x%.8x
              |       If a Reassigns Blocks command then this value is the 1st LBA
              |       from the defect descriptor block that was not reassigned.
              |       If an Auto-Reallocation fails then this value is the LBA
              |       that was not reassigned.

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE
              |         There is no sense key info to be reported for the unit
              |         This code occurs for a successfully completed command

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
  1 0 = 0100 ||         No Index/Sector Signal
              |           Fake and Extra index.
  1 0 = 0200 ||         No Seek Complete
              |           Servo error: Seek timeout.
              |           Servo error, Recalibrate timeout.
              |           Three consecutive missing Servo IDs detected by Path.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics error.
              |           Sector overrun error.
              |           IP write inhibit error.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the PATH.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by the
              |             Servo Processor.
  1 0 = 0c01 ||         Write Error Recovered With Auto Reallocation
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1401 ||         Record Not Found
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found), Recommend Reassignment.
  1 0 = 1406 ||         Record Not Found - Data Auto-Reallocated
              |           No sector found error (ID no sync. found) Data Auto Reallocated.
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           Seek positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           No Data sync. found.
              |           Data sync. error detected while outside of write band.
  1 0 = 1601 ||         Data Synchronization Mark Error - Data Rewritten
              |           No Data sync. found, Data Rewritten.
  1 0 = 1602 ||         Data Synchronization Mark Error - Recommend Rewrite
              |           No Data sync. found, Recommend Rewrite.
  1 0 = 1603 ||         Data Synchronization Mark Error - Data Auto Reallocated
              |           No Data sync. found, Data Auto Reallocated.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1701 ||         Recovered Data with Retries
              |           ECC check corrected without using ECC correction.
              |           ECC Error Detected while outside of write band corrected without ECC.
  1 0 = 1702 ||         Recovered Data with Positive Head Offset
  1 0 = 1703 ||         Recovered Data with Negative Head Offset
  1 0 = 1705 ||         Recovered Data using previous sector ID
              |           Data recovered using No ID Recovery.
  1 0 = 1706 ||         Recovered Data without ECC - Data Auto-Reallocated
  1 0 = 1707 ||         Recovered Data without ECC - Recommended Reassignment
  1 0 = 1708 ||         Recovered Data without ECC - Recommend Rewrite
  1 0 = 1709 ||         Recovered Data without ECC - Data Rewritten
  1 0 = 1801 ||         Recovered Data with Error Correction and Retries Applied
              |           Data correction applied to Drive data for a Data ECC check.
              |           ECC Error detected while outside of write band corrected with ECC.
  1 0 = 1802 ||         Recovered Data - Data Auto-Reallocated
              |           Recovered data with ECC, Auto Reallocated.
  1 0 = 1805 ||         Recovered Data - Recommend Reassignment
              |           Recovered data with ECC, Recommend Reassignment.
  1 0 = 1806 ||         Recovered Data with ECC - Recommend Rewrite
  1 0 = 1807 ||         Recovered Data with ECC - Data Rewritten.
  1 0 = 1c01 ||         Primary Defect List Not Found
              |           Requested P List does match returned list format (READ DEFECT DATA
              |           only).
  1 0 = 1c02 ||         Grown Defect List Not Found
              |           Requested G List does match returned list format (READ DEFECT DATA
              |           only).
  1 0 = 1f00 ||         Partial Defect List Transfered
              |           Defect list longer then 64k, 64k of data returned (READ DEFECT DATA
              |           only).
  1 0 = 4400 ||         Internal Target Failure
              |           Servo Error: Invalid Servo Status Received by the Interface
              |             Processor.
              |           Invalid SP Command Sequence.
              |           Illegal Head Requested.
              |           A servo command is already active.
              |           IP Servo Sanity Error.
              |           Path Servo Sanity Error.
              |           Servo error: Target Cylinder out of Range.
              |           Servo error: Command not accepted while in Retract.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error: Offset out of range.
              |           Servo error: Loss of interupts.
              |           Interupt: Occured with no Interupt bits set.
              |           Motor Speed Error.
              |           Temporary loss of Motor Synchronization.
              |           SCSI Reset Error.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold Reached on Recovered Error.
              |           Media Problem, Recommend Device Replacement.
              |           Hardware Problem, Recommend Device Replacement.

~elseif (sense_key == 2)
             ||       NOT READY
  1 0 = 0400 ||         Logical Unit Not Ready, Cause Not Reportable
              |           Motor is Stuck, Cannot be started.
              |           Motor timeout error.
              |           Motor Thermal Shutdown.
  1 0 = 0401 ||         Logical Unit is in the process of becoming ready
              |           Unavailable while Start Motor active.
              |           Unavailable while Spinup active.
  1 0 = 0402 ||         Logical Unit Not Ready, Initializing command required
              |           Degraded Mode/Motor not running.
              |           Self init Reset, Option Pin Low W/O Auto Motor Start.
  1 0 = 0404 ||         Logical Unit Not Ready, Format in Progress
              |           Unavailable while Format active.
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Degraded Mode/Reassign Block unsuccessful after pushdown started.
  1 0 = 3101 ||         Format Command Failed
              |           Degraded Mode/Format unsuccessful.
  1 0 = 4080 ||         Diagnostic Failure
              |           Degraded Mode/Bringup not successful.
  1 0 = 4085 ||         Diagnostic Failure
              |           Degraded Mode/RAM Microcode not loaded. Download incomplete.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self init Reset, Option Pin High W/O Auto Motor Start
  1 0 = 4c00 ||         Logical Unti Failed Self-Configuration
              |           Degraded Mode/Configuration not loaded.
              |           Degraded Mode/RAM Microcode not loaded.

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
  1 0 = 0c02 ||         Write Error - Auto Reallocation Failed
              |           Recovered Write error, Auto Reallocate failed.
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1100 ||         Unrecoverable Read Error
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           Data ECC Check.
              |           Data ECC Check detected while outside of write band.
  1 0 = 1104 ||         Unrecoverable Read Error - Auto Reallocate Failed
              |           Recoverable Read Error, Auto reallocate failed because of unreadable
              |             data.
  1 0 = 110b ||         Unreoverable Read Error - Recommend Reassignment
  1 0 = 1400 ||         Recorded Entity Not Found
              |           Track characterization failure. Unable to determine sector LBA due
              |             to adjacent read ID failures, with one sector defective.
              |           Reassign (pushdown not started) or Log Sense.
  1 0 = 1401 ||         Record Not Found
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found).
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           Valid only if DRP is limited via Mode Selector during the verify
              |             phase prior to pushdown of a Reassign.
              |           No Data sync. found.
              |           Data sync error detected while outside of the write band.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list (READ DEFECT DATA only).
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (READ DEFECT DATA only).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Unrecovered Read Error of Customer Data during Reassign after
              |           pushdown started.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold on Medium Error
              |           Media Problem, Recommend Device Replacement.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
  1 0 = 0100 ||         No Index/Sector Signal
              |           No sector pulse found.
              |           Fake and Extra index.
              |           Write with No Sector Pulses.
  1 0 = 0200 ||         No Seek Complete
              |           Servo processor did not finish command in time.
              |           Servo error: Seek timeout.
              |           Servo error: Recalibrate timeout.
              |           Three consecutive missing Servo IDs detected by Path.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics Not Ready
              |           Arm Electronics error.
              |           Sector overrun error.
              |           IP write inhibit error.
              |           Microjog Write Inhibit.
              |           Invalid Encoded Bit Stream.
              |           IP Retract Error.
              |           Write Gate not detected during Write.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the PATH.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by Servo
              |             Processor.
  1 0 = 1100 ||         Unrecovered Read Error in Reserved Area
              |           Data ECC Check (Reserved Area).
  1 0 = 1400 ||         Recorded Entity Not Found
              |           No Sector Found caused by hardware fault or software.
  1 0 = 1401 ||         Record Not Found - Reaserve Area
              |           No sector found error (Reserved Area).
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           See positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error in Reserved Area
              |           No Data sync. found (Reserved Area).
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list.
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (used by Format Unit and Reassign Block
              |             commands).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Unrecovered Hardware or Reserved area Data error during reassign
              |             after pushdown started.
  1 0 = 3200 ||         No Defect Spare Location Available
              |           GLIST full. Cannot add more entries.
              |           Entire track of defective errors.
              |           No spare sectors remaining.
  1 0 = 3201 ||         Defect list update failure
  1 0 = 4080 ||         Diagnostic Failure
              |           Microcode Check Sum error detected during ROS Test.
              |           Microcode Check Sum error detected during RAM Test.
              |           Servo data not present in CSR.
              |           Reserved area sector valid check failed.
              |           Configuration Sector valid check failed.
              |           Configuration Sector uploaded but Check Sum error.
              |           Reserved area sector version check failed.
  1 0 = 4085 ||         Diagnostic Failure
              |           Microcode Check Sum error during download of Microcode.
              |           Microcode Check Sum error during upload of Microcode.
  1 0 = 4090 ||         Diagnostic Failure
              |           Servo Data Verify Failure.
              |           BATS#2 Error: Seek test failure.
              |           BATS#2 Error: Head Offset test failure.
  1 0 = 40a0 ||         Diagnostic Failure
              |           BATS#2 Error. Read write test failure.
              |           BATS#2 Error. ECC/CRC test failure.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self init Reset, W Auto Motor Start.
  1 0 = 40c0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor ROS and Interface Processor RAM.
              |           Mismatch between the Interface Processor RAM and DE.
              |           Mismatch between the Interface Processor ROS and RAM.
  1 0 = 40d0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor ROS and DE.
              |           Mismatch between the Interface Processor ROS and DE.
  1 0 = 4400 ||         Internal Target Failure
              |           Mismatch between the Interface Processor ROS and Servo Processor ROS.
              |           Microcode error detected while trying to start SCSI data transfer.
              |           Buffer Controller Chip Channel A Error.
              |           SCSI Controller Chip internal parity error.
              |           Reassign could not find the target LBA.
              |           Servo Error: Invalid Servo Status Received by Interface Processor.
              |           Sanity Error during Read Capacity execution.
              |           Target unexpectedly went Bus Free.
              |           SCSI interrupt invalid (Bus Free).
              |           SP interrupt on but SP Status Valid bit is off.
              |           Format Track parameter error (number of sectors and number of IDs
              |             do not match).
              |           Invalid SP Command Sequence.
              |           Illegal Head Requested.
              |           A servo command is already active.
              |           IP Servo Sanity Error.
              |           Path Servo Sanity Error.
              |           Buffer too small to do a format or reassign.
              |           Servo error: Target Cylinder out of range.
              |           Servo error: Command not accepted while in Retract.
              |           Invalid Update Attempted. Read bit not selected.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error, Offset out of range.
              |           Servo error, Loss of interrupts.
              |           SP lost.
              |           SCSI Reset Disabled.
              |           Interrupt Occured with no interrupt bits set.
              |           Motor Speed Error.
              |           PRML Register Write Invalid.
              |           Temporary loss of Motor Synchronization.
              |           SCSI Reset Error.
              |           Buffer Controller Chip Sequence Error.
              |           Buffer Controller Chip Error.
              |           Invalid UEC.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold on Harware Error
              |           Hardware Problem, Recommend Device Replacement.

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
  1 0 = 1a00 ||         Parameter List Length Error
              |           Command parameter list length error.
  1 0 = 2000 ||         Invalid Command Operation Code
  1 0 = 2100 ||         Logical Block Address out of Range
              |           Invalid LBA.
  1 0 = 2400 ||         Invalid Field in CDB
              |           CDB invalid.
              |           Data length error on Read Long or Write Long.
              |           Invalid Buffer ID in Write Buffer Command.
  1 0 = 2500 ||         Logical Unit Not Supported
              |           Invalid LUN.
  1 0 = 2600 ||         Invalid Field in Parameter List
              |           Command parameter data invalid.
              |           Interface Processor ROS and RAM mismatch during Write Buffer Command.
              |           Invalid field in Parameter Data. See Field Ptr Value.
              |           Invalid LBA in Reassign Command when Reassign degraded.
              |           Servo Processor ROS and interface Processor RAM mismatch during
              |             Write Buffer Command.
              |           Interface Processor RAM and DE mismatch during Write Buffer Command.
  1 0 = 3d00 ||         Invalid Bits in Identify Message
              |           Reserved bits in identify msg are non-zero (Bus Free).

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
  1 0 = 2800 ||         Not Ready to Ready Transition (Medium may have changed) Unit Formatted.
              |           Unit Attention/Not Ready to Ready Transition (Format Completed).
  1 0 = 2900 ||         Power On, Reset, or bus device reset occurred.
              |           Unit Attention/POR.
              |           Unit Attention/Self Initiated Reset.
  1 0 = 2a01 ||         Mode Parameters Changed.
              |           Unit Attention/Mode Select Parameters have changed.
  1 0 = 2f00 ||         Commands Cleared by Another Initiator.
              |           Unit Attention/Command cleared by another initiator.
  1 0 = 3f01 ||         Microcode has been changed.
              |           Unit Attention/Write Buffer.
  1 0 = 5c01 ||         Spindles Synchronized.
              |           Unit Attention/Spindle Synchronized.
  1 0 = 5c02 ||         Spindles Not Synchronized.
              |           Unit Attention/Spindles not Synchronized.

~elseif (sense_key == 7)
             ||       WRITE PROTECTED
              |           Command not allowed while in write protect mode.

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND
  1 0 = 1b00 ||         Synchronous Data Transfer Error.
              |           Synchronous transfer error. Extra pulses on synchronous transfer.
  1 0 = 2500 ||         Logical Unit Not Supported.
              |           Different LUN addressed (identify message) from first selected.
              |             (Bus Free)
  1 0 = 4300 ||         Message Error
              |           Message Reject message. (Bus Free)
              |           Attention dropped too late. (Bus Free)
              |           Message parity error received when no message sent by Target.
              |             (Bus Free)
  1 0 = 4400 ||         Internal Target Failure
              |           Cannot resume the operation (Data Transfer).
  1 0 = 4500 ||         Select or Reselect Failure
              |           Reselection timeout. (Bus Free)
  1 0 = 4700 ||         SCSI Parity Error
              |           Unrecovered SCSI parity error detected by Target during a command
              |             or data phase.
              |           Unrecovered SCSI parity error detected by the Target during a
              |             MESSAGE OUT phase. (Bus Free)
              |           Unrecovered SCSI parity error detected by the Initiator (Message
              |             Parity Error Message). (Bus Free)
  1 0 = 4800 ||         Initiator Detected Error Message Received
              |           Initiator Detected Error for other than STATUS or linked COMMAND
              |             COMPLETE phase.
              |           Initiator Detected Error message for STATUS or Linked COMMAND
              |             COMPLETE phase. (Bus Free)
  1 0 = 4900 ||         Invalid Message Error
              |           Invalid message or attention dropped before all bytes of an extended
              |             message are transferred. (Bus Free)
  1 0 = 4e00 ||         Overlapped Commands Attempted
              |           Invalid Initiator Connection.

~elseif (sense_key == 0xe)
             ||       MISCOMPARE
  1 0 = 1d00 ||         Miscompare During Verify Operation
              |           Miscompare during byte by byte verify.

~else
             ||       NOT IMPLEMENTED
              |         Undefined sense key value.
~endif

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

              %       Error in the Mode Select Command

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Bit Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif

  ~endif
~endif

1 8 @  16     | 7) Unit Error Code (0x%.4x)

1 8 =  0000   |    No Error
1 8 =  0101  ||    Degraded Mode/Motor not running
1 8 =  0102  ||    Unavailable while Start Motor active
1 8 =  0103  ||    Unavailable while Spinup active
1 8 =  0104  ||    Unavailable while Format active
1 8 =  0105  ||    Synchronous transferr error, Extra pulses on sync xfer
1 8 =  0106  ||    Requested P List does match returned list format (READ DEFECT DATA only)
1 8 =  0107  ||    Requested G List does match returned list format (READ DEFECT DATA only)
1 8 =  0108  ||    Defect List Error 
              |       This prevented one or more defects from being used in a 
              |       Format Unit command or from being reported in a Defect Data command.
1 8 =  010a  ||    Defect list longer than 64k, 64k of data returned (READ DEFECT DATA only)
1 8 =  0110  ||    Too few valid GEM measurements available to perform predictive failure analysis.
1 8 =  0111  ||    Degraded Mode/Reassign Block unsucessful after pushdown start
1 8 =  0112  ||    Degraded Mode/Format unsuccessful
1 8 =  0113  ||    Degraded Mode/Configuration not loaded
1 8 =  0114  ||    Degraded Mode/RAM Microcode not loaded
1 8 =  0115  ||    Degraded Mode/RAM Microcode not loaded. Download incomplete.
1 8 =  0116  ||    ROS microcode download failed.
1 8 =  011b  ||    Motor Start filed due to Timer 1 being disabled.
1 8 =  011c  ||    Command not allowed while in Write Protect mode.
1 8 =  011d  ||    An unexpected CIOP message was received and disconnection was not allowed.
1 8 =  011e  ||    A CIOP message was received on an initial connection (bus free).
1 8 =  011f  ||    Mismatch between the Servo Processor and the reference track image.
1 8 =  0120  ||    Microcode Check Sum err detected during download of Microcode
1 8 =  0121  ||    Mismatch between the interface Processor ROS and Servo Processor ROS.
1 8 =  0122  ||    Degraded Mode/Bringup not successful
1 8 =  0123  ||    Microcode err detected while trying to start SCSI data transfer
1 8 =  0124  ||    Mismatch between the Servo Processor ROS and DE.
1 8 =  0125  ||    Mismatch between the Servo Processor ROS and Interface Processor RAM.
1 8 =  0126  ||    Mismatch between the interface Processor RAM and DE.
1 8 =  0127  ||    Buffer Controller Chip Channel A Error - Parity error during transfer in.
1 8 =  0128  ||    Buffer Controller Chip Channel A Error - Parity error during transfer out.
1 8 =  0129  ||    Buffer Controller Chip Channel A Error - Programmed I/O Parity error.
1 8 =  012a  ||    Buffer Controller Chip Channel A Error - Unexpected error.
1 8 =  012b  ||    Command aborted due to fatal hardware error.
1 8 =  012c  ||    SCSI Controller Chip internal parity error
1 8 =  012d  ||    Cannot resume the operation (data transfer)
1 8 =  012e  ||    Mismatch between the Interface Processor ROS and RAM
1 8 =  012f  ||    Mismatch between the Interface Processor ROS and DE
1 8 =  0130  ||    Invalid op code
1 8 =  0131  ||    Invalid Logical Block Address (LBA)
1 8 =  0132  ||    Command Descriptor Block (CDB) Invalid.
1 8 =  0133  ||    Invalid Logical Unit Number (LUN)
1 8 =  0134  ||    Command parameter data invalid
1 8 =  0135  ||    Command parameter list length error
1 8 =  0136  ||    Interface Processor ROS and RAM mismatch during Write Buffer Command
1 8 =  0137  ||    Data length error on Read Long or Write Long
1 8 =  0138  ||    Invalid field in Parameter Data, See Field Pointer Value
1 8 =  0139  ||    Invalid LBA in Reassign Command when Reassign degraded.
1 8 =  013a  ||    Invalid Buffer ID in Write Buffer Command
1 8 =  013b  ||    Servo Processor ROS and Interface Processor RAM mismatch.
              |      During Write Buffer Command.
1 8 =  013c  ||    Interface Processor RAM and DE mismatch during Write Buffer Command.
1 8 =  013d  ||    Mismatch between Interface Processor ROS and Reserved Data Area.
1 8 =  013e  ||    Attempt to write critical data without Thermal Updates allowed.
1 8 =  013f  ||    SCSI Controller Chip detected an LRC error during read.
1 8 =  0140  ||    Unit Attention/Not Ready to Ready Transition, Format Complete.
1 8 =  0141  ||    Unit Attention/POR.
1 8 =  0142  ||    Unit Attention/Mode Select Parameters have changed.
1 8 =  0143  ||    Unit Attention/Write Buffer.
1 8 =  0144  ||    Unit Attention/Command cleared by another initiator.
1 8 =  0145  ||    Unit Attention/Self Initiated Reset.
1 8 =  0147  ||    Unit Attention Spindles not Synchronized
1 8 =  0148  ||    Unit Attention/Spindles Synchronized
1 8 =  0148  ||    Unit Attention/Log Parameters Changed.
1 8 =  0150  ||    Microcode Check Sum error detected during ROS Test.
1 8 =  0151  ||    Microcode Check Sum error detected during RAM Test.
1 8 =  0152  ||    Microcode Check Sum error during upload of Microcode.
1 8 =  0153  ||    Motor Syncronization lost, motor speed maintained.
1 8 =  0156  ||    GLIST full, Cannot add more entries.
1 8 =  0157  ||    Entire track of defective sectors.
1 8 =  0158  ||    No sector pulse found
1 8 =  0159  ||    Defect list update failure
1 8 =  015a  ||    Motor is Stuck. Cannot be started.
1 8 =  015c  ||    Reassign could not find the target LBA.
1 8 =  015d  ||    No Sector Found caused by hardware fault or software.
1 8 =  015e  ||    No spare sectors remaining.
1 8 =  015f  ||    Error in Primary Defect list.
1 8 =  0160  ||    Initiator detected error other than Status or linked COMMAND COMPLETE phase.
1 8 =  0161  ||    Unrecovered SCSI parity error detected by target during a cmd or data phase.
1 8 =  0162  ||    Invalid initiator Connection.
1 8 =  0163  ||    Recommend Replacement due to Reassign Rate Analysis.
1 8 =  0164  ||    Recommend Replacement due to Hardware Error Rate Analysis
1 8 =  0165  ||    Error in Primary Defect list (READ DEFECT DATA only).
1 8 =  0166  ||    Error in Grown Defect list (READ DEFECT DATA only).
1 8 =  016a  ||    Servo Error: Invalid Servo Status received by the Interface Processor.
1 8 =  016b  ||    Arm Electronics Not Ready.
1 8 =  016c  ||    Sanity Error during Read Capacity execution.
1 8 =  016d  ||    Target unexpectedly went Bus Free.
1 8 =  016e  ||    Servo Data not present in CSR.
1 8 =  016f  ||    Servo Data Verify Failure.
1 8 =  0171  ||    Different LUN addressed (Identity msg) from first selected.
1 8 =  0172  ||    Message Reject message (Bus Free)
1 8 =  0173  ||    Reselection timeout (Bus Free)
1 8 =  0174  ||    Unrecovered SCSI parity error detected by Target during a MESSAGE OUT phase
              |      (Bus Free)
1 8 =  0175  ||    Initiator Detected Error message for STATUS or linked COMMAND COMPLETE phase
              |      (Bus Free)
1 8 =  0176  ||    Invalid message or attention dropped
              |      Before all bytes of an extended message are transferred (Bus Free)
1 8 =  0177  ||    Attention dropped too late (Bus Free)
1 8 =  0178  ||    Msg parity err received when no msg sent by target
1 8 =  0179  ||    Rsvd bits in Identity message are non zero. (Bus Free)
1 8 =  017a  ||    Unrecovered SCSI parity error detected by the initiator
              |      (Message Parity Error Message). (Bus Free)
1 8 =  017b  ||    SCSI interrupt invalid (Bus Free).
1 8 =  017e  ||    Media Problem, Recommend Device Replacement.
1 8 =  017f  ||    Hardware Problem, Recommend Device Replacement.
1 8 =  0180  ||    SP interrupt on but SP Status Valid bit is off.
1 8 =  0181  ||    Error in Grown Defect list (used by Format Unit & Reassign Block commands).
1 8 =  0182  ||    Format Track parm err (# of sectors and ids dont match)
1 8 =  0183  ||    Seek positioning error (ID miscompare)
1 8 =  0184  ||    Invalid SP Command Sequence
1 8 =  0185  ||    Illegal head requested
1 8 =  0186  ||    A servo command is already active
1 8 =  0187  ||    IP Servo Sanity Error
1 8 =  0188  ||    Path Servo Sanity Error
1 8 =  0189  ||    Reserved area sector valid check failed.
1 8 =  018a  ||    Servo processor did not finish command in time
1 8 =  018b  ||    Motor timeout error
1 8 =  018c  ||    Configuration sector valid check failed
1 8 =  018d  ||    Configuration sector uploaded but Check Sum error.
1 8 =  018e  ||    Reserved area sector version check failed.
1 8 =  018f  ||    Buffer too small to do a format or reassign.
1 8 =  0190  ||    Self init reset, invalid input:
              |    -- Option Pin Low W/O Auto Motor Start
              |    -- W Auto Motor Start
              |    -- Option Pin High W/O Auto Motor Start
1 8 =  0191  ||    Track characterization failure. Unable to determine
              |    -- sector LBA due to adjacent read ID failures, with one
              |    -- sector defective. Reassign (pushdown not started) or
              |    -- Log Sense.
1 8 =  0192  ||    Miscompare during byte by byte verify.
1 8 =  0193  ||    BATS#2 Error. Read write test failure.
1 8 =  0194  ||    BATS#2 Error. ECC/CRC test failure.
1 8 =  0195  ||    BATS#2 Error: Seek test failure.
1 8 =  0196  ||    BATS#2 Error: Head Offset Test failure.
1 8 =  0197  ||    Self Init Reset, No task available:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  0198  ||    Self Init Reset, Cause Unknown:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  0199  ||    Self Init Reset, SCSI Ctrl Chip Reset Unsuccessful:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019a  ||    Self Init Reset, Buffer Ctrl Chip Reset Unsuccessful:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019b  ||    Self Init Reset, Zero Divide Error:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019c  ||    Self Init Reset, Control Store Address Fault:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019d  ||    Self Init Reset, Unused Op Code.
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  019e  ||    Motor Thermal Shutdown.
1 8 =  019f  ||    Self Init Reset, Invalid Queue Operator:
              |    -- Option Pin Low W/O Auto Motor Start.
              |    -- W Auto Motor Start.
              |    -- Option Pin High W/O Auto Motor Start.
1 8 =  01a0  ||    Servo error: Command not accepted while NOT inretract.
1 8 =  01a1  ||    Servo error: Loss of interrupts from the Path.
1 8 =  01a2  ||    Servo error: Settle timeout.
1 8 =  01a3  ||    Servo error: Coarse offtrack.
1 8 =  01a4  ||    Servo error: 3 consecutive missing Servo IDs detected by Servo Processor.
1 8 =  01a5  ||    Servo error: Unexpected Guardband detected.
1 8 =  01a6  ||    Servo error: Settle overshoot.
1 8 =  01a7  ||    Servo error: Seek timeout.
1 8 =  01a8  ||    Servo error: Target Cylinder out of Range.
1 8 =  01a9  ||    Servo error: Command not accepted while in Retract.
1 8 =  01aa  ||    Invalid update attempted, Read bit not selected.
1 8 =  01ab  ||    Servo error: Maximun velocity exceeded.
1 8 =  01ac  ||    Servo error: Head number out of range.
1 8 =  01ad  ||    Servo error: Reference track is unreadable.
1 8 =  01ae  ||    Servo error: Invalid command.
1 8 =  01af  ||    Servo error: Velocity too high at settle hand off.
1 8 =  01b0  ||    Servo error, Offset out of range.
1 8 =  01b1  ||    Servo error, Recalibrate timeout: OD Guardband not found. ID Guardband Seen.
1 8 =  01b2  ||    Servo error, Recalibrate timeout: OD Guardband not found. Data band Seen.
1 8 =  01b3  ||    Servo error, Recalibrate timeout: OD Guardband not found.
              |      ID and Data band Seen.
1 8 =  01b4  ||    Servo error, Recalibrate timeout: OD Guardband not found.
              |      ID and Data band not Seen.
1 8 =  01b5  ||    Servo error, Recalibrate timeout
              |      While looking for 6 consecutive outside diameter guardband sectors.
1 8 =  01b6  ||    Servo error, Recalibrate timeout
              |      While looking for 6 consecutive data bands.
1 8 =  01b7  ||    Servo error, Recalibrate timeout
              |      While looking for the 1st data band track type zero.
1 8 =  01b8  ||    Servo error, Loss of interupts, motor start bit inactive
1 8 =  01b9  ||    Servo error, Loss of interupts, Servo Processor Lost.
1 8 =  01ba  ||    Servo error, Loss of interupts, Interface Processor initiated retract.
1 8 =  01bb  ||    Servo error, Loss of interupts, Servo Processor initiated retract.
1 8 =  01bc  ||    Servo error, Loss of interupts, PATH reset.
1 8 =  01bd  ||    Servo error, Loss of interupts, Bad Coherence.
1 8 =  01be  ||    Servo error, Loss of interupts, Conversion Too Long.
1 8 =  01bf  ||    Servo error, Loss of interupts,  PATH Demod Conversion in progress.
1 8 =  01c0  ||    Three consecutive missing Servo IDs detected by Path.
1 8 =  01c1  ||    Arm electronics error.
1 8 =  01c2  ||    Fake and Extra Index.
1 8 =  01c3  ||    SP lost.
1 8 =  01c4  ||    Sector overrun error.
1 8 =  01c5  ||    IP write inhibit error.
1 8 =  01c6  ||    SCSI Reset disabled.
1 8 =  01c7  ||    Microjog write inhibit.
1 8 =  01c8  ||    Interupt occured with no interupt bits set.
1 8 =  01c9  ||    Write with no sector pulses.
1 8 =  01ca  ||    Invalid encoded bit stream.
1 8 =  01cb  ||    Motor speed error.
1 8 =  01cc  ||    PRML Register Write Invalid.
1 8 =  01cd  ||    IP retract error.
1 8 =  01ce  ||    Temporary loss of Motor Synchronization.
1 8 =  01cf  ||    SCSI Reset Error.
1 8 =  01d0  ||    No sector found error (ID no sync. found).
1 8 =  01d1  ||    No data sync found
1 8 =  01d2  ||    Data ECC check
1 8 =  01d3  ||    Data correction applied to drive data for data ECC check
1 8 =  01d4  ||    ECC check corrected without using ECC correction
1 8 =  01d5  ||    Data Sync err detected while outside of the write band.
1 8 =  01d6  ||    Data ECC check detected while outside of the write band
1 8 =  01d7  ||    ECC error detected while outside of the write band corrected with ECC.
1 8 =  01d8  ||    ECC error detected while outside of the write band corrected without ECC.
1 8 =  01d9  ||    Data recovered using positive offsets.
1 8 =  01da  ||    Data recovered using negative offsets.
1 8 =  01db  ||    Data recovered using No ID recovery.
1 8 =  01f0  ||    Buffer Controller Chip Sequence Error - Check Sum error when loading.
1 8 =  01f1  ||    Buffer Controller Chip Sequence Error - Not stopped when loading.
1 8 =  01f2  ||    Buffer Controller Chip Error - Invalid interupt error.
1 8 =  01f3  ||    Buffer Controller Chip Error - Invalid read SEQSTOP.
1 8 =  01f4  ||    Buffer Controller Chip Error - Parity error on read.
1 8 =  01f5  ||    Buffer Controller Chip Error - Parity error on write.
1 8 =  01f6  ||    Write gate not detected during write.
1 8 =  01f7  ||    Buffer Controller Chip Error - Non parity channel error.
1 8 =  01f8  ||    Buffer Controller Chip Err - Channel parity error on read.
1 8 =  01f9  ||    Buffer Controller Chip Err - Channel parity error on write
1 8 =  01fa  ||    Buffer Controller Chip Err -  Buffer access not ready error.
1 8 =  01fb  ||    Buffer Controller Chip Error
              |      Channel B was busy before the start of a data transfer.
1 8 =  01fc  ||    Buffer Controller Chip Error
              |      Channel error during n transfer from the Data buffer to the Control Store
              |      RAM (CSR).
1 8 =  01fd  ||    Buffer Controller Chip Error
              |      Channel error during an transfer from the Control Store RAM to the Data
              |      buffer.
1 8 =  01fe  ||    Buffer Controller Chip Error
              |      Transfer from the Data buffer to Control Store RAM (CSR) never completed.
1 8 =  01ff  ||    Buffer Controller Chip Error
              |      Transfer from Control Store RAM (CSR) to Data buffer never completed.
1 8 =  0200  ||    Buffer Controller Chip Error - ECC On The Fly Timeout.
1 8 =  0201  ||    Buffer Controller Chip Error - Pipeline already full.
1 8 =  0202  ||    Buffer Controller Chip Error - FIFO overun/underun.
1 8 =  0203  ||    Disk Manager Chip detected an LRC error during write.
1 8 =  0210  ||    Channel Module Write Parity Error.
1 8 =  0211  ||    Channel Module Read Parity Error.
1 8 =  0212  ||    Tangential Correction(TC) RAM Parity Error.
1 8 =  0213  ||    Data Manager Write Parity Error.
1 8 =  0215  ||    Track Personalization Memory (TPM) Error.
1 8 =  0216  ||    Servo ID overun Error.
1 8 =  0217  ||    Channel Module Write Unlock Error.
1 8 =  0218  ||    ARM Electronics (AE) Idle Error.
1 8 =  0219  ||    Interface Processor Ready Timeout Error.
1 8 =  021a  ||    Address Mark Enable (AMENA) After Sync.
1 8 =  0220  ||    Channel Noise Problem, Recommend Device Replacement.
1 8 =  0221  ||    Channel Assymetry Problem, Recommend Device Replacement.
1 8 =  0222  ||    Channel Precompensation Problem, Recommend Device Replacement.
1 8 =  0223  ||    Channel DC Offset Problem, Recommend Device Replacement.
1 8 =  0224  ||    Channel Timing Offset Problem, Recommend Device Replacement.
1 8 =  0225  ||    Fly Height Change Problem, Recommend Device Replacement.
1 8 =  0226  ||    Torque Amplification Problem, Recommend Device Replacement.
1 8 =  0227  ||    ECC On the Fly Hardware Problem, Recommend Device Replacement.
1 8 =  0230  ||    BATS#2 Error. LRC test failure.
1 8 =  0231  ||    BATS#2 Error. Pallette RAM test failure.
1 8 =  0232  ||    BATS#2 Error. Digital Filter RAM test failure.
1 8 =  0233  ||    BATS#2 Error. Tangential Correction RAM test failure.

1 8 @ @temp_byte
temp_byte := temp_byte & 0xf0

* If UEC high order nibble is 4 then display message
~if (temp_byte == 0x40)
             %%    Thermal Asperity Detected during error.
~endif

* If UEC high order nibble is 8 then display message
~if (temp_byte == 0x80)
             %%    Invalid UEC.
~endif

* If correct length and either RECOVERED ERROR or MEDIUM ERROR or HARDWARE ERROR
~if (! incorrect_len)
  ~if ((sense_key == 1) || (sense_key == 3) || (sense_key == 4))

              % 8) Physical Block Address (physical location of the error)
  1 12 @ 16   |      Cylinder = 0x%.4x
  1 14 @      |          Head = 0x%.2x
  1 15 @      |        Sector = 0x%.2x

  ~endif
~endif


!1100mb
%                     1100 MB Hardfile Sense Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!1100mb16bit
%                     1100 MB (Wide) Hardfile Sense Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!1100mb16bitde
%                     1100 MB (Wide/DE) Hardfile Sense Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2000mb
%                     2000 MB Hardfile Sense Data Analysis
%
#include 1100_4500mb

!2000mbde
%                     2000 MB (DE) Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2000mb16bit
%                     2000 MB 16 bit Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2000mb16bitde
%                     2000 MB 16 bit (DE) Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2200mb
%                     2200 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2200mb16bit
%                     2200 MB 16 bit Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!2200mb16bitde
%                     2200 MB 16 bit (DE) Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!4500mb16bit
%                     4500 MB 16 bit Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb

!4500mb16bitde
%                     4500 MB 16 bit (DE) Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1
#include 1100_4500mb


!1100_4500mb
* Sense information for all 2000 Mb hardfiles

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
 ~if (incorrect_len)
  0 23 !@ 32  |    b) Incorrect Length, off by %d decimal bytes
 ~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
 ~endif
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Logical Block Address not reassigned 0x%.8x
              |       If a Reassigns Blocks command then this value is the 1st LBA
              |       from the defect descriptor block that was not reassigned.
              |       If an Auto-Reallocation fails then this value is the LBA
              |       that was not reassigned.

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE - There is no sense key info to be reported for the unit

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
              |         The last command executed successfully with recovery by the Target.
              |           The Physical Error Record valid for this sense key.
  1 0 = 0100 ||         No Index/Sector Signal
              |           Fake and Extra index.
              |           Write with no sector pulses.
  1 0 = 0200 ||         No Seek Complete
              |           Servo error: Seek timeout.
              |           Servo error, Recalibrate timeout.
              |           Too many missing Servo IDs detected by 
              |           Controller/Channel Hardware.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics Not Ready.
              |           Arm Electronics error.
              |           Sector overrun error.
              |           Interface Processor write inhibit error.
              |           Microjog Write Inhibit.
              |           IP Retract Error.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the Controller/Channel h/w.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by the
              |             Servo Processor.
  1 0 = 0c01 ||         Write Error Recovered With Auto Reallocation
              |             Recovered Write Error, Auto Reallocated
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1401 ||         Record Not Found
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found), Recommend reassignment.
  1 0 = 1406 ||         Record Not Found - Data Auto-Reallocated
              |           No sector found error (ID no sync. found) Data Auto Reallocated.
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           See positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           No Data sync. found.
              |           Data sync. error detected while outside of write band.
  1 0 = 1601 ||         Data Synchronization Mark Error - Data Rewritten
              |           No Data sync. found, Data Rewritten.
  1 0 = 1602 ||         Data Synchronization Mark Error - Recommend Rewrite
              |           No Data sync. found, Recommend Rewrite.
  1 0 = 1603 ||         Data Synchronization Mark Error - Data Auto Reallocated
              |           No Data sync. found, Data Auto Reallocated.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1701 ||         Recovered Data with Retries
              |           ECC check corrected without using ECC correction.
              |           ECC Error Detected while outside of write band corrected without ECC
  1 0 = 1702 ||         Recovered Data with Positive Head Offset
              |           Data Recovered using positive offsets.
  1 0 = 1703 ||         Recovered Data with Negative Head Offset
              |           Data Recovered using negative offsets.
  1 0 = 1705 ||         Recovered Data using previous sector ID
              |           Data recovered using No ID Recovery.
  1 0 = 1706 ||         Recovered Data without ECC - Data Auto-Reallocated
  1 0 = 1707 ||         Recovered Data without ECC - Recommended Reassignment
  1 0 = 1708 ||         Recovered Data without ECC - Recommend Rewrite
  1 0 = 1709 ||         Recovered Data without ECC - Data Rewritten
  1 0 = 1801 ||         Recovered Data with Error Correction and Retries Applied
              |           Data correction applied to Drive data for a Data ECC check.
              |           ECC Error detected while outside of write band corrected with ECC.
  1 0 = 1802 ||         Recovered Data - Data Auto-Reallocated
              |           Recovered data with ECC, Auto Reallocated.
  1 0 = 1805 ||         Recovered Data - Recommend Reassignment
              |           Recovered data with ECC, Recommend Reassignment.
  1 0 = 1806 ||         Recovered Data with ECC - Recommend Rewrite
  1 0 = 1807 ||         Recovered Data with ECC - Data Rewritten.
  1 0 = 1c01 ||         Primary Defect List Not Found
              |           Requested P List does match returned list format (READ DEFECT DATA
              |             only).
  1 0 = 1c02 ||         Grown Defect List Not Found
              |           Requested G List does match returned list format (READ DEFECT DATA
              |             only).
  1 0 = 1f00 ||         Partial Defect List Transfered
              |           Defect list longer then 64k, 64k of data returned (READ DEFECT DATA
              |             only).
  1 0 = 4400 ||         Internal Target Failure
              |           Servo Error: Invalid Servo Status Received by Interface Processor.
              |           Invalid SP Command Sequence.
              |           Illegal Head or Cylinder Requested.
              |           A servo command is already active.
              |           Interface Processor detected Servo Sanity Error.
              |           Controller/Channel Hardware detected Servo Sanity Error.
              |           Servo error: Command not accepted while NOT in Retract.
              |           Servo error: Target Cylinder out of Range.
              |           Servo error: Command not accepted while in Retract.
              |           Servo error: Three consecutive bad Gray Code bursts.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error: Offset out of range.
              |           Servo error: Loss of interupts.
              |           Interupt: Occured with no Interupt bits set.
              |           Motor Speed Error.
              |           Channel module Register Write Error
              |           Temporary loss of Motor Synchronization.
              |           Channel Module Write Parity Error.
              |           Channel Module Read Parity Error.
              |           Data Manager Write Parity Error.
              |           Track Personalization Memory (TPM) Error.
              |           Servo ID overrun Error.
              |           Channel Module Write Unlock Error.
              |           Arm Electronics (AE) Idle Error.
              |           Interface Processor Ready Timeout Error.
              |           Address Mark Enable (AMENA) After Sync.
              |           Tangetial Correction (TC) RAM Parity Error.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold Reached on Recovered Error.
              |           Media Problem, Recommend Device Replacement.
              |           Hardware Problem, Recommend Device Replacement.
              |           Channel Noise Problem, Recommend Device Replacement.
              |           Channel Assymetry Problem, Recommend Device Replacement.
              |           Channel Precomposition Problem, Recommend Device Replacement.
              |           Channel DC Offset Problem, Recommend Device Replacement.
              |           Channel Timing Offset Problem, Recommend Device Replacement.
              |           Fly Height Change Problem, Recommend Device Replacement.
              |           Torque Amplification Problem, Recommend Device Replacement.
              |           ECC On The Fly Hardware Problem, Recommend Device Replacement.

~elseif (sense_key == 2)
             ||       NOT READY - The logical unit cannot be addressed.
  1 0 = 0400 ||         Logical Unit Not Ready, Cause Not Reportable
              |           Motor Start Failed due to Timer 1 being disabled.
              |           Motor is Stuck, Cannot be started.
              |           Motor timeout error.
              |           Motor Thermal Shutdown.
  1 0 = 0401 ||         Logical Unit is in the process of becoming ready
              |           Unavailable while Start/Stop Unit Command active.
              |           Unavailable while Spinup active.
  1 0 = 0402 ||         Logical Unit Not Ready, Initializing command required
              |           Degraded Mode/Motor not running.
  1 0 = 0404 ||         Logical Unit Not Ready, Format in Progress
              |           Unavailable while Format active.
  1 0 = 04b0 ||         Self Init Reset, W/O Auto Motor Start.
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Degraded Mode/Reassign Block unsuccessful after pushdown started.
  1 0 = 3101 ||         Format Command Failed
              |           Degraded Mode/Format unsuccessful.
  1 0 = 4080 ||         Diagnostic Failure
              |           Degraded Mode/Bringup not successful.
  1 0 = 4085 ||         Diagnostic Failure
              |           Degraded Mode/RAM Microcode not loaded. Download incomplete.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self Init Reset, W/O Auto Motor Start.
  1 0 = 4c00 ||         Logical Unti Failed Self-Configuration
              |           Degraded Mode/Configuration not loaded.
              |           Degraded Mode/RAM Microcode not loaded.

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
              |         The command terminated with a non-recoverable error condition caused
              |           by a flaw in the media or an error in recorded data. The Physical
              |           Error Record field is valid for this sense key.
  1 0 = 0c02 ||         Write Error - Auto Reallocation Failed
              |           Recovered Write error, Auto Reallocate failed.
  1 0 = 0c03 ||         Write Error - Recommend Reassignment
  1 0 = 1100 ||         Unrecoverable Read Error
              |           Data ECC Check.
              |           Data ECC Check detected while outside of write band.
  1 0 = 1104 ||         Unrecoverable Read Error - Auto Reallocate Failed
              |           Recoverable Read Error, Auto reallocate failed because of
              |             unreadable data.
  1 0 = 110b ||         Unreoverable Read Error - Recommend Reassignment
  1 0 = 1400 ||         Recorded Entity Not Found
              |           Track characterization failure. Unable to determine sector LBA due
              |             to adjacent read ID failures, with one sector defective.
              |           Reassign (pushdown not started) or Log Sense.
  1 0 = 1401 ||         Record Not Found
              |           No sector found error (ID no sync. found).
  1 0 = 1405 ||         Record Not Found - Recommend Reassignment
              |           No sector found error (ID no sync. found), Recommend Reassignment.
  1 0 = 1600 ||         Data Synchronization Mark Error
              |           No Data sync. found.
              |           Data sync error detected while outside of the write band.
  1 0 = 1604 ||         Data Synchronization Mark Error - Recommend Reassignment
              |           No Data sync. found, Recommend Reassignment.
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list (READ DEFECT DATA only).
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (READ DEFECT DATA only).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Degraded Mode/Reassign Block unsuccessful after pushdown started.
              |           Unrecovered Read Error of Customer Data during Reassign after
              |             pushdown started.
  1 0 = 3101 ||         Format Failed - Degraded Mode/Format unsuccessful.
  1 0 = 4400 ||         Internal Target Failure
              |           Failure to successfully restore an LBA invovled in Track Squeeze
              |             Recovery.

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
              |         The target detected a non-recoverable h/w error while performing a
              |           command or during a diagnostic test. The Physical Error Record field
              |           is valid for this sense key.
  1 0 = 0100 ||         No Index/Sector Signal
              |           No sector pulse found.
              |           Fake and Extra index.
              |           Write with No Sector Pulses.
  1 0 = 0200 ||         No Seek Complete
              |           Servo processor did not finish command in time.
              |           Servo error: Seek timeout.
              |           Servo error: Recalibrate timeout.
              |           Too many missing Servo IDs detected by Controller/Channel
              |             Hardware.
  1 0 = 0300 ||         Peripheral Device Write Fault
              |           Arm Electronics Not Ready
              |           Arm Electronics error.
              |           Sector overrun error.
              |           Interface Processor write inhibit error.
              |           Microjog Write Inhibit.
              |           IP Retract Error.
              |           Write Gate not detected during Write.
  1 0 = 0900 ||         Track Following Error
              |           Servo error: Loss of interrupts from the Controller/Channel H/W.
              |           Servo error: Settle timeout.
              |           Servo error: Coarse offtrack.
              |           Servo error: Three consecutive missing Servo IDs detected by Servo
              |             Processor.
  1 0 = 1100 ||         Unrecovered Read Error in Reserved Area
              |           Data ECC Check (Reserved Area).
  1 0 = 1400 ||         Recorded Entity Not Found
              |           No Sector Found caused by hardware fault or software.
  1 0 = 1401 ||         Record Not Found - Reaserve Area
              |           No sector found error (Reserved Area).
  1 0 = 1500 ||         Random Positioning Error
              |           Servo error: Unexpected Guardband detected.
              |           Servo error: Settle overshoot.
              |           Servo error: Maximum velocity exceeded.
              |           Servo error: Velocity too high at settle hand off.
  1 0 = 1502 ||         Positioning Error Detected by Read of Medium
              |           Seek positioning error (ID miscompare).
  1 0 = 1600 ||         Data Synchronization Mark Error in Reserved Area
              |           No Data sync. found (Reserved Area).
  1 0 = 1902 ||         Defect List Error in Primary List
              |           Error in Primary Defect list.
  1 0 = 1903 ||         Defect List Error in Grown List
              |           Error in Grown Defect list (used by Format Unit and Reassign Block
              |             commands).
  1 0 = 3100 ||         Medium Format Corrupted, Reassign Failed
              |           Unrecovered Hardware or Reserved area Data error during reassign
              |             after pushdown started.
  1 0 = 3200 ||         No Defect Spare Location Available
              |           GLIST full. Cannot add more entries.
              |           Entire track of defective errors.
              |           No spare sectors remaining.
  1 0 = 3201 ||         Defect list update failure
  1 0 = 4080 ||         Diagnostic Failure
              |           Microcode Check Sum error detected during ROS Test.
              |           Microcode Check Sum error detected during RAM Test.
              |           Servo data not present in CSR.
              |           Reserved area sector valid check failed.
              |           Configuration Sector valid check failed.
              |           Configuration Sector uploaded but Check Sum error.
              |           Reserved area sector version check failed.
  1 0 = 4085 ||         Diagnostic Failure
              |           Microcode Check Sum error during download of Microcode.
              |           Microcode Check Sum error during upload of Microcode.
  1 0 = 4090 ||         Diagnostic Failure
              |           Servo Data Verify Failure.
              |           BATS#2 Error; Seek test failure.
              |           BATS#2 Error; Head Offset test failure.
              |           Bats#2 Error; Pallette RAM test failure.
              |           Bats#2 Error; Digital Filter RAM test failure.
              |           Bats#2 Error; Tangential Correction RAM test failure.
  1 0 = 40a0 ||         Diagnostic Failure
              |           BATS#2 Error. Read write test failure.
              |           BATS#2 Error. ECC/CRC test failure.
              |           BATS#2 Error. LRC test failure.
  1 0 = 40b0 ||         Diagnostic Failure
              |           Self init Reset, W Auto Motor Start.
  1 0 = 40c0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor ROS & Interface Processor RAM.
              |           Mismatch between the Interface Processor RAM and DE.
              |           Mismatch between the Interface Processor ROS and RAM.
              |           Servo error; Mismatch between external Servo Processor RAM and
              |             internal Servo Processor ROS.
  1 0 = 40d0 ||         Diagnostic Failure
              |           Mismatch between the Servo Processor and the Reference Track Image.
              |           Mismatch between the Servo Processor ROS and DE.
              |           Mismatch between the Interface Processor ROS and DE.
              |           Mismatch between Interface Processor ROS and Reerved Area Data.
  1 0 = 4400 ||         Internal Target Failure
              |           Defect List Error prevented one or more defects from being used in a
              |             Format Unit cmd or from being reported in a Read Defect Data cmd.
              |           Too few valid GEM measurements available to perform a GEM Predictive
              |             Failure Analysis.
              |           ROS Microcode Download Failed.
              |           Mismatch between the Interface Processor ROS & Servo Processor ROS.
              |           Microcode error detected while trying to start SCSI data transfer or
              |             Missing Bufefr COntroller Chip interrupt detected.
              |           Buffer Controller Chip Channel A Error.
              |           SCSI Controller Chip internal parity error.
              |           Attempt to write critical data without Thermal Updates allowed.
              |           SCSI Controller Chip detected an LRC error during read.
              |           Reassign could not find the target LBA.
              |           Servo Error; Invalid Servo Status Received by the IP.
              |           Sanity Error during Read Capacity execution.
              |           Target unexpectedly went Bus Free.
              |           SCSI interrupt invalid (Bus Free).
              |           SP interrupt on but SP Status Valid bit is off.
              |           Format Track parameter error (number of sectors and number of IDs
              |             do not match).
              |           Invalid SP Command Sequence.
              |           Illegal Head or Cylinder Requested.
              |           A servo command is already active.
              |           Interface Processor detected Servo Sanity Error.
              |           Controller/Channel Hardware detected Servo Sanity Error.
              |           Buffer too small to do a requested function.
              |           Servo error; Command not accepted while NOT in Retract.
              |           Servo error; Target Cylinder out of range.
              |           Servo error; Command not accepted while in Retract.
              |           Servo error; Three consecutive bad Gray Code bursts.
              |           Servo error: Head number out of range.
              |           Servo error: Reference track is unreadable.
              |           Servo error: Invalid Command.
              |           Servo error, Offset out of range.
              |           Servo error, Loss of interrupts.
              |           SP lost.
              |           Interrupt Occured with no interrupt bits set.
              |           Motor Speed Error.
              |           Channel module Register Write Error.
              |           Temporary loss of Motor Synchronization.
              |           Servo error, Servo Nonvolitile Storage RAM error. 
              |           Servo error; Multiple SID Offset Correction error during Tangential
              |             Correction.
              |           Buffer Controller Chip Sequence Error.
              |           Buffer Controller Chip Error.
              |           Disk Manager Chip detected an LRC error during write.
              |           Channel Module Write Parity Error.
              |           Channel Module Read Parity Error.
              |           Tangential Correction (TC) RAM Parity Error.
              |           Data Manager Write Parity Error.
              |           Track Personalization Memory (TPM) Error.
              |           Servo ID overrun Error.
              |           Channel Module Write Unlock Error.
              |           ARM Electronics (AE) Idle Error.
              |           Interface Processor Ready Timeout Error.
              |           Address Mark Enable (AMENA) After Sync.
              |           Invalid UEC.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Motor Synchronization lost, motor speed maintained.
  1 0 = 5d00 ||         Predictive Failure Analysis Threshold on Harware Error

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
              |         There was an illegal parameter in the Command Descriptor Block or
              |           additional parameter supplied as data. If the Target detects an
              |           invalid parameter in the CDB, then the command is terminated w/o
              |           altering the medium. If an invalid parameter is detected in
              |           parameters parameters supplied as data, then the Target may already
              |           have altered the medium.
  1 0 = 1a00 ||         Parameter List Length Error
              |           Command parameter list length error.
  1 0 = 2000 ||         Invalid Command Operation Code
  1 0 = 2100 ||         Logical Block Address out of Range
              |           Invalid LBA.
  1 0 = 2400 ||         Invalid Field in CDB
              |           CDB invalid.
              |           Data length error on Read Long or Write Long.
              |           Invalid Buffer ID in Write Buffer Command.
  1 0 = 2500 ||         Logical Unit Not Supported
              |           Invalid LUN.
  1 0 = 2600 ||         Invalid Field in Parameter List
              |           Command parameter data invalid.
              |           Interface Processor ROS & RAM mismatch during Write Buffer Command.
              |           Invalid field in Parameter Data, See Field Pointer Value.
              |           Invalid LBA in Reassign Command when Reassign degraded.
              |           Servo Processor ROS and interface Processor RAM mismatch during
              |             Write Buffer Command.
              |           Interface Processor RAM and DE mismatch during Write Buffer Command.
  1 0 = 3d00 ||         Invalid Bits in Identify Message
              |           Reserved bits in identify msg are non-zero (Bus Free).

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
  1 0 = 2800 ||         Not Ready to Ready Transition (Medium may have changed)
              |           Unit Attention/Not Ready to Ready Transition (Format Completed).
  1 0 = 2900 ||         Power On, Reset, or bus device reset occurred
              |           Unit Attention/POR.
              |           Unit Attention/Self Initiated Reset.
  1 0 = 2a01 ||         Mode Parameters Changed
              |           Unit Attention/Mode Select Parameters have changed.
  1 0 = 2a02 ||         Log Parameters Changed
              |           Unit Attention/Log Parameters have changed.
  1 0 = 2f00 ||         Commands Cleared by Another Initiator
              |           Unit Attention/Command cleared by another initiator.
  1 0 = 3f01 ||         Microcode has been changed
              |           Unit Attention/Write Buffer.
  1 0 = 5c01 ||         Spindles Synchronized
              |           Unit Attention/Spindle Synchronized.
  1 0 = 5c02 ||         Spindles Not Synchronized
              |           Unit Attention/Spindles not Synchronized

~elseif (sense_key == 0x7)
             ||       DATA PROTECT
              |         Write type command when the target is in Write Protect Mode.
  1 0 = 2700 ||         Command not allowed while in Write Protect Mode.

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND
  1 0 = 1b00 ||         Synchronous Data Transfer Error
              |           Synchronous transfer error. Extra pulses on synchronous transfer.
  1 0 = 2500 ||         Logical Unit Not Supported
              |           Different LUN addressed (Identify message) from first selected.
              |             (Bus Free)
  1 0 = 4300 ||         Message Error
              |           An unexpected CIOP msg was received & disconnection was not allowed.
              |           A CIOP message was received on an intial connection. (Bus Free).
              |           Inappropriate Message Reject message received. (Bus Free)
              |           Attention dropped too late. (Bus Free)
              |           Message parity error received when no message sent by Target.
              |             (Bus Free)
  1 0 = 4400 ||         Internal Target Failure
              |           Command aborted dur to Fatal Hardware error.
              |           Cannot resume the operation (Data Transfer).
  1 0 = 4500 ||         Select or Reselect Failure
              |           Reselection timeout. (Bus Free)
  1 0 = 4700 ||         SCSI Parity Error
              |           Unrecovered SCSI parity error detected by Target during a command
              |             or data phase.
              |           Unrecovered SCSI parity error detected by the Target during a
              |             MESSAGE OUT phase. (Bus Free)
              |           Unrecovered SCSI parity error detected by the Initiator (Message
              |             Parity Error Message). (Bus Free)
  1 0 = 4800 ||         Initiator Detected Error Message Received
              |           Initiator Detected Error for other than STATUS or linked COMMAND
              |             COMPLETE phase.
              |           Initiator Detected Error message for STATUS or Linked COMMAND
              |             COMPLETE phase. (Bus Free)
  1 0 = 4900 ||         Invalid Message Error
              |           Invalid message or attention dropped before all bytes of an
              |             extended message are transferred. (Bus Free)
  1 0 = 4e00 ||         Overlapped Commands Attempted
              |           Invalid Initiator Connection.

~elseif (sense_key == 0xe)
             ||       MISCOMPARE
              |         The source data did not match the data read from the medium.
  1 0 = 1d00 ||         Miscompare During Verify Operation
              |           Miscompare during byte by byte verify.

~else
             ||       NOT IMPLEMENTED
              |         Undefined sense key value.
~endif

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

              %       Error in the Mode Select Command

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Bit Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif

  ~endif
~endif

1 8 @  16     | 7) Unit Error Code (0x%.4x)

1 8 =  0000   |    No Error.
1 8 =  0101  ||    Degraded Mode/Motor not running.
1 8 =  0102  ||    Unavailable while Start/Stop Unit Cmd active.
1 8 =  0103  ||    Unavailable while Spinup active.
1 8 =  0104  ||    Unavailable while Format active.
1 8 =  0105  ||    Synchronous transferr error, Extra pulses on synchronous transfer.
1 8 =  0106  ||    Requested P List does match returned list format (READ DEFECT DATA only).
1 8 =  0107  ||    Requested G List does match returned list format (READ DEFECT DATA only).
1 8 =  0108  ||    Defect List Error
              |      Prevented one or more defects from being used in a Format Unit command or
              |        from being reported ina a Read Defect Data command.
1 8 =  010a  ||    Defect list longer than 64k, 64k of data returned (READ DEFECT DATA only).
1 8 =  010b  ||    BATS#2 Error.  Track Personalization Memory (TPM) Error.
1 8 =  0110  ||    Too few valid GEM measurements avail for a GEM Predictive Failure Analysis.
1 8 =  0111  ||    Degraded Mode/Reassign Block unsucessful after pushdown start.
1 8 =  0112  ||    Degraded Mode/Format unsuccessful.
1 8 =  0113  ||    Degraded Mode/Configuration not loaded.
1 8 =  0114  ||    Degraded Mode/RAM Microcode not loaded.
1 8 =  0115  ||    Degraded Mode/RAM Microcode not loaded, Download incomplete.
1 8 =  0116  ||    ROS Microcode Download Failed.
1 8 =  011b  ||    Motor Start Failed due to Timer 1 being disabled. 
1 8 =  011c  ||    Command not allowed while in Write Protect Mode.
1 8 =  011d  ||    An unexpected CIOP message was received and disconnection was not allowed.
1 8 =  011e  ||    A CIOP message was received on an initial connection (Bus Free).
1 8 =  011f  ||    Mismatch between the Servo Processor and the Reference Track Image.
1 8 =  0120  ||    Microcode Check Sum error detected during download of Microcode.
1 8 =  0121  ||    Mismatch between the interface Processor ROS and Servo Processor ROS.
1 8 =  0122  ||    Degraded Mode/Bringup not successful.
1 8 =  0123  ||    Microcode error detected.
              |      While trying to start SCSI data transfer or Missing Buffer Controller
              |        Chip interrupt detected.
1 8 =  0124  ||    Mismatch between the Servo Processor ROS and DE.
1 8 =  0125  ||    Mismatch between the Servo Processor ROS and Interface Processor RAM
1 8 =  0126  ||    Mismatch between the interface Processor RAM and DE.
1 8 =  0127  ||    Buffer Controller Chip Channel A Error - Parity error during transfer in.
1 8 =  0128  ||    Buffer Controller Chip Channel A Error - Parity error during transfer out.
1 8 =  0129  ||    Buffer Controller Chip Channel A Error - Programmed I/O Parity error.
1 8 =  012a  ||    Buffer Controller Chip Channel A Error - Unexpected error.
1 8 =  012b  ||    Command aborted due to Fatal Hardware Error.
1 8 =  012c  ||    SCSI Controller Chip internal parity error.
1 8 =  012d  ||    Cannot resume the operation (data transfer).
1 8 =  012e  ||    Mismatch between the Interface Processor ROS and RAM.
1 8 =  012f  ||    Mismatch between the Interface Processor ROS and DE.
1 8 =  0130  ||    Invalid op code.
1 8 =  0131  ||    Invalid Logical Block Address (LBA).
1 8 =  0132  ||    Command Descriptor Block (CDB) Invalid.
1 8 =  0133  ||    Invalid Logical Unit Number (LUN).
1 8 =  0134  ||    Command parameter data invalid.
1 8 =  0135  ||    Command parameter list length error.
1 8 =  0136  ||    Interface Processor ROS and RAM mismatch during Write Buffer Command.
1 8 =  0137  ||    Data length error on Read Long or Write Long.
1 8 =  0138  ||    Invalid field in Parameter Data, See Field Pointer Value.
1 8 =  0139  ||    Invalid LBA in Reassign Command when Reassign degraded.
1 8 =  013a  ||    Invalid Buffer ID in Write Buffer Command.
1 8 =  013b  ||    Servo Processor ROS and Interface Processor RAM mismatch
              |      during Write Buffer Command.
1 8 =  013c  ||    Interface Processor RAM and DE mismatch during Write Buffer Command.
1 8 =  013d  ||    Mismatch between Interface Processor ROS and Reserved Area Data.
1 8 =  013e  ||    Attempt to write critical data without Thermal Updates allowed.
1 8 =  013f  ||    SCSI Controller Chip detected an LRC error during read.
1 8 =  0140  ||    Unit Attention/Not Ready to Ready Transition (Format Complete).
1 8 =  0141  ||    Unit Attention/POR.
1 8 =  0142  ||    Unit Attention/Mode Select Parameters have changed.
1 8 =  0143  ||    Unit Attention/Write Buffer.
1 8 =  0144  ||    Unit Attention/Command cleared by another initiator.
1 8 =  0145  ||    Unit Attention/Self Initiated Reset.
1 8 =  0147  ||    Unit Attention Spindles not Synchronized.
1 8 =  0148  ||    Unit Attention/Spindles Synchronized.
1 8 =  0149  ||    Unit Attention/Log Parameters Changed.
1 8 =  0150  ||    Microcode Check Sum error detected during ROS Test.
1 8 =  0151  ||    Microcode Check Sum error detected during RAM Test.
1 8 =  0152  ||    Microcode Check Sum error detected during upload of Microcode.
1 8 =  0153  ||    Motor Syncronization lost, motor speed maintained.
1 8 =  0156  ||    GLIST full, Cannot add more entries.
1 8 =  0157  ||    Entire track of defective sectors.
1 8 =  0158  ||    No sector pulse found.
1 8 =  0159  ||    Defect list update failure.
1 8 =  015a  ||    Motor is Stuck. Cannot be started.
1 8 =  015c  ||    Reassign could not find the target LBA.
1 8 =  015d  ||    No Sector Found caused by hardware fault or software.
1 8 =  015e  ||    No spare sectors remaining.
1 8 =  015f  ||    Error in Primary Defect list.
1 8 =  0160  ||    Initiator detected err other than Status or linked COMMAND COMPLETE phase.
1 8 =  0161  ||    Unrecovered SCSI parity err detected by target during a cmd or data phase.
1 8 =  0162  ||    Invalid initiator Connection.
1 8 =  0163  ||    Media Problem, Recommend Device Replacement.
1 8 =  0164  ||    Hardware Problem, Recommend Device Replacement.
1 8 =  0165  ||    Error in Primary Defect list (READ DEFECT DATA only).
1 8 =  0166  ||    Error in Grown Defect list (READ DEFECT DATA only).
1 8 =  0168  ||    Failure to successfully restore an LBA involved in Track Squeeze Recovery.
1 8 =  016a  ||    Servo Error: Invalid Servo Status received by the Interface Processor.
1 8 =  016b  ||    Arm Electronics Not Ready.
1 8 =  016c  ||    Sanity Error during Read Capacity execution.
1 8 =  016d  ||    Target unexpectedly went Bus Free.
1 8 =  016e  ||    Servo Data not present in CSR.
1 8 =  016f  ||    Servo Data Verify Failure.
1 8 =  0171  ||    Different LUN addressed (Identity message) from first selected (Bus Free).
1 8 =  0172  ||    Inappropriate Message Reject message received (Bus Free).
1 8 =  0173  ||    Reselection timeout (Bus Free).
1 8 =  0174  ||    Unrecovered SCSI parity error detected by Target
              |      During a MESSAGE OUT phase (Bus Free).
1 8 =  0175  ||    Initiator Detected Error msg for STATUS or linked COMMAND COMPLETE phase
              |      (Bus Free).
1 8 =  0176  ||    Invalid message or attention dropped
              |      before all bytes of an extended message are transferred (Bus Free).
1 8 =  0177  ||    Attention dropped too late (Bus Free).
1 8 =  0178  ||    Message parity error received when no message sent by Target (Bus Free).
1 8 =  0179  ||    Reserved bits in Identity message are non zero. (Bus Free).
1 8 =  017a  ||    Unrecovered SCSI parity error
              |      detected by the Initiator (Message Parity Error Message). (Bus Free)
1 8 =  017b  ||    SCSI interrupt invalid (Bus Free).
1 8 =  0180  ||    SP interrupt on but SP Status Valid bit is off.
1 8 =  0181  ||    Error in Grown Defect list (used by Format Unit & Reassign Block commands).
1 8 =  0182  ||    Format Track parameter error (# of sectors and # if IDs do not match).
1 8 =  0183  ||    Seek positioning error (ID miscompare).
1 8 =  0184  ||    Invalid SP Command Sequence.
1 8 =  0185  ||    Illegal Head or Cylinder requested.
1 8 =  0186  ||    A servo command is already active.
1 8 =  0187  ||    Interface Processor detected  Servo Sanity Error.
1 8 =  0188  ||    Controller/Channel Hardware detected Servo Sanity Error.
1 8 =  0189  ||    Reserved area sector valid check failed.
1 8 =  018a  ||    Servo processor did not finish command in time.
1 8 =  018b  ||    Motor timeout error.
1 8 =  018c  ||    Configuration sector valid check failed.
1 8 =  018d  ||    Configuration sector uploaded but Check Sum error.
1 8 =  018e  ||    Reserved area sector version check failed.
1 8 =  018f  ||    Buffer too small to do a requested function.
1 8 =  0190  ||    Self init reset, invalid input
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  0191  ||    Track characterization failure.
              |      Unable to determine sector LBA due to adjacent read ID failures, with one
              |        sector defective. Reassign (pushdown not started) or Log Sense.
1 8 =  0192  ||    Miscompare during byte by byte verify.
1 8 =  0193  ||    BATS#2 Error. Read write test failure.
1 8 =  0194  ||    BATS#2 Error. ECC/CRC test failure.
1 8 =  0195  ||    BATS#2 Error: Seek test failure.
1 8 =  0196  ||    BATS#2 Error: Head Offset Test failure.
1 8 =  0197  ||    Self Init Reset, No task available:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  0198  ||    Self Init Reset, Cause Unknown:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  0199  ||    Self Init Reset, SCSI Ctrl Chip Reset Unsuccessful:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  019a  ||    Self Init Reset, Buffer Ctrl Chip Reset Unsuccessful:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  019b  ||    Self Init Reset, Zero Divide Error:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  019c  ||    Self Init Reset, Control Store Address Fault:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  019d  ||    Self Init Reset, Unused Op Code.
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  019e  ||    Motor Thermal Shutdown.
1 8 =  019f  ||    Self Init Reset, Invalid Queue Operator:
              |      W/O Auto Motor Start.
              |      W Auto Motor Start.
1 8 =  01a0  ||    Servo error: Command not accepted while NOT in retract.
1 8 =  01a1  ||    Servo error: Loss of interrupts from the Controller/Channel Hardware.
1 8 =  01a2  ||    Servo error: Settle timeout.
1 8 =  01a3  ||    Servo error: Coarse offtrack.
1 8 =  01a4  ||    Servo error: 3 consecutive missing Servo IDs detected by Servo Processor.
1 8 =  01a5  ||    Servo error: Unexpected Guardband detected.
1 8 =  01a6  ||    Servo error: Settle overshoot.
1 8 =  01a7  ||    Servo error: Seek timeout.
1 8 =  01a8  ||    Servo error: Target Cylinder out of Range.
1 8 =  01a9  ||    Servo error: Command not accepted while in Retract.
1 8 =  01aa  ||    Servo error: Three consecutive bad Gray Code bursts.
1 8 =  01ab  ||    Servo error: Maximum velocity exceeded.
1 8 =  01ac  ||    Servo error: Head number out of range.
1 8 =  01ad  ||    Servo error: Reference track is unreadable.
1 8 =  01ae  ||    Servo error: Invalid command.
1 8 =  01af  ||    Servo error: Velocity too high at settle hand off.
1 8 =  01b0  ||    Servo error, Offset out of range.
1 8 =  01b1  ||    Servo error, Recalibrate timeout - State 0.
              |      No OD Guardband. ID Guardband Seen.
1 8 =  01b2  ||    Servo error, Recalibrate timeout - State 0.
              |      No OD Guardband. Data band Seen.
1 8 =  01b3  ||    Servo error, Recalibrate timeout - State 0.
              |      No OD Guardband. ID and Data band Seen.
1 8 =  01b4  ||    Servo error, Recalibrate timeout - State 0.
              |      No OD Guardband. ID and Data band not Seen.
1 8 =  01b5  ||    Servo error, Recalibrate timeout.
              |      While looking for 6 consecutive outside diameter guardband sectors.
1 8 =  01b6  ||    Servo error, Recalibrate timeout.
              |      While looking for 6 consecutive data bands.
1 8 =  01b7  ||    Servo error, Recalibrate timeout.
              |      While looking for the 1st data band track type zero.
1 8 =  01b8  ||    Servo error, Loss of interupts - motor start bit not active.
1 8 =  01b9  ||    Servo error, Loss of interupts - Servo Processor Lost.
1 8 =  01ba  ||    Servo error, Loss of interupts - Interface Processor initiated.
1 8 =  01bb  ||    Servo error, Loss of interupts -  Servo Processor initiated retract.
1 8 =  01bc  ||    Servo error, Loss of interupts - Controller/Channel Hardware reset.
1 8 =  01bd  ||    Servo error, Loss of interupts - Bad Coherence.
1 8 =  01be  ||    Servo error, Loss of interupts - Conversion Too Long.
1 8 =  01bf  ||    Servo error, Loss of interupts
              |      Controller/Channel Hardware Demod Conversion in progress.
1 8 =  01c0  ||    Too many missing Servo IDs detected by Controller/Channel Hardware.
1 8 =  01c1  ||    Arm Electronics error.
1 8 =  01c2  ||    Fake and Extra Index.
1 8 =  01c3  ||    SP lost.
1 8 =  01c4  ||    Sector overrun error.
1 8 =  01c5  ||    Interface Processor write inhibit error.
1 8 =  01c7  ||    Microjog Write Inhibit.
1 8 =  01c8  ||    Interrupt occured with no interrupt bits set.
1 8 =  01c9  ||    Write with No Sector pulses.
1 8 =  01cb  ||    Motor Speed Error.
1 8 =  01cc  ||    Channel module Register Write Error.
1 8 =  01cd  ||    IP Retract Error.
1 8 =  01ce  ||    Temporary loss of Motor Synchronization.
1 8 =  01d0  ||    No sector found error (ID no sync. found).
1 8 =  01d1  ||    No Data sync found.
1 8 =  01d2  ||    Data ECC check.
1 8 =  01d3  ||    Data correction applied to drive data for Data ECC check.
1 8 =  01d4  ||    ECC check corrected without using ECC correction.
1 8 =  01d5  ||    Data Sync error detected while outside of the write band.
1 8 =  01d6  ||    Data ECC Check detected while outside of the write band
1 8 =  01d7  ||    ECC Error Detected while outside of the write band corrected with ECC.
1 8 =  01d8  ||    ECC Error Detected while outside of the write band corrected without ECC.
1 8 =  01d9  ||    Data recovered using positive offsets.
1 8 =  01da  ||    Data recovered using negative offsets.
1 8 =  01db  ||    Data recovered using No ID recovery.
1 8 =  01e0  ||    Servo error, Loss of interupts - No SID signal.
1 8 =  01e2  ||    Servo error, Servo Nonvolitile Storage RAM error - Command not allowed
              |      while NVSRAM not loaded.
1 8 =  01e3  ||    Servo error.
              |      Multiple SID Offset Correction error during Tangential Correction.
1 8 =  01e5  ||    Servo error.
              |      Mismatch between external Servo Process RAM & internal Servo Processor ROS
1 8 =  01e6  ||    Servo error, Recalibrate timeout - Coarse Offtrack.
1 8 =  01e8  ||    Servo error, Recalibrate timeout - State A, ID and data band seen.
1 8 =  01e9  ||    Servo error, Recalibrate timeout - State A, ID and data band not seen.
1 8 =  01ea  ||    Servo error, Recalibrate timeout - State B, No OD guardband. ID guardband
              |      seen.
1 8 =  01eb  ||    Servo error, Recalibrate timeout - State B, No OD guardband. ID guardband
              |      seen.
1 8 =  01ec  ||    Servo error, Recalibrate timeout - State B, ID and data band seen.
1 8 =  01ed  ||    Servo error, Recalibrate timeout - State B.
1 8 =  01ee  ||    Servo error, Recalibrate error - State 2, Unexpected Data or ID when
              |      in OD band.
1 8 =  01f0  ||    Buffer Controller Chip Sequence Error - Check Sum error when loading.
1 8 =  01f1  ||    Buffer Controller Chip Sequence Error - Not stopped when loading.
1 8 =  01f2  ||    Buffer Controller Chip Error - Invalid interupt error.
1 8 =  01f3  ||    Buffer Controller Chip Error - Invalid read SEQSTOP.
1 8 =  01f6  ||    Write gate not detected during write.
1 8 =  01f8  ||    Buffer Controller Chip Error - Channel parity error on read.
1 8 =  01f9  ||    Buffer Controller Chip Error - Channel parity error on write
1 8 =  01fb  ||    Buffer Controller Chip Error - Channel B was busy
              |      before the start of a data transfer.
1 8 =  01fc  ||    Buffer Controller Chip Error - Channel error during a transfer
              |      from the Data buffer to the Control Store RAM (CSR).
1 8 =  01fd  ||    Buffer Controller Chip Error - Channel error during a transfer
              |      from the Control Store RAM to the Data buffer.
1 8 =  0200  ||    Buffer Controller Chip Error - ECC On The Fly Timeout.
1 8 =  0201  ||    Buffer Controller Chip Error - Pipeline already full.
1 8 =  0202  ||    Buffer Controller Chip Error - FIFO overun/underun.
1 8 =  0203  ||    Disk Manager Chip detected an LRC error during write.
1 8 =  0210  ||    Channel Module Write Parity Error.
1 8 =  0211  ||    Channel Module Read Parity Error.
1 8 =  0212  ||    Tangential Correction(TC) RAM Parity Error.
1 8 =  0213  ||    Data Manager Write Parity Error.
1 8 =  0215  ||    Track Personalization Memory (TPM) Error.
1 8 =  0216  ||    Servo ID overun Error.
1 8 =  0217  ||    Channel Module Write Unlock Error.
1 8 =  0218  ||    ARM Electronics (AE) Idle Error.
1 8 =  0219  ||    Interface Processor Ready Timeout Error.
1 8 =  021a  ||    Address Mark Enable (AMENA) After Sync.
1 8 =  0220  ||    Channel Noise Problem, Recommend Device Replacement.
1 8 =  0221  ||    Channel Assymetry Problem, Recommend Device Replacement.
1 8 =  0222  ||    Channel Precompensation Problem, Recommend Device Replacement.
1 8 =  0223  ||    Channel DC Offset Problem, Recommend Device Replacement.
1 8 =  0224  ||    Channel Timing Offset Problem, Recommend Device Replacement.
1 8 =  0225  ||    Fly Height Change Problem, Recommend Device Replacement.
1 8 =  0226  ||    Torque Amplification Problem, Recommend Device Replacement.
1 8 =  0227  ||    ECC On the Fly Hardware Problem, Recommend Device Replacement.
1 8 =  0230  ||    BATS#2 Error. LRC test failure.
1 8 =  0231  ||    BATS#2 Error. Pallette RAM test failure.
1 8 =  0232  ||    BATS#2 Error. Digital Filter RAM test failure.
1 8 =  0233  ||    BATS#2 Error. Tangential Correction RAM test failure.

1 8 @ @temp_byte
temp_byte := temp_byte & 0xf0

* If UEC high order nibble is 4 then display message
~if (temp_byte == 0x40)
             %%    Thermal Asperity Detected during error.
~endif

* If UEC high order nibble is 8 then display message
~if (temp_byte == 0x80)
             %%    Invalid UEC.
~endif

* If correct length and either RECOVERED ERROR or MEDIUM ERROR or HARDWARE ERROR
~if (! incorrect_len)
  ~if ((sense_key == 1) || (sense_key == 3) || (sense_key == 4))

              % 8) Physical Block Address (physical location of the error)
  1 12 @ 16   |      Cylinder = 0x%.4x
  1 14 @      |          Head = 0x%.2x
  1 15 @      |        Sector = 0x%.2x

  ~endif
~endif

              % 9) Error segment information
4 20  @  32   |     Segment count in which err occurred      = 0x%.8x
4 24  @  32   |     Byte count within seg where err occurred = 0x%.8x

!1370mb
%                     1370 MB Hardfile Sense Data Analysis
%
#set is_scsi_dasd 1

* Include general SCSI information
#include scsi_gen

* Local variables
#define valid_info
#define incorrect_len
#define sense_key
#define SKSV
#define CD
#define BPV
#define bit_ptr
#define field_ptr
#define temp_byte

              % 6) Request Sense Data Decode

* valid_info is true if the Valid Information Field bit is set.
0 20  ^ 80  ^valid_info

0 20  ^ 70    |    Error for the current command
0 20  ^ 71    |    Deferred error

0 22  @ @temp_byte
sense_key := temp_byte & 0xf
0 22  ^ 20   ^incorrect_len

              %    a) Sense Key (0x%.2x)  ~sense_key
~if (!valid_info)
              %    b) Information field does not contain valid information.
~else
 ~if (incorrect_len)
  0 23 !@ 32  |    b) Incorrect Length, off by %d decimal bytes
 ~else
  0 23 @ 32   |    b) Logical Block Address associated with sense key = 0x%.8x
 ~endif
~endif

0 27  @       |    c) Additional Sense Length = %d decimal

0 28 @ 32     |    d) Command Specific Information: 0x%.8x

1 0 @ 16      |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 0)
              |       NO SENSE
              |         There is no sense key info to be reported for the unit
              |         This code occurs for a successfully completed command

~elseif (sense_key == 1)
             ||       RECOVERED ERROR
  1 0 = 0100 ||         No Index
  1 0 = 0300 ||         Write Fault
  1 0 = 0800 ||         LUN communication error
  1 0 = 0900 ||         Track Following Error
  1 0 = 1000 ||         ID CRC error
  1 0 = 1103 ||         Multiple read errors
  1 0 = 1200 ||         ID address mark not found
  1 0 = 1501 ||         Mechanical positioning error
  1 0 = 1600 ||         Data Synchronization Mark Error
  1 0 = 1700 ||         Recovered data with no ECC
  1 0 = 1701 ||         Recovered Data with Retries
  1 0 = 1702 ||         Recovered Data with Positive Head Offset
  1 0 = 1703 ||         Recovered Data with Negative Head Offset
  1 0 = 1801 ||         Recovered read error with 12 bit ECC (w/syndrome match)
  1 0 = 1880 ||         Recovered read error with 16 bit ECC (w/syndrome match)
  1 0 = 1881 ||         Recovered read error with 12 bit ECC (no syndrome match)
  1 0 = 1882 ||         Recovered read error with 16 bit ECC (no/syndrome match)
  1 0 = 1900 ||         Defect list error
  1 0 = 1c00 ||         Defect list not found
  1 0 = 3700 ||         Rounded parameter

~elseif (sense_key == 2)
             ||       NOT READY
  1 0 = 0400 ||         File Not Ready, cause not reportable
  1 0 = 0401 ||         File Not Ready, in process of becoming ready
  1 0 = 0402 ||         File Not Ready, Start Unit command required
  1 0 = 0403 ||         File Not Ready, manual intervention required
  1 0 = 0404 ||         File Not Ready, Format command in progress
  1 0 = 0800 ||         File Not Ready, logical unit communication failure
  1 0 = 4400 ||         File Not Ready, internal target failure

~elseif (sense_key == 3)
             ||       MEDIUM ERROR
  1 0 = 1000 ||         ID CRC error
  1 0 = 1100 ||         Unrecoverable Read Error
  1 0 = 1101 ||         Read retires exhausted
  1 0 = 1102 ||         Error too long to correct
  1 0 = 1200 ||         ID address mark not found
  1 0 = 1400 ||         Record not found
  1 0 = 1501 ||         Mechanical positioning error
  1 0 = 1502 ||         Positioning error detected by read of medium
  1 0 = 1600 ||         Data Synchronization Mark Error
  1 0 = 1c00 ||         Defect list not found
  1 0 = 3100 ||         Medium Format Corrupted
  1 0 = 3200 ||         No defect spare location available
  1 0 = 4400 ||         Internal target failure

~elseif (sense_key == 4)
             ||       HARDWARE ERROR
  1 0 = 0100 ||         No Index/Sector Signal
  1 0 = 0200 ||         No Seek Complete
  1 0 = 0300 ||         Write fault
  1 0 = 0400 ||         Drive not ready, cause not reportable
  1 0 = 0800 ||         Logical unit communication failure
  1 0 = 0900 ||         Track following error
  1 0 = 1000 ||         ID CRC error
  1 0 = 1901 ||         Defect list not available
  1 0 = 1902 ||         Defect List Error in Primary List
  1 0 = 1b00 ||         Synchronous data transfer error
  1 0 = 1c01 ||         Primary defect list not found
  1 0 = 2100 ||         Logical block address out of range
  1 0 = 4200 ||         Power-on or self-test failure
  1 0 = 4400 ||         Internal Target Failure
  1 0 = 4700 ||         SCSI parity error

~elseif (sense_key == 5)
             ||       ILLEGAL REQUEST
  1 0 = 2000 ||         Invalid Command Operation Code
  1 0 = 2100 ||         Logical Block Address out of Range
  1 0 = 2200 ||         Illegal function for this device type
  1 0 = 2400 ||         Invalid field in CDB
  1 0 = 2500 ||         Logical Unit Not Supported
  1 0 = 2600 ||         Invalid Field in Parameter List
  1 0 = 2601 ||         Parameter not supported
  1 0 = 2602 ||         Parameter value invalid
  1 0 = 3d00 ||         Invalid Bits in Identify Message
  1 0 = 4900 ||         Invalid message error

~elseif (sense_key == 6)
             ||       UNIT ATTENTION
  1 0 = 2800 ||         Not Ready to Ready Transition
  1 0 = 2900 ||         Power On or Reset occurred
  1 0 = 2a01 ||         Mode Select parameters written by another initiator
  1 0 = 2a02 ||         Log parameters changed
  1 0 = 2f00 ||         Commands Cleared by Another Initiator
  1 0 = 3f00 ||         Target operating conditions may have changed (fmt unit)
  1 0 = 3f01 ||         Microcode has been changed
  1 0 = 3f02 ||         Changed operating definition
  1 0 = 4200 ||         Power on or self test failure
  1 0 = 5c00 ||         RPL status change
  1 0 = 5c01 ||         Spindles Synchronized
  1 0 = 5c02 ||         Spindles Not Synchronized

~elseif (sense_key == 0x7)
             ||       DATA PROTECT
  1 0 = 2700 ||         Write Protected

~elseif (sense_key == 0xb)
             ||       ABORTED COMMAND
  1 0 = 1b00 ||         Synchronous Data Transfer Error
  1 0 = 4300 ||         Message Error
  1 0 = 4700 ||         SCSI Parity Error
  1 0 = 4800 ||         Initiator Detected Error Message Received
  1 0 = 4e00 ||         Overlapped Commands Attempted

~elseif (sense_key == 0xe)
             ||       MISCOMPARE
  1 0 = 1d00 ||         Miscompare During Verify Operation

~else
             ||       NOT IMPLEMENTED
              |         Undefined sense key value.
~endif

1 2 @         |    f) Field Replaceable Unit Code (0x%.4x)

1 3 @ 24      |    g) Sense Key Specific Information (0x%.6x)

1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  * If Recovered Error or Medium Error or Hardware Error
  ~if ( (sense_key == 1) || (sense_key == 3) || (sense_key == 4) )

    1 4 @ 16  |       Actual Retry Count = %d decimal

  * If Not Ready
  ~elseif (sense_key == 2)

    1 4 @ 16  |       Fraction of cmd in progress complete = %d (decimal)
              %         (Amount completed / 65535)

  * If Illegal Request
  ~elseif (sense_key == 5)

    1 3 ^ 40       ^CD
    1 3 ^ 8        ^BPV
    1 3 @          @temp_byte
    bit_ptr := (temp_byte & 0x7)
    1 4 @ 16       @field_ptr

              %       Error in the Mode Select Command

    * If in the Command Descriptor Block else Data Parameter
    ~if (CD)
              %       Illegal parameter in the Command Descriptor Block
              %         Byte 0x%.8x of the Command Descriptor Block ~field_ptr
    ~else
              %       Illegal param sent by the initiator during DATA OUT phase
              %         Byte 0x%.8x of the parameter data ~field_ptr
    ~endif

    * if Bit Pointer Value is valid
    ~if (BPV)
              %         Bit %d of the specified byte ~bit_ptr
    ~endif

  ~endif
~endif

              % 7) Error segment information
4 20  @  32   |     Segment count in which err occurred      = 0x%.8x
4 24  @  32   |     Byte count within seg where err occurred = 0x%.8x

!osdisk
* Sense information for a Generic OEM Hardfile
%               Non-IBM or Unknown Hardfile Sense Data Analysis
%
* Set flag indicating this is an oemhf  device
#set is_oemhf 1
#set is_scsi_dasd 1
* Do particulars for hardfiles
#include scsi_hf

!ost
* For these purposes, assuming 8mm5gb tape drive
% Warning: Device type is ost.  This could be several things,
%          Decoding as if 8mm5gb, but this could be wrong.
#include 8mm5gb

!hscsi
* Sense information for general SCSI Errors
%                  SCSI Adapter Error Sense Data Analysis
%
#define pioerr
#define temp
#define op_code
#define ID
#define DT
#define cmd_len
#define burst
#define residual
#define length
#define LUN
#define valid
* See /usr/include/sys/scsi.h struct rc for details

%
0  0  @      | 1) Validity of Diagnostics Status/Add Hwr Status (0x%.2x)
* See if diag status valid or invalid
0  0  =  01  |    Diagnostic Status Valid $diag_stat
0  0  != 01  |    No diagnostic Status
* See if AHS valid
0  1  =  01  |    Additional Hardware Status valid $ahs_valid
0  1  != 01  |    No additional hardware status

~if diag_stat
  0 2 @      | 2) Diagnostic Status (0x%.2x)
  0 2 ^  01  ||    Data miscompare during diag (mbox addr holds eff. addr)
  0 2 ^  02  ||    Recovered PIO error occurred (mbox addr holds eff addr)
  0 2 ^  04  ||    Unrecovered PIO Error occurred
~endif
~if ahs_valid
  0 3 @      | 3) Additional Hardware Status (0x%.2x)
  0 3 ^  01  ||    Error other than data parity on PIO Read    $pioerr
  0 3 ^  02  ||    Error other than data parity on PIO write   $pioerr
  0 3 ^  04  ||    Data Parity error on PIO Read            $pioerr
  0 3 ^  08  ||    Data Parity error on PIO write           $pioerr
  0 3 ^  10  ||    Error during DMA transfer
  0 3 ^  20  ||    Unknown card error occurred (POS, other regs valid saveISR)
  0 3 ^  40  ||    Invalid card interrupt occurred (see ISR contents)
  0 3 ^  80  ||    A command time out occurred
~endif

~if pioerr
%
  0 4 @  32  | 4) Supposed channel status reg contents at time of err (0x%.8x)
~endif
~if ( ! pioerr )
%
  0 4 @  32  | 4) Failed system call return code (0x%.8x)
~endif

*
%
             % 5) Device Driver Information
  0 8  @ 32  |    a) Unique Error Number  = 0x%.8x
 0 10 = 0012 ||       Unknown select
 0 10 = 0014 ||       Possible SYSDMA/Fatal Error
 0 10 = 0034 ||       Possible Inquiry Timeout (may be loose term/cable/noise)
 0 10 = 0042 ||       Possible PIO Error
 0 10 = 000A ||       Fatal Error most likey
 0 10 = 000B ||       SCSI Bus Reset Most likely
 0 10 = 000C ||       Valid reselect received from an unknown device
             |        Note experience says could be cabling/terminator problems
 0 10 = 000F ||       Probable System Bus DMA Error

  0 12 @ 32  |    b) Mail box number      = 0x%.8x
~if pioerr
  0 16 @ 32  |    c) Address of PIO error = 0x%.8x
~else
  0 16 @ 32  |    c) Mail box address     = 0x%.8x
~endif

  0 20 @ 32  |    d) Sys buf address      = 0x%.8x
  0 24 @ 32  |    e) xmem descr aspace_id = 0x%.8x
  0 28 @ 32  |    f) xmem subspace id     = 0x%.8x
  1  0 @ 32  |    g) pointer to sc_buf    = 0x%.8x
  1  4 @ 32  |    g) Mail box cmd state   = 0x%.8x
  1  8 @ 32  |    h) Number of tcws allowd= 0x%.8x
  1 12 @ 32  |    i) Starting TCW number  = 0x%.8x
  1 16 @ 32  |    j) Index into sta tab   = 0x%.8x
%
             % 6) Mail box information
             %    a) Mail Box Contents (Hex Values)
  1 23 @     |       Bytes  3..0  = %.2x\
  1 22 @     | %.2x\
  1 21 @     | %.2x\
  1 20 @     | %.2x
  1 27 @     |       Bytes  7..4  = %.2x\
  1 26 @     | %.2x\
  1 25 @     | %.2x\
  1 24 @     | %.2x
  1 31 @     |       Bytes 11..8  = %.2x\
  1 30 @     | %.2x\
  1 29 @     | %.2x\
  1 28 @     | %.2x
  2  3 @     |       Bytes 15..12 = %.2x\
  2  2 @     | %.2x\
  2  1 @     | %.2x\
  2  0 @     | %.2x
  2  7 @     |       Bytes 19..16 = %.2x\
  2  6 @     | %.2x\
  2  5 @     | %.2x\
  2  4 @     | %.2x
  2 11 @     |       Bytes 23..20 = %.2x\
  2 10 @     | %.2x\
  2  9 @     | %.2x\
  2  8 @     | %.2x
  2 15 @     |       Bytes 27..24 = %.2x\
  2 14 @     | %.2x\
  2 13 @     | %.2x\
  2 12 @     | %.2x
  2 19 @     |       Bytes 31..28 = %.2x\
  2 18 @     | %.2x\
  2 17 @     | %.2x\
  2 16 @     | %.2x
 
 1 20 @ @temp    
 op_code := temp & 0xf

~if (op_code == 3)
             %    b) Mail Box Contents (Decoded)

 1 21 @ @temp
 ID := temp & 0x7
 DT := temp >> 3
 DT := temp & 0x3

 1 22 @ @temp
 cmd_len := temp & 0xf
 burst := temp >> 4
 burst := burst & 0x7

 2 1 @ @temp
 LUN := temp >> 5
 LUN := LUN & 0x7

             %       SCSI ID = 0x%.2x ~ID
 1 21 !^ 10  |       No Data Transfer (DMA field invalid)
 DT = 1      |       Data to be written to the SCSI bus.
 DT = 3      |       Data to be read from the SCSI bus.

 1 21 !^ 80  |       Allow the device to disconnect for this command. 
 1 21 ^ 80   |       Do not allow the device to disconnect for this command. 

             %       SCSI Command length = %d ~cmd_len

 burst = 0   |       DMA burst length of 64 bytes.
 burst = 1   |       DMA burst length of 64 bytes.
 burst = 2   |       DMA burst length of 4 bytes.
 burst = 3   |       DMA burst length of 8 bytes.
 burst = 4   |       DMA burst length of 16 bytes.
 burst = 5   |       DMA burst length of 32 bytes.
 burst = 6   |       DMA burst length of 64 bytes.
 burst = 7   |       DMA burst length of 128 bytes.

 1 23 @      |       Sequence Number = 0x%.2x (Mailbox to be loaded with the next command).

  1 27 @     |       DMA Start Address for data transfer = 0x%.2x\
  1 26 @     |%.2x\
  1 25 @     |%.2x\
  1 24 @     |%.2x

*1 28 @ 24 @length
*            %       DMA Data transfer length in bytes = 0x%.6x = %d (decimal) ~length ~length
*            %       LUN = %d ~LUN
*2 12 @ 24 @residual
*            %       Residual = 0x%.6x = %d (decimal) ~residual ~residual

 2 15 ^ 80 ^valid
  ~if (valid)
 2 18 @      |       SCSI Status = 0x%.2x
  ~endif

 2 16 @      |       Adapter RC Status (0x%.2x)
 2 16 = 00   |         Initial Status
 2 16 = 01  ||         Command Parameters to mailbox not valid
 2 16 = 02  ||         Mailbox parity error during mailbox load operation
 2 16 = 03  ||         Cmd loaded into mailbox not pointed to by prev cmd
 2 16 = 04  ||         Probable Adapter fuse failure (Ctrller terminal pwr err.)
 2 16 = 05  ||         Controller hardware failure. RAM other hw error found
 2 16 = 06  ||         SCSI Bus reset occurred
 2 16 = 07  ||         Valid reselect from unknown device ^reselect
 2 16 = 08  ||         Command terminated by an initialize cmd
 2 16 = 09  ||         Cmd rejected due to error in a previous cmd
 2 16 = 0b  ||         Cmd cant because other cmds in progress would be disrupted
 2 16 = 0c  ||         Diags in a paused state after an error condition
 2 16 = 10  ||         Cmd or IPL diags paused.
 2 16 = 15  ||         Diags ran successfully after a SCSI bus reset
 2 16 = 1F  ||         Command completed with errors
 2 16 = FF   |         Command completed without errors

 2 17 @      |       Extra Status (0x%.2x)
 2 17 = 00   |         No extra status
 2 17 = 01  ||         SCSI device does not respond to selection by controller
 2 17 = 02  ||         SCSI device did not transfer correct number of bytes
 2 17 = 03   |         Transfer occured with residual non zero. Normal for
             |           some cmds to a variable block device
 2 17 = 04  ||         Checksum error in downloaded data.
~if (reselect)
 2 17 @      |         Device asserting the reselect = %d
~else
 2 17 = 05  ||         Microchannel Parity error during DMA operation
~endif
 2 17 = 08  ||         SCSI bus unexpectedly freed
 2 17 = 0A  ||         System Aborted (due to SCSI errors or SCSI Bus reset)
 2 17 = 0E   |         Controller ready for next block for downloading code
 2 17 = 0f   |         Resend last block of data. (Err in last block)
 2 17 = 80  ||         ROM CRC check
 2 17 = 81  ||         Contoller RAM error discovered
 2 17 = 82  ||         Microchannel interface control-chip control error
 2 17 = 83  ||         SCSI chip POR error discovered during diagnostics
 2 17 = 88  ||         SCSI chip register error discovered
 2 17 = 89  ||         SCSI chip register error discovered
 2 17 = 8f   |         Resume adapter diagnostics completed w/o further errors

~endif
%
             % 7) Mail Box 31 status area
 2 20 @ 32   |       0x%.8x \
 2 24 @ 32   | 0x%.8x

 2 24 @      |    a) Status (0x%.2x)
 2 24 = 00   |       Initial Status
 2 24 = 01   ||       Command Parameters to mailbox not valid
 2 24 = 02   ||       Mailbox parity error during mailbox load operation
 2 24 = 03   ||       Cmd loaded into mailbox not pointed to by prev cmd
 2 24 = 04   ||       Probable Adapter fuse failure (Ctrller terminal pwr err.)
 2 24 = 05   ||       Controller hardware failure. RAM other hw error found
 2 24 = 06   ||       SCSI Bus reset occurred
 2 24 = 07   ||       Valid reselect from unknown device ^reselect
 2 24 = 08   ||       Command terminated by an initialize cmd
 2 24 = 09   ||       Cmd rejected due to error in a previous cmd
 2 24 = 0b   ||       Cmd cant because other cmds in progress would be disrupted
 2 24 = 0c   ||       Diags in a paused state after an error condition
 2 24 = 10   ||       Diag. Cmd or IPL diags paused.
 2 24 = 15   ||       Soft failure. Diags ran ok after failure
 2 24 = 1F   ||       Command completed with errors
 2 24 = FF   |       Command completed without errors

 2 25 @      |    b) Extra Status (0x%.2x)
 2 25 = 00   |       No extra status
 2 25 = 01   ||       SCSI device does not respond to selection by controller
 2 25 = 02   ||       SCSI device did not transfer correct number of bytes
 2 25 = 03   |        Transfer occured with residual non zero. Normal for
             |        some cmds to a variable block device
 2 25 = 04   ||       Checksum error in downloaded data.
~if (reselect)
 2 25 @      |        Device asserting the reselect = %d
~else
 2 25 = 05   ||       Microchannel Parity error during DMA operation
~endif
 2 25 = 08   ||       SCSI bus unexpectedly freed
 2 25 = 0A   ||       Cmd aborted due SCSI Bus reset or SCSI errors
 2 25 = 0E   |        Controller ready for next block for downloading code
 2 25 = 0f   |        Resend last block of data. (Err in last block)
 2 25 = 80   ||       ROM CRC check
 2 25 = 81   ||       Adapter RAM error discovered
 2 25 = 82   ||       Microchannel interface control-chip control error
 2 25 = 83   ||       SCSI chip logic error discovered.
 2 25 = 88   ||       SCSI chip POR error discovered during diagnostics
 2 25 = 89   ||       SCSI chip register error discovered
 2 25 = 8f   |        Resume adapter diagnostics completed w/o further errors

* 3 Bytes reserved
%
             % 8) Miscellaneous Status
  2 28 @ 32  |    a) ISR (Interrupt Status Register) = 0x%.8x
  3  0 @ 24  |    b)                        Reserved = 0x%.6x
  3  3 @     |    c)    BCR (Basic Control Register) = 0x%.2x
  3  4 @     |    d) Pos Reg 0 =0x%.2x \
  3  5 @     |  Reg 1 =0x%.2x \
  3  6 @  8  |  Reg 2 =0x%.2x \
  3  7 @  8  |  Reg 3 =0x%.2x
  3  8 @  8  |       Pos Reg 4 =0x%.2x \
  3  9 @  8  |  Reg 5 =0x%.2x \
  3 10 @  8  |  Reg 6 =0x%.2x \
  3 11 @  8  |  Reg 7 =0x%.2x


!pscsi
#set is_pscsi 1
#include scsi_int_gen

!ncr810
#set is_ncr810 1
#include scsi_int_gen

!8fba
#set is_8fba 1
#include scsi_int_gen

!scsi_int_gen
* Sense information for general SCSI Errors for Integrated SCSI Adapter
%          Integrated SCSI Adapter Error Sense Data Analysis
%
%
#define is_pscsi
#define is_ncr810
#define is_8fba

pioerr   := 0
chnerr   := 0
dmaerr   := 0
unkerr   := 0
no_reg   := 0
isr_invalid := 1

0  0  @      | 1) Validity of Diagnostics Status/Add Hwr Status (0x%.2x)
* See if diag status valid or invalid
0  0  =  01  |    Diagnostic Status Valid $diag_stat
0  0  != 01  |    No diagnostic Status
* See if AHS valid
0  1  =  01  |    Additional Hardware Status valid $ahs_valid
0  1  != 01  |    No additional hardware status

~if diag_stat
  0 2 @      | 2) Diagnostic Status (0x%.2x)
  0 2 ^  01  ||    Data miscompare during diag
  0 2 ^  02  ||    Recovered PIO error occurred
  0 2 ^  04  ||    Unrecovered PIO Error occurred
~endif

~if ahs_valid
  0 3 @      | 3) Additional Hardware Status (0x%.2x)
  0 3 =  01  ||    Error other than data parity on PIO read  $pioerr $ahs_on $no_reg
  0 3 =  02  ||    Error other than data parity on PIO write $pioerr $no_reg
  0 3 =  04  ||    Data Parity error on PIO Read             $pioerr $no_reg
  0 3 =  05  ||    Error Occurred on Copy $no_reg
  0 3 =  06  ||    Unknown error occurred $unkerr
  0 3 =  07  ||    Unknown or unexpected interrupt occurred
  0 3 =  08  ||    Data Parity error on PIO write            $pioerr $no_reg
  0 3 =  09  ||    Received unexpected status from target scsi device
  0 3 =  0a  ||    Message error received on abort
  0 3 =  0b  ||    Message error received on bdr
  0 3 =  0c  ||    Bad LUN presented on bus
  0 3 =  0d  ||    A channel check has occurred $chnerr        
  0 3 =  0e  ||    Bad parity on the SCSI bus
  0 3 =  0f  ||    A SCSI gross error received
  0 3 =  10  ||    Error during DMA transfer $dmaerr
  0 3 =  11  ||    Invalid SCSI ID on SCSI Bus
  0 3 =  12  ||    Unknown SCSI msg received
  0 3 =  13  ||    Phase error on SCSI bus
  0 3 =  14  ||    Ext message expected and not received
  0 3 =  15  ||    Target device dropped off the SCSI bus unexpectedly
  0 3 =  16  ||    Unexpected selection timeout
  0 3 =  17  ||    SCSI Bus reset detected
  0 3 =  20  ||    Unknown card error occurred  $isr_invalid
  0 3 =  40  ||    Invalid card interrupt occurred
  0 3 =  80  ||    A command time out occurred $no_reg
~endif

~if pioerr
  0 4 @  32    | 4) Supposed channel status reg contents at time of err (0x%.8x)
~elseif dmaerr
  0 4 @  32    | 4) Return code from system call which indicated the DMA fail (0x%.8x)
~endif

~if ( ! pioerr )
  0 4 @  32  | 4) Failed system call return code (0x%.8x)
~endif

0 8  @ 32  | 5) Device Driver Error Number (0x%.8x) @dnum

* If unknown error and unique error is 0A then no register information is available
~if ( ( unkerr ) && ( dnum == 0a ) )
   no_reg := 1
~endif

           % 6) Last SCSI command seen by SCSI controller
#include scsi_cmnd

           % 7) Other Cmd/Controller Information
0 24       |    a) Target SCSI ID         = 0x%.2x
0 25       |    b) Queue State            = 0x%.2x @curstate
0 25 = 00  |       Active

~if ( is_8fba && ( curstate == 01 || curstate == 02 ) )
             %       If AIX is level 4.1.1 then value indicates
  0 25 = 01  |       Active
  0 25 = 02  |       Wait_Flush
             %       Otherwise the value indicates
  0 25 = 01  |       Stopping
  0 25 = 02  |       Halted
~endif

0 25 = 04  |       Halted
0 25 = 08  |       Halted_Cac
0 25 = 10  |       Wait_Info
0 25 = 20  |       Stopping

0 26 @     |    c) Command Activity State = 0x%.2x @curstate
0 26 = 01  |       Nothing in Progress
0 26 = 02  |       BDR in Progress
0 26 = 03  |       Abort in Progress

~if ( is_8fba && ( curstate == 04 ) )
             %       If AIX is level 4.1.1 then value indicates
  0 26 = 04  |       BDR in Progress
             %       Otherwise the value indicates
  0 26 = 04  |       Abort in Progress
~endif

0 26 = 05  |       Cmd in Progress
0 26 = 06  |       Negotiate in Progress
0 26 = 08  |       BDR in Progress
0 26 = 10  |       Negotiate in Progress

0 27 @     |    d) Resource State         = 0x%.2x
0 27 = 1   |       Large TCW Resoruces Used
0 27 = 2   |       Small TCW Resoruces Used
0 27 = 3   |       STA Resoruces Used
0 27 = 4   |       No TCW/STA resources used

*  Not currently used
*  0 28 @ 32  |    e) Sys buf address        = 0x%.8x
*  1 0  @ 32  |    f) X mem descr aspace     = 0x%.8x
*  1 4  @ 32  |    f) X mem descr subspace   = 0x%.8x
*  1 8  @ 32  |    g) Pointer to sc_buf      = 0x%.8x

~if ( pioerr || chnerr )
  1 12 @ 32  |     e) Offset part of address trying to be read = 0x%.8x
~endif

~if ( ! no_reg )
             % 8) NCR Status Register Information
  1 16 @     |    a) ISTAT Register            = 0x%.2x
  1 16 ^ 80  |       Software aborting microcode on chip (may be okay)
  1 16 ^ 40  |       Software is resetting all chip registers
  1 16 ^ 20  |       Software is signalling chip
  1 16 ^ 10  |       Semaphore is set
  1 16 ^ 08  |       Chip connected to SCSI bus
* 1 16 ^ 04  |       Not used (1 on 700)
  1 16 ^ 02  |       SCSI interrupt from chip to host is pending
  1 16 ^ 01  |       DMA interrupt from chip to host is pending

  1 17 @     |    b) DSTAT Register            = 0x%.2x
  *  Apparently no longer used
  *  1 17 ^ 80  |       DMA FIFO On Chip is empty
  *  1 17 ^ 10  |       CHIP was aborted
  *  1 17 ^ 04  |       Microcode interrupted by the host
  *  1 17 ^ 01  |       Illegal instruction executed by the Chip (may be normal)

  1 18 @     |    c) SSTAT0 Register           = 0x%.2x
  ~if ( is_8fba || is_ncr810 )
     1 18 ^ 80  |       SIDL least significant byte full
     1 18 ^ 40  |       SODR least significant byte full
     1 18 ^ 20  |       SODL least significant byte full
     1 18 ^ 10  |       Arbitration in progress
     1 18 ^ 08  |       Lost aribitration
     1 18 ^ 04  |       Won arbitration
     1 18 ^ 02  |       SCSI reset signal
     1 18 ^ 01  |       SCSI SDPO parity signal

     1 19 @     |    d) SSTAT1 Register           = 0x%.2x @nibble
     nibble := nibble & 0xf0
                %       Bytes or word in the SCSI FIFO = 0x%.2x ~nibble
     1 19 ^ 08  |       Lost aribitration
     1 19 ^ 04  |       Won arbitration
     1 19 ^ 02  |       SCSI reset signal
     1 19 ^ 01  |       SCSI SDPO parity signal

     1 20 @     |    e) SSTAT2 Register           = 0x%.2x
     1 20 ^ 80  |       SIDL most significant byte full
     1 20 ^ 40  |       SODR most significant byte full
     1 20 ^ 20  |       SODL most significant byte full
     1 20 ^ 10  |       Reserved
     1 20 ^ 08  |       Latched SCSI parity for SIDL15-8
     1 20 ^ 04  |       Reserved
     1 20 ^ 02  |       Last Disconnect
     1 20 ^ 01  |       SCSI SDP1 parity signal
  ~endif

  ~if ( is_pscsi )
     1 19 @     |    d) SSTAT1 Register           = 0x%.2x
     1 19 ^ 80  |       SIDL least significant byte full
     1 19 ^ 40  |       SODR least significant byte full
     1 19 ^ 20  |       SODL least significant byte full
     1 19 ^ 10  |       Arbitration in progress
     1 19 ^ 08  |       Lost aribitration
     1 19 ^ 04  |       Won arbitration
     1 19 ^ 02  |       SCSI reset signal
     1 19 ^ 01  |       SCSI SDPO parity signal

     1 20 @     |    d) SSTAT2 Register           = 0x%.2x @nibble
     nibble := nibble & 0xf0
                %       Bytes or word in the SCSI FIFO = 0x%.2x ~nibble
     1 20 ^ 08  |       Lost aribitration
     1 20 ^ 04  |       Won arbitration
     1 20 ^ 02  |       SCSI reset signal
     1 20 ^ 01  |       SCSI SDPO parity signal
  ~endif
~else
             % 8) No Available Status Register Information
~endif


             % 9) Additional Register Information
  1 21 @     |    a) Encoded SCSI Addr of Device selected   = (0x%.2x) @sdid
  ~if ( is_pscsi )
    1 21   ^80 |       SCSI ID = 7
    1 21   ^40 |       SCSI ID = 6
    1 21   ^20 |       SCSI ID = 5
    1 21   ^10 |       SCSI ID = 4
    1 21   ^08 |       SCSI ID = 3
    1 21   ^04 |       SCSI ID = 2
    1 21   ^02 |       SCSI ID = 1
    1 21   ^01 |       SCSI ID = 0
  ~else
    sdid := sdid & 0x0f
    sdid = 07  |       SCSI ID = 7
    sdid = 06  |       SCSI ID = 6
    sdid = 05  |       SCSI ID = 5
    sdid = 04  |       SCSI ID = 4
    sdid = 03  |       SCSI ID = 3
    sdid = 02  |       SCSI ID = 2
    sdid = 01  |       SCSI ID = 1
    sdid = 00  |       SCSI ID = 0
  ~endif

  ~if ( is_pscsi )
    1 22 @     |    b) Encoded SCSI ID of the chip   = (0x%.2x)
    1 22   ^80 |       SCSI ID = 7
    1 22   ^40 |       SCSI ID = 6
    1 22   ^20 |       SCSI ID = 5
    1 22   ^10 |       SCSI ID = 4
    1 22   ^08 |       SCSI ID = 3
    1 22   ^04 |       SCSI ID = 2
    1 22   ^02 |       SCSI ID = 1
    1 22   ^01 |       SCSI ID = 0
  ~else
    1 22 @     |    b) SCID value = (0x%.2x)
    1 22   ^40 |         Chip is configured to respond to SCSI reselection
    1 22   ^20 |         Chip is configured to respond to SCSI selection
    scid := scid & 0x0f
               %         ID the chip will assert when arbitrating for the  SCSI bus = (0x%.2x) ~scid
    1 23 @     |       NCR Chip Revision Level = 0x%.2x
  ~endif

1 24 @ 32  |    c) First word of currently executing microcode instruction = 0x%.8x
2 00 @ 32  |    c) Second word of currently executing microcode instruction = 0x%.8x
1 28 @ 32  |    d) Address of the next microcode intstruction to be executed = 0x%.8x

~if ( is_pscsi || 8fba )
             %10) NCR POS Register Storage
  2 4  @ 16  |    a) Card ID (POS 0 and 1) =0x%.4x
  2 4 = f48f |       f48f Indicates NCR 700 Chip
  2 4 = ba8f |       ba8f Indicates NCR 720 Chip
  2 4 = 0000 |       0000 For Card ID indicates card is busy

  2 6  @     |    b) POS 2 = 0x%.2x
  2 6  ^ 80  |       SCSI IRQ = 5
  2 6 !^ 80  |       SCSI IRQ = 4
  2 6  ^ 10  |       Even Address Parity
  2 6  ^ 08  |       Even Data Parity
  2 6  ^ 04  |       Enabled select feedback exception IRQ
  2 6  ^ 02  |       SCSI Parity Enabled
  2 6 !^ 02  |       SCSI Parity Not Enabled (not typical mode)
  2 6  ^ 01  |       SCSI Card Enabled
  2 6 !^ 01  |       SCSI Card Not Enabled

  2 7  @     |    c) POS 3 (VPD Data; only used on NCR700) = 0x%.2x

  2 8  @     |    d) POS 4 = 0x%.2x @scsi_arb
                     scsi_arb := scsi_arb & 0x0f
  2 8  !^ 80 |       External SCSI port disabled (SECURE)
  2 8  !^ 10 |       SCSI Fairness disabled (not typical mode)
  2 8  ^ 10  |       SCSI Fairness enabled
             %       SCSI DMA Arb Level (lower nibble of Reg 4) =0x%.2x ~scsi_arb

  2 9  @     |    e) POS 5 = 0x%.2x
  2 9 !^ 80  |       Channel Check is Active

  2 10 @     |    f) POS 6 (VPD Offset; only used on NCR700) = 0x%.2x

  2 11 @     |    g) POS 7 (unused) = 0x%.2x

             %11) Additional NCR Register Information
  2 12 @     |    a) SIST0 = 0x%.2x
  2 13 @     |    b) SIST1 = 0x%.2x
  2 14 @     |    c) SCSI ID of the Last target device to reselect @sdid
                  sdid := sdid & 0x0f
             %       the chip = 0x%.2x ~sdid
  2 15 @     |    d) SXFER register = 0x%.2x @nibble
                  hnibble := nibble & 0xe0
                  lnibble := nibble & 0x0f
             %       High order 3 bits effect data transfer period = 0x%.2x ~hnibble
             %       Low order nibble indicate the maximum req/ack offset = 0x%.2x ~lnibble
  1 16 @     |    e) SCNTL2 = (0x%.2x)
  1 16   ^80 |       SCSI Disconnect Unexpected 
  1 16   ^40 |       Chained Mode
  1 16   ^20 |       Reserved
  1 16   ^10 |       Reserved
  1 16   ^08 |       Wide SCSI Send
  1 16   ^04 |       Reserved
  1 16   ^02 |       Reserved
  1 16   ^01 |       Wide SCSI Receive
  2 17 @     |    f) SCNTL3 = 0x%.2x @nibble
                  hnibble := nibble & 0x70
                  lnibble := nibble & 0x03
             %       Bits 6-4 affect synchronous data transfer periods = 0x%.2x ~hnibble
             %       Bits 2-0 affect synchronous data transfer periods = 0x%.2x ~lnibble
             ~if ( lnibble != hnibble )
               %       They are not the same so FAST SCSI is being used.
               2 17   ^08 |       Data transfers will be wide
             ~endif
  ~if ( is_8fba )
     2 18 @     |    g) GPREG = 0x%.2x
     2 18   ^00 |       SCSI circuit breaker has tripped
     2 19 @     |    h) SCNTL1 = 0x%.2x
     2 19   ^80 |       Extra clock cycle data setup
     2 19   ^40 |       Assert SCSI data bus
     2 19   ^20 |       Target only disable halt parity error
     2 19   ^10 |       720 connected to SCSI bus
     2 19   ^08 |       Assert SCSI RST signal
     2 19   ^04 |       Force bad parity
     2 19   ^02 |       Immediate Arbitration
     2 19   ^01 |       Start SCSI transfer

     2 20 @     |    i) SBCL (SCSI Bus Control Lines)= 0x%.2x
     2 20   ^80 |       REQ
     2 20   ^40 |       ACK
     2 20   ^20 |       BSY
     2 20   ^10 |       SEL
     2 20   ^08 |       ATN
     2 20   ^04 |       MSG
     2 20   ^02 |       C/D
     2 20   ^01 |       I/O

     2 21 @     |    j) SFBR (SCSI First Byte Received) = 0x%.2x
                |       Contains the first byte received in the last SCSI asychronous
                |       information transfer phase.  The register is also used and 
                |       modified during microcode operation.
     2 22 @     |    k) SCRATCHA0 (Only valid on AIX 411) = 0x%.2x
                |       Identifies the queue tag associated with the command being
                |       processed, or a hashed value of the target SCSI ID and LUN
                |       if either a non-queuing command is being processed or only 
                |       an I-T-L nexus has been established.
  ~endif
~endif


!8efc
* Sense information for general SCSI Errors for Wide Controller
%          SCSI Adapter Error Sense Data Analysis
%
%
#define uniquerr
pioerr   := 0
isr_invalid := 1

0  0  @      | 1) Validity of Diagnostics Status/Add Hwr Status (0x%.2x)
* See if diag status valid or invalid
0  0  =  01  |    Diagnostic Status Valid $diag_stat
0  0  != 01  |    No diagnostic Status
* See if AHS valid
0  1  =  01  |    Additional Hardware Status valid $ahs_valid
0  1  != 01  |    No additional hardware status

~if diag_stat
  0 2 @      | 2) Diagnostic Status (0x%.2x)
  0 2 ^  01  ||    Data miscompare during diag
  0 2 ^  02  ||    Recovered PIO error occurred
  0 2 ^  04  ||    Unrecovered PIO Error occurred
~endif
~if ahs_valid
  0 3 @      | 3) Additional Hardware Status (0x%.2x)
  0 3 =  01  ||    Error other than data parity on PIO read  $pioerr $ahs_on
  0 3 =  02  ||    Error other than data parity on PIO write $pioerr
  0 3 =  04  ||    Data Parity error on PIO Read             $pioerr
  0 3 =  05  ||    Error Occurred on Copy
  0 3 =  06  ||    Unknown error occurred
  0 3 =  07  ||    Unknown or unexpected interrupt occurred
  0 3 =  08  ||    Data Parity error on PIO write            $pioerr
  0 3 =  09  ||    Received unexpected status from target scsi device
  0 3 =  0a  ||    Message error received on abort
  0 3 =  0b  ||    Message error received on bdr
  0 3 =  0c  ||    Bad LUN presented on bus
  0 3 =  0d  ||    A channel check has occurred
  0 3 =  0e  ||    Bad parity on the SCSI bus
  0 3 =  0f  ||    A SCSI gross error received
  0 3 =  10  ||    Error during DMA transfer
  0 3 =  11  ||    Invalid SCSI ID on SCSI Bus
  0 3 =  12  ||    Unknown SCSI msg received
  0 3 =  13  ||    Phase error on SCSI bus
  0 3 =  14  ||    Ext message expected and not received
  0 3 =  15  ||    Target device dropped off the SCSI bus unexpectedly
  0 3 =  16  ||    Unexpected selection timeout
  0 3 =  17  ||    SCSI Bus reset detected
  0 3 =  20  ||    Unknown card error occurred  $isr_invalid
  0 3 =  40  ||    Invalid card interrupt occurred
  0 3 =  80  ||    A command time out occurred
~endif

~if pioerr
  0 4 @  32    | 4) Supposed channel status reg contents at time of err (0x%.8x)
~endif
~if ( ! pioerr )
  0 4 @  32  | 4) Failed system call return code (0x%.8x)
~endif

  0 8  @ 32       | 5) Device Driver Error Number (0x%.8x) @uniquerr
  0 8 = 00000001  ||   BUSY bit set in the adapters BCR.
  0 8 = 00000002  ||   Read Immediate Request Command timed out.
  0 8 = 00000003  ||   Assign/Delete Entity ID command timed out.
  0 8 = 00000004  ||   Invalid 'reason' code in the watchdog timer structure.
  0 8 = 00000005  ||   Get Adapter Information command timed out.
  0 8 = 00000006  ||   DMA Error (detected by system, not adapter).
  0 8 = 00000007  ||   Adapter exception reported in the BSR.
  0 8 = 00000008  ||   PIO error on load (read from adapter).
  0 8 = 00000009  ||   PIO error on store (write to adapter).
  0 8 = 0000000a  ||   Adapter reset failed.
                  |      Based on the ISR return code it is probably NOT an
                  |      an attached bus of PRC problem.
  0 8 = 0000000b  ||   Invalid reply element dequeued from an inbound pipe.
  0 8 = 0000000c  ||   Adapter reset failed.
                  |      Based on the ISR return code, cause is likely external
                  |      bus or PTC problem.
  0 8 = 00000010  ||   An adapter command failed.
                  |      - Read Immediate
                  |      - Get Adapter Information
                  |      - Assign/Delete Entity ID
  0 8 = 00000011  ||   Invalid call to the asc_stub routine.
  0 8 = 00000012  ||   DMA Error.

  1 28  @ 32      | 6) DMA Return Code = 0x%.8x
  2 04  @ 32      | 7) Size of PIO = 0x%.8x
  2 08  @         | 8) Target SCSI ID = 0x%.2x
  2 09  @         | 9) I/O Register = 0x%.2x
                  % 10) POS Registers
  2 10  @         |       POS 0  = 0x%.2x
  2 11  @         |       POS 1  = 0x%.2x
  2 12  @         |       POS 2  = 0x%.2x
  2 13  @         |       POS 3  = 0x%.2x
  2 14  @         |       POS 4  = 0x%.2x
  2 15  @         |       POS 5  = 0x%.2x
  2 16  @         |       POS 3b = 0x%.2x
  2 17  @         |       POS 4b = 0x%.2x


!vscsi
* Sense information for general SCSI Errors for Wide Controller
%          SCSI Adapter Error Sense Data Analysis
%
%
#define uniquerr
pioerr   := 0
isr_invalid := 1

0  0  @      | 1) Validity of Diagnostics Status/Add Hwr Status (0x%.2x)
* See if diag status valid or invalid
0  0  =  01  |    Diagnostic Status Valid $diag_stat
0  0  != 01  |    No diagnostic Status
* See if AHS valid
0  1  =  01  |    Additional Hardware Status valid $ahs_valid
0  1  != 01  |    No additional hardware status

~if diag_stat
  0 2 @      | 2) Diagnostic Status (0x%.2x)
  0 2 ^  01  ||    Data miscompare during diag
  0 2 ^  02  ||    Recovered PIO error occurred
  0 2 ^  04  ||    Unrecovered PIO Error occurred
~endif
~if ahs_valid
  0 3 @      | 3) Additional Hardware Status (0x%.2x)
  0 3 =  01  ||    Error other than data parity on PIO read  $pioerr $ahs_on
  0 3 =  02  ||    Error other than data parity on PIO write $pioerr
  0 3 =  04  ||    Data Parity error on PIO Read             $pioerr
  0 3 =  05  ||    Error Occurred on Copy
  0 3 =  06  ||    Unknown error occurred
  0 3 =  07  ||    Unknown or unexpected interrupt occurred
  0 3 =  08  ||    Data Parity error on PIO write            $pioerr
  0 3 =  09  ||    Received unexpected status from target scsi device
  0 3 =  0a  ||    Message error received on abort
  0 3 =  0b  ||    Message error received on bdr
  0 3 =  0c  ||    Bad LUN presented on bus
  0 3 =  0d  ||    A channel check has occurred
  0 3 =  0e  ||    Bad parity on the SCSI bus
  0 3 =  0f  ||    A SCSI gross error received
  0 3 =  10  ||    Error during DMA transfer
  0 3 =  11  ||    Invalid SCSI ID on SCSI Bus
  0 3 =  12  ||    Unknown SCSI msg received
  0 3 =  13  ||    Phase error on SCSI bus
  0 3 =  14  ||    Ext message expected and not received
  0 3 =  15  ||    Target device dropped off the SCSI bus unexpectedly
  0 3 =  16  ||    Unexpected selection timeout
  0 3 =  17  ||    SCSI Bus reset detected
  0 3 =  20  ||    Unknown card error occurred  $isr_invalid
  0 3 =  40  ||    Invalid card interrupt occurred
  0 3 =  80  ||    A command time out occurred
~endif

~if pioerr
  0 4 @  32    | 4) Supposed channel status reg contents at time of err (0x%.8x)
~endif
~if ( ! pioerr )
  0 4 @  32  | 4) Failed system call return code (0x%.8x)
~endif

  0 8  @ 32       | 5) Device Driver Error Number (0x%.8x) @uniquerr
  0 8 = 00000001  ||   SCSI Inquiry command timed out.
                  |      SCSI Inquiry command issued through the SCIOINQ ioctl
                  |      timed out.  
  0 8 = 00000002  ||   SCSI Start Unit command timed out.
                  |      SCSI Start Unit command issued through the SCIOSTUNIT
                  |      ioctl timed out.  
  0 8 = 00000003  ||   SCSI Test Unit Ready command timed out.
                  |      SCSI Test Unit Ready command issued through the SCIOTUR
                  |      ioctl timed out.  
  0 8 = 00000004  ||   SCSI Read command timed out.
                  |      SCSI Read command issued through the SCIOREAD ioctl
                  |      timed out.  
  0 8 = 00000005  ||   Unexpected sc_buf with an SC_Q_CLR indication was received.
                  |      This can occur if errors were detected during error
                  |      recovery.
  0 8 = 00000006  ||   SCSI Bus Reset Request Completed with Error.
  0 8 = 00000008  ||   SCSI Abort or Bus Device Reset Completed with Error.
                  |      Issued through ioctl interface (SCIORESET or SCIOHALT)
  0 8 = 00000009  ||   SCSI Abort or Bus Device Reset Completed with Error.
                  |      Issued by the device driver as part of error recovery.
  0 8 = 0000000a  ||   SCSI Command Aborted by Controller
                  |      SCSI Command issued to the controller completed with
                  |      a command error code indicating it was aborted by the
                  |      controller.
  0 8 = 0000000b  ||   SCSI Command Completed with DMA Error.
                  |      SCSI Command issued to the controller completed with
                  |      a command error code indicating a DMA error.
  0 8 = 0000000c  ||   SCSI Command Completed with Unknown Command Error.
                  |      SCSI Command issued to the controller completed with
                  |      an unknown command error code.
  0 8 = 0000000d  ||   SCSI Command Completed with Device Error.
                  |      Error is one of the following:
                  |        SCSI Interface Fault
                  |        Unexpected SCSI Bus Free
                  |        Invalid SCSI Phase Sequence
                  |        Mandatory SCSI Message Rejected
  0 8 = 0000000e  ||   SCSI Command Completed with Device Error.
                  |      Error is one of the following:
                  |        Terminator Power Failure
                  |        Differential Sense Error
  0 8 = 0000000f  ||   SCSI Command Completed with Unknown Device Error.
                  |      SCSI Command issued to the controller completed with
                  |      an unknown device error code.
  0 8 = 00000010  ||   Watchdog Timer Expired (unopen)
                  |    This indicates that a watchdog timer for the device
                  |    driver expired on a device which was not open.
  0 8 = 00000011  ||   Watchdog Timer Expired (no commands active)
                  |    This indicates that a watchdog timer for the device
                  |    driver expired on a device which had no commands
                  |    active.
  0 8 = 00000012  ||   Watchdog Timer Expired (non active state)
                  |    This indicates that a watchdog timer for the device
                  |    driver expired on a device and the command at the 
                  |    head of the active list was not in the active state.
  0 8 = 00000013  ||   SCSI Abort or Bus Device Reset Timed Out.
                  |      Issued through ioctl interface (SCIORESET or SCIOHALT)
  0 8 = 00000014  ||   SCSI Abort or Bus Device Reset Timed Out.
                  |      Issued by the device driver as part of error recovery.
  0 8 = 00000015  ||   SCSI Bus Reset Sent to Controller Timed Out.
  0 8 = 00000016  ||   Watchdog Timer of Unknown Type Expired for Device Driver.
  0 8 = 00000018  ||   Device Reselected Controller (no commands)
                  |      Indicates that a device reselected the controller
                  |      when the controller had no outstanding command to the
                  |      device.
  0 8 = 00000019  ||   Controller Detected Loss of Terminator Power on SCSI Bus.
  0 8 = 0000001a  ||   Controller Detected Differential Sense Error on the SCSI Bus.
  0 8 = 0000001b  ||   Device Driver Received Asynchronous Notification of Unknown Type.
  0 8 = 0000001c  ||   Controller Detected SCSI Bus Reset or SCSI Bus Parity Error.
                  |      (While controller was acting as target device).
  0 8 = 0000001d  ||   Controller Detected Hardware Error or System Parity Error.
                  |      (While controller was acting as target device).
  0 8 = 0000001e  ||   Controller Detected Error of Unknown Type.
                  |      (While controller was acting as target device).
  0 8 = 0000001f  ||   Controller Detected SCSI Bus Reset or SCSI Bus Parity Error.
                  |      (While controller was acting as target device).
  0 8 = 00000020  ||   Controller Detected Hardware Error or System Parity Error.
                  |      (While controller was acting as target device).
  0 8 = 00000021  ||   Controller Detected Error of Unknown Type.
                  |      (While controller was acting as target device).
  0 8 = 00000100  ||   SCSI Bus Reset Initiated by the Device Driver has completed.
  0 8 = 00000101  ||   Controller Detected SCSI Bus Reset NOT Initiated by the Device Driver.

* Additional Information

* Create variable for these because it's too complex for dsense:
have_cmd := 0
~if ((uniquerr == 0a) || (uniquerr == 0b))
  have_cmd := 1
~endif
~if ((uniquerr == 0c) || (uniquerr == 0d))
  have_cmd := 1
~endif
~if ((uniquerr == 0e) || (uniquerr == 0f))
  have_cmd := 1
~endif
~if (uniquerr == 12)
  have_cmd := 1
~endif

  ~if ( uniquerr == 06 )
      0 13 @            |   Command Error Code (0x%.2x)
      0 14 @            |   Device Error Code (0x%.2x)
  ~elseif ( (uniquerr == 08) || (uniquerr == 09) )
      0 13 @            |   Command Error Code (0x%.2x)
      0 14 @            |   Device Error Code (0x%.2x)
      0 23 @ 16         |   SCSI ID for which abort or BDR was issued (0x%.4x)
      0 25 @ 16         |   LUN for which abort or BDR was issued (0x%.4x)
      0 27 = 00002400   |   Media flag indicates SCSI BDR was issued.
      0 27 = 00004400   |   Media flag indicates SCSI abort was issued.
  ~elseif ( have_cmd == 1 )
      0 12 @            |   SCSI Status (0x%.2x)
      0 13 @            |     Command Error Code (0x%.2x)
      0 14 @            |   Device Error Code (0x%.2x)
      0 23 @ 16         |   SCSI ID for which abort or BDR was issued (0x%.4x)
      0 25 @ 16         |   LUN for which abort or BDR was issued (0x%.4x)
      0 31  @           |   Command (in hex %.2x \
      1 00 @            | %.2x \
      1 01 @            | %.2x \
      1 02 @            | %.2x \
      1 03 @            | %.2x \
      1 04 @            | %.2x \
      1 05 @            | %.2x \
      1 06 @            | %.2x \
      1 07 @            | %.2x \
      1 08 @            | %.2x \
      1 09 @            | %.2x \
      1 10 @            | %.2x)
      1 15 @ 32         |   Number of bytes of data transfer (%.8x)
  ~elseif ( (uniquerr == 13) || (uniquerr == 14) )
      0 23 @ 16         |   SCSI ID for which abort or BDR was issued (0x%.4x)
      0 25 @ 16         |   LUN for which abort or BDR was issued (0x%.4x)
      0 27 = 00002400   |   Media flag indicates SCSI BDR was issued.
      0 27 = 00004400   |   Media flag indicates SCSI abort was issued.
  ~endif


!scsi_gen
* General Sense information for SCSI Errors
* Given general information on all scsi Errors. Should be included for
* for specific SCSI devices
* See /usr/include/sys/scsi.h for details
#define is_scsi_tape
#define is_scsi_cd
#define is_scsi_rwcd
#define is_serdasd
#define is_scsi_dasd
#define is_scsi_array
#define is_oemhf

#define temp_byte
#define LUN
#define controller

#set  fnd_cmnd  0
#set  request_sense_cmnd 0
#set  read_cmnd 0
#set  write_cmnd 0

%
0  0  @      | 1) SCSI Command Information (0x%.2x)

0  0  @      |    Length of SCSI command in bytes = %d @length
0  0  >   0  ^length_okay

~if is_serdasd

  0  1 @ @temp_byte
  controller := temp_byte & 0x7
  LUN := temp_byte & 0xf0
  LUN := LUN >> 4

             %      Controller = %d ~controller
  0  1  ^ 8  |      Command is to the controller.
  0  1 !^ 8  |      Command is to the drive.
             %      LUN = %d ~LUN

~else

  0  1  @    |                    SCSI Identifier = %d (decimal)
  0  3  ^ 80 |    Disconnection Not Allowed for the command
  0  3 !^ 80 |    Disconnection Allowed for the command
  0  3  ^ 08 |    Asynchronous data transfer Forced
  0  3 !^ 08 |    Asynchronous data transfer Not Forced

~endif

* Display and decode what is known of SCSI command to the extent of the
* command length.
~if length_okay
 * Display SCSI Command bytes only to length defined. Expect bytes not
 * included in the length to be zero
 0  4  @     | 2) SCSI Command (in hex %.2x \
 0  5 @ @byte2
 0  6 @ @byte3
 0  7 @ @byte4
 0  8 @ @byte5
 0  9 @ @byte6
 0 10 @ @byte7
 0 11 @ @byte8
 0 12 @ @byte9
 0 13 @ @byte10
 0 14 @ @byte11
 0 15 @ @byte12

 ~if length > 1
             % %.2x \ ~byte2
 ~elseif (byte2 != 0)
             % )
             % (Note 2nd byte was 0x%.2x but cmd length was %d \ ~byte2 length
 ~endif
 ~if length > 2
             % %.2x \ ~byte3
 ~elseif (byte3 != 0)
             % )
             % (Note 3rd byte was 0x%.2x but cmd length was %d \ ~byte3 length
 ~endif
 ~if length > 3
             % %.2x \ ~byte4
 ~elseif (byte4 != 0)
             % )
             % (Note 4th byte was 0x%.2x but cmd length was %d \ ~byte4 length
 ~endif
 ~if length > 4
             % %.2x \ ~byte5
 ~elseif (byte5 != 0)
             % )
             % (Note 5th byte was 0x%.2x but cmd length was %d \ ~byte5 length
 ~endif
 ~if length > 5
             % %.2x \ ~byte6
 ~elseif (byte6 != 0)
             % )
             % (Note 6th byte was 0x%.2x but cmd length was %d \ ~byte6 length
 ~endif
 ~if length > 6
             % %.2x \ ~byte7
 ~elseif (byte7 != 0)
             % )
             % (Note 7th byte was 0x%.2x but cmd length was %d \ ~byte7 length
 ~endif
 ~if length > 7
             % %.2x \ ~byte8
 ~elseif (byte8 != 0)
             % )
             % (Note 8th byte was 0x%.2x but cmd length was %d \ ~byte8 length
 ~endif
 ~if length > 8
             % %.2x \ ~byte9
 ~elseif (byte9 != 0)
             % )
             % (Note 9th byte was 0x%.2x but cmd length was %d \ ~byte9 length
 ~endif
 ~if length > 9
             % %.2x \ ~byte10
 ~elseif (byte10 != 0)
             % )
             % (Note 10th byte was 0x%.2x but cmd length was %d \ ~byte10 length
 ~endif
 ~if length > 10
             % %.2x \ ~byte11
 ~elseif (byte11 != 0)
             % )
             % (Note 11th byte was 0x%.2x but cmd length was %d \ ~byte11 length
 ~endif
 ~if length > 11
             % %.2x \ ~byte12
 ~elseif (byte12 != 0)
             % )
             % (Note 12th byte was 0x%.2x but cmd length was %d \ ~byte12 length
 ~endif
             % )

 * Display SCSI command based on SCSI device
 ~if (is_serdasd || is_scsi_dasd || is_scsi_array)
   * Is a hardfile SCSI command see if it is a known one
   0  4  = 00 |    Test unit ready                              $fnd_cmnd
   0  4  = 01 |    Rezero unit                                  $fnd_cmnd
   0  4  = 03 |    Request Sense Data   $request_sense_cmnd     $fnd_cmnd
   0  4  = 04 |    Format unit                                  $fnd_cmnd
   0  4  = 07 |    Reassign block                               $fnd_cmnd
   0  4  = 08 |    Read                  $read_cmnd             $fnd_cmnd
   0  4  = 0A |    Write                 $write_cmnd            $fnd_cmnd
   0  4  = 0B |    Seek                                         $fnd_cmnd
   0  4  = 12 |    Inquiry                                      $fnd_cmnd
   0  4  = 15 |    Mode select                                  $fnd_cmnd
   0  4  = 16 |    Reserve Unit                                 $fnd_cmnd
   0  4  = 17 |    Release Unit                                 $fnd_cmnd
   0  4  = 1a |    Mode sense                                   $fnd_cmnd
   0  4  = 1b |    Stop/start unit                              $fnd_cmnd
   0  4  = 1c |    Receive diagnostics results                  $fnd_cmnd
   0  4  = 1d |    Send diagnostic                              $fnd_cmnd
   0  4  = 25 |    Read capacity                                $fnd_cmnd
   0  4  = 28 |    Read extended                                $fnd_cmnd
   0  4  = 2a |    Write extended                               $fnd_cmnd
   0  4  = 2b |    Seek extended                                $fnd_cmnd
   0  4  = 2e |    Write and verify                             $fnd_cmnd
   0  4  = 2f |    Verify                                       $fnd_cmnd
   0  4  = 37 |    Read defect list                             $fnd_cmnd
   0  4  = 3B |    Write buffer                                 $fnd_cmnd
   0  4  = 3C |    Read buffer                                  $fnd_cmnd
   0  4  = E8 |    Read long  (IBM hardfiles)                   $fnd_cmnd
   0  4  = Ea |    Write long (IBM hardfiles)                   $fnd_cmnd
   ~if (! fnd_cmnd)
              %    Warning SCSI Cmnd is supposed to be a hardfile command, but
              %    the command is not recognized as a hardfile command.
        ~if is_oemhf
              %    It may be unique to the OEM vendor of the hardfile, however.
        ~endif
   ~endif

 ~elseif (is_scsi_tape)
 * Is a tape, look for command in set of tape commands

   0  4  = 00 |    Test Unit Ready                             $fnd_cmnd
   0  4  = 01 |    Rewind                                      $fnd_cmnd
   0  4  = 03 |    Request Sense Data   $request_sense_cmnd    $fnd_cmnd
   0  4  = 05 |    Read block limits                           $fnd_cmnd
   0  4  = 08 |    Read                                        $fnd_cmnd
   0  4  = 0A |    Write                                       $fnd_cmnd
   0  4  = 0D |    Extended Diagnostics                        $fnd_cmnd
   0  4  = 10 |    Write Filemark                              $fnd_cmnd
   0  4  = 11 |    Space                                       $fnd_cmnd
   0  4  = 12 |    Inquiry                                     $fnd_cmnd
   0  4  = 13 |    Verify                                      $fnd_cmnd
   0  4  = 14 |    Recover Buffered data                       $fnd_cmnd
   0  4  = 15 |    Mode select                                 $fnd_cmnd
   0  4  = 16 |    Reserve                                     $fnd_cmnd
   0  4  = 17 |    Release                                     $fnd_cmnd
   0  4  = 18 |    Copy                                        $fnd_cmnd
   0  4  = 19 |    Erase                                       $fnd_cmnd
   0  4  = 1a |    Mode sense                                  $fnd_cmnd
   0  4  = 1b |    Load/unload                                 $fnd_cmnd
   0  4  = 1c |    Receive Diagnostics Results                 $fnd_cmnd
   0  4  = 1d |    Send diagnostic                             $fnd_cmnd
   0  4  = 1e |    Prevent/Allow Media Removal                 $fnd_cmnd
   0  4  = 1f |    Read Log                                    $fnd_cmnd
   0  4  = 34 |    Read position                               $fnd_cmnd
   0  4  = 3b |    Locate                                      $fnd_cmnd
   ~if (! fnd_cmnd)
              %    Warning SCSI Cmnd is supposed to be a tape drive cmnd
              %    but the command is not recognized as a tape drive command.
   ~endif
 ~elseif is_scsi_cd
   * Is a cdrom, look for command in set of cdrom commands
   0  4  = 00 |    Test Unit Ready                             $fnd_cmnd
   0  4  = 01 |    Rezero Unit                                 $fnd_cmnd
   0  4  = 03 |    Request Sense Data   $request_sense_cmnd    $fnd_cmnd
   0  4  = 08 |    Read                                        $fnd_cmnd
   0  4  = 0B |    Seek                                        $fnd_cmnd
   0  4  = 12 |    Inquiry                                     $fnd_cmnd
   0  4  = 15 |    Mode select                                 $fnd_cmnd
   0  4  = 16 |    Reserve                                     $fnd_cmnd
   0  4  = 17 |    Release                                     $fnd_cmnd
   0  4  = 1a |    Mode sense                                  $fnd_cmnd
   0  4  = 1b |    Start/stop unit                             $fnd_cmnd
   0  4  = 1d |    Send diagnostic                             $fnd_cmnd
   0  4  = 1e |    Prevent/Allow Media Removal                 $fnd_cmnd
   0  4  = 25 |    Read capacity                               $fnd_cmnd
   0  4  = 28 |    Read extended                               $fnd_cmnd
   0  4  = c0 |    Audio track search                          $fnd_cmnd
   0  4  = c1 |    Play audio                                  $fnd_cmnd
   0  4  = c2 |    Still (halts the playing of audio)          $fnd_cmnd
   0  4  = c3 |    Set stop time                               $fnd_cmnd
   0  4  = c4 |    Eject disk caddy                            $fnd_cmnd
   0  4  = c6 |    Read subcode Q data & Playing Status        $fnd_cmnd
   0  4  = c7 |    Read Disc information                       $fnd_cmnd
   0  4  = c8 |    Read CD-ROM Mode                            $fnd_cmnd
 ~elseif is_scsi_rwcd
   * Is a rewriteable cdrom, look for command in set of cdrom commands
   0  4  = 00 |    Test Unit Ready                             $fnd_cmnd
   0  4  = 01 |    Rezero Unit                                 $fnd_cmnd
   0  4  = 03 |    Request Sense Data   $request_sense_cmnd    $fnd_cmnd
   0  4  = 04 |    Format Unit                                 $fnd_cmnd
   0  4  = 07 |    Reassign Blocks                             $fnd_cmnd
   0  4  = 08 |    Read                                        $fnd_cmnd
   0  4  = 0A |    Write                                       $fnd_cmnd
   0  4  = 0B |    Seek                                        $fnd_cmnd
   0  4  = 12 |    Inquiry                                     $fnd_cmnd
   0  4  = 15 |    Mode select                                 $fnd_cmnd
   0  4  = 16 |    Reserve                                     $fnd_cmnd
   0  4  = 17 |    Release                                     $fnd_cmnd
   0  4  = 1a |    Mode sense                                  $fnd_cmnd
   0  4  = 1b |    Start/stop unit                             $fnd_cmnd
   0  4  = 1d |    Send diagnostic                             $fnd_cmnd
   0  4  = 1e |    Prevent/Allow Media Removal                 $fnd_cmnd
   0  4  = 25 |    Read capacity                               $fnd_cmnd
   0  4  = 28 |    Read extended                               $fnd_cmnd
   0  4  = 2A |    Write extended                              $fnd_cmnd
   0  4  = 2B |    Seek extended                               $fnd_cmnd
   0  4  = 2C |    Erase                                       $fnd_cmnd
   0  4  = 2E |    Write and Verify                            $fnd_cmnd
   0  4  = 2F |    Verify                                      $fnd_cmnd
   0  4  = 37 |    Read Defect Data                            $fnd_cmnd
   0  4  = 3B |    Write Buffer                                $fnd_cmnd
   0  4  = 3C |    Read Buffer                                 $fnd_cmnd
   0  4  = 3E |    Read Long                                   $fnd_cmnd
   0  4  = 3F |    Write Long                                  $fnd_cmnd
   0  4  = A8 |    Read (Extended)                             $fnd_cmnd
   0  4  = AA |    Write (Extended)                            $fnd_cmnd
   0  4  = AC |    Erase (Extended)                            $fnd_cmnd
   0  4  = AE |    Write and Verify (Extended)                 $fnd_cmnd
   0  4  = AF |    Verify (Extended)                           $fnd_cmnd
   0  4  = D8 |    Restore Unit                                $fnd_cmnd

   ~if (! fnd_cmnd)
              %    Warning SCSI Cmnd is supposed to be a CD-ROM cmnd
              %    but the command is not recognized as a CD-ROM command.
   ~endif
 ~endif
 ~if (! fnd_cmnd)
 * Try basic SCSI spec definition of the command
   0  4  = 00  |    Test Unit Ready
   0  4  = 01  |    Rewind or Rezero unit
   0  4  = 03  |    Request Sense Data   $request_sense_cmnd
   0  4  = 04  |    Format or Format unit
   0  4  = 05  |    Read block limits
   0  4  = 07  |    Reassign blocks or Initialize Element status
   0  4  = 08  |    Read  or get message or receive
   0  4  = 0A  |    Write or send or Send Message or write
   0  4  = 0B  |    Seek or slew and print
   0  4  = 0D  |    Extended Diagnostics
   0  4  = 0F  |    Read reverse
   0  4  = 10  |    Write Filemark or Synchronize buffer
   0  4  = 11  |    Space
   0  4  = 12  |    Inquiry
   0  4  = 13  |    Verify Tape or Verify
   0  4  = 14  |    Recover buffered data
   0  4  = 15  |    Mode select
   0  4  = 16  |    Reserve or Reserve unit
   0  4  = 17  |    Release or Release unit
   0  4  = 18  |    Copy
   0  4  = 19  |    Erase
   0  4  = 1a  |    Mode sense
   0  4  = 1b  |    Load/unload or scan or stop print or stop/start unit
   0  4  = 1c  |    Receive diagnostics results
   0  4  = 1d  |    Send diagnostic
   0  4  = 1e  |    Prevent/allow medium removal
   0  4  = 24  |    Define window parameters
   0  4  = 25  |    Read capacity or Get window parameters
   0  4  = 28  |    Read extended or get message or read
   0  4  = 2a  |    Write extended or Send or send message or write
   0  4  = 2b  |    Seek extended or locate or position to element
   0  4  = 2c  |    Erase
   0  4  = 2d  |    Read updated block
   0  4  = 2e  |    Write and verify
   0  4  = 2f  |    Verify
   0  4  = 30  |    Search data high
   0  4  = 31  |    Medium position or search data equal
   0  4  = 32  |    Search data low
   0  4  = 33  |    Set limits
   0  4  = 34  |    Get data status or pre-fetch or read position
   0  4  = 35  |    Synchronize cache
   0  4  = 36  |    Lock/unlock cache
   0  4  = 37  |    Read defect list
   0  4  = 38  |    Medium scan
   0  4  = 39  |    Compare
   0  4  = 3A  |    Copy and verify
   0  4  = 3B  |    Write buffer
   0  4  = 3C  |    Read buffer
   0  4  = 3D  |    Update block
   0  4  = 3E  |    Read long
   0  4  = 3F  |    Write long
   0  4  = 40  |    Change definition
   0  4  = 41  |    Write same
   0  4  = 42  |    Read sub-channel
   0  4  = 43  |    Read TOC
   0  4  = 44  |    Read header
   0  4  = 45  |    Play audio
   0  4  = 47  |    Play audio MSF
   0  4  = 48  |    Play audio track/index
   0  4  = 49  |    Play track relative
   0  4  = 4B  |    Pause/resume
   0  4  = 4C  |    Log select
   0  4  = 4D  |    Log sense
   0  4  = 55  |    Mode select
   0  4  = 5A  |    Mode sense
   0  4  = A5  |    Move medium or play audio
   0  4  = A6  |    Exchange medium
   0  4  = A8  |    Read
   0  4  = A9  |    Play track relative
   0  4  = AA  |    Send message or write
   0  4  = AC  |    Erase
   0  4  = AE  |    Write and verify
   0  4  = AF  |    Verify
   0  4  = B0  |    Search Data High
   0  4  = B1  |    Search Data Equal
   0  4  = B2  |    Search Data Low
   0  4  = B3  |    Set limits
   0  4  = B5  |    Request volume element address
   0  4  = B6  |    Send volume tag
   0  4  = B7  |    Read defect data
   0  4  = B8  |    Read element status
   0  4  = E8  |    Read long  (IBM hardfiles)
   0  4  = Ea  |    Write long (IBM hardfiles)
 ~endif
  * Compute Display Logic Unit Number
  * LUN comprise bits 7-5 of byte below, the rest of the byte is cmd depend.
  0  5  @   @lun
  lun := lun >> 5
               %    LUN = %d ~lun
~else
 * No valid SCSI command
             % 2) SCSI Command Length 0, can't decode command.
~endif

* Determine whether status is from SCSI bus or from Card
0 16  @      | 4) Status validity (0x%.2x)
0 16  ^ 01   |    -- Have SCSI Bus Status $scsi_bus
0 16  ^ 02   |    -- Have SCSI Adapter Status  $adpt_status
~if ((! scsi_bus) && (! adpt_status))
             %    -- No status logged for SCSI bus or adapter
~endif

~if scsi_bus
  * Display SCSI bus status using words that make sense for the device
  ~if (is_serdasd || is_scsi_dasd || is_scsi_array)
     0 17 @     | 5) SCSI Bus Status (0x%.2x)
     0 17 = 00  |    Good status.
                |    --Drive operation completed successfully
     0 17 = 02  ||    Check condition
                |    --Drive is reporting an error, exception or abnormal cond.
     0 17 = 08  ||    Busy Status
                |    --Drive is busy and can't accept a cmnd from initiator
     0 17 = 10  ||    Intermediate good status
                |    --Intermediate status when used with link cmds
     0 17 = 18  ||    Reservation conflict

  ~elseif (is_scsi_tape)

     0 17 @     | 5) SCSI Bus Status (0x%.2x)
     0 17 = 00  |    Good status.
                |    --Tape device operation completed successfully
     0 17 = 02  ||    Check condition
                |    --Tape drive is reporting an err, exception or abnormal\
                | condition
     0 17 = 08  ||    Busy Status
                |    --Tape drive is busy and can't accept cmnd from  \
                | initiator
     0 17 = 10  ||    Intermediate good status
                |    --Intermediate good status when used with link cmds
     0 17 = 18  ||    Reservation conflict
  ~else
     0 17 @     | 5) SCSI Bus Status (0x%.2x)
     0 17 = 00  |    Good status.
                |    --Target completed successfully
     0 17 = 02  ||    Check condition
                |    --Target is reporting an error, exception or abnormal cond.
     0 17 = 08  ||    Busy Status
                |    --Target is busy and can't accept a cmnd from initiator
     0 17 = 10  ||    Intermediate good status
                |    --Intermediate good status when used with link cmds
     0 17 = 18  ||    Reservation conflict
  ~endif
~endif

~if adpt_status
  * Display adapter status
  0 18 @     | 5) SCSI Adapter Card Status (0x%.2x)
  0 18 ^ 01  ||    Host I/O Bus error
             |    --Indicates a Host I/O bus error condition
  0 18 ^ 02  ||    SCSI Bus fault
             |    --Indicates a failure of the SCSI bus
  0 18 ^ 04  ||    Command timeout
             |    --Cmnd did not complete before timeout value expired
  0 18 ^ 08  ||    No device response
             |    --Device did not respond to adapter's attempts to address it
  0 18 ^ 10  ||    Adapter hardware failure
             |    --The adapter is indicating a hardware failure
  0 18 ^ 20  ||    Adapter software failure
             |    --The adapter is indicating a microcode failure
  0 18 ^ 40  ||    Fuse or terminal power
             |    --Adapter is indicating a blown fuse or bad termination
  0 18 ^ 80  ||   SCSI Bus reset
             |    --The adapter detected an external SCSI bus reset
~endif

* (0,20 - 1,09) Are reserved for Raw Request Sense data
* (1,10 - ...)  Are reserved for SCSI Mailbox Information

! scsi_cmnd
 0 12  @     |    Command (in hex %.2x \
 0 13 @      | %.2x \
 0 14 @      | %.2x \
 0 15 @      | %.2x \
 0 16 @      | %.2x \
 0 17 @      | %.2x \
 0 18 @      | %.2x \
 0 19 @      | %.2x \
 0 20 @      | %.2x \
 0 21 @      | %.2x \
 0 22 @      | %.2x \
 0 23 @      | %.2x )
 0 12  = 00  |    Test Unit Ready
 0 12  = 01  |    Rewind or Rezero unit
 0 12  = 03  |    Request Sense Data   $request_sense_cmnd
 0 12  = 04  |    Format or Format unit
 0 12  = 05  |    Read block limits
 0 12  = 07  |    Reassign blocks or Initialize Element status
 0 12  = 08  |    Read  or get message or receive
 0 12  = 0A  |    Write or send or Send Message or write
 0 12  = 0B  |    Seek or slew and print
 0 12  = 0D  |    Extended Diagnostics
 0 12  = 0F  |    Read reverse
 0 12  = 10  |    Write Filemark or Synchronize buffer
 0 12  = 11  |    Space
 0 12  = 12  |    Inquiry
 0 12  = 13  |    Verify Tape or Verify
 0 12  = 14  |    Recover buffered data
 0 12  = 15  |    Mode select
 0 12  = 16  |    Reserve or Reserve unit
 0 12  = 17  |    Release or Release unit
 0 12  = 18  |    Copy
 0 12  = 19  |    Erase
 0 12  = 1a  |    Mode sense
 0 12  = 1b  |    Load/unload or scan or stop print or stop/start unit
 0 12  = 1c  |    Receive diagnostics results
 0 12  = 1d  |    Send diagnostic
 0 12  = 1e  |    Prevent/allow medium removal
 0 12  = 24  |    Define window parameters
 0 12  = 25  |    Read capacity or Get window parameters
 0 12  = 28  |    Read extended or get message or read
 0 12  = 2a  |    Write extended or Send or send message or write
 0 12  = 2b  |    Seek extended or locate or position to element
 0 12  = 2c  |    Erase
 0 12  = 2d  |    Read updated block
 0 12  = 2e  |    Write and verify
 0 12  = 2f  |    Verify
 0 12  = 30  |    Search data high
 0 12  = 31  |    Medium position or search data equal
 0 12  = 32  |    Search data low
 0 12  = 33  |    Set limits
 0 12  = 34  |    Get data status or pre-fetch or read position
 0 12  = 35  |    Synchronize cache
 0 12  = 36  |    Lock/unlock cache
 0 12  = 37  |    Read defect list
 0 12  = 38  |    Medium scan
 0 12  = 39  |    Compare
 0 12  = 3A  |    Copy and verify
 0 12  = 3B  |    Write buffer
 0 12  = 3C  |    Read buffer
 0 12  = 3D  |    Update block
 0 12  = 3E  |    Read long
 0 12  = 3F  |    Write long
 0 12  = 40  |    Change definition
 0 12  = 41  |    Write same
 0 12  = 42  |    Read sub-channel
 0 12  = 43  |    Read TOC
 0 12  = 44  |    Read header
 0 12  = 45  |    Play audio
 0 12  = 47  |    Play audio MSF
 0 12  = 48  |    Play audio track/index
 0 12  = 49  |    Play track relative
 0 12  = 4B  |    Pause/resume
 0 12  = 4C  |    Log select
 0 12  = 4D  |    Log sense
 0 12  = 55  |    Mode select
 0 12  = 5A  |    Mode sense
 0 12  = A5  |    Move medium or play audio
 0 12  = A6  |    Exchange medium
 0 12  = A8  |    Read
 0 12  = A9  |    Play track relative
 0 12  = AA  |    Send message or write
 0 12  = AC  |    Erase
 0 12  = AE  |    Write and verify
 0 12  = AF  |    Verify
 0 12  = B0  |    Search Data High
 0 12  = B1  |    Search Data Equal
 0 12  = B2  |    Search Data Low
 0 12  = B3  |    Set limits
 0 12  = B5  |    Request volume element address
 0 12  = B6  |    Send volume tag
 0 12  = B7  |    Read defect data
 0 12  = B8  |    Read element status
 0 12  = E8  |    Read long  (IBM hardfiles)
 0 12  = Ea  |    Write long (IBM hardfiles)
 0 13  @     |    LUN = %.2x


!satsuma
* General Sense Decode for Satsuma family of drives.
#define is_scsi_dasd
#define servo_err
#define retryable
#define sense_progress
#define nonrecoverable
#define illegal_request
#set tms_status 0

* Include general SCSI information
#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   $lba_valid

0 20  = 70   |    Error for the current command
0 20  = f0   |    Error for the current command
0 20  = 71   |    Deferred error
0 20  = f1   |    Deferred error

0 22  @      |    a) Sense Key (0x%.4x)  @sense_key
0 22  = 00   |        No Sense
             |       --There is no sense key info to be reported for the unit
             |         This code occurs for a successfully completed command
0 22  = 01   ||       Recovered error  $retryable
             |       --The last cmnd completed successfully with recovery
             |         action performed by the target
0 22  = 02   ||       Not Ready   ^sense_progress
             |       --The logical unit can not be addressed
0 22  = 03   ||       Medium Error     $retryable
             |       --The command terminated with a non-recoverable
             |         error condition caused by a flaw in the media or
             |         an error in the recorded data
0 22  = 04   ||       Hardware Error   $retryable $nonrecovered
             |       --The target detected a non-recoverable hardware error
             |         while performing a command or during a diagnostic test
0 22  = 05   ||       Illegal Request  $illegal_request
             |       --There was an illegal parameter in the Command
             |         Descriptor Block or additional parameter supplied as data
0 22  = 06   ||       Unit Attention
             |       --Reported after an attention causing event
0 22  = 0b   ||       Aborted Command
             |       --Target aborted the command

0 23 @ 32    |    b) Logic Block Address = 0x%.8x  or %d decimal @lba ~lba lba

  ~if lba_valid
             %       LBA is valid
  ~else
             %       NOTE LBA is not precisely valid for the error
  ~endif

0 27 @       |    c) Additional Sense Length = %d
1  0 @ 16    |    d) Additional Sense Code and Sense Code Qualifier (0x%.4x)

~if (sense_key == 1)
  1 0 = 0100 ||        write error no index
  1 0 = 0300 ||        write error - write fault
  1 0 = 1401 ||        write error Id not found
  1 0 = 1600 ||        write error DAM not found
  1 0 = 1700 ||        read error without ECC applied
  1 0 = 1701 ||        read error with retries
  1 0 = 1706 ||        read error without ECC applied. Auto reallocated
  1 0 = 1707 ||       read error without ECC applied. recommend reassign
  1 0 = 1709 ||        read error without ECC applied. data re-written
  1 0 = 1800 ||        read error with ECC applied
  1 0 = 1802 ||        read error with ECC applied. auto reallocated
  1 0 = 1805 ||        read error with ECC applied. recommend reassign
  1 0 = 1807 ||        read error with ECC applied. data re-written
  1 0 = 1c01 ||        pimary defect list not found
  1 0 = 1c02 ||        grown defect list not found
  1 0 = 4400 ||        internal target failure
~elseif (sense_key == 2)
  1 0 = 0400 ||        Start spindle Motor fail
  1 0 = 0401 ||        in process of becoming ready
  1 0 = 0402 ||        initializing command required (start unit)
  1 0 = 0404 ||        format in progress
  1 0 = 3100 ||        media format corrupt
  1 0 = 4080 ||        diag fail - bring-up fail
  1 0 = 4085 ||        diag fail - RAM microcode not loaded
  1 0 = 4c00 ||       degraded mode - self config fail, ucode not loaded
~elseif (sense_key == 3)
  1 0 = 1000 ||        ID CRC error
  1 0 = 1100 ||        Unrecovered read error
  1 0 = 1401 ||        record not found
  1 0 = 1600 ||        data synchroniztion mark error (DAM error)
  1 0 = 1900 ||        defect list error
  1 0 = 3101 ||        medium format corrupted, reassign failed
~elseif (sense_key == 4)
  1 0 = 0100 ||        no index or sector
  1 0 = 0200 ||        no seek complete
  1 0 = 0300 ||        write fault
  1 0 = 0900 ||        track following error
  1 0 = 1100 ||        unrecovered read error in reserved area
  1 0 = 3100 ||        format corrupted
  1 0 = 3200 ||        no defect spare location available
  1 0 = 4080 ||        degrade mode: diag fail, config sector valid fail
  1 0 = 4081 ||        degrade mode: HDC error
  1 0 = 4082 ||        degrade mode: HIC error
  1 0 = 4083 ||        degrade mode: other LSI error
  1 0 = 4084 ||        degrade mode: RAM error
  1 0 = 4085 ||        degrade mode: RAM microcode not loaded
  1 0 = 4400 ||        Internal target failure
  1 0 = 4700 ||        SCSI parity error
~elseif (sense_key == 5)
  1 0 = 1a00 ||        parameter list length error
  1 0 = 2000 ||        illegal command operation code
  1 0 = 2100 ||        logical block address out of range
  1 0 = 2400 ||        invalid field in CDB
  1 0 = 2600 ||        invalid fields in the parameter list
~elseif (sense_key == 6)
  1 0 = 2800 ||        not ready to ready transition
  1 0 = 2900 ||        power on reset or bus device reset occured
  1 0 = 2a01 ||        mode select parameter changed
  1 0 = 2f00 ||        command cleared by another initiator
  1 0 = 3f01 ||        micro code has been changed
~elseif (sense_key == 0xb)
  1 0 = 1b00 ||        synchronous data transfer error
  1 0 = 2500 ||        Unsupported LUN
  1 0 = 4300 ||        Message reject error
  1 0 = 4500 ||        Select/Reselect failure
  1 0 = 4700 ||        SCSI Parity Error
  1 0 = 4800 ||        Initiator detected error message
  1 0 = 4900 ||        Invalid message error
  1 0 = 4e00 ||        Overlapped commands attempted
~endif

  1 3 @ 24   |    f) Sense Key Specific Information (0x%.6x)

  1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)

  ~if retryable
       1 4 @ 16    |       Retry count for retryable error = %d (decimal)

  ~elseif (sense_progress)
       1 4 @ 16    |       Fraction of cmd in progress complete = %d (decimal)
             %             (Amount completed / 65535)

  ~elseif (illegal_request)
       1 4 !^ 40    |       illegal parameter is in the data parameters sent
                    |       by the initiator during DATA OUT phase
       1 4 ^ 40     |       illegal parameter is in the command descrip block
       1 4 ^ 08    ^valid_bpv
       ~if (valid_bpv)
           1 4 ^ 07  @bpv_value
           % Bit Pointer: %d ~bpv_value
       ~endif
       1 4 @ 16    |       Field pointer is 0x%.4x

  ~endif
~endif

1  8  @  16     | 7) Unit Error Code (0x%.4x)
                %    UEC information not available at this time.


!scsi_hf
* General Sense Decode for SCSI Attached Hardfiles-- Should be called
* by another sense file that sets one of the following flags being defined
* below
#define is_320mb
#define is_355mb
#define is_400mb
#define is_670mb
#define is_857mb
#define is_oemhf
#define servo_err
#define retryable
#define sense_progress
#define nonrecoverable
#define illegal_request
#set tms_status 0

* Include general SCSI information
#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   $lba_valid

0 20  = 70   |    Error for the current command
0 20  = f0   |    Error for the current command
0 20  = 71   |    Deferred error
0 20  = f1   |    Deferred error

0 22  @      |    a) Sense Key (0x%.4x)  @sense_key
0 22  = 00   |        No Sense
             |       --There is no sense key info to be reported for the unit
             |         This code occurs for a successfully completed command
0 22  = 01   ||       Recovered error  $retryable
             |       --The last cmnd completed successfully with recovery
             |         action performed by the target
0 22  = 02   ||       Not Ready   ^sense_progress
             |       --The logical unit can not be addressed
0 22  = 03   ||       Medium Error     $retryable
             |       --The command terminated with a non-recoverable
             |         error condition caused by a flaw in the media or
             |         an error in the recorded data
0 22  = 04   ||       Hardware Error   $retryable $nonrecovered
             |       --The target detected a non-recoverable hardware error
             |         while performing a command or during a diagnostic test
0 22  = 05   ||       Illegal Request  $illegal_request
             |       --There was an illegal parameter in the Command
             |         Descriptor Block or additional parameter supplied as data
0 22  = 06   ||       Unit Attention
             |       --Reported after an attention causing event
0 22  = 0b   ||       Aborted Command
             |       --Target aborted the command

~if is_355mb || is_670mb

  0 22  = 07   ||       Data Protect
               |       --Write attempted on a write protected media
  0 22  = 0e   ||       Miscompare
               |       --Used by Verify or Read Data Buffer to indicate that
               |         the source data did not match data read from disk
~endif

* ~if ( is_320mb || is_400mb || is_857mb)
   0 23 @ 32 |    b) Logic Block Address = 0x%.8x  or %d decimal @lba ~lba lba
   ~if lba_valid
             %       LBA is valid
   ~else
             %       NOTE LBA is not precisely valid for the error
   ~endif

* ~endif

0 27 @       |    c) Additional Sense Length = %d

~if ( is_320mb || is_400mb || is_857mb)
             %    d) Last Logical Block not reassigned if reassign failed
  0 28 @ 32  |       = 0x%.8x
             |       --This value is the first Logical Block Address (LBA)
             |         from the defect descriptor block that was not reassigned
~endif
1  0 @ 16    |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

#set as_fnd 0
~if (is_355mb || is_670mb)
  * Unique to these drives
  1 0 = 0500 ||        Drive not selected                        $as_fnd
  1 0 = 0600 ||        No track 0                                $as_fnd
  1 0 = 0800 ||        LUN communication error                   $as_fnd
  1 0 = 0900 ||        Track following error                     $as_fnd
  1 0 = 1000 ||        ID Field CRC error                        $as_fnd
  1 0 = 1200 ||        ID Field Address Mark not found           $as_fnd
  1 0 = 1300 ||        Data Address Mark not found               $as_fnd
  1 0 = 1400 ||        Record not found                          $as_fnd
  1 0 = 1800 ||        Data recovered with ECC and retries       $as_fnd
  1 0 = 1c00 ||        Primary defect list not found             $as_fnd
  1 0 = 1d00 ||        Compare error                             $as_fnd
  1 0 = 2200 ||        Illegal function for device type          $as_fnd
  1 0 = 2700 ||        Write protected                           $as_fnd
  1 0 = 3900 ||        No saved values                           $as_fnd
  1 0 = 3e00 ||        LUN has not self-configured yet           $as_fnd
  1 0 = 4000 ||        Diagnostic failure                        $as_fnd
  1 0 = 4400 ||        Internal target failure                   $as_fnd
  1 0 = 4500 ||        Select/reselect failure                   $as_fnd
  1 0 = 4700 ||        SCSI parity error.                        $as_fnd
  1 0 = 4800 ||        Initiator detected error message received $as_fnd
  1 0 = 4900 ||        Invalid message error                     $as_fnd

~elseif (is_320mb || is_400mb || is_857mb)
* In common to 320mb, 400mb, and 857mb

  1 0 = 0000 |        No additional sense information                    $as_fnd
  1 0 = 0200 ||        No seek complete                                  $as_fnd
  1 0 = 0400 ||        Not ready, cause not reportable                   $as_fnd
  1 0 = 0401 ||        Not ready, drive is powering on                   $as_fnd
  1 0 = 0402 ||        Not ready, Start Unit command required            $as_fnd
  1 0 = 0404 ||        Not ready, Format in Progress                     $as_fnd
  1 0 = 0900 ||        Track following error                             $as_fnd
  1 0 = 1100 ||        Unrecovered read error                            $as_fnd
  1 0 = 1401 ||        Record not found                                  $as_fnd
  1 0 = 1500 ||        Random positioning error (Seek Error)             $as_fnd
  1 0 = 1502 ||        Positioning error detected by incorrect ID        $as_fnd
  1 0 = 1600 ||        Data sync error                                   $as_fnd
  1 0 = 1700 ||        Data recovered without ECC                        $as_fnd
  1 0 = 1801 ||        Data recovered with ECC and retries               $as_fnd
  1 0 = 1900 ||        Defect list error                                 $as_fnd
  1 0 = 1901 ||        Grown defect list not available                   $as_fnd
  1 0 = 1a00 ||        Parameter list length error                       $as_fnd
  1 0 = 2000 ||        Invalid command                                   $as_fnd
  1 0 = 2100 ||        Logical Block Address out of range                $as_fnd
  1 0 = 2400 ||        Invalid field in Command descriptor block         $as_fnd
  1 0 = 2500 ||        Unsupported Logical Unit Number                   $as_fnd
  1 0 = 2600 ||        Invalid field in parameter list                   $as_fnd
  1 0 = 2900 ||        Power on, Reset, or Bus Device Reset occurred     $as_fnd
  1 0 = 2A00 ||        Mode Select parms changed by another initiator    $as_fnd
  1 0 = 3100 ||        Medium corrupted - Reassign Failed or Interrupted $as_fnd
  1 0 = 3101 ||        Medium corrupted - Format Failed or Interrupted   $as_fnd
  1 0 = 3200 ||        No defect spare location available                $as_fnd
  1 0 = 3d00 ||        Invalid Identity message                          $as_fnd
  1 0 = 3f00 ||        Target Operating Conditions have changed          $as_fnd
  1 0 = 3f01 ||        Microcode has been changed                        $as_fnd
  1 0 = 4080 ||        Diagnostic failure                                $as_fnd
  1 0 = 4090 ||        Diagnostic failure - servo fault                  $as_fnd
  1 0 = 40a0 ||        Diagnostic failure - Read/write fault             $as_fnd
  1 0 = 40b0 ||        Self initiated reset for internal fault           $as_fnd
  1 0 = 40c0 ||        RAM/ROS mis-match                                 $as_fnd
  1 0 = 40d0 ||        Card/DE mis-match                                 $as_fnd
  1 0 = 4300 ||        Message reject error                              $as_fnd
  1 0 = 4400 ||        Internal target failure                           $as_fnd
  1 0 = 4500 ||        Select/Reselect failure                           $as_fnd
  1 0 = 4700 ||        SCSI Parity Error                                 $as_fnd
  1 0 = 4800 ||        Initiator detected error message                  $as_fnd
  1 0 = 4900 ||        Invalid message error                             $as_fnd
  1 0 = 4c00 ||        LUN failed self configuration                     $as_fnd
  1 0 = 4e00 ||        Overlapped commands attempted                     $as_fnd
~endif

~if (! as_fnd)
  * Will try the SCSI II Bus standard
  1 0 = 0000 |        No additional sense information
  1 0 = 0200 ||        No seek complete
  1 0 = 0300 ||        Write fault
  1 0 = 0302 ||        Excessive write errors
  1 0 = 0400 ||        Not ready, cause not reportable
  1 0 = 0401 ||        Not ready, drive is powering on
  1 0 = 0402 ||        Not ready, Start Unit command required
  1 0 = 0404 ||        Not ready, Format in Progress
  1 0 = 1000 ||        ID CRC or ECC error
  1 0 = 1100 ||        Unrecovered read error
  1 0 = 1102 ||        Error too long to correct
  1 0 = 1200 ||        ID Field Address Mark not found
  1 0 = 1300 ||        Data Address Mark not found
  1 0 = 1500 ||        Random positioning error (Seek Error)
  1 0 = 1502 ||        Positioning error detected by incorrect ID
  1 0 = 1600 ||        Data synchronization mark error
  1 0 = 1700 ||        Data recovered without ECC
  1 0 = 1705 ||        Data recovered using previous sector ID
  1 0 = 1801 ||        Data recovered with ECC and retries
  1 0 = 1900 ||        Defect list error
  1 0 = 1901 ||        Grown defect list not available
  1 0 = 1902 ||        Defect list error in primary list
  1 0 = 1903 ||        Defect list error in grown list
  1 0 = 1a00 ||        Parameter list length error
  1 0 = 1b00 ||        Synchronous transfer error
  1 0 = 1c00 ||        Defect list not found
  1 0 = 1c02 ||        Grown defect list not found
  1 0 = 2000 ||        Invalid command
  1 0 = 2100 ||        Logical Block Address out of range
  1 0 = 2200 ||        Illegal function for device type
  1 0 = 2400 ||        Invalid field in Command descriptor block
  1 0 = 2500 ||        Unsupported Logical Unit Number
  1 0 = 2600 ||        Invalid field in parameter list
  1 0 = 2800 ||        Not ready to ready transition
  1 0 = 2900 ||        Power on, Reset, or Bus Device Reset occurred
  1 0 = 2A00 ||        Mode Select parameters changed by another initiator
  1 0 = 2c00 ||        Command sequence error
  1 0 = 3000 ||        Incompatible medium installed
  1 0 = 3001 ||        Can not read medium -- Unknown format
  1 0 = 3002 ||        Can not read medium -- Incompatible format
  1 0 = 3100 ||        Medium format corrupted
  1 0 = 3101 ||        Format Command failed
  1 0 = 3200 ||        No defect spare location available
  1 0 = 3201 ||        Defect list update failure
  1 0 = 4100 ||        Data Path failure
  1 0 = 4a00 ||        Command phase error
  1 0 = 4b00 ||        Data Phase error
~endif

  1 3 @ 24   |    f) Sense Key Specific Information (0x%.6x)

  1 3 ^ 80    ^SKSV

* If Sense Key Specific Valid
~if (SKSV)
  ~if retryable
       1 4 @ 16    |       Retry count for retryable error = %d (decimal)

  ~elseif (sense_progress)
       1 4 @ 16    |       Fraction of cmd in progress complete = %d (decimal)
             %             (Amount completed / 65535)

  ~elseif ( (illegal_request) && (is_355mb || is_670mb) )
       1 4 @ 16    |       Field pointer is 0x%.4x
  ~endif
~endif

%
* UEC codes common to 320mb, 400mb, and 857mb
~if (is_320mb || is_400mb || is_857mb)

  1  8  @  16     | 7) Unit Error Code (0x%.4x)
  1  8  =  0000   |    No Error ^invalid_uec
  1  8  =  0101   ||    Degraded Mode/Motor not running
                  |    -- Cmnds cannot be accepted with this degradation
  1  8  =  0102   ||    Unavailable while start motor active
  1  8  =  0103   ||    Unavailable while bringup active
  1  8  =  0104   ||    Unavailable while format active
  1  8  =  0111   ||    Degraded Mode/Previous Reassign Block unsucessful
                  |    --Command can not be accepted
  1  8  =  0112   ||    Degraded Mode/Previous Format unsuccessful
                  |    --Command can not be accepted
  1  8  =  0113   ||    Degraded Mode/Configuration not loaded
                  |    --Command can not be accepted
  1  8  =  0114   ||    Degraded Mode/RAM Microcode not loaded
                  |    --Command can not be accepted
  1  8  =  0115   ||    Error in grown defect list
  1  8  =  0116   ||    No spare sectors remaining
  1  8  =  0117   ||    Error in primary defect list
  1  8  =  0120   ||    Microcode checksum err
                  |    --detected after loading into control storage
                  |      RAM or after write buf download
  1  8  =  0121   ||    RAM code installation error
  1  8  =  0122   ||    Degraded Mode/bringup not successful
                  |    --Command can not be accepted
  1  8  =  0123   ||    Microcode err detected trying to start SCSI data xfer
  1  8  =  0124   ||    SCSI interrupt invalid
  1  8  =  0126   ||    Dynamic data RAM error
  1  8  =  0127   ||    60C40 Channel A parity error during transfer in
  1  8  =  0128   ||    60C40 Channel A parity error during transfer out
  1  8  =  0129   ||    60C40 Channel A Programmed I/O parity error
  1  8  =  012A   ||    60C40 Channel A unexpected error
  1  8  =  012C   ||    33C93A (SBIC) internal parity error
  1  8  =  012D   ||    Can not resume operation (data transfer)
  1  8  =  012e   ||    Load IDs do not match on bringup
  1  8  =  0130   ||    Invalid op code
  1  8  =  0131   ||    Invalid Logical Block Address (LBA)
  1  8  =  0132   ||    Command Descriptor Block Invalid (CDB)
  1  8  =  0133   ||    Invalid Logical Unit Number (LUN)
  1  8  =  0134   ||    Command parameter data invalid
  1  8  =  0135   ||    Command parameter list length error
  1  8  =  0136   ||    ROS and RAM load IDs do not match
  1  8  =  0140   ||    Unit Attention/Format Unit.
  1  8  =  0141   ||    Unit Attention/POR
  1  8  =  0142   ||    Unit Attention/Mode Select
  1  8  =  0143   ||    Unit Attention/Write Buffer
  1  8  =  0145   ||    Unit Attention/Self Initiated Reset
  1  8  =  0160   ||    Initiator detected err other than Status or linked
                  |       COMMAND COMPLETE phase.
  1  8  =  0161   ||    Abnormal termination; unrecovered SCSI parity err
                  |       detected by target during cmd or data phase
  1  8  =  0162   ||    Overlapped command
  1  8  =  0171   ||    Abnormal termination; different LUN addressed
                  |       (Identity message) from first selected
  1  8  =  0172   ||    Abnormal termination; Message Reject message
  1  8  =  0173   ||    Abnormal termination; Reselection timeout
  1  8  =  0174   ||    Abnormal termination; Unrecovered SCSI parity error
                  |        detected by target during a MESSAGE OUT phase
  1  8  =  0175   ||    Abnormal termination; Initiator Detected Error
                  |        msg for Status or linked COMMAND COMPLETE phase
  1  8  =  0176   ||    Abnormal termination; Invalid msg or attention dropped
                  |        before all bytes of an extended msg are xfered
  1  8  =  0177   ||    Abnormal termination; Attention dropped too late
  1  8  =  0178   ||    Abnormal termination; Msg parity err received
                  |        when no message sent to target
  1  8  =  0179   ||    Abnormal termination; Rsvd bits in Identity msg !=0
  1  8  =  017A   ||    Abnormal termination; Unrecovered SCSI parity err
                  |        detected by initiator (Message parity err msg)
  1  8  =  0180   ||    Sector found in both SAT map and Grown defect lst
  1  8  =  0181   ||    Internal software error
  1  8  =  0182   ||    Format Track parm err (# of sectors and ids dont match)
  1  8  =  0183   ||    Seek positioning error (ID miscompare)
  1  8  =  0184   ||    A previous servo error was not recovered
  1  8  =  0185   ||    Illegal head or cylinder requested
  1  8  =  0186   ||    A servo command is already active
  1  8  =  0187   ||    A servo command was already sent
  1  8  =  0188   ||    Servo command ACK was active for no reason
  1  8  =  0189   ||    Servo processor did not respond to request
  1  8  =  018a   ||    Servo processor did not finish command in time
  1  8  =  018c   ||    Configuration sector upload error
  1  8  =  018d   ||    Configuration sector uploaded but data invalid
  1  8  =  018f   ||    Buffer too small to do a format
  1  8  =  0195   ||    BATS #2 Error; Seek test failure
  1  8  =  0196   ||    BATS #2 Error; Head Offset Test failure
  1  8  =  0197   ||    Self Init Reset; No task available
  1  8  =  0199   ||    Self Init Reset; 33C93A reset was unsuccessful
  1  8  =  019A   ||    Self Init Reset; 60C40 reset was unsuccessful
  1  8  =  019C   ||    Self Init Reset; Control Store Address Fault
  1  8  =  019F   ||    Controller data not loaded
  1  8  =  01C0   ||    ISERE error
  1  8  =  01D0   ||    No sector found error.  (ID no sync failure)
  1  8  =  01D1   ||    No data sync found
  1  8  =  01D2   ||    Data ECC check (no ECC applied to file data)
  1  8  =  01D3   ||    Data correction applied to file for data ECC check
  1  8  =  01D4   ||    ECC check corrected without using ECC correction
  1  8  =  01D5   ||    Track characterization failure during reassign
                  |        Can't determine LBA due to adjacent read ID
                  |        and one of the sectors is defective
  1  8  =  01D6   ||    Data ECC check detected while outside of the write band
  1  8  =  01D7   ||    Offtrack ECC corrected using ECC correction
  1  8  =  01D8   ||    Offtrack ECC corrected without using ECC correction
  1  8  =  01D9   ||    BATS#2 Error; Read Write test failure
  1  8  =  01DA   ||    BATS#2 Error; ECC/CRC test failure
  1  8  =  01DB   ||    Data sync error detected while outside the write band
  1  8  =  01E0   ||    a60C40 non channel parity error
  1  8  =  01E1   ||    a60C40 channel parity error on read
  1  8  =  01E2   ||    a60C40 channel parity error on write
  1  8  =  01E3   ||    a60C40 buffer access not ready error
  1  8  =  01E5   ||    a60C40 channel B was busy before start of a data xfer
  1  8  =  01E6   ||    a60C40 chan err during a xfer from Data buf to ctrlstr RAM
  1  8  =  01E7   ||    a60C40 chan err during a xfer from Ctr str RAM to Data Buf
  1  8  =  01E8   ||    a60C40 xfer from data buf to ctrl str ram never completed
  1  8  =  01E9   ||    a60C40 xfer from ctrl str ram to data buf never completed
  1  8  =  01F0   ||    Checksum error when loading 10C00 sequencer
  1  8  =  01F1   ||    a10C00 sequencer not stopped when loading
  1  8  =  01F2   ||    a10C00 sequencer invalid interrupt error
  1  8  =  01F3   ||    a10C00 sequencer invalid read SEQSTOP
  1  8  =  01F4   ||    a10C00 parity error on read
  1  8  =  01F5   ||    a10C00 parity error on write

~endif
%
* UEC codes specific to 320mb and 400mb
~if (is_320mb || is_400mb)

  1  8  =  018b   ||    Motor timeout error
  1  8  =  01C1   ||    ISERE error; AE2 error; Arm electronics error
  1  8  =  01C2   ||    ISERE error; AE2 error; Arm electronics error
  1  8  =  01C3   ||    ISERE error; TMS lost i.e.s.; Servo microproc err
  1  8  =  01C4   ||    ISERE error; Sector overrun err
  1  8  =  01C5   ||    ISERE error; TMS write inhibit err
  1  8  =  01C6   ||    ISERE error; SCSI Reset err
  1  8  =  01C8   ||    ISERE error; No err bit set in drive fault stat reg
                  |        but drive fault int was detected
  1  8  =  01F6   ||    Write Gate not detected during Write
  1  8  =  02A1   ||    Servo error; Loss of PES Interrupt Status
  1  8  =  02A2   ||    Servo error; Settle timeout
  1  8  =  02A3   ||    Servo error; Coarse offtrack
  1  8  =  02A4   ||    Servo error; 3 consecutive fack SIDS detected
  1  8  =  02A5   ||    Servo error; Unexpected Guardband detected
  1  8  =  02A6   ||    Servo error; Settle Overshoot
  1  8  =  02A7   ||    Servo error; Seek timeout
  1  8  =  02A8   ||    Servo error; Invalid cylinder address
  1  8  =  02A9   ||    Servo error; Command reject
  1  8  =  02AB   ||    Servo error; Max velocity exceeded
  1  8  =  02AC   ||    Servo error; Invalid head requested
  1  8  =  02AD   ||    Servo error; Reference track unreadable
  1  8  =  02AE   ||    Servo error; Headswitch move out of range. (TURBO ONLY)
  1  8  =  02AF   ||    Servo error; Vel. too high( >1 trk/sec) at settle hndoff
  1  8  =  02B3   ||    Servo error; Recal tmout looking for outside diam grdband
  1  8  =  02B4   ||    Servo error; Recal tmout failed to see data band looking for grdband
  1  8  =  02B5   ||    Servo error; Recal tmout looking 6 outsd diam grdband sctrs
  1  8  =  02B6   ||    Servo error; Recal tmout looking 6 consecutive non-grdbands
  1  8  =  02B7   ||    Servo error; Recal tmout looking for 1st non-grdband track type 0.

~endif

* UEC codes specific to 857mb
~if (is_857mb)

  1  8  =  0190   ||    Servo microprocessor BATs err #1 code - as detected
                  |        by interface processor
  1  8  =  0193   ||    Servo microprocessor executing incorrect cmd
  1  8  =  0194   ||    Servo error; Servo microprocessor has presented
                  |        unexpected status.
  1  8  =  01C9   ||    ISERE error; ISERE Drive fault err bits can be reset
  1  8  =  01CA   ||    BATS #2 err; Safety circuits test failure
  1  8  =  02A0   ||    Servo error; bad TMS status returned  $tms_status
  1  8  =  02B0   ||    Servo error; Servo microproc fnd Spindle motor error

~endif

~if is_857mb
                  % 8) Internal Registers
  1 10  @  16     |    Safety stat 1, 2 = 0x%.4x
  1 17  @         |    Safety Ras 0 = 0x%.2x
  1 18  @  16     |    TMS Status (0x%.4x) @tms_is
  ~if tms_status
    * TMS status is two bytes wide but the inner two nibble are valid as
    * follows:  xTTx. To decode shift to right 4 bits then ignore upper 4
    tms_is := tms_is >> 4
    tms_is := tms_is & 255
    tms_is = 0000 |      Command complete
    tms_is = 0002 |      TMS RAM
    tms_is = 0003 |      ROM
    tms_is = 0006 |      Waiting for command
    tms_is = 0008 |      Command ready bit set
    tms_is = 0009 |      command FFFF,F7FF,F7F7
    tms_is = 000A |      command ready bit cleared
    tms_is = 000B |      Status ready bit set
    tms_is = 000C |      Status ready bit cleared
    tms_is = 000E |      BATS failed
    tms_is = 000F |      Response to reset commands F200/F700
    tms_is = 0010 |      PES interrupt missing
    tms_is = 0018 |      Port3 write/read
    tms_is = 001A |      Port6 write/port7 read
    tms_is = 001B |      Port7 write/port6 read
    tms_is = 001C |      Port8 read
    tms_is = 0020 |      Settle timeout
    tms_is = 0021 |      FETs are not shorted
    tms_is = 0022 |      Settle timeout single retry
    tms_is = 0023 |      FETs turn on ok and fire current limit
    tms_is = 0024 |      Current limit resets ok
    tms_is = 0025 |      Current limit or PFET failed
    tms_is = 0045 |      Retract failed
    tms_is = 0047 |      Carriage latch/actuator stuck at ID
    tms_is = 0048 |      Requested seek track # is too high
    tms_is = 0051 |      Motor locked up
    tms_is = 0052 |      Motor start accepted
    tms_is = 0054 |      Motor rotor stuck
    tms_is = 0057 |      Demod sync failed (see SW0(11,10))
    tms_is = 0059 |      Motor cold start timeout
    tms_is = 005E |      Motor hot start timeout
    tms_is = 0062 |      Motor soft phase error
    tms_is = 0063 |      Motor stop complete
    tms_is = 0064 |      Motor stop accepted
    tms_is = 0065 |      Motor soft velocity error ??? new msg
    tms_is = 0066 |      Demod locked in wrong phase
    tms_is = 0068 |      Motor demod in sync
    tms_is = 0069 |      Demod lost, motor unlocked
    tms_is = 006A |      Motor phase lock commencing
    tms_is = 006B |      Motor demod sync erratic
    tms_is = 006D |      Current sense circuit failed
    tms_is = 006E |      Motor open circuit
    tms_is = 006F |      Motor short circuit
    tms_is = 0070 |      Seek timeout
    tms_is = 0073 |      Demodulator became unsafe
    tms_is = 007A |      Motor stuck or stalled during start
    tms_is = 007F |      Motor interrupt failed
    tms_is = 0081 |      Manufacturing mode set up
    tms_is = 0084 |      Actuator ID guard band violation
    tms_is = 0085 |      Actuator OD guard band violation
    tms_is = 0086 |      Shift head accepted ??? new msg
    tms_is = 0090 |      Reject command
    tms_is = 00A2 |      Recalibrate accepted
    tms_is = 00B0 |      Actuator maximum velocity exceeded
    tms_is = 00D0 |      3 consecutive PES rejections
    tms_is = 00FA |      Command accepted
    tms_is = 00FD |      Stationary supervisor
    tms_is = 00FE |      Recalibrate complete
    tms_is = 00FF |      FCM not ready for message
  ~endif
~endif

~if (is_355mb || is_670mb)
                  % 9) Error Recovery

    1 17  @           |    Recovery Count = %d decimal
    1 18  @  16       |    Actual Error Recovery Time = %d decimal

~elseif retryable || (servo_err && nonrecovered)
                  % 9) Physical Block Address
                  %    -- physical location of the error
    1 12  @  16       |       Cylinder = 0x%.4x
    1 14  @           |           Head = 0x%.2x
    1 15  @           |         Sector = 0x%.2x
~endif
                  % 10) Error segment information
4 20  @  32       |     Segment count in which err occurred      = 0x%.8x
4 24  @  32       |     Byte count within seg where err occurred = 0x%.8x

