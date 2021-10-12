static char sccsid[] = "@(#)74	1.12  src/bos/kernel/io/machdd/pgs_bump_init.c, machdd, bos41J, bai15 4/10/95 17:43:50";
/*
 * COMPONENT_NAME:  (MACHDD) Machine Device Driver
 * 
 * FUNCTIONS: PEGASUS BUMP interface initialization code
 * 
 * ORIGINS: 83 
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifdef _RS6K_SMP_MCA

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/mpc.h>
#include "pgs_novram.h"
#include "pgs_bump.h"

extern ulong NVRAM_base;
extern char *pgs_lcd;			/* lcd display address in NVRAM */
extern uint *pgs_electro_keyp;		/* electronic key address in NVRAM */
extern uint pgs_tp_buf_sz;		/* NVRAM buffer size  */
extern char *pgs_tp_buf;		/* NVRAM buffer address */
extern uint pgs_tp_buf_off;		/* NVRAM buffer offset */
extern struct tp_fifo_header *pgs_pfifo;
extern pgs_bump_msg_t *pgs_fifo_cmd, *pgs_fifo_stat;
extern Simple_lock pgs_bump_lock;	/* lock for serializing 
					   BUMP accesses with BUMP 
					   interrupt handler */

extern struct intr mdd_intr;
extern mdbumpintr();
extern mpc_msg_t mdbumpepow_mpc;
extern void mdbumpepow();
extern int nio_buid;

extern int pgs_Flash_lvl, pgs_IPL_lvl, pgs_SSGA_lvl, pgs_IONIAN_lvl;
extern int is_a_Junior, intbump;
extern int pgs_CCA2_lvl, pgs_601_lvl, pgs_DCB_lvl;
static struct vpd_field v_f = {0};   /* local copy of a VPD (not in BSS !!) */
extern uchar pgs_bump_init_done;

mdbumpinit()
{
	struct interf_header *interf_header;

	interf_header = &((struct novram *)NVRAM_base)->cpu_bump.interf_header;
	pgs_lcd = (char *)((struct op_interf *)(NVRAM_base
			+ interf_header->op_interf_adr))->LCD_str_new;
	pgs_electro_keyp = (uint *) &((struct op_interf *)(NVRAM_base
			+ interf_header->op_interf_adr))->electro_key;
	pgs_tp_buf_off = interf_header->tp_buffer_adr;
	pgs_tp_buf = (char *)(NVRAM_base + pgs_tp_buf_off);
	pgs_tp_buf_sz = interf_header->tp_buffer_lg;
	pgs_pfifo = (struct tp_fifo_header *)(NVRAM_base
					+ interf_header->tp_header_adr);
	pgs_fifo_cmd = (pgs_bump_msg_t *)((struct tp_fifo *)(NVRAM_base
					+ interf_header->tp_fifo_adr))->tp_command;
	pgs_fifo_stat = (pgs_bump_msg_t *)((struct tp_fifo *)(NVRAM_base
					+ interf_header->tp_fifo_adr))->tp_status;
	lock_alloc(&pgs_bump_lock, LOCK_ALLOC_PIN, PGS_BUMP_LOCK_CLASS, -1);
	simple_lock_init(&pgs_bump_lock);

	/* init BUMP off-level interrupt structure */
	mdbumpepow_mpc = mpc_register(INTEPOW, mdbumpepow);
	INIT_OFFL0( &mdd_intr, mdbumpintr, nio_buid);
	pgs_bump_init_done = 1;
}

/*
 * NAME: find_field
 *
 * FUNCTION: Search a field with the given name in a VPD area.
 *	Copy the field to the global buffer.
 *
 * EXECUTION ENVIRONMENT:
 *	called from pgs_check_levels(), early system init.
 *	WARNING: BSS not initialized
 *
 * RETURNS: a pointer to the global buffer copy of the VPD.
 */
char *
find_field(struct vpd_head *v_h, char *field_name)
{
	struct vpd_head l_v_h;
	char *v_end;
	extern char *nvram_cpy( char *s1, const char *s2, int n);
	char *v_fh = (char *)v_h + sizeof(*v_h);
	uint vfh_sz = sizeof(v_f.h);

	/* copy VPD area header */
	(void) nvram_cpy( (char *)&l_v_h, (char *)v_h, sizeof(*v_h));

	/* Is this actually a VPD ? */
	if ( strncmp( l_v_h.ident, VPD_IDENT, 3) )
		hw_fw_mismatch("find_field: bad VPD area\n");

	v_end = v_fh + l_v_h.length * 2 ;

	do {
		/* Copy field header */
		(void) nvram_cpy( (char *)&v_f, v_fh, vfh_sz);

		/* Is this the name we are looking for ? */
		if( !strncmp( v_f.h.ident, field_name, 2) )

			/* Found it: copy field data and return the address 
			   (skip header) */
			return( nvram_cpy(v_f.data, v_fh+vfh_sz, v_f.h.length*2));

		/* Advance to next vpd field head */
		v_fh += v_f.h.length*2;
	} while ( v_fh < v_end );
	
	/* End of VPD area: failure */
	hw_fw_mismatch("find_field: End of VPD area\n");
}	

/*
 * NAME: pgs_check_levels
 *
 * FUNCTION: Search some VPD fields and check that the key FW/HW components
 *	match the level supported by AIX.
 *	Also, set-up some globals for work-arounds tuning, prototypes, etc ...
 *
 * EXECUTION ENVIRONMENT:
 *	called from hardinit(), early system init (translate off).
 *	WARNING: BSS not initialized.
 *
 * INPUT: a pointer to the Pegasus configuration table, in NVRAM
 *
 * RETURNS: none.
 */
pgs_check_levels(struct config_table *conf_table)
{
	struct vpd_head *iod_h= (struct vpd_head *)&conf_table->iod.board_vpd[0];
	struct vpd_head *mpb_h= (struct vpd_head *)&conf_table->mpb.board_vpd[0];
	struct vpd_head *cpu_h= (struct vpd_head *)&conf_table->
		cpu[my_phys_id()/2].board_vpd[0];

	/* Are we actually in the NVRAM ? */
	if ( ((uint)iod_h & 0xFFE00000) != 0xFF600000 )
		hw_fw_mismatch("Bad configuration table pointer\n");

	pgs_Flash_lvl  = atoi_n( find_field( iod_h, ALTERABLE_ROM_ID)+Flash_lvl_o, 
				Flash_lvl_l);
	pgs_IPL_lvl    = atoi_n( find_field( iod_h, ALTERABLE_ROM_ID)+IPL_lvl_o,
				IPL_lvl_l);
	pgs_SSGA_lvl = *(short *)( find_field( iod_h, PROCESSOR_COMPONENT)+
					    SSGA_lvl_o);
	pgs_IONIAN_lvl = *(short *)( find_field( iod_h, PROCESSOR_COMPONENT)+
					    IONIAN0_lvl_o);
	pgs_CCA2_lvl   = *(short *)( find_field( cpu_h,  PROCESSOR_COMPONENT)+
					    CCA2_lvl_o);
	pgs_601_lvl   = *(short *)( find_field( cpu_h,  PROCESSOR_COMPONENT)+
					    CPU_lvl_o);
	pgs_DCB_lvl   = *(short *)( find_field( mpb_h,  PROCESSOR_COMPONENT)+
					    DCB_lvl_o);

	/* NVRAM WA */
	if( pgs_SSGA_lvl <= 2 ) {
		pgs_SSGA_lvl = 2;
		INTBUMP = INTCLASS3;
	}

	/* reject old FW or HW */

	if ( pgs_Flash_lvl < 301  ||  pgs_IPL_lvl < 301 )
		hw_fw_mismatch("FW_Flash or IPL not supported\n");

	if ( pgs_CCA2_lvl && (pgs_CCA2_lvl < 4) )
		hw_fw_mismatch("Unsupported CCA2 level\n");
}

/* Convert a string to an integer, limiting to length characters */ 
atoi_n(char *ptr, int length)
{
	int res = 0;
	while( length-- ) {
		res *= 10;
		res += *ptr++ - '0';
	}
	return(res);
}

/*
 * Stop the system if something is wrong.
 * Write a code to the leds, call the debugger if possible, and then
 * loop for ever.
 */
hw_fw_mismatch(char *msg)
{
#define MISMATCH_CODE	0x88300000   /* "Pegasus Support (Japan)" :-) */

	write_leds(MISMATCH_CODE);
#ifdef _KDB
	if (__kdb()) {
		kdb_printf(msg);
		gimmeabreak();
	} else
#endif /* _KDB */
		for (;;) write_leds(MISMATCH_CODE);
}

#endif /* _RS6K_SMP_MCA */
