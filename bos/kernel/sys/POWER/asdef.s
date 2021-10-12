# @(#)28	1.24  src/bos/kernel/sys/POWER/asdef.s, cmdas, bos411, 9428A410j 11/2/92 15:38:02
#
# COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
#
# FUNCTIONS: 
#
# ORIGINS: 3, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# XCOFF version
divert(99)
# define the CPU registers

        define(r0,0)
        define(r1,1)
        define(r2,2)
        define(r3,3)
        define(r4,4)
        define(r5,5)
        define(r6,6)
        define(r7,7)
        define(r8,8)
        define(r9,9)
        define(r10,10)
        define(r11,11)
        define(r12,12)
        define(r13,13)
        define(r14,14)
        define(r15,15)
        define(r16,16)
        define(r17,17)
        define(r18,18)
        define(r19,19)
        define(r20,20)
        define(r21,21)
        define(r22,22)
        define(r23,23)
        define(r24,24)
        define(r25,25)
        define(r26,26)
        define(r27,27)
        define(r28,28)
        define(r29,29)
        define(r30,30)
        define(r31,31)

#**********************************************************************
#                                                                     *
#   commonly used equates                                             *
#                                                                     *
#**********************************************************************

        define(false,0x04)      # branch false bo
        define(true,0x0c)       # branch true bo
        define(falsectr,0x00)   # dec ctr, branch false and ctr != 0 bo
        define(falzezct,0x02)   # dec ctr, branch false and ctr  = 0 bo
        define(truectr,0x08)    # dec ctr, branch true  and ctr != 0 bo
        define(truezctr,0x0a)   # dec ctr, branch true  and ctr  = 0 bo
        define(always,0x14)     # branch unconditional bo
        define(brctr,0x10)      # dec ctr, branch ctr != 0 bo
        define(brzctr,0x12)     # dec ctr, branch ctr  = 0 bo
        define(lt,0x00)         # less than condition (bit 0)
        define(gt,0x01)         # greater than condition (bit 1)
        define(eq,0x02)         # equal condition (bit 2 from 0)
        define(so,0x03)         # so bit in cr (bit 3)
        define(nolk,0x00)       # no link
        define(lk,0x01)         # link
        define(mq,0x00)         # mq register
        define(xer,0x01)        # fixed point exception register
        define(lr,0x08)         # link register
        define(ctr,0x09)        # count register
        define(tid,0x11)        # tid
        define(dsisr,0x12)      # dsisr
        define(dar,0x13)        # dar
        define(sdr0,0x18)       # storage descriptor pft
        define(sdr1,0x19)       # storage descriptor hat
        define(srr0,0x1a)       # save/restore register 0
        define(srr1,0x1b)       # save/restore register 1

#**********************************************************************
#                                                                     *
#   equates for msr bits                                              *
#                                                                     *
#**********************************************************************

        define(ee,0x8000)       # external interrupt
        define(pr,0x4000)       # problem state
        define(fp,0x2000)       # floating point available
        define(me,0x1000)       # machine check
        define(fe,0x0800)       # floating  point exception enable
        define(al,0x0080)       # alignment check
        define(ip,0x0040)       # prefix
        define(ir,0x0020)       # instruction relocate
        define(dr,0x0010)       # data relocate
        define(idr,0x0030)      # instruction and data relocate
        define(allon,0x00f0)    # all on
        define(off,0x0000)      # mask to set msr off
        define(dis,0x7fff)      # mask to disable


#**********************************************************************
#                                                                     *
#   equates for floating regs                                         *
#                                                                     *
#**********************************************************************

	define(fr0,0)
	define(fr1,1)
	define(fr2,2)
	define(fr3,3)
	define(fr4,4)
	define(fr5,5)
	define(fr6,6)
	define(fr7,7)
	define(fr8,8)
	define(fr9,9)
	define(fr10,10)
	define(fr11,11)
	define(fr12,12)
	define(fr13,13)
	define(fr14,14)
	define(fr15,15)
	define(fr16,16)
	define(fr17,17)
	define(fr18,18)
	define(fr19,19)
	define(fr20,20)
	define(fr21,21)
	define(fr22,22)
	define(fr23,23)
	define(fr24,24)
	define(fr25,25)
	define(fr26,26)
	define(fr27,27)
	define(fr28,28)
	define(fr29,29)
	define(fr30,30)
	define(fr31,31)

#for now, copy in defines of f form from asdeft.s
	define(f0,0)
	define(f1,1)
	define(f2,2)
	define(f3,3)
	define(f4,4)
	define(f5,5)
	define(f6,6)
	define(f7,7)
	define(f8,8)
	define(f9,9)
	define(f10,10)
	define(f11,11)
	define(f12,12)
	define(f13,13)
	define(f14,14)
	define(f15,15)
	define(f16,16)
	define(f17,17)
	define(f18,18)
	define(f19,19)
	define(f20,20)
	define(f21,21)
	define(f22,22)
	define(f23,23)
	define(f24,24)
	define(f25,25)
	define(f26,26)
	define(f27,27)
	define(f28,28)
	define(f29,29)
	define(f30,30)
	define(f31,31)

#**********************************************************************
#                                                                     *
#   equates for floating point exception status bits                  *
#                                                                     *
#**********************************************************************

	define(fpxinvalid,0x20000000)
	define(fpxoverflo,0x10000000)
	define(fpxunderfl,0x08000000)
	define(fpxzdiv,   0x04000000)
	define(fpxinex,   0x02000000)
	define(fpxnvsnan, 0x01000000)
	define(fpxnvisi,  0x00800000)
	define(fpxnvidi,  0x00400000)
	define(fpxnvzdz,  0x00200000)
	define(fpxnvimz,  0x00100000)
	define(fpxnvcomp, 0x00080000)
	define(fpxnvrmz,  0x00000800)
	define(fpxnvrmi,  0x00000400)
	define(fpxnvsqr,  0x00000200)
	define(fpxnvcvi,  0x00000100)

#**********************************************************************
#                                                                     *
#   equates for segment regs                                          *
#                                                                     *
#**********************************************************************

        define(sr0,0)
        define(sr1,1)
        define(sr2,2)
        define(sr3,3)
        define(sr4,4)
        define(sr5,5)
        define(sr6,6)
        define(sr7,7)
        define(sr8,8)
        define(sr9,9)
        define(sr10,10)
        define(sr11,11)
        define(sr12,12)
        define(sr13,13)
        define(sr14,14)
        define(sr15,15)

#**********************************************************************
#                                                                     *
#   equates for condition register bit fields			      *
#                                                                     *
#**********************************************************************

        define(cr0,0)
        define(cr1,1)
        define(cr2,2)
        define(cr3,3)
        define(cr4,4)
        define(cr5,5)
        define(cr6,6)
        define(cr7,7)


#**********************************************************************
#                                                                     *
#   equates for condition register bit sub-fields                     *
#                                                                     *
#**********************************************************************

	define(cr0_0,0)
	define(cr0_1,1)
	define(cr0_2,2)
	define(cr0_3,3)

	define(cr1_0,4)
	define(cr1_1,5)
	define(cr1_2,6)
	define(cr1_3,7)

	define(cr2_0,8)
	define(cr2_1,9)
	define(cr2_2,10)
	define(cr2_3,11)

	define(cr3_0,12)
	define(cr3_1,13)
	define(cr3_2,14)
	define(cr3_3,15)

	define(cr4_0,16)
	define(cr4_1,17)
	define(cr4_2,18)
	define(cr4_3,19)

	define(cr5_0,20)
	define(cr5_1,21)
	define(cr5_2,22)
	define(cr5_3,23)

	define(cr6_0,24)
	define(cr6_1,25)
	define(cr6_2,26)
	define(cr6_3,27)

	define(cr7_0,28)
	define(cr7_1,29)
	define(cr7_2,30)
	define(cr7_3,31)


#
# Format: ENTRY(label_name)
#   Where:
#       label_name - is the label to be referenced
# This macro generates the proper naming convention for a code label
#
define( ENTRY,`.$1')



#
# Format: DATA(label_name)
#   Where:
#       label_name - is the label
# This macro generates the proper naming convention for a data label
#
define( DATA,`$1')

#
#	_DF defines the traceback table for R2 dependent assembly programs.
#	It uses the assembler pseudo-op .tbtag, which defines
#	the beginning of the traceback table. 
#
#	Following is the traceback table structure as defined in the
#	sys/debug.h. The seven arguments of the _DF macros are shown below:
#
####### First argument is the bitwise or of the following 8 bitfield,
#
#	unsigned globallink:1	Set if routine is global linkage
#	unsigned is_eprol:1	Set if is out-of-line epilog/prologue
#	unsinged has_tboff	Set if offset from start of proc stored
#	unsigned int_proc:1;	Set if routine is internal
#	unsigned has_ctl:1;	Set if routine involves controlled storage
#	unsigned tocless:1;	Set if routine contains no TOC
#	unsigned fp_present:1;  Set if routine performs FP operations
#	unsigned log_abort:1;   Set if routine logs or aborts FP ops
#
######  Second argument is the bitwise or of the following birfields.
#
#	unsigned int_hndl:1;    Set if routine is interrupt handler
#	unsigned name_present:1;Set if name is present in proc table
#	unsigned uses_alloca:1; Set if alloca used to allocate storage
#	unsigned cl_dis_inv:3;  On-condition directives, see below
#	unsigned saves_cr:1;    Set if procedure saves the condition reg
#	unsigned saves_lr:1;    Set if procedure saves the link reg
#
###### Third argument is the bitwise or of the following bitfields.
#
#	unsigned stores_bc:1;   Set if procedure saves the link reg
#	unsigned spare2:1;      Spare bit
#	unsigned fpr_saved:6;   Number of FPRs saved, max of 32
#
###### Fourth argument is the bitwise or of the following bitfields.
#
#	unsigned spare3:2;      Spare bits
#	unsigned gpr_saved:6;   Number of GPRs saved, max of 32
#
###### Fifth argument is the bitwise or of the following bitfields.
#
#	unsigned fixedparms:8;  Number of fixed point parameters
#
###### Sixth argument is the bitwise or of the following bitfields.
#
#	unsigned floatparms:7;  Number of floating point parameters
#	unsigned parmsonstk:1;  Set if all parameters placed on stack
#
###### Seventh argument is the bitwise or of the following bitfields.
#
#	unsigned int parminfo;  Order and type encoding of parameters:
#				Left-justified bit-encoding as follows:
#				'0'  ==> fixed parameter
#				'10' ==> single-precision float parameter
#				'11' ==> double-precision float parameter
#
define(_DF, `.align 2
	.tbtag 0x0,0xc,$1,$2,$3,$4,$5,$6,$7,0x0,0x0,0x0,0x0,0x0,0x0,0x0')

#
# Secial types of stack frames:
#	_DF_NOFRAME - no stack frame, parameters in registers
#	_DF_START - first stack frame
#	_DF_GLUE - the routine is a global linkage routine
#
define( _DF_NOFRAME,    `0x0,0x0,0x0,0x0,0x0,0x0,0x0')dnl
define( _DF_START,      `0x0,0x0,0x0,0x0,0x0,0x0,0x0')dnl
define( _DF_GLUE,	`0x8,0x0,0x0,0x0,0x0,0x0,0x0')dnl


#
# S_PROLOG(name) - prolog for a simple routine which
#                   does not call or modify r13-r31
define( S_PROLOG,
        `.csect ENTRY($1[PR])
	 .globl ENTRY($1[PR])')

# FCNDES(name,[label]) - function descriptor for a routine.
#               Where: label - keyword indicates LD entry vs SD of type PR
# .toc is to make sure there is at least on in program so TOC[t0] works
define( FCNDES,
       `.toc
        .csect  $1[DS]
        .globl  $1[DS]
ifelse($2x,labelx,
`       .long   ENTRY($1)',
`       .long   ENTRY($1[PR])')
        .long   TOC[t0]')

# S_EPILOG - epilog for S_PRLOG

define( S_EPILOG,
       `br
        _DF(_DF_NOFRAME)')


#
#  PROLOG(fname,GPR, FPR, Framesize)
#
#	fname: name of the function
#	GPR  : the least number of the GPR, (13-32) 32 = no GPRs saved
#	FPR  : the least number of the FPR, (14-32) 32 = no FPRs saved
#	Framesize: the size of the stack to hold the local variables and
#		   function parameters.
#
#	Example:
#	PROLOG(hello,20,17,100)
#
#	Here, hello is the name of the function,
#	this prolog will save GPRs 20 through 31, FPRs 17 through 31,
#	get a local stack area of 100 bytes
#	and will modify the stack size accordingly.
#
define( PROLOG,
       `.csect  ENTRY($1[PR])
	.globl	ENTRY($1[PR])
        define(`SAVEgpr',`eval(32-$2)')dnl
	define(`SAVEfpr',`eval(32-$3)')
        define(`SAVEstk',`eval(56+4*SAVEgpr+8*SAVEfpr+$4)')
	ifelse(eval(SAVEstk&7),0,,`define(`SAVEstk',
				eval(SAVEstk+8-eval(SAVEstk&7)))')
	define(`SAVEfname',$1)

	mflr	0
	ifelse($3,32,,`b ._sfpr.$1.$3
	SAVEFPR($1)')
	ifelse($2,32,,`stm $2, eval(-8*(SAVEfpr)-4*(SAVEgpr))(1)')
	st	0,8(1)
	stu 	r1, -SAVEstk`'(1)
')

#
#
# EPILOG(fixed params, float params, type info of the parameters)
#
#   	fixed params:  total number of fixed point parameters,
#	float params:  total number of floating point parameters,
#	type	    :  type of each parameter (see linkage convention)
#		       this field is only used for debugger stack trace
#		       back info
#
define( EPILOG,
        `
	l	r0,SAVEstk+8(1)
	cal	1,SAVEstk`'(1)
	mtlr	0
	ifelse(SAVEgpr,0,,
		`lm eval(32-SAVEgpr), eval(-8*(SAVEfpr)-4*(SAVEgpr))(1)')
	define(`SaveFpr',`eval(32-SAVEfpr)')
	ifelse(SaveFpr,32,,`b ._rfpr.SAVEfname.SaveFpr
	RESTOREFPR(SAVEfname)
	')
        br
        _DF(0x0,0x1,0x8|SAVEfpr,SAVEgpr,$1,$2,$3)
	.short	eval(8*(SAVEfpr)+4*(SAVEgpr))
')

#
define(SAVEFPR, `
._sfpr.$1.14:   stfd  14, -144(1)
._sfpr.$1.15:   stfd  15, -136(1)
._sfpr.$1.16:   stfd  16, -128(1)
._sfpr.$1.17:   stfd  17, -120(1)
._sfpr.$1.18:   stfd  18, -112(1)
._sfpr.$1.19:   stfd  19, -104(1)
._sfpr.$1.20:   stfd  20, -96(1)
._sfpr.$1.21:   stfd  21, -88(1)
._sfpr.$1.22:   stfd  22, -80(1)
._sfpr.$1.23:   stfd  23, -72(1)
._sfpr.$1.24:   stfd  24, -64(1)
._sfpr.$1.25:   stfd  25, -56(1)
._sfpr.$1.26:   stfd  26, -48(1)
._sfpr.$1.27:   stfd  27, -40(1)
._sfpr.$1.28:   stfd  28, -32(1)
._sfpr.$1.29:   stfd  29, -24(1)
._sfpr.$1.30:   stfd  30, -16(1)
._sfpr.$1.31:   stfd  31, -8(1)
')
#
#	RESTOREFPR retrieves the floating point registers from the stack.
#
define(RESTOREFPR, `
._rfpr.$1.14:   lfd  14, -144(1)
._rfpr.$1.15:   lfd  15, -136(1)
._rfpr.$1.16:   lfd  16, -128(1)
._rfpr.$1.17:   lfd  17, -120(1)
._rfpr.$1.18:   lfd  18, -112(1)
._rfpr.$1.19:   lfd  19, -104(1)
._rfpr.$1.20:   lfd  20, -96(1)
._rfpr.$1.21:   lfd  21, -88(1)
._rfpr.$1.22:   lfd  22, -80(1)
._rfpr.$1.23:   lfd  23, -72(1)
._rfpr.$1.24:   lfd  24, -64(1)
._rfpr.$1.25:   lfd  25, -56(1)
._rfpr.$1.26:   lfd  26, -48(1)
._rfpr.$1.27:   lfd  27, -40(1)
._rfpr.$1.28:   lfd  28, -32(1)
._rfpr.$1.29:   lfd  29, -24(1)
._rfpr.$1.30:   lfd  30, -16(1)
._rfpr.$1.31:   lfd  31, -8(1)
')


#	Macros to set and clear a single bit in a register,
#	putting the result in any GPR.

define( SETBIT,`rlinm	$1,$2,0,$3,$3')
define( CLRBIT,`rlinm	$1,$2,0,$3+1,$3-1')

# TRAP()

define( TRAP,
       `teq     1,1')

# MFSR()

define( MFSR,
       `mfsr    $1,3')

# MTSR()

define( MTSR,
       `mtsr    $1,3')

# MTSPR()

define( MTSPR,
       `mtspr   $1,3')

# MFSPR()

define( MFSPR,
       `mfspr   3,$1')

# LOOP()

define( LOOP,
       `b       $')


# simulate TOCE and LTOC MACRO
# Format: TOCE(labelname,[entry|data])
#    where
#       labelname - the name of the csect to create a toc entry for
#       entry - indicates it is a procedure entry
#       data - indicates it is a procedure's static area
# This macro creates a TOC entry called labelname
#
define(TOCE,
`ifelse($2,entry,
`$1.A:  .tc     ENTRY($1)[tc],ENTRY($1)
        .extern ENTRY($1)',
$2,data,
`$1.S:  .tc     DATA($1)[tc],DATA($1)[ua]
        .extern DATA($1)[ua]',
`errprint(`illegal argument "$2"')')')

#
# Format: TOCL(labelname,[entry|data])
#    where
#       labelname - the name of the csect to create a toc entry for
#       entry - indicates it is a procedure entry
#       data - indicates it is a procedure's static area
# This macro creates a TOC entry called labelname
#
define(TOCL,
`ifelse($2,entry,
`$1.A:  .tc     ENTRY($1)[tc],ENTRY($1)',
$2,data,
`$1.S:  .tc     DATA($1)[tc],DATA($1)',
`errprint(`illegal argument "$2"')')')


# Format: LTOC(regno,labelname,[entry|data])
#    where
#       regno - is the register number to load
#       labelname - is the name of the csect address you want to load
#       entry - indicates it is a procedure entry
#       data - indicates it is a procedure's static area
# This macro loads an address from a TOC entry created by toce above
#
define(LTOC,
`ifelse($3,entry,
`       l       $1,$2.A(2)',
$3,data,
`       l       $1,$2.S(2)',
`errprint(`illegal argument "$3"')')')

# Format: LTOCR(regno,labelname,[entry|data],tocr)
#    where
#       regno - is the register number to load
#       labelname - is the name of the csect address you want to load
#       entry - indicates it is a procedure entry
#       data - indicates it is a procedure's static area
#	tocr - indicates register that contains TOC
# This macro loads an address from a TOC entry created by toce/tocl above
# it is used when the toc base is not contained in r2
#
define(LTOCR,
`ifelse($3,entry,
`       l       $1,$2.A($4)',
$3,data,
`       l       $1,$2.S($4)',
`errprint(`illegal argument "$3"')')')


#
# Format: TOC_ORIGIN
#
# Usage:
# label: .long TOC_ORIGIN
#    to get the address of this modules TOC
#    generates TOC[tc] for TOC and TOC[tc0] for XCOFF binders
#
define(TOC_ORIGIN,`TOC[t0]')

#
# Format: CSECT(labelname,[PR|RW|RO|TC],align)
#    where
#       labelname - is the name of the csect you want to create
#       [PR|RW|RO|TC] - csect type
#       align - segment alignment
# This macro is used to define csects.  S_PROLOG should be used to declare code
# segments
#
define( CSECT,
`ifelse(x$3,x,                          # no align parameter
    `ifelse($2,PR,
`       .csect  ENTRY($1[PR])',
    $2,RW,
`       .csect  DATA($1[RW])',
    $2,RO,
`       .csect  DATA($1[RO])',
    $2,TC,
`       .csect  DATA($1[TC])',
   `errprint(`illegal argument "$2"')')'
    ,                                  # alignment parameter specified
    `ifelse($2,PR,
`       .csect  ENTRY($1[PR]),$3',
    $2,RW,
`       .csect  DATA($1[RW]),$3',
    $2,RO,
`       .csect  DATA($1[RO]),$3',
    $2,TC,
`       .csect  DATA($1[TC]),$3',
   `errprint(`illegal argument "$2"')')'
 )'
)

divert(0)dnl
