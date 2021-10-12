static char sccsid[] = "@(#)06	1.7  src/bos/usr/ccs/lib/libdiag/diag_support.c, libdiag, bos41J, 9520A_all 5/15/95 14:05:26";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS:   supports_diagnostics
 *              has_LED_support
 *		has_service_processor
 *		has_isa_capability
 *              is_rspc_model
 *		is_mp
 *		diag_get_device_flag
 *		diag_get_pdbits
 *		diag_get_resid_errlg
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

#include <stdio.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/residual.h>
#include <sys/pnp.h>
#include <diag/diag.h>
#include <diag/class_def.h>
#include <diag/diagresid.h>
#define _KERNSYS
#define _RS6K
#define _RSPC
#define _RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef  _KERNSYS

IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory		*/
SYSTEM_INFO system_info;	/* IPL control block info section	*/

char    platform[16];		/* ASCII string of platform		*/
char    modeltype[50];
int 	mp_capable = 0;		/* MP capable flag			*/

/* Functions prototype */
 
void decode_dev(u_int, char *);
u_int swap_endian(u_int);
int diag_get_device_flag(char *, long *);
int diag_get_pdbits(int *, uint **, ulong **);
int diag_get_resid_errlg(int *, uchar **);
 

/*  */
/*
 * NAME: supports_diagnostics
 *
 * FUNCTION: This function returns to the calling function a  
 * value indicating whether AIX diagnostics is supported on this
 * platform or not.   
 *        
 * NOTES: 
 *
 * RETURNS: 
 *	 0 - Not supported
 *	 1 - Supported
 *	-1 - System error obtaining data
 *
 */

int
supports_diagnostics() 
{
	int	aix_model_code;

	if (init_iplcb_structure() == -1 ) return (-1);	

	/* Diagnostics is supported on RS6K models, and RS6KSMP models */
	if ( !strcmp(platform,"rs6k") || !strcmp(platform,"rs6ksmp") ) 
		return (1);
	/* If running on rspc platform check model code to see if */
	/* it supports diagnostics. Bit 18 of model code.	  */

	if ( !strcmp(platform, "rspc") )
	{
		(void)get_cpu_model(&aix_model_code);
		if(aix_model_code & 0x40000)
			return(1);
	}
	return(0);
}


/*  */
/*
 * NAME: supports_ela
 *
 * FUNCTION: This function returns to the calling function a  
 * value indicating whether error log analysis is supported on
 * this platform. At this time, if the function returns a 1,
 * it means that diagnostics will only run ela on devices, not
 * any tests.
 *        
 * NOTES: 
 *
 * RETURNS: 
 *	 0 - Not supported.
 *	 1 - Supported
 *	-1 - System error obtaining data
 *
 */

int
supports_ela() 
{
	int	aix_model_code;

	if (init_iplcb_structure() == -1 ) return (-1);	

	/* Diagnostics is supported on RS6K models, and RS6KSMP models */
	if ( !strcmp(platform,"rs6k") || !strcmp(platform,"rs6ksmp") ) 
		return (0);
	/* If running on rspc platform check model code to see if */
	/* it supports ela. Bit 19 of model code		  */

	if ( !strcmp(platform, "rspc") )
	{
		(void)get_cpu_model(&aix_model_code);
		if(aix_model_code & 0x80000)
			return(1);
	}
	return(0);
}

/*
 * NAME: has_LED_support
 *
 * FUNCTION: This function returns to the calling function a  
 * value indicating whether LED support is present on this
 * platform or not.   
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0 - Not supported
 *	1 - Supported
 *
 */

int 
has_LED_support()
{

	if (init_iplcb_structure() == -1 ) return (-1);	

	/* LED support is on RS6K models, and RS6KSMP models */
	if ( !strcmp(platform,"rs6k") || !strcmp(platform,"rs6ksmp") ) 
		return (1);
        return (0);

}


/*
 * NAME: has_isa_capability
 *
 * FUNCTION: This function returns to the calling function a
 * value indicating whether the system has an ISA bus
 *
 * NOTES:
 *
 * RETURNS:
 *       0 - do not have ISA bus
 *       1 - has ISA bus
 *
 */

int has_isa_capability()
{
	RESIDUAL	*rp;
	int		i;
	int	has_isa=0;
	char	pnp_code[8];

	if (iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB))
		return(-1);

	if ( strcmp(platform, "rspc") )
		return(0);

	rp = (RESIDUAL *) malloc(iplcb_dir.residual_size);
	if(rp == (RESIDUAL *)NULL)
		return(0);

	if (iplcb_get(rp,iplcb_dir.residual_offset,
		iplcb_dir.residual_size,MIOIPLCB))
	{
		free(rp);
		return(0);
	}
	for(i=0; i<rp->ActualNumDevices; i++)
	{			
		if (rp->Devices[i].DeviceId.BusId & ISADEVICE)
		{
			has_isa=1;
			break;
		}
	}
	free(rp);
	return(has_isa);
}

/*
 * NAME: is_mp
 *
 * FUNCTION: This function returns to the calling function a
 * value indicating whether the system is a MP machine or not.
 *
 * NOTES:
 *
 * RETURNS:
 *       0 - Not MP machine
 *       1 - MP machine
 *      -1 - System error obtaining data
 *
 */

int
is_mp()
{
        if (init_iplcb_structure() == -1 ) return (-1);

        return(mp_capable);
}



/*
 * NAME: is_rspc_model 
 *
 * FUNCTION: This function returns to the calling function a  
 * value indicating whether the model is a RSPC model or not.
 *        
 * NOTES: 
 *
 * RETURNS: 
 *	 0 - Not RSPC
 *	 1 - Is RSPC
 *	-1 - System error obtaining data
 *
 */

int
is_rspc_model() 
{
	if (init_iplcb_structure() == -1 ) return (-1);	

	/* Platform is 'rspc' if on 'rspc' model */
	if ( !strcmp(platform,"rspc") ) 
		return (1);
	return(0);
}

/* ^L */
/*
 * NAME: init_iplcb_structure
 *
 * FUNCTION: This function reads the IPL control block and 
 * initializes the data structures. 
 *
 * NOTES: 
 *
 * RETURNS: 
 *	 0 - Ok 
 *	-1 - System error occurred reading IPL data
 *
 */

int 
init_iplcb_structure() 
{

       strcpy(platform,"");
       if (iplcb_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB))
                return(-1);

       strcpy(platform,"rs6k");
 
        /*
         * The ipl_directory varies in size, so the following calculation
         * is intended to determine whether the iplcb_dir.system_info_offset
         * field exists or not. Do this by comparing the address of the
         * system_info_offset against the address of the ipl_info struct, which
         * follows the ipl_directory in memory. The address of the ipl_info
         * struct is calculated by adding the address of the ipl_directory
         * (cast to int to prevent incrementing by size of the struct)
         * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
         * If the address of system_info_offset is less than the address of the
         * ipl_info struct, assume existence and validity of the
         * system_info_offset and system_info_size fields.
         */
        if ((int)(&iplcb_dir.system_info_offset) <
                ((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128)) {
            if((iplcb_dir.system_info_size > 0) &&
                (iplcb_dir.system_info_offset > 0)) {

                if (iplcb_get(&system_info,iplcb_dir.system_info_offset,
                              sizeof(SYSTEM_INFO),MIOIPLCB))
                        return(-1);
                /* Make sure pkg_desc field is there */
                if( ((long unsigned) &system_info.pkg_descriptor + 16 -
                       (long unsigned) &system_info )
                                <= iplcb_dir.system_info_size)  {
                        if(system_info.pkg_descriptor[0] != '\0')
                                strcpy(platform,system_info.pkg_descriptor);
                }
	        if (system_info.num_of_procs > 1)
                	mp_capable = 1;     /* multi-processor capable */

            }
        }
	if( __rspc() ) strcpy(platform,"rspc");
        if( __rs6k_smp_mca() ) strcpy(platform,"rs6ksmp");
        if( (!strcmp(platform,"rs6k")) && (mp_capable ==  1))
                strcpy(platform,"rs6ksmp");
	return (0);
}


/* ^L */
/*
 * NAME: iplcb_get
 *
 * FUNCTION: Read in section of the IPL control block.  The directory
 *              section starts at offset 128.
 *
 * NOTES: 
 *
 * RETURNS: 
 *	 0 - OK
 *	-1 - error
 *
 */

int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
        int             fd;             /* file descriptor */
        MACH_DD_IO      mdd;


        if ((fd = open("/dev/nvram",0)) < 0) {
                return(-1);
        }

        mdd.md_addr = (long)address;
        mdd.md_data = dest;
        mdd.md_size = (long)num_bytes;
        mdd.md_incr = MV_BYTE;

        if (ioctl(fd,iocall,&mdd)) {
                return(-1);
        }

        close(fd);
        return(0);
}

/*  */
/*
 * NAME: diag_get_device_flag
 *
 * FUNCTION: Obtain device flag from residual data information.
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0  if find device flag
 *	-1 if error found during search (i.e fail to obtain residual data)
 *	-2 if fail to find device.
 *
 */

int diag_get_device_flag(char *devname, long *Flag)
{

	RESIDUAL	*rp;
	int		i, j, found=0;
	struct	CuDv	*cudv;
	struct listinfo obj_info;
	char	criteria[128];
	char	pnp_code[8];
	char	connwhere[32];
	char	serial[16];
	struct	CuAt *cuat;
	uchar	devfuncnum;	/* DevFuncNumber */
	uchar	par_bus_num;	/* PCI bus number of parent PCI bus */
	int	cnt;

	*Flag = 0;
	if (!is_rspc_model()) return(0);

	if (iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB))
		return(-1);

	rp = (RESIDUAL *) malloc(iplcb_dir.residual_size);
	if(rp == (RESIDUAL *)NULL)
		return(-1);

	if (iplcb_get(rp,iplcb_dir.residual_offset,
		iplcb_dir.residual_size,MIOIPLCB))
	{
		free(rp);
		return(-1);
	}

	sprintf(criteria, "name = %s", devname);
	cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, criteria,
			&obj_info, 1, 2);
	if( (cudv == (struct CuDv *)-1) ||
	    (cudv == (struct CuDv *)NULL) )
	{
		free(rp);
		return(-1);
	}

	/* device subclass of PCI, obtain bus number and  */
	/* devfuncnumber to match with residual data.     */

	if(!strncmp(cudv->PdDvLn->subclass, "pci", 3))
	{
		devfuncnum = (uchar)strtoul(cudv->connwhere,NULL,10);
		cuat = (struct CuAt *)getattr(cudv->parent, "bus_number", FALSE, &cnt);
		if (cuat == NULL)
			return(-1);
		par_bus_num = (uchar)strtoul(cuat->value, (char **)NULL, 0);
	}


	for (j=0; j<rp->ActualNumDevices; j++)
	{
		/* Match L2 cache information */

		if(!strncmp(cudv->PdDvLn->type, "L2cache", 7))
		{
			decode_dev((uint)rp->Devices[j].DeviceId.DevId,
				pnp_code);
			if(!strcmp(pnp_code,PNPL2))
			{
				*Flag = rp->Devices[j].DeviceId.Flags;
				found=1;
				break;
			}
		}

		if(!strncmp(cudv->PdDvLn->subclass, "pci", 3))
		{

			if ( (rp->Devices[j].DeviceId.BusId & PCIDEVICE) &&
			     (rp->Devices[j].BusAccess.PnPAccess.CSN == 
					par_bus_num) &&
			     (rp->Devices[j].BusAccess.PnPAccess.LogicalDevNumber == devfuncnum) )
			{
				*Flag = rp->Devices[j].DeviceId.Flags;
				found=1;
				break;
			} else
				continue; /* Go to next entry */

		}

		/* ISA device information */
		/* connwhere has PNP id and Serial number */

		else if(!strncmp(cudv->PdDvLn->subclass, "isa", 3))
		{
			if(rp->Devices[j].DeviceId.BusId & ISADEVICE)
			{
				sprintf(serial, "%x",
					rp->Devices[j].DeviceId.SerialNum);
				decode_dev((uint)rp->Devices[j].DeviceId.DevId,
					pnp_code);
				sprintf(connwhere, "%s%s", pnp_code, serial);
				if(!strcmp(cudv->connwhere, connwhere))
				{
					*Flag = rp->Devices[j].DeviceId.Flags;
					found=1;
					break;
				}
			}
			else
				continue;
		}
	}

	free(rp);
	diag_free_list(cudv, &obj_info);
	return((found=1) ? 0 : -2);
}

/*  */
/*
 * NAME: diag_get_pdbits
 *
 * FUNCTION: Obtain pd bits from residual data information.
 *
 * NOTES: 
 *
 * RETURNS: 
 *	0  if found information. num_entries will show number of
 *         entries in pdbits array.
 *	-1 if error found during search (i.e fail to obtain residual data)
 *
 */

int diag_get_pdbits(int *num_entries, uint **pdbits,
		ulong **ppcmem)
{

	RESIDUAL	*rp;
	PnP_TAG_PACKET	*pnpheap;
	int		i, done=0;
	int	offset=0;
	int	start;
	uint	*tptr;
	ulong *ppcmem_ptr;
	char	pnp_code[8];

	*num_entries = 0;
	*pdbits = (uint *)NULL;
	*ppcmem = (ulong *)NULL;
	if (!is_rspc_model()) return(0);

	if (iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB))
		return(-1);

	rp = (RESIDUAL *) malloc(iplcb_dir.residual_size);
	if(rp == (RESIDUAL *)NULL)
		return(-1);

	if (iplcb_get(rp,iplcb_dir.residual_offset,
		iplcb_dir.residual_size,MIOIPLCB))
	{
		free(rp);
		return(-1);
	}

	/* Format of PNP heap where PD array is: */
	/* 00000000: 75 01 24 4D 05 80 84 09 - 00 0D EB EB FF FF FF FF */
	/* 00000010: FF FF 78 78 78 47 01 14 - 08 14 08 01 01 47 01 1C */

	for (i=0; i<rp->ActualNumDevices; i++)
	{

		decode_dev((uint)rp->Devices[i].DeviceId.DevId, pnp_code);
		if(!strcmp(pnp_code,PNPmemctl))
		{
			while(!done)
			{
				/* Search the device pnp heap starting at */
				/* the Allocated Offset, until the tag    */
				/* 0x84 is found. Next check the validity */
				/* by looking for the 0xd in byte 3.	  */

				start=rp->Devices[i].AllocatedOffset;
				pnpheap = (PnP_TAG_PACKET *)&rp->DevicePnPHeap
					[start+offset];
				switch(rp->DevicePnPHeap[start+offset])
				{
				case 0x84:
					if((uint)rp->DevicePnPHeap[start+offset+3] == 0xd)
					{
						*num_entries = 
						    (uint)rp->DevicePnPHeap[start+offset+1] +
						    ((uint)rp->DevicePnPHeap[start+offset+2]*256) - 1;
						tptr = (uint *)calloc(*num_entries, sizeof(uint));
						for(i=0; i<*num_entries; i++)
							tptr[i] = (uint)rp->DevicePnPHeap[start+offset+4+i];
						offset += (*num_entries + 3);
						*pdbits = tptr;
						done=1;
					} else
						offset++;
					break;
				case 0x75:
					offset +=6;
					break;
				case 0x78:
				case 0x79:
					/* End of packet */

					done=1;
					break;
				default:
					offset++;
					break;
				}
			}
			break;
		}
	}
	if(*num_entries > 0)
	{
		ppcmem_ptr = (ulong *)calloc(*num_entries, sizeof(ulong));
		for(i=0; i < *num_entries; i++)
			ppcmem_ptr[i] = rp->Memories[i].SIMMSize;
		*ppcmem = ppcmem_ptr;
	}
	free(rp);
	return(0);
}

/*  */
/*
 * NAME: diag_get_resid_errlg
 *
 * FUNCTION: Obtain error log data from residual data
 *
 * NOTES:  Look at RESIDUAL.VitalProductData.RAMErrorLogOffset
 *         if not null, use that offset to look in PnPHeap.
 *
 * RETURNS: 
 *	0  if found information. num_entries will show number of
 *         entries in error log array.
 *	-1 if error found during search (i.e fail to obtain residual data)
 *
 */

int diag_get_resid_errlg(int *num_entries, uchar **errlog)
{

	RESIDUAL	*rp;
	PnP_TAG_PACKET	*pnpheap;
	int		i,done=0;
	int	offset=0;
	int	start;
	uchar 	*tptr;

	*num_entries = 0;
	*errlog = (uchar *)NULL;
	if (!is_rspc_model()) return(0);

	if (iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB))
		return(-1);

	rp = (RESIDUAL *) malloc(iplcb_dir.residual_size);
	if(rp == (RESIDUAL *)NULL)
		return(-1);

	if (iplcb_get(rp,iplcb_dir.residual_offset,
		iplcb_dir.residual_size,MIOIPLCB))
	{
		free(rp);
		return(-1);
	}

	/*
	if(rp->VitalProductData.RAMErrorLogOffset != 0)
	*/
	if ( 1 )
	{
		while(!done)
		{
		/* Search the device pnp heap starting at */
		/* the Allocated Offset, until the tag    */
		/* 0x84 is found. Next check the validity */
		/* by looking for the 0xd in byte 3.	  */

			/*
			start=rp->VitalProductData.RAMErrorLogOffset;
			*/
			pnpheap = (PnP_TAG_PACKET *)&rp->DevicePnPHeap
					[start+offset];
			switch(rp->DevicePnPHeap[start+offset])
			{
			case 0x84:
				if((uint)rp->DevicePnPHeap[start+offset+3] == 0xf)
				{
					*num_entries = 
					    (uint)rp->DevicePnPHeap[start+offset+1] +
					    ((uint)rp->DevicePnPHeap[start+offset+2]*256) - 1;
					    tptr = (uchar *)calloc(*num_entries, sizeof(uchar));
					    for(i=0; i<*num_entries; i++)
						tptr[i] = (uint)rp->DevicePnPHeap[start+offset+4+i];
					    offset += (*num_entries + 3);
					    *errlog = tptr;
					    done=1;
				} else
					offset++;
				break;
			case 0x75:
				offset +=6;
				break;
			case 0x78:
			case 0x79:
				/* End of packet */

				done=1;
				break;
			default:
				offset++;
			}
		}
	}
	free(rp);
	return(0);

}


/* ---------------------------------------------------------------------------*
 *
 * NAME: decode_dev
 *
 * FUNCTION:  Decode the devid into a string
 *
 * RETURNS: A pointer to the pnp device number
 * 
 * ---------------------------------------------------------------------------*/
void decode_dev(u_int devid, char *pnp_code)
{
        /* Convert back to string rep first */

	int     i,j;
	uchar	tmp;


	pnp_code[0] = ((devid >> 26) & 0x0000001F)+'@';
	pnp_code[1] = ((devid >> 21) & 0x0000001F)+'@';
	pnp_code[2] = ((devid >> 16) & 0x0000001F)+'@';



	for (i=3, j=3; i<7; i++, j--) {
		/* Pick out hex digit */
		tmp = (devid >> (j*4)) & 0x0000000F;

		/* Convert to ascii character */
        	if(tmp > 9)
                	pnp_code[i] = tmp -9 +'@';
        	else
                	pnp_code[i] = tmp +'0';
        }

	/* Add null delimiter */
	pnp_code[7]='\0';
	return;
}
/* ---------------------------------------------------------------------------*
 *
 * NAME: swap_endian 
 *
 * FUNCTION:  swap endian order
 *
 * RETURNS: nothing
 *
 * ---------------------------------------------------------------------------*/
u_int swap_endian(u_int num)
{
   return(((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
          ((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
}

