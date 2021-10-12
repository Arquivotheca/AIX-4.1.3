/* "@(#)47	1.2  src/bos/kernel/ios/POWER/dse601.h, sysios, bos411, 9428A410j 12/7/93 15:51:00 */

#ifndef _H_DSE601
#define _H_DSE601
#ifdef _POWER_601
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * |< Primary >|<  rT   >|<  rA   >|<       displacement          >| D Form
 * |           |         |         |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
 * |0| | | | |5| | | | |0| | | | | | | | | |0| | | | | | | | | |0| |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           |         |         |         |                   | |
 * |< Primary >|<  rT   >|<  rA   >|<  rB   >|< extended opcode >|R| X Form
 */

struct s_Dform
{
	unsigned int			op   :  6;	/* Primary Opcode */
	unsigned int			rT   :  5;	/* Dest. Register */
	unsigned int			rA   :  5;	/* Address Register */
	unsigned int			off  : 16;	/* Signed offset */
};

struct s_Xform
{
	unsigned int			op   :  6;	/* Primary Opcode */
	unsigned int			rT   :  5;	/* Dest. Register */
	unsigned int			rA   :  5;	/* Address Register */
	unsigned int			rB   :  5;	/* Offset Register */
	unsigned int			xop  : 10;	/* Extended Opcode */
	unsigned int			Rc   :  1;	/* Record Bit */
};

#define	rS	rT			/* For Stores, rT (to) is rS (source) */

typedef union
{
	struct s_Dform		d_inst;
	struct s_Xform		x_inst;
	unsigned long		opcode;
} t_power_i;

/*
 * This D-Form decode makes 1 assumption.
 * 1) OpCodes 2E and 2F, Load/Store Multiple Words, will be handled just like
 *    floating point Load/Stores: no backout is performed and the instruction
 *    is treated as restartable.  See dse601.c for a full opcode list.
 */

/*
 *  D-Form Load/Store Primary OpCode Field Definitions:
 *
 *   0           1           2           3           4           5
 *  +-----------+-----------+-----------+-----------+-----------+------------+
 *  |     1     |     0     |   Size 1  | Load = 0  |   Size 0  | Update = 1 |
 *  |           |           |           | Store = 1 |           |            |
 *  +-----------+-----------+-----------+-----------+-----------+------------+
 *
 *  Sizes:     Size 1    Size 0    Data Type
 *             ------    ------    ---------
 *                0         0       Word
 *                0         1       Byte
 *                1         0       Halfword
 *                1         1       Halfword Algebraic (Loads only)
 */

#define	LDST_DF_UPD	0x01	/* 0b000001 Primary Load/Store Update bit */
#define	STORE_DF	0x04	/* 0b000100 Subset: Primary Stores */
#define	DF_MIN_FX	0x20	/* D-Form Load/Store Min. Fixed Point OpCode  */
#define	DF_MAX_FX	0x2D	/* D-Form Load/Store Max. Fixed Point OpCode  */
#define	DF_MAX_FP	0x37	/* D-Form Load/Store Max. Floating Point Op   */
				/* FP Operations arrive via Alignment Handler */

/*
 *  X-Form Secondary OpCode Field Definitions:
 *
 *    0      1      2      3      4      5      6      7      8      9
 *   +------+------+------+------+------+------+------+------+------+------+
 * 1 |   0  | Sz 1 | Ld 0 | Sz 0 |      |   1  |   0  |   1  |   1  |   1  |
 *   |      |      | St 1 |      | Up 1 |      |      |      |      |      |
 *   +------+------+------+------+------+------+------+------+------+------+
 * 2 |   1  | Size | Ld 0 |   0  |   0  |   1  |   0  |   1  |   1  |   0  |
 *   |      |      | St 1 |      |      |      |      |      |      |      |
 *   +------+------+------+------+------+------+------+------+------+------+
 * 3 |   1  |   0  | Ld 0 | In 0 |   0  |   1  |   0  |   1  |   0  |   1  |
 *   |      |      | St 1 | Im 1 |      |      |      |      |      |      |
 *   +------+------+------+------+------+------+------+------+------+------+
 * 4 |   1  |   0  | Ld 0 | Size |      |   1  |   0  |   1  |   1  |   1  |
 *   |      |      | St 1 |      | Up 1 |      |      |      |      |      |
 *   +------+------+------+------+------+------+------+------+------+------+
 *
 * Line 1 is used for normal X Form (register indexed) Load/Stores
 *   The SZ(1:0) bits decode as for D Form
 * Line 2 is used for Byte reversed X Form Load/Stores
 *   The Size bit is 0 for Word, 1 for Halfword
 * Line 3 is for Load and Store String, Immediate (Im) or Indexed (In).
 * Line 4 is for Floating Point X-Form Load/Stores
 *   Size indicates Double when set, Single when clear
 */

/*
 * Only four types of instructions arrive at the X-Form decode, they are
 * listed in the table above.
 *
 * Load / Store Fixed Point Indexed, w w/o Update
 * Load / Store Byte Reversed
 * Load / Store String, Immediate / Indexed
 * Load / Store Floating Point Indexed
 *
 * The two latter pass through without any backout.  See the C source file
 * for a complete explanation.
 */

#define	LDST_XF		0x1F	/* 0b011111 Extended Load/Stores (X-Form) */
#define	STORE_XF	0x080	/* 0b0010000000 Extended Store */
#define	LDST_XF_UPD	0x020	/* 0b0000100000 Update forms (Load/Store) */

#define	XF_NORMAL_MSK	0x21F	/* X-Form Normal Move Mask */
#define	XF_NORMAL	0x017	/* X-Form Normal Move */

#define	XF_REVERSED_MSK	0x27F	/* X-Form Byte Reversed Move Mask */
#define	XF_REVERSED	0x216	/* X-Form Byte Reversed Move */
#define	XF_REVERSED_SZ	0x100	/* 0 = Word, 1 = Halfword */

#define	XF_STRING_MSK	0x33F	/* X-Form String Move Mask */
#define	XF_STRING	0x215	/* X-Form String Move */
#define	XF_STRING_IMM	0x040	/* 0 = Indexed, 1 = Immediate */

#define	XF_FLOAT_MSK	0x317	/* X-Form Floating Move Mask */
#define	XF_FLOAT	0x217	/* X-Form Floating Move */
#define	XF_FLOAT_DBL	0x040	/* 0 = Single, 1 = Double */

void	undo_601_dse( struct mstsave  *, unsigned long, struct thread * );

/*
 * Statistics are kept on 601 Direct Store Errors.  Depending on the viewing
 * context, these are volatile.
 */

extern unsigned long		dse_601_total;

#ifdef  _POWER_MP

#include <sys/user.h>
#include <sys/machine.h>
extern void dse_backt();
extern void dse_ex_handler();

#endif  /* _POWER_MP */
#endif  /* _POWER_601 */
#endif  /* _H_DSE601 */
