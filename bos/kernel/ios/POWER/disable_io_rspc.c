static char sccsid[] = "@(#)73  1.1  src/bos/kernel/ios/POWER/disable_io_rspc.c, sysios, bos41J, 9519A_all 5/2/95 15:05:39";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:
 *      disable_io, dis_pcidev
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef _RSPC

#include <sys/types.h>
#include <sys/machine.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/adspace.h>
#include <sys/system_rspc.h>
#include <sys/ioacc.h>
#include <sys/inline.h>
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/pnp.h>
#include <../io/machdd/md_extern.h>

extern struct io_map	nio_map;

#define	PCI_DEV_SHFT	3	/* Number of bits to shift for device   */
#define PCI_DEV_MASK	0x1f	/* Mask for device			*/
#define PCI_MAX_FUNC	8	/* Max number of functions in device	*/
#define PCI_FUN_MASK	0x7	/* Mask for function			*/
#define PCI_CFG_READ	0	/* Read config space			*/
#define PCI_CFG_WRT	1	/* Write config space			*/
#define PCI_MULTI_FUNC	0x80	/* Multifunction flag in header type	*/
#define _HPB_		HostProcessorBridge
#define _ISAB_		ISABridge
#define _EISAB_		EISABridge

/*
 * NAME: dis_pcidev
 *
 * FUNCTION: 
 *	This function is used to disable a PCI device.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from disable_io.
 *	It cannot pagefault.
 *
 * NOTE:
 *	The config data is read by the word so it is in little endian.
 *
 * RETURNS: None
 */

void
dis_pcidev( int bid, int devnum )
{
	struct	mdio	dev_md;		/* pci_cfgrw structure	    	*/
	ulong		cfg_data[4];	/* Place to hold config data	*/
	int		rc;		/* Return code			*/
	int		first_time;	/* First time check		*/
	int		func;		/* Function in device		*/
	char		class, sub_class, hdr_type;
	ushort		cmd;

	devnum <<= PCI_DEV_SHFT;

	/* set up to use machdd PCI config function */
	dev_md.md_addr = 0;
	dev_md.md_size = sizeof( cfg_data ) / sizeof( ulong );
	dev_md.md_incr = MV_WORD;
	dev_md.md_length = (ulong *)0;
	dev_md.md_sla = devnum;
	dev_md.md_data = (char *)(&cfg_data);

	/* Read the base device and check for a multifunction device */
	(void)pci_cfgrw( bid, &dev_md, PCI_CFG_READ );

	/* Loop through all functions if multifunction device */
	for( first_time = 1, func = 0; func >= 0; func-- ) {

		/* If first time then look for a multifunction device.
		 * If found then reset and start at function 7 continuing
		 * the backward scan.  If not a multifunction then
		 * just proceed to disable this one if it meets
		 * criteria.
		 */
		if( first_time ) {
			first_time = 0;
			hdr_type = (cfg_data[3] >> 8) & 0xFF;
			if( (hdr_type & PCI_MULTI_FUNC) == PCI_MULTI_FUNC ) {
				func = PCI_MAX_FUNC - 1;
				continue;
			}
		}
		else {
			dev_md.md_sla = devnum + func;
			(void)pci_cfgrw( bid, &dev_md, PCI_CFG_READ );
		}

		cmd = (cfg_data[1] >> 16) & 0xFFFF;
		class = cfg_data[2] & 0xFF;
		sub_class = (cfg_data[2] >> 8) & 0xFF;
#ifdef SR_DEBUG
		hdr_type = (cfg_data[3] >> 8) & 0xFF;
		printf( "Dev/func = %x 0 = %x 4 = %x  8 = %x  c = %x\n",
			devnum, cfg_data[0], cfg_data[1], 
			cfg_data[2], cfg_data[3] );
		printf("\tclass = %d sub class = %d ", class, sub_class);
		printf( "cmd = 0x%x hdr_type = %d\n", cmd, hdr_type);
#endif /* SR_DEBUG */

		/* Memory controllers and host bridges are not disabled.
		 * Some machines cannot handle the ISA/EISA bridge being
		 * disabled either.
		 */
		if( (class != MemoryController) &&
		    !((class == BridgeController) && (sub_class == _HPB_)) &&
		    !((class == BridgeController) && (sub_class==_ISAB_)) &&
		    !((class == BridgeController) && (sub_class==_EISAB_)) )
		{
			/* Disable this dev/func */
			cmd &= 0xF8FF;
			dev_md.md_addr = 4;
			dev_md.md_incr = MV_SHORT;
			dev_md.md_data = (char *)(&cmd);
			dev_md.md_size = sizeof( cmd ) / sizeof( short );
			(void)pci_cfgrw( bid, &dev_md, PCI_CFG_WRT );

			/* Restore values */
			dev_md.md_addr = 0;
			dev_md.md_incr = MV_WORD;
			dev_md.md_data = (char *)(&cfg_data);
			dev_md.md_size = sizeof( cfg_data ) / sizeof( ulong );
#ifdef SR_DEBUG
			(void)pci_cfgrw( bid, &dev_md, PCI_CFG_READ );
			cmd = (cfg_data[1] >> 16) & 0xFFFF;
			printf( "  after disable - cmd = 0x%x \n", cmd);
#endif /* SR_DEBUG */
		}
	}
}

/*
 * NAME: disable_io
 *
 * FUNCTION: 
 *	This function is used to disable all ISA DMA channels
 *	and disable all PCI devices
 *
 * EXECUTION ENVIRONMENT:
 *	Called from sr_slih just before wipl on RSPC platforms only.
 *	It cannot pagefault.
 *
 * RETURNS: None
 */

void
disable_io()
{

	volatile struct rspcsio	*sio;	/* Pointer to IO segment	*/
	int	bid;			/* place to build the bid	*/
	int	devnum;			/* PCI device number		*/
	int	busnum;			/* PCI bus number		*/
	int	rc;			/* place to hold return code	*/
	struct	mdio	bus_md;		/* pci_cfgrw structure	    	*/
	ulong		cfg_data;	/* Place to hold config data	*/


	assert( __rspc() );

	/*
	 * Hit the ISA DMA with a master clear
	 */
	sio = (volatile struct rspcsio *)iomem_att( &nio_map );
	
	/* clear the two controllers */
	sio->dma2_mclear = 1;	/* reset controller 2 */
	eieio();
	sio->dma1_mclear = 1;	/* reset controller 1 */
	eieio();

	iomem_det( (void *)sio );
	
	/* Turn off disk activity light */
	operator_panel_light_off();

	/* Walk all PCI busses and disable devices.  Skip any
	 * host bridges or memory controllers.
	 */
	/* set up to use machdd PCI config function */
	bus_md.md_addr = 0;
	bus_md.md_size = sizeof( cfg_data ) / sizeof( ulong );
	bus_md.md_incr = MV_WORD;
	bus_md.md_length = (ulong *)0;

	for( busnum = MD_MAX_BUS-1; busnum >= 0; busnum-- ) {
		devnum = MD_MAX_SLOT - 1;
		bus_md.md_sla = devnum << PCI_DEV_SHFT;
		bus_md.md_data = (char *)(&cfg_data);
		bid = BID_VAL(IO_PCI, PCI_CFGMEM, busnum);
		rc = pci_cfgrw( bid, &bus_md, PCI_CFG_READ );
#ifdef SR_DEBUG
		printf( "Bus num = %x\n", busnum );
#endif /* SR_DEBUG */
		if( rc == EINVAL ) {
			/* Bus does not exists */
			continue;
		}

		/* There is a valid bus here, look for devices */
		do {
			/* Check for a valid device */
			if( cfg_data != 0xFFFFFFFF ) {
				dis_pcidev( bid, devnum );
			}

			/* Don't go below device number of 0 */
			devnum--;
			if( devnum >= 0 ) {
				bus_md.md_sla = devnum << PCI_DEV_SHFT;
				(void)pci_cfgrw(bid, &bus_md, PCI_CFG_READ);
			}
		} while( devnum >= 0 );
	}
}

#endif /* _RSPC */
