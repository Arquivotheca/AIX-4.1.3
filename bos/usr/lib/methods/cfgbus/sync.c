static char sccsid[] = "@(#)03	1.21.1.11  src/bos/usr/lib/methods/cfgbus/sync.c, cmdbuscf, bos41J, 9511A_all 3/10/95 11:03:12";
/*
 *   COMPONENT_NAME: (CMDBUSCF)
 *
 *   FUNCTIONS: disable_bus_adapters
 *		get_cudv_entry
 *		get_pddv_entry
 *		invoke_adapt_cfg
 *		sync_bus
 *		output_pkgs
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <fcntl.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mdio.h>

#include "bcm.h"


extern	int	allpkg;			/* packaging flag */



/* -------------------------------------------------------------------------- */
/*
 * NAME: get_pddv_entry 
 *
 * FUNCTION: Locates the correct predefine entry matching card id
 *
 * RETURNS: 0 if predefined device information found
 *          E_FINDCHILD if predefined device information NOT found
 *          E_ODMGET if other ODM error encountered
 */

int
get_pddv_entry(phase, busname, cardid, slot, pddv)
int	phase;		/* IPL phase */
char	*busname;	/* parent bus name */
ushort	cardid;		/* card's id */
int	slot;		/* slot containing card */
struct PdDv *pddv;	/* Returned PdDv for card */

{
	struct CuAt	CuAt;	/* to determine if FRS should be called    */
	int    rc;
	char   sstr[80];	/* search string */


	/* Attempt to get device information from PdDv */
	sprintf(sstr, "devid=0x%04x AND subclass=mca", cardid);
	rc = (int)odm_get_first(PdDv_CLASS, sstr, pddv);
	if (rc==-1) {
		/* ODM failure */
		return(E_ODMGET);
	}
	else if (rc==0) {
		/* No PdDv object for this device */

		/* See if predefined data can be obtained from feature ROM */
		/* This can always be attempted in phase 1 */
		/* Otherwise it can only be attempted if CCM attrs exist */

		if (phase != PHASE1) {
			/* Not phase 1, see if CCM attrs exist */
			sprintf(sstr,"name=%s AND attribute like rcrw*",
								busname);
			rc = (int)odm_get_first(CuAt_CLASS,sstr,&CuAt);
			if (rc == -1)
				return(E_ODMGET);	/* ODM failure */
			else if (rc == 0)
				/* No CCM attr, so can't get PdDv from ROM */
				return(E_FINDCHILD);
		}

		/* OK to attempt to get PdDv from feature ROM */
		rc = FRS_Add_MCA_Device(busname, slot);
		if (rc)
			/* Did not get predefineds from feature ROM */
			return(E_FINDCHILD);

		/* Obtained predefineds from feature ROM */
		/* Attempt to get device information from PdDv again */
		sprintf(sstr, "devid=0x%04x AND subclass=mca", cardid);
		rc = (int)odm_get_first(PdDv_CLASS, sstr, pddv);
		if (rc==-1) {
			/* ODM failure */
			return(E_ODMGET);
		}
		else if (rc==0) {
			/* No PdDv object for this device */
			return(E_FINDCHILD);
		}
	}

	/* Trace the predefined data */
	if (prntflag)
		fprintf(trace_file,"\tuniquetype = %s\n\tbus_ext = %d\n",
			pddv->uniquetype,pddv->bus_ext);

	return(0);
}

/* ------------------------------------------------------------------------- */
/*
 * NAME: get_cudv_entry 
 *
 * FUNCTION: Locates the correct customized entry matching card type and slot.
 *           Defines new customized entry if necessary.
 *
 * RETURNS: 0 if customized device information found
 *          >0 if error encountered
 */

int
get_cudv_entry(busname, slot, pddv, cudv)

char	*busname;		/* parent bus name */
int	slot;			/* slot containing card */
struct PdDv *pddv;		/* PdDv for card */
struct CuDv *cudv;		/* Returned CuDv for card */
{
	int	rc;
	char	sstr[80];
	char	cmdarg[DEFAULTSIZE];
	char	*out_ptr = NULL;
	char	*err_ptr = NULL;


	/* Attempt to get CuDv for card */
	/* NOTE: Maybe should always attempt to find an AVAILABLE CuDv first*/
	sprintf(sstr, "parent=%s AND connwhere=%d AND PdDvLn=%s",
					busname, slot, pddv->uniquetype);
	rc = (int)odm_get_first(CuDv_CLASS, sstr, cudv);
	if (rc==-1) {
		/* ODM failure */
		return(E_ODMGET);
	}
	else if (rc==0) {

		/* No CuDv object for this device, need to define one */
		sprintf(cmdarg, "-c %s -s %s -t %s -p %s -w %d",
			pddv->class,pddv->subclass,pddv->type,busname,slot);

		rc = odm_run_method(pddv->Define, cmdarg, &out_ptr, &err_ptr);

		if (prntflag) {
			fprintf(trace_file,"invoke: %s %s\nrc = %d\n",
						pddv->Define, cmdarg, rc);
			trace_output( out_ptr, err_ptr);
		}

		/* Attempt to get CuDv that was just defined for card */
		rc = (int)odm_get_first(CuDv_CLASS, sstr, cudv);
		if (rc==-1) {
			/* ODM failure */
			return(E_ODMGET);
		}
		else if (rc==0) {
			/* No CuDv found for object, define failed.
			   Could be define method not found in phase
			   1 boot */
			if (prntflag)
				fprintf (trace_file, "Could not find CuDv Object\n");
			return(E_NOCuDv);
		}
	}

	/* Successfully obtained the card's matching CuDv object */
	/* Set the chgstatus field */
	if (cudv->chgstatus == MISSING) {
		cudv->chgstatus = SAME;
		if (odm_change_obj(CuDv_CLASS, cudv) == -1)
			return(E_ODMUPDATE);
	}

	/* Trace the customized data */
	if (prntflag)
		fprintf(trace_file,
			"\tname= %s\n\tstatus= %d\n\tchgstatus = %d\n",
			cudv->name,cudv->status,cudv->chgstatus);

	return(0);
}


/* ------------------------------------------------------------------------- */
/*
 * NAME: invoke_adapt_cfg 
 *
 * FUNCTION: called to define children of a bus extender. 
 *
 * RETURNS: 0 if customized device information found
 *          >0 if error encountered
 */

int
invoke_adapt_cfg(phase, pddv, cudv)
int	phase;		/* IPL phase */
struct PdDv *pddv;	/* PdDv for card */
struct CuDv *cudv;	/* CuDv for card */

{
	int	rc;
	char	cmdarg[DEFAULTSIZE];
	char	*out_ptr = NULL;
	char	*err_ptr = NULL;


	if (phase != 0)
		sprintf(cmdarg, "-%d -l %s", phase, cudv->name);
	else
		sprintf(cmdarg, "-l %s", cudv->name);

	rc = odm_run_method(pddv->Configure, cmdarg, &out_ptr, &err_ptr);

	/* if outptr not null and length not 0, print pkg names */
	if (out_ptr != NULL && strlen(out_ptr) != 0 )
		output_pkgs(out_ptr);

	if (prntflag) {
		fprintf(trace_file,"invoke: %s %s\nrc = %d\n",
					pddv->Configure, cmdarg, rc);
		trace_output( out_ptr, err_ptr);
	}

	return(rc);
}

/* -------------------------------------------------------------------------- */
/*
 * NAME: sync_bus 
 *
 * FUNCTION: This function finds the CuDv objects corresponding to the
 *           adapter cards that have been found to be present on the bus.
 *           It defines new CuDv objects for new adapters.  It sets the
 *           chgstatus in the CuDv object so that busresolve() will
 *           know what adapters need to have their bus resources
 *           resolved.
 *
 * RETURNS: 0 if successful
 *          >0 if error encountered
 * successful. 
 *
 */

int
sync_bus(busname, phase, card_table, card_table_size)

char	*busname;	/* the name of the bus being configured */
int	phase;		/* The phase currently being executed.   */
ushort	card_table[];	/* The table of card id's.       */
int	card_table_size;

{
	struct PdDv	pddv;
	struct CuDv	cudv;
	register int    slot;	/* card slot number */
	int		rc;
	char		pkg_name[256];


	/* Process each card that was found to be present */
	for (slot = 0; slot < card_table_size; slot++) {

		/* Skip integrated SCSI and Ethernet adapters */
		if (slot == 14 || slot == 15)
			continue;

		/* Skip slots that have no cards */
		if (card_table[slot] == EMPTYSLOT || card_table[slot] == 0)
			continue;	/* slot was empty, next iteration */

		if (prntflag)
			fprintf(trace_file,
				"\nFound card in slot %d\n\tcardid = 0x%04x\n",
				slot, card_table[slot]);

		/* Get PdDv for card in slot */
		rc = get_pddv_entry(phase,busname,card_table[slot],slot,&pddv);

		if (rc == E_FINDCHILD || allpkg) {
			/* generate the pkg name- (posid bytes swapped) */
			sprintf(pkg_name,"%s.mca.%02x%02x", DEVPKG_PREFIX,
				card_table[slot] & 0xff, card_table[slot]>>8);

			/* output the pkg name */
			printf(":%s ", pkg_name);

			if (rc == E_FINDCHILD) {
				/* card has no PdDv installed so continue */
				if (prntflag)
					fprintf(trace_file,"\tNo PdDv found - need to install pkg: %s\n",pkg_name);
				continue;
			}

		}

		if (rc)
			/* fatal error */
			return(rc);

		/* Got the card's PdDv, now get its CuDv object */
		rc = get_cudv_entry(busname, slot, &pddv, &cudv);

		if (rc==E_NOCuDv)
			/* ignore if no CuDv found, process next object */
			continue;
		if (rc)
			/* fatal error */
			return(rc);

		/* Got the card's CuDv object */

		/* Disable SCSI and Direct Attached disk adapters if needed */
		if (phase == 1 && cudv.status == DEFINED)
			disable_bus_adapters(card_table[slot], slot, busname);

		/* If card is a bus extender, need to invoke its    */
		/* configure method in order to define its children */
		/* Note : ignore errors from invoke_adapt_cfg() to  */
		/* ensure all devices are processed if a Configure  */
		/* method fails.                                    */
		if (pddv.bus_ext) {
			rc = invoke_adapt_cfg(phase, &pddv, &cudv);

			/* Need to restore bus's LED value */
			if (phase)
				setleds(BUS_CONFIG_METHOD);
		}
	}

	return(0);
}

/*
 * Check for presence of SCSI_ADAPTER or BADISK_ADAPTER and disable them if
 * they are not in the AVAILABLE state in the CuDv. 
 */
static int
disable_bus_adapters(card_table_entry, slot, busname)
ushort	card_table_entry;
int	slot;
char	*busname;

{
	MACH_DD_IO	mddRecord;
	uchar		pos2_data;
	int		fd;
	char		name[100];

	if (card_table_entry == SCSI_ADAPTER)
		pos2_data = SCSI_ADAPTER_POS2;
	else if (card_table_entry == SCSI2_ADAPTER)
		pos2_data = SCSI_ADAPTER_POS2;
	else if (card_table_entry == BADISK_ADAPTER)
		pos2_data = BADISK_ADAPTER_POS2;
	else
		return(0);

	sprintf(name, "/dev/%s", busname);
	if ((fd = open(name, O_RDWR)) == -1)
		return(-1);

	/* build mdd record. */
	mddRecord.md_size = 1;
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_data = &pos2_data;
	mddRecord.md_addr = POSREG(2, ((slot - 1) & 0x0f));

	ioctl(fd, MIOCCPUT, &mddRecord);
	close(fd);

	return(0);
}
 
/* ------------------------------------------------------------------------- */
/*
 * NAME: output_pkgs
 *
 * FUNCTION: This function parses the output returned from a cfg method
 * and print the package name to stdout.  Package names begin with ":" 
 * and may be interspersed with logical device names.  The package name
 * is used by cfgmgr to determine installation packages.
 *
 *
 * RETURNS: 0 
 *
 */
int
output_pkgs( outptr )
char	*outptr;			/* ptr to stdout string produced by */
{					/* cfg method 			    */
	int	i, k;
	char	buf[FNAME_SIZE];	/* temp buffer for pkg name */

	i = k = 0 ;
	/* loop through output from run method */

	while (outptr[i] != NULL) {
		/* ':' designates beginning of a pkg name */
		if (outptr[i]  != ':' ){
			i++;
			continue;
		}	
		/* found start of pkg name, build buffer to print */
		while ( outptr[i] != ' '  &&
			outptr[i] != '\n' &&
			outptr[i] != '\t' &&
			outptr[i] != ','  ) {
			buf[k] = outptr[i];
			i++;
			k++;
		}
		buf[k] = '\0';
		/* print pkg name */
		printf("%s ",buf);
		k=0;
	} /* end while */
	return (0);
		 
}
	
		
/* ------------------------------------------------------------------------- */
/*
 * NAME: trace_output
 *
 * FUNCTION: This function parses the output returned from a cfg method
 * and print the package name to stdout.  Package names begin with ":" 
 * and may be interspersed with logical device names.  The package name
 * is used by cfgmgr to determine installation packages.
 *
 *
 * RETURNS: 0 
 *
 */
int
trace_output( outptr, errptr )
char	*outptr;			/* ptr to stdout string */
char	*errptr;			/* ptr to stderr string */

{

	if (outptr != NULL && strlen(outptr) != 0 )
		fprintf(trace_file,"*** stdout ***\n%s\n", outptr);
	else
		fprintf(trace_file,"*** no stdout ***\n");

	if (errptr != NULL && strlen(errptr) != 0 )
		fprintf(trace_file,"*** stderr ***\n%s\n", errptr);
	else
		fprintf(trace_file,"*** no stderr ***\n");

	return(0);
}
