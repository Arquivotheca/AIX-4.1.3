	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  ORIGINS: 27
;
;  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
;  combined with the aggregated modules for this product)
;                   SOURCE MATERIALS
;  (C) COPYRIGHT International Business Machines Corp. 1988, 1989
;  All Rights Reserved
;
;  US Government Users Restricted Rights - Use, duplication or
;  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;
;  FUNCTION: 
;
;**********************************************************************
;
;	"@(#)68	1.3  src/bos/usr/lib/asw/mpqp/cio.inc, ucodmpqp, bos411, 9428A410j 1/30/91 17:07:15"
;
CIO_0	EQU	180h		; CIO 0 Base Address
CIO_0A	EQU	1C0h		; CIO 0 Port A Register Base
CIO_0B	EQU	1D0h		; CIO 0 Port B Register Base

CIO_1	EQU	500h		; CIO 1 Base Address
CIO_1A	EQU	540h		; CIO 1 Port A Register Base
CIO_1B	EQU	550h		; CIO 1 Port B Register Base

SCCREL_C0 EQU	3224h		; CIO 0 Release Interrupt Register
SCCREL_C1 EQU	3225h		; CIO 1 Release Interrupt Register

MIC	EQU	0		; Master Interrupt Control Register
MCC	EQU	2		; Master Configuration Control Register
P_A_IV	EQU	4		; Port A Interrupt Vector
P_B_IV	EQU	6		; Port B Interrupt Vector
CT_IV	EQU	8		; Counter/Timer Interrupt Vector
P_C_DPP	EQU	10		; Port C Data Path Polarity Register
P_C_DD	EQU	12		; Port C Data Direction Register
P_C_SC	EQU	14		; Port C Special I/O Control Register
P_A_CS	EQU	16		; Port A Command/Status Register
P_B_CS	EQU	18		; Port B Command/Status Register
T_1_CS	EQU	20		; Counter/Timer 1 Command/Status Register
T_2_CS	EQU	22		; Counter/Timer 2 Command/Status Register
T_3_CS	EQU	24		; Counter/Timer 3 Command/Status Register
P_A_DATA EQU	26		; Port A Data Register
P_B_DATA EQU	28		; Port B Data Register
P_C_DATA EQU	30		; Port C Data Register

T_1_CCM	EQU	32		; Counter/Timer 1 Current Count, MSB
T_1_CCL	EQU	34		; Counter/Timer 1 Current Count, LSB
T_2_CCM	EQU	36		; Counter/Timer 2 Current Count, MSB
T_2_CCL	EQU	38		; Counter/Timer 2 Current Count, LSB
T_3_CCM	EQU	40		; Counter/Timer 3 Current Count, MSB
T_3_CCL	EQU	42		; Counter/Timer 3 Current Count, LSB
T_1_TCM	EQU	44		; Counter/Timer 1 Time Constant, MSB
T_1_TCL	EQU	46		; Counter/Timer 1 Time Constant, LSB
T_2_TCM	EQU	48		; Counter/Timer 2 Time Constant, MSB
T_2_TCL	EQU	50		; Counter/Timer 2 Time Constant, LSB
T_3_TCM	EQU	52		; Counter/Timer 3 Time Constant, MSB
T_3_TCL	EQU	54		; Counter/Timer 3 Time Constant, LSB
T_1_MS	EQU	56		; Counter/Timer 1 Mode Specification
T_2_MS	EQU	58		; Counter/Timer 2 Mode Specification
T_3_MS	EQU	60		; Counter/Timer 3 Mode Specification
CUR_VEC	EQU	62		; Current Vector

CIOCH_A	EQU	64		; Channel A Register Offset
CIOCH_B	EQU	80		; Channel B Register Offset

P_MODE	EQU	0		; Mode Specification
P_SHAKE	EQU	2		; Handshaking Specification, Not Used
P_DPPOL	EQU	4		; Data Path Polarity
P_DDIR	EQU	6		; Data Direction
P_SPCTL	EQU	8		; Special I/O Control
P_PPOL	EQU	10		; Pattern Polarity
P_PTRAN	EQU	12		; Pattern Transition
P_PMASK	EQU	14		; Pattern Mask

; Bit definitions, Master Interrupt Control				052289

MIC_MIE		EQU	80h	; Master Interrupt Enable, entire CIO
MIC_DLC		EQU	40h	; Disable Lower Chain
MIC_NV		EQU	20h	; CIO Provides no vector during IACK
MIC_VIS_A	EQU	10h	; Port A Interrupt Vector Includes Status
MIC_VIS_B	EQU	8h	; Port A Interrupt Vector Includes Status
MIC_VIS_C	EQU	4h	; C/T Interrupt Vector Includes Status
MIC_RJA		EQU	2h	; Right Justified Address
MIC_RESET	EQU	1h	; Reset

; Bit definitions, Master Configuration Control

MCC_B_EN	EQU	80h	; Port B Enable
MCC_1_EN	EQU	40h	; Timer 1 Enable
MCC_2_EN	EQU	20h	; Timer 2 Enable
MCC_C3_EN	EQU	10h	; Timer 3/Port C Enable
MCC_PLC		EQU	8h	; Port Link Control (0=Independent)
MCC_A_EN	EQU	4h	; Port A Enable
MCC_1O2I	EQU	3h	; Timer 1 Output is Timer 2 Input
MCC_1O2T	EQU	2h	; Timer 1 Output is Timer 2 Trigger
MCC_1O2G	EQU	1h	; Timer 1 Output is Timer 2 Gate
MCC_12INDEP	EQU	0h	; Timers 1 and 2 are independent

; Bit Definitions, Counter/Timer Mode Specification Registers

CIO_CM_CONT	EQU	80h	; CONTINUOUS/single cycle (Stop on term. cnt.)
CIO_CM_EOE	EQU	40h	; ENABLE/disable External Output of the timer
CIO_CM_ECE	EQU	20h	; ENABLE/disable External Count (1=Counter)
CIO_CM_ETE	EQU	10h	; ENABLE/disable External Trigger
CIO_CM_EGE	EQU	08h	; ENABLE/disable External Gate
CIO_CM_RTE	EQU	04h	; ENABLE/disable Int/Ext Trigger Restart
				; Select *one* of the output duty cycles below
CIO_CM_SQU	EQU	02h	; Square Wave Output
CIO_CM_ONES	EQU	01h	; One-Shot Output
CIO_CM_PULSE	EQU	00h	; Pulse Output

; Bit Definitions, Counter/Timer Command and Status Registers

CIO_TCS_IUS	EQU	80h	; Interrupt Under Service
CIO_TCS_IE	EQU	40h	; ENABLE/disable C/T Interrupt
CIO_TCS_IP	EQU	20h	; Interrupt Pending
CIO_TCS_ERR	EQU	10h	; Interrupt Error
CIO_TCS_RCC	EQU	08h	; Read Counter Control
CIO_TCS_GC	EQU	04h	; RUN/halt the countdown sequence
CIO_TCS_TC	EQU	02h	; Trigger Timer (Reset w/time constant) [W]
CIO_TCS_CIP	EQU	01h	; Count In Progress [R]

CIO_TCS_CLRIE	EQU	0E0h	; Clear IE Bit
CIO_TCS_SETIE	EQU	0C0h	; Set IE Bit
CIO_TCS_CLRIP	EQU	0A0h	; Clear IP Bit
CIO_TCS_SETIP	EQU	 80h	; Set IP Bit
CIO_TCS_CLRIUS	EQU	 60h	; Clear IUS Bit
CIO_TCS_SETIUS	EQU	 40h	; Set IUS Bit
CIO_TCS_CBOTH	EQU	 20h	; Clear IP and IUS at once

; Time constants and other bit jobs

CIO_T_033	EQU	0D8h	; Timer period (MSB) 0.033S = 3 per 0.1S
CIO_T_020	EQU	090h	; Timer period (MSB) 0.02S = 5 per 0.1S

; Port register bit definitions.  These are generic for all 4 ports.

CIO_S_DTR	EQU	 01h	; Set DTR (data terminal ready)
CIO_C_DTR	EQU	0FEh	; Clear DTR
CIO_S_HRS	EQU	 02h	; Set HRS (half rate select)
CIO_C_HRS	EQU	0FDh	; Clear DTR
CIO_S_232	EQU	 04h	; Set RS232 (Port is not RS232)
CIO_C_232	EQU	0FBh	; Clear RS232 (Port is RS232)
CIO_S_BLK	EQU	 04h	; Set blocking for T and C
CIO_C_BLK	EQU	0FBh	; Clear blocking for T and C
CIO_S_T232	EQU	 08h	; Set RS232 Timing Select (DTE Clocking)
CIO_C_T232	EQU	0F7h	; Clear RS232 Timing Select (BMC Clocking)
CIO_S_V35	EQU	 10h	; Set V.35 (Port is not V.35)
CIO_C_V35	EQU	0EFh	; Clear V.35 (Port is V.35)
CIO_S_422	EQU	 20h	; Set RS422 (Port is RS422)
CIO_C_422	EQU	0DFh	; Clear RS422 (Port is not RS422)
CIO_Q_DSR	EQU	 40h	; Check DSR (Data Set Ready)
CIO_Q_RI	EQU	 80h	; Check RI (Ring Indicate)

; Special extension bits in CIO1 Port C and the Enable register

CIO_X21_00	EQU	0h	; X21 Pattern Detection Logic detected Zeroes
CIO_X21_11	EQU	3h	; X21 Pattern Detection Logic detected Ones
CIO_X21_01	EQU	1h	; Pattern was 01...
CIO_X21_10	EQU	2h	; Pattern was 10...

CIO_X21_IA	EQU	8h	; X21 Network INDICATE line

ENR_S_X21_CA	EQU	 01h	; X21 CONTROL line, Drive HIGH
ENR_C_X21_CA	EQU	0FEh	; X21 CONTROL line, Drive LOW
ENR_S_X21	EQU	 02h	; X21 Driver Control (Port is not X21)
ENR_C_X21	EQU	0FDh	; X21 Driver Control (Port is X21)

; Port Mode Specification Control Register fields

PMS_BIT		EQU	 3Fh	; Port is a Bit Port.  Not an OR mask
PMS_BI		EQU	0C0h	; Port is bidirectional   (handshaking only)
PMS_OUTPUT	EQU	 80h	; Port is an OUTPUT port  (handshaking only)
PMS_INPUT	EQU	 40h	; Port is an INPUT port   (handshaking only)
PMS_ITB		EQU	 20h	; Interrupt on Two Bytes  (handshaking only)
PMS_SB		EQU	 10h	; Single Buffered Mode    (handshaking only)
PMS_IMO		EQU	  8h	; Interrupt on Match Only (handshaking only)
PMS_NOPM	EQU	  6h	; Disable Pattern Matching.  Not an OR mask
PMS_OR_PRI	EQU	  6h	; OR-Priority Encoded Vector Mode
PMS_OR		EQU	  4h	; OR Pattern Match Mode
PMS_AND		EQU	  2h	; AND Pattern Match Mode
PMS_LPM		EQU	  1h	; Latch on Pattern Match

PMS_INIT	EQU	PMS_OR+PMS_LPM

; Port Handshaking Specification Register fields

PHS_INIT	EQU	0	; Handshaking is not used on the adapter

; Bit Definitions, Port Command and Status Registers

CIO_PCS_IUS	EQU	80h	; Interrupt Under Service
CIO_PCS_IE	EQU	40h	; ENABLE/disable C/T Interrupt
CIO_PCS_IP	EQU	20h	; Interrupt Pending
CIO_PCS_ERR	EQU	10h	; Interrupt Error
CIO_PCS_ORE	EQU	08h	; Output Register Empty
CIO_PCS_IRF	EQU	04h	; Input Register Full
CIO_PCS_PM	EQU	02h	; Pattern Match flag
CIO_PCS_IOE	EQU	01h	; Interrupt On Error

CIO_PCS_CLRIE	EQU	0E0h	; Clear IE Bit
CIO_PCS_SETIE	EQU	0C0h	; Set IE Bit
CIO_PCS_CLRIP	EQU	0A0h	; Clear IP Bit
CIO_PCS_SETIP	EQU	 80h	; Set IP Bit
CIO_PCS_CLRIUS	EQU	 60h	; Clear IUS Bit
CIO_PCS_SETIUS	EQU	 40h	; Set IUS Bit
CIO_PCS_CBOTH	EQU	 20h	; Clear IP and IUS at once

