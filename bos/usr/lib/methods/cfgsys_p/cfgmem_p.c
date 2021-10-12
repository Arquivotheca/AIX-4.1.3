#ifndef lint
static char sccsid[] = "@(#)22 1.3 src/bos/usr/lib/methods/cfgsys_p/cfgmem_p.c, cfgmethods, bos411, 9430C411a 7/12/94 11:22:02";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Memory Configuration Functions
 *
 * FUNCTIONS: cfgmem, get_cudv, get_card_size, find_bad_bits, find_bad_simms,
 *	      report_bad_simm, report_bad_card, report_pair_failure,
 *	      get_desc_val, get_mem_vpd_MP
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */                                                                   
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include "cfgdebug.h"
#include <sys/errids.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <pgs_novram.h>

/* Note:  These must match defines in cfgsys.c */
#define RS1_MODEL		0x00000001
#define RS1_XIO_MODEL		0x01000000
#define RSC_MODEL		0x02000000
#define RS2_MODEL		0x04000000
#define PowerPC_MODEL		0x08000000 
#define PowerPC_MP_MODEL        0x080000A0

void get_mem_vpd_MP();
void report_bad_simm();
void report_bad_card();
void report_pair_failure();

extern struct Class 	*cusdev;	/* customized devices class ptr */
extern struct Class 	*cusatt;	/* customized attributes class ptr */
extern struct Class	*preatt;	/* predefined attributes class ptr */
extern struct Class	*predev;	/* predefined devices class ptr */
extern struct Class	*cusvpd;	/* customized vpd class ptr */

struct config_table config_tb;          /* structure for configuration table built by the BUMP */

/* Memory model type enumerations for lookup tables */
enum model_types {	cache_128,
			cache_64_floor,
			cache_64_table,
			RSC
} memory_model;

/* The following lookup table provides a correlation between a memory card's */
/* slot number and the first extent which corresponds to it based on memory */
/* model.  A value of -1 indicates that the slot is not supported by the */
/* particular memory model, and a value of -2 indicates a slot containing */
/* the second card in a paired set. */
int first_extent[3][8] = {
	/* 128 byte cache line models */
	{ 12, 4, 8, 0, -2, -2, -2, -2 },
	/* 64 byte cache line models (floorstanding) */
	{ 14, 6, 10, 2, 12, 4, 8, 0 },
	/* 64 byte cache line models (tabletop) */
	{ -1, 2, 0, -1, -1, -1, -1, -1 }
};

/* This table indicates the number of extents per card based on memory model */
int extents_per_card[3] = { 4, 2, 4 };

/* This table yields the simm_info entry for each card ( in ABCDEFGH order ) */
int simm_entry_for_card[3][8] = {
	{ 7,5,6,4,3,1,2,0 },	/* 128 byte cache line models */
	{ 7,3,5,1,6,2,4,0 },	/* 64 byte cache line models (floorstanding) */
	{ 8,1,0,8,8,8,8,8 }	/* 64 byte cache line models (tabletop) */
				/* ( 8 = invalid card name ) */
	};

int	simm_nibble[8] = 
	{
		0x0000000F, /* Simm #1 */
		0x000F0000, /* Simm #2 */
		0x00000F00, /* Simm #3 */
		0x0F000000, /* Simm #4 */
		0x000000F0, /* Simm #5 */
		0x00F00000, /* Simm #6 */
		0x0000F000, /* Simm #7 */
		0xF0000000  /* Simm #8 */
	};

char *memslots = "ABCDEFGH";

/* To find the size in bytes of an extent: */
#define SIZE_OF_EXTENT(extent)  (( -iplcb_info->cre[extent] ) << 16)

/* To find the number of bit map bits for an extent: */
#define NUM_BITS_FOR_EXTENT(extent) \
		( SIZE_OF_EXTENT(extent)/iplcb_info->bit_map_bytes_per_bit)

/* To find the number of bit map bytes for an extent: */
#define NUM_BYTES_FOR_EXTENT(extent) ( NUM_BITS_FOR_EXTENT(extent) >> 3 )


/*
 *   vpd_element[]
 *
 *   NOTES:
 *	This array contains lookup information to extract individual bits
 *	from the VPD returned by memory cards. As there is effectively no
 *	pattern to the VPD, the bits must be masked out individually, and
 *	reconstructed into meaningful values. The bit positions are different
 *	for the first, and last four cards in a 128-byte cache line machine,
 *	and different again for 64-byte cache line models.
 */
struct {
	char	vpd_mnem[3];		/* 2-Char Mnemonic for VPD element   */
	int	store_in_CuVPD;		/* Whether to add to CuVPD */
	struct {
		char	byte_no;	/* Byte no. within VPD for card(pair)*/
		char	byte_mask;	/* Bit within byte                   */
	}  bit_location[3][8]; 
	/* bit_location contains locations of up to 8 bits for each of three */
	/* situations: */
	/* bit_location[0][0..7] is for 128-byte cache Cards D,B,C, & A */
	/* bit_location[1][0..7] is for 128-byte cache Cards H,F,G, & E */
	/* bit_location[2][0..7] is for all 64-byte cache Cards */
	/* The first bit is the lowest order one. */
	/* Where byte_mask == 0, there are no more bits available */
} vpd_element[] =
{
    { "S#", 0, { 		/* Simm Number */
	{{4,0x40},{16,8},{16,0x40},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{12,0x40},{16,2},{12,0x80},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{4,0x10},{4,0x20},{4,0x80},{0,0},{0,0},{0,0},{0,0},{0,0}}
    } },
    { "Z3", 1, {		/* Type of smallest Simm on card */
	{{0,0x40},{0,0x80},{5,0x20},{5,0x40},{17,8},{17,0x40},{0,0},{0,0}},
	{{8,0x40},{8,0x80},{13,0x20},{13,0x40},{17,2},{13,0x80},{0,0},{0,0}},
	{{0,0x20},{0,0x80},{6,4},{6,0x10},{6,0x20},{6,0x80},{0,0},{0,0}}
    } },
    { "EC", 1, {		/* EC-Level for card */
	{{16,0x20},{0,1},{16,0x80},{0,2},{0,4},{0,8},{0,0x10},{0,0}},
	{{8,1},{8,2},{16,4},{8,4},{8,8},{8,0x10},{16,0x10},{0,0}},
	{{1,1},{1,4},{1,0x10},{1,0x40},{8,0x40},{0,1},{0,2},{0,0}}
    } },
    { "Z0", 1, {		/* EC-Level for Left chip on card */
	{{5,2},{5,4},{5,8},{5,0x10},{0,0},{0,0},{0,0},{0,0}},
	{{13,2},{13,4},{13,8},{13,0x10},{0,0},{0,0},{0,0},{0,0}},
	{{3,0x10},{3,0x40},{10,0x40},{2,1},{0,0},{0,0},{0,0},{0,0}}
    } },
    { "Z1", 1, {		/* EC-Level for Right chip on card */
	{{17,0x80},{1,2},{1,4},{1,8},{0,0},{0,0},{0,0},{0,0}},
	{{17,4},{9,4},{9,8},{9,0x10},{0,0},{0,0},{0,0},{0,0}},
	{{7,0x10},{7,0x40},{7,0x80},{6,2},{0,0},{0,0},{0,0},{0,0}}
    } },
    { "Z2", 1, {		/* EC-Level for Center chip on card */
	{{1,0x10},{1,0x20},{1,0x40},{1,0x80},{0,0},{0,0},{0,0},{0,0}},
	{{17,0x10},{9,0x20},{9,0x40},{9,0x80},{0,0},{0,0},{0,0},{0,0}},
	{{2,2},{2,8},{2,0x20},{2,0x80},{0,0},{0,0},{0,0},{0,0}}
    } },
    { "", 0, {
	{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
	{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}
    } }
};

int	csize[8];		/* Card sizes */
int	cbadbits[8];		/* Number of bad bits for each card */
int	cbadsimms[8];		/* Number of bad simms for each card */
char	*msg_no[8];		/* pointers to text desc msg nos */
char	msg_values[ATTRVALSIZE];/* values from pdat for message */
char	mem_ut[UNIQUESIZE] = "";	/* Type of memory currently being configured */
char	msg_no_ut[UNIQUESIZE] = "";	/* Unique type upon which msg_no array is based */

#define	MEM_UTYPE	"memory/sys/memory"
#define	SIMM_UTYPE	"memory/sys/simm"


/*
 * NAME: cfgmem
 *
 * FUNCTION: Determines what memory cards are present and configures them
 *	     into the system.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
cfgmem(pname,iplcb_dir,iplcb_info,model)
char	*pname;				/* parent name, i.e. sys object name */
IPL_DIRECTORY	*iplcb_dir;		/* Pointer to IPL Control block Dir */
IPL_INFO	*iplcb_info;		/* Pointer to IPL Control block Info*/
uint		model;			/* Model type we are dealing with  */
{
   int		rc;		/* return code from subroutines */


   /* Process memory based on type of model 	*/
   /* First, try to process a mem table 	*/

   DEBUG_0("Calling cfgmem_MEM_DATA\n")
   rc = cfgmem_MEM_DATA(pname, iplcb_dir, iplcb_info);
	
   /* If a table was not found, check for other ways 	*/ 
   /* to process memory.				*/
   if (rc == -1)
   {
	 DEBUG_0("****Called no cfgmem routine!****\n")
	 rc = 0;
   }

   return(rc);
}


/*
 * cfgmem_MEM_DATA
 *
 * FUNCTION: Processes the memdata structure stored iplros and
 *		configures this memory into the system.
 *
 * RETURNS:  error code.  0 means no error.
 */
int
cfgmem_MEM_DATA(pname, iplcb_dir, iplcb_info)
char	*pname;				/* parent name, i.e. sys object name */
IPL_DIRECTORY	*iplcb_dir;		/* Pointer to IPL Control block Dir */
IPL_INFO	*iplcb_info;		/* Pointer to IPL Control block Info*/
{
	struct CuDv	cudv;		/* structure for CuDv object */
	struct PdDv	pddv;
	char	sstring[256];		/* ODM search string */
	char	slot_name;		/* Slot designation, i.e. A-H */
	int	slot;			/* Location of bad simm */
	int	rc;			/* subroutine return codes */
	int	i, j;			/* loop variables */
	int	card_size;		/* Current card's size */

	MEM_DATA memdata;		/* new format memory data structure */
	uint	mem_offset;		/* offset into memory data structure array */

	char	vpd[VPDSIZE];		/* storage for mem card VPD */
	char	tmpstr[10];		/* temp string storage */
	int	fetchPdDv;		/* flag for fetching the PdDv */
	int	num_of_memdata;		/* number of structures in memdata */
    int     sys_vpd;                /* use to store a pointer on config_table filled by the firmware */

	/*
	 * Determine if there is a memdata table.  This involves three checks.
	 *
	 * 1.  Address check.
	 * The ipl_directory varies in size, so the following calculation
	 * is intended to determine whether the iplcb_dir.mem_data_offset
	 * field exists or not.  Do this by comparing the address of the
	 * mem_data_offset against the address of the ipl_info struct, which
	 * follows the ipl_directory in memory.  The address of the ipl_info
	 * struct is calculated by adding the address of the ipl_directory
	 * (cast to char * to prevent incrementing by size of the struct)
	 * to the ipl_info_offset (subtract 128 for 32 4 byte GP registers).
	 * If the address of mem_data_offset is less than the address of the
	 * ipl_info struct, assume existence and validity of the mem_data_offset.
	 *
	 * 2.  Do the memdata_info_offset and memdata_info_size both equal 0?  If so,
	 * the memdata table is not valid.
	 *
	 * 3.  Check to see if the number of structures is 0.  If so, the memdata
 	 * table is not valid. 
 	 *
	 */

	/* Determine the total memory size and save as sys0 attribute */
	/* Check 1.  If the address of the table conflicts with other info, return -1 */
	if (&iplcb_dir->mem_data_offset >=
		((int)(iplcb_dir) + iplcb_dir->ipl_info_offset-128)) 
		return(-1);

	/* It looks like there is a table.  		 	*/
	/* Check 2.  If the offset OR the size is 0, return -1	*/
	if ((iplcb_dir->mem_data_offset == 0) || (iplcb_dir->mem_data_size == 0))
		return(-1);
	
	/* get first memdata section 		*/
	/* set up memdata offset from IPLCB 	*/
	mem_offset = iplcb_dir->mem_data_offset;

	/* read in the first memdata structure  */
	rc = mdd_get(&memdata, mem_offset, sizeof(memdata), MIOIPLCB);
	if (rc)
		return(rc);

	num_of_memdata = memdata.num_of_structs;
	
	/* Check 3.  If the number of mems inside the table says "0", return -1 */
	if (num_of_memdata == 0)
		return(-1);

	/* If we've reached this point, then we can assume that there 	*/
	/* is a memdata table and it is valid. 				*/

	/* Get the first pddv.  If the type does not change, we won't	*/
	/* have to do another fetch in the loop. 			*/

	get_pddv(memdata.card_or_simm_indicator, &pddv);

	num_of_memdata = memdata.num_of_structs;	

	for (i=0; i<num_of_memdata; i++)
	{	
		/* get the memdata.  The first will be retrieved twice, but  */
		/* by doing the get at the top of the loop, we avoid reading */
		/* beyond the memdata tables.				     */
	    	rc = mdd_get(&memdata, mem_offset, sizeof(memdata), MIOIPLCB);
		if (rc)
			return(rc);
		
		if (memdata.state != IS_EMPTY)
		{
			/* a card or SIMM is present */
			/* get the pddv - type may have changed */
			get_pddv(memdata.card_or_simm_indicator, &pddv);
	
			/* set up slot_name */
			slot_name = memdata.location[0][3];

			/* set up size */
			card_size = memdata.card_or_SIMM_size;
			
			rc = get_cudv(&pddv,slot_name,card_size,&cudv,pname);

			if (cudv.status != AVAILABLE)
			{

			     	DEBUG_0("**** Updating memory attributes ****")
			        
				rc = set_msg_no(cudv.name, mem_ut, card_size);

				if (rc)
					return(rc);
	
				if (memdata.card_or_simm_indicator != SIMM)
				{
					/* set up the "card_type" attribute */
					sprintf(sstring,"0x%x",memdata.PD_bits);
					setattr(cudv.name,"type",sstring);
			
					/* Set card EC attribute */
					sprintf(sstring,"0x%x",memdata.EC_level);
					setattr(cudv.name,"cardec",sstring);
				} 
	

				/* set "card size" attribute */
				sprintf(sstring,"%d",card_size);
				setattr(cudv.name,"size",sstring);
	
				/* set up VPD  			*/
				/* get the configuration table in novram */
				DEBUG_0("Get the configuration table built by the BUMP\n")
				rc = mdd_get( &config_tb, iplcb_dir->system_vpd_offset, sizeof(struct config_table), MIONVGET);
			    if (rc)
				  {
					err_exit(rc);
				  }
				get_mem_vpd_MP(cudv.name, config_tb.mem[i].board_vpd);

				/* set up location in cudv */
				sprintf(cudv.location, "%c%c-%c%c", 
					memdata.location[0][0],
					memdata.location[0][1],
					memdata.location[0][2],
					memdata.location[0][3]);
				
				rc = update_cudv(&cudv, cudv.location, 
					pname, mem_ut, &cudv.location[4], TRUE);
				if (rc)
					return(rc);

				if (memdata.state == IS_BAD)
				{
					if (memdata.card_or_simm_indicator == SIMM)
					{
						/* slot name == simm slot and simm # == 0 */
						report_bad_simm(&slot_name, 0);
					} 
					else 	/* bad CARD or bad SIMMS on card */
					{
						if (memdata.num_of_bad_simms == 0)
						{	
							/* report bad card  */
							report_bad_card(&slot_name);
						}
		
						else
						{
							/* report each bad simm			*/
							for (j=1; j<=memdata.num_of_bad_simms; j++)
							{
							    	rc = sscanf(&memdata.location[j][0], "%4d", &slot);
								report_bad_simm(&slot_name, slot);
							}
						}

					}  /* end of indicator == CARD */
				}  /* end of state is bad */
			} /* end of "if not available" section */
		} /* end of if not empty section */
		
		/* update offset and continue loop */
		mem_offset = mem_offset + memdata.struct_size;

	}  /* end of for loop for each mem data structure */
} /* end of cfgmem_MEMDATA */



/*
 * get_pddv
 *
 * FUNCTION: Gets the pddv object for the current
 *		type of memory passed in, either
 *		CARD or SIMM.
 *
 * RETURNS:  error code.  0 means no error.
 */
int 
get_pddv(cur_type, pddv)   
enum entry_indicator cur_type;
struct PdDv *pddv;
{
	int fetchPdDv;
	char sstring[256];
	int  rc;

	/* check to see if we need to update the PdDv and mem_ut */
	/* first, assume we don't need to fetch a new pddv	*/
	fetchPdDv = FALSE;
		if (cur_type == SIMM)
	{
		/* If this is a different type than previous 	*/ 
		/* loop iterations, get the pddv again 		*/
			if (strcmp(mem_ut, SIMM_UTYPE)) 
		{
			fetchPdDv = TRUE;
			strcpy(mem_ut,SIMM_UTYPE);
		}
	}
	else
	{
		/* If this is a different type than previous 	*/ 
		/* loop iterations, get the pddv again 		*/
		if (strcmp(mem_ut, MEM_UTYPE)) 
		{
			fetchPdDv = TRUE;
			strcpy(mem_ut,MEM_UTYPE);
		}
	}
	if (fetchPdDv)
	{
		/* get predefined device object for memory */
		sprintf(sstring, "uniquetype = %s", mem_ut);
		rc = (int)odm_get_first(predev,sstring,pddv);
		if (rc==-1) {
			/* ODM failure */
			DEBUG_0("ODM failure getting memory PdDv\n")
			return(E_ODMGET);
		} else if (rc == 0) {
			/* No memory PdDv present, so ignore memory config */
			/* This allows diskette based boot without memory PdDv's */
			DEBUG_0("No memory PdDv\n")
			return(E_OK);
		}
	}

	return(0);
}

/*
 * NAME: get_cudv
 *
 * FUNCTION: Finds the CuDv object corresponding to a memory card or
 *	     defines a new one if there is no match.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
get_cudv(pddv,slot_name,size,cudv,pname)
struct PdDv	*pddv;		/* pddv for memory object */
int	slot_name;		/* slot name, i.e. A-H */
int	size;			/* size of the memory card */
struct CuDv	*cudv;		/* pointer to CuDv object */
char	*pname;			/* parent device name */

{
	struct CuAt cuatobj;	/* structure for customized attribute obj */
	int	firstnext;	/* indicates get first or next object */
	char	defstr[256];	/* used for invoking define method */
	char	sstring1[256];	/* search string for CuDv object */
	char	sstring2[256];	/* search string for size attribute */
	char	*outp;		/* stdout from define method */
	long	tmp_size;	/* */
	int	rc;		/* ODM return code */
#define DEFSTR1	"-c memory -s sys -t memory -p %s -w %c"
#define DEFSTR2	"-c memory -s sys -t simm -p %s -w %c"

	sprintf(sstring1,"PdDvLn =%s AND connwhere = %c",pddv->uniquetype,slot_name);

	for(firstnext = ODM_FIRST;;firstnext = ODM_NEXT) {
		/* find a CuDv obj with desired parent and slot connection */
		rc = (int)odm_get_obj( cusdev, sstring1,cudv, firstnext);
		if (rc == -1) {
			DEBUG_1("Unable to read CuDv : %s\n", sstring1)
			return(E_ODMGET);
		} else if (rc == 0) {
			/* No CuDv object for card, so define one */
			if (!strcmp(pddv->uniquetype, SIMM_UTYPE)) {
				DEBUG_2("%d Mb simm in slot %c is new!\n",
							size, slot_name)
				sprintf(defstr, DEFSTR2, pname, slot_name);
			} else {
				DEBUG_2("%d Mb card in slot %c is new!\n",
							size, slot_name)
				sprintf(defstr, DEFSTR1, pname, slot_name);
			}

			rc = odm_run_method(pddv->Define,defstr,&outp,NULL);
			if (rc) {
				DEBUG_0("Define failed\n")
				return(E_ODMRUNMETHOD);
			}

			/* Its defined, so now get CuDv object */
			outp[strlen(outp)-1] = '\0';
			DEBUG_1("Defined device %s\n",outp)
			sprintf(defstr,"name = %s",outp);
			rc = (int)odm_get_first(cusdev,defstr,cudv);
			if (rc == -1) {
				return(E_ODMGET);
			} else if (rc == 0) {
				DEBUG_0("Failed to get CuDv\n")
				return(E_NOCuDv);
			}
			return(0);
		}

		/* Found a CuDv for same slot, now check size */
		sprintf(sstring2,"name = %s AND attribute = size",cudv->name);
		DEBUG_1("Finding size:%s\n", sstring2)

		/* get CuDv obj's corresponding size attribute */
		rc = (int)odm_get_first(cusatt,sstring2,&cuatobj);
		if (rc == 0) {
			DEBUG_1("Cant find size of %s\n",cudv->name)
			continue;		/* Check next CuDv object */
		} else if (rc == -1) {
			DEBUG_1("Error getting CuAt: %s\n",sstring2)
			return(E_ODMGET);
		}

		/* Compare sizes */
		tmp_size = strtoul(cuatobj.value,(char**)NULL,0);
		if (tmp_size == (long)size) {
			/* Found desired match */
			DEBUG_1("Memory card matches entry: %s\n",cudv->name)
			break;
		} else {
			/* Does not match, check next CuDv object */
			DEBUG_2("Card does not match old %s Mb card %s\n",
						cuatobj.value, cudv->name)
		}
	}
	return(0);
}

/*
 * NAME: set_msg_no(lname, ut, cardsize)
 *
 * FUNCTION: Sets up array of messages for card/simm sizes
 *
 * RETURNS:  
 */

int
set_msg_no(lname,ut,cardsize)
char 	*lname;			/* cudv name */
char 	*ut;			/* unique type for device */
int	cardsize;		/* size of card */
{
	char	sstring[256];		/* ODM search string */
	int	test_size;		/* used in determining desc mesg no. */
	int	rc;			/* return code */
	int	index;			/* loop control variable */
	struct PdAt	pdat;		/* structure for PdAt object */

	/* if the array of messages has not yet been set up OR
	 *	the type of memory has changed, then set up the 
	 *	array.
	 */
	if ((msg_no_ut[0] == "\0") || strcmp(ut, msg_no_ut))
	{
		/* get predefined description */
		sprintf(sstring, "uniquetype = %s AND type = T", ut);
		rc = (int)odm_get_first(preatt,sstring,&pdat);
		if (rc==-1) {
			/* ODM failure */
			DEBUG_0("ODM failure getting description attribute \n")
			return(E_ODMGET);
		} else if (rc == 0) {
			/* No description attribute present */
			DEBUG_0("No description attribute\n")
			msg_no[0] = (char *)NULL;
		} else {
			/* set up ut to show what array is based on */
			strcpy(msg_no_ut, ut);
			strcpy(msg_values, pdat.values);
			msg_no[0] = strtok(msg_values,",");
			for (index=1; index<8; index++) {
				msg_no[index] = strtok((char *)NULL,",");
			}
		}

	} /* end of setting up array */

	/* test_size is used to compare against card or simm size */
	/* in order to determine which message number to use.  The */
	/* message numbers for memory cards must correspond to: */
	/* default, 8MB, 16MB, 32MB, 64MB, 128MB, 256MB, 512MB and */
	/* must be in that order.  The message numbers for memory */
	/* simms must correspond to: default, 1MB, 2MB, 4MB, 8MB, */
	/* 16MB, 32MB, 64MB and must be that order. */
	if ( !strcmp(ut, SIMM_UTYPE)) {
		/* max size for simm */
		test_size = 64;
	} else {
		/* max size for card */
		test_size = 512;
	}
	/* check sizes from highest to lowest */
	index = 7;
	while(index > 0) {
		if (test_size == cardsize)
			break;
		index--;
		test_size = test_size / 2;
	}
	/* Set desc attribute */
	setattr(lname,"desc",msg_no[index]);

	return(0);

}  /* end of set_msg_no */




/*
 * NAME: get_card_size
 *
 * FUNCTION: Determines the size of a memory card.
 *
 * RETURNS:  card size in MB.
 */

int
get_card_size(slot,num_bad_bits,iplcb_info,bit_map_ptr)
int	slot;			/* card slot number */
int	*num_bad_bits;		/* where to return num of bad bits */
IPL_INFO *iplcb_info;		/* ptr to IPL control blk info section */
char	*bit_map_ptr;		/* ptr to bit map in IPL control block */
{

	int	extent;			/* loop variable for going thru exts */
	int	start_extent;		/* first extent for card or pair */
	int	end_extent;		/* for terminating loop */
	int	extent_size;		/* size of extent in bytes */
	int	total_size;		/* total size in bytes of all extents */
	int	card_size;		/* Card size in MB */
	int	total_bad_bits;		/* total bad bits for card or pair */
	int	bad_bits;		/* bad bits in extent */
	int	i;			/* loop control	*/


	total_size = 0;
	total_bad_bits = 0;

	start_extent = first_extent[memory_model][slot];

	if (start_extent == -1) {
		/* then this slot is not supported on this model */
		card_size = 0;
		total_bad_bits = 0;
	} else if (start_extent == -2) {
		/* then this card is the second card in a paired set */
		card_size = csize[slot-4];
		total_bad_bits = cbadbits[slot-4];
	} else {
		/* then this slot number is supported by this model */

		/* initialize the first extent */
		extent = start_extent;

		/* loop through all extents for card */
		for (i=0; i<extents_per_card[memory_model];i++)
		{
			/* get size of extent */
			extent_size = SIZE_OF_EXTENT( extent );
			DEBUG_2("EXTENT %d: size = %ld bytes\n",
							extent,extent_size)

			/* if size is not zero, process extent */
			if (extent_size != 0) 
			{

				/* Add size of extent to running total */
				total_size = total_size + extent_size;

				/* determine number of bad bits in extent */
				bad_bits = find_bad_bits(extent,iplcb_info,bit_map_ptr);
				DEBUG_2("Number of bad bits in extent: %d ( / %d )\n",
						bad_bits,NUM_BITS_FOR_EXTENT(extent) )

				/* Add bad bits to running total */
				total_bad_bits = total_bad_bits + bad_bits;
			}

			/* increment the extent number */
			/* for 64 line cache table top models, we need to process the 
				following extents:
				slot B:  2, 3, 6, 7
				slot c:  0, 1, 4, 5
			*/
			if ((memory_model == cache_64_table) && ((extent==1) || (extent==3)))
				extent = extent + 3;
			else
				extent++;

		}

		/* Compute card size in MB */
		if (memory_model == cache_128)
			card_size = total_size >> 21;
		else
			card_size = total_size >> 20;
	}

	/* Save in card size and bad-bit tables */
	csize[slot] = card_size;
	cbadbits[slot] = total_bad_bits;

	/* return total number of bad bits */
	*num_bad_bits = total_bad_bits;

	return(card_size);
}





/*
 * NAME: find_bad_bits
 *
 * FUNCTION: 
 * 	Finds the number of bits in the bitmap which represent bad areas
 *	within the extent passed in.
 *
 * RETURNS:
 *	Number of bad bits found ( as an integer )
 */

int
find_bad_bits( extent, iplcb_info, bit_map_ptr )
int	extent;			/* Extent to be chaecked */
IPL_INFO *iplcb_info;		/* info section of IPL control block */
char	*bit_map_ptr;		/* bit map section of IPL control block */

{
	unsigned char *ptr;		/* used as a pointer to bit map */
	int	bm_offset;		/* offset in bit map for extent */
	int	bytes_to_check;		/* bytes to check in bit map */
	unsigned char byte_from_bitmap;	/* byte read from bit map */
	int 	bad_bits_found;		/* running total of bad bits */

	bad_bits_found = 0;
	bytes_to_check = NUM_BYTES_FOR_EXTENT(extent);

	DEBUG_2("Checking extent %d, number of bytes to check = %d\n",
							extent,bytes_to_check )

	/* Find the address of the first bit map byte for this extent */
	bm_offset = ((iplcb_info->cre[extent] & 0xffff0000) /
			iplcb_info->bit_map_bytes_per_bit) >> 3;

	DEBUG_1("extent offset = 0x%x\n",
		iplcb_info->cre[extent] & 0xffff0000 )
	DEBUG_1("offset within bitmap = 0x%x\n", bm_offset)

	ptr = (char *) ((int)bit_map_ptr + bm_offset);

	while( bytes_to_check-- ) {
		byte_from_bitmap = *ptr++;
		while( byte_from_bitmap != 0 ) {
			if (byte_from_bitmap & 1)
				bad_bits_found++;
			byte_from_bitmap >>= 1;
		}
	}
	return(bad_bits_found);
}	


/*
 * NAME: find_bad_simms
 *
 * FUNCTION: 
 *	Finds the number of bad simms indicated by the SIMM_INFO array for
 *	the card passed in.
 *
 * RETURNS:
 *	The number of bad simms ( as an integer 0..8 ), and if there is a bad
 *	simm, the simm number is stored via the bad_simm_ptr.
 */

int
find_bad_simms( slot_no, bad_simm_ptr, iplcb_info )
int  slot_no;			/* slot number of card, i.e. 0-7 */
int *bad_simm_ptr;		/* where bad simm number is to be returned */
IPL_INFO *iplcb_info;		/* info section of IPL control block */
{
	int		simm_no;	/* for looping through simm numbers */
	unsigned int	card_info;	/* simm info about card */
	int		num_bad_simms;	/* running total of bad simms */

	DEBUG_0("	Scanning for bad simms in card\n")
	num_bad_simms = 0;

	card_info = iplcb_info->SIMM_INFO.slots_of_SIMMs[
		simm_entry_for_card[memory_model][slot_no] ];

	for( simm_no=1; simm_no<=8; simm_no++ )
	{
		if( card_info & simm_nibble[simm_no-1] )
		{
			num_bad_simms++;
			*bad_simm_ptr = simm_no;
		}
	}

	return(num_bad_simms);
}


/*
 * NAME: report_bad_simm
 *
 * FUNCTION: Logs an error ( i.e. One SIMM on a particular card failed )
 *
 * RETURNS: NONE
 */

void
report_bad_simm( slot_name, bad_simm_no )
char  *slot_name;	/* slot name of card, i.e. A-H */
int bad_simm_no;	/* simm number of bad simm */

{
	int rc;		/* return code from errlog() */

	struct {
		struct err_rec0	err_info;
		char card[4];
		int simm_no;
	} simm_err;		/* error log structure */

	DEBUG_2( "Reporting simm %d on card %c is bad\n", bad_simm_no,
		slot_name )

	simm_err.err_info.error_id = ERRID_MEM2;
	strcpy( simm_err.err_info.resource_name, "memory" );
	simm_err.card[0] = *slot_name;
	simm_err.card[1] = '\0';
	simm_err.simm_no = bad_simm_no;
	rc = errlog( (char *)&simm_err, sizeof(simm_err) );
#ifdef CFGDEBUG
	if( rc == -1 )
		DEBUG_0("report_bad_simm() failed\n")
#endif

	return;
}


/*
 * NAME: report_bad_card
 *
 * FUNCTION: Logs an error ( i.e. A card failed to respond, or >1 bad simm
 *	on a particular card )
 *
 * RETURNS: NONE
 */

void
report_bad_card( slot_name )
char  *slot_name;			/* slot name of card, i.e. A-H */

{
	int rc;		/* return code from errlog() */

	struct {
		struct err_rec0 err_info;
		char card;
	} card_err;		/* error log structure */

	DEBUG_1( "Reporting card %c is bad\n", slot_name )

#ifndef ERRID_MEM3
#define ERRID_MEM3 0
#endif

	card_err.err_info.error_id = ERRID_MEM3;
	strcpy( card_err.err_info.resource_name, "memory" );
	card_err.card = *slot_name;
	rc = errlog( (char *)&card_err, sizeof(card_err) );
#ifdef CFGDEBUG
	if( rc == -1 )
		DEBUG_0("report_card_failure() failed\n")
#endif

	return;
}


/*
 * NAME: report_pair_failure
 *
 * FUNCTION: Logs an error.
 *	If it is a 128 byte cache line system, a card pair failure is reported
 *
 * RETURNS: NONE
 */

void
report_pair_failure( slot_no )
int slot_no;		/* slot number of first card in pair, i.e. 0-3 */

{
	int rc;		/* return code from errlog() */

	struct {
		struct err_rec0 err_info;
		char card_1;
		char card_2;
	} pair_err;		/* error log structure */

	DEBUG_2( "	Reporting cards %c, and/or %c are bad\n",
				memslots[slot_no],memslots[slot_no+4])

	pair_err.err_info.error_id = ERRID_MEM1;
	strcpy( pair_err.err_info.resource_name, "memory" );
	pair_err.card_1 = memslots[slot_no];
	pair_err.card_2 = memslots[slot_no+4];
	rc = errlog( (char *)&pair_err, sizeof(pair_err) );
#ifdef CFGDEBUG
	if( rc == -1 )
		DEBUG_0("report_pair_failure() failed\n")
#endif

	return;
}



/*
 * NAME: get_desc_val
 *
 * FUNCTION: Extracts a particular field from the vpd bits for a memory card
 *	returning the value as an integer.
 *
 * RETURNS: The value of the field for the slot_no passed in ( or 0=default )
 *
 */

int
get_desc_val( desc_mnem, slot_no, iplcb_info )
char 	*desc_mnem;	/* "EC", "Z0", etc */
int	slot_no;	/* 0=A, 1=B, etc */
IPL_INFO *iplcb_info;	/* info section of IPL control block */

{
	int	desc_no;
	int	bit_no;
	int	bit_val;
	int	ret_val;
	int	slot_type;
	int	ext_no;		/* extent set number for card */
	int	byte_no;
	char	*vpd_src;
	char	*deb_ptr;
	int	slot1,slot2;

	DEBUG_2("Searching for descriptor %s on card %d\n", desc_mnem, slot_no)

	if (memory_model == cache_128) {
		if (slot_no < 4)
			slot_type = 0;
		else
			slot_type = 1;

		ext_no = first_extent[memory_model][slot_no] / 4;
		vpd_src =&(iplcb_info->MEMCD_VPD.memcd_vpd[(ext_no)*20]);

#ifdef CFGDEBUG
		if (slot_no < 4) {
			slot1 = slot_no;
			slot2 = slot_no + 4;
		} else {
			slot1 = slot_no - 4;
			slot2 = slot_no;
		}

		deb_ptr = vpd_src;
		DEBUG_2("Cards %c & %c VPD: ",memslots[slot1],memslots[slot2])
		
		for( byte_no = 0; byte_no < 20; byte_no++ )
			DEBUG_1("%02X ", *deb_ptr++ )
		DEBUG_0("\n")
#endif
	}
	else
	{
		slot_type = 2;
		ext_no = first_extent[memory_model][slot_no] / 2;
		vpd_src =&(iplcb_info->MEMCD_VPD.memcd_vpd[(ext_no)*12]);
#ifdef CFGDEBUG
		deb_ptr = vpd_src;
		DEBUG_1("Card %c VPD: ", memslots[slot_no] )
		for( byte_no = 0; byte_no < 10; byte_no++ )
			DEBUG_1("%02X ", *deb_ptr++ )
		DEBUG_0("\n")
#endif
	}

	/* Check if the card has VPD: */
	/* .. if not, then return the attribute as value 0 */

	if( *((int *)vpd_src) == 0xffffffff )	/* No VPD on card */
		return(0);
	if( *((int *)vpd_src) == 0x22222222 )	/* Card gave error response */
		return(0);
	if( *((int *)vpd_src) == 0x11111111 )	/* Card not present */
		return(0);

	ret_val = 0;

	for (desc_no=0; vpd_element[desc_no].vpd_mnem[0] != '\0'; desc_no++) {
		if( strcmp( vpd_element[desc_no].vpd_mnem, desc_mnem ) != 0 )
			continue;
		
		for( bit_no = 0, bit_val = 1; ; bit_no++, bit_val <<= 1 ) {
			if( vpd_element[desc_no].
				bit_location[slot_type][bit_no].byte_mask ==0 )
				break;

			if( vpd_src[ vpd_element[desc_no].
				bit_location[slot_type][bit_no].byte_no ] & 
				vpd_element[desc_no].
				bit_location[slot_type][bit_no].byte_mask )

				ret_val += bit_val;
		}
	}
	DEBUG_4("Card %d (%c), *%s=%d\n", slot_no, memslots[slot_no],
		desc_mnem, ret_val )

	return(ret_val);
}

/*
 * NAME: get_mem_vpd_MP
 *
 * FUNCTION: Obtains the VPD ,
 *               and formats them
 *
 *
 */
void
get_mem_vpd_MP(lname, config_table_vpd)
char  *lname;                  /* Device logical name */
char  config_table_vpd[];      /* string containing the VPD of the device */
{
        struct  vpd_head *v_h;
        struct  vpd_field_head *v_f;
        int l, rc, h, k;
        char    *v_p;
        char    value[256] = "";
        char    value_ok[256]= "";
        char    ident[3] = "";

        char    vpd[VPDSIZE];                   /* storage for holding VPD */

        /* Save VPD of the object corresponding to lname */
        memset( vpd, 0, VPDSIZE);
        *vpd = '\0';
        memset(value, 0, 256);
        memset(value_ok, 0, 256);
        memset(ident, 0, 3);

        v_h = (struct vpd_head *)config_table_vpd;
        v_f = (struct vpd_field_head *)((int) v_h + sizeof(*v_h));

        if ( v_h->length != 0)
        {
                l= 0;
                while( (2 * v_h->length) > l) {
                        memcpy(ident, v_f->ident, 2);
                        ident[3] = '\0';
                        DEBUG_3("field VPD  %c%s, field length %d\n", v_f->star, ident, v_f->length);
			if (v_f->length != 0) {
	                        v_p = (char *)v_f + 4;
	                        memcpy(value, v_p, ((2 * v_f->length) - 4));
	                        value[((2 * v_f->length) - 4) + 1] = '\0';
	                        DEBUG_1("field VPD value: %s\n", value);
	                        if ( !strncmp("Y0", ident, 2) || !strncmp("Y1", ident, 2)) {
	                                sprintf(&value_ok[0], "%02x%02x", value[0], value[1]);
	                                for( h=2, k=4; h < ((2 * v_f->length) - 4); h++, k++)
	                                  sprintf(&value_ok[k], "%c", value[h]);
	                        }
							else if ( !strncmp("PC", ident, 2)) {
							  for( h=0, k=0; h < ((2 * v_f->length) - 4); h++, k = k+2)
								sprintf(&value_ok[k], "%02x", value[h]);
							}
	                        else {
	                                strcpy(value_ok, value);
	                        }
	                        DEBUG_1("field VPD decoded: %s\n", value_ok);
	                        l = l + (2 * v_f->length);
	                        v_f = (struct vpd_field_head *) (v_p + ((2 * v_f->length) - 4));

	                        add_descriptor(vpd, ident, value_ok);

	                        memset(value, 0, 256);
	                        memset(value_ok, 0, 256);
	                        memset(ident, 0, 3);
			}
			else {
				break;
			}
                }

                update_vpd( lname, vpd);
        }

        return;
}

