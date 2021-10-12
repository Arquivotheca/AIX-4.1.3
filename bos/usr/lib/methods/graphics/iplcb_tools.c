/* static char sccsid[] = "@(#)13	1.2  src/bos/usr/lib/methods/graphics/iplcb_tools.c, dispcfg, bos411, 9428A410j 8/10/93 13:11:49"; */
/*
 *   COMPONENT_NAME: SYSXDISPCCM
 *
 *   FUNCTIONS  get_buc
 *		
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
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <stdio.h>

#include "cfgdebug.h"


#include <sys/mdio.h> 		/* machine driver */

/* Required for IPL-control block: */
#include <sys/iplcb.h> 	  /* need new iplcb.h with BUCs and Processor(s) info */

#define BUS_SLOT_MASK	0x1ff     /* ?? */

/* ------------------------------------------------------------------------
 |
 | NAME:  get_buc
 |                                                                    
 | FUNCTION: Reads Bus Unit Controller (BUC) information from IPL control block
 |                                                                    
 | RETURNS:  
 |
 |    0 means no error.  p_buc_info has BUC data
 |
 |    -1 means error.
 |
 | References:  Rain Bow 3 Engineering Work Book
 |              Power PC 32 Bit Arch.
 ------------------------------------------------------------------------*/  


get_buc(slot, p_buc_info)

   unsigned int  slot;                /* value to store in lower 9 bits of Bus Slot 
                                         Config Select for Graphics slot
                                      */
   BUC_DATA       * p_buc_info;

{

   IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory 	*/
   BUC_DATA *		p_buc_arr;	/* IPLCB BUC data array 	*/

   int            rc,i, num_bucs;
   unsigned int   addr;

   char           * p_buf;

   /* 
      Read in the IPL Control Block directory 
   */

   DEBUG_1("get_buc: slot # =%x\n",slot);

   rc = mdd_get(  &iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB	);
   if ( rc != 0 ) 
   {
        DEBUG_0("failed to read the ipl control directory\n");
        return (-1);
   }


   /*--------------------------------------------------- 
      if the BUC  array does not exit, then return error
   ---------------------------------------------------*/

   if ( (iplcb_dir.buc_info_offset == 0) || (iplcb_dir.buc_info_offset == 0))
   {
      DEBUG_0("buc array offset/size is zero\n");
      return (-1);
   }

   /* ---------------------- 
      Read the IPL BUC array 
    ----------------------*/

   addr = iplcb_dir.buc_info_offset ;
   p_buf = malloc (iplcb_dir.buc_info_size) ;

   rc = mdd_get(p_buf, addr , iplcb_dir.buc_info_size , MIOIPLCB );

   if ( rc != 0 ) 
   {
        DEBUG_0("failed to ready buc array\n");

   	free(p_buf);

        return (-1);
   }
  
   p_buc_arr = (BUC_DATA *) p_buf;
   num_bucs = p_buc_arr[0].num_of_structs ;

   DEBUG_1("there are %d entries in buc array\n", num_bucs);

   rc = -1;    /* haven't found the buc */

   for (i = 0 ; i < num_bucs ; i++)
   {

      DEBUG_2("%d entry in buc array, slot=%x\n", i, p_buc_arr[i].bscr_value);

      if ( ((unsigned int)p_buc_arr[i].bscr_value & BUS_SLOT_MASK) == slot)  
      {
         DEBUG_1("found the buc entry with matching slot\n", slot);

         *p_buc_info = p_buc_arr[i]; 

	 rc = 0;	    /* found the buc */

         break;
      }

   }

   free(p_buf);

   return(rc);
} 



/* ------------------------------------------------------------------------
 |
 | NAME: mdd_get
 |                                                                    
 | FUNCTION: Reads "num_bytes" bytes from nvram, IPL control block, or the
 |	     iocc.  Bytes are read from the address "address" and stored at
 |	     address "dest".
 |                                                                    
 | RETURNS:  
 |
 |    0 means no error.
 |
 ------------------------------------------------------------------------*/  

int mdd_get(dest, address, num_bytes, ioctl_type)

   char	*dest;

   int	address;

   int	num_bytes;

   int	ioctl_type;

{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;

	if ((fd = open("/dev/nvram",0)) < 0) 
        {
		DEBUG_0("mdd_get: Unable to open /dev/nvram");
		return(-1);
	}
	mdd.md_addr = address;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	if (ioctl(fd,ioctl_type,&mdd)) 
        {
		DEBUG_0("mdd_get: ioctl failed\n");
		return(-1);
	}

	close(fd);
	return(0);
}



/* ----------------------------------------------- 
|  print all fields in a BUC.  For debugging only!
-------------------------------------------------*/ 

prnt(p_buc)

   BUC_DATA *		p_buc;

{
      int i;

      DEBUG_1("bsrr_offset = %x\n",p_buc->bsrr_offset);
      DEBUG_1("bsrr_mask = %x\n",p_buc->bsrr_mask);

      DEBUG_1("bscr_value = %x\n",p_buc->bsrr_mask);
      DEBUG_1("bscr_value = %x\n",p_buc->bscr_value);

      DEBUG_1("cfg_status = %x\n",p_buc->cfg_status);
      DEBUG_1("device_type = %x\n\n",p_buc->device_type);

      DEBUG_1("num of buids = %x\n",p_buc->num_of_buids);

      for (i = 0 ; i < p_buc->num_of_buids; i++)
      {
         DEBUG_1("   buid = %x\n",p_buc->buid_data[i].buid_value);
         DEBUG_1("   buid_Sptr = %x\n\n",p_buc->buid_data[i].buid_Sptr);
      }

      DEBUG_1("mem_alloc1 = %x\n",p_buc->mem_alloc1);
      DEBUG_1("mem_addr1 = %x\n",p_buc->mem_addr1);

      DEBUG_1("mem_alloc2 = %x\n",p_buc->mem_alloc2);
      DEBUG_1("mem_addr2 = %x\n",p_buc->mem_addr2);

      DEBUG_1("vpd_rom_width = %x\n",p_buc->vpd_rom_width);
      DEBUG_1("cfg_addr_inc = %x\n",p_buc->cfg_addr_inc);

      DEBUG_1("dev_id_reg = %x\n",p_buc->device_id_reg);
      DEBUG_1("aux_info_offset = %x\n",p_buc->aux_info_offset);
      DEBUG_1("feature_rom_code = %x\n",p_buc->feature_rom_code);
      DEBUG_1("IOCC_flag = %x\n",p_buc->IOCC_flag);
}

