# @(#)74        1.1  src/bos/diag/util/udsense/arrays.dsense.sh, dsaudsense, bos411, 9435D411a 9/2/94 09:32:14
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
arrays.dsense    Sense Data file
                 (c) IBM Corporation 1991, 1992, 1993

The following is a list of routines included

  array             -- 7135 RAIDiant Array
  7135_array        -- General 7135 array routine
  do_7135_lun_stat  -- Support routines for 7135
  do_7135_dto
  do_7135_hd
  do_7135_recovery
  do_7135_fru_code
  do_7135_fcq
  do_7135_asc
  3514212           -- 3514-212 Array
  3514213           -- 3514-213 Array
  3514312           -- 3514-312 Array
  3514313           -- 3514-313 Array
  7137412           -- 7137-412 Array
  7137413           -- 7137-413 Array
  7137414           -- 7137-414 Array
  7137512           -- 7137-512 Array
  7137513           -- 7137-513 Array
  7137514           -- 7137-514 Array
  3514_gen          -- General Sense Analysis for 3514 and 7137 arrays


!array
* Sense information for the 7135 array
%                     7135 RAIDiant Array Sense Data Analysis
%
* Set flag indicating this is the 7135 array device
#set is_7135 1
#set is_scsi_array 1
* Do particulars for hardfiles same for array
#include 7135_array

!dac7135
* Sense information for the 7135 array
%                     7135 RAIDiant Array Sense Data Analysis
%
* Set flag indicating this is the 7135 array device
#set is_7135 1
#set is_scsi_array 1
* Do particulars for hardfiles same for array
#include 7135_array

!7135_array
#define is_7135
#define is_scsi_array
#define fru_code
#define bus
#define id
#define asc

#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   $lba_valid

0 20  = 70            |    Error for the current command
0 20  = f0            |    Error for the current command
0 20  = 71            |    Deferred error
0 20  = f1            |    Deferred error
0 22  @               |    a) Sense Key (0x%.4x)  @sense_key
0 22  = 00            |        No Sense
                      |       --There is no sense key info to be reported for the unit
                      |         This code occurs for a successfully completed command
0 22  = 01           ||       Recovered error  $retryable
                      |       --The last cmnd completed successfully with recovery
                      |         action performed by the target
0 22  = 02           ||       Not Ready   ^sense_progress
                      |       --The logical unit can not be addressed
0 22  = 03           ||       Medium Error     $retryable
                      |       --The command terminated with a non-recoverable
                      |         error condition caused by a flaw in the media or
                      |         an error in the recorded data
0 22  = 04           ||       Hardware Error   $retryable $nonrecovered
                      |       --The target detected a non-recoverable hardware error
                      |         while performing a command or during a diagnostic test
0 22  = 05           ||       Illegal Request  $illegal_request
                      |       --There was an illegal parameter in the Command
                      |         Descriptor Block or additional parameter supplied as data
0 22  = 06           ||       Unit Attention
                      |       --Reported after an attention causing event
0 22  = 0b           ||       Aborted Command
                      |       --Target aborted the command
0 22  = 07           ||       Data Protect
                      |       --Write attempted on a write protected media
0 22  = 0e           ||       Miscompare
                      |       --Used by Verify or Read Data Buffer to indicate that
                      |         the source data did not match data read from disk
0 27 @                |    c) Additional Sense Length = %d



1  0 @ 16             |    e) ASC/ASCQ (%.4X)
1  0 @ 16 @asc
#include do_7135_asc
1  2 @ 8              |    f) FRU Code                    (%.2X)
1  2 @ 8 @fru_code
#include do_7135_fru_code
1 14 @ 16 @fcq
#include do_7135_fcq
1  6 @ 16             |    g) Recovery Action(s)          (%.4X)
#include do_7135_recovery
1  8 @ 8              |    h) Total number of Errors      (%d)
1  9 @ 8              |    i) Total Retry Count           (%d)
                      |    j) ASC/ASCQ Stack (for multiple errors)
1 10 @ 16 @asc
#include do_7135_asc
1 16 @ 8 @fru_code
#include do_7135_fru_code
1 17 @ 16 @fcq
#include do_7135_fcq
1 12 @ 16 @asc
#include do_7135_asc
1 19 @ 8 @fru_code
#include do_7135_fru_code
1 20 @ 16 @fcq
#include do_7135_fcq
2  7 @ 8              |    k) Host ID                     (%d)
2  8 @ 16             |    l) Host Descriptor             (0x%.4X)
#include do_7135_hd
2 10 @ 8              |    m) Board Serial Number   %c\
2 11 @ 8              |%c\
2 12 @ 8              |%c\
2 13 @ 8              |%c\
2 14 @ 8              |%c\
2 15 @ 8              |%c\
2 16 @ 8              |%c\
2 17 @ 8              |%c\
2 18 @ 8              |%c\
2 19 @ 8              |%c
2 26 @ 8              |    n) Software Level        %c\
2 27 @ 8              |%c\
2 28 @ 8              |%c\
2 29 @ 8              |%c
2 30 @ 8              |    o) Data Transfer Operation     (0x%.2x)
#include do_7135_dto
2 31 @ 8              |    p) LUN                         (%d)
3  0 @ 8              |    q) LUN Status                  (0x%.2x)
#include do_7135_lun_stat
3  1 @ 8              |    r) Drive Identifier            (0x%.2x)
3  1 @ 8 @fru_code
~if (fru_code != 0)
  #include do_7135_fru_code
~endif
3  2 @ 8              |    s) Transfer Start ID           (0x%.2x)
3  2 @ 8 @fru_code
~if (fru_code != 0)
  #include do_7135_fru_code
~endif
3  3 @ 8              |    t) Drive Software level        %c\
3  4 @ 8              |%c\
3  5 @ 8              |%c\
3  6 @ 8              |%c
3  7 @ 8              |    u) Drive Product ID            %c\
3  8 @ 8              |%c\
3  9 @ 8              |%c\
3 10 @ 8              |%c\
3 11 @ 8              |%c\
3 12 @ 8              |%c\
3 13 @ 8              |%c\
3 14 @ 8              |%c\
3 15 @ 8              |%c\
3 16 @ 8              |%c\
3 17 @ 8              |%c\
3 18 @ 8              |%c\
3 19 @ 8              |%c\
3 20 @ 8              |%c\
3 21 @ 8              |%c\
3 22 @ 8              |%c
3 25 @ 8              |    v) RAID Level                  (%d)
3 26 @ 8              |    w) Drive Sense ID              (0x%.2x)
3 26 @ 8 @fru_code
~if (fru_code != 0)
  #include do_7135_fru_code
~endif
3 28 @ 8 @id
~if (id != 0)
                      | 7) Drive Has Returned Sense Information
3 28  = 70            |      Error for the current command
3 28  = f0            |      Error for the current command
3 28  = 71            |      Deferred error
3 28  = f1            |      Deferred error
3 30 @ 8              |    a)  Sense Key                   (0x%.2X)
4  8 @ 16             |    b)  Drive Sense Code/Qualifier  (%.4X)
4 16 @ 16             |    c)  Drive UEC                   (%.4X)
4 12 @ 16             |    d)  Retry Count                 (%d)

!do_7135_lun_stat
#define lun_stat
3  0 @ 8 @lun_stat
~if (lun_stat = 0x0)
                      |         Optimal LUN
  3  0 = 00           |           Optimal condition
  3  0 = 10           |           Parameter mismatch
  3  0 = 40           |           Replaced RAID 0 drive being reformatted
~ elseif (lun_stat = 0x1)
                      |         Degraded LUN
  3  0 = 01           |           Drive failure
  3  0 = 11           |           Parameter mismatch
  3  0 = 21           |           Channel mismatch
  3  0 = 31           |           ID mismatch
  3  0 = 41           |           Replaced drive being formatted
  3  0 = 81           |           Component failure
~elseif (lun_stat = 0x2)
                      |         Reconstructing LUN
  3  0 = 02           |           Reconstruction initiated
  3  0 = 12           |           Parameter mismatch
  3  0 = 22           |           Channel mismatch
  3  0 = 82           |           Component failure
~elseif (lun_stat = 0x4)
                      |         Dead LUN
  3  0 = 04           |           Multiple drive failures
  3  0 = 14           |           Parameter mismatch
  3  0 = 24           |           Channel mismatch
  3  0 = 34           |           ID mismatch
  3  0 = 44           |           Format in progress
  3  0 = 54           |           Awaiting format
  3  0 = 74           |           Replaced wrong drive
  3  0 = 84           |           Component failure
~endif

!do_7135_dto
2 30 ^ 80             |         Error Occurred on Parity Drive.
2 30 = 00             |         Read operation to the drive(s)
2 30 = 01             |         Read portion of a read/modify/write operation RAID 5
                      |         Write operation for RAID 0, 1, or 3
2 30 = 02             |         Write portion of read/modify/write operation RAID 5
2 30 = 03             |         Read recovery operation
2 30 = 04             |         Read recovery for the read portion of a read/modify/write
                      |         operation RAID 5
                      |         Write recovery in RAID 1 or 3
2 30 = 05             |         Write recovery for the write portion of a read/modify/write
                      |         operation RAID 5
2 30 = 06             |         Reconstruction
2 30 = 07             |         Processor data transfer to source
2 30 = 08             |         Processor data transfer from source
2 30 = 09             |         Processor data transfer to destination
2 30 = 0A             |         Processor data transfer from destination
2 30 = 0B             |         Transfer zeros to source
2 30 = 0C             |         SRAM transfer to source
2 30 = 0D             |         SRAM transfer from source
2 30 = 0E             |         Verify parity operation
2 30 = 0F             |         Processor to source transfer followed by save pointers
2 30 = 10             |         Processor from source transfer followed by save pointers
2 30 = 11             |         Write data and generate parity by reading all other
                      |         data drives
2 30 = 12             |         SRAM read from destination
2 30 = 13             |         SRAM read to destination
2 30 = 14             |         Processor from source with no disconnect
2 30 = 15             |         No data transfer
2 30 = 16             |         Copy data channel to channel
2 30 = 17             |         Completing the write operation of a read/modify/write
                      |         after an abort condition
2 30 = 18             |         Continuos write operation
2 30 = 19             |         SDP data transfer abort
2 30 = 1B             |         Two-drive reconstruct with SRAM

!do_7135_hd
2  8 ^ 0100           |         Data is being transfered 16-bit wide
2  8 ^ 0200           |         Data is being transfered 32-bit wide
2  8 ^ 0400           |         Wide negotiation completed
                      |         May have negotiated for 8-bit wide
2  8 ^ 0001           |         Message-using host
2  8 ^ 0002           |         Reselectable host
2  8 ^ 0004           |         Data is being transfered synchonously
2 8 !^ 0004           |         Data is being transfered asynchonously
2  8 ^ 0008           |         Synchonous negotiation successful
2  8 ^ 0020           |         AEN supported
2  8 ^ 0040           |         Polled AEN supported

!do_7135_recovery

1  6 ^ 0001           |         Parity used
1  6 ^ 0002           |         Error manager reconstruction
1  6 ^ 0004           |         Issued Rezero Unit command
1  6 ^ 0008           |         Issued Reassign Block command
1  6 ^ 0010           |         Issued Start/Stop Unit command
1  6 ^ 0020           |         Issues Abort or Bus Device Reset message
1  6 ^ 0040           |         Asserted Reset signal on drive channel
1  6 ^ 0080           |         Retried SCSI operation without chip sequences
1  6 ^ 0100           |         Transferred data asynchronously
1  6 ^ 0200           |         Renegotiated synchronous parameters
1  6 ^ 0400           |         Retried drive busy status
1  6 ^ 0800           |         Drive marked in Warning state
1  6 ^ 1000           |         Drive command retried
1  6 ^ 2000           |         Downed drive
1  6 ^ 4000           |         Downed LUN
1  6 ^ 8000           |         Information logged

!do_7135_fru_code
~if (fru_code >= 0x10)
  bus := fru_code >> 4
  id  := fru_code & 15
                      |         SCSI BUS = %d SCSI ID = %d ~bus id
~endif
fru_code = 01         |         Host Channel Group
                      |         Consists of the host SCSI bus, its SCSI interface chip, and all
                      |         the initiators and other targets connected to the bus.
fru_code = 02         |         Controller Drive Interface Group
                      |         Consists of the SCSI interface chips on the controller that
                      |         connect to the drive buses.
fru_code = 03         |         Controller Buffer Group
                      |         Consists of the controller logic used to implement the on-board
                      |         data buffer.
fru_code = 04         |         Controller Array ASIC's Group
                      |         Consists of the ASIC's on the controller that are associated with
                      |         the array functions.
fru_code = 05         |         Controller Other Group
                      |         Consists of all controller related hardware not associated with
                      |         another group. This group includes the controller power supply.
fru_code = 06         |         Subsystem Group
                      |         Consists of subsystem components (such as power supplies, fans,
                      |         thermal sensors and AC power monitors) that are monitored by the
                      |         controller.
fru_code = 07         |         Subsystem Configuration Group
                      |         Consists of user-configurable subsystem components on which the
                      |         controller displays information (such as faults).

!do_7135_fcq
~if (fru_code)
                      |       FRU Code Qualifier  0x%.4x ~fcq
        ~if (fru_code = 0x01)
           fcq ^ 0001 |         SCSI bus cable
           fcq ^ 0002 |         SCSI interface chip
        ~elseif (fru_code >= 0x10)
           fcq ^ 0001 |         Drive HDA
           fcq ^ 0002 |         Drive controller electronics
           fcq ^ 0004 |         Drive power supply
           fcq ^ 0008 |         SCSI cable (drive)
        ~elseif (fru_code = 0x02)
           fcq ^ 0100 |         Channel  9 SCSI chip
           fcq ^ 0200 |         Channel 10 SCSI chip
           fcq ^ 0400 |         Channel 11 SCSI chip
           fcq ^ 0800 |         Channel 12 SCSI chip
           fcq ^ 1000 |         Channel 13 SCSI chip
           fcq ^ 2000 |         Channel 14 SCSI chip
           fcq ^ 4000 |         Channel 15 SCSI chip
           fcq ^ 0001 |         Channel  1 SCSI chip
           fcq ^ 0002 |         Channel  2 SCSI chip
           fcq ^ 0004 |         Channel  3 SCSI chip
           fcq ^ 0008 |         Channel  4 SCSI chip
           fcq ^ 0010 |         Channel  5 SCSI chip
           fcq ^ 0020 |         Channel  6 SCSI chip
           fcq ^ 0040 |         Channel  7 SCSI chip
           fcq ^ 0080 |         Channel  8 SCSI chip
        ~elseif (fru_code = 0x03)
                      |         Failing memory device identifier
        ~elseif (fru_code = 0x04)
                      |         SDP Chip
        ~elseif (fru_code = 0x05)
           fcq ^ 0001 |         Microprocessor
           fcq ^ 0002 |         Array controller power system
           fcq ^ 0004 |         Non volatile memory
        ~elseif (fru_code = 0x07)
           fcq = 0000 |         Checksum error in Drive Table
           fcq = 0100 |         Invalid Drive Table
           fcq = 0200 |         Invalid rank ID in Drive Table
           fcq = 0300 |         Invalid controller destination ID in Drive Table
           fcq = 1000 |         No Subsystem Table
           fcq = 1100 |         Checksum error in Subsystem Table
        ~endif
~endif

!do_7135_asc
#define asc1
asc = 0000            |         No Additional Sense Information
asc = 0006            |         I/O Process Terminated
asc = 0100            |         No Index/Sector Signal
asc = 0200            |         No Seek Complete
asc = 0300            |         Peripheral Device Write Fault
asc = 0400            |         Logical Unit Not Ready, Cause Not Reportable
asc = 0401            |         Logical Unit Is In Process Of Becoming Ready
asc = 0402            |         Logical Unit Not Ready, Initializing Command Required
                      |         Start Unit command required
asc = 0403            |         Logical Unit Not Ready, Manual Intervention Required
asc = 0404            |         Logical Unit Not Ready, Format In Progress
asc = 0480            |         Logical Unit Not Ready, ROM Installed Does Not Support Redundant
                      |         Controller Configuration
asc = 0500            |         Logical Unit Does Not Respond To Selection
asc = 0600            |         No Reference Position Found
asc = 0700            |         Multiple Peripheral Devices Selected
asc = 0800            |         Logical Unit Communication Failure
asc = 0801            |         Logical Unit Communication Timeout
asc = 0802            |         Logical Unit Communication Parity Error
asc = 0900            |         Track Following Error
asc = 0A00            |         Error Log Overflow
asc = 0C01            |         Write Error Recovered With Auto Reallocation
asc = 0C02            |         Write Error - Auto Reallocation Failed
asc = 1000            |         ID CRC or ECC Error
asc = 1100            |         Unrecovered Read Error
                      |           The description of this error as being unrecovered may be
                      |           misleading because a drive-reported Unrecovered Read Error can
                      |           be recovered by the array using parity.  The sense key for this
                      |           case is Recovered and the Recovery Actions field in the sense
                      |           indicates that parity was used.
asc = 1101            |         Read Retries Exhausted
asc = 1102            |         Error Too Long To Correct
asc = 1104            |         Unrecovered Read Error - Auto Reallocate Failed
asc = 110A            |         Miscorrected Error
asc = 110B            |         Unrecovered Read Error - Recommend Reassignment
asc = 110C            |         Unrecovered Read Error - Recommend Rewrite The Data
asc = 1200            |         Address Mark Not Found For ID Field
asc = 1300            |         Address Mark Not Found For Data Field
asc = 1400            |         Recorded Entity Not Found
asc = 1401            |         Record Not Found
asc = 1500            |         Random Positioning Error
asc = 1501            |         Mechanical Positioning Error
asc = 1502            |         Positioning Error Detected By Read Of Medium
asc = 1600            |         Data Synchronization Mark Error
asc = 1700            |         Recovered Data With No Error Correction Applied
asc = 1701            |         Recovered Data With Retries
asc = 1702            |         Recovered Data With Positive Head Offset
asc = 1703            |         Recovered Data With Negative Head Offset
asc = 1705            |         Recovered Data Using Previous Sector ID
asc = 1706            |         Recovered Data Without ECC - Data Auto-Reallocated
asc = 1707            |         Recovered Data Without ECC - Recommend Reassignment
asc = 1800            |         Recovered Data With Error Correction Applied
asc = 1801            |         Recovered Data With Error Correction And Retries Applied
asc = 1802            |         Recovered Data - Data Auto-Reallocated
asc = 1805            |         Recovered Data - Recommend Reassignment
asc = 1900            |         Defect List Error
asc = 1901            |         Defect List Not Available
asc = 1902            |         Defect List Error In Primary List
asc = 1903            |         Defect List Error In Grown List
asc = 1A00            |         Parameter List Length Error
asc = 1B00            |         Synchronous Data Transfer Error
asc = 1C00            |         Defect List Not Found
asc = 1C01            |         Primary Defect List Not Found
asc = 1C02            |         Grown Defect List Not Found
asc = 1D00            |         Miscompare During Verify Operation
asc = 1E00            |         Recovered ID With ECC Correction
asc = 2000            |         Invalid Command Operation Code
asc = 2100            |         Logical Block Address Out Of Range
asc = 2200            |         Illegal Function
asc = 2400            |         Invalid Field In CDB
asc = 2500            |         Logical Unit Not Supported
asc = 2600            |         Invalid Field In Parameter List
asc = 2601            |         Parameter Not Supported
asc = 2602            |         Parameter Value Invalid
asc = 2603            |         Threshold Parameters Not Supported
asc = 2700            |         Write Protected
asc = 2800            |         Not Ready To Ready Transition (Medium May Have Changed)
asc = 2900            |         Power On, Reset, Or Bus Device Reset Occurred
asc = 2A00            |         Parameters Changed
asc = 2A01            |         Mode Parameters Changed
asc = 1800            |         Recovered Data With Error Correction Applied
asc = 1801            |         Recovered Data With Error Correction And Retries Applied
asc = 1802            |         Recovered Data - Data Auto-Reallocated
asc = 1805            |         Recovered Data - Recommend Reassignment
asc = 1900            |         Defect List Error
asc = 1901            |         Defect List Not Available
asc = 1902            |         Defect List Error In Primary List
asc = 1903            |         Defect List Error In Grown List
asc = 1A00            |         Parameter List Length Error
asc = 1B00            |         Synchronous Data Transfer Error
asc = 1C00            |         Defect List Not Found
asc = 1C01            |         Primary Defect List Not Found
asc = 1C02            |         Grown Defect List Not Found
asc = 1D00            |         Miscompare During Verify Operation
asc = 1E00            |         Recovered ID With ECC Correction
asc = 2000            |         Invalid Command Operation Code
asc = 2100            |         Logical Block Address Out Of Range
asc = 2200            |         Illegal Function
asc = 2400            |         Invalid Field In CDB
asc = 2500            |         Logical Unit Not Supported
asc = 2600            |         Invalid Field In Parameter List
asc = 2601            |         Parameter Not Supported
asc = 2602            |         Parameter Value Invalid
asc = 2603            |         Threshold Parameters Not Supported
asc = 2700            |         Write Protected
asc = 2800            |         Not Ready To Ready Transition (Medium May Have Changed)
asc = 2900            |         Power On, Reset, Or Bus Device Reset Occurred
asc = 2A00            |         Parameters Changed
asc = 2A01            |         Mode Parameters Changed
asc = 3200            |         No Defect Spare Location Available
asc = 3201            |         Defect List Update Failure
asc = 3700            |         Rounded Parameter
asc = 3900            |         Saving Parameters Not Supported
asc = 3A00            |         Medium Not Present
asc = 3D00            |         Invalid Bits In Identify Message
asc = 3E00            |         Logical Unit Has Not Self-Configured Yet
asc = 3F00            |         Target Operating Conditions Have Changed
asc = 3F01            |         Microcode Has Been Changed
                      |           Returned when new drive microcode has been downloaded.
asc = 3F02            |         Changed Operating Definition
asc = 3F03            |         Inquiry Data Has Changed
asc = 3F80            |         Drive Failed Because Of A Failed Write Operation
                      |           This is the result of an error that prevents access to user data
                      |           on this drive or that may result in erroneous data being
                      |           read/written (for example, an Unrecovered Write Error).
asc = 3F81            |         Drive Failed- Automatic Reallocation Failed
                      |           Either reassign block or recovery of data on the reassigned
                      |           sector failed.
asc = 3F82            |         Drive Failed - Reconstruction Failed
                      |           (Error On Drive Being Reconstructed)
                      |           The reconstruction operation can not complete because of an
                      |           unrecoverable Write error or a failed drive format (before
                      |           reconstruction starts) on the drive being reconstructed.  It
                      |           must be replaced (again) before the reconstruction can be
                      |           retried.
asc = 3F83            |         Reconstruction Failed - Drive Warned
                      |           (Error On Drive Required For Reconstruction)
                      |           The reconstruction operation can not complete because of an
                      |           unrecoverable Read error on one of the drives needed for
                      |           reconstruction.  You can still run the array in degraded mode,
                      |           but if user data is stored at the address in error, you can not
                      |           read it You should attempt a back up of the array to recover the
                      |           remaining data.  To bring the LUN to an optimal state, replace
                      |           the failed drive, format the array, and restore the data from a
                      |           back-up copy (the one just made or a previous successful back-up
                      |           copy).
asc = 3F84            |         Drive Failed Due To A Hardware Component Diagnostics Failure
asc = 3F85            |         Drive Failed Because It Failed A Test Unit Ready Command (during
                      |         start-of-day) Or Read Capacity Command (during start-of-day or
                      |         during a format or re-construction operation)
asc = 3F86            |         Drive Failed Because It Failed A Format Unit Command
asc = 3F87            |         Drive Failed By A Host Mode Select Command
asc = 3F88            |         Drive Failed Because Of Deferred Error Reported By The Drive
asc = 3F89            |         Drive Failed By Start-of-Day Application Code Because Of A Drive
                      |         Replacement Error
asc = 3F8A            |         Drive Failed Due To A Failure In Reconstructing Parity
asc = 3F90            |         Unrecovered Read/Write Error
                      |           Unrecovered Read errors always generate this condition.
                      |           Unrecovered Write errors cause this condition if the state of
                      |           the LUN or the RAID level guarantees that data at the other
                      |           addresses will not be affected by the failed operation and
                      |           continued access to this drive.
                      |           A second drive failure or warning condition on another drive
                      |           can prevent reconstruction of this drive.
asc = 3F91            |         Drive Reported Deferred Error Caused Drive To Be Placed In Warning
asc = 3FA0            |         Single Drive Array Assurance Error
                      |            x = 0x3FA0 - Drive ECC Test Failed
                      |            x = 0x3FAOther - Not Currenlly Implemented
asc = 3FB0            |         Excessive Media Error Rate
asc = 3FB1            |         Excessive Seek Error Rate
asc = 3FB2            |         Excessive Grown Defects
asc = 3FC0            |         No Response From One Or More Drives
asc = 3FC1            |         Communication Errors
asc = 3FC2            |         Firmware Indicates No Drive Is Present Although Information
                      |         Stored On Disk Indicates Drive Should Be Present
asc = 3FC7            |         Subsystem Component Failure
                      |           FRU code and qualifiers identify the failed component.
asc = 3FC8            |         AC Power Is Lost, DC Power Is Being Supplied By A Hold-Over Battery
asc = 3FC9            |         AC Power Is Lost, DC Power Supplied For A Maximum Of Two Minutes
                      |           The host should start its shutdown procedure.
asc = 3FCA            |         AC Power Is Lost, DC Power Is Exhausted
                      |           The controller completes currenlly executing drive commands to
                      |           maintain data integriry
asc = 3FCB            |         AC Power Was Lost, But Is Now Restored
asc = 3FD0            |         > 75% Of Transfer Delays Caused By One Drive
asc = 3FD1            |         > 75% Of Non-Aligned Reselections Caused By One Drive
asc = 3FD2            |         Synchronous Transfer Value Differences Between Drives
asc = 3FD3            |         Software Measured Performance Degradation
asc = 3FD4            |         Mode Parameter Differences Between Drives May Cause Performance
                      |         Degradation
asc = 3FE0            |         LUN Downed
asc = 3FE1            |         Multiple Drives Have Been Downed
asc = 3FE2            |         Mode Parameters For Drives In LUN Don't Match
asc = 3FE3            |         Drive Channel Verification Failed
asc = 3FE4            |         SCSI ID Verification Failed
asc = 3FE5            |         Wrong Drive Was Replaced
asc = 3FE6            |         Component Failure Affecting Multiple Channels
asc = 3FF0            |         EEPROM Error
asc = 3FF1            |         EEPROM Hard Checksum Error
asc = 3FF2            |         Maximum EEPROM Write Count Exceeded
asc = 3FF8            |         Application Software Copy To More Than One Drive Failed.
                      |           Another download is required if the drive with the current
                      |           application code failed.
asc = 4000            |         RAM Failure
~if ( (asc > 0x4000) & (asc <= 0x40FF) )
                      |         Diagnostic Failure On Component NN (80H-FFH)
                      |           In this release of the software, this ASC and ASCQ are only
                      |           returned if reported by a drive.  FRU codes distinguish between
                      |           failures on components of the array controller and components of
                      |           the drive controllers.
~endif
asc = 4100            |         Data Path Failure, Carryover From CCS (same as 40 NN)
asc = 4200            |         Power-On Or Self-Test Failure, Carryover From CCS (same as 40 NN)
asc = 4300            |         Message Error
asc = 4400            |         Internal Target Failure
asc = 4500            |         Select/Reselect Failure
asc = 4600            |         Unsuccessful Soft Reset
asc = 4700            |         SCSI Parity Error
asc = 4800            |         Initiator Detected Error Message Received
asc = 4900            |         Invalid Message Error
asc = 4A00            |         Command Phase Error
asc = 4B00            |         Data Phase Error
asc = 4B80            |         Data Overrun/Underrun
asc = 4C00            |         Logical Unit Failed Self-Configuration
asc = 4E00            |         Overlapped Commands Attempted
asc = 5A00            |         Operator Request Or State Change Input (Unspecified)
asc = 5A02            |         Operator Selected Write Protect
asc = 5A03            |         Operator Selected Write Permit
asc = 5B00            |         Log Exception
asc = 5B01            |         Threshold Condition Met
asc = 5B02            |         Log Counter At Maximum
asc = 5B03            |         Log List Codes Exhausted
asc = 5C00            |         RPL Status Change
asc = 5C01            |         Spindles Synchronized
asc = 5C02            |         Spindles Not Synchronized
asc = 8000            |         Error Manager Detected Error
asc = 8001            |         The Error Manager Was Invoked Without Any Error Code(s) Loaded
asc = 8002            |         The Error Manager Was Passed An Out-Of-Range Code
asc = 8003            |         The Error Manager Was Passed A Code By The SCSI Driver That Was
                      |           Not Defined As An Error Condition
asc = 8004            |         Fatal Null Pointer
asc = 8005            |         No AEN Code Or An Invalid AEN Code Was Loaded By The Application
                      |         Software
                      |           This error is detected when the AEN condition is requested by
                      |           the command handler (either when the next command for this
                      |           host/LUN is received or when a polled Request Sense is
                      |           received), not when the AEN condition was detected.  The illegal
                      |           AEN code is loaded into the Error-Specific Information field in
                      |           the sense data.
asc = 8006            |         Maximum # Of Errors For This I/O Exceeded
asc = 8007            |         Drive Reported Recovered Error Without Transferring All Of Data
asc = 8100            |         Reconstruction Setup Failed
asc = 8200            |         Out Of Heap
asc = 8201            |         No Command Control Structures Available
asc = 8202            |         No DAC Application Control Blocks Available
asc = 8300            |         Reservation Conflict
asc = 8400            |         Command Cannot Execute Because The LUN Has Been Downed
asc = 8401            |         Multiple Drives Have Been Downed
asc = 8402            |         Mode Parameters For Drives in LUN Do Not Match
asc = 8403            |         Drive Channel Verification Failed
asc = 8404            |         SCSI ID Verification Failed
asc = 8405            |         Format In Progress
asc = 8406            |         Awaiting Format Command
asc = 8408            |         Wrong Drive Was Replaced
asc = 8409            |         Component Failure Affecting Multiple Channels
asc = 8500            |         General Application Code Command Handler Error
asc = 8501            |         Drive Error
asc = 8502            |         Host Error
asc = 8503            |         Drive Type Mismatch Within LUN
asc = 8504            |         Operation Not Allowed During Reconstruction
asc = 8505            |         Data Returned By Drive Is Invalid
asc = 8506            |         Non-Failed Drive Unavailable For Operations
asc = 8507            |         Insufficient Rank Structures Available
asc = 8508            |         Full Format Required, But Not Allowed (Sub LUNs)
asc = 8509            |         Drive Cannot Be Mode Selected To Meet LUN Parameters
asc = 850A            |         Data Recovery After Re-Assign Block Command Failed
asc = 850B            |         Drive Not Returning Required Mode Sense Page(s)
                      |           Drives must support Mode Sense pages 3 and 4.
asc = 850C            |         Format Completed With A LUN Status Of Degraded
asc = 850D            |         Format Completed With A LUN Status Of Dead
asc = 8600            |         Command Cannot Execute Because The LUN Is In Degraded Mode
asc = 8700            |         Code Download/Upload Error
asc = 8701            |         Partial Download (Missing Application Code Segment)
asc = 8702            |         Downloaded Code Cannot Be Saved To Disk
                      |           Old application code will be uploaded on next power-up/reset.
asc = 8703            |         Code CRC Failure
                      |         Check is performed on the disk read after the new code is saved
                      |         to disk.
asc = 8704            |         Upload Of Latest Version Of Code Failed
                      |           An older version of code has been uploaded successfully.
asc = 8705            |         No Package Verification Partition Downloaded
asc = 8706            |         ROM Partitions Required For Download Of Code Missing
asc = 8707            |         Incomplete RAM Partitions
asc = 8708            |         Incompatible Board Type For The Code Downloaded
asc = 8709            |         Incompatible ROM Version For Support Of The Downloaded Code
asc = 870A            |         Download Of Microcode To A Failed Disk Completed Successfully
                      |           The drive is still unusable until it is marked as replaced and
                      |           has been reconstructed.
asc = 8800            |         EEPROM Command Error
asc = 8802            |         EEPROM Not Responding
                      |           (There is an EEPROM on the board.)
asc = 8803            |         EEPROM Not "Formatted" (certain key fields have not been set.)
asc = 8804            |         Invalid EEPROM Offset (a Write to the write-protected maintenance
                      |         area causes this error.)
asc = 8805            |         EEPROM Soft Checksum Error (indicates that an update was interrupted)
asc = 8806            |         EEPROM Hard Checksum Error (indicates a component may be going bad)
asc = 8807            |         Maximum Write Count Exceeded
asc = 8809            |         EEPROM Not Initialized
asc = 880A            |         Subsystem Configuration Error
                      |           This code is always reported with a FRU code of 0x07 (Subsytem                                                                                  This code is always reported with an FRU code of 0x07 (Subsystem
                      |           configuration Group). The FRU qualifiers indicate the type and
                      |           location of the error.
asc = 8900            |         Error On Request Sense Command To A Drive
asc = 8A00            |         Illegal Command For Pass-Through Mode
                      |           The original CDB field in the array sense data shows the failing
                      |           command.
asc = 8A01            |         Illegal Command For Current RAID Level
asc = 8B00            |         Write Buffer Command (For Code Download) Was Attempted While
                      |         Another Command Was Active
asc = 8B01            |         Write Buffer Command (For Drive Microcode Download) Attempted
                      |         But The Enable Bit In The EEPROM Was Not On
asc = 8C00            |         Destination Transfer State Machine Error
asc = 8C01            |         Invalid Transfer Release Requester
asc = 8C02            |         Invalid Transfer Requester
asc = 8C03            |         Data Stripe/Parity Generation ASIC Configuration Error
asc = 8C04            |         Data Transfer Request Error
asc = 8C05            |         Invalid Transfer Pad Requester
asc = 8D00            |         Destination Driver Data Transfer Did Not Complete
asc = 8E00            |         Data Stripe/Parity Generation ASIC Error
asc = 8E01            |         Parity/Data Mismatch
asc = 8E02            |         Data Underrun
asc = 8F00            |         Premature Completion Of a Drive Command (expected Data Transfer
                      |         and received Good Status instead)
~if ( (asc >= 0x9000) && (asc < 0x9100) )
                      %         DACSTORE Errors
                      %           DACSTORE Identifier :\
asc1 := asc & 0xF
asc1 = 0              |   DACSTORE directory
asc1 = 1              |   Disk store
asc1 = 2              |   LUN store
asc1 = 3              |   Controller store
asc1 = 4              |   Log store
asc1 = 5              |   High ID controller serial # store
asc1 = 6              |   Low ID controller serial # store
asc1 = 7              |   RDAC common store
asc1 = 8              |   Boot block EEPROM store
                      %           ErrorType           :\
asc1 := asc & 0x30
asc1 = 00             |   Setup error
asc1 = 10             |   Invalid directory data
asc1 = 20             |   Drive error
asc1 = 30             |   Invalid store data
                      %           Operation Type      :\
asc !^ 0080           |   Read
asc ^ 0080            |   Write
~endif
asc = 9100            |         Mode Select Errors
asc = 9101            |         LUN Already Exists; Cannot Do "Add LUN" Function "80"
asc = 9102            |         LUN Does Not Exist; Cannot Do "Replace LUN" Function "83" Or Any
                      |         Logical Function
asc = 9103            |         Drive Already Exists; Cannot Do "Add Drive" Function "80"
asc = 9104            |         Drive Does Not Exist; Cannot Do Requested Action For It
asc = 9105            |         Drive Can't Be Deleted; It Is Part Of A LUN
asc = 9106            |         Drive Can't Be Failed; It Is Formatting
asc = 9107            |         Drive Can't Be Replaced; It Is Not Marked As Failed Or Replaced
asc = 9108            |         Invalid Action To Take
asc = 9109            |         Invalid Action With Multiple Sub LUNs Defined (probably an attempt
                      |         to change page 3 - Format Device page)
asc = 910A            |         Invalid Reconstruction Amount
asc = 910B            |         Invalid Reconstruction Frequency
asc = 910C            |         Invalid LUN Block Size
asc = 910D            |         Invalid LUN Type
asc = 910E            |         Invalid Segment Size
asc = 910F            |         Invalid Segment 0 Size
asc = 9110            |         Invalid Number Of Drivea In LUN
asc = 9111            |         Invalid Number Of LUN Blocks
asc = 9112            |         Invalid RAID Level
asc = 9113            |         Invalid Drive Sector Size
asc = 9114            |         Invalid LUN Block Size/Drive Sector Size Modulo
asc = 9115            |         No Disks Defined For LUN
asc = 9116            |         Insufficient Rank Structures Available To Define LUN
asc = 9117            |         Disk Defined Multiple Times For LUN
asc = 9118            |         Sub LUN Drives Not The Same As Those Used By Other Sub LUNs On
                      |         These Drives
asc = 9119            |         Sub LUN RAID Level Mismatch
asc = 911A            |         First Sub LUN Defined For These Drives Has Not Yet Been Formatted;
                      |         Second Sub LUN Is Illegal
asc = 911B            |         Non-Sub LUN Drive Already Owned By Another LUN
asc = 911C            |         Sub LUN Drive Already Owned By a Non-Sub LUN
asc = 911D            |         Drive Type Does Not Match The Drive Type Of The Other Drives In
                      |         The LUN
asc = 911E            |         Drive Cannot Be Included In Rank Because Rank Is Full
asc = 911F            |         Ranks Have Different Number Of Disks Defined
asc = 9120            |         Multiple Disks On Same Channel Within Same Rank
asc = 9121            |         Mirrored Disks On The Same Channel
asc = 9122            |         No Parity Disk Defined
asc = 9123            |         No Data Disks Defined
asc = 9124            |         Too Many Disks Defined
asc = 9125            |         No Space Available For LUN - Sub LUN Cannot Be Defined
asc = 9126            |         Drive Status Can Not Be Changed To Good (drive can not be revived
                      |         through Mode Select)
asc = 9127            |         Error In Processing A Subsystem Mode Page
asc = 9128            |         Drive Inquiry Data Mismatch Between Drives In The LUN
asc = 9129            |         Drive Capacity Mismatch Between Drives In The LUN
asc = 912A            |         Drive Block Size Mismatch Between Drives In The LUN
asc = 912B            |         Support Of TTD/CIOP Messages Is Not The Same For All Drives In The
                      |         LUN
asc = 912C            |         Firmware Does Not Support Redundant Controller Options Selected
asc = 9200            |         BUSY Status From Drives Could Not Be Cleared By Array Controller
asc = 9300            |         Drive Vendor Unique Sense Data Returned
                      |           See the drive sense area of the array sense for the drive
                      |           ASC/ASCQ and sense key.  Also refer to the drive documentation
                      |           for further description.
asc = 9400            |         Invalid Request Of A Controller In Redundant Controller Mode.
asc = 9500            |         A Drive Channel Was Reset
                      |         Probable cause is the removal or replacement of a drive during
                      |         a hot swap operation
asc = 9501            |         An Extended Drive Channel Reset Has Been Detected
                      |           Probable cause is a drive left partially removed or inserted
asc = 9600            |         Redundant Controller Not Supported By Current Firmware
asc = 9601            |         Alternate Controller Not Supported By Current Firmware (however,
                      |         the alternate controller has been detected)
asc = B000            |         Command Timeout
asc = B001            |         Watchdog Timer Timeout
asc = B002            |         Software Loop Timeout
asc = D000            |         SCSI Driver Timeout
asc = D001            |         Disconnect Timeout
asc = D002            |         Chip Command Timeout
asc = D003            |         Byte Transfer Timeout
asc = D100            |         Bus Errors
asc = D101            |         CDB Transfer Incomplete
asc = D102            |         Unexpected Bus Phase
asc = D103            |         Disconnect Expected
asc = D104            |         ID Message Not Sent
asc = D105            |         Synchronous Negotiation Error
asc = D106            |         Target Transfer Disable (TTD) Negotiation Conflict
asc = D107            |         Unexpected Disconnect
asc = D108            |         Unexpected Message
asc = D109            |         Unexpected Tag Message
asc = D10A            |         Channel Busy
asc = D200            |         Miscellaneous SCSI Driver Error
asc = D201            |         Illegal C96 Chip Command
asc = D202            |         Uncoded Execution Path
asc = D300            |         Drive SCSI Chip Reported Gross Error
asc = D400            |         Non-SCSI Bus Parity Error
asc = D500            |         Miscellaneous Host-Related Errors
asc = D501            |         Maximum Messages Received
asc = D502            |         Message Reject Received on a Valid Message
asc = D600            |         Source Driver Chip-Related Error
asc = D700            |         Source Driver Programming Error
asc = D800            |         An Error Was Encountered That Required The Data Pointert To Be
                      |         Restored But The Host Is Non-Disconnecting And Does Not Support
                      |         The Restore Pointers Message (indicated by an EEPROM option
                      |         control bit or by host selection without sending the identify
                      |         message)

!3514212
* Sense information for the 3514-212 Storage Subsystem
%                   3514-212 High Availability Storage Subsystem
%
#include 3514_gen


!3514213
* Sense information for the 3514-213 Storage Subsystem
%                   3514-213 High Availability Storage Subsystem
%
#include 3514_gen

!3514312
* Sense information for the 3514-312 Storage Subsystem
%                   3514-312 High Availability Storage Subsystem
%
#include 3514_gen


!3514313
* Sense information for the 3514-313 Storage Subsystem
%                   3514-313 High Availability Storage Subsystem
%
#include 3514_gen

!7137412
* Sense information for the 3514-412 Storage Subsystem
%                   7137-412 High Availability Storage Subsystem
%
#include 3514_gen

!7137413
* Sense information for the 3514-413 Storage Subsystem
%                   7137-413 High Availability Storage Subsystem
%
#include 3514_gen

!7137414
* Sense information for the 3514-414 Storage Subsystem
%                   7137-414 High Availability Storage Subsystem
%
#include 3514_gen

!7137512
* Sense information for the 3514-512 Storage Subsystem
%                   7137-512 High Availability Storage Subsystem
%
#include 3514_gen

!7137513
* Sense information for the 3514-513 Storage Subsystem
%                   7137-513 High Availability Storage Subsystem
%
#include 3514_gen

!7137514
* Sense information for the 3514-514 Storage Subsystem
%                   7137-514 High Availability Storage Subsystem
%
#include 3514_gen

!3514_gen
* General decode for 3514 and 7137

#set is_scsi_array 1
* Include general SCSI information
#include scsi_gen

             % 6) Request Sense Data Decode
* Decode error code info.  At same time detect if LBA or PBA is valid
0 20  ^ 80   ^lba_valid

0 20  = 70    |    Error for the current command
0 20  = f0    |    Error for the current command
0 20  = 71    |    Deferred error
0 20  = f1    |    Deferred error

0 22  @ @temp_byte
   sense_key := (temp_byte & 0x0f)
0 22  @       |    a) Sense Key (0x%.2x)
sense_key = 00  |        No Sense
                |       --There is no sense key info to be reported for the unit
                |         This code occurs for a successfully completed command
sense_key = 01 ||       Recovered error  $retryable ^recovered_error
                |       --The last cmnd completed successfully with recovery
                |         action performed by the target
sense_key = 02 ||       Not Ready   ^not_ready
                |       --The logical unit addressed can not be accessed
sense_key = 03 ||       Medium Error     $retryable ^medium_error
                |       --The command terminated with a non-recoverable
                |         error condition caused by a flaw in the media or
                |         an error in the recorded data
sense_key = 04 ||       Hardware Error   $retryable $nonrecovered ^hardware_error
                |       --The target detected a non-recoverable hardware error
                |         while performing a command or during a diagnostic test
sense_key = 05 ||       Illegal Request  ^illegal_request
                |       --There was an illegal parameter in the Command
                |         Descriptor Block or additional parameter supplied as data
sense_key = 06 ||       Unit Attention
                |       --Reported after an attention causing event
sense_key = 07 ||       Data Protect
                |       --Write attempted on a write protected media
sense_key = 0b ||       Aborted Command
                |       --Target aborted the command

   0 23 @ 32 |    b) Logic Block Address = 0x%.8x  or %d decimal @lba ~lba lba
   ~if lba_valid
             %       LBA is valid
   ~else
             %       NOTE LBA is not precisely valid for the error
   ~endif


0 27 @       |    c) Additional Sense Length = %d

             %    d) Last Logical Block not reassigned if reassign failed
  0 28 @ 32  |       = 0x%.8x
             |       --This value is the first Logical Block Address (LBA)
             |         from the defect descriptor block that was not reassigned
1  0 @ 16    |    e) Additional Sense Code and Sense Code Qualifier (0x%.4x)

#set as_fnd 0
* Codes particular to the device
1 0 = 0000  |       No additional sense information            $as_fnd
1 0 = 0100 ||       No index/sector signal                    $as_fnd
1 0 = 0200 ||       No seek complete                          $as_fnd
1 0 = 0300 ||       Peripheral device write fault             $as_fnd
1 0 = 0400 ||       Logic unit not ready, cause not reportable  $as_fnd
1 0 = 0401 ||       Logic unit is in the process of becoming ready  $as_fnd
1 0 = 0402 ||       Logic unit not ready, initialization cmd required $as_fnd
1 0 = 0403 ||       Logic unit not ready, manual intervention required $as_fnd
1 0 = 0404 ||       Logic unit not ready, format in progress $as_fnd
1 0 = 0802 ||       Logic unit communication parity error    $as_fnd
1 0 = 1B00 ||       Synchronous data transfer error          $as_fnd
1 0 = 0900 ||       Track following error                     $as_fnd
1 0 = 0c01 ||       Write error recovered with auto reallocation  $as_fnd
1 0 = 0c02 ||       Write error - Auto reallocation failed    $as_fnd
1 0 = 0c03 ||       Write error - Recommend reassignment      $as_fnd
1 0 = 0802 ||       Logic unit communication parity error     $as_fnd
1 0 = 0900 ||       Track following error                     $as_fnd
1 0 = 1A00 ||       Parameter List Length Error               $as_fnd
~if (sense_key == 3)
  1 0 = 1100 ||       Unrecovered read error                    $as_fnd
              |       -- Valid only when mode select parameters select
              |       -- limited DPR
~elseif (sense_key == 4)
  1 0 = 1100 ||       Unrecovered read error in reserved area   $as_fnd
  1 0 = 1100 ||       Unrecovered read error                    $as_fnd
~endif
1 0 = 1104 ||       Unrecovered read error -- Auto reallocate failed  $as_fnd
1 0 = 110B ||       Unrecovered read error -- Recommend reassignment  $as_fnd
1 0 = 1400 ||       Recorded entity not found                 $as_fnd
~if (sense_key == 3)
  1 0 = 1401 ||       Record not found                          $as_fnd
              |       -- Valid only when mode select parameters select
              |       -- limited DPR
~elseif (sense_key == 4)
  1 0 = 1401 ||       Record not found - Reserved area          $as_fnd
~else
  1 0 = 1401 ||       Record not found                          $as_fnd
~endif
1 0 = 1405 ||       Record not found - Recommend reassignment $as_fnd
1 0 = 1406 ||       Record not found - Data auto-reallocated  $as_fnd
1 0 = 1500 ||       Random positioning error (Seek Error)     $as_fnd
1 0 = 1502 ||       Positioning error detected by read of medium  $as_fnd
~if (sense_key == 4)
  1 0 = 1600 ||       Data synchronization mark error in reserved area $as_fnd
~else
  1 0 = 1600 ||       Data synchronization mark error           $as_fnd
~endif
1 0 = 1601 ||       Data synchronization mark error - Data rewritten  $as_fnd
1 0 = 1602 ||       Data synchronization mark error - Recommend rewrite $as_fnd
1 0 = 1603 ||       Data synchronization mark error - Data auto reallocated $as_fnd
1 0 = 1604 ||       Data synchronization mark error - Recommend reassignment $as_fnd
1 0 = 1701 ||       Recovered data with retries               $as_fnd
1 0 = 1702 ||       Recovered data with positive head offset  $as_fnd
1 0 = 1703 ||       Recovered data with negative head offset  $as_fnd
1 0 = 1705 ||       Recovered data using previous sector ID   $as_fnd
1 0 = 1706 ||       Recovered data without ECC - Data auto-reallocated  $as_fnd
1 0 = 1707 ||       Recovered data without ECC - Recommend reassignment $as_fnd
1 0 = 1708 ||       Recovered data without ECC - Recommend rewrite      $as_fnd
1 0 = 1709 ||       Recovered data without ECC - Data rewritten $as_fnd
1 0 = 1801 ||       Data recovered with Error Correction and retries $as_fnd
1 0 = 1802 ||       Data recovered - Data auto-reallocated    $as_fnd
1 0 = 1805 ||       Data recovered - Recommed reassignment    $as_fnd
1 0 = 1806 ||       Data recovered with ECC - Recommend rewrite    $as_fnd
1 0 = 1807 ||       Data recovered with ECC - Data rewritten    $as_fnd
1 0 = 1902 ||       Defect list error in primary list         $as_fnd
1 0 = 1903 ||       Defect list error in grown list           $as_fnd
1 0 = 2000 ||       Invalid command operation code            $as_fnd
1 0 = 2100 ||       Logical block address out of range        $as_fnd
1 0 = 2400 ||       Invalid field in CDB                      $as_fnd
1 0 = 2500 ||       Logic unit number not supported           $as_fnd
1 0 = 2600 ||       Invalid field in parameter list           $as_fnd
1 0 = 2800 ||       Not ready to ready transition             $as_fnd
           |        --Medium may have changed. Unit formatted
1 0 = 2900 ||       Power on, Reset, or bus device reset occurred $as_fnd
1 0 = 2A01 ||       Mode parameters changed                   $as_fnd
1 0 = 2C00 ||       Command sequence error                    $as_fnd
1 0 = 2F00 ||       Commands cleared by another initiator     $as_fnd
1 0 = 3100 ||       Medium format corrupted, reassign failed  $as_fnd
1 0 = 3101 ||       Format command failed                     $as_fnd
1 0 = 3200 ||       No defect spare location available        $as_fnd
1 0 = 3201 ||       Defect list update failure                $as_fnd
1 0 = 3D00 ||       Invalid bits in indentify message         $as_fnd
1 0 = 3F01 ||       Microcode has been changed                $as_fnd
1 0 = 4080 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 4085 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 4090 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 40A0 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 40B0 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 40C0 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 40C5 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 40D0 ||       Diagnostic failure -- Sense details has further info  $as_fnd
1 0 = 4300 ||       Message error                             $as_fnd
~if (sense_key == 4)
  1 0 = 4400 ||       Internal target failure                   $as_fnd
~else
  1 0 = 4400 ||       Internal target failure                   $as_fnd
~endif
1 0 = 4C00 ||       Logic unit failured self-configuration    $as_fnd
           |        Direct-access device requires a code download
1 0 = 4500 ||       Select or reselect failure                $as_fnd
1 0 = 4700 ||       SCSI parity error                         $as_fnd
1 0 = 4800 ||       Initiator detected error message received $as_fnd
1 0 = 4900 ||       Invalid message error                     $as_fnd
1 0 = 4E00 ||       Overlapped commands attempted             $as_fnd
~if (sense_key == 1)
  1 0 = 5D00 ||       Reached recovered error threshold on predictive failure analysis      $as_fnd
~elseif (sense_key == 3)
  1 0 = 5D00 ||       Reached medium error threshold on predictive failure analysis      $as_fnd
~else
  1 0 = 5D00 ||       Reached error threshold on predictive failure analysis      $as_fnd
~endif
~if (! as_fnd)
  * Will try the SCSI II Bus standard
  1 0 = 0302 ||       Excessive write errors
  1 0 = 1000 ||       ID CRC or ECC error
  1 0 = 1102 ||       Error too long to correct
  1 0 = 1200 ||       ID Field Address Mark not found
  1 0 = 1300 ||       Data Address Mark not found
  1 0 = 1700 ||       Data recovered without ECC
  1 0 = 1900 ||       Defect list error
  1 0 = 1901 ||       Grown defect list not available
  1 0 = 1c00 ||       Defect list not found
  1 0 = 1c02 ||       Grown defect list not found
  1 0 = 2200 ||       Illegal function for device type
  1 0 = 2A00 ||       Mode Select parameters changed by another initiator
  1 0 = 3000 ||       Incompatible medium installed
  1 0 = 3001 ||       Can not read medium -- Unknown format
  1 0 = 3002 ||       Can not read medium -- Incompatible format
  1 0 = 4100 ||       Data Path failure
  1 0 = 4a00 ||       Command phase error
  1 0 = 4b00 ||       Data Phase error
~endif


1  2 @ 8     |    f) Field Replaceable Unit Code (0x%.2x)
~if (medium_error || recovered_error)
   1 2 = 00  |       Recommend Reassign
   1 2 = 11  |       Direct-Access Device Number 1
   1 2 = 12  |       Direct-Access Device Number 2
   1 2 = 13  |       Direct-Access Device Number 3
   1 2 = 14  |       Direct-Access Device Number 4
   1 2 = 15  |       Direct-Access Device Number 5
   1 2 = 16  |       Direct-Access Device Number 6
   1 2 = 17  |       Direct-Access Device Number 7
   1 2 = 18  |       Direct-Access Device Number 8
~elseif hardware_error
   1 2 = 00  |       None
   1 2 = 01  |       Fan #1, Op Panel Card, Op Panel Cable
   1 2 = 02  |       Fan #2, Op Panel Card, Op Panel Cable
   1 2 = 04  |       Op Panel Card
   1 2 = 05  |       Fan #1
   1 2 = 06  |       Fan #2
   1 2 = 0A  |       Power Module A
   1 2 = 0B  |       Power Module B
   1 2 = 0F  |       Host SCSI
   1 2 = 11  |       Direct-Access Device Number 1
   1 2 = 12  |       Direct-Access Device Number 2
   1 2 = 13  |       Direct-Access Device Number 3
   1 2 = 14  |       Direct-Access Device Number 4
   1 2 = 15  |       Direct-Access Device Number 5
   1 2 = 16  |       Direct-Access Device Number 6
   1 2 = 17  |       Direct-Access Device Number 7
   1 2 = 18  |       Direct-Access Device Number 8
   1 2 = 19  |       Direct-Access Device Number 1, Power to D-A-Dev 1
   1 2 = 1A  |       Direct-Access Device Number 2, Power to D-A-Dev 2
   1 2 = 1B  |       Direct-Access Device Number 3, Power to D-A-Dev 3
   1 2 = 1C  |       Direct-Access Device Number 4, Power to D-A-Dev 4
   1 2 = 1D  |       Direct-Access Device Number 5, Power to D-A-Dev 5
   1 2 = 1E  |       Direct-Access Device Number 6, Power to D-A-Dev 6
   1 2 = 1F  |       Direct-Access Device Number 7, Power to D-A-Dev 7
   1 2 = 20  |       Direct-Access Device Number 8, Power to D-A-Dev 8
   1 2 = 21  |       Direct-Access Device Number 1, D-A-Dev SCSI Cable
   1 2 = 22  |       Direct-Access Device Number 2, D-A-Dev SCSI Cable
   1 2 = 23  |       Direct-Access Device Number 3, D-A-Dev SCSI Cable
   1 2 = 24  |       Direct-Access Device Number 4, D-A-Dev SCSI Cable
   1 2 = 25  |       Direct-Access Device Number 5, D-A-Dev SCSI Cable
   1 2 = 26  |       Direct-Access Device Number 6, D-A-Dev SCSI Cable
   1 2 = 27  |       Direct-Access Device Number 7, D-A-Dev SCSI Cable
   1 2 = 28  |       Direct-Access Device Number 8, D-A-Dev SCSI Cable
   1 2 = 29  |       Direct-Access Device SCSI Cable 1 or 2
   1 2 = 2B  |       Direct-Access Device SCSI Cable 3 or 4
   1 2 = 2D  |       Direct-Access Device SCSI Cable 5 or 6
   1 2 = 2F  |       Direct-Access Device SCSI Cable 7 or 8
   1 2 = 31  |       Direct-Access Dev 1, Power to D-A-Dev 1, D-A-Dev SCSI Cable
   1 2 = 32  |       Direct-Access Dev 2, Power to D-A-Dev 2  D-A-Dev SCSI Cable
   1 2 = 33  |       Direct-Access Dev 3, Power to D-A-Dev 3  D-A-Dev SCSI Cable
   1 2 = 34  |       Direct-Access Dev 4, Power to D-A-Dev 4  D-A-Dev SCSI Cable
   1 2 = 35  |       Direct-Access Dev 5, Power to D-A-Dev 5  D-A-Dev SCSI Cable
   1 2 = 36  |       Direct-Access Dev 6, Power to D-A-Dev 6  D-A-Dev SCSI Cable
   1 2 = 37  |       Direct-Access Dev 7, Power to D-A-Dev 7  D-A-Dev SCSI Cable
   1 2 = 38  |       Direct-Access Dev 8, Power to D-A-Dev 8  D-A-Dev SCSI Cable
   1 2 = 40  |       Controller Electronics
   1 2 = 41  |       Fan 1, Controller Electronics, Op Panel Card, Op Panel Cable
   1 2 = 42  |       Fan 2, Controller Electronics, Op Panel Card, Op Panel Cable
   1 2 = 43  |       Both Fans,Controller Electronics, Op Panel Card, Op Panel Cable
   1 2 = 44  |       Controller Electronics, Op Panel Card, Op Panel Cable
   1 2 = 45  |       Fan 1, Controller Electronics
   1 2 = 46  |       Fan 2, Controller Electronics
   1 2 = 48  |       Op Panel communications to all power modules
   1 2 = 49  |       Op Panel communications control
   1 2 = 4A  |       Controller Electronics, Power Module A
   1 2 = 4B  |       Controller Electronics, Power Module B
   1 2 = 4D  |       High Availability Storage Subsystem
   1 2 = 4E  |       High Availability Storage Subsystem
   1 2 = 4F  |       Controller Electronics, Host SCSI, System Adapter Card
   1 2 = 51  |       Controller Electronics, Direct-Access Dev 1
   1 2 = 52  |       Controller Electronics, Direct-Access Dev 2
   1 2 = 53  |       Controller Electronics, Direct-Access Dev 3
   1 2 = 54  |       Controller Electronics, Direct-Access Dev 4
   1 2 = 55  |       Controller Electronics, Direct-Access Dev 5
   1 2 = 56  |       Controller Electronics, Direct-Access Dev 6
   1 2 = 57  |       Controller Electronics, Direct-Access Dev 7
   1 2 = 58  |       Controller Electronics, Direct-Access Dev 8
   1 2 = 59  |       Ctrl Electronics, Direct-Access Dev 1, Power to D-A-D 1
   1 2 = 5A  |       Ctrl Electronics, Direct-Access Dev 2, Power to D-A-D 2
   1 2 = 5B  |       Ctrl Electronics, Direct-Access Dev 3, Power to D-A-D 3
   1 2 = 5C  |       Ctrl Electronics, Direct-Access Dev 4, Power to D-A-D 4
   1 2 = 5D  |       Ctrl Electronics, Direct-Access Dev 5, Power to D-A-D 5
   1 2 = 5E  |       Ctrl Electronics, Direct-Access Dev 6, Power to D-A-D 6
   1 2 = 5F  |       Ctrl Electronics, Direct-Access Dev 7, Power to D-A-D 7
   1 2 = 60  |       Ctrl Electronics, Direct-Access Dev 8, Power to D-A-D 8
   1 2 = 61  |       Ctrl Electronics, Direct-Access Dev 1, D-A-D SCSI Cable
   1 2 = 62  |       Ctrl Electronics, Direct-Access Dev 2, D-A-D SCSI Cable
   1 2 = 63  |       Ctrl Electronics, Direct-Access Dev 3, D-A-D SCSI Cable
   1 2 = 64  |       Ctrl Electronics, Direct-Access Dev 4, D-A-D SCSI Cable
   1 2 = 65  |       Ctrl Electronics, Direct-Access Dev 5, D-A-D SCSI Cable
   1 2 = 66  |       Ctrl Electronics, Direct-Access Dev 6, D-A-D SCSI Cable
   1 2 = 67  |       Ctrl Electronics, Direct-Access Dev 7, D-A-D SCSI Cable
   1 2 = 68  |       Ctrl Electronics, Direct-Access Dev 8, D-A-D SCSI Cable
   1 2 = 69  |       Ctrl Electronics, Direct-Access Dev SCSI Cable 1 or 2
   1 2 = 6B  |       Ctrl Electronics, Direct-Access Dev SCSI Cable 3 or 4
   1 2 = 6D  |       Ctrl Electronics, Direct-Access Dev SCSI Cable 5 or 6
   1 2 = 6F  |       Ctrl Electronics, Direct-Access Dev SCSI Cable 7 or 8
   1 2 = 71  |       Ctrl Electr., Dir-Acc-Dev 1, Pwr to D-A-D 1, D-A-D SCSI Cable
   1 2 = 72  |       Ctrl Electr., Dir-Acc-Dev 2, Pwr to D-A-D 2, D-A-D SCSI Cable
   1 2 = 73  |       Ctrl Electr., Dir-Acc-Dev 3, Pwr to D-A-D 3, D-A-D SCSI Cable
   1 2 = 74  |       Ctrl Electr., Dir-Acc-Dev 4, Pwr to D-A-D 4, D-A-D SCSI Cable
   1 2 = 75  |       Ctrl Electr., Dir-Acc-Dev 5, Pwr to D-A-D 5, D-A-D SCSI Cable
   1 2 = 76  |       Ctrl Electr., Dir-Acc-Dev 6, Pwr to D-A-D 6, D-A-D SCSI Cable
   1 2 = 77  |       Ctrl Electr., Dir-Acc-Dev 7, Pwr to D-A-D 7, D-A-D SCSI Cable
   1 2 = 78  |       Ctrl Electr., Dir-Acc-Dev 8, Pwr to D-A-D 8, D-A-D SCSI Cable
   1 2 = 7E  |       High Availability Storage Subsystem
   1 2 = 7F  |       High Availability Storage Subsystem

~endif

1 3 @ 24  |    g) Sense Key Specific Information (0x%.6x)

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
%
1  6  @  16     | 7) Direct-Access Device Unit Reference Code (0x%.4x) ^dadurc
~if (dadurc == 0)
                %    No Direct access device error reported
~endif

1  8  @  16     | 8) Unit Reference Code (0x%.4x)

* Unit reference code has the format of kmpx
* Where k=category  m=modifier for category p=position of fru x=type of failure
1  8  @ @cat_mod
category := (cat_mod & 0xf0)
category := (category >> 4)
modifier := (cat_mod & 0x0f)
1  9  @ @pos_type
position := (pos_type & 0xf0)
position := (position >> 4)
type     := (pos_type & 0x0f)

1  8  =  0000   |    None
#set gen_err 0

category = 05   |    Direct-Access Device related error during restore $gen_err
category = 06   |    Direct-Access Device related error during resynchronization $gen_err
category = 07   |    Direct-Access Device related error $gen_err
                |    One Direct-Access Device on port
                |    Availability maintained during service
category = 08   |    Direct-Access Device related error $gen_err
                |    Two Direct-Access Devices on port but only 1 has error
                |    Availability maintained during service
~if (category == 0x09)
          ~if ((type <= 0x03) || (type >= 0x0a))
                %    Second Direct Access Device Failure        $gen_err
                %    Availability lost at failure
           ~else
                %    Device Location record (DLR) error         $gen_err
                %    Availability lost at failure
           ~endif
~endif
category = 0D   |    Direct-Access Device related error $gen_err
                |    One Direct-Access Devices on port or two, both have same err
                |    Availability not maintained during service
category = 0E   |    Direct-Access Device related error $gen_err
                |    Two Direct-Access Devices on port
                |    Availability not maintained during service
category = 0F   |    Host reporting only
~if (gen_err)
   modifier  =  01     |    0661 - Model 467 Disk Drive
   modifier  =  02     |    0663 - Model H08 Disk Drive
   modifier  =  03     |    0663 - Model H12 Disk Drive
   modifier  =  04     |    0663 - Model L08 Disk Drive
   modifier  =  05     |    0663 - Model L12 Disk Drive
   modifier  =  06     |    0664 - Model M1H Disk Drive
   modifier  =  07     |    0662 - Model S08 Disk Drive
   modifier  =  08     |    0662 - Model S12 Disk Drive
   modifier  =  09     |    DFHS Any Model Disk Drive
   modifier  =  0E     |    Other IBM Direct-Access Device type
   modifier  =  0F     |    Non-IBM or newly installed broken Direct-Access Device
   ~if (position > 0)
                       %    Position %d ~position
   ~endif
   type      =  00     |    Direct-Access Device reported fault
   type      =  01     |    SCSI-Parity Error
   type      =  02     |    Communications Error
   type      =  03     |    Direct-Access device error
   type      =  05     |    Direct-Access device miscompare during BATS
   type      =  06     |    Initialize required
   type      =  07     |    Missing Direct-Access Device at BATS
   type      =  08     |    Direct-Access device broken, had hot spare
   type      =  09     |    Invalid Direct-Access-Device replacement
   type      =  0A     |    Threshold hardware errors requiring FRU replacement
   type      =  0B     |    Threshold rellocates requiring FRU replacement
   type      =  0C     |    Command not supported
   type      =  0D     |    Additional Direct-Access device required for this cmd
   type      =  0E     |    Direct access device has insufficient memory to incorporate
   type      =  0F     |    LRC error
~endif
category = 0B   |    Bringup Hang State
cat_mod  = BB   |    Op Panel reporting only
~if (cat_mod == 0xC0)
                       %    Host Recover Required
        type    = 00   |    Direct-Access Device requests reallocation
        type    = 01   |    Direct-Access Device reassign required during reallocate
        type    = 0C   |    Direct-Access Device requires a code download
       ~if (position != 0x0D)
                       %    Direct-Access Device is %d ~position
       ~endif
       pos_type = DC   |    Controller Electronics requests code download
~elseif (cat_mod == 0xC9)
                       %    Controller or Op Panel detected error
        pos_type = 00  |    Op panel internal failure
        pos_type = 01  |    Fan #1 failed
        pos_type = 02  |    Fan #2 failed
        pos_type = 03  |    Both fans failed
        pos_type = 08  |    Communications failure with all power modules
        pos_type = 09  |    Communications failure with all controllers
        pos_type = 0A  |    Communications failure with power module A
        pos_type = 0B  |    Communications failure with power module B
        pos_type = 0D  |    Op panel detected controller electronics D timeout
        pos_type = 0F  |    Communications failure with SPCN
        pos_type = 90  |    Multiple power module failure
                       |    availability lost
        pos_type = 91  |    +5V overcurrent detected; availability lost
        pos_type = 92  |    +12V overcurrent detected; availability lost
        pos_type = A0  |    Power Supply A failure; concurrently service
        pos_type = A1  |    +5V overcurrent detected by module A only; concurrently service
        pos_type = A2  |    +12V overcurrent detected by module A only; concurrently service
        pos_type = A3  |    Single power envelope failed; concurrently service
        pos_type = B0  |    Power Module B failed; concurrently service
        pos_type = B1  |    +5V overcurrent detected by module B only; concurrently service
        pos_type = B2  |    +12V overcurrent detected by module B only; concurrently service
        pos_type = B3  |    Single power envelope failed; concurrently service
        pos_type = D0  |    Communications error to Op panel
        pos_type = D1  |    Control store error
        pos_type = D2  |    Data store error
        pos_type = D3  |    XOR error
        pos_type = D4  |    Controller card failure; write cache has no data (1 MB)
        pos_type = D5  |    XOR timeout (microcode detected)
        pos_type = D6  |    Microcode sanity error
        pos_type = D8  |    LCR error; controller card
        pos_type = DA  |    1960 bringup error in BATS
        pos_type = DB  |    Configuration error in BATS
        pos_type = DC  |    HOST SCSI chip error in BATS
        pos_type = DD  |    Direct-Access Degice SCSI chip error in BATS
        pos_type = DE  |    NVRAM error
        pos_type = DF  |    Host communications error; controller card or host cable
        pos_type = E0  |    Ctlr card and daughter card error, data lost occurred (1 MB)
        pos_type = E1  |    Ctlr card failure, write cache contains data (1 MB)
        pos_type = E2  |    Daughter card error, data loss occurred (1 MB)
        pos_type = E4  |    Daughter card error, no data loss occurred (1 MB)
        pos_type = E8  |    Ctlr card and daughter card error, data lost occurred (4 MB)
        pos_type = E9  |    Ctlr card failure, write cache contains data (4 MB)
        pos_type = EA  |    Daughter card error, data loss occurred (4 MB)
        pos_type = EB  |    Ctlr card failure, write cache contains no data (4 MB)
        pos_type = EC  |    Daughter card error, no data loss occurred (4 MB)
        pos_type = EF  |    Host or controller card error, daughter card contains data
~elseif (cat_mod == 0xCE)
        pos_type = 00  |    All configured Direct-Access Devices correctly installed
                       |    Extra Direct-Access Devices present
        pos_type = 10  |    Some configured Direct-Access Devices incorrectly installed
                       |    All configured Direct-Access-Devices are present
        pos_type = 12  |    Some configured Direct-Access Devices incorrectly installed
                       |    Direct-Access-Devices may be missing, new D-A Devices may be
                       |    present or extra ones may be present
        pos_type = 27  |    More than 1 Direct-Access Device is missing
                       |    High Availability Storage Subsystem is in protect mode
        pos_type = 29  |    More than 1 new Direct-Access Device is present
                       |    High Availability Storage Subsystem is in protect mode
        pos_type = 2D  |    No correct DLR can be determined
        pos_type = EB  |    Replacement daughter card is of incorrect capacity
        pos_type = EC  |    Daughter card replaced, both old and new cards contain data
        pos_type = ED  |    Daughter card replaced, new card has data from another box
        pos_type = EE  |    Daughter card replaced, need old one to prevent data loss
~elseif (cat_mod == 0xff)
        pos_type = 00  |    System tracer record missing during a change to protected mode
        pos_type = 01  |    Write cache error
                       |    Change protect mode cmnd in progress checkpoint missing
~endif
                % 9) Direct Access Device Sense Data (in hex)
1 10  @     |    Byte  0=%.2x \
1 11 @      |    Byte  2=%.2x \
1 12 @      |    Byte  3=%.2x \
1 13 @      |    Byte  4=%.2x
1 14 @      |    Byte  5=%.2x \
1 15 @      |    Byte  6=%.2x \
1 16 @      |    Byte 12=%.2x \
1 17 @      |    Byte 13=%.2x
1 18 @      |    Byte 17=%.2x

