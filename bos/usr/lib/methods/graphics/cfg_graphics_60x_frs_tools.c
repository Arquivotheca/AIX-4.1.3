static char sccsid[] = "@(#)93	1.9  src/bos/usr/lib/methods/graphics/cfg_graphics_60x_frs_tools.c, dispcfg, bos411, 9428A410j 3/22/94 15:41:22";
/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: FRS_Add_PPC_Video_Device 
 *              FRS_Add_MCA_Device 
 *              wr_60x_std_cfg_reg_w, 
 *              rd_60x_std_cfg_reg_w,
 *              rd_60x_vpd_feat_rom, 
 *              nvram_open
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
 *  Standard include files
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>                   /* open()      */
#include <sys/mdio.h>                /* Machine dd  */
#include "cfgdebug.h"		     /* DEBUG_x     */

#include <sys/cfgodm.h>

/* 
 * 601 registers definition.  For rd/wr functions
 */
#include <graphics/60x_regs.h>



/* 
 * Feature ROM structure defintions (for FRS_Add_PPC_Video_Device()) 
 */
#include <cf.h>                     /* E_xx constants */ 
#include "cfg_graphics_frs.h"

#include "frs_display.h"
#include "frs_display_macs.h"


/* 
 * for FRS_Add_MCA_Device()
 */
#include "cdd.h"
#include "frs_60x.h"


/* ------------------------------------------------------------------------------ 
|
|   FRS_Add_PPC_Video_Device: this function only supports Feature ROM on 60x Power PC
|                             Its main task is to extract the .add file from ROM and
|                             populates the ODM data base with information from .add
|
|   Inputs:   logical parent name of device  -  not used
|             slot number                    -  from buc entry in iplcb (buc.bscr_value & 0x1FF) 
|
|   Output:   ODM database updated with device's .add
|
|   error codes:  0  - successfully populated database
|                -1  - no ODM information in ROM
|                > 0 - error occurred  
|
|   Note these error codes cannot be changed.  See design for feature 62524 
|
|  References:   POWER PC 32 BIT ARCH
|                RAINBOW 3 ENGINEERING WORKBOOK
|	         Design for Feature 62524 - ppc bus configuration and mdd changes 
|
-------------------------------------------------------------------------------- */

int 
FRS_Add_PPC_Video_Device(parent_name, slot)

   char * parent_name;      /* logical name of parent of device - not used by this func */

   int  slot;              /* physical slot - CuDv.connWhere  */
{

   	ulong rc, bus_bid, devid = 0;


   	DEBUG_2("FRS_Add_PPC_Video_Device: parent lname = <%s> , slot =%d\n",parent_name, slot);


	/*
	 *  Make sure we deal with display device by checking the device id 
	 */
	rc = rd_60x_std_cfg_reg_w(slot, BUS_60X_DEV_ID_REG, &devid);

	if (rc)
	{
   	   	DEBUG_0("FRS_Add_PPC_Video_Device: can't read dev id\n");
	}

   	DEBUG_1("FRS_Add_PPC_Video_Device: dev id = %x\n", devid);

	if (  (!rc )  &&  ((devid & BUS_60X_DEV_ID_MASK) == 0) )      /* real device id is in ROM */ 
	{
		rc = rd_60x_vpd_feat_rom(slot, BUS_60X_ROM_DEV_ID_REG, &devid, sizeof(devid) );
		if (rc)
		{
   	   		DEBUG_0("FRS_Add_PPC_Video_Device: can't read ROM dev id\n");
		}

   		DEBUG_1("FRS_Add_PPC_Video_Device: ROM dev id = %x\n", devid);
	}

	if (! rc)   /* no error */
	{
		/* 
		 * extract the device id type - bits 8 - 23.  0x0040 is the id
		 * for display adapter
		 */
		if ( (devid & RSCAN_60X_DEVID_TYPE_MASK) == RSCAN_60X_DEVID_TYPE_GRAPHICS)
		{  
   			bus_bid  = BUS_60X_BID_SDSS_ACCESS | BUS_60X_32BIT_SET_T_BIT; 

			rc = frs_ODM_add(CFG_GRAPH_BUID_7F, slot, bus_bid, 0);   /* don't have iocc */

			DEBUG_1("FRS_Add_PPC_Vide_Device: frs_ODM_add returns code = %d\n",rc);

		}
		else   /* not display device */
		{
			rc = 1;    /* error */
		}
	}
	else
	{
		rc = 1;    /* error */
	}

	DEBUG_1("FRS_Add_PPC_Vide_Device: exit with rc = %d\n",rc);

	return (rc);
}




/* ------------------------------------------------------------------------------ 
|
|   FRS_Add_MCA_Device:       this function only supports Feature ROM on micro-channel 
|                             Its main task is to extract the .add file from ROM and
|                             populates the ODM data base with information from .add
|
|   Inputs:   logical parent name of device  -  "bus0" or "bus1" 
|             slot number                    -   micro-channel slot nunmber
|
|   Output:   ODM database updated with device's .add
|
|   error codes:  0  - successfully populated database
|                -1  - no ODM information in ROM
|                > 0 - error occurred  
|
|   Note these error code cannot be changed.  See design for feature 62524 
|
|  References:   Micro-channel Feature ROM Architecture 
|	         Design for Feature 62524 - ppc bus configuration and mdd changes 
|
|  Note:         This code has not been tested - hardware not available
|
-------------------------------------------------------------------------------- */

int
FRS_Add_MCA_Device(parent_name, slot)

   char * parent_name;       /* logical name of parent of device */

   int  slot;              /* physical slot - CuDv.connWhere  */
{
        struct Class *cusdev;           /* customized devices class ptr */
        struct CuDv cudv;               /* customized device object info */

	int rc;
	ulong iocc_bid, bus_bid, buid = 0;
        char sstring[50];

   	DEBUG_2("FRS_Add_MCA_Device: parent lname = <%s> , slot =%d\n",parent_name, slot);

        /* start up odm */
        if (odm_initialize() == -1) 
	{
                /* initialization failed */
                DEBUG_0("FRS_Add_MCA_Device: odm_initialize() failed\n")
                return(1); 
        }

        DEBUG_0 ("FRS_Add_MCA_Device: ODM initialized\n")

        /* 
         * open customized devices object class 
         */

        if ( (int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) 
        {
                DEBUG_0("FRS_Add_MCA_Device: open class CuDv failed\n");
		odm_terminate();
                return(1);
        }
        /* 
         * search for customized object with this logical name 
         */

        sprintf(sstring, "name = '%s'", parent_name);

        rc = (int)odm_get_first(cusdev,sstring,&cudv);

        if (rc==0) 
	{
                /* No CuDv object with this name */
                DEBUG_1("FRS_Add_MCA_Device: failed to find CuDv object for %s\n", parent_name);
		odm_close_class(CuDv_CLASS);
		odm_terminate();
                return(1);
        }


	buid = 0x20 | ( cudv.location[3] - '0' );

        DEBUG_1("FRS_Add_MCA_Device: computed buid = %x\n", buid);

	#define BUS_2x_BUID_SHIFT	20

	bus_bid  = (buid << BUS_2x_BUID_SHIFT)  | CDD_SEG_T_BIT    /* T = 1        */
                                                | CDD_BYPASS_TCW   /* no TCWs yet  */
                              			| CDD_ADDR_CHK
                              			| CDD_ADDR_INCR;

        DEBUG_1("FRS_Add_MCA_Device: computed bus_bid = %x\n", bus_bid);

	iocc_bid = bus_bid | CDD_IOCC_SELECT | CDD_ADDR_RTMODE ; 

	/* 
	 *  Assume slot is whatever value in CuDv.connWhere of the device
         *  This is physical slot number.  We deal with logical slot number
	 *  which is one less than physical slot number - IOCC rule.
	 *  FRS_Find_Video_Device() will take care of this.
         */
	rc = frs_ODM_add(CFG_GRAPH_BUID_2x, slot, bus_bid, iocc_bid);

	if (rc >0 )
	{	
      		DEBUG_0("FRS_Add_MCA_Device: error occurred\n");
	} 
	else if (rc < 0 )
	{
      		DEBUG_0("FRS_Add_MCA_Device: .add not found in Feature ROM\n");
	}
	else
	{
      		DEBUG_0("FRS_Add_MCA_Device: successfully did odmadd\n");
	}

   	DEBUG_1("FRS_Add_MCA_Device: exit with rc = %d\n", rc);

	return (rc);
}



/* ------------------------------------------------------------------------------ 
|
|   frs_ODM_add:    this function supports Micro-channel and 60x bus 
|                   Its main task is to extract the .add file from ROM and
|                   populates the ODM data base with information from .add
|
|   Inputs:   bus type  - micro-channel or 60x 
|             slot number                 
|	      bus_bid   - value to load in a segment to do bus io
|	      iocc_bid  - value to load in a segment to talk to iocc 
|
|   Output:   ODM database updated with device's .add
|
|   error codes:  0  - successfully populated database
|                -1  - no ODM information in ROM
|                > 0 - error occurred  
|
|   Note these error code cannot be changed.  See design for feature 62524 
|
----------------------------------------------------------------------------------*/


int frs_ODM_add(bus_type, slot, bus_bid, iocc_bid)

   ulong bus_type,           /* Micro-channel or 60x */

         bus_bid,

         iocc_bid;

   	 int  slot;           /* slot number */

{

   RSCAN_VIDEO_HEAD *	mem_head;

   char sstr[80];	
   char file_name[256], s_child_utype[256], s_child_method[256];
   char * child_utype, * child_method;
   struct PdAt pdat;           /* predefined attribute object */


   int rc;


   child_utype =  s_child_utype; 
   child_method = s_child_method;


   /*-----------------------------------------------------------------
   | ppc bus configuration didn't find PdDv in ODM database for the device 
   | it needs to configure.   Now we look for the .add in device's Feature ROM 
   |------------------------------------------------------------------*/

   /* 
    * This busid is not used to do pio but for the nvram_read/write to  
    * tell it is 60x std/frs access. 
    */ 


   rc = FRS_Find_Video_Device(	bus_type, &mem_head, bus_bid, iocc_bid, slot);

   switch (rc)
   {
      case E_OK:
		/*----------------------------------------
		| Found a device -- continue in this routine
		|----------------------------------------*/

	DEBUG_0("frs_ODM_Add: found FRS device\n");

	break;


      case E_NODETECT:

		/*-----------------------------------------
		| Did not find a device -- exit OK
		|------------------------------------------*/

	DEBUG_0("frs_ODM_Add: no FRS device detected\n");

	return (-1);

	break;

      default:

	DEBUG_0("frs_ODM_Add: error occurred looking for FRS device\n");

	return (1);     /* error - pick some constant from cfg.h */

   }

	
	/*---------------------------------------------------------------
	| Fall through to here means we found Feature ROM device. 
	| mem_head points to the contents of the FRS ROM for us
	|
	| Get the ODM adapter.add file from the ROM contents
	| Update the ODM with that file's contents
	| Save the unique type and cfg method name of the child
	|---------------------------------------------------------------*/

   rc = FRS_Make_Temp_File(	mem_head,
				CFG_GRAPH_ODM_FILE,
				0444,	/* perms */
				&file_name	);

   DEBUG_1("frs_ODM_Add: .add file name = <%s>\n",file_name);

   if ( rc != 0 )
   {
        DEBUG_0("frs_ODM_Add: FRS_Make_Temp_File failed\n");

	return (1);     /* error - pick some constant from cfg.h */
   }

   /* 
    * For diskless work station, can we update predefined data base ??
    * How does it work ???
    */
   rc = FRS_Update_ODM_From_File( file_name, &child_utype, &child_method);

   if ( rc != 0 )
   {
        DEBUG_0("frs_ODM_Add: FRS_Update_ODM_From_File failed \n");

	return (1);
   }

   DEBUG_2("frs_ODM_Add:  utype= %s method = %s\n", child_utype, child_method);

   /* 
    * remove the temporary file 
    */

    rc = unlink( file_name );

    if (rc)
    {
       DEBUG_1("frs_ODM_Add:  Can't unlink tmp file <%s>\n",file_name);
    }

	/*---------------------------------------------------------------
	| Need to add a special FRS attribute to the PdAt so that cfgbus
	| can later read it and determine if a CCM adapter is present.
	|---------------------------------------------------------------*/

	sprintf(sstr, "uniquetype=%s AND attribute=frs",child_utype);
	if( (rc = odm_get_obj(PdAt_CLASS, sstr, &pdat, ODM_FIRST)) == 0 )
	{
		strcpy(pdat.uniquetype, child_utype);
		strcpy(pdat.attribute, "frs");
		*pdat.deflt = '\0';
		*pdat.values = '\0';
		*pdat.width = '\0';
		strcpy(pdat.type, "R");
		*pdat.generic = '\0';
		strcpy(pdat.rep, "sl");
		pdat.nls_index = 0;

		if (odm_add_obj(PdAt_CLASS,&pdat) < 0)
    	{
       		DEBUG_0("frs_ODM_Add: Can't create FRS attribute in the PdAt.\n");
    	}
	}
	else if( rc == -1 )
    {
    	DEBUG_0("frs_ODM_Add: Can't access FRS attribute in the PdAt.\n");
   		return(rc);
    }

	return(0);
}





/*----------------------------------------------------------------
|
|  Open the special file in order to request services from the 
|  machine the machine driver
|
| ---------------------------------------------------------------*/ 

static nvram_open(fd)

   int * fd;

{
 	*fd = open( "/dev/nvram", O_RDWR, 0 );
	
	if ( *fd < 0 )
	{
		DEBUG_0("open of /dev/nvram failed\n");
		return (*fd);
	}

	return (0);
}





/* ---------------------------------------------------------------
|
|	FUNCTION DESCRIPTION 
|
|   This function is ONLY for reading the 601 VPD and 
|   Feature ROM in bytes.  The specified address has to be 
|   0xFFA0 0000 + some offset - where offset < 0x1F FFFF
| 
|   Since the machine driver adds the the base, 0xFFA0 0000 to
|   the offset, this function, will figure out the offset before invoking mdd 
|
|   Note caller will take into account the word incr value 
|   in order calculate the address correctly.  Also, the ioctl 
|   call will fail on Power Architecture !
|
|  rd_60x_vpd_feat_rom:
|
|     inputs:  - slot number (buc.bscr_value & 0x1FF - see BUC structure in iplcbl.h)
|              - addr is 0xFFA0 0000 + some number 
|              - pointer to buffer for return data
|              - length of buffer 
|
|    outputs:   buffer with data 
|
|    return code:  0  - data is valid
|                 -1  - data is not valid 
|
|  References:   POWER PC 32 BIT ARCH
|                RAINBOW 3 ENGINEERING WORKBOOK
|	         Design for Feature 62524 - ppc bus configuration and mdd changes 
| 
| ---------------------------------------------------------------*/ 

rd_60x_vpd_feat_rom(slot, addr, p_buf, len)

   ulong slot,          /* lower 9 bits of  Bus Slot Config. Reg to 
                           select graphics device 
                        */

         addr ;         /* where to read from */ 

   char * p_buf;        /* buffer to store returned data */

   int    len;          /* how many bytes to read        */

{

	int		fd, rc;
	MACH_DD_IO 	mdd;

	DEBUG_4("rd_bytes_60x_vpd_feat_rom: slot=%x, addr=%x, pbuf=%x, len=%d\n",
                                                 slot, addr, p_buf, len);

	if ( (addr < BUS_60X_START_VPD_FEATURE_ROM) || 
	     (addr > BUS_60X_END_VPD_FEATURE_ROM)      )
	{
	   DEBUG_0("rd_bytes_60x_vpd_feat_rom: address is out of range\n"); 
	   return (-1);
	}

	rc = nvram_open(&fd);
        if (rc)
        {
	   return (-1);
        }

        /* 
         * Calculate the offset  
        */
	mdd.md_addr		= addr - BUS_60X_START_VPD_FEATURE_ROM;

	mdd.md_size		= len;
	mdd.md_data		= (char * ) p_buf;
        /* 
           mdd.md_incr has to be byte (mdd always does byte read) 
        */
	mdd.md_incr		= MV_BYTE;

	mdd.md_sla	 	= slot; 

	rc = ioctl( fd, MIOVPDGET, & mdd, 0);

	if ( rc != 0 )
	{
	   DEBUG_0("rd_bytes_60x_vpd_feat_rom: MIOVPDGET failed\n");

	   close(fd); 

	   return (-1);
	}

	close(fd);       /* close the special file */

	return (0);
}




/* ---------------------------------------------------------------
|
|	FUNCTION DESCRIPTION 
|
|
|   This function is ONLY for reading the 60x Standard 
|   Configuration Space, 0xFF20 0000 to 0xFF20 1FFF, a 
|   word at a time.  All reads are from 0xFF20 0000 + some offset (
|   where the offset < 0x1FFF).
| 
|   Since the machine driver knows about the base, 0xFF20 0000
|   This function, will figure out the offset before invoking mdd 
|
|   Note caller will take into account the word incr value 
|   in order calculate the address correctly.  Also, the ioctl 
|   will fail on Power Arch!
|
|
|  rd_60x_std_cfg_reg_w: 
|
|     inputs:  - slot number (buc.bscr_value & 0x1FF - see BUC structure in iplcbl.h)
|              - addr is 0xFF20 0000 + some number and the addr is on word or doubled 
                 word boundary (depending on the word increment value) 
|              - pointer to buffer for returned data
|
|    outputs:   buffer with data 
|
|    return code:  0  - data is valid
|                 -1  - data is not valid 
|
|  References:   POWER PC 32 BIT ARCH
|                RAINBOW 3 ENGINEERING WORKBOOK
|	         Design for Feature 62524 - ppc bus configuration and mdd changes 
| 
| ---------------------------------------------------------------*/ 

rd_60x_std_cfg_reg_w( 

		ulong			slot,  /* lower 9 bits of Bus Slot Select Cfg Reg 
                                                   value to select graphics device. 
                                                */

		ulong			addr,  /* where to read */

		uchar *			p_buf  /* buffer for returning a word */

	       )
{
	int		fd, rc;
	MACH_DD_IO	mdd;

	DEBUG_3("rd_60x_std_cfg_reg_w: slot =%x, addr=%x, pbuf=%x\n",
                                                 slot, addr, p_buf);

	if ( (addr < BUS_60X_START_CFG_REGS) || 
	     (addr > BUS_60X_END_CFG_REGS)      )
	{
	   DEBUG_0("rd_60x_std_cfg_reg_w: address is out of range\n"); 
	   return (-1);
	}

	rc = nvram_open(&fd);
        if (rc)
        {
	   return (-1);
        }

        /* 
         * Calculate the offset  
        */
	mdd.md_addr		= addr - BUS_60X_START_CFG_REGS;

	mdd.md_data		= (char * ) p_buf;
	mdd.md_incr		= MV_WORD; 

        /* 
           mdd.md_size has to be 1 (mdd always assumes 1) 
        */
	mdd.md_size		= 1;

	mdd.md_sla	 	= slot; 

	rc = ioctl(fd, MIOCFGGET, & mdd, 0);

	if ( rc != 0 )
	{
	   DEBUG_0("rd_60x_std_cfg_reg_w: MIOCFGGET failed\n");

	   close(fd);

	   return (-1);
	}

	close(fd);       /* close the special file */

	return (0);
}




/* ---------------------------------------------------------------
|
|	FUNCTION DESCRIPTION 
|
|
|   This function is ONLY for writing the 60x Standard 
|   Configuration Space, 0xFF20 0000 to 0xFF20 1FFF, a 
|   word at a time.  All writes are from 0xFF20 0000 + some offset (
|   where the offset < 0x1FFF)
|
|   Since the machine driver knows about the base, 0xFF20 0000
|   This function, will figure out the offset before invoking mdd 
|
|   Note caller will take into account the word incr value 
|   in order calculate the address correctly.  Also, the ioctl 
|   will fail on Power Arch!
|
|
|   wr_60x_std_cfg_reg_w: 
|
|     inputs:  - slot number (buc.bscr_value & 0x1FF - see BUC structure in iplcbl.h)
|              - addr is 0xFF20 0000 + some number and the addr is on word or double word
                 boundary (depending on the word increment value) 
|              - pointer to buffer of data
|
|    outputs:  none 
|
|    return code:  0  - data is valid
|                 -1  - data is not valid 
|
|   References:   POWER PC 32 BIT ARCH
|                RAINBOW 3 ENGINEERING WORKBOOK
|	         Design for Feature 62524 - ppc bus configuration and mdd changes 
| 
| ---------------------------------------------------------------*/ 


wr_60x_std_cfg_reg_w( 

		ulong			slot,  /* lower 9 bits of Bus Slot Select Cfg Reg 
                                                  to slect the graphics device 
                                                */

		ulong			addr,   /* where to write */ 

		uchar *			p_buf  /* buffer with a word of data to write */

	       )
{
	int		fd, rc;
	MACH_DD_IO	mdd;

	DEBUG_3("wr_60x_std_cfg_reg_w: slot=%x, addr=%x, val=%x\n",
                                                 slot, addr, *p_buf);

	if ( (addr < BUS_60X_START_CFG_REGS) || 
	     (addr > BUS_60X_END_CFG_REGS)      )
	{
	   DEBUG_0("wr_60x_std_cfg_reg_w: address is out of range\n"); 
	   return (-1);
	}

	rc = nvram_open(&fd);
        if (rc)
        {
	   return (-1);
        }

        /* 
         * Calculate the offset  
        */
	mdd.md_addr		= addr - BUS_60X_START_CFG_REGS;

	mdd.md_data		= (char * ) p_buf;
	mdd.md_incr		= MV_WORD; 

        /* 
           mdd.md_size has to be 1 (mdd always assumes 1) 
        */
	mdd.md_size		= 1;
	mdd.md_sla	 	= slot; 

	rc = ioctl( fd, MIOCFGPUT, & mdd, 0);

	if ( rc != 0 )
	{
	   DEBUG_0("wr_60x_std_cfg_reg_w: MIOCFGPUT failed\n");

	   close(fd);

	   return (-1);
	}

	close(fd);       /* close the special file */

	return (0);
}
